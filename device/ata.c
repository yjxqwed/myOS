#include <device/ata.h>
#include <common/types.h>
#include <common/debug.h>
#include <common/utils.h>
#include <arch/x86.h>
#include <myos.h>
#include <kprintf.h>
#include <sys/isr.h>

/**
 * @file device/ata.c
 * @brief from 《操作系统真象还原》
 */


/**
 * Block Registers
 */

// r: data; w: data
#define reg_data(channel)        (channel->port_base + 0)
// r: error; w: reatures
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
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
// Identyfy ata device
#define ATA_CMD_IDENTIFY          0xEC

/**
 * The commands below are for ATAPI^ devices.
 * ^ATAPI: ATA packet interface, i.e. CD-ROM.
 */

#define      ATAPI_CMD_READ       0xA8
#define      ATAPI_CMD_EJECT      0x1B

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

// dev reg placeholder (MBS is for must be set)
#define DEV_REG_MBS      0xa0
#define DEV_REG_MOD_CHS  0x00
#define DEV_REG_MOD_LBA  0x40
// master device
#define DEV_REG_DEV_MST  0x00
// slave device
#define DEV_REG_DEV_SLV  0x10


struct PartitionTableEntry {
    // 是否可引导
    uint8_t bootable;
    // 起始磁头号
    uint8_t start_head;
    // 起始扇区号
    uint8_t start_sec;
    // 起始柱面号
    uint8_t start_chs;
    // 分区类型
    uint8_t fs_type;
    // 结束磁头号
    uint8_t end_head;
    // 结束扇区号
    uint8_t end_sec;
    // 结束柱面号
    uint8_t end_chs;

    /* 更需要关注的是下面这两项 */
    // 本分区起始扇区的lba地址
    uint32_t start_lba;
    // 本分区的扇区数目
    uint32_t sec_cnt;
} __attr_packed;

typedef struct PartitionTableEntry part_tab_entry_t;


/* 引导扇区,mbr或ebr所在的扇区 */
struct BootSector {
    // 引导代码
    uint8_t boot_code[446];
    // 分区表中有4项,共64字节
    part_tab_entry_t partition_table[4];
    // 启动扇区的结束标志是0x55,0xaa
    uint16_t signature;
} __attr_packed;

typedef struct BootSector boot_sector_t;


// 按硬盘数计算的通道数
// uint32_t channel_cnt;
// 有两个ATA通道
static ata_channel_t channels[2];

// list of partitions
static list_t partition_list;

/**
 * @brief select the disk and sector(s) to read/write
 * @param hd target device
 * @param lba starting sector address
 * @param sec_cnt number of sectors (0 for 256)
 */
static void select_disk_sector(disk_t *hd, uint32_t lba, uint8_t sec_cnt);

/**
 * @brief send command to channel
 * @param chan target channel
 * @param cmd command
 */
