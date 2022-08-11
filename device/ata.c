#include <device/ata.h>
#include <common/types.h>
#include <common/debug.h>
#include <common/utils.h>
#include <arch/x86.h>
#include <myos.h>
#include <kprintf.h>
#include <sys/isr.h>
#include <mm/kvmm.h>
#include <lib/string.h>

/**
 * @file device/ata.c
 */


// for more info: https://wiki.osdev.org/PCI_IDE_Controller

/**
 * Block Registers
 */

// r: data; w: data
#define reg_data(channel)        (channel->port_base + 0)
// r: error; w: features
#define reg_error(channel)       (channel->port_base + 1)
#define reg_features(channel)    (reg_error(channel))
// r: sector count; w: sector count
#define reg_sect_cnt(channel)    (channel->port_base + 2)
// r: lba low/mid/high; w: lba low/mid/high
#define reg_lba_l(channel)       (channel->port_base + 3)
#define reg_lba_m(channel)       (channel->port_base + 4)
#define reg_lba_h(channel)       (channel->port_base + 5)
// r: device; w: device
#define reg_dev(channel)         (channel->port_base + 6)
// r: status; w: command
#define reg_status(channel)      (channel->port_base + 7)
#define reg_cmd(channel)         (reg_status(channel))
// r: alternate status; w: device control
#define reg_alt_status(channel)  (channel->port_base + 0x206)
#define reg_dev_ctl(channel)     (reg_alt_status(channel))

/**
 * The Command/Status Port returns a bit mask
 * referring to the status of a channel when read.
 */

#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive ready
#define ATA_SR_DF      0x20    // Drive write fault
#define ATA_SR_DSC     0x10    // Drive seek complete
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_CORR    0x04    // Corrected data
#define ATA_SR_IDX     0x02    // Index
#define ATA_SR_ERR     0x01    // Error

/**
 * When you write to the Command/Status port,
 * you are executing one of the commands below.
 */

// Read sector
#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
// Write sector
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35

#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
// Identyfy ata device
#define ATA_CMD_IDENTIFY          0xEC

#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1


/**
 * The commands below are for ATAPI^ devices.
 * ^ATAPI: ATA packet interface, i.e. CD-ROM.
 */

#define ATAPI_CMD_READ            0xA8
#define ATAPI_CMD_EJECT           0x1B

/**
 * ATA_CMD_IDENTIFY_PACKET and ATA_CMD_IDENTIFY return
 * a buffer of 512 bytes called the identification space;
 * the following definitions are used to read information
 * from the identification space.
 */

#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

/**
 * device register
 * 
 * 7 |  1  |
 *   +-----+     /  Addressing mode
 * 6 | MOD | --> |  MOD = 0: CHS mode (Cylinder-Head-Sector Addressing)
 *   +-----+     \  MOD = 1: LBA mode (Logical Block Addressing)
 * 5 |  1  |
 *   +-----+     /  Select device
 * 4 | DEV | --> |  DEV = 0: master device
 *   +-----+     \  DEV = 1: slave device
 * 3 | HS3 | \
 *   +-----+  \
 * 2 | HS2 |  |  IF in CHS mode, these four bits select the head number.
 *   +-----+   > IF in LBA mode, HS0 through HS3 contain bit 24-27 of
 * 1 | HS1 |  |                  the LBA.
 *   +-----+  /
 * 0 | HS0 | /
 */

// dev reg placeholder (MBS is for "Must Be Set")
#define DEV_REG_MBS      0xA0
// CHS addressing mode
#define DEV_REG_MOD_CHS  0x00
// LBA addressing mode
#define DEV_REG_MOD_LBA  0x40
// master device
#define DEV_REG_DEV_MST  0x00
// slave device
#define DEV_REG_DEV_SLV  0x10

// for more info: https://wiki.osdev.org/Partition_Table
struct PartitionTableEntry {
    // 0x80 = bootable; 0x00 = no
    uint8_t bootable;

    // start head/sector/cylinder number of this partition
    // 8bits/6bits/10bits each, 3bytes in total
    uint8_t start_head;
    uint8_t start_sec : 6;
    uint16_t start_cyl : 10;

