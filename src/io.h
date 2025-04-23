/*
  Copyright (C) 2025  leleneme
  This file is part of huffman, which is free software:
  you can redistribute it and/or modify   it under the terms of the
  GNU General Public License as published by the Free Software Foundation,
  either version 3 of the License, or (at your option) any later version.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef HF_IO_H
#define HF_IO_H

#include "huffman.h"
#include <stdbool.h>

struct io_stream {
    void* context;
    bool valid;

    usize (*read)(void* context, void* buffer, usize size);
    usize (*write)(void* context, const void* buffer, usize size);
    int (*seek)(void* context, long offset, int origin);
    long (*tell)(void* context);
    void (*close)(void* content);
};

// TODO: io_stream for memory buffers
struct io_stream io_fopen(const char* path, const char* modes);

usize io_write(struct io_stream* io, void* buffer, usize size);
usize io_read(struct io_stream* io, void* buffer, usize size);
long io_tell(struct io_stream* io);
long io_seek(struct io_stream* io, long offset, int origin);
void io_close(struct io_stream* io);

#endif
