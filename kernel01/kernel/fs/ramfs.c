#include "ramfs.h"
#include "../log.h"

enum {
    HNM_RAMFS_WORKSPACE_DRAFT_CAPACITY = 1024
};

struct hnm_ramfs_entry {
    struct hnm_fs_node node;
    const char *parent_path;
};

static const char hnm_ramfs_version[] =
    "HNMos Kernel 01\n"
    "Version: v0.10 filesystem foundation\n"
    "Storage: readonly kernel ramfs\n";

static const char hnm_ramfs_system_info[] =
    "Graphics: framebuffer launcher online\n"
    "Input: keyboard mouse event queue\n"
    "Memory: PMM and early heap online\n"
    "Filesystem: readonly system + writable HNLang workspace RAMFS\n";

static const char hnm_ramfs_readme[] =
    "Welcome to HNMos.\n"
    "This is the first built in readonly RAM filesystem.\n"
    "Use ls and cat from the graphical terminal.\n";

static char hnm_ramfs_workspace_main[HNM_RAMFS_WORKSPACE_DRAFT_CAPACITY] =
    "# AI Studio workspace draft\n"
    "print \"Ready for provider response\"\n";

static struct hnm_ramfs_entry hnm_ramfs_entries[] = {
    {
        {"/", "/", HNM_FS_NODE_DIRECTORY, 0, 0},
        0
    },
    {
        {"system", "/system", HNM_FS_NODE_DIRECTORY, 0, 0},
        "/"
    },
    {
        {"version", "/system/version", HNM_FS_NODE_FILE, sizeof(hnm_ramfs_version) - 1, hnm_ramfs_version},
        "/system"
    },
    {
        {"info", "/system/info", HNM_FS_NODE_FILE, sizeof(hnm_ramfs_system_info) - 1, hnm_ramfs_system_info},
        "/system"
    },
    {
        {"readme.txt", "/readme.txt", HNM_FS_NODE_FILE, sizeof(hnm_ramfs_readme) - 1, hnm_ramfs_readme},
        "/"
    },
    {
        {"workspace", "/workspace", HNM_FS_NODE_DIRECTORY, 0, 0},
        "/"
    },
    {
        {
            "main.hn",
            "/workspace/main.hn",
            HNM_FS_NODE_FILE,
            0,
            hnm_ramfs_workspace_main
        },
        "/workspace"
    }
};

static int hnm_ramfs_ready;

static u32 hnm_ramfs_entry_count(void)
{
    return (u32)(sizeof(hnm_ramfs_entries) / sizeof(hnm_ramfs_entries[0]));
}

static int hnm_ramfs_string_equal(const char *left, const char *right)
{
    u32 i = 0;

    if (left == 0 || right == 0) {
        return 0;
    }

    while (left[i] != '\0' && right[i] != '\0') {
        if (left[i] != right[i]) {
            return 0;
        }

        i++;
    }

    return left[i] == right[i];
}

void hnm_ramfs_init(void)
{
    u32 workspace_length = 0;

    while (hnm_ramfs_workspace_main[workspace_length] != '\0') {
        workspace_length++;
    }

    for (u32 i = 0; i < hnm_ramfs_entry_count(); i++) {
        if (hnm_ramfs_string_equal(
                hnm_ramfs_entries[i].node.path,
                "/workspace/main.hn")) {
            hnm_ramfs_entries[i].node.size = workspace_length;
        }
    }

    hnm_ramfs_ready = 1;
    hnm_log_write_line("ramfs: builtin readonly nodes ready.");
}

int hnm_ramfs_read(const char *path, const struct hnm_fs_node **node)
{
    const char *target_path = path;

    if (!hnm_ramfs_ready || node == 0) {
        return 0;
    }

    if (target_path == 0 || target_path[0] == '\0') {
        target_path = "/";
    }

    for (u32 i = 0; i < hnm_ramfs_entry_count(); i++) {
        if (hnm_ramfs_string_equal(hnm_ramfs_entries[i].node.path, target_path)) {
            *node = &hnm_ramfs_entries[i].node;
            return 1;
        }
    }

    return 0;
}

int hnm_ramfs_list(const char *path, struct hnm_fs_list *list)
{
    const struct hnm_fs_node *directory;
    const char *target_path = path;

    if (!hnm_ramfs_ready || list == 0) {
        return 0;
    }

    list->count = 0;
    list->truncated = 0;

    if (target_path == 0 || target_path[0] == '\0') {
        target_path = "/";
    }

    if (!hnm_ramfs_read(target_path, &directory) ||
        directory->type != HNM_FS_NODE_DIRECTORY) {
        return 0;
    }

    for (u32 i = 0; i < hnm_ramfs_entry_count(); i++) {
        if (hnm_ramfs_string_equal(hnm_ramfs_entries[i].parent_path, target_path)) {
            if (list->count >= HNM_FS_LIST_MAX) {
                list->truncated = 1;
                continue;
            }

            list->nodes[list->count] = &hnm_ramfs_entries[i].node;
            list->count++;
        }
    }

    return 1;
}

int hnm_ramfs_write_workspace_hnlang(const char *path, const char *data)
{
    u32 length = 0;

    if (!hnm_ramfs_ready || data == 0 ||
        !hnm_ramfs_string_equal(path, "/workspace/main.hn")) {
        return 0;
    }

    while (data[length] != '\0' &&
        length + 1 < sizeof(hnm_ramfs_workspace_main)) {
        hnm_ramfs_workspace_main[length] = data[length];
        length++;
    }

    hnm_ramfs_workspace_main[length] = '\0';

    for (u32 i = 0; i < hnm_ramfs_entry_count(); i++) {
        if (hnm_ramfs_string_equal(
                hnm_ramfs_entries[i].node.path,
                "/workspace/main.hn")) {
            hnm_ramfs_entries[i].node.size = length;
            hnm_ramfs_entries[i].node.data = hnm_ramfs_workspace_main;
            return 1;
        }
    }

    return 0;
}

u32 hnm_ramfs_node_count(void)
{
    if (!hnm_ramfs_ready) {
        return 0;
    }

    return hnm_ramfs_entry_count();
}
