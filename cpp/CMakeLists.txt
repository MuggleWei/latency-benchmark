cmake_minimum_required(VERSION 3.0.2)
project(latency-benchmark)

# set compile parameter
if (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wall -Wextra -std=c++11")
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wall -Wextra -Wno-missing-field-initializers")
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
if (NOT WIN32)
	target_compile_definitions(uWS PRIVATE UWS_THREADSAFE)
endif()
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

# glog
set(WITH_GFLAGS OFF CACHE BOOL "glog with gflags") 
set(BUILD_TESTING OFF CACHE BOOL "glog build testting") 
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/glog)
set_target_properties(glog
	PROPERTIES FOLDER "thirdparty/google"
)

# dependencies
set(dependencies_inc
	${OPENSSL_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIR}
	${CMAKE_CURRENT_LIST_DIR}/thirdparty/libuv/include
	${CMAKE_BINARY_DIR}/thirdparty/uWS/include/
	${CMAKE_CURRENT_LIST_DIR}/thirdparty/rapidjson/include
	${CMAKE_CURRENT_LIST_DIR}/thirdparty/muggleCC
	${CMAKE_CURRENT_LIST_DIR}/thirdparty/google/glog/src
	${CMAKE_BINARY_DIR}/thirdparty/google/glog/
)

