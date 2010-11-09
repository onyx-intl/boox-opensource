include_directories(${CMAKE_SOURCE_DIR}/src)

add_definitions(-DTEST_DATA_DIR="${CMAKE_SOURCE_DIR}/tests/data")
add_definitions(-DTEST_TMP_DIR="${CMAKE_SOURCE_DIR}/tests/tmp")

# onyx_test(onyx_feed_reader_acceptance_test acceptance_test.cpp )
# target_link_libraries(onyx_feed_reader_acceptance_test
#   onyx_feed_reader_lib ${ONYX_LIBS} ${QT_LIBRARIES})

onyx_test(onyx_feed_reader_article_test article_test.cpp )
target_link_libraries(onyx_feed_reader_article_test
  onyx_feed_reader_lib ${ONYX_LIBS} ${QT_LIBRARIES})

onyx_test(onyx_feed_reader_database_test database_test.cpp )
target_link_libraries(onyx_feed_reader_database_test
  onyx_feed_reader_lib ${ONYX_LIBS} ${QT_LIBRARIES})

onyx_test(onyx_feed_reader_feed_test feed_test.cpp )
target_link_libraries(onyx_feed_reader_feed_test
  onyx_feed_reader_lib ${ONYX_LIBS} ${QT_LIBRARIES})

onyx_test(onyx_feed_reader_rss_feed_parser_test rss_feed_parser_test.cpp )
target_link_libraries(onyx_feed_reader_rss_feed_parser_test
  onyx_feed_reader_lib ${ONYX_LIBS} ${QT_LIBRARIES})
