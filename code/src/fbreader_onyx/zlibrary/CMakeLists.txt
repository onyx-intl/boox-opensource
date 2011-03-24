# include directories.
# zlibrary core
INCLUDE_DIRECTORIES(core/src)
INCLUDE_DIRECTORIES(core/src/application)
INCLUDE_DIRECTORIES(core/src/desktop)
INCLUDE_DIRECTORIES(core/src/dialogs)
INCLUDE_DIRECTORIES(core/src/encoding)
INCLUDE_DIRECTORIES(core/src/filesystem)
INCLUDE_DIRECTORIES(core/src/filesystem/zip)
INCLUDE_DIRECTORIES(core/src/filesystem/bzip2)
INCLUDE_DIRECTORIES(core/src/filesystem/tar)
INCLUDE_DIRECTORIES(core/src/image)
INCLUDE_DIRECTORIES(core/src/language)
INCLUDE_DIRECTORIES(core/src/library)
INCLUDE_DIRECTORIES(core/src/message)
INCLUDE_DIRECTORIES(core/src/network)
INCLUDE_DIRECTORIES(core/src/options)
INCLUDE_DIRECTORIES(core/src/resources)
INCLUDE_DIRECTORIES(core/src/runnable)
INCLUDE_DIRECTORIES(core/src/splittedView)
INCLUDE_DIRECTORIES(core/src/time)
INCLUDE_DIRECTORIES(core/src/view)

