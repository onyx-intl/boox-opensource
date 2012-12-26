# Source files.
SET(SRCS
  sqlite3.c  sqlite3ext.h  sqlite3.h)


ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64)

# sqlite3 library.
ADD_LIBRARY(sqlite3  ${SRCS})

