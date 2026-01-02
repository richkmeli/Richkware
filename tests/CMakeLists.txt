find_package(GTest REQUIRED)

# Unit tests
file(GLOB_RECURSE TEST_SOURCES "unit/*.cpp")

add_executable(richkware_tests ${TEST_SOURCES})

target_link_libraries(richkware_tests
    PRIVATE
        richkware_lib
        GTest::gtest
        GTest::gtest_main
)

target_include_directories(richkware_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add tests to CTest
include(GoogleTest)
gtest_discover_tests(richkware_tests)

# Integration tests
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/integration")
    file(GLOB_RECURSE INTEGRATION_TEST_SOURCES "integration/*.cpp")
    
    if(INTEGRATION_TEST_SOURCES)
        add_executable(richkware_integration_tests ${INTEGRATION_TEST_SOURCES})
        
        target_link_libraries(richkware_integration_tests
            PRIVATE
                richkware_lib
                GTest::gtest
                GTest::gtest_main
        )
        
        gtest_discover_tests(richkware_integration_tests)
    endif()
endif()