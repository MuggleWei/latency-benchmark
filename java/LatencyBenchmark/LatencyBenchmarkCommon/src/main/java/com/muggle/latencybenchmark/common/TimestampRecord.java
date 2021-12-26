package com.muggle.latencybenchmark.common;

public class TimestampRecord {
    private int action;
    private int id;
    private long sec;
    private long nsec;

    public int getAction() {
        return action;
    }

    public void setAction(int action) {
        this.action = action;
    }

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public long getSec() {
        return sec;
    }

    public void setSec(long sec) {
        this.sec = sec;
    }

    public long getNsec() {
        return nsec;
    }

    public void setNsec(long nsec) {
        this.nsec = nsec;
    }
}
