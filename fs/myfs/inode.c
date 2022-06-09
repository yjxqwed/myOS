
#include <device/ata.h>
#include <fs/myfs/inode.h>
#include <fs/myfs/fs.h>
#include <fs/myfs/fs_types.h>
#include <fs/myfs/utils.h>

#include <lib/list.h>
#include <lib/string.h>
#include <lib/kprintf.h>

#include <mm/kvmm.h>
#include <common/debug.h>

/**
 * @brief locate inode on disk; results will be stored in lba and offset
 */

/**
 * @brief locate inode on disk; results will be stored in lba and offset
 */
static void inode_locate(
    const partition_t *part, uint32_t i_no, uint32_t *blk_id, uint32_t *offset
) {
    ASSERT(i_no < MAX_FILE_CNT_PER_PART);
    uint32_t num_inode_per_block = BLOCK_SIZE / sizeof(inode_t);
    uint32_t blk_offset = i_no / num_inode_per_block;
    ASSERT(blk_offset < __myfs_field(part, sb)->inode_table_blk_cnt);
    *blk_id = __myfs_field(part, sb)->inode_table_start_blk_id + blk_offset;
    *offset = i_no % num_inode_per_block;
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
    ASSERT(part != NULL && i_no < __myfs_field(part, sb)->inode_cnt);
    // first find in the open_inodes cache
    list_node_t *p;
    list_t *part_open_inodes = &__myfs_field(part, open_inodes);
    __list_for_each(part_open_inodes, p) {
        im_inode_t *im_inode = __container_of(im_inode_t, i_tag, p);
        if (im_inode->inode.i_no == i_no) {
            (im_inode->i_open_times)++;
            return im_inode;
        }
    }

    // if not found, find on disk
    uint32_t blk_id, offset;
    inode_locate(part, i_no, &blk_id, &offset);
    kprintf(KPL_DEBUG, "inode-to-open: i_no = %d, blk_id = %d, offset = %d ... ", i_no, blk_id, offset);

    void *buffer = kmalloc(BLOCK_SIZE);
    im_inode_t *im_inode = kmalloc(sizeof(im_inode_t));

    if (buffer == NULL || im_inode == NULL) {
        kfree(buffer);
        kfree(im_inode);
        return NULL;
    }

    // ata_read(part->my_disk, lba, buffer, 1);
    partition_block_read(part, blk_id, buffer, 1);
    memcpy(((inode_t *)buffer + offset), &(im_inode->inode), sizeof(inode_t));
    kprintf(KPL_DEBUG, " (%d)done!\n", im_inode->inode.i_no);

    kfree(buffer);
    im_inode->i_open_times = 1;
    im_inode->write_deny = False;
    // add the inode to the front of the cache list
    __list_push_front(part_open_inodes, im_inode, i_tag);
    im_inode->dirty = False;
    return im_inode;
}


void inode_close(im_inode_t *im_inode) {
    ASSERT(im_inode != NULL);
    // TO figure out: the book turned off interrupt here
    (im_inode->i_open_times)--;
    if ((im_inode->i_open_times) == 0) {
        list_erase(&(im_inode->i_tag));
        kfree(im_inode);
    }
}

void im_inode_init(im_inode_t *im_inode, int i_no) {
    im_inode->dirty = False;
    im_inode->i_open_times = 1;
    im_inode->write_deny = False;
    im_inode->i_tag.next = NULL;
    im_inode->i_tag.prev = NULL;

    im_inode->inode.i_no = i_no;
    im_inode->inode.i_size = 0;
    for (int i = 0; i < NR_BLOCKS_PER_INODE; i++) {
        im_inode->inode.i_blocks[i] = 0;
    }
}
