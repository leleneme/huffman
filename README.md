A simple (and slow) Huffman coding compressor and decompressor. This program is not intended for real world applications and was made as a learning exercise.

This program uses a custom file format for output. A simple overview and definition of this format can be found in [fformat.h](src/fformat.h).

### Building

With gcc or clang:
```
$ cc -O3 src/*.c -o huffman
```

### Usage

```
$ huffman <option> <input> <output>
```

The command-line has two options:
- `c`: Compresses `<input>` and writes the compressed output to `<output>`.
- `d`: Decompressing `<input>` and writes the original contents to `<output>`.

