// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import com.zeroc.Ice.Instrumentation.ChildInvocationObserver;
import com.zeroc.Ice.Instrumentation.InvocationObserver;

import java.util.concurrent.ExecutionException;

//
// Base class for handling asynchronous invocations. This class is
// responsible for the handling of the output stream and the child invocation observer.
//
abstract class OutgoingAsyncBase<T> extends InvocationFuture<T> {
    public boolean sent() {
        return sent(true);
    }

    public boolean completed(InputStream is) {
        assert false; // Must be implemented by classes that handle responses
        return false;
    }

    public boolean completed(LocalException ex) {
        return finished(ex);
    }

    public final void attachRemoteObserver(ConnectionInfo info, Endpoint endpt, int requestId) {
        InvocationObserver observer = getObserver();
        if (observer != null) {
            final int size = _os.size() - Protocol.headerSize - 4;
            _childObserver = observer.getRemoteObserver(info, endpt, requestId, size);
            if (_childObserver != null) {
                _childObserver.attach();
            }
        }
    }

    public final void attachCollocatedObserver(ObjectAdapter adapter, int requestId) {
        InvocationObserver observer = getObserver();
        if (observer != null) {
            final int size = _os.size() - Protocol.headerSize - 4;
            _childObserver = observer.getCollocatedObserver(adapter, requestId, size);
            if (_childObserver != null) {
                _childObserver.attach();
            }
        }
    }

    public final OutputStream getOs() {
        return _os;
    }

    public T waitForResponse() {
        try {
            return get();
        } catch (InterruptedException ex) {
            throw new OperationInterruptedException(ex);
        } catch (ExecutionException ee) {
            try {
                throw ee.getCause().fillInStackTrace();
            } catch (RuntimeException ex) // Includes LocalException
                {
                    throw ex;
                } catch (Throwable ex) {
                throw new UnknownException(ex);
            }
        }
    }

    protected OutgoingAsyncBase(Communicator com, Instance instance, String op) {
        super(com, instance, op);
        _os =
            new OutputStream(
                Protocol.currentProtocolEncoding,
                instance.defaultsAndOverrides().defaultFormat,
                instance.cacheMessageBuffers() > 1);
    }

    protected OutgoingAsyncBase(Communicator com, Instance instance, String op, OutputStream os) {
        super(com, instance, op);
        _os = os;
    }

    @Override
    protected boolean sent(boolean done) {
        if (done) {
            if (_childObserver != null) {
                _childObserver.detach();
                _childObserver = null;
            }
        }
        return super.sent(done);
    }

    @Override
    protected boolean finished(LocalException ex) {
        if (_childObserver != null) {
            _childObserver.failed(ex.ice_id());
            _childObserver.detach();
            _childObserver = null;
        }
        return super.finished(ex);
    }

    protected OutputStream _os;
    protected ChildInvocationObserver _childObserver;
}