    // system id
    // 0x05 or 0x0f = extended partition entry
    uint8_t fs_type;

    // end head/sector/cylinder number of this partition
    // 8bits/6bits/10bits each, 3bytes in total
    uint8_t end_head;
    uint8_t end_sec : 6;
    uint16_t end_cyl : 10;

    // start lba of this partitioin
    uint32_t start_lba;
    // number of sectors in this partition
    uint32_t sec_cnt;
} __attr_packed;

typedef struct PartitionTableEntry part_tab_entry_t;

struct BootSector {
    // the first 446 bytes contain the boot code
    uint8_t boot_code[446];
    // the following 64 bytes contain 4 disk partition table entry
    // 16 bytes each
    part_tab_entry_t partition_table[4];
    // the last 2 bytes are 0x55, 0xaa
    uint16_t signature;
} __attr_packed;

typedef struct BootSector boot_sector_t;


// there are 2 ATA channels
static ata_channel_t channels[2];

// list of partitions
list_t partition_list;

partition_t *get_partition(const char *part_name) {
    list_node_t *p;
    __list_for_each((&partition_list), p) {
        partition_t *part = __container_of(partition_t, part_tag, p);
        if (strcmp(part_name, part->part_name) == 0) {
            return part;
        }
    }
    return NULL;
}

/**
 * @brief select the disk and sector(s) to read/write
 * @param hd target device
 * @param lba starting sector address
 * @param sec_cnt number of sectors (0 for 256)
 */
static void select_disk_sector(disk_t *hd, uint32_t lba, uint8_t sec_cnt);

/**
 * @brief select the disk
 * 
 * @param hd target device
 */
static void select_disk(disk_t *hd);

/**
 * @brief send command to channel
 * @param chan target channel
 * @param cmd command
 */
static inline void cmd_out(ata_channel_t *chan, uint8_t cmd) {
    chan->expecting_intr = True;
    outportb(reg_cmd(chan), cmd);
    thread_msleep(5);
}

/**
 * @brief read from sector
 * @param hd target device
 * @param buf data buffer
 * @param sec_cnt number of sectors (0 for 256)
 */
static void read_sector(disk_t *hd, void *buf, uint8_t sec_cnt);

/**
 * @brief write sector
 * @param hd target device
 * @param buf data buffer
 * @param sec_cnt number of sectors (0 for 256)
 */
static void write_sector(disk_t *hd, const void *buf, uint8_t sec_cnt);

/**
 * @brief [0x12, 0x34, 0x56, 0x78] -> [0x34, 0x12, 0x78, 0x56]
 */
static void swap_adjacent_bytes(const char *src, char *buf, uint32_t len) {
    uint32_t i;
    for (i = 0; i < len; i++) {
        buf[i + 1] = *src++;
        buf[i] = *src++;
    }
    buf[i] = '\0';
}

/**
 * @brief wait 30s for the hd to finish the submitted request.
 * 
 * @param hd target hd to waith
 * @return 0 - no err; 1 - error; 2 - timeout
 */
static int busy_wait(disk_t *hd) {
    ata_channel_t *chan = hd->my_channel;
    int sleep_time = 30 * 1000;
    while ((sleep_time -= 10) >= 0) {
        thread_msleep(10);
        uint8_t status = inportb(reg_alt_status(chan));
        if (status & ATA_SR_ERR) {
            return 1;  // error
        }
        if ((!(status & ATA_SR_BSY)) && (status & ATA_SR_DRQ)) {
            // device not busy and data requesty ready
            return 0;  // no err
        }
    }
    return 2;  // timeout
}

/**
 * @brief identify disk
 */
