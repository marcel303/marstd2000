cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_STANDARD 14)

set(HAS_LIBGG 0)
set(HAS_MARSTD2000 0)

# --- dependencies ---

if (NOT HAS_LIBGG)
	message("including libgg")
	add_subdirectory(../../framework/libgg libgg)
endif (NOT HAS_LIBGG)
if (NOT HAS_FRAMEWORK)
	message("including framework")
	add_subdirectory(../../framework/framework framework)
endif (NOT HAS_FRAMEWORK)

add_subdirectory(marstd marstd2000)

#

project(marstd2000-examples)

# --- marstd2000-examples ---

set(libs marstd2000 framework ${FRAMEWORK_LIBRARIES})

set(source_test main.cpp)
add_executable(test ${source_test})
target_link_libraries(test ${libs})

set(source_bezier examples/bezier/main.cpp)

add_executable(bezier ${source_bezier})

target_link_libraries(bezier ${libs})