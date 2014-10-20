// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

//
// Base class for handling asynchronous invocations. This class is
// responsible for the handling of the output stream and the child
// invocation observer.
//
public abstract class OutgoingAsyncBase extends IceInternal.AsyncResultI
{
    public int send(Ice.ConnectionI connection, boolean compress, boolean response) throws RetryException
    {
        assert(false); // This should be overriden if this object is used with a request handler
        return AsyncStatus.Queued; 
    }

    public int invokeCollocated(CollocatedRequestHandler handler)
    {
        assert(false); // This should be overriden if this object is used with a request handler
        return AsyncStatus.Queued; 
    }

    public boolean sent()
    {
        return sent(true);
    }

    public boolean completed(Ice.Exception ex)
    {
        return finished(ex);
    }

    public final void attachRemoteObserver(Ice.ConnectionInfo info, Ice.Endpoint endpt, int requestId)
    {
        if(_observer != null)
        {
            final int size = _os.size() - IceInternal.Protocol.headerSize - 4;
            _childObserver = getObserver().getRemoteObserver(info, endpt, requestId, size);
            if(_childObserver != null)
            {
                _childObserver.attach();
            }
        }
    }
    
    public final void attachCollocatedObserver(Ice.ObjectAdapter adapter, int requestId)
    {
        if(_observer != null)
        {
            final int size = _os.size() - IceInternal.Protocol.headerSize - 4;
            _childObserver = getObserver().getCollocatedObserver(adapter, requestId, size);
            if(_childObserver != null)
            {
                _childObserver.attach();
            }
        }
    }

    public final IceInternal.BasicStream getOs()
    {
        return _os;
    }

    protected OutgoingAsyncBase(Ice.Communicator com, Instance instance, String op, CallbackBase del)
    {
        super(com, instance, op, del);
        _os = new BasicStream(instance, Protocol.currentProtocolEncoding);
    }

    protected OutgoingAsyncBase(Ice.Communicator com, Instance instance, String op, CallbackBase del, BasicStream os)
    {
        super(com, instance, op, del);
        _os = os;
    }

    @Override
    protected boolean sent(boolean done)
    {
        if(done)
        {
            if(_childObserver != null)
            {
                _childObserver.detach();
                _childObserver = null;
            }
        }
        return super.sent(done);
    }

    @Override
    protected boolean finished(Ice.Exception ex)
    {
        if(_childObserver != null)
        {
            _childObserver.failed(ex.ice_name());
            _childObserver.detach();
            _childObserver = null;
        }
        return super.finished(ex);
    }

    protected BasicStream _os;
    protected Ice.Instrumentation.ChildInvocationObserver _childObserver;
}
