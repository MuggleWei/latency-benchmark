package com.muggle.latencybenchmark;

public class LatencyBenchmarkThreadTransProducer<T> implements Runnable {
    static class ProducerArgs<T> {
        public T userArgs;
        public LatencyBenchmarkThreadTrans.WriteCallback<T> writeCallback;
        public LatencyBenchmarkHandle handle;
        public LatencyBenchmarkConfig config;
        public int writeBegAction;
        public int writeEndAction;
        public int producerId;
        public LatencyBenchmarkThreadTransMessage[] messages;
    }

    private ProducerArgs<T> args;

    public LatencyBenchmarkThreadTransProducer(ProducerArgs<T> args) {
        this.args = args;
    }

    @Override
    public void run() {
        T userArgs = this.args.userArgs;
        LatencyBenchmarkThreadTrans.WriteCallback<T> writeCallback = this.args.writeCallback;
        LatencyBenchmarkRecord[] writeBegRecords = this.args.handle.getActionTimestampRecords(this.args.writeBegAction);
        LatencyBenchmarkRecord[] writeEndRecords = this.args.handle.getActionTimestampRecords(this.args.writeEndAction);
        int writeBegAction = this.args.writeBegAction;
        int writeEndAction = this.args.writeEndAction;
        LatencyBenchmarkThreadTransMessage[] messages = this.args.messages;

        // get start message index
        LatencyBenchmarkConfig config = this.args.config;
        int count = config.getTotalRounds() * config.getRecordPerRound();
        int offset = this.args.producerId * count;
        for (int round = 0; round < config.getTotalRounds(); round++) {
            for (int i = 0; i < config.getRecordPerRound(); i++) {
                int id = offset + round * config.getRecordPerRound() + i;
                LatencyBenchmarkThreadTransMessage msg = messages[id];

                long nanoTimeWriteBeg = System.nanoTime();
                try {
                    writeCallback.write(userArgs, msg);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
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
            if (this.args.config.getIntervalBetweenRound() > 0) {
                try {
                    Thread.sleep(this.args.config.getIntervalBetweenRound());
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
    }
}
