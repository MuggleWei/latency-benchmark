package com.muggle.latencybenchmark;

public class LatencyBenchmarkThreadTransConsumer<T> implements Runnable {
    static class ConsumerArgs<T> {
        public T userArgs;
        public LatencyBenchmarkThreadTrans.ReadCallback<T> readCallback;
        public LatencyBenchmarkHandle handle;
        public int readAction;
        public int consumerId;
        public int totalRead;
    }

    private ConsumerArgs<T> args;

    public LatencyBenchmarkThreadTransConsumer(ConsumerArgs<T> args) {
        this.args = args;
    }

    @Override
    public void run() {
        T userArgs = this.args.userArgs;
        LatencyBenchmarkThreadTrans.ReadCallback<T> readCallback = this.args.readCallback;
        LatencyBenchmarkRecord[] readRecords = this.args.handle.getActionTimestampRecords(this.args.readAction);
        int readAction = this.args.readAction;

        this.args.totalRead = 0;
        while (true) {
            LatencyBenchmarkThreadTransMessage msg = null;
            try {
                msg = readCallback.read(userArgs);
            } catch (InterruptedException e) {
                e.printStackTrace();
                continue;
            }

            if (msg.id == -1) {
                break;
            }
            long nanoTime = System.nanoTime();

            LatencyBenchmarkRecord record = new LatencyBenchmarkRecord();
            record.setAction(readAction);
            record.setId(msg.id);
            record.setNsec(nanoTime);
            readRecords[msg.id] = record;

            this.args.totalRead++;
        }

        System.out.println(String.format("consumer[%d] completed, total read record: %d",
                this.args.consumerId, this.args.totalRead));
    }
}
