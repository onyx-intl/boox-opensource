ENABLE_QT()

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/boox-opensource/code/src/podofo)
LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/boox-opensource/code/src/podofo/libs/imx31)
INCLUDE_DIRECTORIES(include)

# Header files.
FILE(GLOB MOC_HDRS include/*.h)
QT4_WRAP_CPP(MOC_SRCS ${MOC_HDRS})

# Source files.
FILE(GLOB SRCS src/*.cpp)
SET(SRCS
  ${MOC_HDRS}
  ${SRCS}
  ${MOC_SRCS})

# Resources.
#SET(RESOURCE_SRCS images.qrc)
#QT4_ADD_RESOURCES(IMAGES_RES ${RESOURCE_SRCS})

# pdf_tools 
ADD_EXECUTABLE(pdf_tools ${SRCS} )
TARGET_LINK_LIBRARIES(pdf_tools
  onyx_data
  onyx_screen
  podofo
  freetype
  fontconfig
  jpeg
  expat
  ${QT_LIBRARIES}
  ${ADD_LIB}
)

# Install the gui_shell to $BUILD/bin
INSTALL(TARGETS  pdf_tools RUNTIME DESTINATION bin)



