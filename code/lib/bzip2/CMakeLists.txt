# Source files.
SET(SRCS
  blocksort.c
  huffman.c
  crctable.c
  randtable.c
  compress.c
  decompress.c
  bzlib.c)


ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64)

# bzip2 library.
ADD_LIBRARY(bz2 ${SRCS})

