package com.muggle.latencybenchmark.arrayblockingqueue;

import com.muggle.latencybenchmark.common.LatencyBenchmarkCommon;
import com.muggle.latencybenchmark.common.LatencyBenchmarkConfig;
import org.apache.commons.cli.ParseException;

import java.util.ArrayList;
import java.util.concurrent.ArrayBlockingQueue;

public class LatencyBenchmarkArrayBlockingQueue {
    private static final int ACTION_WRITE_BEG = 0;
    private static final int ACTION_WRITE_END = 1;
    private static final int ACTION_READ = 2;

    private LatencyBenchmarkConfig config = new LatencyBenchmarkConfig();
    private LatencyBenchmarkCommon latencyBenchmark;

    public void run(String[] args) throws ParseException, InterruptedException {
        System.out.println("run LatencyBenchmarkArrayBlockingQueue");

        this.config.parseCommandLine(args);
        this.config.output();

        int recordCount =
                this.config.getTotalRounds() * this.config.getRecordPerRound() * this.config.getProducer();
        this.latencyBenchmark = new LatencyBenchmarkCommon(recordCount);
        this.latencyBenchmark.addAction(ACTION_WRITE_BEG, "write_begin");
        this.latencyBenchmark.addAction(ACTION_WRITE_END, "write_end");
        this.latencyBenchmark.addAction(ACTION_READ, "read");

        // run latency benchmark
        this.run();

        // generate report
        this.latencyBenchmark.genTimestampRecordsReport(
                "output/lb_records_array_blocking_queue.csv");

        ArrayList<int[]> pairs = new ArrayList<>();
        pairs.add(new int[]{ACTION_WRITE_BEG, ACTION_WRITE_END});
        pairs.add(new int[]{ACTION_WRITE_BEG, ACTION_READ});
        this.latencyBenchmark.genLatencyReport(
                "output/latency_array_blocking_queue.csv",
                pairs,
                this.config);
    }

    public void run() throws InterruptedException {
        ArrayBlockingQueue<Message> queue = new ArrayBlockingQueue<>(this.config.getCapacity());

        // prepare all message
        int total = this.latencyBenchmark.getRecordCount();
        Message[] messages = new Message[total];
        for (int i = 0; i < total; ++i) {
            messages[i] = new Message(i);
        }

        ArrayList<Thread> consumers = new ArrayList<>();
        for (int i = 0; i < this.config.getConsumer(); i++) {
            Consumer.ConsumerArgs consumerArgs = new Consumer.ConsumerArgs();
            consumerArgs.queue = queue;
            consumerArgs.readAction = ACTION_READ;
            consumerArgs.readRecords = this.latencyBenchmark.getActionTimestampRecords(ACTION_READ);

            Thread consumer = new Thread(new Consumer(consumerArgs));
            consumers.add(consumer);
        }

        // prepare producers
        ArrayList<Thread> producers = new ArrayList<>();
        for (int i = 0; i < this.config.getProducer(); i++) {
            Producer.ProducerArgs producerArgs = new Producer.ProducerArgs();
            producerArgs.queue = queue;
            producerArgs.writeBegAction = ACTION_WRITE_BEG;
            producerArgs.writeEndAction = ACTION_WRITE_END;
            producerArgs.writeBegRecords = this.latencyBenchmark.getActionTimestampRecords(ACTION_WRITE_BEG);
            producerArgs.writeEndRecords = this.latencyBenchmark.getActionTimestampRecords(ACTION_WRITE_END);
            producerArgs.rounds = this.config.getTotalRounds();
            producerArgs.intervalBetweenRound = this.config.getIntervalBetweenRound();
            producerArgs.recordPerRounds = this.config.getRecordPerRound();
            producerArgs.producerIdx = i;
            producerArgs.messages = messages;

            Thread producer = new Thread(new Producer(producerArgs));
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
        for (Thread consumer : consumers) {
            queue.put(new Message(-1));
        }
        for (Thread consumer : consumers) {
            consumer.join();
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
