/**
 * @file simplefs.c
 */


#include "bitmap.h"

#include "ata.h"
#include "simplefs.h"


#define SECTOR_SIZE 512
#define SECTOR_SIZE_IN_BIT (8 * SECTOR_SIZE)
#define SIMPLE_FS_MAGIC 0x12345678
#define BOOT_BLOCK_SEC_CNT 1
#define SUPER_BLOCK_SEC_CNT 1
#define MAX_FILE_CNT 4096
#define MAX_FILE_OPEN 32
#define MAX_FILE_SIZE 65536

#define __simplefs_field(part, member) (((simplefs_struct_t *)(part->fs_struct))->member)
#define NR_DESC_PER_SEC (SECTOR_SIZE / sizeof(file_desc_t))

typedef struct {
    // lba of the idx array
    uint32_t idx_lba;
    // number of bytes in this file
    uint32_t size;
    // filename
    char filename[MAX_FILENAME_LENGTH + 1];
} __attr_packed file_desc_t;

typedef struct {
    file_desc_t desc;
    int desc_id;
    // current file pointer position
    uint32_t file_pos;
} file_t;

typedef struct {
    uint32_t fs_type;
    // sectors used by simplefs
    uint32_t start_lba;
    uint32_t sec_cnt;

    // an on disk array of file descriptors
    uint32_t file_cnt;
    uint32_t file_desc_table_start_lba;
    uint32_t file_desc_table_sec_cnt;

    // an on-disk bitmap for used/free sectors
    uint32_t sector_btmp_start_lba;
    uint32_t sector_btmp_sec_cnt;

    // sectors with lba >= data_start_lba are free to use for files
    uint32_t data_start_lba;

} __attr_packed super_block_t;

// implements partition_t.fs_sturct
typedef struct {
    super_block_t *sb;
    btmp_t sector_btmp;
    // in memory bitmap to record which fids are used
    btmp_t file_btmp;
} simplefs_struct_t;

extern partition_t *first_part;

// simple fs only operates the first partition
static partition_t *__part = NULL;
static simplefs_struct_t *__simplefs = NULL;
static file_t __file_table[MAX_FILE_OPEN];

static void sync_sec_btmp(int id) {
    int btmp_lba = __simplefs->sb->sector_btmp_start_lba + id / SECTOR_SIZE_IN_BIT;
    void *data = __simplefs->sector_btmp.bits_ + id / SECTOR_SIZE_IN_BIT * SECTOR_SIZE;
    ata_write(__part->my_disk, btmp_lba, data, 1);
}

static int sec_alloc() {
    ASSERT(__simplefs != NULL);
    int idx = bitmap_scan(&(__simplefs->sector_btmp), 1);
    if (idx == -1) {
        return -1;
    }
    bitmap_set(&(__simplefs->sector_btmp), idx, 1);
    sync_sec_btmp(idx);
    return __simplefs->sb->start_lba + idx;
}

static void sec_free(uint32_t lba) {
    ASSERT(lba >= __simplefs->sb->start_lba);
    int idx = lba - __simplefs->sb->start_lba;
    ASSERT(idx < __simplefs->sb->sec_cnt);
    bitmap_set(&(__simplefs->sector_btmp), idx, 0);
    sync_sec_btmp(idx);
}

static void print_simplefs(const simplefs_struct_t *simplefs) {
    super_block_t *sb = simplefs->sb;
    kprintf(
        KPL_DEBUG,
        "simplefs on %s\n"
        "    fs_type: 0x%x\n"
        "    start_lba: 0x%x\n"
        "    sec_cnt: 0x%x\n"
        "    file_desc_table_start_lba: 0x%x\n"
        "    file_desc_table_sec_cnt: 0x%x\n"
        "    sector_btmp_start_lba: 0x%x\n"
        "    sector_btmp_sec_cnt: 0x%x\n"
        "    data_start_lba: 0x%x\n",
        __part->part_name,
        sb->fs_type, sb->start_lba, sb->sec_cnt,
        sb->file_desc_table_start_lba, sb->file_desc_table_sec_cnt,
        sb->sector_btmp_start_lba, sb->sector_btmp_sec_cnt,
        sb->data_start_lba
    );
    print_btmp(&(simplefs->sector_btmp));
    print_btmp(&(simplefs->file_btmp));
}

