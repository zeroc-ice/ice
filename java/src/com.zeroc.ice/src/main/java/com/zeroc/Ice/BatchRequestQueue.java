// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.Optional;

class BatchRequestQueue {
    class BatchRequestI implements BatchRequest {
        public void reset(ObjectPrx proxy, String operation, int size) {
            _proxy = proxy;
            _operation = operation;
            _size = size;
        }

        @Override
        public void enqueue() {
            enqueueBatchRequest(_proxy);
        }

        @Override
        public ObjectPrx getProxy() {
            return _proxy;
        }

        @Override
        public String getOperation() {
            return _operation;
        }

        @Override
        public int getSize() {
            return _size;
        }

        private ObjectPrx _proxy;
        private String _operation;
        private int _size;
    }

    public BatchRequestQueue(Instance instance, boolean datagram) {
        InitializationData initData = instance.initializationData();
        _interceptor = initData.batchRequestInterceptor;
        _batchStreamInUse = false;
        _batchRequestNum = 0;
        _batchStream =
            new OutputStream(
                Protocol.currentProtocolEncoding,
                instance.defaultsAndOverrides().defaultFormat,
                instance.cacheMessageBuffers() > 1);
        _batchStream.writeBlob(Protocol.requestBatchHdr);
        _batchMarker = _batchStream.size();
        _batchCompress = false;
        _request = new BatchRequestI();

        _maxSize = instance.batchAutoFlushSize();
        if (_maxSize > 0 && datagram) {
            int udpSndSize = initData.properties.getPropertyAsIntWithDefault("Ice.UDP.SndSize", 65535 - _udpOverhead);
            if (udpSndSize < _maxSize) {
                _maxSize = udpSndSize;
            }
        }
    }

    public synchronized void prepareBatchRequest(OutputStream os) {
        if (_exception != null) {
            throw (LocalException) _exception.fillInStackTrace();
        }

        waitStreamInUse(false);
        _batchStreamInUse = true;
        _batchStream.swap(os);
    }

    public void finishBatchRequest(OutputStream os, ObjectPrx proxy, String operation) {
        // No need for synchronization, no other threads are supposed to modify the queue since we
        // set _batchStreamInUse to true.
        assert _batchStreamInUse;
        _batchStream.swap(os);

        try {
            // Allow flush to proceed even if the stream is marked in use.
            _batchStreamCanFlush = true;

            if (_maxSize > 0 && _batchStream.size() >= _maxSize) {
                proxy.ice_flushBatchRequestsAsync(); // Auto flush
            }

            assert (_batchMarker < _batchStream.size());
            if (_interceptor != null) {
                _request.reset(proxy, operation, _batchStream.size() - _batchMarker);
                _interceptor.enqueue(_request, _batchRequestNum, _batchMarker);
            } else {
                Optional<Boolean> compress = proxy._getReference().getCompressOverride();
                if (compress.isPresent()) {
                    _batchCompress |= compress.get();
                }
                _batchMarker = _batchStream.size();
                ++_batchRequestNum;
            }
        } finally {
            synchronized (this) {
                _batchStream.resize(_batchMarker);
                _batchStreamInUse = false;
                _batchStreamCanFlush = false;
                notifyAll();
            }
        }
    }

    public synchronized void abortBatchRequest(OutputStream os) {
        if (_batchStreamInUse) {
            _batchStream.swap(os);
            _batchStream.resize(_batchMarker);
            _batchStreamInUse = false;
            notifyAll();
        }
    }

    public class SwapResult {
        public int batchRequestNum;
        public boolean compress;
    }

    public synchronized SwapResult swap(OutputStream os) {
        if (_batchRequestNum == 0) {
            return null;
        }

        waitStreamInUse(true);

        byte[] lastRequest = null;
        if (_batchMarker < _batchStream.size()) {
            lastRequest = new byte[_batchStream.size() - _batchMarker];
            Buffer buffer = _batchStream.getBuffer();
            buffer.position(_batchMarker);
            buffer.b.get(lastRequest);
            _batchStream.resize(_batchMarker);
        }

        SwapResult result = new SwapResult();
        result.batchRequestNum = _batchRequestNum;
        result.compress = _batchCompress;
        _batchStream.swap(os);

        // Reset the batch.
        _batchRequestNum = 0;
        _batchCompress = false;
        _batchStream.writeBlob(Protocol.requestBatchHdr);
        _batchMarker = _batchStream.size();
        if (lastRequest != null) {
            _batchStream.writeBlob(lastRequest);
        }
        return result;
    }

    public synchronized void destroy(LocalException ex) {
        _exception = ex;
    }

    private void waitStreamInUse(boolean flush) {
        // This is similar to a mutex lock in that the stream is only "locked" while marshaling.
        // As such we don't permit the wait to be interrupted.
        // Instead the interrupted status is saved and restored.
        boolean interrupted = false;
        while (_batchStreamInUse && !(flush && _batchStreamCanFlush)) {
            try {
                wait();
            } catch (InterruptedException ex) {
                interrupted = true;
            }
        }

        // Restore the interrupted flag if we were interrupted.
        if (interrupted) {
            Thread.currentThread().interrupt();
        }
    }

    private void enqueueBatchRequest(ObjectPrx proxy) {
        assert (_batchMarker < _batchStream.size());
        Optional<Boolean> compress = proxy._getReference().getCompressOverride();
        if (compress.isPresent()) {
            _batchCompress |= compress.get();
        }
        _batchMarker = _batchStream.size();
        ++_batchRequestNum;
    }

    private final BatchRequestInterceptor _interceptor;
    private final OutputStream _batchStream;
    private boolean _batchStreamInUse;
    private boolean _batchStreamCanFlush;
    private int _batchRequestNum;
    private int _batchMarker;
    private boolean _batchCompress;
    private final BatchRequestI _request;
    private LocalException _exception;
    private int _maxSize;

    private static final int _udpOverhead = 20 + 8;
}
