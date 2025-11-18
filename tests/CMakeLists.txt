include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
FetchContent_MakeAvailable(googletest)

add_executable(le_test test_database.cpp)

target_include_directories(le_test PUBLIC ${CMAKE_SOURCE_DIR}/database/include)

target_link_libraries(le_test PRIVATE GTest::gtest_main database)

target_compile_definitions(
  le_test PRIVATE DB_PATH="${CMAKE_CURRENT_BINARY_DIR}/le_test.db3")

target_compile_definitions(le_test
                           PRIVATE DB_QUERY_FOLDER="${CMAKE_SOURCE_DIR}/")

target_compile_definitions(
  database PUBLIC DB_QUERY_FOLDER="${CMAKE_SOURCE_DIR}/queries"
                  INIT_QUERY="init.sql")

include(GoogleTest)
gtest_discover_tests(le_test)
