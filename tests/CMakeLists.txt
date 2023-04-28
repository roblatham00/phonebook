add_executable (test-admin test-admin.cpp)
target_include_directories (test-admin PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${CMAKE_CURRENT_BINARY_DIR}/../src
)
target_link_libraries (test-admin PRIVATE Catch2::Catch2WithMain alpha-server alpha-admin)
add_test (NAME TestAdmin COMMAND ./test-admin)

add_executable (test-client test-client.cpp)
target_include_directories (test-client PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${CMAKE_CURRENT_BINARY_DIR}/../src
)
target_link_libraries (test-client PRIVATE Catch2::Catch2WithMain alpha-server alpha-admin alpha-client)
add_test (NAME TestClient COMMAND ./test-client)
