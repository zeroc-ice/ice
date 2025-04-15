// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

final class FileLoggerI extends LoggerI {
    private final FileOutputStream _out;

    FileLoggerI(String prefix, String file) {
        super(prefix);

        if (file.isEmpty()) {
            throw new FileException("FileLogger: file name is empty");
        }

        try {
            _out = new FileOutputStream(new File(file), true);
        } catch (FileNotFoundException ex) {
            throw new FileException("FileLogger: cannot open '" + file + "': file not found", ex);
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
        } catch (IOException ex) {}
    }

    @Override
    protected void writeMessage(String message) {
        try {
            _out.write(message.getBytes());
        } catch (IOException ex) {}
    }
}