static void sync_super_block(uint8_t *buf) {
    memset(buf, 0, SECTOR_SIZE);
    memcpy(__simplefs->sb, buf, sizeof(super_block_t));
    ata_write(__part->my_disk, __part->start_lba + 1, buf, 1);
}

static void sync_desc_table(uint32_t file_id, file_desc_t *fdesc, uint8_t *buf) {
    super_block_t *sb = __simplefs->sb;
    uint32_t lba = sb->file_desc_table_start_lba + file_id / NR_DESC_PER_SEC;
    uint32_t offset = file_id % NR_DESC_PER_SEC;
    ata_read(__part->my_disk, lba, buf, 1);
    memcpy(fdesc, (file_desc_t *)buf + offset, sizeof(file_desc_t));
    ata_write(__part->my_disk, lba, buf, 1);
}

static int gfd_alloc() {
    for (int i = 0; i < MAX_FILE_OPEN; i++) {
        if (__file_table[i].desc.idx_lba == 0) {
            return i;
        }
    }
    return -1;
}

static void gfd_free(int gfd) {
    if (gfd == -1) {
        return;
    }
    ASSERT(0 <= gfd && gfd < MAX_FILE_OPEN);
    __file_table[gfd].desc.idx_lba = 0;
}

static void format_partition(uint8_t *buf, super_block_t *sb) {
    ASSERT(__part != NULL);
    sb->fs_type = SIMPLE_FS_MAGIC;
    sb->start_lba = __part->start_lba;
    sb->sec_cnt = __part->sec_cnt;

    sb->file_cnt = 0;
    sb->file_desc_table_start_lba = sb->start_lba + 2;
    sb->file_desc_table_sec_cnt = ROUND_UP_DIV(MAX_FILE_CNT, SECTOR_SIZE / sizeof(file_desc_t));

    sb->sector_btmp_start_lba = sb->file_desc_table_start_lba + sb->file_desc_table_sec_cnt;
    sb->sector_btmp_sec_cnt = ROUND_UP_DIV(sb->sec_cnt, SECTOR_SIZE_IN_BIT);

    sb->data_start_lba = sb->sector_btmp_start_lba + sb->sector_btmp_sec_cnt;

    kprintf(KPL_DEBUG, "%s info: \n", __part->part_name);
    kprintf(
        KPL_DEBUG,
        "    fs_type: 0x%x\n"
        "    start_lba: 0x%x\n"
        "    sec_cnt: 0x%x\n"
        "    file_desc_table_start_lba: 0x%x\n"
        "    file_desc_table_sec_cnt: 0x%x\n"
        "    sector_btmp_start_lba: 0x%x\n"
        "    sector_btmp_sec_cnt: 0x%x\n"
        "    data_start_lba: 0x%x\n",
        sb->fs_type, sb->start_lba, sb->sec_cnt,
        sb->file_desc_table_start_lba, sb->file_desc_table_sec_cnt,
        sb->sector_btmp_start_lba, sb->sector_btmp_sec_cnt,
        sb->data_start_lba
    );

    memcpy(sb, buf, sizeof(super_block_t));
    kprintf(KPL_NOTICE, "  Writing super block to disk... ");
    ata_write(__part->my_disk, __part->start_lba + 1, buf, 1);
    kprintf(KPL_NOTICE, "  Done!\n");
    memset(buf, 0, SECTOR_SIZE);

    uint32_t bit_len = sb->sector_btmp_sec_cnt * SECTOR_SIZE_IN_BIT;

    // [0, sb->data_start_lba) -> 1
    // [sb->data_start_lba, sb->sec_cnt) -> 0
    // [sb->sec_cnt, bit_len) -> 1

    kprintf(KPL_NOTICE, "Used sectors: [0x%X, 0x%X){%x}\n", sb->start_lba, sb->data_start_lba, sb->data_start_lba - sb->start_lba);
    kprintf(KPL_NOTICE, "Free sectors: [0x%X, 0x%X){%x}\n", sb->data_start_lba, sb->start_lba + sb->sec_cnt, sb->start_lba + sb->sec_cnt - sb->data_start_lba);
    kprintf(KPL_NOTICE, "None sectors: [0x%X, 0x%X){%x}\n", sb->start_lba + sb->sec_cnt, sb->start_lba + bit_len, bit_len - sb->sec_cnt);

    kprintf(KPL_NOTICE, "  Writing sector bitmap to disk... ");

    int with1 = 0;
    for (uint32_t i = 0; i < bit_len; i++) {
        if (i < sb->data_start_lba || i >= sb->sec_cnt) {
            int id = i % SECTOR_SIZE_IN_BIT;
            buf[id / 8] |= (1 << (id % 8));
            with1 = 1;
        }
        if ((i + 1) % SECTOR_SIZE_IN_BIT == 0) {
            uint32_t lba = sb->sector_btmp_start_lba + i / SECTOR_SIZE_IN_BIT;
            ata_write(__part->my_disk, lba, buf, 1);
            if (with1) {
                memset(buf, 0, SECTOR_SIZE);
                with1 = 0;
            }
        }
    }

    kprintf(KPL_NOTICE, "  Done!\n");

    memset(buf, 0, SECTOR_SIZE);
    kprintf(KPL_NOTICE, "  Writing empty desc table to disk... ");
    for (uint32_t i = 0; i < sb->file_desc_table_sec_cnt; i++) {
        ata_write(__part->my_disk, sb->file_desc_table_start_lba + i, buf, 1);
    }
    kprintf(KPL_NOTICE, "  Done!\n");

    kprintf(KPL_NOTICE, "Done formatting partition %s with simplefs\n", __part->part_name);
}

