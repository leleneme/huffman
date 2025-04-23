/*
  Copyright (C) 2025  leleneme
  This file is part of huffman, which is free software:
  you can redistribute it and/or modify   it under the terms of the
  GNU General Public License as published by the Free Software Foundation,
  either version 3 of the License, or (at your option) any later version.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "io.h"
#include <errno.h>

usize io_write(struct io_stream* io, void* buffer, usize size) {
    return io->write(io->context, buffer, size);
}

usize io_read(struct io_stream* io, void* buffer, usize size) {
    return io->read(io->context, buffer, size);
}

long io_tell(struct io_stream* io) {
    return io->tell(io->context);
}

long io_seek(struct io_stream* io, long offset, int origin) {
    return io->seek(io->context, offset, origin);
}

void io_close(struct io_stream* io) {
    if (io->valid)
        io->close(io->context);
}

struct io_filestream {
    FILE* file;
};

static usize fstream_read(void* context, void* buffer, usize size) {
    struct io_filestream* fs = context;
    return fread(buffer, sizeof(u8), size, fs->file);
}

static usize fstream_write(void* context, const void* buffer, usize size) {
    struct io_filestream* fs = context;
    return fwrite(buffer, sizeof(u8), size, fs->file);
}

static int fstream_seek(void* context, long offset, int origin) {
    struct io_filestream* fs = context;
    return fseek(fs->file, offset, origin);
}

static long fstream_tell(void* context) {
    struct io_filestream* fs = context;
    return ftell(fs->file);
}

static void fstream_close(void* context) {
    struct io_filestream* fs = context;
    if (fs->file)
        fclose(fs->file);
    free(fs);
}

struct io_stream io_fopen(const char* path, const char* modes) {
    struct io_stream io = { 0 };
    struct io_filestream* fs = malloc(sizeof(struct io_filestream));
    if (!fs) {
        fprintf(stderr, "failed to allocate file io descriptor: %s\n", strerror(errno));
        return io;
    }

    fs->file = fopen(path, modes);
    if (!fs->file) {
        fprintf(stderr, "failed to open file '%s': %s\n", path, strerror(errno));
        free(fs);
        return io;
    }

    if (fseek(fs->file, 0, SEEK_CUR) != 0) {
        fprintf(stderr, "file '%s' is not seekable\n", path);
        fclose(fs->file);
        free(fs);
        return io;
    }

    io.context = fs;
    io.read = fstream_read;
    io.read = fstream_read;
    io.write = fstream_write;
    io.seek = fstream_seek;
    io.tell = fstream_tell;
    io.close = fstream_close;
    io.valid = true;

    return io;
}
