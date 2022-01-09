#!/bin/bash

# ArrayBlockingQueue
java -cp \
	LatencyBenchmarkArrayBlockingQueue/target/lib/*:LatencyBenchmarkArrayBlockingQueue/target/LatencyBenchmarkArrayBlockingQueue-1.0-SNAPSHOT.jar \
	com.muggle.latencybenchmark.LatencyBenchmarkArrayBlockingQueue

# Disruptor
java -cp \
	LatencyBenchmarkDisruptor/target/lib/*:LatencyBenchmarkDisruptor/target/LatencyBenchmarkDisruptor-1.0-SNAPSHOT.jar \
	com.muggle.latencybenchmark.LatencyBenchmarkDisruptor
