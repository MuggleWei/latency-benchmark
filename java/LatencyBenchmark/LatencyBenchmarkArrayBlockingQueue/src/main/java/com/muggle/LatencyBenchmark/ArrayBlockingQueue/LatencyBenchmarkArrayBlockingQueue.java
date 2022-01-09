package com.muggle.latencybenchmark.arrayblockingqueue;

import com.muggle.latencybenchmark.common.LatencyBenchmarkConfig;
import com.muggle.latencybenchmark.common.LatencyBenchmarkThreadTrans;
import com.muggle.latencybenchmark.common.LatencyBenchmarkThreadTransMessage;
import org.apache.commons.cli.ParseException;

import java.util.concurrent.ArrayBlockingQueue;

public class LatencyBenchmarkArrayBlockingQueue {

    public static class ArrayBlockingQueueWrite
            implements LatencyBenchmarkThreadTrans.WriteCallback<ArrayBlockingQueue<LatencyBenchmarkThreadTransMessage>> {

        @Override
        public boolean write(
                ArrayBlockingQueue<LatencyBenchmarkThreadTransMessage> userArgs,
                LatencyBenchmarkThreadTransMessage msg) throws InterruptedException {
            userArgs.put(msg);
            return true;
        }
    }

    public static class ArrayBlockingQueueRead
            implements LatencyBenchmarkThreadTrans.ReadCallback<ArrayBlockingQueue<LatencyBenchmarkThreadTransMessage>> {

        @Override
        public LatencyBenchmarkThreadTransMessage read(ArrayBlockingQueue<LatencyBenchmarkThreadTransMessage> userArgs) throws InterruptedException {
            return userArgs.take();
        }
    }

    public static class ArrayBlockingQueueCompleted
            implements LatencyBenchmarkThreadTrans.CompletedCallback<ArrayBlockingQueue<LatencyBenchmarkThreadTransMessage>> {

        @Override
        public void completed(LatencyBenchmarkConfig config, ArrayBlockingQueue<LatencyBenchmarkThreadTransMessage> userArgs) throws InterruptedException {
            LatencyBenchmarkThreadTransMessage msg = new LatencyBenchmarkThreadTransMessage();
            msg.id = -1;
            userArgs.put(msg);
        }
    }


    public void run(String[] args) throws ParseException, InterruptedException {
        System.out.println("run LatencyBenchmarkArrayBlockingQueue");

        LatencyBenchmarkConfig config = new LatencyBenchmarkConfig();
        config.parseCommandLine(args);
        config.setProducer(0);
        config.output();

        int hc = Runtime.getRuntime().availableProcessors();
        if (hc <= 0) {
            hc = 2;
        }
        int[] producerNums = new int[]{
                1, 2, 4, hc / 2, hc, hc * 2, hc * 4
        };

        for (int producerNum : producerNums) {
            config.setProducer(producerNum);

            LatencyBenchmarkThreadTrans<ArrayBlockingQueue<LatencyBenchmarkThreadTransMessage>> benchmark =
                    new LatencyBenchmarkThreadTrans<>(
                            config,
                            new ArrayBlockingQueue<>(config.getCapacity()),
                            new ArrayBlockingQueueWrite(),
                            new ArrayBlockingQueueRead(),
                            new ArrayBlockingQueueCompleted());

            System.out.println(String.format("run ArrayBlockingQueue - %d writer and %d reader",
                    config.getProducer(), config.getConsumer()));
            benchmark.run();

            benchmark.genReport(String.format("arrayblockingqueue_%d_%d",
                    config.getProducer(), config.getConsumer()));
        }
    }

    public static void main(String[] args) {
        LatencyBenchmarkArrayBlockingQueue latencyBenchmark = new LatencyBenchmarkArrayBlockingQueue();
        try {
            latencyBenchmark.run(args);
        } catch (ParseException | InterruptedException e) {
            e.printStackTrace();
        }
    }
}
