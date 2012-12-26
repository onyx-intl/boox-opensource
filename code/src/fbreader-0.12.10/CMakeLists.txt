ENABLE_QT()

# Dependencies
#INCLUDE(FindCURL)
#INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})
#LINK_DIRECTORIES(${CURL_LIBRARY_DIR})

add_subdirectory(bzip2)
add_subdirectory(zlibrary)
add_subdirectory(sqlite3)

# linebreak library
FILE(GLOB_RECURSE linebreak_headers linebreak/*.h)
FILE(GLOB_RECURSE linebreak_srcs linebreak/*.c)
add_library(linebreak  ${linebreak_headers} ${linebreak_srcs})
INCLUDE_DIRECTORIES(linebreak)


# INCLUDE(FindFriBiDi)
# INCLUDE_DIRECTORIES(${FRIBIDI_INCLUDE_DIR})
# LINK_DIRECTORIES(${FRIBIDI_LIBRARY_DIR})


# fribidi library
INCLUDE_DIRECTORIES(fribidi)
INCLUDE_DIRECTORIES(fribidi/charset)
INCLUDE_DIRECTORIES(fribidi/lib)

IF(NOT WIN32)
    ADD_DEFINITIONS(-DHAVE_CONFIG_H)
    FILE(GLOB_RECURSE fribidi_charset_headers fribidi/charset/*.h)
    FILE(GLOB_RECURSE fribidi_charset_srcs fribidi/charset/*.c)
    FILE(GLOB_RECURSE fribidi_headers fribidi/lib/*.h)
    FILE(GLOB_RECURSE fribidi_srcs fribidi/lib/*.c)
    add_library(fribidi
      ${fribidi_charset_headers}
      ${fribidi_charset_srcs}
      ${fribidi_headers}
      ${fribidi_srcs})
ENDIF(NOT WIN32)

# include directories.
# zlibrary core
INCLUDE_DIRECTORIES(zlibrary/core/include)
INCLUDE_DIRECTORIES(zlibrary/text/include)
INCLUDE_DIRECTORIES(zlibrary/core/src/xml)
INCLUDE_DIRECTORIES(zlibrary/core/src)
INCLUDE_DIRECTORIES(zlibrary/core/src/application)
INCLUDE_DIRECTORIES(zlibrary/core/src/desktop)
INCLUDE_DIRECTORIES(zlibrary/core/src/dialogs)
INCLUDE_DIRECTORIES(zlibrary/core/src/encoding)
INCLUDE_DIRECTORIES(zlibrary/core/src/filesystem)
INCLUDE_DIRECTORIES(zlibrary/core/src/filesystem/zip)
INCLUDE_DIRECTORIES(zlibrary/core/src/filesystem/bzip2)
INCLUDE_DIRECTORIES(zlibrary/core/src/filesystem/tar)
INCLUDE_DIRECTORIES(zlibrary/core/src/image)
INCLUDE_DIRECTORIES(zlibrary/core/src/language)
INCLUDE_DIRECTORIES(zlibrary/core/src/library)
INCLUDE_DIRECTORIES(zlibrary/core/src/message)
#INCLUDE_DIRECTORIES(zlibrary/core/src/network)
INCLUDE_DIRECTORIES(zlibrary/core/src/options)
INCLUDE_DIRECTORIES(zlibrary/core/src/resources)
INCLUDE_DIRECTORIES(zlibrary/core/src/runnable)
INCLUDE_DIRECTORIES(zlibrary/core/src/splittedView)
INCLUDE_DIRECTORIES(zlibrary/core/src/time)
INCLUDE_DIRECTORIES(zlibrary/core/src/view)
INCLUDE_DIRECTORIES(zlibrary/core/src/unix)
#IF(UNIX)
#  INCLUDE_DIRECTORIES(zlibrary/core/src/unix)
#ELSE(UNIX)
#  INCLUDE_DIRECTORIES(zlibrary/core/src/win32)
#ENDIF(UNIX)

INCLUDE_DIRECTORIES(zlibrary/core/src/util)
INCLUDE_DIRECTORIES(zlibrary/core/src/xml)
INCLUDE_DIRECTORIES(zlibrary/core/src/xml/expat)

# zlibrary text
INCLUDE_DIRECTORIES(zlibrary/text/src/hyphenation)
INCLUDE_DIRECTORIES(zlibrary/text/src/model)
INCLUDE_DIRECTORIES(zlibrary/text/src/view)

# zlibrary ui
INCLUDE_DIRECTORIES(zlibrary/ui/src/qt4)

# Source files.
INCLUDE_DIRECTORIES(sqlite3)
INCLUDE_DIRECTORIES(src/database)
INCLUDE_DIRECTORIES(src/database/sqldb)
INCLUDE_DIRECTORIES(src/database/sqldb/implsqlite)
INCLUDE_DIRECTORIES(src/database/booksdb)
INCLUDE_DIRECTORIES(src/database/booksdb/runnables)
INCLUDE_DIRECTORIES(src/migration)
INCLUDE_DIRECTORIES(src/constants)
INCLUDE_DIRECTORIES(src/options)
INCLUDE_DIRECTORIES(src/library)
INCLUDE_DIRECTORIES(src/bookmodel)
INCLUDE_DIRECTORIES(src/formats)
INCLUDE_DIRECTORIES(src/formats/fb2)
INCLUDE_DIRECTORIES(src/formats/css)
INCLUDE_DIRECTORIES(src/formats/html)
INCLUDE_DIRECTORIES(src/formats/pdb)
INCLUDE_DIRECTORIES(src/formats/txt)
INCLUDE_DIRECTORIES(src/formats/tcr)
INCLUDE_DIRECTORIES(src/formats/chm)
INCLUDE_DIRECTORIES(src/formats/xhtml)
INCLUDE_DIRECTORIES(src/formats/oeb)
INCLUDE_DIRECTORIES(src/formats/rtf)
INCLUDE_DIRECTORIES(src/formats/openreader)
INCLUDE_DIRECTORIES(src/formats/util)
INCLUDE_DIRECTORIES(src/external)
INCLUDE_DIRECTORIES(src/optionsDialog)
INCLUDE_DIRECTORIES(src/fbreader)
INCLUDE_DIRECTORIES(src/encodingOption)
INCLUDE_DIRECTORIES(src/network)
INCLUDE_DIRECTORIES(src/network/authentication)
INCLUDE_DIRECTORIES(src/network/authentication/basic)
INCLUDE_DIRECTORIES(src/network/atom)
INCLUDE_DIRECTORIES(src/network/opds)
INCLUDE_DIRECTORIES(src/network/authentication/litres)
INCLUDE_DIRECTORIES(src/blockTree)
INCLUDE_DIRECTORIES(src/libraryActions )
INCLUDE_DIRECTORIES(src/libraryTree )
INCLUDE_DIRECTORIES(src/networkActions)
INCLUDE_DIRECTORIES(src/networkTree)

#IF(UNIX)
  FILE(GLOB_RECURSE ZLIBRARY_ui_sys_HEADERS zlibrary/ui/src/unix/*.h)
  FILE(GLOB_RECURSE ZLIBRARY_ui_sys_SRCS zlibrary/ui/src/unix/*.cpp)
#ENDIF(UNIX)


FILE(GLOB_RECURSE fbreader_headers src/*.h)
FILE(GLOB_RECURSE fbreader_srcs src/*.cpp)
#MESSAGE("fbreader_headers PAT="${fbreader_headers})
#MESSAGE("fbreader_srcs PAT="${fbreader_srcs})

ADD_DEFINITIONS(-DVERSION="0.12")
ADD_DEFINITIONS(-DINSTALLDIR="install")

ADD_DEFINITIONS(-DBASEDIR="/usr/share/onyx_reader")
ADD_DEFINITIONS(-DLIBDIR="libdir")
ADD_DEFINITIONS(-DIMAGEDIR="image")
ADD_DEFINITIONS(-DAPPIMAGEDIR="app")
ADD_DEFINITIONS(-DUNICODE)


INCLUDE_DIRECTORIES(bzip2)
ADD_library(new_fbreader_lib
  ${fbreader_headers} ${fbreader_srcs}
  )

QT4_WRAP_CPP(moc_new_fb_reader_app_srcs zlibrary/core/src/library/ZLibrary.h)

add_executable(new_fbreader  ${moc_new_fb_reader_app_srcs})
target_link_libraries(new_fbreader new_fbreader_lib  expat bz2 linebreak fribidi zlibrary sqlite3
  ${ADD_LIB}
  ${QT_LIBRARIES})
link_iconv(new_fbreader)


