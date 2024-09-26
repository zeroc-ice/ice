//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

//
// Base class for handling asynchronous invocations. This class is
// responsible for the handling of the output stream and the child
// invocation observer.
//
abstract class OutgoingAsyncBase<T> extends InvocationFuture<T> {
    public boolean sent() {
        return sent(true);
    }

    public boolean completed(InputStream is) {
        assert (false); // Must be implemented by classes that handle responses
        return false;
    }

    public boolean completed(LocalException ex) {
        return finished(ex);
    }

    public final void attachRemoteObserver(ConnectionInfo info, Endpoint endpt, int requestId) {
        com.zeroc.Ice.Instrumentation.InvocationObserver observer = getObserver();
        if (observer != null) {
            final int size = _os.size() - Protocol.headerSize - 4;
            _childObserver = observer.getRemoteObserver(info, endpt, requestId, size);
            if (_childObserver != null) {
                _childObserver.attach();
            }
        }
    }

    public final void attachCollocatedObserver(ObjectAdapter adapter, int requestId) {
        com.zeroc.Ice.Instrumentation.InvocationObserver observer = getObserver();
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
        if (Thread.interrupted()) {
            throw new OperationInterruptedException();
        }

        try {
            return get();
        } catch (InterruptedException ex) {
            throw new OperationInterruptedException(ex);
        } catch (java.util.concurrent.ExecutionException ee) {
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
        _os = new OutputStream(instance, Protocol.currentProtocolEncoding);
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
    protected com.zeroc.Ice.Instrumentation.ChildInvocationObserver _childObserver;
}