static void identify_disk(disk_t *hd) {
    select_disk(hd);
    cmd_out(hd->my_channel, ATA_CMD_IDENTIFY);
    // test whether it is empty or not
    if (inportb(reg_alt_status(hd->my_channel)) == 0) {
        hd->existed = False;
        return;
    }
    // sem_down(&(hd->my_channel->disk_done));
    if (busy_wait(hd) != 0) {
        hd->existed = False;
        return;
    }

    char id_info[512];
    read_sector(hd, id_info, 1);

    char buf[64];
    uint32_t sn_start = 10 * 2, sn_len = 20;
    uint32_t md_start = 27 * 2, md_len = 40;
    swap_adjacent_bytes(id_info + sn_start, buf, sn_len);
    strncpy(buf, hd->SN, sn_len);
    swap_adjacent_bytes(id_info + md_start, buf, md_len);
    strncpy(buf, hd->MODULE, md_len);
    uint32_t sec_start = 60 * 2;
    uint32_t sectors = *(uint32_t *)(id_info + sec_start);
    hd->sectors = sectors;
    hd->existed = True;
    return;
}


static void select_disk_sector(disk_t *hd, uint32_t lba, uint8_t sec_cnt) {
    ata_channel_t *chan = hd->my_channel;
    outportb(reg_sect_cnt(chan), sec_cnt);
    outportb(reg_lba_l(chan), lba);
    outportb(reg_lba_m(chan), lba >> 8);
    outportb(reg_lba_h(chan), lba >> 16);
    uint8_t dev_reg = DEV_REG_MBS | DEV_REG_MOD_LBA;
    if (hd->dev_no == 1) {
        dev_reg |= DEV_REG_DEV_SLV;
    }
    dev_reg |= (lba >> 24);
    outportb(reg_dev(chan), dev_reg);
}


static void select_disk(disk_t *hd) {
    ata_channel_t *chan = hd->my_channel;
    uint8_t dev_reg = DEV_REG_MBS | DEV_REG_MOD_LBA;
    if (hd->dev_no == 1) {
        dev_reg |= DEV_REG_DEV_SLV;
    }
    outportb(reg_dev(chan), dev_reg);
}


static void read_sector(disk_t *hd, void *buf, uint8_t sec_cnt) {
    uint32_t num_bytes  = 512 * (sec_cnt == 0 ? 256 : sec_cnt);
    inportsw(reg_data(hd->my_channel), buf, num_bytes / 2);
}


static void write_sector(disk_t *hd, const void *buf, uint8_t sec_cnt) {
    uint32_t num_bytes  = 512 * (sec_cnt == 0 ? 256 : sec_cnt);
    outportsw(reg_data(hd->my_channel), buf, num_bytes / 2);
}


void ata_read(disk_t *hd, uint32_t lba, void *buf, uint32_t sec_cnt) {
    // we use ATA-1, which only supports 28-bit lba
    ASSERT(lba < 0x10000000);

    // can only access one disk on the same channel at the same time
    mutex_lock(&(hd->my_channel->chan_lock));

    // num of sectors done operated
    uint32_t sec_done = 0;
    while (sec_done < sec_cnt) {
        uint32_t num_sectors = MIN(sec_cnt - sec_done, 256);
        select_disk_sector(hd, lba + sec_done, num_sectors);
        cmd_out(hd->my_channel, ATA_CMD_READ_PIO);
        // kprintf(KPL_DEBUG, "chan: %s\n", hd->my_channel->chan_name);
        // use sem_down to wait for the completion of the disk
        // sem_down(&(hd->my_channel->disk_done));
        if (busy_wait(hd) != 0) {
            char error[64];
            ksprintf(error, "%s read sector %d failed", hd->disk_name, lba);
            PANIC(error);
        }
        read_sector(hd, (void *)((uintptr_t)buf + sec_done * 512), num_sectors);
        sec_done += num_sectors;
    }
    mutex_unlock(&(hd->my_channel->chan_lock));
}


