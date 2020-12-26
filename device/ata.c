#include <device/ata.h>
#include <common/types.h>
#include <arch/x86.h>
#include <myos.h>

#define reg_data(channel)        (channel->port_base + 0)
#define reg_error(channel)       (channel->port_base + 1)
#define reg_sect_cnt(channel)    (channel->port_base + 2)
#define reg_lba_l(channel)       (channel->port_base + 3)
#define reg_lba_m(channel)       (channel->port_base + 4)
#define reg_lba_h(channel)       (channel->port_base + 5)
#define reg_dev(channel)         (channel->port_base + 6)
#define reg_status(channel)      (channel->port_base + 7)
#define reg_cmd(channel)         reg_status(channel)
#define reg_alt_status(channel)  (channel->port_base + 0x206)
#define reg_ctl(channel)         reg_alt_status(channel)

/* reg_status寄存器的一些关键位 */
#define BIT_STAT_BSY     0x80          // 硬盘忙
#define BIT_STAT_DRDY    0x40          // 驱动器准备好
#define BIT_STAT_DRQ     0x08          // 数据传输准备好了

/* device寄存器的一些关键位 */
#define BIT_DEV_MBS      0xa0          // 第7位和第5位固定为1
#define BIT_DEV_LBA      0x40
#define BIT_DEV_DEV      0x10

/* 一些硬盘操作的指令 */
#define CMD_IDENTIFY       0xec        // identify指令
#define CMD_READ_SECTOR    0x20        // 读扇区指令
#define CMD_WRITE_SECTOR   0x30        // 写扇区指令

// 按硬盘数计算的通道数
uint8_t channel_cnt;
// 有两个ide通道
static ata_channel_t channels[2];


void ata_init() {
    uint8_t hd_cnt = *(uint8_t *)(0x475);
}