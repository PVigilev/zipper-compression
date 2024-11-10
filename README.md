# Zipper Compression library

## Functionality

1. Zipper comression library implements deflate algorithm for compression and decompression
2. Allows to use different modificators for building a huffman tree and for dictionary coder algorithm like max-length of window-frame and predicates for frequencies of bytes in case of Huffman tree
3. Provide an interface for expansion to implement other compression algorithms
   
Compression and decompression are intendent to be done on a sequence of bytes which is provided by a pointer.

