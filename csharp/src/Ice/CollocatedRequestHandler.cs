//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using ZeroC.Ice;
using ZeroC.Ice.Instrumentation;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;

namespace IceInternal
{
    public class CollocatedRequestHandler : IRequestHandler
    {
        public
        CollocatedRequestHandler(Reference reference, ObjectAdapter adapter)
        {
            _reference = reference;
            _adapter = adapter;
            _requestId = 0;
        }

        public IRequestHandler? Update(IRequestHandler previousHandler, IRequestHandler? newHandler) =>
            previousHandler == this ? newHandler : this;

        public void SendAsyncRequest(ProxyOutgoingAsyncBase outAsync) => outAsync.InvokeCollocated(this);

        public void AsyncRequestCanceled(OutgoingAsyncBase outAsync, Exception ex)
        {
            lock (this)
            {
                if (_sendAsyncRequests.TryGetValue(outAsync, out int requestId))
                {
                    if (requestId > 0)
                    {
                        _asyncRequests.Remove(requestId);
                    }
                    _sendAsyncRequests.Remove(outAsync);
                    if (outAsync.Exception(ex))
                    {
                        Task.Run(outAsync.InvokeException);
                    }
                    _adapter.DecDirectCount(); // invokeAll won't be called, decrease the direct count.
                    return;
                }
                if (outAsync is OutgoingAsync o)
                {
                    Debug.Assert(o != null);
                    foreach (KeyValuePair<int, OutgoingAsyncBase> e in _asyncRequests)
                    {
                        if (e.Value == o)
                        {
                            _asyncRequests.Remove(e.Key);
                            if (outAsync.Exception(ex))
                            {
                                Task.Run(outAsync.InvokeException);
                            }
                            return;
                        }
                    }
                }
            }
        }

        public Connection? GetConnection() => null;

        public void InvokeAsyncRequest(ProxyOutgoingAsyncBase outAsync, bool synchronous)
        {
            //
            // Increase the direct count to prevent the thread pool from being destroyed before
            // invokeAll is called. This will also throw if the object adapter has been deactivated.
            //
            _adapter.IncDirectCount();

            int requestId = 0;
            try
            {
                lock (this)
                {
                    outAsync.Cancelable(this); // This will throw if the request is canceled

                    if (!outAsync.IsOneway)
                    {
                        requestId = ++_requestId;
                        _asyncRequests.Add(requestId, outAsync);
                    }

                    _sendAsyncRequests.Add(outAsync, requestId);
                }
            }
            catch (Exception)
            {
                _adapter.DecDirectCount();
                throw;
            }

            outAsync.AttachCollocatedObserver(_adapter, requestId, outAsync.RequestFrame.Size);
            if (_adapter.TaskScheduler != null || !synchronous || outAsync.IsOneway || _reference.InvocationTimeout > 0)
            {
                // Don't invoke from the user thread if async or invocation timeout is set. We also don't dispatch
                // oneway from the user thread to match the non-collocated behavior where the oneway synchronous
                // request returns as soon as it's sent over the transport.
                Task.Factory.StartNew(
                    () =>
                    {
                        if (SentAsync(outAsync))
                        {
                            ValueTask vt = InvokeAllAsync(outAsync.RequestFrame, requestId);
                            // TODO: do something with the value task
                        }
                    }, default, TaskCreationOptions.None, _adapter.TaskScheduler ?? TaskScheduler.Default);
            }
            else // Optimization: directly call invokeAll
            {
                if (SentAsync(outAsync))
                {
                    Debug.Assert(outAsync.RequestFrame != null);
                    ValueTask vt = InvokeAllAsync(outAsync.RequestFrame, requestId);
                    // TODO: do something with the value task
                }
            }
        }

        private bool SentAsync(OutgoingAsyncBase outAsync)
        {
            lock (this)
            {
                if (!_sendAsyncRequests.Remove(outAsync))
                {
                    return false; // The request timed-out.
                }

                if (!outAsync.Sent())
                {
                    return true;
                }
            }
            // The progress callback is always called from the default task scheduler
            Task.Run(outAsync.InvokeSent);
            return true;
        }

