cmake_minimum_required(VERSION 3.0.2)
project(latency-benchmark)

# set compile parameter
if (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wall -Wextra -std=c++17")
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wall -Wextra -Wno-missing-field-initializers -std=c++17")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS=1 -D_UNICODE -DUNICODE)
	# add_definitions(-DNOMINMAX) # for fix std::max conflict in windows
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

# add custom cmake modules
INCLUDE(FindPackageHandleStandardArgs)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/cmake/Modules)
include(${CMAKE_CURRENT_LIST_DIR}/thirdparty/muggleCC/cmake/muggle_utils.cmake)

# dependencies
find_package(Threads)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

# glog
set(WITH_GFLAGS OFF CACHE BOOL "glog with gflags") 
set(BUILD_TESTING OFF CACHE BOOL "glog build testting") 
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/google/glog)
set_target_properties(glog
	PROPERTIES FOLDER "thirdparty/google"
)

# muggleCC
set(MUGGLE_BUILD_ONLY_C OFF)
set(MUGGLE_BUILD_EXAMPLE OFF)
set(MUGGLE_BUILD_TEST OFF)
set(MUGGLE_BUILD_BENCHMARK OFF)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/muggleCC)
set_target_properties(muggleCC
	PROPERTIES
	FOLDER "thirdparty/muggle"
)

# libuv
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/libuv)
set_target_properties(uv uv_a
	PROPERTIES FOLDER "thirdparty/libuv"
)

