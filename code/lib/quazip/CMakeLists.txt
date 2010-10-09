ENABLE_QT()

# Header files.
FILE(GLOB hrds *.h)

SET(moc_hdrs
    quazipfile.h)
QT4_WRAP_CPP(moc_srcs ${moc_hdrs})

# Source files.
FILE(GLOB srcs *.cpp *.c)
SET(srcs
    ${srcs}
    ${hrds})


ADD_LIBRARY(quazip ${srcs} ${moc_srcs})

