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

function(add_pprof name folder)
	message("add pprof ${name} ${folder}")
	
	set(name pprof_${name})

	file(GLOB tmp_h ${folder}/*.h)
	file(GLOB tmp_c ${folder}/*.c)
	file(GLOB tmp_cpp ${folder}/*.cpp)
	file(GLOB tmp_cc ${folder}/*.cc)

	if (WIN32)
		add_executable(${name} ${tmp_h} ${tmp_c} ${tmp_cpp} ${tmp_cc})
		set_target_properties(${name}
			PROPERTIES
			FOLDER "pprof"
			VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
		)
	else()
		add_executable(${name} ${tmp_c} ${tmp_cpp} ${tmp_cc})
	endif(WIN32)
	add_dependencies(${name} log_msg)
	target_link_libraries(${name} log_msg)
	target_include_directories(${name} PUBLIC
		${CMAKE_CURRENT_LIST_DIR}/log_msg)

	if (BUILD_SANITIZER)
		if ((${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU") OR
			(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
			target_compile_options(${name} PUBLIC
				-fsanitize=undefined
				-fsanitize=address
				-fsanitize=leak)
			target_link_options(${name} PUBLIC
				-fsanitize=undefined
				-fsanitize=address
				-fsanitize=leak)
		endif()
	endif()
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

	add_pprof(mugglec_log_sync ${CMAKE_CURRENT_LIST_DIR}/pprof/mugglec_log_sync)
	add_dependencies(pprof_mugglec_log_sync mugglec)
	target_link_libraries(pprof_mugglec_log_sync mugglec)
	target_include_directories(pprof_mugglec_log_sync PUBLIC
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

	if (BUILD_SANITIZER)
		if ((${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU") OR
			(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
			target_compile_options(spdlog PUBLIC
				-fsanitize=undefined
				-fsanitize=address
				-fsanitize=leak)
			target_link_options(spdlog PUBLIC
				-fsanitize=undefined
				-fsanitize=address
				-fsanitize=leak)
		endif()
	endif()

	add_log_gbenchmark(spdlog_basic ${CMAKE_CURRENT_LIST_DIR}/spdlog_basic)
	add_dependencies(gbenchmark_spdlog_basic spdlog)
	target_link_libraries(gbenchmark_spdlog_basic spdlog)

	add_log_gbenchmark(spdlog_async ${CMAKE_CURRENT_LIST_DIR}/spdlog_async)
	add_dependencies(gbenchmark_spdlog_async spdlog)
	target_link_libraries(gbenchmark_spdlog_async spdlog)

	add_pprof(spdlog_basic ${CMAKE_CURRENT_LIST_DIR}/pprof/spdlog_basic)
	add_dependencies(pprof_spdlog_basic spdlog)
	target_link_libraries(pprof_spdlog_basic spdlog)
endif()

# nano log
if (benchmark_nanolog)
	message("# fetch nanolog")
	FetchContent_Declare(
		nanolog
		GIT_REPOSITORY https://github.com/PlatformLab/NanoLog.git
		GIT_TAG "2a94d70f9d1db4da416053b1b926387fa068a59b"
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

	add_pprof(nanolog_basic ${CMAKE_CURRENT_LIST_DIR}/pprof/nanolog_basic)
	add_dependencies(pprof_nanolog_basic nanolog_lib)
	target_link_libraries(pprof_nanolog_basic nanolog_lib)
	target_include_directories(pprof_nanolog_basic PUBLIC ${FETCHCONTENT_BASE_DIR}/nanolog-src/runtime)
endif()

# copy pprof.sh
file(GLOB_RECURSE scripts "${CMAKE_CURRENT_LIST_DIR}/pprof/*.sh")
foreach(script ${scripts})
	file(RELATIVE_PATH rel_path ${CMAKE_CURRENT_LIST_DIR}/pprof ${script})
	set(dst_file "${CMAKE_CURRENT_LIST_DIR}/build/${rel_path}")
	add_custom_command(
		OUTPUT "${dst_file}"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "${script}" "${CMAKE_BINARY_DIR}/"
		DEPENDS "${script}"
		COMMENT "copy ${script} to ${dst_file}"
		)
	list(APPEND dst_files "${dst_file}")
endforeach()
add_custom_target(COPY_SCRIPTS ALL DEPENDS ${dst_files})
