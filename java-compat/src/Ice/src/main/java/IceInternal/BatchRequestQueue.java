//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public class BatchRequestQueue
{
    class BatchRequestI implements Ice.BatchRequest
    {
        public void reset(Ice.ObjectPrx proxy, String operation, int size)
        {
            _proxy = proxy;
            _operation = operation;
            _size = size;
        }

        @Override
        public void enqueue()
        {
            enqueueBatchRequest(_proxy);
        }

        @Override
        public Ice.ObjectPrx getProxy()
        {
            return _proxy;
        }

        @Override
        public String getOperation()
        {
            return _operation;
        }

        @Override
        public int getSize()
        {
            return _size;
        }

        private Ice.ObjectPrx _proxy;
        private String _operation;
        private int _size;
    }

    public
    BatchRequestQueue(Instance instance, boolean datagram)
    {
        Ice.InitializationData initData = instance.initializationData();
        _interceptor = initData.batchRequestInterceptor;
        _batchStreamInUse = false;
        _batchRequestNum = 0;
        _batchStream = new Ice.OutputStream(instance, Protocol.currentProtocolEncoding);
        _batchStream.writeBlob(Protocol.requestBatchHdr);
        _batchMarker = _batchStream.size();
        _batchCompress = false;
        _request = new BatchRequestI();

        _maxSize = instance.batchAutoFlushSize();
        if(_maxSize > 0 && datagram)
        {
            int udpSndSize = initData.properties.getPropertyAsIntWithDefault("Ice.UDP.SndSize", 65535 - _udpOverhead);
            if(udpSndSize < _maxSize)
            {
                _maxSize = udpSndSize;
            }
        }
    }

    synchronized public void
    prepareBatchRequest(Ice.OutputStream os)
    {
        if(_exception != null)
        {
            throw (Ice.LocalException)_exception.fillInStackTrace();
        }

        waitStreamInUse(false);
        _batchStreamInUse = true;
        _batchStream.swap(os);
    }

    public void
    finishBatchRequest(Ice.OutputStream os, Ice.ObjectPrx proxy, String operation)
    {
        synchronized(this)
        {
            //
            // Bring the request stream back and become the owner so this thread's own auto-flush below can
            // re-enter swap(). swap() lets only _batchStreamOwner through while the stream is in use, and only
            // after passing that gate does it read the queue's bookkeeping; no other thread can touch the queue,
            // so the mutations below need no further synchronization.
            //
            assert(_batchStreamInUse);
            _batchStream.swap(os);
            _batchStreamOwner = Thread.currentThread();
        }

        try
        {
            if(_maxSize > 0 && _batchStream.size() >= _maxSize)
            {
                proxy.begin_ice_flushBatchRequests(); // Auto flush
            }

            assert(_batchMarker < _batchStream.size());
            if(_interceptor != null)
            {
                _request.reset(proxy, operation, _batchStream.size() - _batchMarker);
                _interceptor.enqueue(_request, _batchRequestNum, _batchMarker);
            }
            else
            {
                Boolean compress = ((Ice.ObjectPrxHelperBase)proxy)._getReference().getCompressOverride();
                if(compress != null)
                {
                    _batchCompress |= compress.booleanValue();
                }
                _batchMarker = _batchStream.size();
                ++_batchRequestNum;
            }
        }
        finally
        {
            synchronized(this)
            {
                _batchStream.resize(_batchMarker);
                _batchStreamInUse = false;
                _batchStreamOwner = null;
                notifyAll();
            }
        }
    }

    synchronized public void
    abortBatchRequest(Ice.OutputStream os)
    {
        if(_batchStreamInUse)
        {
            _batchStream.swap(os);
            _batchStream.resize(_batchMarker);
            _batchStreamInUse = false;
            notifyAll();
        }
    }

    synchronized public int
    swap(Ice.OutputStream os, Ice.BooleanHolder compress)
    {
        waitStreamInUse(true);

        //
        // Read the bookkeeping only after waitStreamInUse: finishBatchRequest mutates it without the lock while it
        // owns the in-use stream, so reading _batchRequestNum before the wait would race those writes.
        //
        if(_batchRequestNum == 0)
        {
            return 0;
        }

        byte[] lastRequest = null;
        if(_batchMarker < _batchStream.size())
        {
            lastRequest = new byte[_batchStream.size() - _batchMarker];
            Buffer buffer = _batchStream.getBuffer();
            buffer.position(_batchMarker);
            buffer.b.get(lastRequest);
            _batchStream.resize(_batchMarker);
        }

        int requestNum = _batchRequestNum;
        if(compress != null)
        {
            compress.value = _batchCompress;
        }
        _batchStream.swap(os);

        //
        // Reset the batch.
        //
        _batchRequestNum = 0;
        _batchCompress = false;
        _batchStream.writeBlob(Protocol.requestBatchHdr);
        _batchMarker = _batchStream.size();
        if(lastRequest != null)
        {
            _batchStream.writeBlob(lastRequest);
        }
        return requestNum;
    }

    synchronized public void
    destroy(Ice.LocalException ex)
    {
        _exception = ex;
    }

    synchronized public boolean
    isEmpty()
    {
        return _batchStream.size() == Protocol.requestBatchHdr.length;
    }

    private void
    waitStreamInUse(boolean flush)
    {
        //
        // This is similar to a mutex lock in that the stream is
        // only "locked" while marshaling. As such we don't permit the wait
        // to be interrupted. Instead the interrupted status is saved and
        // restored.
        //
        // Only the thread that currently owns the in-use stream may bypass the wait to run its own auto-flush;
        // every other caller waits for the in-progress batch request to finish.
        //
        boolean interrupted = false;
        while(_batchStreamInUse && !(flush && _batchStreamOwner == Thread.currentThread()))
        {
            try
            {
                wait();
            }
            catch(InterruptedException ex)
            {
                interrupted = true;
            }
        }
        //
        // Restore the interrupted flag if we were interrupted.
        //
        if(interrupted)
        {
            Thread.currentThread().interrupt();
        }
    }

    private void enqueueBatchRequest(Ice.ObjectPrx proxy)
    {
        assert(_batchMarker < _batchStream.size());
        Boolean compress = ((Ice.ObjectPrxHelperBase)proxy)._getReference().getCompressOverride();
        if(compress != null)
        {
            _batchCompress |= compress.booleanValue();
        }
        _batchMarker = _batchStream.size();
        ++_batchRequestNum;
    }

    private Ice.BatchRequestInterceptor _interceptor;
    private Ice.OutputStream _batchStream;
    private boolean _batchStreamInUse;

    //
    // While finishBatchRequest holds the in-use stream, this is its thread: the only thread allowed to re-enter
    // swap() (for its own auto-flush). A null owner means no thread may flush the in-use stream, so other threads
    // wait for _batchStreamInUse to clear.
    //
    private Thread _batchStreamOwner;

    private int _batchRequestNum;
    private int _batchMarker;
    private boolean _batchCompress;
    private BatchRequestI _request;
    private Ice.LocalException _exception;
    private int _maxSize;

    final private static int _udpOverhead = 20 + 8;
}