void simplefs_init() {
    // __part = __container_of(partition_t, part_tag, list_front(&partition_list));
    __part = first_part;
    if (__part == NULL) {
        PANIC("simplefs failed to init: no first partition!");
    }
    uint8_t *buf = kmalloc(SECTOR_SIZE);
    super_block_t *sb = kmalloc(sizeof(super_block_t));
    simplefs_struct_t *simplefs = kmalloc(sizeof(simplefs_struct_t));
    if (buf == NULL || sb == NULL || simplefs == NULL) {
        PANIC("simplefs failed to init: no mem!");
    }
    ata_read(__part->my_disk, __part->start_lba + 1, buf, 1);
    if (*(uint32_t *)buf != SIMPLE_FS_MAGIC) {
        format_partition(buf, sb);
    } else {
        memcpy(buf, sb, sizeof(super_block_t));
    }

    // mount part
    uint32_t byte_len = sb->sector_btmp_sec_cnt * SECTOR_SIZE;
    simplefs->sector_btmp.bits_ = kmalloc(byte_len);
    if (simplefs->sector_btmp.bits_ == NULL) {
        PANIC("simplefs failed to init: no mem!");
    }
    ata_read(
        __part->my_disk, sb->sector_btmp_start_lba,
        simplefs->sector_btmp.bits_, sb->sector_btmp_sec_cnt
    );
    bitmap_reinit(&(simplefs->sector_btmp), byte_len);

    // init file bitmap
    byte_len = MAX_FILE_CNT / 8;
    btmp_t *fbtmp = &(simplefs->file_btmp);
    fbtmp->bits_ = kmalloc(byte_len);
    if (fbtmp->bits_ == NULL) {
        PANIC("simplefs failed to init: no mem!");
    }
    bitmap_init(fbtmp, byte_len);

    for (uint32_t i = 0; i < sb->file_desc_table_sec_cnt; i++) {
        uint32_t lba = sb->file_desc_table_start_lba + i;
        ata_read(__part->my_disk, lba, buf, 1);
        file_desc_t *fds = buf;
        for (uint32_t j = 0; j < NR_DESC_PER_SEC; j++) {
            if (fds[j].idx_lba != 0) {
                bitmap_set(fbtmp, i * NR_DESC_PER_SEC + j, 1);
            }
        }
    }

    ASSERT(sb->file_cnt == MAX_FILE_CNT - fbtmp->num_zero);

    simplefs->sb = sb;
    __part->fs_struct = simplefs;
    __simplefs = simplefs;
    kfree(buf);
    print_simplefs(simplefs);
}