void ata_write(disk_t *hd, uint32_t lba, const void *buf, uint32_t sec_cnt) {
    // we use ATA-1, which only supports 28-bit lba
    ASSERT(lba < 0x10000000);

    // can only access one disk on the same channel at the same time
    mutex_lock(&(hd->my_channel->chan_lock));

    // num of sectors done operated
    uint32_t sec_done = 0;
    while (sec_done < sec_cnt) {
        uint32_t num_sectors = MIN(sec_cnt - sec_done, 256);
        // if num_sectors == 256, will pass 0 to select_disk_sector
        select_disk_sector(hd, lba + sec_done, (uint8_t)num_sectors);
        cmd_out(hd->my_channel, ATA_CMD_WRITE_PIO);
        if (busy_wait(hd) != 0) {
            char error[64];
            ksprintf(error, "%s write sector %d failed", hd->disk_name, lba);
            PANIC(error);
        }
        write_sector(hd, (void *)((uintptr_t)buf + sec_done * 512), num_sectors);
        // use sem_down to wait for the completion of the device
        // sem_down(&(hd->my_channel->disk_done));
        sec_done += num_sectors;
    }
    mutex_unlock(&(hd->my_channel->chan_lock));
}


static void *hd_handler(isrp_t *p) {
    uint32_t irq_no = p->int_no;
    ASSERT(irq_no == INT_ATA0 || irq_no == INT_ATA1);
    ata_channel_t *chan = &(channels[irq_no - INT_ATA0]);
    ASSERT(chan->irq_no == irq_no);
    /**
     * this interrupt must be for the cmd we just sent out since
     * the mutex is used everytime when issuing a cmd.
     */
    if (chan->expecting_intr) {
        chan->expecting_intr = False;
        sem_up(&(chan->disk_done));
    }
    /**
     * to tell the hard drive controller that this interrupt is
     * handled so that further operations can be executed
     */
    inportb(reg_alt_status(chan));
    kprintf(KPL_DEBUG, "hd_handler: irq_no=%d\n", irq_no);
}

partition_t *first_part = NULL;
disk_t *first_disk = NULL;

static void partition_scan(disk_t *hd, uint32_t base_lba) {
    boot_sector_t *bs = kmalloc(sizeof(boot_sector_t));
    static uint32_t primary_ext_base_lba = 0;
    if (!bs) {
        PANIC("ERR_NOMEM");
    }
    ata_read(hd, base_lba, bs, 1);
    // traverse the 4 entries
    for (int i = 0; i < 4; i++) {
        part_tab_entry_t *pe = &(bs->partition_table[i]);
        if (pe->fs_type == 0) {
            // unused entry
            continue;
        } else if (pe->fs_type == 0x5 || pe->fs_type == 0xf) {
            // extended partition entry
            if (primary_ext_base_lba == 0) {
                primary_ext_base_lba = pe->start_lba;
                partition_scan(hd, pe->start_lba);
            } else {
                partition_scan(hd, primary_ext_base_lba + pe->start_lba);
            }
        } else {
            // normal partition entry
            partition_t *part;
            if (base_lba == 0) {
                // primary partition
                ASSERT(hd->p_no < 4);
                part = &(hd->prim_parts[hd->p_no]);
                ksprintf(part->part_name, "%s%d", hd->disk_name, 1 + hd->p_no++);
            } else {
                // logical partition
                if (hd->l_no >= 8) {
                    kfree(bs);
                    return;
                }
                part = &(hd->logic_parts[hd->l_no]);
                ksprintf(part->part_name, "%s%d", hd->disk_name, 5 + hd->l_no++);
            }
            part->start_lba = base_lba + pe->start_lba;
            part->sec_cnt = pe->sec_cnt;
            part->my_disk = hd;
            __list_push_back(&partition_list, part, part_tag);
            if (!first_part) {
                first_part = part;
            }
        }
    }
    kfree(bs);
}


static void print_partitions() {
    kprintf(KPL_NOTICE, "All partitions info:\n");
    list_node_t *p;
    __list_for_each((&partition_list), p) {
        partition_t *part = __container_of(partition_t, part_tag, p);
        kprintf(
            KPL_NOTICE,
            "    %s start_lba: 0x%X, sec_cnt: 0x%X\n",
            part->part_name, part->start_lba, part->sec_cnt
        );
    }
}