        private async ValueTask InvokeAllAsync(OutgoingRequestFrame outgoingRequest, int requestId)
        {
            // The object adapter DirectCount was incremented by the caller and we are responsible to decrement it
            // upon completion.

            IDispatchObserver? dispatchObserver = null;
            try
            {
                if (_adapter.Communicator.TraceLevels.Protocol >= 1)
                {
                    ProtocolTrace.TraceCollocatedFrame(
                        _adapter.Communicator,
                        (byte)Ice1Definitions.FrameType.Request,
                        requestId,
                        outgoingRequest);
                }

                var incomingRequest = new IncomingRequestFrame(_adapter.Communicator, outgoingRequest);
                var current = new Current(_adapter, incomingRequest, requestId);

                // Then notify and set dispatch observer, if any.
                ICommunicatorObserver? communicatorObserver = _adapter.Communicator.Observer;
                if (communicatorObserver != null)
                {
                    dispatchObserver = communicatorObserver.GetDispatchObserver(current, incomingRequest.Size);
                    dispatchObserver?.Attach();
                }

                try
                {
                    IObject? servant = current.Adapter.Find(current.Identity, current.Facet);

                    if (servant == null)
                    {
                        throw new ObjectNotExistException(current.Identity, current.Facet, current.Operation);
                    }

                    ValueTask<OutgoingResponseFrame> vt = servant.DispatchAsync(incomingRequest, current);
                    if (requestId != 0)
                    {
                        OutgoingResponseFrame response = await vt.ConfigureAwait(false);
                        dispatchObserver?.Reply(response.Size);
                        SendResponse(requestId, response);
                    }
                }
                catch (Exception ex)
                {
                    if (requestId != 0)
                    {
                        RemoteException actualEx;
                        if (ex is RemoteException remoteEx && !remoteEx.ConvertToUnhandled)
                        {
                            actualEx = remoteEx;
                        }
                        else
                        {
                            actualEx = new UnhandledException(current.Identity, current.Facet, current.Operation, ex);
                        }

                        Incoming.ReportException(actualEx, dispatchObserver, current);
                        var response = new OutgoingResponseFrame(current, actualEx);
                        dispatchObserver?.Reply(response.Size);
                        SendResponse(requestId, response);
                    }
                }
            }
            catch (Exception ex)
            {
                HandleException(requestId, ex);
            }
            finally
            {
                dispatchObserver?.Detach();
                _adapter.DecDirectCount();
            }
        }

        private void SendResponse(int requestId, OutgoingResponseFrame outgoingResponseFrame)
        {
            OutgoingAsyncBase? outAsync;
            lock (this)
            {
                var incomingResponseFrame = new IncomingResponseFrame(
                    _adapter.Communicator,
                    VectoredBufferExtensions.ToArray(outgoingResponseFrame.Data));
                if (_adapter.Communicator.TraceLevels.Protocol >= 1)
                {
                    ProtocolTrace.TraceCollocatedFrame(
                        _adapter.Communicator,
                        (byte)Ice1Definitions.FrameType.Reply,
                        requestId,
                        incomingResponseFrame);
                }

                if (_asyncRequests.TryGetValue(requestId, out outAsync))
                {
                    if (!outAsync.Response(incomingResponseFrame))
                    {
                        outAsync = null;
                    }
                    _asyncRequests.Remove(requestId);
                }
            }

            if (outAsync != null)
            {
                outAsync.InvokeResponse();
            }
        }

        private void HandleException(int requestId, Exception ex)
        {
            if (requestId == 0)
            {
                return; // Ignore exception for oneway messages.
            }

            OutgoingAsyncBase? outAsync;
            lock (this)
            {
                if (_asyncRequests.TryGetValue(requestId, out outAsync))
                {
                    if (!outAsync.Exception(ex))
                    {
                        outAsync = null;
                    }
                    _asyncRequests.Remove(requestId);
                }
            }

            if (outAsync != null)
            {
                outAsync.InvokeException();
            }
        }

        private readonly Reference _reference;
        private readonly ObjectAdapter _adapter;
        private int _requestId;
        private readonly Dictionary<OutgoingAsyncBase, int> _sendAsyncRequests = new Dictionary<OutgoingAsyncBase, int>();
        private readonly Dictionary<int, OutgoingAsyncBase> _asyncRequests = new Dictionary<int, OutgoingAsyncBase>();
    }
}
