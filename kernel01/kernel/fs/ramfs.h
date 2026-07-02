#ifndef HNM_FS_RAMFS_H
#define HNM_FS_RAMFS_H

#include "vfs.h"
#include "../types.h"

void hnm_ramfs_init(void);
int hnm_ramfs_read(const char *path, const struct hnm_fs_node **node);
int hnm_ramfs_list(const char *path, struct hnm_fs_list *list);
u32 hnm_ramfs_node_count(void);

#endif