static void print_devices() {
    for (int i = 0; i < 2; i++) {
        ata_channel_t *ch = &(channels[i]);
        for (int j = 0; j < 2; j++) {
            disk_t *hd = &(ch->devices[j]);
            if (!(hd->existed)) {
                continue;
            }
            kprintf(KPL_NOTICE, "%s-%s: %s\n", ch->chan_name, j ? "slave" : "master", hd->disk_name);
            kprintf(KPL_NOTICE, "    SN       = %s\n", hd->SN, hd->sectors, hd->sectors / 2048);
            kprintf(KPL_NOTICE, "    MODULE   = %s\n", hd->MODULE);
            kprintf(KPL_NOTICE, "    SECTORS  = %d\n", hd->sectors);
            kprintf(KPL_NOTICE, "    CAPACITY = %dMiB\n", hd->sectors / 2048);
            kprintf(KPL_NOTICE, "    p_no = %d, l_no = %d\n", hd->p_no, hd->l_no);
        }
    }
}

void ata_init() {
    list_init(&partition_list);
    // there are 2 ata channels
    for (int i = 0; i < 2; i++) {
        ata_channel_t *ch = &(channels[i]);
        ksprintf(ch->chan_name, "ata%d", i);
        if (i == 0) {
            ch->port_base = 0x1F0;
            ch->irq_no = INT_ATA0;
        } else {
            ch->port_base = 0x170;
            ch->irq_no = INT_ATA1;
        }
        ch->expecting_intr = False;
        // disable hd int at all
        outportb(reg_dev_ctl(ch), 1 << 1);
        mutex_init(&(ch->chan_lock));
        sem_init(&(ch->disk_done), 0);
        register_handler(ch->irq_no, hd_handler);
        // each channel has up to 2 devices
        for (int j = 0; j < 2; j++) {
            disk_t *hd = &(ch->devices[j]);
            hd->my_channel = ch;
            hd->dev_no = j;
            identify_disk(hd);
            if (!(hd->existed)) {
                continue;
            }
            ksprintf(hd->disk_name, "sd%c", 'a' + i * 2 + j);
            // hd->existed = True;
            if (!first_disk) {
                first_disk = hd;
            }
            hd->p_no = hd->l_no = 0;
            partition_scan(hd, 0);
        }
    }

    if (first_disk == NULL) {
        PANIC("No disk!");
    }

    if (first_part == NULL) {
        boot_sector_t *bs = kmalloc(sizeof(boot_sector_t));
        if (!bs) {
            PANIC("ERR_NOMEM");
        }
        part_tab_entry_t *pte = &(bs->partition_table[0]);
        pte->bootable = 0x00;
        pte->fs_type = 0x66;  // myOS
        pte->start_lba = 0x800;
        pte->sec_cnt = (first_disk->sectors - pte->start_lba);
        ata_write(first_disk, 0, bs, 1);
        kfree(bs);
        partition_scan(first_disk, 0);
    }

    ASSERT(first_part != NULL);

    print_devices();
    print_partitions();
}


void dirty_blocks_add(partition_t *part, uint32_t lba, void *data) {
    // for (int i = 0; i < NR_DIRTY_BLOCKS; i++) {
    //     if (lba == part->dirty_blocks[i].first) {
    //         ASSERT(part->dirty_blocks[i].second == data);
    //         return;
    //     } else if (part->dirty_blocks[i].first == 0) {
    //         part->dirty_blocks[i].first = lba;
    //         part->dirty_blocks[i].second = data;
    //         return;
    //     }
    // }
    // dirty_blocks_sync(part);
    // ASSERT(part->dirty_blocks[0].first == 0);
    // part->dirty_blocks[0].first = lba;
    // part->dirty_blocks[0].second = data;
    // return;
}

void dirty_blocks_sync(partition_t *part) {
    // for (int i = 0; i < NR_DIRTY_BLOCKS; i++) {
    //     lba_data_pair_t *p = &(part->dirty_blocks[i]);
    //     if (p->first != 0) {
    //         ASSERT(__valid_kva(p->second));
    //         ata_write(part->my_disk, p->first, p->second, 1);
    //         p->first = 0;
    //         p->second = NULL;
    //     } else {
    //         break;
    //     }
    // }
}
