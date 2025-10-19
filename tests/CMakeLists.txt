cmake_minimum_required(VERSION 2.8...3.14)

set(TARGET_LIST
 test-threads
 test-thrd_local
 test-stress
 test-atomicity
 test-spmc
 #test-mpsc
 #test-leak
 #test-malloc
)

foreach (TARGET ${TARGET_LIST})
    add_executable(${TARGET} ${TARGET}.c )
    target_link_libraries(${TARGET} ${PROJECT_NAME})
    add_test(NAME ${TARGET} COMMAND ${TARGET} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endforeach()
