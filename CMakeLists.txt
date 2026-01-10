cmake_minimum_required(VERSION 3.16)
project(csuite C)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall)

set(BASE_LIBS_DIR "" CACHE PATH "Base directory for external libraries")

# WIN32
if(UNIX)
    set(BASE_LIBS_DIR "/usr" CACHE PATH "" FORCE)
endif()

file(GLOB SRC_FILES CONFIGURE_DEPENDS
    src/*.c
)

# -----------------------------
# Tests
# -----------------------------
enable_testing()

file(GLOB TEST_FILES CONFIGURE_DEPENDS
    tests/*.c
)

foreach(test_src ${TEST_FILES})
    get_filename_component(test_name ${test_src} NAME_WE)

    add_executable(${test_name}
        ${test_src}
        ${SRC_FILES}
    )

    target_include_directories(${test_name}
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
    )

    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

# -----------------------------
# Benches
# -----------------------------
file(GLOB BENCH_FILES CONFIGURE_DEPENDS
    benches/*.c
)

foreach(bench_src ${BENCH_FILES})
    get_filename_component(bench_name ${bench_src} NAME_WE)

    add_executable(${bench_name}
        ${bench_src}
        ${SRC_FILES}
    )

    target_include_directories(${bench_name}
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
            ${BASE_LIBS_DIR}/include
    )

    target_link_directories(${bench_name}
        PRIVATE
            ${BASE_LIBS_DIR}/lib
    )

    target_link_libraries(${bench_name}
        PRIVATE
            m
            jansson
    )
endforeach()
