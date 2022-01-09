package com.muggle.latencybenchmark.common;

import java.util.ArrayList;

/**
 * thread transfer benchmark
 */
public class LatencyBenchmarkThreadTrans<T> {

    public final static int ACTION_WRITE_BEGIN = 0;
    public final static int ACTION_WRITE_END = 1;
    public final static int ACTION_READ = 2;

    /**
     * @param <T> thread transfer benchmark write function
     */
    public interface WriteCallback<T> {
        boolean write(T userArgs, LatencyBenchmarkThreadTransMessage msg) throws InterruptedException;
    }

    /**
     * @param <T> thread transfer benchmark read function
     */
    public interface ReadCallback<T> {
        LatencyBenchmarkThreadTransMessage read(T userArgs) throws InterruptedException;
    }

    /**
     * @param <T> thread transfer benchmark completed function
     */
    public interface CompletedCallback<T> {
        public void completed(LatencyBenchmarkConfig config, T userArgs) throws InterruptedException;
    }

    /**
     * benchmark config
     */
    private LatencyBenchmarkConfig config;

    /**
     * benchmark handle
     */
    private LatencyBenchmarkHandle handle;

    /**
     * user customized arguments
     */
    private T userArgs;

    private WriteCallback<T> writeCallback;
    private ReadCallback<T> readCallback;
    private CompletedCallback<T> completedCallback;

    /**
     * thread transfer constructor
     *
     * @param config            benchmark config
     * @param userArgs          user customized arguments
     * @param writeCallback     write callback function
     * @param readCallback      read callback function
     * @param completedCallback completed callback function
     */
    public LatencyBenchmarkThreadTrans(
            LatencyBenchmarkConfig config,
            T userArgs,
            WriteCallback<T> writeCallback,
            ReadCallback<T> readCallback,
            CompletedCallback<T> completedCallback) {
        this.config = config;
        this.userArgs = userArgs;
        this.writeCallback = writeCallback;
        this.readCallback = readCallback;
        this.completedCallback = completedCallback;

        int recordCound =
                config.getTotalRounds() * config.getRecordPerRound() * config.getProducer();

        this.handle = new LatencyBenchmarkHandle(recordCound);
        this.handle.addAction(ACTION_WRITE_BEGIN, "write_begin");
        this.handle.addAction(ACTION_WRITE_END, "write_end");
        this.handle.addAction(ACTION_READ, "read");
    }

    /**
     * run thread transfer benchmark
     */
    public void run() throws InterruptedException {
        // initialize messages
        int recordCount = this.handle.getRecordCount();
        LatencyBenchmarkThreadTransMessage[] messages =
                new LatencyBenchmarkThreadTransMessage[recordCount];
        for (int i = 0; i < recordCount; i++) {
            messages[i] = new LatencyBenchmarkThreadTransMessage();
            messages[i].id = i;
        }

        // prepare consumers
        ArrayList<Thread> consumers = new ArrayList<>();
        for (int i = 0; i < this.config.getConsumer(); i++) {
            LatencyBenchmarkThreadTransConsumer.ConsumerArgs<T> consumerArgs =
                    new LatencyBenchmarkThreadTransConsumer.ConsumerArgs<>();
            consumerArgs.userArgs = this.userArgs;
            consumerArgs.readCallback = this.readCallback;
            consumerArgs.handle = this.handle;
            consumerArgs.readAction = ACTION_READ;
            consumerArgs.consumerId = i;

            Thread consumer = new Thread(
                    new LatencyBenchmarkThreadTransConsumer<T>(consumerArgs));
            consumers.add(consumer);
        }

        // prepare producers
        ArrayList<Thread> producers = new ArrayList<>();
        for (int i = 0; i < this.config.getProducer(); i++) {
            LatencyBenchmarkThreadTransProducer.ProducerArgs<T> producerArgs =
                    new LatencyBenchmarkThreadTransProducer.ProducerArgs<>();
            producerArgs.userArgs = this.userArgs;
            producerArgs.writeCallback = this.writeCallback;
            producerArgs.handle = this.handle;
            producerArgs.config = this.config;
            producerArgs.writeBegAction = ACTION_WRITE_BEGIN;
            producerArgs.writeEndAction = ACTION_WRITE_END;
            producerArgs.producerId = i;
            producerArgs.messages = messages;

            Thread producer = new Thread(
                    new LatencyBenchmarkThreadTransProducer<T>(producerArgs));
            producers.add(producer);
        }

        // run consumers and producers
        for (Thread consumer : consumers) {
            consumer.start();
        }
        try {
            Thread.sleep(1);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        for (Thread producer : producers) {
            producer.start();
        }

        // write producers and consumers exit
        for (Thread producer : producers) {
            producer.join();
        }

        this.completedCallback.completed(this.config, this.userArgs);

        for (Thread consumer : consumers) {
            consumer.join();
        }
    }

    /**
     * generate latency benchmark report
     */
    public void genReport(String name) {
        String recordFileName = String.format("output/benchmark_%s_records", name);
        this.handle.genTimestampRecordsReport(recordFileName);

        String latencyFileName = String.format("output/benchmark_%s_latency", name);
        ArrayList<int[]> pairs = new ArrayList<>();
        pairs.add(new int[]{ACTION_WRITE_BEGIN, ACTION_WRITE_END});
        pairs.add(new int[]{ACTION_WRITE_BEGIN, ACTION_READ});
        this.handle.genLatencyReport(latencyFileName, pairs, this.config);
    }
}
