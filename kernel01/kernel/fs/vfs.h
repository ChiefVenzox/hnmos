#ifndef HNM_FS_VFS_H
#define HNM_FS_VFS_H

#include "../types.h"

enum hnm_fs_node_type {
    HNM_FS_NODE_DIRECTORY = 1,
    HNM_FS_NODE_FILE = 2
};

struct hnm_fs_node {
    const char *name;
    const char *path;
    enum hnm_fs_node_type type;
    u32 size;
    const char *data;
};

enum {
    HNM_FS_LIST_MAX = 16
};

struct hnm_fs_list {
    const struct hnm_fs_node *nodes[HNM_FS_LIST_MAX];
    u32 count;
    int truncated;
};

void hnm_fs_init(void);
int hnm_fs_is_ready(void);
int hnm_fs_read(const char *path, const struct hnm_fs_node **node);
int hnm_fs_list(const char *path, struct hnm_fs_list *list);
u32 hnm_fs_node_count(void);
const char *hnm_fs_node_type_name(enum hnm_fs_node_type type);

#endif
