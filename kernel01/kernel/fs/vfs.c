#include "vfs.h"
#include "ramfs.h"
#include "../log.h"

static int hnm_fs_ready;

void hnm_fs_init(void)
{
    hnm_ramfs_init();
    hnm_fs_ready = 1;

    hnm_log_write_line("fs: VFS ready.");
    hnm_log_write_line("fs: readonly ramfs mounted.");
}

int hnm_fs_is_ready(void)
{
    return hnm_fs_ready;
}

int hnm_fs_read(const char *path, const struct hnm_fs_node **node)
{
    if (!hnm_fs_ready) {
        return 0;
    }

    return hnm_ramfs_read(path, node);
}

int hnm_fs_list(const char *path, struct hnm_fs_list *list)
{
    if (!hnm_fs_ready) {
        return 0;
    }

    return hnm_ramfs_list(path, list);
}

u32 hnm_fs_node_count(void)
{
    if (!hnm_fs_ready) {
        return 0;
    }

    return hnm_ramfs_node_count();
}

const char *hnm_fs_node_type_name(enum hnm_fs_node_type type)
{
    if (type == HNM_FS_NODE_DIRECTORY) {
        return "dir";
    }

    if (type == HNM_FS_NODE_FILE) {
        return "file";
    }

    return "unknown";
}
