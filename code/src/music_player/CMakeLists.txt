ENABLE_QT()

ADD_DEFINITIONS(-DFPM_DEFAULT)
ADD_DEFINITIONS(-DHAVE_CONFIG_H)
ADD_DEFINITIONS(-DASO_ZEROCHECK)
ADD_DEFINITIONS(-D_USE_MATH_DEFINES)

INCLUDE_DIRECTORIES(./)
INCLUDE_DIRECTORIES(./libmad)
INCLUDE_DIRECTORIES(./libtag)
INCLUDE_DIRECTORIES(./libbs2b)
INCLUDE_DIRECTORIES(./libsr)
INCLUDE_DIRECTORIES(./libtag/taglib)
INCLUDE_DIRECTORIES(./libcurl)

# Header files.
FILE(GLOB MOC_HDRS core/*.h
                   core/equ/*.h
                   input/*.h
                   output/*.h
                   model/*.h
                   effect/*.h
                   utils/*.h
                   libmad/*.h
                   libbs2b/*.h
                   libsr/*.h
                   libtag/taglib/*.h
                   player_utils.h
                   onyx_player_view.h
                   player_application.h
                   player_title_bar.h)
QT4_WRAP_CPP(MOC_SRCS ${MOC_HDRS})

# Source files.
FILE(GLOB SRCS core/*.cpp
               core/equ/*.c
               input/*.cpp
               output/*.cpp
               model/*.cpp
               effect/*.cpp
               utils/*.cpp
               libtag/taglib/*.cpp
               libmad/*.c
               libbs2b/*.c
               libsr/*.c
               onyx_player_view.cpp
               player_application.cpp
               player_title_bar.cpp)
SET(SRCS
  ${MOC_HDRS}
  ${SRCS}
  ${MOC_SRCS})

QT4_ADD_RESOURCES(MUSIC_PLAYER_RES music_player.qrc)

IF (BUILD_WITH_TFT)
    set(ALSALIB asound)
ELSE (BUILD_WITH_TFT)
    set(ANYKALIB )
ENDIF (BUILD_WITH_TFT)

ADD_LIBRARY(music_player_lib ${SRCS} ${MUSIC_PLAYER_RES})
TARGET_LINK_LIBRARIES(music_player_lib
   onyx_screen
   sound
   onyx_data
   onyx_ui
   onyx_sys
   onyx_cms
   curl
   ${ANYKALIB}
   ${QT_LIBRARIES}
   ${ADD_LIB}
)

add_executable(music_player main.cpp ${MUSIC_PLAYER_RES})
target_link_libraries(music_player music_player_lib)
maybe_link_tcmalloc(music_player)

# Install the explorer to $BUILD/bin
INSTALL(TARGETS music_player RUNTIME DESTINATION bin)
