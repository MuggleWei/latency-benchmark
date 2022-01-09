package com.muggle.latencybenchmark;


import com.lmax.disruptor.*;
import com.lmax.disruptor.dsl.Disruptor;
import com.lmax.disruptor.dsl.ProducerType;
import com.lmax.disruptor.util.DaemonThreadFactory;
import org.apache.commons.cli.ParseException;


public class LatencyBenchmarkDisruptor {

    /**
     * user arguments
     */
    public static class UserArgs {
        // empty
    }

    /**
     * producer writer
     */
    public static class DisruptorWrite implements LatencyBenchmarkThreadTrans.WriteCallback<UserArgs> {
        private final Disruptor<LatencyBenchmarkThreadTransMessage> disruptor;
        private final RingBuffer<LatencyBenchmarkThreadTransMessage> ringBuffer;

        public DisruptorWrite(Disruptor<LatencyBenchmarkThreadTransMessage> disruptor) {
            this.disruptor = disruptor;
            this.ringBuffer = disruptor.getRingBuffer();
        }

        @Override
        public boolean write(
                UserArgs userArgs,
                LatencyBenchmarkThreadTransMessage msg) {
            long sequence = this.ringBuffer.next();
            LatencyBenchmarkThreadTransMessage eventMsg = this.ringBuffer.get(sequence);
            eventMsg.id = msg.id;
            ringBuffer.publish(sequence);
            return true;
        }
    }

    /**
     * consumer
     */
    public static class DisruptorConsumer implements EventHandler<LatencyBenchmarkThreadTransMessage> {
        private LatencyBenchmarkRecord[] readRecords;
        private int action;

        public DisruptorConsumer(LatencyBenchmarkRecord[] readRecords, int action) {
            this.readRecords = readRecords;
            this.action = action;
        }

        @Override
        public void onEvent(LatencyBenchmarkThreadTransMessage msg, long l, boolean b) throws Exception {
            if (msg.id == -1) {
                return;
            }
            long nanoTime = System.nanoTime();

            LatencyBenchmarkRecord record = new LatencyBenchmarkRecord();
            record.setAction(this.action);
            record.setId(msg.id);
            record.setNsec(nanoTime);
            readRecords[msg.id] = record;
        }
    }

    public void run(String[] args) throws InterruptedException, ParseException {
        System.out.println("run LatencyBenchmarkDisruptor");

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
        WaitStrategy[] waitStrategies = new WaitStrategy[] {
                new BlockingWaitStrategy(),
                new SleepingWaitStrategy(),
                new YieldingWaitStrategy(),
                new BusySpinWaitStrategy(),
        };

        for (int consumerNum : consumerNums) {
            for (int producerNum : producerNums) {
                for (WaitStrategy waitStrategy : waitStrategies) {
                    config.setProducer(producerNum);
                    config.setConsumer(0);

                    ProducerType producerType = ProducerType.MULTI;
                    if (producerNum == 1) {
                        producerType = ProducerType.SINGLE;
                    }

                    Disruptor<LatencyBenchmarkThreadTransMessage> disruptor =
                            new Disruptor<>(
                                    LatencyBenchmarkThreadTransMessage::new,
                                    config.getCapacity(), DaemonThreadFactory.INSTANCE,
                                    producerType, waitStrategy);

                    LatencyBenchmarkThreadTrans<UserArgs> benchmark =
                            new LatencyBenchmarkThreadTrans<>(
                                    config,
                                    null,
                                    new DisruptorWrite(disruptor),
                                    null,
                                    null);

                    // set consumer
                    int readAction = LatencyBenchmarkThreadTrans.ACTION_READ;
                    for (int i = 0; i < consumerNum; i++) {
                        disruptor.handleEventsWith(new DisruptorConsumer(
                                benchmark.getActionTimestampRecords(readAction), readAction
                        ));
                    }
                    disruptor.start();

                    // run benchmark
                    System.out.println(String.format("run Disruptor - %s %d writer and %d reader",
                            waitStrategy.getClass().getName(), config.getProducer(), consumerNum));
                    benchmark.run();

                    // wait consumer completed
                    Thread.sleep(1000);

                    config.setConsumer(consumerNum);
                    benchmark.genReport(String.format("Disruptor_%s_%d_%d",
                            waitStrategy.getClass().getName(), config.getProducer(), config.getConsumer()));
                }
            }
        }
    }

    public static void main(String[] args) {
        LatencyBenchmarkDisruptor latencyBenchmark = new LatencyBenchmarkDisruptor();
        try {
            latencyBenchmark.run(args);
        } catch (InterruptedException | ParseException e) {
            e.printStackTrace();
        }
    }
}
