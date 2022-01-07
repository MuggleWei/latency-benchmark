package com.muggle.latencybenchmark.arrayblockingqueue;

import com.muggle.latencybenchmark.common.LatencyBenchmarkRecord;

import java.util.concurrent.BlockingQueue;

public class Producer implements Runnable {
    static class ProducerArgs {
        public BlockingQueue<Message> queue;

        public int writeBegAction;
        public int writeEndAction;
        public LatencyBenchmarkRecord[] writeBegRecords;
        public LatencyBenchmarkRecord[] writeEndRecords;

        public int rounds;
        public int intervalBetweenRound;
        public int recordPerRounds;

        public int producerIdx;
        public Message[] messages;
    }

    private ProducerArgs args;

    public Producer(ProducerArgs args) {
        this.args = args;
    }

    @Override
    public void run() {
        BlockingQueue<Message> queue = this.args.queue;

        int writeBegAction = this.args.writeBegAction;
        LatencyBenchmarkRecord[] writeBegRecords = this.args.writeBegRecords;

        int writeEndAction = this.args.writeEndAction;
        LatencyBenchmarkRecord[] writeEndRecords = this.args.writeEndRecords;

        // get start message index
        int msgId = this.args.producerIdx * this.args.rounds * this.args.recordPerRounds;

        // loop put message
        try {
            for (int i = 0; i < this.args.rounds; i++) {
                for (int j = 0; j < this.args.recordPerRounds; j++) {
                    Message msg = this.args.messages[msgId++];

                    long nanoTimeWriteBeg = System.nanoTime();
                    queue.put(msg);
                    long nanoTimeWriteEnd = System.nanoTime();

                    LatencyBenchmarkRecord writeBegRecord = new LatencyBenchmarkRecord();
                    writeBegRecord.setAction(writeBegAction);
                    writeBegRecord.setId(msg.id);
                    writeBegRecord.setNsec(nanoTimeWriteBeg);
                    writeBegRecords[msg.id] = writeBegRecord;

                    LatencyBenchmarkRecord writeEndRecord = new LatencyBenchmarkRecord();
                    writeEndRecord.setAction(writeEndAction);
                    writeEndRecord.setId(msg.id);
                    writeEndRecord.setNsec(nanoTimeWriteEnd);
                    writeEndRecords[msg.id] = writeEndRecord;
                }

                // sleep between round
                if (this.args.intervalBetweenRound > 0) {
                    Thread.sleep(this.args.intervalBetweenRound);
                }
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
