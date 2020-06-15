//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

using ZeroC.Ice.Instrumentation;

namespace ZeroC.Ice
{
    internal class CollocatedRequestHandler : IRequestHandler
    {
        public
        CollocatedRequestHandler(Reference reference, ObjectAdapter adapter)
        {
            _reference = reference;
            _adapter = adapter;
            _requestId = 0;
        }

        public Connection? GetConnection() => null;

        public ValueTask<Task<IncomingResponseFrame>?> SendRequestAsync(OutgoingRequestFrame outgoingRequestFrame,
            bool oneway, bool synchronous, IInvocationObserver? observer)
        {
            //
            // Increase the direct count to prevent the thread pool from being destroyed before
            // invokeAll is called. This will also throw if the object adapter has been deactivated.
            //
            _adapter.IncDirectCount();

            IChildInvocationObserver? childObserver = null;
            int requestId = 0;
            lock (this)
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
                    ProtocolTrace.TraceCollocatedFrame(_adapter.Communicator, (byte)Ice1Definitions.FrameType.Request,
                        requestId, outgoingRequestFrame);
                }
            }

            Task<IncomingResponseFrame?> task;
            if (_adapter.TaskScheduler != null || !synchronous || oneway || _reference.InvocationTimeout > 0)
            {
                // Don't invoke from the user thread if async or invocation timeout is set. We also don't dispatch
                // oneway from the user thread to match the non-collocated behavior where the oneway synchronous
                // request returns as soon as it's sent over the transport.
                task = Task.Factory.StartNew(() => InvokeAllAsync(outgoingRequestFrame, requestId), default,
                    TaskCreationOptions.None, _adapter.TaskScheduler ?? TaskScheduler.Default).Unwrap();

                if (oneway)
                {
                    childObserver?.Detach();
                    return default;
                }
            }
            else // Optimization: directly call invokeAll
            {
                task = InvokeAllAsync(outgoingRequestFrame, requestId);
            }
            return new ValueTask<Task<IncomingResponseFrame>?>(WaitForResponseAsync(task, childObserver));

            static async Task<IncomingResponseFrame> WaitForResponseAsync(Task<IncomingResponseFrame?> task,
                IChildInvocationObserver? observer)
            {
                try
                {
                    IncomingResponseFrame? incomingResponseFrame = await task.ConfigureAwait(false);
                    observer?.Reply(incomingResponseFrame!.Size);
                    return incomingResponseFrame!;
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

        private async Task<IncomingResponseFrame?> InvokeAllAsync(OutgoingRequestFrame outgoingRequest,
            int requestId)
        {
            // The object adapter DirectCount was incremented by the caller and we are responsible to decrement it
            // upon completion.

            IDispatchObserver? dispatchObserver = null;
            try
            {
                var incomingRequest = new IncomingRequestFrame(_adapter.Communicator, outgoingRequest);
                var current = new Current(_adapter, incomingRequest, requestId);

                // Then notify and set dispatch observer, if any.
                ICommunicatorObserver? communicatorObserver = _adapter.Communicator.Observer;
                if (communicatorObserver != null)
                {
                    dispatchObserver = communicatorObserver.GetDispatchObserver(current, incomingRequest.Size);
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

                    var incomingResponseFrame = new IncomingResponseFrame(_adapter.Communicator,
                        VectoredBufferExtensions.ToArray(outgoingResponseFrame!.Data));
                    if (_adapter.Communicator.TraceLevels.Protocol >= 1)
                    {
                        ProtocolTrace.TraceCollocatedFrame(_adapter.Communicator, (byte)Ice1Definitions.FrameType.Reply,
                            requestId, incomingResponseFrame);
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

        private readonly Reference _reference;
        private readonly ObjectAdapter _adapter;
        private int _requestId;
    }
}
