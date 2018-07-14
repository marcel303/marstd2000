cmake_minimum_required(VERSION 2.6)

set(HAS_MARSTD2000 1 PARENT_SCOPE)
message("setting HAS_MARSTD2000")

if (NOT HAS_LIBGG)
	message("including libgg")
	add_subdirectory(../../../framework/libgg libgg)
endif (NOT HAS_LIBGG)

# --- dependencies ---

#

project(marstd2000)

# --- marstd2000 ---

file(GLOB_RECURSE other RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.*")
set_source_files_properties(${other} PROPERTIES HEADER_FILE_ONLY TRUE)

set(source cISOSURFACE.cpp xBSP.cpp xCOLLISION.cpp xCOLLISION2.cpp xCOLLISION3.cpp xGLOBALS.cpp XMESSAGE.cpp xPOLY.cpp xTESTENV.cpp)
set_source_files_properties(${source} PROPERTIES HEADER_FILE_ONLY FALSE)

source_group("sources" FILES ${source} ${other})

add_library(marstd2000 ${source} ${other})

target_include_directories(marstd2000 PUBLIC "${PROJECT_SOURCE_DIR}")

target_link_libraries(marstd2000 PUBLIC libgg)
