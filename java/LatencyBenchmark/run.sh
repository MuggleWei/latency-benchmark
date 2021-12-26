#!/bin/bash

# ArrayBlockingQueue
java -cp \
	LatencyBenchmarkArrayBlockingQueue/target/lib/*:LatencyBenchmarkArrayBlockingQueue/target/LatencyBenchmarkArrayBlockingQueue-1.0-SNAPSHOT.jar \
	com.muggle.latencybenchmark.arrayblockingqueue.LatencyBenchmarkArrayBlockingQueue
