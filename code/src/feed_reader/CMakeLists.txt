ENABLE_QT()
include_directories(BEFORE .)

qt4_wrap_cpp(MOC_SRCS
  add_feed_dialog.h
  article_list_model.h
  article_list_page.h
  article_page.h
  feed_fetcher.h
  feed_list_model.h
  feeds_page.h
  main_widget.h
  widget_updater.h
)

add_library(feed_reader_lib feed_action.cpp
  add_feed_dialog.cpp
  article.cpp
  article_list_model.cpp
  article_list_page.cpp
  article_page.cpp
  database.cpp
  feed.cpp
  feed_fetcher.cpp
  feed_list_model.cpp
  feeds_page.cpp
  init.cpp
  main_widget.cpp
  rss_feed_parser.cpp
  widget_updater.cpp
  util.cpp
  ${MOC_SRCS}
)

set(ONYX_LIBS onyx_sys onyx_ui onyx_wireless onyx_wpa onyx_screen onyx_data onyx_cms)

target_link_libraries(feed_reader_lib  ${QT_LIBRARIES} ${ONYX_LIBS} ${feedreader})
strict_warning(feed_reader_lib)

add_executable(feed_reader feed_reader.cpp ${feedreader})
target_link_libraries(feed_reader  feed_reader_lib ${QT_LIBRARIES})
strict_warning(feed_reader)