void simplefs_close() {
    kfree(__simplefs->sb);
    kfree(__simplefs->sector_btmp.bits_);
    kfree(__simplefs->file_btmp.bits_);
    kfree(__simplefs);
}

static int file_create(const char *filename, file_desc_t *desc, uint8_t *buf) {
    super_block_t *sb = __simplefs->sb;
    int fid = -1, lba = -1;

    if (
        (fid = bitmap_scan(&(__simplefs->file_btmp), 1)) >= 0 &&
        (lba = sec_alloc()) > 0
    ) {
        bitmap_set(&(__simplefs->file_btmp), fid, 1);

        // clean the index array
        memset(buf, 0, SECTOR_SIZE);
        ata_write(__part->my_disk, lba, buf, 1);

        strcpy(filename, desc->filename);
        desc->size = 0;
        desc->idx_lba = lba;

        // modify file desc table
        sync_desc_table(fid, desc, buf);

        // modify super block
        sb->file_cnt++;
        ASSERT(sb->file_cnt <= MAX_FILE_CNT);
        sync_super_block(buf);
    }

    return fid;
}

static int find(const char *filename, file_desc_t *fd, uint8_t *buf, int create) {
    super_block_t *sb = __simplefs->sb;
    int avail_lba = sb->file_desc_table_start_lba - 1;

    btmp_t *fbtmp = &(__simplefs->file_btmp);
    ASSERT(fbtmp->num_zero + sb->file_cnt == MAX_FILE_CNT);
    if (sb->file_cnt > 0) {
        for (uint32_t i = 0; i < MAX_FILE_CNT; i++) {
            if (bitmap_bit_test(fbtmp, i)) {
                uint32_t this_lba = sb->file_desc_table_start_lba + i / NR_DESC_PER_SEC;
                if (this_lba > avail_lba) {
                    ata_read(__part->my_disk, this_lba, buf, 1);
                    avail_lba = this_lba;
                }
                file_desc_t *fdesc = (file_desc_t *)buf + i % NR_DESC_PER_SEC;
                if (strcmp(fdesc->filename, filename) == 0) {
                    memcpy(fdesc, fd, sizeof(file_desc_t));
                    return i;
                }
            }
        }
    }

    int fid = -1;
    if (create) {
        fid = file_create(filename, fd, buf);
    }
    return fid;
}


static int lfd_install(int gfd) {
    ASSERT(0 <= gfd && gfd < MAX_FILE_OPEN);
    return gfd + 3;
}

static void lfd_free(int lfd) {
    if (lfd == -1) {
        return;
    }
}

static int lfd2gfd(int lfd) {
    if (lfd < 0 || lfd >= 8) {
        return -1;
    }
    int gfd = lfd - 3;
    return gfd;
}

static file_t *lfd2file(int lfd) {
    int gfd = lfd2gfd(lfd);
    if (gfd < 0 || gfd >= MAX_FILE_OPEN) {
        return NULL;
    }
    return &(__file_table[gfd]);
}

static bool_t already_open(const char *filename) {
    for (int i = 0; i < MAX_FILE_OPEN; i++) {
        if (
            __file_table[i].desc.idx_lba != 0 &&
            strcmp(__file_table[i].desc.filename, filename) == 0
        ) {
            return True;
        }
    }
    return False;
}

int simplefs_file_open(const char *filename, uint32_t flags) {
    int gfd = -1, lfd = -1;
    uint8_t *buf = NULL;

    if (
        strlen(filename) <= MAX_FILENAME_LENGTH &&  // valid filename
        !already_open(filename) &&  // not open
        (gfd = gfd_alloc()) != -1 &&  // valid gfd
        (lfd = lfd_install(gfd)) != -1 &&  // valid lfd
        (buf = kmalloc(SECTOR_SIZE)) != NULL &&  // enough memory
        (__file_table[gfd].desc_id = find(filename, &(__file_table[gfd].desc), buf, flags & O_CREAT)) >= 0
    ) {
        __file_table[gfd].file_pos = 0;
    } else {
        lfd_free(lfd);
        lfd = -1;
        gfd_free(gfd);
        gfd = -1;
    }

    kfree(buf);
    return lfd;
}

