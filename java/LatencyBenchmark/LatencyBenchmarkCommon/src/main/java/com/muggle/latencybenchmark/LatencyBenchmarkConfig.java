package com.muggle.latencybenchmark;

import org.apache.commons.cli.*;

/**
 * benchmark config
 */
public class LatencyBenchmarkConfig {

    /**
     * total rounds in benchmark
     */
    private int totalRounds = 100;

    /**
     * how many times run in a round
     */
    private int recordPerRound = 10;

    /**
     * round interval in milliseconds
     */
    private int intervalBetweenRound = 1;

    /**
     * [optional] usually refers to buffer capacity
     */
    private int capacity = 1024;

    /**
     * [optional] data block size
     */
    private int blockSize = 1024;

    /**
     * [optional] number of producer
     */
    private int producer = 1;

    /**
     * [optional] number of consumer
     */
    private int consumer = 1;

    /**
     * sampling step in report
     */
    private int reportStep = 10;

    public int getTotalRounds() {
        return totalRounds;
    }

    public void setTotalRounds(int totalRounds) {
        this.totalRounds = totalRounds;
    }

    public int getRecordPerRound() {
        return recordPerRound;
    }

    public void setRecordPerRound(int recordPerRound) {
        this.recordPerRound = recordPerRound;
    }

    public int getIntervalBetweenRound() {
        return intervalBetweenRound;
    }

    public void setIntervalBetweenRound(int intervalBetweenRound) {
        this.intervalBetweenRound = intervalBetweenRound;
    }

    public int getCapacity() {
        return capacity;
    }

    public void setCapacity(int capacity) {
        this.capacity = capacity;
    }

    public int getBlockSize() {
        return blockSize;
    }

    public void setBlockSize(int blockSize) {
        this.blockSize = blockSize;
    }

    public int getProducer() {
        return producer;
    }

    public void setProducer(int producer) {
        this.producer = producer;
    }

    public int getConsumer() {
        return consumer;
    }

    public void setConsumer(int consumer) {
        this.consumer = consumer;
    }

    public int getReportStep() {
        return reportStep;
    }

    public void setReportStep(int reportStep) {
        this.reportStep = reportStep;
    }

    public void parseCommandLine(String[] args) throws ParseException {
        Options options = new Options();
        options.addOption(new Option("r", "rounds", true, "total rounds"));
        options.addOption(new Option("i", "interval", true, "interval millisecond between round"));
        options.addOption(new Option("m", "records", true, "record per round"));
        options.addOption(new Option("p", "producer", true, "number of producer"));
        options.addOption(new Option("c", "consumer", true, "number of consumer"));
        options.addOption(new Option("s", "capacity", true, "capacity size"));
        options.addOption(new Option("b", "block_size", true, "data block size"));
        options.addOption(new Option("f", "report_step", true, "report step"));

        CommandLineParser parser = new DefaultParser();
        CommandLine cmd = cmd = parser.parse(options, args);

        if (cmd.hasOption("r")) {
            this.totalRounds = Integer.parseInt(cmd.getOptionValue("r"));
        }
        if (cmd.hasOption("m")) {
            this.recordPerRound = Integer.parseInt(cmd.getOptionValue("m"));
        }
        if (cmd.hasOption("i")) {
            this.intervalBetweenRound = Integer.parseInt(cmd.getOptionValue("i"));
        }
        if (cmd.hasOption("s")) {
            this.capacity = Integer.parseInt(cmd.getOptionValue("s"));
        }
        if (cmd.hasOption("b")) {
            this.blockSize = Integer.parseInt(cmd.getOptionValue("b"));
        }
        if (cmd.hasOption("p")) {
            this.producer = Integer.parseInt(cmd.getOptionValue("p"));
        }
        if (cmd.hasOption("c")) {
            this.consumer = Integer.parseInt(cmd.getOptionValue("c"));
        }
        if (cmd.hasOption("f")) {
            this.reportStep = Integer.parseInt(cmd.getOptionValue("f"));
        }

        if (this.producer == 0) {
            int hc = Runtime.getRuntime().availableProcessors();
            hc /= 2;
            if (hc <= 1) {
                hc = 2;
            }
            this.producer = hc;
        }
    }

    public void output() {
        System.out.println(String.format("round: %d", this.getTotalRounds()));
        System.out.println(String.format("record per round: %d", this.getRecordPerRound()));
        System.out.println(String.format("capacity size: %d", this.getCapacity()));
        System.out.println(String.format("data block size: %d", this.getBlockSize()));
        System.out.println(String.format("interval ms between round: %d", this.getIntervalBetweenRound()));
        System.out.println(String.format("number of producer: %d", this.getProducer()));
        System.out.println(String.format("number of consumer: %d", this.getConsumer()));
        System.out.println(String.format("report step: %d", this.getReportStep()));
    }
}
