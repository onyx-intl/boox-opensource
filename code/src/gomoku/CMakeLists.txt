ENABLE_QT()
# Header files.
FILE(GLOB HDRS *.h)
QT4_WRAP_CPP(MOC_SRCS ${HDRS})

# Source files.
FILE(GLOB SRCS *.cpp)
SET(SRCS ${HDRS} ${SRCS} ${MOC_SRCS})

# Resources.
SET(RESOURCE_SRCS images.qrc)
QT4_ADD_RESOURCES(IMAGES_RES ${RESOURCE_SRCS})

ADD_EXECUTABLE(gomoku.oar main.cpp ${SRCS} ${IMAGES_RES})
TARGET_LINK_LIBRARIES(gomoku.oar
  onyxapp
  onyx_data
  onyx_screen
  onyx_ui
  ${QT_LIBRARIES}
  ${ADD_LIB}
)
