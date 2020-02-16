//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace IceInternal
{
    public class CollocatedRequestHandler : IRequestHandler, IResponseHandler
    {
        private void
        FillInValue(Ice.OutputStream os, int pos, int value) => os.RewriteInt(value, pos);

        public
        CollocatedRequestHandler(Reference @ref, Ice.ObjectAdapter adapter)
        {
            _reference = @ref;
            _response = _reference.GetMode() == Ice.InvocationMode.Twoway;
            _adapter = adapter;

            _logger = _reference.GetCommunicator().Logger; // Cached for better performance.
            _traceLevels = _reference.GetCommunicator().TraceLevels; // Cached for better performance.
            _requestId = 0;
        }

        public IRequestHandler? Update(IRequestHandler previousHandler, IRequestHandler? newHandler) =>
            previousHandler == this ? newHandler : this;

        public int SendAsyncRequest(ProxyOutgoingAsyncBase outAsync) => outAsync.InvokeCollocated(this);

        public void AsyncRequestCanceled(OutgoingAsyncBase outAsync, Ice.LocalException ex)
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
                        outAsync.InvokeExceptionAsync();
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
                                outAsync.InvokeExceptionAsync();
                            }
                            return;
                        }
                    }
                }
            }
        }

        public void SendResponse(int requestId, Ice.OutputStream os, byte status, bool amd)
        {
            OutgoingAsyncBase? outAsync;
            lock (this)
            {
                Debug.Assert(_response);

                if (_traceLevels.Protocol >= 1)
                {

                    FillInValue(os, 10, os.Size);
                }

                // Adopt the OutputStream's buffer.
                var iss = new Ice.InputStream(os.Communicator, os.Encoding, os.GetBuffer(), true);

                iss.Pos = Protocol.replyHdr.Length + 4;

                if (_traceLevels.Protocol >= 1)
                {
                    TraceUtil.TraceRecv(iss, _logger, _traceLevels);
                }

                if (_asyncRequests.TryGetValue(requestId, out outAsync))
                {
                    outAsync.GetIs().Swap(iss);
                    if (!outAsync.Response())
                    {
                        outAsync = null;
                    }
                    _asyncRequests.Remove(requestId);
                }
            }

            if (outAsync != null)
            {
                if (amd)
                {
                    outAsync.InvokeResponseAsync();
                }
                else
                {
                    outAsync.InvokeResponse();
                }
            }
            _adapter.DecDirectCount();
        }

        public void SendNoResponse() => _adapter.DecDirectCount();

        public bool
        SystemException(int requestId, Ice.SystemException ex, bool amd)
        {
            HandleException(requestId, ex, amd);
            _adapter.DecDirectCount();
            return true;
        }

        public void
        InvokeException(int requestId, Ice.LocalException ex, int invokeNum, bool amd)
        {
            HandleException(requestId, ex, amd);
            _adapter.DecDirectCount();
        }

        public Reference GetReference() => _reference;

        public Ice.Connection? GetConnection() => null;

        public int InvokeAsyncRequest(OutgoingAsyncBase outAsync, bool synchronous)
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

                    if (_response)
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

            outAsync.AttachCollocatedObserver(_adapter, requestId);
            if (!synchronous || !_response || _reference.GetInvocationTimeout() > 0)
            {
                // Don't invoke from the user thread if async or invocation timeout is set
                _adapter.ThreadPool.Dispatch(
                    () =>
                    {
                        if (SentAsync(outAsync))
                        {
                            InvokeAll(outAsync.GetOs(), requestId);
                        }
                    });
            }
            else // Optimization: directly call invokeAll
            {
                if (SentAsync(outAsync))
                {
                    InvokeAll(outAsync.GetOs(), requestId);
                }
            }
            return OutgoingAsyncBase.AsyncStatusQueued;
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
            outAsync.InvokeSent();
            return true;
        }

        private void InvokeAll(Ice.OutputStream os, int requestId)
        {
            if (_traceLevels.Protocol >= 1)
            {
                FillInValue(os, 10, os.Size);
                if (requestId > 0)
                {
                    FillInValue(os, Protocol.headerSize, requestId);
                }
                TraceUtil.TraceSend(os, _logger, _traceLevels);
            }

            var iss = new Ice.InputStream(os.Communicator, os.Encoding, os.GetBuffer(), false);
            iss.Pos = Protocol.requestHdr.Length;

            int invokeNum = 1;
            try
            {
                while (invokeNum > 0)
                {
                    //
                    // Increase the direct count for the dispatch. We increase it again here for
                    // each dispatch. It's important for the direct count to be > 0 until the last
                    // collocated request response is sent to make sure the thread pool isn't
                    // destroyed before.
                    //
                    try
                    {
                        _adapter.IncDirectCount();
                    }
                    catch (Ice.ObjectAdapterDeactivatedException ex)
                    {
                        HandleException(requestId, ex, false);
                        break;
                    }

                    Incoming.Invoke(_reference.GetCommunicator(), this, null, _adapter, 0, requestId, iss);
                    --invokeNum;
                }
            }
            catch (Ice.LocalException ex)
            {
                InvokeException(requestId, ex, invokeNum, false); // Fatal invocation exception
            }

            _adapter.DecDirectCount();
        }

        private void
        HandleException(int requestId, Ice.Exception ex, bool amd)
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
                //
                // If called from an AMD dispatch, invoke asynchronously
                // the completion callback since this might be called from
                // the user code.
                //
                if (amd)
                {
                    outAsync.InvokeExceptionAsync();
                }
                else
                {
                    outAsync.InvokeException();
                }
            }
        }

        private readonly Reference _reference;

        private readonly bool _response;
        private readonly Ice.ObjectAdapter _adapter;
        private readonly Ice.ILogger _logger;
        private readonly TraceLevels _traceLevels;

        private int _requestId;

        private readonly Dictionary<OutgoingAsyncBase, int> _sendAsyncRequests = new Dictionary<OutgoingAsyncBase, int>();
        private readonly Dictionary<int, OutgoingAsyncBase> _asyncRequests = new Dictionary<int, OutgoingAsyncBase>();
    }
}
