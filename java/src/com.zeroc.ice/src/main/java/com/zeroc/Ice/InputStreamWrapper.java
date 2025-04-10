// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.io.IOException;
import java.io.InputStream;
import java.nio.Buffer;
import java.nio.ByteBuffer;

//
// Class to provide a java.io.InputStream on top of a ByteBuffer.
// We use this to deserialize arbitrary Java serializable classes from
// a Slice byte sequence. This class is a wrapper around a Buffer
// that passes all methods through.
//

class InputStreamWrapper extends InputStream {
    public InputStreamWrapper(int size, ByteBuffer buf) {
        _buf = buf;
        _markPos = 0;
    }

    @Override
    public int read() throws IOException {
        try {
            return _buf.get();
        } catch (Exception ex) {
            throw new IOException(ex.toString());
        }
    }

    @Override
    public int read(byte[] b) throws IOException {
        return read(b, 0, b.length);
    }

    @Override
    public int read(byte[] b, int offset, int count) throws IOException {
        try {
            _buf.get(b, offset, count);
        } catch (Exception ex) {
            throw new IOException(ex.toString());
        }
        return count;
    }

    @Override
    public int available() {
        return _buf.remaining();
    }

    @Override
    public void mark(int readlimit) {
        _markPos = _buf.position();
    }

    @Override
    public void reset() throws IOException {
        // Cast to java.nio.Buffer to avoid incompatible covariant
        // return type used in Java 9 java.nio.ByteBuffer
        ((Buffer) _buf).position(_markPos);
    }

    @Override
    public boolean markSupported() {
        return true;
    }

    @Override
    public void close() throws IOException {}

    private final ByteBuffer _buf;
    private int _markPos;
}
