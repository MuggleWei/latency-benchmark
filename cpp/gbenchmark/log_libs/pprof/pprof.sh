#!/bin/bash

run() {
	local name=$1
	env LD_PRELOAD="/usr/lib/libprofiler.so" \
		CPUPROFILE_FREQUENCY=10000 \
		CPUPROFILE=./${name}.prof \
		./bin/pprof_${name}

	pprof --text ./bin/pprof_${name} ./${name}.prof
	pprof --callgrind ./bin/pprof_${name} ./${name}.prof > ${name}.callgrind
}

run mugglec_log_sync
run spdlog_basic
run nanolog_basic