file(GLOB zlibrary_core_library_hdrs core/src/library/*.h)
file(GLOB zlibrary_core_library_srcs core/src/library/*.cpp)

file(GLOB zlibrary_core_util_hdrs core/src/util/*.h)
file(GLOB zlibrary_core_util_srcs core/src/util/*.cpp)

file(GLOB_RECURSE zlibrary_core_fs_hdrs core/src/filesystem/*.h)
file(GLOB_RECURSE zlibrary_core_fs_srcs core/src/filesystem/*.cpp)

file(GLOB_RECURSE zlibrary_core_dialogs_hdrs core/src/dialogs/*.h)
file(GLOB_RECURSE zlibrary_core_dialogs_srcs core/src/dialogs/*.cpp)

file(GLOB_RECURSE zlibrary_core_optionEntries_hdrs core/src/optionEntries/*.h)
file(GLOB_RECURSE zlibrary_core_optionEntries_srcs core/src/optionEntries/*.cpp)

file(GLOB_RECURSE zlibrary_core_application_hdrs core/src/application/*.h)
file(GLOB_RECURSE zlibrary_core_application_srcs core/src/application/*.cpp)

file(GLOB_RECURSE zlibrary_core_view_hdrs core/src/view/*.h)
file(GLOB_RECURSE zlibrary_core_view_srcs core/src/view/*.cpp)

file(GLOB_RECURSE zlibrary_core_splittedView_hdrs core/src/splittedView/*.h)
file(GLOB_RECURSE zlibrary_core_splittedView_srcs core/src/splittedView/*.cpp)

file(GLOB_RECURSE zlibrary_core_encoding_hdrs core/src/encoding/*.h)
file(GLOB_RECURSE zlibrary_core_encoding_srcs core/src/encoding/*.cpp)

file(GLOB_RECURSE zlibrary_core_options_hdrs core/src/options/*.h)
file(GLOB_RECURSE zlibrary_core_options_srcs core/src/options/*.cpp)

file(GLOB_RECURSE zlibrary_core_message_hdrs core/src/message/*.h)
file(GLOB_RECURSE zlibrary_core_message_srcs core/src/message/*.cpp)

file(GLOB_RECURSE zlibrary_core_resources_hdrs core/src/resources/*.h)
file(GLOB_RECURSE zlibrary_core_resources_srcs core/src/resources/*.cpp)

file(GLOB_RECURSE zlibrary_core_time_hdrs core/src/time/*.h)
file(GLOB_RECURSE zlibrary_core_time_srcs core/src/time/*.cpp)

file(GLOB_RECURSE zlibrary_core_xml_hdrs core/src/xml/*.h)
file(GLOB_RECURSE zlibrary_core_xml_srcs core/src/xml/*.cpp)

file(GLOB_RECURSE zlibrary_core_image_hdrs core/src/image/*.h)
file(GLOB_RECURSE zlibrary_core_image_srcs core/src/image/*.cpp)

file(GLOB_RECURSE zlibrary_core_language_hdrs core/src/language/*.h)
file(GLOB_RECURSE zlibrary_core_language_srcs core/src/language/*.cpp)

file(GLOB_RECURSE zlibrary_core_network_hdrs core/src/network/*.h)
file(GLOB_RECURSE zlibrary_core_network_srcs core/src/network/*.cpp)

file(GLOB_RECURSE zlibrary_core_desktop_app_hdrs core/src/desktop/application/*.h)
file(GLOB_RECURSE zlibrary_core_desktop_app_srcs core/src/desktop/application/*.cpp)

file(GLOB_RECURSE zlibrary_core_desktop_dlg_hdrs core/src/desktop/dialogs/*.h)
file(GLOB_RECURSE zlibrary_core_desktop_dlg_srcs core/src/desktop/dialogs/*.cpp)

file(GLOB_RECURSE zlibrary_core_unix_hdrs core/src/unix/*.h)
file(GLOB_RECURSE zlibrary_core_unix_srcs core/src/unix/*.cpp)

file(GLOB_RECURSE zlibrary_ui_qt4_hdrs ui/src/qt4/*.h)
file(GLOB_RECURSE zlibrary_ui_qt4_srcs ui/src/qt4/*.cpp)
FILE(GLOB_RECURSE zlibrary_ui_headers ui/src/qt4/*.h)
QT4_WRAP_CPP(moc_zlibrary_ui_srcs ${zlibrary_ui_headers})

file(GLOB_RECURSE zlibrary_ui_sys_hdrs ui/src/unix/message/*.h)
file(GLOB_RECURSE zlibrary_ui_sys_srcs ui/src/unix/message/*.cpp)

FILE(GLOB_RECURSE zlibrary_text_headers text/*.h)
FILE(GLOB_RECURSE zlibrary_text_srcs text/*.cpp)

set(zlibrary_core_hdrs
    ${zlibrary_core_library_hdrs}
    ${zlibrary_core_util_hdrs}
    ${zlibrary_core_fs_hdrs}
    ${zlibrary_core_dialogs_hdrs}
    ${zlibrary_core_optionEntries_hdrs}
    ${zlibrary_core_application_hdrs}
    ${zlibrary_core_view_hdrs}
    ${zlibrary_core_splittedView_hdrs}
    ${zlibrary_core_encoding_hdrs}
    ${zlibrary_core_options_hdrs}
    ${zlibrary_core_message_hdrs}
    ${zlibrary_core_resources_hdrs}
    ${zlibrary_core_time_hdrs}
    ${zlibrary_core_xml_hdrs}
    ${zlibrary_core_image_hdrs}
    ${zlibrary_core_language_hdrs}
    ${zlibrary_core_network_hdrs}
    ${zlibrary_core_desktop_app_hdrs}
    ${zlibrary_core_desktop_dlg_hdrs}
    ${zlibrary_core_unix_hdrs}
    ${zlibrary_core_unix_srcs}
)

set(zlibrary_core_srcs
    ${zlibrary_core_library_srcs}
    ${zlibrary_core_util_srcs}
    ${zlibrary_core_fs_srcs}
    ${zlibrary_core_dialogs_srcs}
    ${zlibrary_core_optionEntries_srcs}
    ${zlibrary_core_application_srcs}
    ${zlibrary_core_view_srcs}
    ${zlibrary_core_splittedView_srcs}
    ${zlibrary_core_encoding_srcs}
    ${zlibrary_core_options_srcs}
    ${zlibrary_core_message_srcs}
    ${zlibrary_core_resources_srcs}
    ${zlibrary_core_time_srcs}
    ${zlibrary_core_xml_srcs}
    ${zlibrary_core_image_srcs}
    ${zlibrary_core_language_srcs}
    ${zlibrary_core_unix_time_srcs}
    ${zlibrary_core_network_srcs}
    ${zlibrary_core_desktop_app_srcs}
    ${zlibrary_core_desktop_dlg_srcs}
)

add_library(zlibrary
  ${zlibrary_core_hdrs} ${zlibrary_core_srcs}
  ${zlibrary_ui_qt4_hdrs} ${zlibrary_ui_qt4_srcs}
  ${zlibrary_ui_sys_hdrs} ${zlibrary_ui_sys_srcs}
  ${zlibrary_text_headers} ${zlibrary_text_srcs}
  ${zlibrary_ui_sys_headers} ${ZLIBRARY_ui_sys_srcs}
  ${moc_zlibrary_ui_srcs})
target_link_libraries(zlibrary ${QT_LIBRARIES} )
