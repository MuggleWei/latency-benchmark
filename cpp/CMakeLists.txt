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
# deps
################################

include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

# google/benchmark
message("# fetch google/benchmark")

set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "")
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "")

FetchContent_Declare(
	benchmark
	GIT_REPOSITORY https://github.com/google/benchmark.git
	GIT_TAG v1.8.3
	GIT_SHALLOW TRUE
	GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(benchmark)

################################
# projects
################################

function(add_app name folder)
	file(GLOB_RECURSE tmp_h ${folder}/*.h)
	file(GLOB_RECURSE tmp_c ${folder}/*.c)
	file(GLOB_RECURSE tmp_cpp ${folder}/*.cpp)
	file(GLOB_RECURSE tmp_cc ${folder}/*.cc)

	if (MSVC OR MINGW)
		add_executable(${name} ${tmp_h} ${tmp_c} ${tmp_cpp} ${tmp_cc})
		set_target_properties(${name}
			PROPERTIES
			FOLDER "example"
			VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
		)
	else()
		add_executable(${name} ${tmp_c} ${tmp_cpp} ${tmp_cc})
		if (APPLE)
			set_target_properties(${name}
				PROPERTIES
				INSTALL_RPATH "@executable_path/../lib"
			)
		elseif (UNIX)
			set_target_properties(${name}
				PROPERTIES
				INSTALL_RPATH "\$ORIGIN/../lib"
			)
		endif()
	endif(MSVC OR MINGW)
	target_include_directories(${name} PUBLIC ${folder})

	if ((${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU") OR
		(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
		target_compile_options(${name} PUBLIC -save-temps=obj)
	endif()
endfunction()

function(add_gbenchmark name folder)
	message("add google benchmark ${name} ${folder}")
	set(name gbenchmark_${name})
	add_app(${name} ${folder})
	add_dependencies(${name} benchmark)
	target_link_libraries(${name} benchmark)
endfunction()

function(add_benchmark name folder)
	message("add benchmark ${name} ${folder}")
	set(name benchmark_${name})
	add_app(${name} ${folder})
endfunction()

set(gbenchmark_root_dir ${CMAKE_CURRENT_LIST_DIR}/gbenchmark)
FILE(GLOB subdirs RELATIVE ${gbenchmark_root_dir} ${gbenchmark_root_dir}/*)
FOREACH(subdir ${subdirs})
	IF(IS_DIRECTORY ${gbenchmark_root_dir}/${subdir})
		add_gbenchmark(${subdir} ${gbenchmark_root_dir}/${subdir})
	ENDIF()
ENDFOREACH()

set(benchmark_root_dir ${CMAKE_CURRENT_LIST_DIR}/benchmark)
FILE(GLOB subdirs RELATIVE ${benchmark_root_dir} ${benchmark_root_dir}/*)
FOREACH(subdir ${subdirs})
	IF(IS_DIRECTORY ${benchmark_root_dir}/${subdir})
		add_benchmark(${subdir} ${benchmark_root_dir}/${subdir})
	ENDIF()
ENDFOREACH()
