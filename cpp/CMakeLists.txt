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

# libuv
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/libuv)
set_target_properties(uv uv_a
	PROPERTIES FOLDER "thirdparty/libuv"
)

# uWebSockets
file(GLOB uSockets_C ${CMAKE_CURRENT_LIST_DIR}/thirdparty/uWebSockets/uSockets/src/*.c)
file(GLOB uSockets_eventing_C ${CMAKE_CURRENT_LIST_DIR}/thirdparty/uWebSockets/uSockets/src/eventing/*.c)
file(GLOB uWebSockets_H ${CMAKE_CURRENT_LIST_DIR}/thirdparty/uWebSockets/src/*.h)
file(GLOB uWebSockets_HPP ${CMAKE_CURRENT_LIST_DIR}/thirdparty/uWebSockets/src/*.hpp)
if (WIN32)
	add_library(uWS SHARED
		${uSockets_C}
		${uSockets_eventing_C}
		${uWebSockets_H}
		${uWebSockets_HPP}
	)
else()
	add_library(uWS SHARED
		${uSockets_C}
		${uSockets_eventing_C}
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
	${CMAKE_CURRENT_LIST_DIR}/thirdparty/uWebSockets/uSockets/src
	${CMAKE_CURRENT_LIST_DIR}/thirdparty/uWebSockets/src
	${CMAKE_CURRENT_LIST_DIR}/thirdparty/libuv/include
)
# target_compile_options(uWS PUBLIC
# 	-DLIBUS_NO_SSL -DUWS_NO_ZLIB -DLIBUS_NO_SSL -DUWS_NO_ZLIB -DLIBUS_USE_LIBUV
# )
set_target_properties(uWS
	PROPERTIES FOLDER "thirdparty/uWs"
)

# glog
set(WITH_GFLAGS OFF CACHE BOOL "glog with gflags") 
set(BUILD_TESTING OFF CACHE BOOL "glog build testting") 
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/google/glog)
set_target_properties(glog
	PROPERTIES FOLDER "thirdparty/google"
)

# dependencies
set(dependencies_inc
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
	PUBLIC ${dependencies_inc}
)

# utils function
function(add_exe name folder)
	file(GLOB tmp_h ${folder}/*.h)
	file(GLOB tmp_cpp ${folder}/*.cpp)
	if (WIN32)
		add_executable(${name} ${tmp_h} ${tmp_cpp})
	else()
		add_executable(${name} ${tmp_cpp})
	endif(WIN32)
	add_dependencies(${name} uWS glog)
	target_include_directories(${name} PRIVATE ${dependencies_inc})
	target_link_libraries(${name} uWS glog latency_common_cpp)
endfunction(add_exe)

# ws server
add_exe(ws_trans_server_cpp ${CMAKE_CURRENT_LIST_DIR}/src/ws_trans/ws_trans_server)

set_target_properties(ws_trans_server_cpp
	PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
)