static inline void cmd_out(ata_channel_t *chan, uint8_t cmd) {
    chan->expecting_intr = True;
    outportb(reg_cmd(chan), cmd);
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
static void write_sector(disk_t *hd, void *buf, uint8_t sec_cnt);

/**
 * @brief [0x12,0x34,0x56,0x78] -> [0x34, 0x12, 0x78, 0x56]
 */
static void swap_adjacent_bytes(const char *src, char *buf, uint32_t len) {
    uint32_t i;
    for (i = 0; i < len; i++) {
        buf[i + 1] = *src++;
        buf[i] = *src++;
    }
    buf[i] = '\0';
}

static bool_t busy_wait(disk_t *hd);

/**
 * @brief identify disk
 * @return -1 if no device; 1 if not ata device; 0 if success
 */
static int identify_disk(disk_t *hd) {
    char id_info[512];
    select_disk_sector(hd, 0, 0);
    cmd_out(hd->my_channel, ATA_CMD_IDENTIFY);
    thread_msleep(100);
    if (inportb(reg_status(hd->my_channel)) == 0) {
        // no such device
        return -1;
    }
    // kprintf(KPL_DEBUG, "status = %x\n", inportb(reg_status(hd->my_channel)));
    sem_down(&(hd->my_channel->disk_done));

    while (1) {
        uint8_t status = inportb(reg_status(hd->my_channel));
        if ((status & ATA_SR_ERR)) {
            // If Err, Device is not ATA.
            return 1;
        }
        if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) {
            // Everything is right.
            break;
        }
        thread_msleep(10);
    }
    read_sector(hd, id_info, 1);

    kprintf(KPL_NOTICE, "disk %s info:\n", hd->disk_name);
    char buf[64];
    uint32_t sn_start = 10 * 2, sn_len = 20;
    uint32_t md_start = 27 * 2, md_len = 40;
    swap_adjacent_bytes(id_info + sn_start, buf, sn_len);
    kprintf(KPL_NOTICE, "    SN: %s\n", buf);
    swap_adjacent_bytes(id_info + md_start, buf, md_len);
    kprintf(KPL_NOTICE, "    MODULE: %s\n", buf);
    uint32_t sec_start = 60 * 2;
    uint32_t sectors = *(uint32_t *)(id_info + sec_start);
    kprintf(KPL_NOTICE, "    SECTORS: %d\n", sectors);
    kprintf(KPL_NOTICE, "    CAPACITY: %dMiB\n", sectors * 512 / 1024 / 1024);
    return 0;
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


static void read_sector(disk_t *hd, void *buf, uint8_t sec_cnt) {
    uint32_t num_bytes = 512;
    if (sec_cnt == 0) {
        num_bytes *= 256;
    } else {
        num_bytes *= sec_cnt;
    }
    inportsw(reg_data(hd->my_channel), buf, num_bytes / 2);
}


static void write_sector(disk_t *hd, void *buf, uint8_t sec_cnt) {
    uint32_t num_bytes = 512;
    if (sec_cnt == 0) {
        num_bytes *= 256;
    } else {
        num_bytes *= sec_cnt;
    }
    outportsw(reg_data(hd->my_channel), buf, num_bytes / 2);
}


static bool_t busy_wait(disk_t *hd) {
    ata_channel_t *chan = hd->my_channel;
    int sleep_time = 30 * 1000;
    while ((sleep_time -= 10) >= 0) {
        uint8_t status = inportb(reg_status(chan));
        if ((!(status & ATA_SR_BSY)) && (status & ATA_SR_DRQ)) {
            return True;
        } else {
            thread_msleep(10);
        }
    }
    return False;
}


void ata_read(disk_t *hd, uint32_t lba, void *buf, uint32_t sec_cnt) {
    // we use ATA-1, which only supports 28-bit lba
    ASSERT(lba < 0x10000000);
    mutex_lock(&(hd->my_channel->chan_lock));

    // num of sectors done operated
    uint32_t sec_done = 0;
    while (sec_done < sec_cnt) {
        uint32_t num_sectors = MIN(sec_cnt - sec_done, 256);
        select_disk_sector(hd, lba + sec_done, num_sectors);
        cmd_out(hd->my_channel, ATA_CMD_READ_PIO);
        // use sem_down to wait for the completion of the disk
        sem_down(&(hd->my_channel->disk_done));
        if (!busy_wait(hd)) {
            char error[64];
            ksprintf(error, "%s read sector %d failed", hd->disk_name, lba);
            PANIC(error);
        }
        read_sector(hd, (void *)((uintptr_t)buf + sec_done * 512), num_sectors);
        sec_done += num_sectors;
    }
    mutex_unlock(&(hd->my_channel->chan_lock));
}


void ata_write(disk_t *hd, uint32_t lba, void *buf, uint32_t sec_cnt) {
    // we use ATA-1, which only supports 28-bit lba
    ASSERT(lba < 0x10000000);
    mutex_lock(&(hd->my_channel->chan_lock));

    // num of sectors done operated
    uint32_t sec_done = 0;
    while (sec_done < sec_cnt) {
        uint32_t num_sectors = MIN(sec_cnt - sec_done, 256);
        select_disk_sector(hd, lba + sec_done, num_sectors);
        cmd_out(hd->my_channel, ATA_CMD_WRITE_PIO);
        if (!busy_wait(hd)) {
            char error[64];
            ksprintf(error, "%s write sector %d failed", hd->disk_name, lba);
            PANIC(error);
        }
        write_sector(hd, (void *)((uintptr_t)buf + sec_done * 512), num_sectors);
        // use sem_down to wait for the completion of the device
        sem_down(&(hd->my_channel->disk_done));
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
     * 不必担心此中断是否对应的是这一次的expecting_intr,
     * 每次读写硬盘时会申请锁, 从而保证了同步一致性.
     */
    if (chan->expecting_intr) {
        chan->expecting_intr = False;
        sem_up(&(chan->disk_done));
        /**
         * to tell the hard drive controller that this interrupt is
         * handled so that further operations can be executed
         */
        inportb(reg_status(chan));
    }
}


void ata_init() {
    // uint8_t hd_cnt = *(uint8_t *)(__va(0x475));
    // if (hd_cnt == 0) {
    //     return;
    // }
    // kprintf(KPL_NOTICE, "num hd installed = %d\n", hd_cnt);
    list_init(&partition_list);
    // uint32_t num_identified_hd = 0;
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
        mutex_init(&(ch->chan_lock));
        sem_init(&(ch->disk_done), 0);
        register_handler(ch->irq_no, hd_handler);
        // each channel has at most 2 devices
        for (int j = 0; j < 2; j++) {
            disk_t *hd = &(ch->devices[j]);
            hd->dev_no = j;
            hd->my_channel = ch;
            ksprintf(hd->disk_name, "sd%c", 'a' + i * 2 + j);
            kprintf(KPL_NOTICE, "%s-%s: ", ch->chan_name, j ? "slave" : "master");
            if (identify_disk(hd) != 0) {
                kprintf(KPL_NOTICE, "no ata device\n");
                hd->existed = False;
                continue;
            }
            hd->existed = True;
        }
    }
}
