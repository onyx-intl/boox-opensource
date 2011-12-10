ENABLE_QT()

SET(iconv_lib "")
IF (BUILD_FOR_ARM OR BUILD_FOR_MIPS)
    ADD_DEFINITIONS(-DHAVE_CONFIG_H)
    SET(iconv_lib iconv)
ENDIF (BUILD_FOR_ARM OR BUILD_FOR_MIPS)

ADD_SUBDIRECTORY(libdjvu)

FILE(GLOB DJVU_READER_HDRS *.h)
QT4_WRAP_CPP(MOC_SRCS ${DJVU_READER_HDRS})

FILE(GLOB DJVU_READER_SRCS *.cpp)
SET(DJVU_READER_SRCS ${DJVU_READER_HDRS} ${DJVU_READER_SRCS} ${MOC_SRCS})

ADD_EXECUTABLE(djvu_reader ${DJVU_READER_SRCS})
TARGET_LINK_LIBRARIES(djvu_reader
  dictionary
  djvulibre
  onyx_base
  onyx_cms
  onyx_data
  onyx_screen
  onyx_ui
  onyx_sys
  ${iconv_lib}
  ${QT_LIBRARIES})
