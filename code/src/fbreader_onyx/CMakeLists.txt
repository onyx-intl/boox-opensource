ENABLE_QT()

# Dependencies
#INCLUDE(FindCURL)
#INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})
#LINK_DIRECTORIES(${CURL_LIBRARY_DIR})

# lib wv2
add_subdirectory(wv2-0.4.2)

add_subdirectory(bzip2)

# linebreak library
FILE(GLOB_RECURSE linebreak_headers linebreak/*.h)
FILE(GLOB_RECURSE linebreak_srcs linebreak/*.c)
add_library(linebreak  ${linebreak_headers} ${linebreak_srcs})
INCLUDE_DIRECTORIES(linebreak)

# fribidi library
INCLUDE_DIRECTORIES(fribidi)
INCLUDE_DIRECTORIES(fribidi/charset)
INCLUDE_DIRECTORIES(fribidi/lib)
INCLUDE_DIRECTORIES(.)

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


# INCLUDE(FindFriBiDi)
# INCLUDE_DIRECTORIES(${FRIBIDI_INCLUDE_DIR})
# LINK_DIRECTORIES(${FRIBIDI_LIBRARY_DIR})

#include(FindBZip2)
#INCLUDE_DIRECTORIES(${bzip2_INCLUDE_DIR})
#LINK_DIRECTORIES(${bzip2_LIBRARY_DIR})

# include directories.
# zlibrary core
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

IF(UNIX)
  INCLUDE_DIRECTORIES(zlibrary/core/src/unix)
ELSE(UNIX)
  INCLUDE_DIRECTORIES(zlibrary/core/src/win32)
ENDIF(UNIX)

INCLUDE_DIRECTORIES(zlibrary/core/src/util)
INCLUDE_DIRECTORIES(zlibrary/core/src/xml)
INCLUDE_DIRECTORIES(zlibrary/core/src/xml/expat)

# zlibrary text
INCLUDE_DIRECTORIES(zlibrary/text/src/hyphenation)
INCLUDE_DIRECTORIES(zlibrary/text/src/model)
INCLUDE_DIRECTORIES(zlibrary/text/src/view)

# zlibrary ui
INCLUDE_DIRECTORIES(zlibrary/ui/src/qt4)

# fbreader
INCLUDE_DIRECTORIES(src/bookmodel)
INCLUDE_DIRECTORIES(src/collection)
INCLUDE_DIRECTORIES(src/constants)
INCLUDE_DIRECTORIES(src/description)
INCLUDE_DIRECTORIES(src/encodingOption)
INCLUDE_DIRECTORIES(src/external)
INCLUDE_DIRECTORIES(src/fbreader)
INCLUDE_DIRECTORIES(src/formats)
INCLUDE_DIRECTORIES(src/migration)
INCLUDE_DIRECTORIES(src/options)
INCLUDE_DIRECTORIES(src/optionsDialog)
INCLUDE_DIRECTORIES(src/formats/util)

# Source files.

IF(UNIX)
  FILE(GLOB_RECURSE ZLIBRARY_ui_sys_HEADERS zlibrary/ui/src/unix/*.h)
  FILE(GLOB_RECURSE ZLIBRARY_ui_sys_SRCS zlibrary/ui/src/unix/*.cpp)
ENDIF(UNIX)


FILE(GLOB_RECURSE fbreader_headers src/*.h)
FILE(GLOB_RECURSE fbreader_srcs src/*.cpp)

ADD_DEFINITIONS(-DVERSION="0.10")
ADD_DEFINITIONS(-DINSTALLDIR="install")

IF(UNIX)
  ADD_DEFINITIONS(-DBASEDIR="/usr/share/onyx_reader")
ELSE(UNIX)
  ADD_DEFINITIONS(-DBASEDIR="d://share")
ENDIF(UNIX)
ADD_DEFINITIONS(-DLIBDIR="libdir")
ADD_DEFINITIONS(-DIMAGEDIR="image")
ADD_DEFINITIONS(-DAPPIMAGEDIR="app")
ADD_DEFINITIONS(-DUNICODE)

ADD_library(fb_reader
  ${fbreader_headers} ${fbreader_srcs}
  )

QT4_WRAP_CPP(moc_onyx_reader_app_srcs zlibrary/ui/src/qt4/library/ZLibrary.h)

add_executable(onyx_reader src/onyx_reader_main.cpp ${moc_onyx_reader_app_srcs})
target_link_libraries(onyx_reader fb_reader zlibrary expat bz2 linebreak
  onyx_data
  onyx_cms
  onyx_ui
  epd_utils
  dictionary
  tts
  sound
  onyx_screen
  onyx_sys
  wv2
  ${ADD_LIB}
  ${QT_LIBRARIES})
link_iconv(onyx_reader)

IF(NOT WIN32)
    target_link_libraries(onyx_reader fribidi)
ENDIF(NOT WIN32)


add_subdirectory(zlibrary)
