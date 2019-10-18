package com.muggle;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.atomic.AtomicInteger;

public class BenchmarkArrayBlockingQueue {
    static class Message {
        public int id;
        public long[] ts;

        public Message(int id) {
            this.id = id;
            ts = new long[8];
        }
    }

    static class Producer implements Runnable {
        private int producerIdx;
        private BlockingQueue<Message> queue;
        private Message[] messages;
        private int totalMsg;
        private int cntInterval;
        private int intervalMs;
        private AtomicInteger idx;

        public Producer(
                int producerIdx, BlockingQueue<Message> queue, Message[] messages, AtomicInteger idx,
                int totalMsg, int cntInterval, int intervalMs) {
            this.producerIdx = producerIdx;
            this.queue = queue;
            this.messages = messages;
            this.totalMsg = totalMsg;
            this.idx = idx;
            this.cntInterval = cntInterval;
            this.intervalMs = intervalMs;
        }

        public void run() {
            int curIdx = 0;
            int cnt = 0;
            long startTs = System.nanoTime();
            try {
                while (true) {
                    curIdx = idx.getAndIncrement();
                    if (curIdx < this.totalMsg) {
                        Message msg = messages[curIdx];
                        msg.ts[0] = System.nanoTime();
                        queue.put(msg);
                        msg.ts[1] = System.nanoTime();
                    } else {
                        break;
                    }

                    if (curIdx == cntInterval && intervalMs > 0) {
                        Thread.sleep(intervalMs);
                    }
                    ++cnt;
                }
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            long endTs = System.nanoTime();
            long elapsedNs = endTs - startTs;
            System.out.printf("producer[%d] write %d messages, total use: %dns(%.3fs)\n",
                    producerIdx, cnt, elapsedNs, elapsedNs / 1000000000.0);
        }
    }

    static class Consumer implements Runnable {
        private BlockingQueue<Message> queue;

        public Consumer(BlockingQueue<Message> queue) {
            this.queue = queue;
        }

        public void run() {
            while (true) {
                try {
                    Message msg = queue.take();
                    if (msg.id == -1) {
                        break;
                    }
                    msg.ts[2] = System.nanoTime();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    public static void print(String name, int step, long[] arr) {
        System.out.printf("%s: ", name);
        for (int i = 0; i < 100; i += step) {
            int idx = (int) ((i / 100.0) * arr.length);
            System.out.printf("%d ", arr[idx]);
        }
        System.out.printf("%d\n", arr[arr.length - 1]);
    }

    public static void producer_consumer(
            int capacity, int total, int cntProducer, int cntConsumer,
            int cntInterval, int intervalMs) throws InterruptedException {
        ArrayBlockingQueue<Message> queue = new ArrayBlockingQueue<>(capacity);
        Message[] messages = new Message[total];
        for (int i = 0; i < total; ++i) {
            messages[i] = new Message(i);
        }

        ArrayList<Thread> producers = new ArrayList<>();
        ArrayList<Thread> consumers = new ArrayList<>();
        AtomicInteger fetchIdx = new AtomicInteger();
        for (int i = 0; i < cntConsumer; i++) {
            Thread consumer = new Thread(new Consumer(queue));
            consumers.add(consumer);
        }
        for (int i = 0; i < cntProducer; i++) {
            Thread producer = new Thread(
                    new Producer(i, queue, messages, fetchIdx, total, cntInterval, intervalMs));
            producers.add(producer);
        }

        for (Thread consumer : consumers) {
            consumer.start();
        }
        Thread.sleep(5);
        for (Thread producer : producers) {
            producer.start();
        }

        for (Thread producer : producers) {
            producer.join();
        }
        for (Thread consumer : consumers) {
            queue.put(new Message(-1));
        }
        for (Thread consumer : consumers) {
            consumer.join();
        }

        long[] elapsed_w = new long[total];
        long[] elapsed_wr = new long[total];
        for (int i = 0; i < total; ++i) {
            Message msg = messages[i];
            elapsed_w[i] = msg.ts[1] - msg.ts[0];
            elapsed_wr[i] = msg.ts[2] - msg.ts[0];
        }

        print("write", 10, elapsed_w);
        Arrays.sort(elapsed_w);
        print("sorted write", 10, elapsed_w);

        print("wr", 10, elapsed_wr);
        Arrays.sort(elapsed_wr);
        print("sorted wr", 10, elapsed_wr);

        long elapsed_ns = messages[total - 1].ts[1] - messages[0].ts[0];
        System.out.printf("total write: %dns(%.3fs), avg: %d\n",
                elapsed_ns, elapsed_ns / 1000000000.0, elapsed_ns / total);
        elapsed_ns = messages[total - 1].ts[2] - messages[0].ts[0];
        System.out.printf("total wr: %dns(%.3fs), avg: %d\n",
                elapsed_ns, elapsed_ns / 1000000000.0, elapsed_ns / total);
    }

    public static void main(String[] args) throws InterruptedException {
        int cntInterval = 1000;
        int intervalMs = 0;
        int capacity = 1024 * 64;
        int total = 10000 * 100;
        int hc = Runtime.getRuntime().availableProcessors();
        hc /= 2;
        if (hc <= 1) {
            hc = 2;
        }

        // mul producer 1 consumer
        producer_consumer(capacity, total, hc, 1, cntInterval, intervalMs);
    }
}