int simplefs_file_close(int fd) {
    int gfd = lfd2gfd(fd);
    if (gfd < 0 || gfd > MAX_FILE_OPEN) {
        return -1;
    }

    __file_table[gfd].desc.idx_lba = 0;
    gfd_free(gfd);
    lfd_free(fd);
    return 0;
}

int simplefs_file_write(int fd, const void *buffer, size_t count) {
    file_t *fp = lfd2file(fd);
    if (fp == NULL) {
        // bad local fd
        return -1;
    }
    ASSERT(fp->file_pos <= fp->desc.size);
    count = MIN(count, MAX_FILE_SIZE - fp->file_pos);

    if (count == 0) {
        return 0;
    }

    uint8_t *io_buf = kmalloc(2 * SECTOR_SIZE);
    if (io_buf == NULL) {
        // no mem
        return -1;
    }
    int *lbas = io_buf + SECTOR_SIZE;
    ata_read(__part->my_disk, fp->desc.idx_lba, lbas, 1);

    int bytes_written = 0;
    int pos = fp->file_pos;
    while (bytes_written < count) {
        int sec_no = pos / SECTOR_SIZE;
        int sec_off = pos % SECTOR_SIZE;
        int bytes_to_write = MIN(count - bytes_written, SECTOR_SIZE - sec_off);
        bool_t new_blk = False;
        if (lbas[sec_no] <= 0) {
            ASSERT(sec_off == 0);
            lbas[sec_no] = sec_alloc();
            if (lbas[sec_no] <= 0) {
                // no blks
                break;
            }
            new_blk = True;
        }
        ASSERT(lbas[sec_no] > 0);
        if (!new_blk) {
            ata_read(__part->my_disk, lbas[sec_no], io_buf, 1);
        }
        memcpy((uint8_t *)buffer + bytes_written, io_buf + sec_off, bytes_to_write);
        ata_write(__part->my_disk, lbas[sec_no], io_buf, 1);
        pos += bytes_to_write;
        bytes_written += bytes_to_write;
    }

    if (pos > fp->desc.size) {
        // sync lbas
        ata_write(__part->my_disk, fp->desc.idx_lba, lbas, 1);
    }

    fp->file_pos = pos;
    fp->desc.size = MAX(fp->desc.size, fp->file_pos);

    sync_desc_table(fp->desc_id, &(fp->desc), io_buf);
    kfree(io_buf);
    return bytes_written;
}

int simplefs_file_read(int fd, void *buffer, size_t count) {
    file_t *fp = lfd2file(fd);
    if (fp == NULL) {
        // bad local fd
        return -1;
    }
    ASSERT(fp->file_pos <= fp->desc.size);
    count = MIN(count, fp->desc.size - fp->file_pos);
    if (count == 0) {
        return 0;
    }
    uint8_t *io_buf = kmalloc(2 * SECTOR_SIZE);
    if (io_buf == NULL) {
        // no mem
        return -1;
    }
    int *lbas = io_buf + SECTOR_SIZE;
    ata_read(__part->my_disk, fp->desc.idx_lba, lbas, 1);

    int bytes_read = 0;
    int pos = fp->file_pos;
    while (bytes_read < count) {
        int sec_no = pos / SECTOR_SIZE;
        int sec_off = pos % SECTOR_SIZE;
        int bytes_to_read = MIN(count - bytes_read, SECTOR_SIZE - sec_off);
        ASSERT(lbas[sec_no] > 0);
        ata_read(__part->my_disk, lbas[sec_no], io_buf, 1);
        memcpy(io_buf + sec_off, (uint8_t *)buffer + bytes_read, bytes_to_read);
        pos += bytes_to_read;
        bytes_read += bytes_to_read;
    }

    ASSERT(pos <= fp->desc.size);
    fp->file_pos = pos;
    kfree(io_buf);
    return bytes_read;
}

