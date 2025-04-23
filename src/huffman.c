/*
  Copyright (C) 2025  leleneme
  This file is part of huffman, which is free software:
  you can redistribute it and/or modify   it under the terms of the
  GNU General Public License as published by the Free Software Foundation,
  either version 3 of the License, or (at your option) any later version.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "huffman.h"

struct buffer_usize frequencies_build(struct buffer_u8* input) {
    struct buffer_usize buf = { 0 };

    const usize freqs_size = UINT8_MAX;
    usize* freqs = calloc(freqs_size, sizeof(usize));

    if (freqs == NULL) {
        fprintf(stderr, "failed to allocate frequency map\n");
        return buf;
    }

    for (usize i = 0; i < input->len; i++) {
        u8 c = input->data[i];
        freqs[c]++;
    }

    buf.data = freqs;
    buf.len = freqs_size;

    return buf;
}

int pkey_cmp(const void* a, const void* b) {
    struct helement* ka = *(struct helement**)a;
    struct helement* kb = *(struct helement**)b;

    if (ka->frequency > kb->frequency)
        return -1;
    else if (ka->frequency < kb->frequency)
        return 1;
    else
        return 0;
}

void pqueue_sort(struct pqueue* queue) {
    qsort(queue->items, queue->len, sizeof(struct helement*), pkey_cmp);
}

struct helement* helement_make(u8 byte, usize frequency) {
    struct helement* new = calloc(1, sizeof(struct helement));
    if (!new) {
        fprintf(stderr, "failed to allocate tree node\n");
        return NULL;
    }

    new->byte = byte;
    new->frequency = frequency;
    return new;
}

struct pqueue pqueue_build(struct buffer_usize frequencies) {
    usize count = UINT8_MAX;
    struct pqueue q = {
        .items = calloc(count, sizeof(struct helement*)),
        .count = count,
        .len = 0,
    };

    if (!q.items) {
        return q;
    }

    for (usize i = 0; i < frequencies.len; i++) {
        usize frequency = frequencies.data[i];
        u8 c = (u8)i;

        if (frequency == 0)
            continue;

        q.items[q.len] = helement_make(c, frequency);
        q.len++;
    }

    pqueue_sort(&q);
    return q;
}

struct helement* htree_build(struct pqueue* queue) {
    while (queue->len > 1) {
        struct helement* left = queue->items[queue->len - 1];
        struct helement* right = queue->items[queue->len - 2];
        queue->len -= 2;

        struct helement* merged = helement_make(0, left->frequency + right->frequency);
        merged->left = left;
        merged->right = right;

        queue->items[queue->len] = merged;
        queue->len++;

        pqueue_sort(queue);
    }

    return queue->items[0];
}

void htree_encode(struct helement* node, struct buffer_hcode* codes, u16 bits, u8 bit_len) {
    if (!node)
        return;

    if (!node->left && !node->right) {
        codes->data[node->byte].bits = bits;
        codes->data[node->byte].bit_len = bit_len;
        return;
    } else {
        // Add a '0' to every left node and add a '1' to every right node
        // we traverse
        htree_encode(node->left, codes, (bits << 1), bit_len + 1);
        htree_encode(node->right, codes, (bits << 1) | 1, bit_len + 1);
    }
}

void tree_free(struct helement* element) {
    if (!element)
        return;
    tree_free(element->left);
    tree_free(element->right);
    free(element);
}

void pqueue_free(struct pqueue* queue) {
    if (!queue)
        return;
    free(queue->items);
    queue->count = queue->len = 0;
}
