# options
set(benchmark_mugglec_log ON CACHE BOOL "generate mugglec log benchmark")
set(benchmark_spdlog ON CACHE BOOL "generate spdlog benchmark")
set(benchmark_nanolog ON CACHE BOOL "generate nanolog benchmark")

# log message
FILE(GLOB_RECURSE log_msg_h "${CMAKE_CURRENT_LIST_DIR}/log_msg/*.h")
FILE(GLOB_RECURSE log_msg_cpp "${CMAKE_CURRENT_LIST_DIR}/log_msg/*.cpp")
add_library(log_msg STATIC
	${log_msg_h}
	${log_msg_cpp})

# functions
function(add_log_gbenchmark name folder)
	add_gbenchmark(${name} ${folder})
	add_dependencies(gbenchmark_${name} log_msg)
	target_link_libraries(gbenchmark_${name} log_msg)
	target_include_directories(gbenchmark_${name} PUBLIC
		${CMAKE_CURRENT_LIST_DIR}/log_msg)
endfunction()

# mugglec log
if (benchmark_mugglec_log)
	add_log_gbenchmark(mugglec_log_sync ${CMAKE_CURRENT_LIST_DIR}/mugglec_log_sync)
	add_dependencies(gbenchmark_mugglec_log_sync mugglec)
	target_link_libraries(gbenchmark_mugglec_log_sync mugglec)
	target_include_directories(gbenchmark_mugglec_log_sync PUBLIC
		${FETCHCONTENT_BASE_DIR}/mugglec-src
		${FETCHCONTENT_BASE_DIR}/mugglec-build/generated)

	add_log_gbenchmark(mugglec_log_async ${CMAKE_CURRENT_LIST_DIR}/mugglec_log_async)
	add_dependencies(gbenchmark_mugglec_log_async mugglec)
	target_link_libraries(gbenchmark_mugglec_log_async mugglec)
	target_include_directories(gbenchmark_mugglec_log_async PUBLIC
		${FETCHCONTENT_BASE_DIR}/mugglec-src
		${FETCHCONTENT_BASE_DIR}/mugglec-build/generated)
endif()

# spdlog
if (benchmark_spdlog)
	message("# fetch spdlog")
	FetchContent_Declare(
		spdlog
		GIT_REPOSITORY https://github.com/gabime/spdlog.git
		GIT_TAG v1.12.0
		GIT_SHALLOW TRUE
		GIT_PROGRESS TRUE
		)
	FetchContent_MakeAvailable(spdlog)

	add_log_gbenchmark(spdlog_basic ${CMAKE_CURRENT_LIST_DIR}/spdlog_basic)
	add_dependencies(gbenchmark_spdlog_basic spdlog)
	target_link_libraries(gbenchmark_spdlog_basic spdlog)

	add_log_gbenchmark(spdlog_async ${CMAKE_CURRENT_LIST_DIR}/spdlog_async)
	add_dependencies(gbenchmark_spdlog_async spdlog)
	target_link_libraries(gbenchmark_spdlog_async spdlog)
endif()

# nano log
if (benchmark_nanolog)
	message("# fetch nanolog")
	FetchContent_Declare(
		nanolog
		GIT_REPOSITORY https://github.com/PlatformLab/NanoLog.git
		GIT_TAG master
		GIT_SHALLOW TRUE
		GIT_PROGRESS TRUE
		)
	FetchContent_Populate(nanolog)

	include(ExternalProject)
	ExternalProject_Add(nanolog
		SOURCE_DIR "${FETCHCONTENT_BASE_DIR}/nanolog-src/runtime"
		#BINARY_DIR "${FETCHCONTENT_BASE_DIR}/nanolog-build"
		DOWNLOAD_COMMAND ""
		CONFIGURE_COMMAND ""
		BUILD_COMMAND "make"
		BUILD_IN_SOURCE TRUE
		INSTALL_COMMAND ""
		BUILD_BYPRODUCTS "${FETCHCONTENT_BASE_DIR}/nanolog-src/runtime/libnano.a"
		)

	add_library(nanolog_lib STATIC IMPORTED GLOBAL)
	set_property(TARGET nanolog_lib
		PROPERTY IMPORTED_LOCATION 
		${FETCHCONTENT_BASE_DIR}/nanolog-src/runtime/libNanoLog.a
		)

	add_log_gbenchmark(nanolog_basic ${CMAKE_CURRENT_LIST_DIR}/nanolog_basic)
	add_dependencies(gbenchmark_nanolog_basic nanolog_lib)
	target_link_libraries(gbenchmark_nanolog_basic nanolog_lib)
	target_include_directories(gbenchmark_nanolog_basic PUBLIC ${FETCHCONTENT_BASE_DIR}/nanolog-src/runtime)
endif()