off_t simplefs_file_lseek(int fd, off_t offset, int whence) {
    file_t *fp = lfd2file(fd);
    if (fp == NULL) {
        // bad local fd
        return -1;
    }
    int new_pos = -1;
    if (whence == SEEK_SET) {
        new_pos = offset;
    } else if (whence == SEEK_CUR) {
        new_pos = (int)(fp->file_pos) + offset;
    } else if (whence == SEEK_END) {
        new_pos = (int)(fp->desc.size) + offset;
    } else {
        // bad whence
        return -1;
    }
    if (new_pos < 0 || new_pos > fp->desc.size) {
        return -1;
    }
    return fp->file_pos = new_pos;
}

int simplefs_file_delete(const char *filename) {
    file_desc_t desc;
    int fid_to_delete;
    uint8_t *buf = NULL;

    int ret = -1;

    if (
        __simplefs->sb->file_cnt > 0 &&
        strlen(filename) <= MAX_FILENAME_LENGTH &&  // valid filename
        !already_open(filename) &&  // not open
        (buf = kmalloc(SECTOR_SIZE)) != NULL &&
        (fid_to_delete = find(filename, &desc, buf, 0)) >= 0  // existed file
    ) {
        // reclaim sectors
        if (desc.size > 0) {
            ata_read(__part->my_disk, desc.idx_lba, buf, 1);
            int *lbas = buf;
            for (int i = 0; i < ROUND_UP_DIV(desc.size, SECTOR_SIZE); i++) {
                ASSERT(lbas[i] > 0);
                sec_free(lbas[i]);
            }
            sec_free(desc.idx_lba);
        }

        // clear the last file desc
        desc.idx_lba = 0;
        desc.size = 0;
        sync_desc_table(fid_to_delete, &desc, buf);
        __simplefs->sb->file_cnt--;
        sync_super_block(buf);

        // reclaim the fid
        bitmap_set(&(__simplefs->file_btmp), fid_to_delete, 0);

        ret = 0;
    }

    kfree(buf);
    return ret;
}

int simplefs_file_stat(const char *filename, stat_t *s) {
    file_desc_t fdesc;
    int fid = -1;
    uint8_t *buf = NULL;
    int ret = -1;
    if (
        strlen(filename) <= MAX_FILENAME_LENGTH &&
        (buf = kmalloc(SECTOR_SIZE)) != NULL &&
        (fid = find(filename, &fdesc, buf, 0)) >= 0
    ) {
        strcpy(filename, s->filename);
        s->size = fdesc.size;
        s->file_id = fid;
        s->blocks = 1 + ROUND_UP_DIV(s->size, SECTOR_SIZE);
        ret = 0;
    }
    kfree(buf);
    return ret;
}

int simplefs_list_files(stat_t *s) {
    super_block_t *sb = __simplefs->sb;
    uint8_t *buf = NULL;
    btmp_t *fbtmp = &(__simplefs->file_btmp);
    ASSERT(fbtmp->num_zero + sb->file_cnt == MAX_FILE_CNT);
    int ret = -1;
    if (sb->file_cnt == 0) {
        ret = 0;
    } else if ((buf = kmalloc(SECTOR_SIZE)) != NULL) {
        int avail_lba = sb->file_desc_table_start_lba - 1;
        int nfiles = 0;
        for (uint32_t i = 0; i < MAX_FILE_CNT; i++) {
            if (bitmap_bit_test(fbtmp, i)) {
                uint32_t this_lba = sb->file_desc_table_start_lba + i / NR_DESC_PER_SEC;
                if (this_lba > avail_lba) {
                    ata_read(__part->my_disk, this_lba, buf, 1);
                    avail_lba = this_lba;
                }
                file_desc_t *fdesc = (file_desc_t *)buf + i % NR_DESC_PER_SEC;
                strcpy(fdesc->filename, s[nfiles].filename);
                s[nfiles].size = fdesc->size;
                s[nfiles].file_id = i;
                s[nfiles].blocks = 1 + ROUND_UP_DIV(fdesc->size, SECTOR_SIZE);
                nfiles++;
            }
        }
        ASSERT(nfiles == sb->file_cnt);
        ret = nfiles;
    }
    return ret;
}