# 简介
当前 cpp 目录分为两部分
* gbenchmark: 使用 google benchmark 来做基准测试
* deprecated: 旧的基准测试

其中 gbenchmark 中又有几部分
* std: 标准库的一些基准测试
* log_libs: 一些开源日志库的基准测试
* log_libs/pprof: 开源日志库不进行基准测试, 而是打印日志, 配合其中的 pprof.sh 使用, 分析代码的开销. 会生成 text 模式以及 callgrind 文件, 可以配合 kcachegrind 分析
