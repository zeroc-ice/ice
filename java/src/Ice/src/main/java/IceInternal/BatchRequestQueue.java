// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
            enqueueBatchRequest();
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
    };

    public
    BatchRequestQueue(Instance instance, boolean datagram)
    {
        Ice.InitializationData initData = instance.initializationData();
        _interceptor = initData.batchRequestInterceptor;
        _batchStreamInUse = false;
        _batchRequestNum = 0;
        _batchStream = new BasicStream(instance, Protocol.currentProtocolEncoding);
        _batchStream.writeBlob(Protocol.requestBatchHdr);
        _batchMarker = _batchStream.size();
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
    prepareBatchRequest(BasicStream os)
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
    finishBatchRequest(BasicStream os, Ice.ObjectPrx proxy, String operation)
    {
        //
        // No need for synchronization, no other threads are supposed
        // to modify the queue since we set _batchStreamInUse to true.
        //
        assert(_batchStreamInUse);
        _batchStream.swap(os);

        try
        {
            _batchStreamCanFlush = true; // Allow flush to proceed even if the stream is marked in use.

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
                _batchMarker = _batchStream.size();
                ++_batchRequestNum;
            }
        }
        finally
        {
            synchronized(this)
            {
                _batchStream.resize(_batchMarker, false);
                _batchStreamInUse = false;
                _batchStreamCanFlush = false;
                notifyAll();
            }
        }
    }

    synchronized public void
    abortBatchRequest(BasicStream os)
    {
        if(_batchStreamInUse)
        {
            _batchStream.swap(os);
            _batchStream.resize(_batchMarker, false);
            _batchStreamInUse = false;
            notifyAll();
        }
    }

    synchronized public int
    swap(BasicStream os)
    {
        if(_batchRequestNum == 0)
        {
            return 0;
        }

        waitStreamInUse(true);

        byte[] lastRequest = null;
        if(_batchMarker < _batchStream.size())
        {
            lastRequest = new byte[_batchStream.size() - _batchMarker];
            Buffer buffer = _batchStream.getBuffer();
            buffer.b.position(_batchMarker);
            buffer.b.get(lastRequest);
            _batchStream.resize(_batchMarker, false);
        }

        int requestNum = _batchRequestNum;
        _batchStream.swap(os);

        //
        // Reset the batch.
        //
        _batchRequestNum = 0;
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
        boolean interrupted = false;
        while(_batchStreamInUse && !(flush && _batchStreamCanFlush))
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

    private void enqueueBatchRequest()
    {
        assert(_batchMarker < _batchStream.size());
        _batchMarker = _batchStream.size();
        ++_batchRequestNum;
    }

    private Ice.BatchRequestInterceptor _interceptor;
    private BasicStream _batchStream;
    private boolean _batchStreamInUse;
    private boolean _batchStreamCanFlush;
    private int _batchRequestNum;
    private int _batchMarker;
    private BatchRequestI _request;
    private Ice.LocalException _exception;
    private int _maxSize;

    final private static int _udpOverhead = 20 + 8;
};
