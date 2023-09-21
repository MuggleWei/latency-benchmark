SUBDIRLIST(gbenchmark_std_root_dir ${CMAKE_CURRENT_LIST_DIR})
FOREACH(subdir ${gbenchmark_std_root_dir})
	add_gbenchmark(${subdir} ${CMAKE_CURRENT_LIST_DIR}/${subdir})
ENDFOREACH()
