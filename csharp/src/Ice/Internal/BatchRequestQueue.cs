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
            new OutputStream(Protocol.currentProtocolEncoding, instance.defaultsAndOverrides().defaultFormat);
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

            // This is similar to a mutex lock in that the stream is only "locked" while marshaling.
            while (_batchStreamInUse)
            {
                Monitor.Wait(_mutex);
            }
            _batchStreamInUse = true;
            _batchStream.swap(os);
        }
    }

    internal void finishBatchRequest(OutputStream os, ObjectPrx proxy, string operation)
    {
        lock (_mutex)
        {
            // Bring the request stream back and become the owner so this thread's own auto-flush below can
            // re-enter swap(). swap() lets only _batchStreamOwner through while the stream is in use, and only
            // after passing that gate does it read the queue's bookkeeping; no other thread can touch the
            // queue, so the mutations below need no further synchronization.
            Debug.Assert(_batchStreamInUse);
            _batchStream.swap(os);
            _batchStreamOwner = Thread.CurrentThread;
        }

        try
        {
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
                _batchStreamOwner = null;
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
            // Only the thread that currently owns the in-use stream may bypass the wait, to run its own
            // auto-flush; every other caller waits for the in-progress batch request to finish.
            if (_batchStreamOwner != Thread.CurrentThread)
            {
                while (_batchStreamInUse)
                {
                    Monitor.Wait(_mutex);
                }
            }

            // Read the bookkeeping only after passing the gate above: finishBatchRequest mutates these scalars
            // without the lock while it owns the in-use stream, so reading _batchRequestNum before the wait
            // would race those writes.
            if (_batchRequestNum == 0)
            {
                compress = false;
                return 0;
            }

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

    // While finishBatchRequest holds the in-use stream, this is its thread: the only thread allowed to
    // re-enter swap() (for its own auto-flush). A null owner means no thread may flush the in-use stream, so
    // other threads wait for _batchStreamInUse to clear.
    private Thread _batchStreamOwner;

    private int _batchRequestNum;
    private int _batchMarker;
    private bool _batchCompress;
    private readonly BatchRequestI _request;
    private LocalException _exception;
    private readonly int _maxSize;
    private const int _udpOverhead = 20 + 8;
}
