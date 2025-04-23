A simple (and slow) Huffman coding compressor and decompressor. This program is not intended for real world applications and was made as a learning exercise.

This program uses a custom file format for output. A simple overview and definition of this format can be found in [fformat.h](src/fformat.h).

#### Building

With gcc or clang:
```
$ cc -O3 src/*.c -o huffman
```

#### Usage

```
$ huffman <option> <input> <output>
```

The command-line has two options:
- `c`: Compresses `<input>` and writes the compressed output to `<output>`.
- `d`: Decompressing `<input>` and writes the original contents to `<output>`.

#### Results

When compressing the King James English bible (4.3M) the compression ratio is 1.73 (2.5M), compared to Zip's 3 (1.4M). This is expected, as Zip is much more advanced than just a naive huffman coding.

#### TODO

Features/changes that would be nice:
- Add memory-backed io_stream
- Clean up main and stop using read_entire_file instead of using io_stream
- Better error handling
- Better support for using this as a library

#### License

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation. You should have received a copy of the GNU General Public License along with this program. If not, see https://www.gnu.org/licenses/.