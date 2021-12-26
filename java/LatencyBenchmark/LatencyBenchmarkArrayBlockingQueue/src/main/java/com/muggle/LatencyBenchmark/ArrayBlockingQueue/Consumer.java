package com.muggle.latencybenchmark.arrayblockingqueue;

import com.muggle.latencybenchmark.common.TimestampRecord;

import java.util.concurrent.BlockingQueue;

public class Consumer implements Runnable {
    static class ConsumerArgs {
        public BlockingQueue<Message> queue;
        public int readAction;
        public TimestampRecord[] readRecords;
    }

    private ConsumerArgs args;

    public Consumer(ConsumerArgs args) {
        this.args = args;
    }

    @Override
    public void run() {
        BlockingQueue<Message> queue = this.args.queue;
        int readAction = this.args.readAction;
        TimestampRecord[] readRecords = this.args.readRecords;

        while (true) {
            try {
                Message msg = queue.take();
                if (msg.id == -1) {
                    break;
                }
                long nanoTime = System.nanoTime();

                TimestampRecord record = new TimestampRecord();
                record.setAction(readAction);
                record.setId(msg.id);
                record.setNsec(nanoTime);
                readRecords[msg.id] = record;
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
}
