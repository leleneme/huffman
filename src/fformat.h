/*
  Copyright (C) 2025  leleneme
  This file is part of huffman, which is free software:
  you can redistribute it and/or modify   it under the terms of the
  GNU General Public License as published by the Free Software Foundation,
  either version 3 of the License, or (at your option) any later version.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/*
  LBCA - lelenemes's bad compression archive file format:

  > All offset/size/length fields are defined in bytes (8-bits)
  > All multi-byte values are stored as little-endian byte order

  The format has three sections:
  - Header: Metadata
  - Code Descriptors: Maps each byte to its corresponding Huffman's code
  - Content: The compressed data

  * File Header *
  +--------+-------+---------------------------------------------------------------------+
  | Offset | Bytes | Description                                                         |
  +--------+-------+---------------------------------------------------------------------+
  | 0      | 4     | File signature = { 0x0, 0x6c, 0x62, 0x63, 0x61, 0x0 }  ->  \0lbca   |
  +--------+-------+---------------------------------------------------------------------+
  | 4      | 8     | Original file size                                                  |
  +--------+-------+---------------------------------------------------------------------+
  | 12     | 4     | The offset (in bytes) where the compressed data starts, relative to |
  |        |       | the beginning of the file                                           |
  +--------+-------+---------------------------------------------------------------------+

  * Code Entries *

  +--------+-------+----------------------------------+
  | Offset | Bytes | Description                      |
  +--------+-------+----------------------------------+
  | 0      | 1     | The symbol this code represents  |
  +--------+-------+----------------------------------+
  | 1      | 2     | The code assigned to this symbol |
  +--------+-------+----------------------------------+
  | 3      | 1     | The significant bits of the code |
  +--------+-------+----------------------------------+

  * Content (Compressed Data) *
  The compressed data section starts at the **Offset to Content** as specified in the header.
  The content is stored as a bitstream, where each byte represents a portion of the encoded data.

  +---------------------+-------+---------------------------------------------------------------------+
  | Offset              | Bytes | Description                                                         |
  +---------------------+-------+---------------------------------------------------------------------+
  | (Offset to content) | N     | The compressed data, stored in a bitstream. Where N is the total    |
  |                     |       | archive size minus the offset to content as defined in the header   |
  +---------------------+-------+---------------------------------------------------------------------+
*/

#ifndef LBCA_FFORMAT_H_
#define LBCA_FFORMAT_H_

#include "io.h"
#include "huffman.h"
#include <stdbool.h>

static u8 FILE_MAGIC[6] = { 0x0, 0x6c, 0x62, 0x63, 0x61, 0x0 }; // \0lbca

void fformat_compress(struct io_stream* io, struct buffer_hcode code_map, struct buffer_u8* input);
struct buffer_u8 fformat_decompress(struct io_stream* io);

struct bitstream {
    u8* data;
    usize capacity;
    usize byte_pos;
    u8 bit_pos;
};

void bitstream_write_bits(struct bitstream* bs, u16 bits, u8 bit_len);
bool bitstream_read_bit(struct bitstream* bs, u8* out_bit);

#endif
