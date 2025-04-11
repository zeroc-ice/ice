// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

namespace Ice.Internal;

internal sealed class BatchRequestI : BatchRequest
{
    internal BatchRequestI(BatchRequestQueue queue) => _queue = queue;

    internal void reset(ObjectPrx proxy, string operation, int size)
    {
        _proxy = proxy;
        _operation = operation;
        _size = size;
    }

    public void enqueue() => _queue.enqueueBatchRequest(_proxy);

    public ObjectPrx getProxy() => _proxy;

    public string getOperation() => _operation;

    public int getSize() => _size;

    private readonly BatchRequestQueue _queue;
    private ObjectPrx _proxy;
    private string _operation;
    private int _size;
}

internal sealed class BatchRequestQueue
{
    internal BatchRequestQueue(Instance instance, bool datagram)
    {
        InitializationData initData = instance.initializationData();
        _interceptor = initData.batchRequestInterceptor;
        _batchStreamInUse = false;
        _batchRequestNum = 0;
        _batchStream =
            new OutputStream(Ice.Util.currentProtocolEncoding, instance.defaultsAndOverrides().defaultFormat);
        _batchStream.writeBlob(Protocol.requestBatchHdr);
        _batchMarker = _batchStream.size();
        _request = new BatchRequestI(this);

        _maxSize = instance.batchAutoFlushSize();
        if (_maxSize > 0 && datagram)
        {
            int udpSndSize = initData.properties.getPropertyAsIntWithDefault(
                "Ice.UDP.SndSize",
                65535 - _udpOverhead);
            if (udpSndSize < _maxSize)
            {
                _maxSize = udpSndSize;
            }
        }
    }

    internal void prepareBatchRequest(OutputStream os)
    {
        lock (_mutex)
        {
            if (_exception != null)
            {
                throw _exception;
            }
            waitStreamInUse(false);
            _batchStreamInUse = true;
            _batchStream.swap(os);
        }
    }

    internal void finishBatchRequest(OutputStream os, ObjectPrx proxy, string operation)
    {
        //
        // No need for synchronization, no other threads are supposed
        // to modify the queue since we set _batchStreamInUse to true.
        //
        Debug.Assert(_batchStreamInUse);
        _batchStream.swap(os);

        try
        {
            _batchStreamCanFlush = true; // Allow flush to proceed even if the stream is marked in use.

            if (_maxSize > 0 && _batchStream.size() >= _maxSize)
            {
                _ = proxy.ice_flushBatchRequestsAsync(); // Auto flush
            }

            Debug.Assert(_batchMarker < _batchStream.size());
            if (_interceptor != null)
            {
                _request.reset(proxy, operation, _batchStream.size() - _batchMarker);
                _interceptor(_request, _batchRequestNum, _batchMarker);
            }
            else
            {
                bool? compress = ((ObjectPrxHelperBase)proxy).iceReference().getCompressOverride();
                if (compress is not null)
                {
                    _batchCompress |= compress.Value;
                }
                _batchMarker = _batchStream.size();
                ++_batchRequestNum;
            }
        }
        finally
        {
            lock (_mutex)
            {
                _batchStream.resize(_batchMarker);
                _batchStreamInUse = false;
                _batchStreamCanFlush = false;
                Monitor.PulseAll(_mutex);
            }
        }
    }

    internal void abortBatchRequest(OutputStream os)
    {
        lock (_mutex)
        {
            if (_batchStreamInUse)
            {
                _batchStream.swap(os);
                _batchStream.resize(_batchMarker);
                _batchStreamInUse = false;
                Monitor.PulseAll(_mutex);
            }
        }
    }

    internal int swap(OutputStream os, out bool compress)
    {
        lock (_mutex)
        {
            if (_batchRequestNum == 0)
            {
                compress = false;
                return 0;
            }

            waitStreamInUse(true);

            byte[] lastRequest = null;
            if (_batchMarker < _batchStream.size())
            {
                lastRequest = new byte[_batchStream.size() - _batchMarker];
                Buffer buffer = _batchStream.getBuffer();
                buffer.b.position(_batchMarker);
                buffer.b.get(lastRequest);
                _batchStream.resize(_batchMarker);
            }

            int requestNum = _batchRequestNum;
            compress = _batchCompress;
            _batchStream.swap(os);

            //
            // Reset the batch.
            //
            _batchRequestNum = 0;
            _batchCompress = false;
            _batchStream.writeBlob(Protocol.requestBatchHdr);
            _batchMarker = _batchStream.size();
            if (lastRequest != null)
            {
                _batchStream.writeBlob(lastRequest);
            }
            return requestNum;
        }
    }

    internal void destroy(LocalException ex)
    {
        lock (_mutex)
        {
            _exception = ex;
        }
    }

    private void waitStreamInUse(bool flush)
    {
        //
        // This is similar to a mutex lock in that the stream is
        // only "locked" while marshaling. As such we don't permit the wait
        // to be interrupted. Instead the interrupted status is saved and
        // restored.
        //
        while (_batchStreamInUse && !(flush && _batchStreamCanFlush))
        {
            Monitor.Wait(_mutex);
        }
    }

    internal void enqueueBatchRequest(ObjectPrx proxy)
    {
        Debug.Assert(_batchMarker < _batchStream.size());
        bool? compress = ((ObjectPrxHelperBase)proxy).iceReference().getCompressOverride();
        if (compress is not null)
        {
            _batchCompress |= compress.Value;
        }
        _batchMarker = _batchStream.size();
        ++_batchRequestNum;
    }

    private readonly object _mutex = new();

    private readonly System.Action<BatchRequest, int, int> _interceptor;
    private readonly OutputStream _batchStream;
    private bool _batchStreamInUse;
    private bool _batchStreamCanFlush;
    private int _batchRequestNum;
    private int _batchMarker;
    private bool _batchCompress;
    private readonly BatchRequestI _request;
    private LocalException _exception;
    private readonly int _maxSize;
    private const int _udpOverhead = 20 + 8;
}
