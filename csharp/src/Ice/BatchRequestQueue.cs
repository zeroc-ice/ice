//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;

namespace IceInternal
{
    sealed class BatchRequestI : Ice.BatchRequest
    {
        public BatchRequestI(BatchRequestQueue queue)
        {
            _queue = queue;
        }

        public void reset(Ice.ObjectPrx proxy, string operation, int size)
        {
            _proxy = proxy;
            _operation = operation;
            _size = size;
        }

        public void enqueue()
        {
            _queue.enqueueBatchRequest(_proxy);
        }

        public Ice.ObjectPrx getProxy()
        {
            return _proxy;
        }

        public string getOperation()
        {
            return _operation;
        }

        public int getSize()
        {
            return _size;
        }

        private BatchRequestQueue _queue;
        private Ice.ObjectPrx _proxy;
        private string _operation;
        private int _size;
    }

    public sealed class BatchRequestQueue
    {
        public BatchRequestQueue(Instance instance, bool datagram)
        {
            Ice.InitializationData initData = instance.initializationData();
            _interceptor = initData.batchRequestInterceptor;
            _batchStreamInUse = false;
            _batchRequestNum = 0;
            _batchStream = new Ice.OutputStream(instance, Ice.Util.currentProtocolEncoding);
            _batchStream.writeBlob(Protocol.requestBatchHdr);
            _batchMarker = _batchStream.size();
            _request = new BatchRequestI(this);

            _maxSize = instance.batchAutoFlushSize();
            if(_maxSize > 0 && datagram)
            {
                int udpSndSize = initData.properties.getPropertyAsIntWithDefault("Ice.UDP.SndSize",
                                                                                 65535 - _udpOverhead);
                if(udpSndSize < _maxSize)
                {
                    _maxSize = udpSndSize;
                }
            }
        }

        public void
        prepareBatchRequest(Ice.OutputStream os)
        {
            lock(this)
            {
                if(_exception != null)
                {
                    throw _exception;
                }
                waitStreamInUse(false);
                _batchStreamInUse = true;
                _batchStream.swap(os);
            }
        }

        public void
        finishBatchRequest(Ice.OutputStream os, Ice.ObjectPrx proxy, string operation)
        {
            lock(this)
            {
                //
                // Bring the request stream back and become the owner so this thread's own auto-flush below can
                // re-enter swap(). swap() lets only _batchStreamOwner through while the stream is in use, and only
                // after passing that gate does it read the queue's bookkeeping; no other thread can touch the
                // queue, so the mutations below need no further synchronization.
                //
                Debug.Assert(_batchStreamInUse);
                _batchStream.swap(os);
                _batchStreamOwner = System.Threading.Thread.CurrentThread;
            }

            try
            {
                if(_maxSize > 0 && _batchStream.size() >= _maxSize)
                {
                    proxy.begin_ice_flushBatchRequests(); // Auto flush
                }

                Debug.Assert(_batchMarker < _batchStream.size());
                if(_interceptor != null)
                {
                    _request.reset(proxy, operation, _batchStream.size() - _batchMarker);
                    _interceptor(_request, _batchRequestNum, _batchMarker);
                }
                else
                {
                    bool compress;
                    if(((Ice.ObjectPrxHelperBase)proxy).iceReference().getCompressOverride(out compress))
                    {
                        _batchCompress |= compress;
                    }
                    _batchMarker = _batchStream.size();
                    ++_batchRequestNum;
                }
            }
            finally
            {
                lock(this)
                {
                    _batchStream.resize(_batchMarker);
                    _batchStreamInUse = false;
                    _batchStreamOwner = null;
                    System.Threading.Monitor.PulseAll(this);
                }
            }
        }

        public void
        abortBatchRequest(Ice.OutputStream os)
        {
            lock(this)
            {
                if(_batchStreamInUse)
                {
                    _batchStream.swap(os);
                    _batchStream.resize(_batchMarker);
                    _batchStreamInUse = false;
                    System.Threading.Monitor.PulseAll(this);
                }
            }
        }

        public int
        swap(Ice.OutputStream os, out bool compress)
        {
            lock(this)
            {
                waitStreamInUse(true);

                //
                // Read the bookkeeping only after waitStreamInUse: finishBatchRequest mutates it without the lock
                // while it owns the in-use stream, so reading _batchRequestNum before the wait would race those
                // writes.
                //
                if(_batchRequestNum == 0)
                {
                    compress = false;
                    return 0;
                }

                byte[] lastRequest = null;
                if(_batchMarker < _batchStream.size())
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
                if(lastRequest != null)
                {
                    _batchStream.writeBlob(lastRequest);
                }
                return requestNum;
            }
        }

        public void
        destroy(Ice.LocalException ex)
        {
            lock(this)
            {
                _exception = ex;
            }
        }

        public bool
        isEmpty()
        {
            lock(this)
            {
                return _batchStream.size() == Protocol.requestBatchHdr.Length;
            }
        }

        private void
        waitStreamInUse(bool flush)
        {
            //
            // This is similar to a mutex lock in that the stream is
            // only "locked" while marshaling. As such we don't permit the wait
            // to be interrupted. Instead the interrupted status is saved and
            // restored.
            //
            // Only the thread that currently owns the in-use stream may bypass the wait to run its own
            // auto-flush; every other caller waits for the in-progress batch request to finish.
            //
            while(_batchStreamInUse && !(flush && _batchStreamOwner == System.Threading.Thread.CurrentThread))
            {
                System.Threading.Monitor.Wait(this);
            }
        }

        internal void enqueueBatchRequest(Ice.ObjectPrx proxy)
        {
            Debug.Assert(_batchMarker < _batchStream.size());
            bool compress;
            if(((Ice.ObjectPrxHelperBase)proxy).iceReference().getCompressOverride(out compress))
            {
                _batchCompress |= compress;
            }
            _batchMarker = _batchStream.size();
            ++_batchRequestNum;
        }

        private System.Action<Ice.BatchRequest, int, int> _interceptor;
        private Ice.OutputStream _batchStream;
        private bool _batchStreamInUse;

        //
        // While finishBatchRequest holds the in-use stream, this is its thread: the only thread allowed to
        // re-enter swap() (for its own auto-flush). A null owner means no thread may flush the in-use stream, so
        // other threads wait for _batchStreamInUse to clear.
        //
        private System.Threading.Thread _batchStreamOwner;
        private int _batchRequestNum;
        private int _batchMarker;
        private bool _batchCompress;
        private BatchRequestI _request;
        private Ice.LocalException _exception;
        private int _maxSize;

        private static int _udpOverhead = 20 + 8;
    }
}
