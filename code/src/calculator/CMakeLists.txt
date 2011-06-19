ENABLE_QT()
# Header files.
FILE(GLOB HDRS *.h)
QT4_WRAP_CPP(MOC_SRCS ${HDRS})

# Source files.
FILE(GLOB SRCS *.cpp)
SET(SRCS ${HDRS} ${SRCS} ${MOC_SRCS})

ADD_EXECUTABLE(calculator main.cpp ${SRCS})
TARGET_LINK_LIBRARIES(calculator
  onyxapp
  onyx_data
  onyx_screen
  onyx_ui
  ${QT_LIBRARIES}
  ${ADD_LIB}
)
