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

        public async Task<IncomingResponseFrame> SendRequestAsync(
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
                    ProtocolTrace.TraceFrame(_adapter.Communicator, requestId, outgoingRequest);
                }
            }

            Task<OutgoingResponseFrame> task;
            if (_adapter.TaskScheduler != null ||
                !synchronous ||
                oneway ||
                cancel != CancellationToken.None)
            {
                // Don't invoke from the user thread if async or cancellation token is set. We also don't dispatch
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
                progress.Report(false);
                task = DispatchAsync(outgoingRequest, requestId, cancel);
            }

            Debug.Assert(!oneway);
            try
            {
                OutgoingResponseFrame outgoingResponseFrame = await task.WaitAsync(cancel).ConfigureAwait(false);

                var incomingResponse = new IncomingResponseFrame(
                    outgoingRequest.Protocol,
                    outgoingResponseFrame.Data.AsArraySegment(),
                    _adapter.IncomingFrameSizeMax);

                if (_adapter.Communicator.TraceLevels.Protocol >= 1)
                {
                    ProtocolTrace.TraceFrame(_adapter.Communicator, requestId, incomingResponse);
                }

                childObserver?.Reply(incomingResponse.Size);
                return incomingResponse;
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
            try
            {
                var incomingRequest = new IncomingRequestFrame(outgoingRequest, _adapter.IncomingFrameSizeMax);
                var current = new Current(_adapter, incomingRequest, oneway: requestId == 0, cancel);
                return await _adapter.DispatchAsync(incomingRequest, requestId, current).ConfigureAwait(false);
            }
            finally
            {
                _adapter.DecDirectCount();
            }
        }
    }
}
