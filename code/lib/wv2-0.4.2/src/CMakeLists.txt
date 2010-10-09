INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${wvWare_BINARY_DIR} )

# I'd say the 'if' is unneeded because we always CONFIGURE_FILE
IF( EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config.h )
  ADD_DEFINITIONS( -DHAVE_CONFIG_H )
ENDIF( EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config.h )

SET( wv2_SOURCES 
    olestorage.cpp 
    olestream.cpp 
    word97_generated.cpp 
    word97_helper.cpp 
    word95_helper.cpp 
    word_helper.cpp 
    word95_generated.cpp 
    convert.cpp 
    global.cpp 
    parser.cpp 
    parser9x.cpp 
    parser95.cpp 
    parser97.cpp 
    parserfactory.cpp 
    ustring.cpp 
    textconverter.cpp 
    styles.cpp 
    handlers.cpp 
    properties97.cpp 
    fonts.cpp 
    lists.cpp 
    paragraphproperties.cpp 
    headers.cpp 
    headers95.cpp 
    headers97.cpp 
    footnotes97.cpp 
    functor.cpp 
    functordata.cpp 
    associatedstrings.cpp 
    fields.cpp 
    utilities.cpp 
    crc32.c 
    wvlog.cpp 
    graphics.cpp 
    wv2version.cpp 
    zcodec.cxx
    pole.cpp
    )

SET( wv2_HEADERS 
    olestorage.h 
    olestream.h 
    word95_generated.h 
    word97_generated.h 
    word_helper.h 
    parser.h 
    parserfactory.h 
    ustring.h 
    textconverter.h 
    styles.h 
    sharedptr.h 
    convert.h 
    global.h 
    handlers.h 
    fonts.h 
    lists.h 
    utilities.h 
    paragraphproperties.h 
    functor.h 
    functordata.h 
    associatedstrings.h 
    fields.h 
    dllmagic.h 
    wvlog.h 
    wv2version.h
    zcodec.hxx
    pole.h
    )

ADD_LIBRARY( wv2 STATIC ${wv2_SOURCES} ${wv2_HEADERS} )
TARGET_LINK_LIBRARIES( wv2 ${ICONV_LIBRARIES} ${ZLIB_LIBRARIES} )
