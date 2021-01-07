#include <fs/myfs/inode.h>
#include <device/ata.h>
#include <common/debug.h>
#include <fs/myfs/fs.h>
#include <fs/myfs/fs_types.h>
#include <list.h>
#include <string.h>
#include <mm/kvmm.h>


/**
 * @brief locate inode on disk; results will be stored in lba and offset
 */
static void inode_locate(
    const partition_t *part, uint32_t i_no,
    uint32_t *lba, uint32_t *offset
) {
    ASSERT(i_no < MAX_FILE_CNT_PER_PART);
    uint32_t num_inode_per_sector = SECTOR_SIZE / sizeof(inode_t);
    uint32_t lba_offset = i_no / num_inode_per_sector;
    ASSERT(lba_offset < part->sb->inode_table_sec_cnt);
    *lba = part->sb->inode_table_start_lba + lba_offset;
    *offset = lba_offset % num_inode_per_sector;
}

void inode_sync(partition_t *part, im_inode_t *im_inode, void *buffer) {
    uint32_t lba, offset;
    inode_locate(part, im_inode->inode.i_no, &lba, &offset);
    ata_read(part->my_disk, lba, buffer, 1);
    inode_t *disk_inode = (inode_t *)buffer + offset;
    memcpy(&(im_inode->inode), disk_inode, sizeof(inode_t));
    ata_write(part->my_disk, lba, buffer, 1);
}

im_inode_t *inode_open(partition_t *part, uint32_t i_no) {
    // first find in the open_inodes cache
    list_node_t *p;
    __list_for_each((&(part->open_inodes)), p) {
        im_inode_t *im_inode = __container_of(im_inode_t, i_tag, p);
        if (im_inode->inode.i_no == i_no) {
            (im_inode->i_open_times)++;
            return im_inode;
        }
    }

    // if not found, find on disk
    uint32_t lba, offset;
    inode_locate(part, i_no, &lba, &offset);

    void *buffer = kmalloc(SECTOR_SIZE);
    if (buffer == NULL) {
        return NULL;
    }
    ata_read(part->my_disk, lba, buffer, 1);
    im_inode_t *im_inode = kmalloc(sizeof(im_inode_t));
    if (im_inode == NULL) {
        kfree(buffer);
        return NULL;
    }
    memcpy((inode_t *)buffer + offset, &(im_inode->inode), sizeof(inode_t));
    kfree(buffer);
    im_inode->i_open_times = 1;
    im_inode->write_deny = False;
    // add the inode at the front of the cache list
    __list_push_front(&(part->open_inodes), im_inode, i_tag);
    im_inode->dirty = False;
    return im_inode;
}


void inode_close(im_inode_t *im_inode) {
    // TO figure out: the book turned off interrupt here
    (im_inode->i_open_times)--;
    if ((im_inode->i_open_times) == 0) {
        list_erase(&(im_inode->i_tag));
        kfree(im_inode);
    }
}

void inode_init(inode_t *inode) {

}