# uWebSockets
file(GLOB UWS_H ${CMAKE_CURRENT_LIST_DIR}/thirdparty/uWebSockets/src/*.h)
file(GLOB UWS_CPP ${CMAKE_CURRENT_LIST_DIR}/thirdparty/uWebSockets/src/*.cpp)
if (WIN32)
	add_library(uWS SHARED
		${UWS_H}
		${UWS_CPP}
	)
else()
	add_library(uWS SHARED
		${UWS_CPP}
	)
endif(WIN32)
add_dependencies(uWS uv)
target_link_libraries(uWS
	${OPENSSL_LIBRARIES}
	${ZLIB_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	uv
)
target_include_directories(uWS PUBLIC
	${OPENSSL_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIR}
	${CMAKE_CURRENT_LIST_DIR}/thirdparty/libuv/include
)
target_compile_definitions(uWS PRIVATE UWS_THREADSAFE)
set_target_properties(uWS
	PROPERTIES FOLDER "thirdparty/uWs"
)
add_custom_command(
	TARGET uWS POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/thirdparty/uWS
)
foreach(uWS_Header ${UWS_H})
	get_filename_component(uWS_Header_name ${uWS_Header} NAME)
	add_custom_command(
		TARGET uWS
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${uWS_Header} ${CMAKE_BINARY_DIR}/thirdparty/uWS/include/uWS/${uWS_Header_name}
	)
endforeach(uWS_Header ${UWS_H})

set_target_properties(uWS PROPERTIES PUBLIC_HEADER "${UWS_H}")
install(
    TARGETS uWS
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static
    PUBLIC_HEADER DESTINATION include/uWS
)

# latency common
file(GLOB latency_common_H ${CMAKE_CURRENT_LIST_DIR}/src/latency_common/*.h)
file(GLOB latency_common_CPP ${CMAKE_CURRENT_LIST_DIR}/src/latency_common/*.cpp)
if (WIN32)
	add_library(latency_common_cpp STATIC
		${latency_common_H}
		${latency_common_CPP}
	)
else()
	add_library(latency_common_cpp STATIC
		${latency_common_CPP}
	)
endif()
target_include_directories(latency_common_cpp
	PUBLIC 
	${CMAKE_CURRENT_LIST_DIR}/thirdparty/rapidjson/include
	${CMAKE_CURRENT_LIST_DIR}/thirdparty/google/glog/src
	${CMAKE_BINARY_DIR}/thirdparty/google/glog/
	${CMAKE_CURRENT_LIST_DIR}/src
)

# thread trans utils function
# function(add_latency_benchmark name folder)
# 	file(GLOB tmp_h ${folder}/*.h)
# 	file(GLOB tmp_cpp ${folder}/*.cpp)
# 	if (WIN32)
# 		add_executable(${name} ${tmp_h} ${tmp_cpp})
# 	else()
# 		add_executable(${name} ${tmp_cpp})
# 	endif(WIN32)
# 	add_dependencies(${name} muggleCC glog latency_common_cpp)
# 	target_include_directories(${name}
# 		PRIVATE 
# 		${CMAKE_CURRENT_LIST_DIR}/thirdparty/rapidjson/include
# 		${CMAKE_CURRENT_LIST_DIR}/thirdparty/google/glog/src
# 		${CMAKE_BINARY_DIR}/thirdparty/google/glog/
# 		${CMAKE_CURRENT_LIST_DIR}/thirdparty/muggleCC
# 		${CMAKE_CURRENT_LIST_DIR}/src
# 	)
# 	target_link_libraries(${name} muggleCC glog latency_common_cpp)
# endfunction(add_latency_benchmark)
# 
function(add_benchmark name folder)
	message("add benchmark ${name} ${folder}")
	
	set(name benchmark_${name})

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
	target_include_directories(${name}
		PRIVATE 
		${CMAKE_CURRENT_LIST_DIR}/thirdparty/rapidjson/include
		${CMAKE_CURRENT_LIST_DIR}/thirdparty/google/glog/src
		${CMAKE_BINARY_DIR}/thirdparty/google/glog/
		${CMAKE_CURRENT_LIST_DIR}/thirdparty/muggleCC
		${CMAKE_CURRENT_LIST_DIR}/src
	)
	add_dependencies(${name} muggleCC muggle_benchmark glog)
	target_link_libraries(${name}
		muggleCC
		muggle_benchmark
		glog
	)
endfunction()


# ws utils function
function(add_ws name folder)
	file(GLOB tmp_h ${folder}/*.h)
	file(GLOB tmp_cpp ${folder}/*.cpp)
	file(GLOB handle_h ${CMAKE_CURRENT_LIST_DIR}/src/ws_trans/ws_trans_handle/*.h)
	file(GLOB handle_pp ${CMAKE_CURRENT_LIST_DIR}/src/ws_trans/ws_trans_handle/*.cpp)
	if (WIN32)
		add_executable(${name} ${tmp_h} ${tmp_cpp} ${handle_h} ${handle_pp})
	else()
		add_executable(${name} ${tmp_cpp} ${handle_pp})
	endif(WIN32)
	add_dependencies(${name} muggleCC glog uWS latency_common_cpp)
	target_include_directories(${name}
		PRIVATE 
		${OPENSSL_INCLUDE_DIR}
		${ZLIB_INCLUDE_DIR}
		${CMAKE_CURRENT_LIST_DIR}/thirdparty/libuv/include
		${CMAKE_CURRENT_LIST_DIR}/thirdparty/uWebSockets/uSockets/src
		${CMAKE_CURRENT_LIST_DIR}/thirdparty/uWebSockets/src
		${CMAKE_CURRENT_LIST_DIR}/thirdparty/rapidjson/include
		${CMAKE_CURRENT_LIST_DIR}/thirdparty/google/glog/src
		${CMAKE_BINARY_DIR}/thirdparty/google/glog/
		${CMAKE_CURRENT_LIST_DIR}/thirdparty/muggleCC
		${CMAKE_CURRENT_LIST_DIR}/src
	)
	target_link_libraries(${name} muggleCC glog uWS latency_common_cpp)
endfunction(add_ws)

# # loop increment
# add_benchmark(loop_increment ${CMAKE_CURRENT_LIST_DIR}/src/loop_increment)

# benchmark
if (NOT ${MUGGLE_BUILD_ONLY_C})
	SUBDIRLIST(benchmark_root_dir ${CMAKE_CURRENT_LIST_DIR}/src/benchmark)
	FOREACH(subdir ${benchmark_root_dir})
		add_benchmark(${subdir} ${CMAKE_CURRENT_LIST_DIR}/src/benchmark/${subdir})
	ENDFOREACH()
endif()

# ws trans
add_ws(ws_trans_server_cpp ${CMAKE_CURRENT_LIST_DIR}/src/ws_trans/ws_trans_server)
add_ws(ws_trans_client_cpp ${CMAKE_CURRENT_LIST_DIR}/src/ws_trans/ws_trans_client)
set_target_properties(ws_trans_server_cpp ws_trans_client_cpp
	PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
)

