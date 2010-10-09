
ENABLE_QT()

IF(UNIX)
    ADD_DEFINITIONS(-DPLATFORM_POSIX)
ELSE (UNIX)
    ADD_DEFINITIONS(-DPLATFORM_WINDOWS)
ENDIF (UNIX)

# aisound
include_directories(aisound)
link_directories(${CMAKE_SOURCE_DIR}/lib/tts/aisound)

FILE(GLOB AISOUND_HDRS aisound/*.h)
FILE(GLOB AISOUND_SRCS aisound/*.cpp)


# ejtts
include_directories(ej)
link_directories(${CMAKE_SOURCE_DIR}/lib/tts/ej/lib)

FILE(GLOB EJ_HDRS ej/*.h)
FILE(GLOB EJ_SRCS ej/*.cpp)
SET(EJ_FILE_SRCS ej/file_func/Win_Linux_file_func.c)


# espeak
include_directories(espeak)
FILE(GLOB ESPEAK_HDRS espeak/src/*.h)
SET(ESPEAK_SRCS
    espeak/src/speak_lib.cpp
    espeak/src/compiledict.cpp
    espeak/src/dictionary.cpp
    espeak/src/intonation.cpp
    espeak/src/readclause.cpp
    espeak/src/setlengths.cpp
    espeak/src/numbers.cpp
    espeak/src/synth_mbrola.cpp
    espeak/src/synthdata.cpp
    espeak/src/synthesize.cpp
    espeak/src/translate.cpp
    espeak/src/tr_languages.cpp
    espeak/src/voices.cpp
    espeak/src/wavegen.cpp
    espeak/src/phonemelist.cpp
    espeak/src/espeak_command.cpp
    espeak/src/event.cpp
    espeak/src/fifo.cpp
    espeak/src/debug.cpp
    espeak/src/klatt.cpp
)

#SET(SRCS ${ESPEAK_SRCS})
#SET(SRCS ${AISOUND_SRCS})
#SET(SRCS ${EJ_SRCS})


# source files.
SET(HDRS tts.h tts_widget.h tts_interface.h )
SET(SRCS ${EJ_SRCS} ${EJ_HDRS} ${HDRS} tts.cpp tts_widget.cpp )

#SET(HDRS tts.h tts_widget.h tts_interface.h aisound/tts_aisound.h)
#SET(SRCS ${AISOUND_SRCS} ${AISOUND_HDRS} ${HDRS} tts.cpp tts_widget.cpp) 
SET(MOC_HDRS ${HDRS})
QT4_WRAP_CPP(MOC_SRCS ${MOC_HDRS})

ADD_LIBRARY(ej_file ${EJ_FILE_SRCS})

# resource
SET(resource_srcs tts_images.qrc)
QT4_ADD_RESOURCES(tts_ui_res ${resource_srcs})

ADD_LIBRARY(tts ${SRCS} ${MOC_SRCS} ${tts_ui_res})
if(BUILD_FOR_ARM)
TARGET_LINK_LIBRARIES(tts ejTTS ej_file sound)
else(BUILD_FOR_ARM)
TARGET_LINK_LIBRARIES(tts ej_file sound)
endif(BUILD_FOR_ARM)


# unittest
FILE(GLOB UNITTEST_SRCS unittest/unittest.cpp)
ADD_EXECUTABLE(tts_unittest ${UNITTEST_SRCS})
if(BUILD_FOR_ARM)
TARGET_LINK_LIBRARIES(tts_unittest tts 
   ${QT_LIBRARIES}
   ${ADD_LIB}
)
else(BUILD_FOR_ARM)
TARGET_LINK_LIBRARIES(tts_unittest tts 
   ${QT_LIBRARIES}
   ${ADD_LIB}
)
endif(BUILD_FOR_ARM)
SET_TARGET_PROPERTIES(tts_unittest PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_PATH})
