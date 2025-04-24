/*
  Copyright (C) 2025  leleneme
  This file is part of huffman, which is free software:
  you can redistribute it and/or modify   it under the terms of the
  GNU General Public License as published by the Free Software Foundation,
  either version 3 of the License, or (at your option) any later version.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef HF_HUFFMAN_H
#define HF_HUFFMAN_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef size_t usize;

#define countof(a) sizeof(a) / sizeof(*a)

struct buffer_u8 {
    u8* data;
    usize len;
};

struct buffer_usize {
    usize* data;
    usize len;
};

#define buffer_alloc_z(buf, count)                         \
    do {                                                   \
        (buf)->data = calloc(count, sizeof(*(buf)->data)); \
        (buf)->len = count;                                \
    } while (0);

#define buffer_alloc(buf, count)                            \
    do {                                                    \
        (buf)->data = malloc(count * sizeof(*(buf)->data)); \
        (buf)->len = count;                                 \
    } while (0);

#define buffer_free(buffer)        \
    do {                           \
        if ((buffer)->data) {      \
            free((buffer)->data);  \
            (buffer)->data = NULL; \
        }                          \
        (buffer)->len = 0;         \
    } while (0)

struct helement {
    u8 byte;
    usize frequency;
    struct helement *left, *right;
};

struct pqueue {
    struct helement** items;
    usize count, len;
};

struct hcode {
    // huffman codes have a max lenght of ceil(log2(n)) * 2, where n is
    // is the alphabet size, in this case n = 256 since we work on bytes
    u16 bits;
    u8 bit_len;
};

struct buffer_hcode {
    struct hcode* data;
    usize len;
};

// Builds a frequency map, where it's indicies are the characters/bytes and the
// value the frequencies
struct buffer_usize frequencies_build(struct buffer_u8*);

// Build a priority queue on the results of make_frequencies
struct pqueue pqueue_build(struct buffer_usize);

// Compare two pkey, 0 -> equal, 1 -> f < s, -1 -> f > s
//   where f = first, s = second
int pkey_cmp(const void*, const void*);

// Sorts a priority queue based on character frequency
void pqueue_sort(struct pqueue*);

// Heap allocates a new huffman tree node
struct helement* helement_make(u8, usize);

// Builds the tree from a priority queue
struct helement* htree_build(struct pqueue*);

// Creates a map that maps each byte to a hcode
void htree_encode(struct helement*, struct buffer_hcode*, u16, u8);

void tree_free(struct helement*);
void pqueue_free(struct pqueue*);

#endif
