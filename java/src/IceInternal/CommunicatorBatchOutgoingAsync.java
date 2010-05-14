// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class CommunicatorBatchOutgoingAsync extends BatchOutgoingAsync
{
    public CommunicatorBatchOutgoingAsync(Ice.Communicator communicator, Instance instance, String operation,
                                          CallbackBase callback)
    {
        super(instance, operation, callback);
        _communicator = communicator;

        //
        // _useCount is initialized to 1 to prevent premature callbacks.
        // The caller must invoke ready() after all flush requests have
        // been initiated.
        //
        _useCount = 1;

        //
        // Assume all connections are flushed synchronously.
        //
        _sentSynchronously = true;
    }

    @Override
    public Ice.Communicator getCommunicator()
    {
        return _communicator;
    }

    public void flushConnection(Ice.Connection con)
    {
        synchronized(_monitor)
        {
            ++_useCount;
        }
        con.begin_flushBatchRequests(_cb);
    }

    public void ready()
    {
        check(null, null, true);
    }

    private void completed(Ice.AsyncResult r)
    {
        Ice.Connection con = r.getConnection();
        assert(con != null);

        try
        {
            con.end_flushBatchRequests(r);
            assert(false); // completed() should only be called when an exception occurs.
        }
        catch(Ice.LocalException ex)
        {
            check(r, ex, false);
        }
    }

    private void sent(Ice.AsyncResult r)
    {
        check(r, null, r.sentSynchronously());
    }

    private void check(Ice.AsyncResult r, Ice.LocalException ex, boolean userThread)
    {
        boolean done = false;

        synchronized(_monitor)
        {
            assert(_useCount > 0);
            --_useCount;

            //
            // We report that the communicator flush request was sent synchronously
            // if all of the connection flush requests are sent synchronously.
            //
            if((r != null && !r.sentSynchronously()) || ex != null)
            {
                _sentSynchronously = false;
            }

            if(_useCount == 0)
            {
                done = true;
                _state |= Done | OK | Sent;
                _monitor.notifyAll();
            }
        }

        if(done)
        {
            //
            // sentSynchronously_ is immutable here.
            //
            if(!_sentSynchronously && userThread)
            {
                __sentAsync();
            }
            else
            {
                assert(_sentSynchronously == userThread); // sentSynchronously && !userThread is impossible.
                __sent();
            }
        }
    }

    private Ice.Communicator _communicator;
    private int _useCount;

    private Ice.Callback _cb = new Ice.Callback()
    {
        @Override
        public void completed(Ice.AsyncResult r)
        {
            CommunicatorBatchOutgoingAsync.this.completed(r);
        }

        @Override
        public void sent(Ice.AsyncResult r)
        {
            CommunicatorBatchOutgoingAsync.this.sent(r);
        }
    };
}
