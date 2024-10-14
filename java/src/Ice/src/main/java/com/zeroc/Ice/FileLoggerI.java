// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

final class FileLoggerI extends LoggerI {
    private final java.io.FileOutputStream _out;

    FileLoggerI(String prefix, String file) {
        super(prefix);

        if (file.isEmpty()) {
            throw new InitializationException("FileLogger: file name is empty");
        }

        try {
            _out = new java.io.FileOutputStream(new java.io.File(file), true);
        } catch (java.io.FileNotFoundException ex) {
            throw new InitializationException("FileLogger: cannot open " + file);
        }
    }

    @Override
    public Logger cloneWithPrefix(String prefix) {
        throw new UnsupportedOperationException(
                "cloneWithPrefix is not supported on a file logger");
    }

    void destroy() {
        try {
            _out.close();
        } catch (java.io.IOException ex) {
        }
    }

    @Override
    protected void writeMessage(String message) {
        try {
            _out.write(message.getBytes());
        } catch (java.io.IOException ex) {
        }
    }
}
