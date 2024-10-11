//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

class LoggerI implements Logger {
    private final String _prefix;
    private final String _formattedPrefix;
    private final String _lineSeparator;
    private final java.text.DateFormat _date;
    private final java.text.SimpleDateFormat _time;

    LoggerI(String prefix) {
        _prefix = prefix;

        if (!prefix.isEmpty()) {
            _formattedPrefix = prefix + ": ";
        } else {
            _formattedPrefix = "";
        }

        _lineSeparator = System.getProperty("line.separator");
        _date = java.text.DateFormat.getDateInstance(java.text.DateFormat.SHORT);
        _time = new java.text.SimpleDateFormat(" HH:mm:ss:SSS");
    }

    @Override
    public void print(String message) {
        StringBuilder s = new StringBuilder(256);
        s.append(message);
        write(s, false);
    }

    @Override
    public void trace(String category, String message) {
        StringBuilder s = new StringBuilder(256);
        s.append("-- ");
        synchronized (this) {
            java.util.Date date = new java.util.Date();
            s.append(_date.format(date));
            s.append(_time.format(date));
        }
        s.append(' ');
        s.append(_formattedPrefix);
        s.append(category);
        s.append(": ");
        s.append(message);
        write(s, true);
    }

    @Override
    public void warning(String message) {
        StringBuilder s = new StringBuilder(256);
        s.append("-! ");
        synchronized (this) {
            s.append(_date.format(new java.util.Date()));
            s.append(_time.format(new java.util.Date()));
        }
        s.append(' ');
        s.append(_formattedPrefix);
        s.append("warning: ");
        s.append(Thread.currentThread().getName());
        s.append(": ");
        s.append(message);
        write(s, true);
    }

    @Override
    public void error(String message) {
        StringBuilder s = new StringBuilder(256);
        s.append("!! ");
        synchronized (this) {
            s.append(_date.format(new java.util.Date()));
            s.append(_time.format(new java.util.Date()));
        }
        s.append(' ');
        s.append(_formattedPrefix);
        s.append("error: ");
        s.append(Thread.currentThread().getName());
        s.append(": ");
        s.append(message);
        write(s, true);
    }

    @Override
    public String getPrefix() {
        return _prefix;
    }

    @Override
    public Logger cloneWithPrefix(String prefix) {
        return new LoggerI(prefix);
    }

    // Writes the message to the output stream.
    protected void writeMessage(String message) {
        System.err.print(message);
    }

    private void write(StringBuilder message, boolean indent) {
        if (indent) {
            int idx = 0;
            while ((idx = message.indexOf("\n", idx)) != -1) {
                message.insert(idx + 1, "   ");
                ++idx;
            }
        }
        message.append(_lineSeparator);
        writeMessage(message.toString());
    }
}
