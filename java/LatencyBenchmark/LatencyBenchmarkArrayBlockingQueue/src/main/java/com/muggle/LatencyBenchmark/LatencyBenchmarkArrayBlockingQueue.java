package com.muggle.latencybenchmark;

import org.apache.commons.cli.ParseException;

import java.util.concurrent.ArrayBlockingQueue;

public class LatencyBenchmarkArrayBlockingQueue {
    /**
     * user arguments
     */
    public static class UserArgs {
        // empty
    }

    /**
     * producer writer
     */
    public static class ArrayBlockingQueueWrite implements LatencyBenchmarkThreadTrans.WriteCallback<UserArgs> {
        public ArrayBlockingQueue<LatencyBenchmarkThreadTransMessage> queue;

        public ArrayBlockingQueueWrite(ArrayBlockingQueue<LatencyBenchmarkThreadTransMessage> queue) {
            this.queue = queue;
        }

        @Override
        public boolean write(
                UserArgs userArgs,
                LatencyBenchmarkThreadTransMessage msg) throws InterruptedException {
            this.queue.put(msg);
            return true;
        }
    }

    /**
     * consumer reader
     */
    public static class ArrayBlockingQueueRead
            implements LatencyBenchmarkThreadTrans.ReadCallback<UserArgs> {
        public ArrayBlockingQueue<LatencyBenchmarkThreadTransMessage> queue;

        public ArrayBlockingQueueRead(ArrayBlockingQueue<LatencyBenchmarkThreadTransMessage> queue) {
            this.queue = queue;
        }

        @Override
        public LatencyBenchmarkThreadTransMessage read(UserArgs userArgs) throws InterruptedException {
            return this.queue.take();
        }
    }

    /**
     * notify all consumer to exit
     */
    public static class ArrayBlockingQueueCompleted
            implements LatencyBenchmarkThreadTrans.CompletedCallback<UserArgs> {
        public ArrayBlockingQueue<LatencyBenchmarkThreadTransMessage> queue;

        public ArrayBlockingQueueCompleted(ArrayBlockingQueue<LatencyBenchmarkThreadTransMessage> queue) {
            this.queue = queue;
        }

        @Override
        public void completed(LatencyBenchmarkConfig config, UserArgs userArgs) throws InterruptedException {
            for (int i = 0; i < config.getConsumer(); i++) {
                LatencyBenchmarkThreadTransMessage msg = new LatencyBenchmarkThreadTransMessage();
                msg.id = -1;
                this.queue.put(msg);
            }
        }
    }

    public void run(String[] args) throws ParseException, InterruptedException {
        System.out.println("run LatencyBenchmarkArrayBlockingQueue");

        LatencyBenchmarkConfig config = new LatencyBenchmarkConfig();
        config.parseCommandLine(args);
        config.setProducer(0);
        config.setConsumer(0);
        config.output();

        int hc = Runtime.getRuntime().availableProcessors();
        if (hc <= 0) {
            hc = 2;
        }
        int[] producerNums = new int[]{
                1, 2, 4, hc / 2, hc, hc * 2, hc * 4
        };
        int[] consumerNums = new int[]{
                1, 2, 4, hc / 2, hc, hc * 2, hc * 4
        };

        for (int producerNum : producerNums) {
            for (int consumerNum : consumerNums) {
                config.setProducer(producerNum);
                config.setConsumer(consumerNum);

                ArrayBlockingQueue<LatencyBenchmarkThreadTransMessage> queue =
                        new ArrayBlockingQueue<>(config.getCapacity());

                LatencyBenchmarkThreadTrans<UserArgs> benchmark =
                        new LatencyBenchmarkThreadTrans<>(
                                config,
                                null,
                                new ArrayBlockingQueueWrite(queue),
                                new ArrayBlockingQueueRead(queue),
                                new ArrayBlockingQueueCompleted(queue));

                System.out.println(String.format("run ArrayBlockingQueue - %d writer and %d reader",
                        config.getProducer(), config.getConsumer()));
                benchmark.run();

                benchmark.genReport(String.format("arrayblockingqueue_%d_%d",
                        config.getProducer(), config.getConsumer()));
            }
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
