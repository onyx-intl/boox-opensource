ENABLE_QT()


# Header files.
FILE(GLOB HDRS *.h)
QT4_WRAP_CPP(MOC_SRCS ${HDRS})

# Source files.
FILE(GLOB SRCS *.cpp)
SET(SRCS ${HDRS} ${SRCS} ${MOC_SRCS})

# Resources.
SET(RESOURCE_SRCS rss_images.qrc)
QT4_ADD_RESOURCES(IMAGES_RES ${RESOURCE_SRCS})

# EXE
ADD_EXECUTABLE(rss_reader ${SRCS} ${IMAGES_RES})
TARGET_LINK_LIBRARIES(rss_reader onyx_cms onyx_ui onyx_wireless onyx_sys onyx_screen 
  ${QT_LIBRARIES}
  ${ADD_LIB}
)

