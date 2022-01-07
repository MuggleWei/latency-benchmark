package com.muggle.latencybenchmark.common;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class LatencyBenchmarkHandle {
    /**
     * expect total record count
     */
    private int recordCount;

    /**
     * action id to name map
     */
    private HashMap<Integer, String> actionMap;

    /**
     * action id to timestamp records map
     */
    private HashMap<Integer, LatencyBenchmarkRecord[]> actionTimeRecords;

    /**
     * constructor
     */
    public LatencyBenchmarkHandle(int recordCount) {
        this.recordCount = recordCount;

        this.actionMap = new HashMap<>();
        this.actionTimeRecords = new HashMap<>();
    }

    public int getRecordCount() {
        return recordCount;
    }

    /**
     * add action id to name map
     *
     * @param id         action id
     * @param actionName action name
     */
    public void addAction(int id, String actionName) {
        this.actionMap.put(id, actionName);

        LatencyBenchmarkRecord[] records = new LatencyBenchmarkRecord[this.recordCount];
        this.actionTimeRecords.put(id, records);
    }

    /**
     * get action name
     *
     * @param id action id
     * @return action name
     */
    public String getActionName(int id) {
        return this.actionMap.get(id);
    }

    /**
     * get action timestamp records
     *
     * @param action action id
     * @return timestamp records array
     */
    public LatencyBenchmarkRecord[] getActionTimestampRecords(int action) {
        return this.actionTimeRecords.get(action);
    }

    /**
     * generate timestamp records report
     *
     * @param filepath report output filepath
     */
    public void genTimestampRecordsReport(String filepath) {
        File filePath = new File(filepath);
        File parentDir = new File(filePath.getParent());
        if (!parentDir.exists()) {
            parentDir.mkdirs();
        }

        try (BufferedWriter out = new BufferedWriter(new FileWriter(filepath))) {
            genTimestampRecordsReport(out);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * generate timestamp records report
     *
     * @param out BufferedWriter
     */
    public void genTimestampRecordsReport(BufferedWriter out) throws IOException {
        for (int i = 0; i < this.recordCount; i++) {
            for (Map.Entry<Integer, LatencyBenchmarkRecord[]> entry : this.actionTimeRecords.entrySet()) {
                String actionName = this.getActionName(entry.getKey());
                LatencyBenchmarkRecord[] records = entry.getValue();
                out.write(String.format("%d,%s,%d,%d\n", i, actionName, records[i].getSec(), records[i].getNsec()));
            }
        }
    }

    /**
     * generate latency report
     *
     * @param filepath     report output filepath
     * @param latencyPairs latency pairs
     * @param config       latency benchmark config
     */
    public void genLatencyReport(
            String filepath, ArrayList<int[]> latencyPairs, LatencyBenchmarkConfig config) {
        File filePath = new File(filepath);
        File parentDir = new File(filePath.getParent());
        if (!parentDir.exists()) {
            parentDir.mkdirs();
        }

        try (BufferedWriter out = new BufferedWriter(new FileWriter(filepath))) {
            genLatencyReport(out, latencyPairs, config);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * generate latency report
     *
     * @param out          BufferedWriter
     * @param latencyPairs latency pairs
     * @param config       latency benchmark config
     */
    public void genLatencyReport(
            BufferedWriter out,
            ArrayList<int[]> latencyPairs,
            LatencyBenchmarkConfig config) throws IOException {
        // write head
        out.write("elapsed unit[ns]\n");
        out.write("case_name,rounds,record_per_round,interval_ms,capacity,producer,consumer,avg,");
        for (int i = 0; i < 100; i += config.getReportStep()) {
            out.write(String.format("%d,", i));
        }
        out.write("100\n");

        // write body
        for (int[] latencyPair : latencyPairs) {
            String idxCaseName = String.format("%s->%s by idx",
                    this.getActionName(latencyPair[0]),
                    this.getActionName(latencyPair[1]));
            genLatencyReportBody(out, latencyPair, config, idxCaseName, false);

            String elapsedCaseName = String.format("%s->%s by elapsed",
                    this.getActionName(latencyPair[0]),
                    this.getActionName(latencyPair[1]));
            genLatencyReportBody(out, latencyPair, config, elapsedCaseName, true);
        }
    }

    private void genLatencyReportBody(
            BufferedWriter out,
            int[] latencyPair,
            LatencyBenchmarkConfig config,
            String caseName,
            boolean sort) throws IOException {
        LatencyBenchmarkRecord[] begRecords = this.getActionTimestampRecords(latencyPair[0]);
        LatencyBenchmarkRecord[] endRecords = this.getActionTimestampRecords(latencyPair[1]);

        int cnt = begRecords.length;
        long[] elapsed = new long[cnt];
        long sum = 0;
        for (int i = 0; i < cnt; i++) {
            if (endRecords[i].getSec() == 0 && endRecords[i].getNsec() == 0) {
                elapsed[i] = Integer.MAX_VALUE;
            } else {
                elapsed[i]
                        = (endRecords[i].getSec() - begRecords[i].getSec()) * 1000000000
                        + endRecords[i].getNsec() - begRecords[i].getNsec();
                sum += elapsed[i];
            }
        }
        long avg = sum / cnt;

        if (sort) {
            Arrays.sort(elapsed);
        }

        // case_name,rounds,record_per_round,interval_ms,capacity,producer,consumer,avg
        out.write(String.format("%s,%d,%d,%d,%d,%d,%d,%d,",
                caseName,
                config.getTotalRounds(),
                config.getRecordPerRound(),
                config.getIntervalBetweenRound(),
                config.getCapacity(),
                config.getProducer(),
                config.getConsumer(),
                avg));

        // quantile
        for (int i = 0; i < 100; i += config.getReportStep()) {
            int pos = (int) ((i / 100.0) * cnt);
            if (elapsed[pos] == Integer.MAX_VALUE) {
                out.write("-");
            } else {
                out.write(String.format("%d,", elapsed[pos]));
            }
        }
        if (elapsed[cnt - 1] == Integer.MAX_VALUE) {
            out.write("-");
        } else {
            out.write(String.format("%d", elapsed[cnt - 1]));
        }
        out.write("\n");
    }
}
