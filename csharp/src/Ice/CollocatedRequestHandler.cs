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

        public async ValueTask<IncomingResponseFrame> SendRequestAsync(
            OutgoingRequestFrame outgoingRequest,
            bool oneway,
            bool synchronous,
            IInvocationObserver? observer,
            IProgress<bool> progress,
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
                    childObserver = observer.GetCollocatedObserver(_adapter, requestId, outgoingRequest.Size);
                    childObserver?.Attach();
                }

                if (_adapter.Communicator.TraceLevels.Protocol >= 1)
                {
                    ProtocolTrace.TraceCollocatedFrame(_adapter.Communicator,
                                                       (byte)Ice1Definitions.FrameType.Request,
                                                       requestId,
                                                       outgoingRequest);
                }
            }

            Task<OutgoingResponseFrame> task;
            if (_adapter.TaskScheduler != null || !synchronous || oneway || _reference.InvocationTimeout > 0)
            {
                // Don't invoke from the user thread if async or invocation timeout is set. We also don't dispatch
                // oneway from the user thread to match the non-collocated behavior where the oneway synchronous
                // request returns as soon as it's sent over the transport.
                task = Task.Factory.StartNew(() =>
                                             {
                                                progress.Report(false);
                                                return DispatchAsync(outgoingRequest, requestId, cancel);
                                             },
                                             cancel,
                                             TaskCreationOptions.None,
                                             _adapter.TaskScheduler ?? TaskScheduler.Default).Unwrap();

                if (oneway)
                {
                    childObserver?.Detach();
                    return IncomingResponseFrame.WithVoidReturnValue(outgoingRequest.Protocol,
                                                                     outgoingRequest.Encoding);
                }
            }
            else // Optimization: directly call DispatchAsync
            {
                Debug.Assert(!oneway);
                progress.Report(false);
                task = DispatchAsync(outgoingRequest, requestId, cancel);
            }

            try
            {
                OutgoingResponseFrame outgoingResponseFrame = await task.WaitAsync(cancel).ConfigureAwait(false);

                var incomingResponseFrame = new IncomingResponseFrame(
                    outgoingRequest.Protocol,
                    VectoredBufferExtensions.ToArray(outgoingResponseFrame.Data));

                if (_adapter.Communicator.TraceLevels.Protocol >= 1)
                {
                    ProtocolTrace.TraceCollocatedFrame(_adapter.Communicator,
                                                        (byte)Ice1Definitions.FrameType.Reply,
                                                        requestId,
                                                        incomingResponseFrame);
                }

                childObserver?.Reply(incomingResponseFrame.Size);
                return incomingResponseFrame;
            }
            catch (Exception ex)
            {
                childObserver?.Failed(ex.GetType().FullName ?? "System.Exception");
                throw;
            }
            finally
            {
                childObserver?.Detach();
            }
        }

        internal CollocatedRequestHandler(Reference reference, ObjectAdapter adapter)
        {
            _reference = reference;
            _adapter = adapter;
            _requestId = 0;
        }

        private async Task<OutgoingResponseFrame> DispatchAsync(
            OutgoingRequestFrame outgoingRequest,
            int requestId,
            CancellationToken cancel)
        {
            // Increase the direct count to prevent the object adapter from being destroyed while the dispatch is in
            // progress. This will also throw if the object adapter has been deactivated.
            _adapter.IncDirectCount();

            IDispatchObserver? dispatchObserver = null;
            try
            {
                var incomingRequest = new IncomingRequestFrame(outgoingRequest);
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
                        dispatchObserver?.Reply(outgoingResponseFrame.Size);
                    }
                }
                catch (Exception ex)
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

                    if (requestId != 0)
                    {
                        outgoingResponseFrame = new OutgoingResponseFrame(incomingRequest, actualEx);
                        dispatchObserver?.Reply(outgoingResponseFrame.Size);
                    }
                }

                return outgoingResponseFrame ?? OutgoingResponseFrame.WithVoidReturnValue(current);
            }
            finally
            {
                dispatchObserver?.Detach();
                _adapter.DecDirectCount();
            }
        }
    }
}
