//WRITTEN USING OOPENAI CHATGPT
//TESTED

#include <archive.h>
#include <archive_entry.h>
#include <iostream>
#include <string>
#include <spdlog/spdlog.h>

static long copy_data(struct archive *ar, struct archive *aw);

int decompress(const char* filename, const char* destination) {
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int flags;
    long r;

    flags = ARCHIVE_EXTRACT_TIME |
            ARCHIVE_EXTRACT_PERM |
            ARCHIVE_EXTRACT_ACL |
            ARCHIVE_EXTRACT_FFLAGS;

    a = archive_read_new();
    archive_read_support_format_tar(a);
    archive_read_support_filter_gzip(a);

    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, flags);
    archive_write_disk_set_standard_lookup(ext);

    if ((r = archive_read_open_filename(a, filename, 10240))) {
        spdlog::error("Decompressor: archive_read_open_filename() failed");
        return 1;
    }

    while (true) {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF)
            break;
        if (r < ARCHIVE_OK)
            spdlog::error("Decompressor:{}",archive_error_string(a));
        if (r < ARCHIVE_WARN)
            return 1;

        std::string fullPath = std::string(destination) + "/" + archive_entry_pathname(entry);
        archive_entry_set_pathname(entry, fullPath.c_str());

        r = archive_write_header(ext, entry);
        if (r < ARCHIVE_OK)
            spdlog::error("Decompressor: {}",archive_error_string(ext));
        else if (archive_entry_size(entry) > 0) {
            r = copy_data(a, ext);
            if (r < ARCHIVE_OK)
                spdlog::error("Decompressor: {}",archive_error_string(ext));
        }
        r = archive_write_finish_entry(ext);
        if (r < ARCHIVE_OK)
            spdlog::error("Decompressor: {}",archive_error_string(ext));
    }

    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);

    return 0;
}

static long copy_data(struct archive *ar, struct archive *aw) {
    const void *buff;
    size_t size;
    la_int64_t offset;
    long r;

    while (true) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return ARCHIVE_OK;
        if (r < ARCHIVE_OK)
            return r;

        r = archive_write_data_block(aw, buff, size, offset);
        if (r < ARCHIVE_OK) {
            spdlog::error("Decompressor: copy_data() failed:{}",archive_error_string(aw));
            return r;
        }
    }
}