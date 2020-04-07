//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceInternal;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace Ice
{
    // publicly visible Ice-internal class used as base class for requests
    public class OutgoingRequest<TReturnValue>
    {
        private readonly InputStreamReader<TReturnValue> _reader;

        private protected OutgoingRequest(InputStreamReader<TReturnValue> reader) => _reader = reader;

        private protected TReturnValue Invoke(IObjectPrx prx, OutgoingRequestFrame request)
        {
            try
            {
                var completed = new IObjectPrx.InvokeTaskCompletionCallback(null, default);
                new OutgoingAsync(prx, completed, request, oneway: false).Invoke(
                    request.Operation, request.Context, synchronous: true);
                IncomingResponseFrame response = completed.Task.Result;
                return response.ReadReturnValue(_reader);
            }
            catch (AggregateException ex)
            {
                Debug.Assert(ex.InnerException != null);
                throw ex.InnerException;
            }
        }

        private protected Task<TReturnValue> InvokeAsync(IObjectPrx prx,
                                                         OutgoingRequestFrame request,
                                                         IProgress<bool>? progress,
                                                 CancellationToken cancel)
        {
            var completed = new IObjectPrx.InvokeTaskCompletionCallback(progress, cancel);
            new OutgoingAsync(prx, completed, request, oneway: false).Invoke(
                request.Operation, request.Context, synchronous: false);

            return ReadReturnValueAsync(completed.Task, _reader);

            static async Task<TReturnValue> ReadReturnValueAsync(Task<IncomingResponseFrame> task,
                                                                 InputStreamReader<TReturnValue> reader)
            {
                IncomingResponseFrame response = await task.ConfigureAwait(false);
                return response.ReadReturnValue(reader);
            }
        }
    }

    // publicly visible Ice-internal class used to make request without input parameters.
    public sealed class OutgoingRequestWithEmptyParamList<TReturnValue>
        : OutgoingRequest<TReturnValue>
    {
        private readonly string _operationName;
        private readonly bool _idempotent;

        public OutgoingRequestWithEmptyParamList(string operationName,
                                                 bool idempotent,
                                                 InputStreamReader<TReturnValue> reader)
            : base(reader)
        {
            _operationName = operationName;
            _idempotent = idempotent;
        }

        public TReturnValue Invoke(IObjectPrx prx, IReadOnlyDictionary<string, string>? context) =>
            Invoke(prx, OutgoingRequestFrame.WithEmptyParamList(prx, _operationName, _idempotent, context));

        public Task<TReturnValue> InvokeAsync(IObjectPrx prx,
                                            IReadOnlyDictionary<string, string>? context,
                                            IProgress<bool>? progress,
                                            CancellationToken cancel) =>
            InvokeAsync(prx, OutgoingRequestFrame.WithEmptyParamList(prx, _operationName, _idempotent, context),
                progress, cancel);
    }

    // publicly visible Ice-internal type used to make request with input and output parameters,
    // the input parameters are pass without the `in' modifier, this helper is used when there is a single
    // input parameter and the parameter is of a non structure type.
    public sealed class OutgoingRequestWithParam<TParamList, TReturnValue>
        : OutgoingRequest<TReturnValue>
    {
        private readonly string _operationName;
        private readonly bool _idempotent;
        private readonly FormatType? _format;
        private readonly OutputStreamWriter<TParamList> _writer;

        public OutgoingRequestWithParam(string operationName,
                                        bool idempotent,
                                        FormatType? format,
                                        OutputStreamWriter<TParamList> writer,
                                        InputStreamReader<TReturnValue> reader)
            : base(reader)
        {
            _operationName = operationName;
            _idempotent = idempotent;
            _format = format;
            _writer = writer;
        }

        public TReturnValue Invoke(IObjectPrx prx, TParamList paramList, IReadOnlyDictionary<string, string>? context) =>
            Invoke(prx, OutgoingRequestFrame.WithParamList(prx, _operationName, _idempotent, _format, context,
                                                           paramList, _writer));

        public Task<TReturnValue> InvokeAsync(IObjectPrx prx,
                                            TParamList paramList,
                                            IReadOnlyDictionary<string, string>? context,
                                            IProgress<bool>? progress,
                                            CancellationToken cancel) =>
            InvokeAsync(prx, OutgoingRequestFrame.WithParamList(
                prx, _operationName, _idempotent, _format, context, paramList, _writer), progress, cancel);
    }

    // publicly visible Ice-internal class used to make request with input and output parameters,
    // the input parameters are pass with the `in' modifier, this helper is used when input parameter
    // is an structure type, or when there is several input parameters as we pass them wrapped in a
    // tuple.
    public sealed class OutgoingRequestWithStructParam<TParamList, TReturnValue>
        : OutgoingRequest<TReturnValue> where TParamList : struct
    {
        private readonly string _operationName;
        private readonly bool _idempotent;
        private readonly FormatType? _format;
        private readonly OutputStreamStructWriter<TParamList> _writer;

        public OutgoingRequestWithStructParam(string operationName,
                                              bool idempotent,
                                              FormatType? format,
                                              OutputStreamStructWriter<TParamList> writer,
                                              InputStreamReader<TReturnValue> reader)
            : base(reader)
        {
            _operationName = operationName;
            _idempotent = idempotent;
            _format = format;
            _writer = writer;
        }

        public TReturnValue Invoke(IObjectPrx prx, in TParamList paramList, IReadOnlyDictionary<string, string>? context) =>
            Invoke(prx, OutgoingRequestFrame.WithParamList(
                prx, _operationName, _idempotent, _format, context, paramList, _writer));

        public Task<TReturnValue> InvokeAsync(IObjectPrx prx,
                                            in TParamList paramList,
                                            IReadOnlyDictionary<string, string>? context,
                                            IProgress<bool>? progress,
                                            CancellationToken cancel) =>
            InvokeAsync(prx, OutgoingRequestFrame.WithParamList(prx, _operationName, _idempotent, _format,
                                                                context, paramList, _writer),
                progress, cancel);
    }

    // publicly visible Ice-internal class used as base class for void requests
    public class OutgoingRequest
    {
        private protected void Invoke(IObjectPrx prx, OutgoingRequestFrame request)
        {
            try
            {
                var completed = new IObjectPrx.InvokeTaskCompletionCallback(null, default);
                new OutgoingAsync(prx, completed, request, oneway: prx.IsOneway).Invoke(
                    request.Operation, request.Context, synchronous: true);
                IncomingResponseFrame response = completed.Task.Result;
                if (!prx.IsOneway)
                {
                    response.ReadVoidReturnValue();
                }
            }
            catch (AggregateException ex)
            {
                Debug.Assert(ex.InnerException != null);
                throw ex.InnerException;
            }
        }

        private protected Task InvokeAsync(IObjectPrx prx, OutgoingRequestFrame request, IProgress<bool>? progress,
                                           CancellationToken cancel)
        {
            var completed = new IObjectPrx.InvokeTaskCompletionCallback(progress, cancel);
            new OutgoingAsync(prx, completed, request, oneway: prx.IsOneway).Invoke(
                request.Operation, request.Context, synchronous: false);

            return ReadVoidReturnValueAsync(prx, completed.Task);

            static async Task ReadVoidReturnValueAsync(IObjectPrx prx, Task<IncomingResponseFrame> task)
            {
                IncomingResponseFrame response = await task.ConfigureAwait(false);
                if (!prx.IsOneway)
                {
                    response.ReadVoidReturnValue();
                }
            }
        }
    }

    // publicly visible Ice-internal class used to make void requests without input parameters.
    public class OutgoingRequestWithEmptyParamList : OutgoingRequest
    {
        private readonly string _operationName;
        private readonly bool _idempotent;

        public OutgoingRequestWithEmptyParamList(string operationName, bool idempotent)
        {
            _operationName = operationName;
            _idempotent = idempotent;
        }

        public void Invoke(IObjectPrx prx, IReadOnlyDictionary<string, string>? context) =>
            Invoke(prx, OutgoingRequestFrame.WithEmptyParamList(prx, _operationName, _idempotent, context));

        public Task InvokeAsync(IObjectPrx prx,
                                IReadOnlyDictionary<string, string>? context,
                                IProgress<bool>? progress,
                                CancellationToken cancel) =>
            InvokeAsync(prx, OutgoingRequestFrame.WithEmptyParamList(prx, _operationName, _idempotent, context),
                progress, cancel);
    }

    // publicly visible Ice-internal class used to make void requests with input parameters, the input parameters
    // are pass without the `in' modifier, this helper is used when there is a single input parameter and the parameter
    // is not a structure type.
    public sealed class OutgoingRequestWithParam<TParamList> : OutgoingRequest
    {
        private readonly string _operationName;
        private readonly bool _idempotent;
        private readonly FormatType? _format;
        private readonly OutputStreamWriter<TParamList> _writer;

        public OutgoingRequestWithParam(
            string operationName,
            bool idempotent,
            FormatType? format,
            OutputStreamWriter<TParamList> writer)
        {
            _operationName = operationName;
            _idempotent = idempotent;
            _format = format;
            _writer = writer;
        }

        public void Invoke(IObjectPrx prx, TParamList paramList, IReadOnlyDictionary<string, string>? context) =>
            Invoke(prx, OutgoingRequestFrame.WithParamList(prx, _operationName, _idempotent, _format, context,
                paramList, _writer));

        public Task InvokeAsync(IObjectPrx prx, TParamList paramList,
                                IReadOnlyDictionary<string, string>? context,
                                IProgress<bool>? progress,
                                CancellationToken cancel) =>
            InvokeAsync(prx, OutgoingRequestFrame.WithParamList(
                prx, _operationName, _idempotent, _format, context, paramList, _writer), progress, cancel);
    }

    // publicly visible Ice-internal type used to make void request with input parameters,
    // the input parameters are pass with the `in' modifier, this helper is used when input parameter
    // is a structure type, or when there is several input parameters as we pass them wrapped in a tuple.
    public sealed class OutgoingRequestWithStructParam<TParamList>
        : OutgoingRequest where TParamList : struct
    {
        private readonly string _operationName;
        private readonly bool _idempotent;
        private readonly FormatType? _format;
        private readonly OutputStreamStructWriter<TParamList> _writer;

        public OutgoingRequestWithStructParam(
            string operationName,
            bool idempotent,
            FormatType? format,
            OutputStreamStructWriter<TParamList> writer)
        {
            _operationName = operationName;
            _idempotent = idempotent;
            _format = format;
            _writer = writer;
        }

        public void Invoke(IObjectPrx prx, in TParamList paramList, IReadOnlyDictionary<string, string>? context) =>
            Invoke(prx, OutgoingRequestFrame.WithParamList(
                prx, _operationName, _idempotent, _format, context, paramList, _writer));

        public Task InvokeAsync(IObjectPrx prx, in TParamList paramList,
                                IReadOnlyDictionary<string, string>? context,
                                IProgress<bool>? progress,
                                CancellationToken cancel) =>
            InvokeAsync(prx, OutgoingRequestFrame.WithParamList(
                prx, _operationName, _idempotent, _format, context, paramList, _writer), progress, cancel);
    }
}
