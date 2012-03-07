ENABLE_QT()

# Header files.
SET(DS_HDRS
    ds_view.h
    ds_application.h
    vsftpd.h)


# Srouce files.
SET(DS_SRCS
    ${DS_HDRS}
    ds_application.cpp
    ds_view.cpp
    vsftpd.cpp
    main.cpp)

SET(RESOURCE_SRCS res.qrc)
QT4_ADD_RESOURCES(DS_RES ${RESOURCE_SRCS})

# Moc files.
SET(MOC_DS_HDRS ds_application.h ds_view.h vsftpd.h)
QT4_WRAP_CPP(MOC_DS_SRCS ${MOC_DS_HDRS})

# Executable file.
ADD_EXECUTABLE(ds_server ${DS_SRCS} ${MOC_DS_SRCS} ${DS_RES})
TARGET_LINK_LIBRARIES(ds_server
   onyx_data
   onyx_screen
   onyx_wireless
   onyx_sys
   onyx_ui
   private_conf
   ${QT_LIBRARIES}
   ${ADD_LIB}
)

# Install the ds_server to $BUILD/bin
INSTALL(TARGETS ds_server RUNTIME DESTINATION bin)

