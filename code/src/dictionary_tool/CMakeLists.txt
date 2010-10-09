ENABLE_QT()

# Header files.
FILE(GLOB HDRS *.h)
QT4_WRAP_CPP(MOC_SRCS ${HDRS})

# Source files.
FILE(GLOB SRCS *.cpp)
SET(SRCS ${HDRS} ${SRCS} ${MOC_SRCS})

ADD_EXECUTABLE(dict_tool ${SRCS})
TARGET_LINK_LIBRARIES(dict_tool
  onyxapp
  dictionary
  tts
  sound
  onyx_data
  onyx_screen
  onyx_ui
  private_conf
  ${QT_LIBRARIES}
  ${ADD_LIB}
)
