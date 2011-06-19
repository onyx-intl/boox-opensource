ENABLE_QT()
#SET(CMAKE_CXX_FLAGS "-g ")
# Header files.
FILE(GLOB HDRS *.h)
QT4_WRAP_CPP(MOC_SRCS ${HDRS})

# Source files.
FILE(GLOB SRCS *.cpp)
SET(SRCS ${HDRS} ${SRCS} ${MOC_SRCS})
# Resources.
SET(READER_RES_SRC icons/icon.qrc symmetry/symmetry.qrc)
QT4_ADD_RESOURCES(sudoku_RES ${READER_RES_SRC})

ADD_EXECUTABLE(sudoku sudokuactions.cpp ${SRCS} ${sudoku_RES})
TARGET_LINK_LIBRARIES(sudoku
  onyxapp
  onyx_data
  onyx_screen
  onyx_ui
  ${QT_LIBRARIES}
  ${ADD_LIB}
)
