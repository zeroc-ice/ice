//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

using ZeroC.Ice.Instrumentation;

namespace ZeroC.Ice
{
    internal class CollocatedRequestHandler : IRequestHandler
    {
        private readonly ObjectAdapter _adapter;
        private readonly object _mutex = new object();
        private readonly Reference _reference;
        private int _requestId;

        public Connection? GetConnection() => null;

        public ValueTask<Task<IncomingResponseFrame>?> SendRequestAsync(
            OutgoingRequestFrame outgoingRequestFrame,
            bool oneway,
            bool synchronous,
            IInvocationObserver? observer,
            CancellationToken cancel)
        {
            cancel.ThrowIfCancellationRequested();

            IChildInvocationObserver? childObserver = null;
            int requestId = 0;

            // The CollocatedRequestHandler is an internal object so it's safe to lock (this) as long as our
            // code doesn't use the collocated request handler as a lock. The lock here is useful to ensure
            // the protocol trace and observer call is output or called in the same order as the request ID
            // is allocated.
            lock (_mutex)
            {
                if (!oneway)
                {
                    requestId = ++_requestId;
                }

                if (observer != null)
                {
                    childObserver = observer.GetCollocatedObserver(_adapter, requestId, outgoingRequestFrame.Size);
                    childObserver?.Attach();
                }

                if (_adapter.Communicator.TraceLevels.Protocol >= 1)
                {
                    ProtocolTrace.TraceCollocatedFrame(_adapter.Communicator,
                                                       (byte)Ice1Definitions.FrameType.Request,
                                                       requestId,
                                                       outgoingRequestFrame);
                }
            }

            Task<IncomingResponseFrame?> task;
            if (_adapter.TaskScheduler != null || !synchronous || oneway || _reference.InvocationTimeout > 0)
            {
                // Don't invoke from the user thread if async or invocation timeout is set. We also don't dispatch
                // oneway from the user thread to match the non-collocated behavior where the oneway synchronous
                // request returns as soon as it's sent over the transport.
                task = Task.Factory.StartNew(() => InvokeAllAsync(outgoingRequestFrame, requestId, cancel),
                                             cancel,
                                             TaskCreationOptions.None,
                                             _adapter.TaskScheduler ?? TaskScheduler.Default).Unwrap();

                if (oneway)
                {
                    childObserver?.Detach();
                    return default;
                }
            }
            else // Optimization: directly call invokeAll
            {
                Debug.Assert(!oneway);
                task = InvokeAllAsync(outgoingRequestFrame, requestId, cancel);
            }
            return new ValueTask<Task<IncomingResponseFrame>?>(WaitForResponseAsync(task, childObserver, cancel));

            static async Task<IncomingResponseFrame> WaitForResponseAsync(
                Task<IncomingResponseFrame?> task,
                IChildInvocationObserver? observer,
                CancellationToken cancel)
            {
                try
                {
                    IncomingResponseFrame? incomingResponseFrame = await task.WaitAsync(cancel).ConfigureAwait(false);
                    if (incomingResponseFrame != null)
                    {
                        observer?.Reply(incomingResponseFrame.Size);
                        return incomingResponseFrame;
                    }
                    else
                    {
                        // WaitForResponseAsync is only called for twoway invocations and InvokeAllAsync only
                        // returns null for oneway invocations so we should never get a null incoming response
                        // frame here.
                        Debug.Assert(false);
                        return null!;
                    }
                }
                catch (Exception ex)
                {
                    observer?.Failed(ex.GetType().FullName ?? "System.Exception");
                    throw;
                }
                finally
                {
                    observer?.Detach();
                }
            }
        }

        internal CollocatedRequestHandler(Reference reference, ObjectAdapter adapter)
        {
            _reference = reference;
            _adapter = adapter;
            _requestId = 0;
        }

        private async Task<IncomingResponseFrame?> InvokeAllAsync(
            OutgoingRequestFrame outgoingRequest,
            int requestId,
            CancellationToken cancel)
        {
            //
            // Increase the direct count to prevent the object adapter from being destroyed while the dispatch is
            // in progress. This will also throw if the object adapter has been deactivated.
            //
            _adapter.IncDirectCount();

            IDispatchObserver? dispatchObserver = null;
            try
            {
                var incomingRequest = new IncomingRequestFrame(_adapter.Communicator, outgoingRequest);
                var current = new Current(_adapter, incomingRequest, oneway: requestId == 0, cancel);

                // Then notify and set dispatch observer, if any.
                ICommunicatorObserver? communicatorObserver = _adapter.Communicator.Observer;
                if (communicatorObserver != null)
                {
                    dispatchObserver = communicatorObserver.GetDispatchObserver(current,
                                                                                requestId,
                                                                                incomingRequest.Size);
                    dispatchObserver?.Attach();
                }

                OutgoingResponseFrame? outgoingResponseFrame = null;
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
                        // We don't use the cancelable WaitAsync for the await here. The asynchronous dispatch is
                        // not completed yet and we want to make sure the observer is detached only when the dispatch
                        // completes, not when the caller cancels the request.
                        outgoingResponseFrame = await vt.ConfigureAwait(false);
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
                        outgoingResponseFrame = new OutgoingResponseFrame(current, actualEx);
                    }
                }

                if (outgoingResponseFrame != null)
                {
                    dispatchObserver?.Reply(outgoingResponseFrame.Size);

                    var incomingResponseFrame =
                        new IncomingResponseFrame(_adapter.Communicator,
                                                  outgoingResponseFrame.Protocol,
                                                  VectoredBufferExtensions.ToArray(outgoingResponseFrame!.Data));
                    if (_adapter.Communicator.TraceLevels.Protocol >= 1)
                    {
                        ProtocolTrace.TraceCollocatedFrame(_adapter.Communicator,
                                                           (byte)Ice1Definitions.FrameType.Reply,
                                                           requestId,
                                                           incomingResponseFrame);
                    }
                    return incomingResponseFrame;
                }
                else
                {
                    return null;
                }
            }
            finally
            {
                dispatchObserver?.Detach();
                _adapter.DecDirectCount();
            }
        }
    }
}
