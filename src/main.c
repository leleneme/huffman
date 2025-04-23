/*
  Copyright (C) 2025, leleneme on github.com
  This file is part of huffman, which is free software:
  you can redistribute it and/or modify   it under the terms of the
  GNU General Public License as published by the Free Software Foundation,
  either version 3 of the License, or (at your option) any later version.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "huffman.h"
#include "fformat.h"
#include "io.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

static struct buffer_u8 read_entire_file(const char* path);
static void usage(const char* program, FILE* file);

int main(int argc, char** argv) {
    if (argc != 4) {
        usage(argv[0], stderr);
        return EXIT_FAILURE;
    }

    const char* method = argv[1];
    const char* target = argv[2];
    const char* out_path = argv[3];

    if (strcmp(method, "c") == 0) {
        struct buffer_u8 contents = read_entire_file(target);
        if (contents.data == NULL) {
            return EXIT_FAILURE;
        }

        printf("- compressing '%s' of size %zu bytes\n", target, contents.len);
        struct buffer_usize freqs = frequencies_build(&contents);
        struct pqueue queue = pqueue_build(freqs);
        struct helement* root = htree_build(&queue);

        struct buffer_hcode code_map;
        buffer_alloc_z(&code_map, UINT8_MAX);

        htree_encode(root, &code_map, 0, 0);

        struct io_stream io = io_fopen(out_path, "wb");
        if (!io.valid) {
            fprintf(stderr, "invalid file stream for '%s'\n", out_path);
            goto cleanup_c;
        }

        fformat_compress(&io, code_map, &contents);
        {
            io_seek(&io, 0, SEEK_END);
            long end = io_tell(&io);
            double ratio = (double)contents.len / end;
            printf("- written to '%s' with '%ld' bytes (ratio of x%.2f)\n", out_path, end, ratio);
        }

    cleanup_c:
        io_close(&io);
        buffer_free(&code_map);
        tree_free(root);
        pqueue_free(&queue);
        buffer_free(&freqs);
        buffer_free(&contents);
    } else if (strcmp(method, "d") == 0) {
        struct io_stream io, os = { 0 };

        io = io_fopen(target, "rb");
        if (!io.valid) {
            fprintf(stderr, "invalid file stream for '%s'\n", target);
            goto cleanup_d;
        }

        struct buffer_u8 data = fformat_decompress(&io);
        if (!data.data) {
            fprintf(stderr, "failed to decompress file\n");
            goto cleanup_d;
        }

        os = io_fopen(out_path, "wb");
        if (!os.valid) {
            fprintf(stderr, "invalid file stream for '%s'\n", out_path);
            goto cleanup_d;
        }

        io_write(&os, data.data, data.len * sizeof(u8));

    cleanup_d:
        io_close(&io);
        io_close(&os);
        buffer_free(&data);
    } else {
        fprintf(stderr, "invalid option '%s'\n", method);
        usage(argv[0], stderr);
        return EXIT_FAILURE;
    }

    return 0;
}

static void usage(const char* program, FILE* file) {
    fprintf(file, "usage: %s <c|d> <input> <output>\n", program);
}

static struct buffer_u8 read_entire_file(const char* path) {
    struct buffer_u8 contents = { 0 };

    FILE* fp = fopen(path, "rb");
    if (fp == NULL) {
        fprintf(stderr, "failed to open '%s': %s\n", path, strerror(errno));
        return contents;
    }

    if (fseek(fp, 0, SEEK_END) == ESPIPE) {
        fprintf(stderr, "file '%s' is not seekable\n", path);
        fclose(fp);
        return contents;
    }

    long file_size = ftell(fp);
    if (file_size == -1) {
        fprintf(stderr, "error while querying file size: %s\n", strerror(errno));
        fclose(fp);
        return contents;
    }

    buffer_alloc(&contents, file_size);
    if (contents.data == NULL) {
        fprintf(stderr, "failed to allocate memory for file: %s\n", strerror(errno));
        fclose(fp);
        return contents;
    }

    rewind(fp);
    fread(contents.data, sizeof(u8), file_size, fp);
    fclose(fp);

    return contents;
}
