//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

import com.zeroc.Ice.CloseConnectionException;
import com.zeroc.Ice.CommunicatorDestroyedException;
import com.zeroc.Ice.InvocationTimeoutException;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.ObjectNotExistException;
import com.zeroc.Ice.OperationMode;
import com.zeroc.Ice.ReplyStatus;
import com.zeroc.Ice.RequestFailedException;

//
// Base class for proxy based invocations. This class handles the
// retry for proxy invocations. It also ensures the child observer is
// correct notified of failures and make sure the retry task is
// correctly canceled when the invocation completes.
//
public abstract class ProxyOutgoingAsyncBaseI<T> extends OutgoingAsyncBaseI<T>
    implements ProxyOutgoingAsyncBase {
  public boolean isBatch() {
    return _proxy._getReference().isBatch();
  }

  @Override
  public com.zeroc.Ice.ObjectPrx getProxy() {
    return _proxy;
  }

  @Override
  public boolean completed(com.zeroc.Ice.InputStream is) {
    //
    // NOTE: this method is called from ConnectionI.parseMessage
    // with the connection locked. Therefore, it must not invoke
    // any user callbacks.
    //

    assert (_proxy.ice_isTwoway()); // Can only be called for twoways.

    if (_childObserver != null) {
      _childObserver.reply(is.size() - Protocol.headerSize - 4);
      _childObserver.detach();
      _childObserver = null;
    }

    ReplyStatus replyStatus;
    try {
      replyStatus = ReplyStatus.valueOf(is.readByte());

      switch (replyStatus) {
        case Ok:
          break;

        case UserException:
          if (_observer != null) {
            _observer.userException();
          }
          break;

        case ObjectNotExist:
        case FacetNotExist:
        case OperationNotExist:
          {
            com.zeroc.Ice.Identity id = com.zeroc.Ice.Identity.ice_read(is);

            //
            // For compatibility with the old FacetPath.
            //
            String[] facetPath = is.readStringSeq();
            String facet;
            if (facetPath.length > 0) {
              if (facetPath.length > 1) {
                throw new com.zeroc.Ice.MarshalException();
              }
              facet = facetPath[0];
            } else {
              facet = "";
            }

            String operation = is.readString();

            RequestFailedException ex =
                switch (replyStatus) {
                  case ObjectNotExist -> new ObjectNotExistException();
                  case FacetNotExist -> new com.zeroc.Ice.FacetNotExistException();
                  default -> new com.zeroc.Ice.OperationNotExistException();
                };

            ex.id = id;
            ex.facet = facet;
            ex.operation = operation;
            throw ex;
          }

        case UnknownException:
        case UnknownLocalException:
        case UnknownUserException:
          {
            String unknown = is.readString();

            com.zeroc.Ice.UnknownException ex =
                switch (replyStatus) {
                  case UnknownException -> new com.zeroc.Ice.UnknownException();
                  case UnknownLocalException -> new com.zeroc.Ice.UnknownLocalException();
                  default -> new com.zeroc.Ice.UnknownUserException();
                };

            ex.unknown = unknown;
            throw ex;
          }

        default:
          // Can't happen. We throw MarshalException in ReplyStatus.valueOf if we receive an invalid
          // byte.
          assert false;
      }

      return finished(replyStatus == ReplyStatus.Ok, true);
    } catch (com.zeroc.Ice.Exception ex) {
      return completed(ex);
    }
  }

  @Override
  public boolean completed(com.zeroc.Ice.Exception exc) {
    if (_childObserver != null) {
      _childObserver.failed(exc.ice_id());
      _childObserver.detach();
      _childObserver = null;
    }

    _cachedConnection = null;

    //
    // NOTE: at this point, synchronization isn't needed, no other threads should be
    // calling on the callback.
    //
    try {
      // It's important to let the retry queue do the retry even if
      // the retry interval is 0. This method can be called with the
      // connection locked so we can't just retry here.
      _instance.retryQueue().add(this, handleException(exc));
      return false;
    } catch (com.zeroc.Ice.Exception ex) {
      return finished(ex); // No retries, we're done
    }
  }

  @Override
  public void retryException(com.zeroc.Ice.Exception ex) {
    try {
      // Clear request handler and always retry.
      _proxy._getRequestHandlerCache().clearCachedRequestHandler(_handler);
      // It's important to let the retry queue do the retry. This is
      // called from the connect request handler and the retry might
      // require could end up waiting for the flush of the
      // connection to be done.
      _instance.retryQueue().add(this, 0);
    } catch (com.zeroc.Ice.Exception exc) {
      if (completed(exc)) {
        invokeCompletedAsync();
      }
    }
  }

  @Override
  public void retry() {
    invokeImpl(false);
  }

  @Override
  public void abort(com.zeroc.Ice.Exception ex) {
    assert (_childObserver == null);
    if (finished(ex)) {
      invokeCompletedAsync();
    } else if (ex instanceof CommunicatorDestroyedException) {
      //
      // If it's a communicator destroyed exception, don't swallow
      // it but instead notify the user thread. Even if no callback
      // was provided.
      //
      throw ex;
    }
  }

  protected ProxyOutgoingAsyncBaseI(com.zeroc.Ice._ObjectPrxI prx, String op) {
    super(prx.ice_getCommunicator(), prx._getReference().getInstance(), op);
    _proxy = prx;
    _mode = OperationMode.Normal;
    _cnt = 0;
    _sent = false;
  }

  protected ProxyOutgoingAsyncBaseI(
      com.zeroc.Ice._ObjectPrxI prx, String op, com.zeroc.Ice.OutputStream os) {
    super(prx.ice_getCommunicator(), prx._getReference().getInstance(), op, os);
    _proxy = prx;
    _mode = OperationMode.Normal;
    _cnt = 0;
    _sent = false;
  }

  protected void invokeImpl(boolean userThread) {
    try {
      if (userThread) {
        int invocationTimeout = _proxy._getReference().getInvocationTimeout();
        if (invocationTimeout > 0) {
          _timerFuture =
              _instance
                  .timer()
                  .schedule(
                      () -> {
                        cancel(new InvocationTimeoutException());
                      },
                      invocationTimeout,
                      java.util.concurrent.TimeUnit.MILLISECONDS);
        }
      } else // If not called from the user thread, it's called from the retry queue
      {
        if (_observer != null) {
          _observer.retried();
        }
      }

      while (true) {
        try {
          _sent = false;
          _handler = _proxy._getRequestHandlerCache().getRequestHandler();
          int status = _handler.sendAsyncRequest(this);
          if ((status & AsyncStatus.Sent) > 0) {
            if (userThread) {
              _sentSynchronously = true;
              if ((status & AsyncStatus.InvokeSentCallback) > 0) {
                invokeSent(); // Call the sent callback from the user thread.
              }
            } else {
              if ((status & AsyncStatus.InvokeSentCallback) > 0) {
                invokeSentAsync(); // Call the sent callback from a client thread pool thread.
              }
            }
          }
          return; // We're done!
        } catch (RetryException ex) {
          // Clear request handler and always retry.
          _proxy._getRequestHandlerCache().clearCachedRequestHandler(_handler);
        } catch (com.zeroc.Ice.Exception ex) {
          if (_childObserver != null) {
            _childObserver.failed(ex.ice_id());
            _childObserver.detach();
            _childObserver = null;
          }
          final int interval = handleException(ex);
          if (interval > 0) {
            _instance.retryQueue().add(this, interval);
            return;
          } else if (_observer != null) {
            _observer.retried();
          }
        }
      }
    } catch (com.zeroc.Ice.Exception ex) {
      //
      // If called from the user thread we re-throw, the exception
      // will be catch by the caller and abort() will be called.
      //
      if (userThread) {
        throw ex;
      } else if (finished(ex)) // No retries, we're done
      {
        invokeCompletedAsync();
      }
    }
  }

  @Override
  protected boolean sent(boolean done) {
    _sent = true;
    if (done) {
      if (_timerFuture != null) {
        _timerFuture.cancel(false);
        _timerFuture = null;
      }
    }
    return super.sent(done);
  }

  @Override
  protected boolean finished(com.zeroc.Ice.Exception ex) {
    if (_timerFuture != null) {
      _timerFuture.cancel(false);
      _timerFuture = null;
    }
    return super.finished(ex);
  }

  @Override
  protected boolean finished(boolean ok, boolean invoke) {
    if (_timerFuture != null) {
      _timerFuture.cancel(false);
      _timerFuture = null;
    }
    return super.finished(ok, invoke);
  }

  protected int handleException(com.zeroc.Ice.Exception ex) {
    // Clear the request handler
    _proxy._getRequestHandlerCache().clearCachedRequestHandler(_handler);

    // We only retry local exceptions.
    //
    // A CloseConnectionException indicates graceful server shutdown, and is therefore
    // always repeatable without violating "at-most-once". That's because by sending a
    // close connection message, the server guarantees that all outstanding requests
    // can safely be repeated.
    //
    // An ObjectNotExistException can always be retried as well without violating
    // "at-most-once" (see the implementation of the checkRetryAfterException method
    // below for the reasons why it can be useful).
    //
    // If the request didn't get sent or if it's non-mutating or idempotent it can
    // also always be retried if the retry count isn't reached.
    boolean shouldRetry =
        ex instanceof LocalException
            && (!_sent
                || _mode == OperationMode.Nonmutating
                || _mode == OperationMode.Idempotent
                || ex instanceof CloseConnectionException
                || ex instanceof ObjectNotExistException);

    if (shouldRetry) {
      try {
        return checkRetryAfterException((LocalException) ex);
      } catch (CommunicatorDestroyedException exc) {
        throw ex; // The communicator is already destroyed, so we cannot retry.
      }
    } else {
      throw ex; // Retry could break at-most-once semantics, don't retry.
    }
  }

  private int checkRetryAfterException(LocalException ex) {
    Reference ref = _proxy._getReference();
    Instance instance = ref.getInstance();
    TraceLevels traceLevels = instance.traceLevels();
    com.zeroc.Ice.Logger logger = instance.initializationData().logger;
    int[] retryIntervals = instance.retryIntervals();

    // We don't retry batch requests because the exception might have caused
    // all the requests batched with the connection to be aborted and we
    // want the application to be notified.
    if (ref.isBatch()) {
      throw ex;
    }

    // If it's a fixed proxy, retrying isn't useful as the proxy is tied to
    // the connection and the request will fail with the exception.
    if (ref instanceof FixedReference) {
      throw ex;
    }

    if (ex instanceof ObjectNotExistException) {
      ObjectNotExistException one = (ObjectNotExistException) ex;

      if (ref.getRouterInfo() != null && one.operation.equals("ice_add_proxy")) {
        // If we have a router, an ObjectNotExistException with an
        // operation name "ice_add_proxy" indicates to the client
        // that the router isn't aware of the proxy (for example,
        // because it was evicted by the router). In this case, we
        // must *always* retry, so that the missing proxy is added
        // to the router.

        ref.getRouterInfo().clearCache(ref);

        if (traceLevels.retry >= 1) {
          String s = "retrying operation call to add proxy to router\n" + ex.toString();
          logger.trace(traceLevels.retryCat, s);
        }

        return 0; // We must always retry, so we don't look at the retry count.
      } else if (ref.isIndirect()) {
        // We retry ObjectNotExistException if the reference is indirect.

        if (ref.isWellKnown()) {
          LocatorInfo li = ref.getLocatorInfo();
          if (li != null) {
            li.clearCache(ref);
          }
        }
      } else {
        // For all other cases, we don't retry ObjectNotExistException.
        throw ex;
      }
    } else if (ex instanceof RequestFailedException) {
      // For all other cases, we don't retry ObjectNotExistException
      throw ex;
    }

    // There is no point in retrying an operation that resulted in a
    // MarshalException. This must have been raised locally (because
    // if it happened in a server it would result in an
    // UnknownLocalException instead), which means there was a problem
    // in this process that will not change if we try again.
    //
    // The most likely cause for a MarshalException is exceeding the
    // maximum message size, which is represented by the subclass
    // MemoryLimitException. For example, a client can attempt to send
    // a message that exceeds the maximum memory size, or accumulate
    // enough batch requests without flushing that the maximum size is
    // reached.
    //
    // This latter case is especially problematic, because if we were
    // to retry a batch request after a MarshalException, we would in
    // fact silently discard the accumulated requests and allow new
    // batch requests to accumulate. If the subsequent batched
    // requests do not exceed the maximum message size, it appears to
    // the client that all of the batched requests were accepted, when
    // in reality only the last few are actually sent.
    if (ex instanceof com.zeroc.Ice.MarshalException) {
      throw ex;
    }

    // Don't retry if the communicator is destroyed, object adapter is deactivated,
    // or connection is manually closed.
    if (ex instanceof CommunicatorDestroyedException
        || ex instanceof com.zeroc.Ice.ObjectAdapterDeactivatedException
        || ex instanceof com.zeroc.Ice.ConnectionManuallyClosedException) {
      throw ex;
    }

    // Don't retry invocation timeouts.
    if (ex instanceof InvocationTimeoutException
        || ex instanceof com.zeroc.Ice.InvocationCanceledException) {
      throw ex;
    }

    // Don't retry on OperationInterruptedException.
    if (ex instanceof com.zeroc.Ice.OperationInterruptedException) {
      throw ex;
    }

    ++_cnt;
    assert (_cnt > 0);

    int interval;
    if (_cnt == (retryIntervals.length + 1) && ex instanceof CloseConnectionException) {
      // Always retry a CloseConnectionException at least once, even if the retry limit is reached.
      interval = 0;
    } else if (_cnt > retryIntervals.length) {
      if (traceLevels.retry >= 1) {
        String s =
            "cannot retry operation call because retry limit has been exceeded\n" + ex.toString();
        logger.trace(traceLevels.retryCat, s);
      }
      throw ex;
    } else {
      interval = retryIntervals[_cnt - 1];
    }

    if (traceLevels.retry >= 1) {
      String s = "retrying operation call";
      if (interval > 0) {
        s += " in " + interval + "ms";
      }
      s += " because of exception\n" + ex;
      logger.trace(traceLevels.retryCat, s);
    }

    return interval;
  }

  protected void prepare(java.util.Map<String, String> ctx) {
    Reference ref = _proxy._getReference();

    Protocol.checkSupportedProtocol(Protocol.getCompatibleProtocol(ref.getProtocol()));

    _observer = ObserverHelper.get(_proxy, _operation, ctx == null ? _emptyContext : ctx);

    if (ref.isBatch()) {
      ref.getBatchRequestQueue().prepareBatchRequest(_os);
    } else {
      _os.writeBlob(Protocol.requestHdr);
    }
    ref.getIdentity().ice_writeMembers(_os);

    //
    // For compatibility with the old FacetPath.
    //
    String facet = ref.getFacet();
    if (facet == null || facet.isEmpty()) {
      _os.writeStringSeq(null);
    } else {
      String[] facetPath = {facet};
      _os.writeStringSeq(facetPath);
    }

    _os.writeString(_operation);

    _os.writeByte((byte) _mode.value());

    if (ctx != com.zeroc.Ice.ObjectPrx.noExplicitContext) {
      //
      // Explicit context
      //
      com.zeroc.Ice.ContextHelper.write(_os, ctx);
    } else {
      //
      // Implicit context
      //
      com.zeroc.Ice.ImplicitContextI implicitContext = ref.getInstance().getImplicitContext();
      java.util.Map<String, String> prxContext = ref.getContext();

      if (implicitContext == null) {
        com.zeroc.Ice.ContextHelper.write(_os, prxContext);
      } else {
        implicitContext.write(prxContext, _os);
      }
    }
  }

  protected final com.zeroc.Ice._ObjectPrxI _proxy;
  protected RequestHandler _handler;
  protected OperationMode _mode;

  private java.util.concurrent.Future<?> _timerFuture;
  private int _cnt;
  private boolean _sent;

  private static final java.util.Map<String, String> _emptyContext = new java.util.HashMap<>();
}
