cmake_minimum_required(VERSION 3.17.6)
project(latency-benchmark)

################################
# general config
################################

# set compile parameter
if (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wall -Wextra -std=c++17")
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wall -Wextra -Wno-missing-field-initializers -std=c++17")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS=1 -D_UNICODE -DUNICODE)
	# add_definitions(-DNOMINMAX) # for fix std::max conflict in windows
	add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

# set output dir
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# for vim plugin - YCM
if (NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# set use folder in vs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################
# options
################################

option(BUILD_SANITIZER "Compile mugglec with sanitizer" OFF)

if (BUILD_SANITIZER)
	if ((${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU") OR
		(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
		add_compile_options(
			-fsanitize=undefined
			-fsanitize=address
			-fsanitize=leak)
		add_link_options(
			-fsanitize=undefined
			-fsanitize=address
			-fsanitize=leak)
	endif()
endif()

################################
# deps
################################

include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

# mugglec
message("# fetch mugglec")

set(MUGGLE_BUILD_TRACE OFF CACHE BOOL "")
set(MUGGLE_BUILD_SHARED_LIB ON CACHE BOOL "")
set(MUGGLE_BUILD_STATIC_PIC ON CACHE BOOL "")
set(MUGGLE_BUILD_BENCHMARK OFF CACHE BOOL "")
set(MUGGLE_BUILD_TESTING OFF CACHE BOOL "")
set(MUGGLE_BUILD_EXAMPLE OFF CACHE BOOL "")

FetchContent_Declare(
	mugglec
	GIT_REPOSITORY https://github.com/MuggleWei/mugglec.git
	GIT_TAG v1.3.2
	GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(mugglec)

# google/benchmark
message("# fetch google/benchmark")

set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "")
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "")

FetchContent_Declare(
	benchmark
	GIT_REPOSITORY https://github.com/google/benchmark.git
	GIT_TAG v1.8.3
	GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(benchmark)

################################
# projects
################################

# functions
function(add_gbenchmark name folder)
	message("add google benchmark ${name} ${folder}")
	
	set(name gbenchmark_${name})

	file(GLOB tmp_h ${folder}/*.h)
	file(GLOB tmp_c ${folder}/*.c)
	file(GLOB tmp_cpp ${folder}/*.cpp)
	file(GLOB tmp_cc ${folder}/*.cc)

	if (WIN32)
		add_executable(${name} ${tmp_h} ${tmp_c} ${tmp_cpp} ${tmp_cc})
		set_target_properties(${name}
			PROPERTIES
			FOLDER "benchmark"
			VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
		)
	else()
		add_executable(${name} ${tmp_c} ${tmp_cpp} ${tmp_cc})
	endif(WIN32)
	add_dependencies(${name} benchmark)
	target_link_libraries(${name} benchmark)
	target_include_directories(${name} PRIVATE
		${FETCHCONTENT_BASE_DIR}/benchmark-src
	)
endfunction()

# google benchmark - std
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/gbenchmark/std)
