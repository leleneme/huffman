/*
  Copyright (C) 2025  leleneme
  This file is part of huffman, which is free software:
  you can redistribute it and/or modify   it under the terms of the
  GNU General Public License as published by the Free Software Foundation,
  either version 3 of the License, or (at your option) any later version.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// for htoleXX and leXXtoh
#define _DEFAULT_SOURCE

#include "fformat.h"
#include <stdbool.h>
#include <assert.h>
#include <endian.h>

#define write_macro(type, f)                                                             \
    static inline unsigned long io_write_##type##_le(struct io_stream* io, type value) { \
        type le = f(value);                                                              \
        return io_write(io, &le, sizeof(type));                                          \
    }

// usize io_read(struct io_stream* io, void* buffer, usize size) {
#define read_macro(type, f)                                        \
    static inline type io_read_##type##_le(struct io_stream* io) { \
        type le;                                                   \
        io_read(io, &le, sizeof(type));                            \
        return f(le);                                              \
    }

write_macro(u64, htole64)
write_macro(u32, htole32)
write_macro(u16, htole16)
write_macro(u8, )

read_macro(u64, le64toh)
read_macro(u32, le32toh)
read_macro(u16, le16toh)
read_macro(u8, )

#undef read_macro
#undef write_macro

#define HCODE_ENTRY_SIZE (sizeof(u8) + sizeof(u16) + sizeof(u8))

void fformat_compress(struct io_stream* io, struct buffer_hcode code_map, struct buffer_u8* input) {
    usize code_count = 0;
    u8 max_code_len = 0;

    for (usize i = 0; i < code_map.len; i++) {
        struct hcode code = code_map.data[i];
        if (code.bit_len > 0) {
            code_count++;

            if (code.bit_len > max_code_len)
                max_code_len = code.bit_len;
        }
    }

    io_write(io, (void*)FILE_MAGIC, countof(FILE_MAGIC));
    io_write_u64_le(io, (u64)input->len);

    u32 offset_to_content = countof(FILE_MAGIC) + sizeof(u64) + sizeof(u32) + (code_count * HCODE_ENTRY_SIZE);
    io_write_u32_le(io, offset_to_content);

    for (usize i = 0; i < code_map.len; i++) {
        struct hcode code = code_map.data[i];
        if (code.bit_len == 0)
            continue;

        io_write_u8_le(io, (u8)i);
        io_write_u16_le(io, code.bits);
        io_write_u8_le(io, code.bit_len);
    }

    usize compressed_worst_len = ((input->len * max_code_len) / 8) + 1;
    struct buffer_u8 compressed;
    buffer_alloc_z(&compressed, compressed_worst_len);

    assert(compressed.data);

    struct bitstream bs = {
        .data = compressed.data,
        .capacity = compressed.len,
        .byte_pos = 0,
        .bit_pos = 0
    };

    for (usize i = 0; i < input->len; i++) {
        u8 symbol = input->data[i];
        struct hcode code = code_map.data[symbol];
        if (code.bit_len == 0) {
            abort();
        }

        bitstream_write_bits(&bs, code.bits, code.bit_len);
    }

    io_write(io, bs.data, bs.byte_pos + (bs.bit_pos > 0 ? 1 : 0));
    buffer_free(&compressed);
}

struct buffer_u8 fformat_decompress(struct io_stream* io) {
    struct buffer_u8 decompressed = { 0 };

    // Read and compare file signature
    u8 magic[countof(FILE_MAGIC)];
    io_read(io, &magic, countof(FILE_MAGIC) * sizeof(u8));

    if (memcmp(magic, FILE_MAGIC, countof(FILE_MAGIC)) != 0) {
        fprintf(stderr, "- file magic does not match\n");
        return decompressed;
    }

    u64 original_file_size = io_read_u64_le(io);
    u32 offset_to_content = io_read_u32_le(io);

    // Count how many entries there are
    usize entries_count = 0;
    {
        long current_position = io_tell(io);
        entries_count = (offset_to_content - current_position) / HCODE_ENTRY_SIZE;
    }

    struct buffer_hcode code_map;
    buffer_alloc_z(&code_map, UINT8_MAX);
    assert(code_map.data);

    u8 max_code_len = 0;
    for (usize i = 0; i < entries_count; i++) {
        u8 symbol = io_read_u8_le(io);
        struct hcode new_code = {
            .bits = io_read_u16_le(io),
            .bit_len = io_read_u8_le(io)
        };

        if (new_code.bit_len > max_code_len) {
            max_code_len = new_code.bit_len;
        }

        code_map.data[symbol] = new_code;
    }

    usize compressed_size = 0;
    {
        io_seek(io, 0, SEEK_END);
        unsigned long end = io_tell(io);
        compressed_size = (end - offset_to_content);
    }

    struct buffer_u8 compressed_data;
    buffer_alloc(&compressed_data, compressed_size);
    assert(compressed_data.data);

    io_seek(io, offset_to_content, SEEK_SET);
    io_read(io, compressed_data.data, compressed_size * sizeof(u8));

    buffer_alloc(&decompressed, original_file_size);
    assert(decompressed.data);

    u16 current_bits = 0;
    u8 current_len = 0;
    usize output_pos = 0;

    struct bitstream bs = {
        .data = compressed_data.data,
        .capacity = compressed_data.len,
        .byte_pos = 0,
        .bit_pos = 0
    };

    while (output_pos < decompressed.len) {
        u8 bit;
        if (!bitstream_read_bit(&bs, &bit)) {
            fprintf(stderr, "Unexpected end of compressed data\n");
            abort();
        }

        current_bits = (current_bits << 1) | bit;
        current_len++;

        for (usize sym = 0; sym < code_map.len; sym++) {
            struct hcode code = code_map.data[sym];
            if (code.bit_len == 0)
                continue;

            if (code.bit_len == current_len) {
                u16 mask = (1 << current_len) - 1;
                u16 aligned_input = current_bits & mask;
                if (aligned_input == code.bits) {
                    decompressed.data[output_pos++] = (u8)sym;
                    current_bits = 0;
                    current_len = 0;
                    break;
                }
            }
        }

        if (current_len > 16) {
            fprintf(stderr, "caught infinite loop\n");
            abort();
        }
    }

    buffer_free(&code_map);
    buffer_free(&compressed_data);
    return decompressed;
}

void bitstream_write_bits(struct bitstream* bs, u16 bits, u8 bit_len) {
    while (bit_len--) {
        u8 bit = (bits >> bit_len) & 1;
        bs->data[bs->byte_pos] |= (bit << (7 - bs->bit_pos));
        bs->bit_pos++;

        if (bs->bit_pos == 8) {
            bs->bit_pos = 0;
            bs->byte_pos++;
            bs->data[bs->byte_pos] = 0;
        }
    }
}

bool bitstream_read_bit(struct bitstream* bs, u8* out_bit) {
    if (bs->byte_pos >= bs->capacity)
        return false;

    u8 byte = bs->data[bs->byte_pos];
    *out_bit = (byte >> (7 - bs->bit_pos)) & 1;

    bs->bit_pos++;
    if (bs->bit_pos == 8) {
        bs->bit_pos = 0;
        bs->byte_pos++;
    }

    return true;
}
