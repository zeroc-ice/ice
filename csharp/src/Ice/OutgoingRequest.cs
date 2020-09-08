//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>publicly visible Ice-internal class used as base class for requests.</summary>
    /// <typeparam name="TReturnValue">The return value type.</typeparam>
    public class OutgoingRequest<TReturnValue>
    {
        private readonly InputStreamReader<TReturnValue> _reader;
        private protected readonly bool _compress;

        private protected OutgoingRequest(InputStreamReader<TReturnValue> reader, bool compress)
        {
            _reader = reader;
            _compress = compress;
        }

        private protected TReturnValue Invoke(IObjectPrx prx, OutgoingRequestFrame request, CancellationToken cancel) =>
            prx.Invoke(request, oneway: false, cancel).ReadReturnValue(prx.Communicator, _reader);

        private protected Task<TReturnValue> InvokeAsync(
            IObjectPrx prx,
            OutgoingRequestFrame request,
            IProgress<bool>? progress,
            CancellationToken cancel)
        {
            return ReadReturnValueAsync(prx.InvokeAsync(request, oneway: false, progress, cancel),
                                        prx.Communicator,
                                        _reader);

            static async Task<TReturnValue> ReadReturnValueAsync(
                ValueTask<IncomingResponseFrame> task,
                Communicator communicator,
                InputStreamReader<TReturnValue> reader)
            {
                IncomingResponseFrame response = await task.ConfigureAwait(false);
                return response.ReadReturnValue(communicator, reader);
            }
        }
    }

    /// <summary>Publicly visible Ice-internal class used to make request without input parameters.</summary>
    /// <typeparam name="TReturnValue">The return value type.</typeparam>
    public sealed class OutgoingRequestWithEmptyParamList<TReturnValue>
        : OutgoingRequest<TReturnValue>
    {
        private readonly string _operationName;
        private readonly bool _idempotent;

        /// <summary>Constructs a new request.</summary>
        /// <param name="operationName">The request operation name.</param>
        /// <param name="idempotent"><c>True</c> if the request is idempotent, <c>False</c> otherwise.</param>
        /// <param name="reader">The <see cref="InputStream"/> reader used to read the request return value.</param>
        public OutgoingRequestWithEmptyParamList(
            string operationName,
            bool idempotent,
            InputStreamReader<TReturnValue> reader)
            : base(reader, compress: false)
        {
            _operationName = operationName;
            _idempotent = idempotent;
        }

        /// <summary>Invoke the request synchronously.</summary>
        /// <param name="prx">The proxy used to send the request.</param>
        /// <param name="context">The Ice request context.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The request return value.</returns>
        public TReturnValue Invoke(
            IObjectPrx prx,
            IReadOnlyDictionary<string, string>? context,
            CancellationToken cancel) =>
            Invoke(prx, OutgoingRequestFrame.WithEmptyParamList(prx, _operationName, _idempotent, context), cancel);

        /// <summary>Invoke the request asynchronously.</summary>
        /// <param name="prx">The proxy used to send the request.</param>
        /// <param name="context">The Ice request context.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The request return value.</returns>
        public Task<TReturnValue> InvokeAsync(
            IObjectPrx prx,
            IReadOnlyDictionary<string, string>? context,
            IProgress<bool>? progress,
            CancellationToken cancel) =>
            InvokeAsync(prx, OutgoingRequestFrame.WithEmptyParamList(prx, _operationName, _idempotent, context),
                progress, cancel);
    }

    /// <summary>Publicly visible Ice-internal type used to make request with input and output parameters, the input
    /// parameters are pass without the `in' modifier, this helper is used when there is a single input parameter and
    /// the parameter is of a non structure type.</summary>
    /// <typeparam name="TParamList">The input parameters type.</typeparam>
    /// <typeparam name="TReturnValue">The return value type</typeparam>
    public sealed class OutgoingRequestWithParam<TParamList, TReturnValue>
        : OutgoingRequest<TReturnValue>
    {
        private readonly bool _idempotent;
        private readonly string _operationName;
        private readonly FormatType _format;
        private readonly OutputStreamWriter<TParamList> _writer;

        /// <summary>Constructs a new request.</summary>
        /// <param name="operationName">The request operation name.</param>
        /// <param name="idempotent"><c>True</c> if the request is idempotent, <c>False</c> otherwise.</param>
        /// <param name="compress"><c>True</c> if the request params must use protocol compression, <c>False</c>
        /// otherwise.</param>
        /// <param name="format">The format used for marshal class parameters.</param>
        /// <param name="writer">The <see cref="OutputStream"/> writer used to write the request input parameters.</param>
        /// <param name="reader">The <see cref="InputStream"/> reader used to read the request return value.</param>
        public OutgoingRequestWithParam(
            string operationName,
            bool idempotent,
            bool compress,
            FormatType format,
            OutputStreamWriter<TParamList> writer,
            InputStreamReader<TReturnValue> reader)
            : base(reader, compress)
        {
            _operationName = operationName;
            _idempotent = idempotent;
            _format = format;
            _writer = writer;
        }

        /// <summary>Invoke the request synchronously.</summary>
        /// <param name="prx">The proxy used to send the request.</param>
        /// <param name="paramList">The request input parameters.</param>
        /// <param name="context">The Ice request context.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The request return value.</returns>
        public TReturnValue Invoke(
            IObjectPrx prx,
            TParamList paramList,
            IReadOnlyDictionary<string, string>? context,
            CancellationToken cancel) =>
            Invoke(prx,
                   OutgoingRequestFrame.WithParamList(prx,
                                                      _operationName,
                                                      _idempotent,
                                                      _compress,
                                                      _format,
                                                      context,
                                                      paramList,
                                                      _writer),
                   cancel);

        /// <summary>Invoke the request asynchronously.</summary>
        /// <param name="prx">The proxy used to send the request.</param>
        /// <param name="paramList">The request input parameters.</param>
        /// <param name="context">The Ice request context.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The request return value.</returns>
        public Task<TReturnValue> InvokeAsync(
            IObjectPrx prx,
            TParamList paramList,
            IReadOnlyDictionary<string, string>? context,
            IProgress<bool>? progress,
            CancellationToken cancel) =>
            InvokeAsync(prx,
                        OutgoingRequestFrame.WithParamList(prx,
                                                           _operationName,
                                                           _idempotent,
                                                           _compress,
                                                           _format,
                                                           context,
                                                           paramList,
                                                           _writer),
                        progress,
                        cancel);
    }

    /// <summary>Publicly visible Ice-internal class used to make request with input and output parameters, the input
    /// parameters are pass with the `in' modifier, this helper is used when input parameter is an structure type, or
    /// when there is several input parameters as we pass them wrapped in a tuple.</summary>
    /// <typeparam name="TParamList">The input parameters type.</typeparam>
    /// <typeparam name="TReturnValue">The return value type</typeparam>
    public sealed class OutgoingRequestWithStructParam<TParamList, TReturnValue>
        : OutgoingRequest<TReturnValue> where TParamList : struct
    {
        private readonly string _operationName;
        private readonly bool _idempotent;
        private readonly FormatType _format;
        private readonly OutputStreamValueWriter<TParamList> _writer;

        /// <summary>Constructs a new request.</summary>
        /// <param name="operationName">The request operation name.</param>
        /// <param name="idempotent"><c>True</c> if the request is idempotent, <c>False</c> otherwise.</param>
        /// <param name="compress"><c>True</c> if the request params must use protocol compression, <c>False</c>
        /// otherwise.</param>
        /// <param name="format">The format used for marshal class parameters.</param>
        /// <param name="writer">The <see cref="OutputStream"/> writer used to write the request input parameters.</param>
        /// <param name="reader">The <see cref="InputStream"/> reader used to read the request return value.</param>
        public OutgoingRequestWithStructParam(
            string operationName,
            bool idempotent,
            bool compress,
            FormatType format,
            OutputStreamValueWriter<TParamList> writer,
            InputStreamReader<TReturnValue> reader)
            : base(reader, compress)
        {
            _operationName = operationName;
            _idempotent = idempotent;
            _format = format;
            _writer = writer;
        }

        /// <summary>Invoke the request synchronously.</summary>
        /// <param name="prx">The proxy used to send the request.</param>
        /// <param name="paramList">The request input parameters.</param>
        /// <param name="context">The Ice request context.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The request return value.</returns>
        public TReturnValue Invoke(
            IObjectPrx prx,
            in TParamList paramList,
            IReadOnlyDictionary<string, string>? context,
            CancellationToken cancel) =>
            Invoke(prx,
                   OutgoingRequestFrame.WithParamList(prx,
                                                      _operationName,
                                                      _idempotent,
                                                      _compress,
                                                      _format,
                                                      context,
                                                      paramList,
                                                      _writer),
                   cancel);

        /// <summary>Invoke the request asynchronously.</summary>
        /// <param name="prx">The proxy used to send the request.</param>
        /// <param name="paramList">The request input parameters.</param>
        /// <param name="context">The Ice request context.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The request return value.</returns>
        public Task<TReturnValue> InvokeAsync(
            IObjectPrx prx,
            in TParamList paramList,
            IReadOnlyDictionary<string, string>? context,
            IProgress<bool>? progress,
            CancellationToken cancel) =>
            InvokeAsync(prx,
                        OutgoingRequestFrame.WithParamList(prx,
                                                           _operationName,
                                                           _idempotent,
                                                           _compress,
                                                           _format,
                                                           context,
                                                           paramList,
                                                           _writer),
                        progress,
                        cancel);
    }

    /// <summary>Publicly visible Ice-internal class used as base class for void requests.</summary>
    public class OutgoingRequest
    {
        private readonly bool _oneway;
        private protected readonly bool _compress;

        private protected OutgoingRequest(bool oneway, bool compress)
        {
            _oneway = oneway;
            _compress = compress;
        }

        private protected void Invoke(IObjectPrx prx, OutgoingRequestFrame request, CancellationToken cancel)
        {
            bool isOneway = _oneway || prx.IsOneway;
            IncomingResponseFrame response = prx.Invoke(request, isOneway, cancel);
            if (!isOneway)
            {
                response.ReadVoidReturnValue(prx.Communicator);
            }
        }

        private protected Task InvokeAsync(
            IObjectPrx prx,
            OutgoingRequestFrame request,
            IProgress<bool>? progress,
            CancellationToken cancel)
        {
            bool isOneway = _oneway || prx.IsOneway;
            return ReadVoidReturnValueAsync(prx.InvokeAsync(request, isOneway, progress, cancel),
                                            prx.Communicator,
                                            isOneway);

            static async Task ReadVoidReturnValueAsync(
                ValueTask<IncomingResponseFrame> task,
                Communicator communicator,
                bool oneway)
            {
                IncomingResponseFrame response = await task.ConfigureAwait(false);
                if (!oneway)
                {
                    response.ReadVoidReturnValue(communicator);
                }
            }
        }
    }

    /// <summary>Publicly visible Ice-internal class used to make void requests without input parameters.</summary>
    public class OutgoingRequestWithEmptyParamList : OutgoingRequest
    {
        private readonly string _operationName;
        private readonly bool _idempotent;

        /// <summary>Constructs a new request.</summary>
        /// <param name="operationName">The request operation name.</param>
        /// <param name="idempotent"><c>True</c> if the request is idempotent, <c>False</c> otherwise.</param>
        /// <param name="oneway"><c>True</c> if the request use oneway invocation mode, <c>False</c> otherwise.</param>
        public OutgoingRequestWithEmptyParamList(string operationName, bool idempotent, bool oneway)
            : base(oneway, compress: false)
        {
            _operationName = operationName;
            _idempotent = idempotent;
        }

        /// <summary>Invoke the request synchronously.</summary>
        /// <param name="prx">The proxy used to send the request.</param>
        /// <param name="context">The Ice request context.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        public void Invoke(IObjectPrx prx, IReadOnlyDictionary<string, string>? context, CancellationToken cancel) =>
            Invoke(prx, OutgoingRequestFrame.WithEmptyParamList(prx, _operationName, _idempotent, context), cancel);

        /// <summary>Invoke the request asynchronously.</summary>
        /// <param name="prx">The proxy used to send the request.</param>
        /// <param name="context">The Ice request context.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The request return value.</returns>
        public Task InvokeAsync(
            IObjectPrx prx,
            IReadOnlyDictionary<string, string>? context,
            IProgress<bool>? progress,
            CancellationToken cancel) =>
            InvokeAsync(prx,
                        OutgoingRequestFrame.WithEmptyParamList(prx, _operationName, _idempotent, context),
                        progress,
                        cancel);
    }

    /// <summary>Publicly visible Ice-internal class used to make void requests with input parameters, the input
    /// parameters are pass without the `in' modifier, this helper is used when there is a single input parameter and
    /// the parameter is not a structure type.</summary>
    /// <typeparam name="TParamList">The input parameters type.</typeparam>
    public sealed class OutgoingRequestWithParam<TParamList> : OutgoingRequest
    {
        private readonly string _operationName;
        private readonly bool _idempotent;
        private readonly FormatType _format;
        private readonly OutputStreamWriter<TParamList> _writer;

        /// <summary>Constructs a new request.</summary>
        /// <param name="operationName">The request operation name.</param>
        /// <param name="idempotent"><c>True</c> if the request is idempotent, <c>False</c> otherwise.</param>
        /// <param name="oneway"><c>True</c> if the request use oneway invocation mode, <c>False</c> otherwise.</param>
        /// <param name="compress"><c>True</c> if the request params must use protocol compression, <c>False</c>
        /// otherwise.</param>
        /// <param name="format">The format used for marshal class parameters.</param>
        /// <param name="writer">The <see cref="OutputStream"/> writer used to write the request input parameters.</param>
        public OutgoingRequestWithParam(
            string operationName,
            bool idempotent,
            bool oneway,
            bool compress,
            FormatType format,
            OutputStreamWriter<TParamList> writer)
            : base(oneway, compress)
        {
            _operationName = operationName;
            _idempotent = idempotent;
            _format = format;
            _writer = writer;
        }

        /// <summary>Invoke the request synchronously.</summary>
        /// <param name="prx">The proxy used to send the request.</param>
        /// <param name="paramList">The request input parameters.</param>
        /// <param name="context">The Ice request context.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        public void Invoke(
            IObjectPrx prx,
            TParamList paramList,
            IReadOnlyDictionary<string, string>? context,
            CancellationToken cancel) =>
            Invoke(prx,
                   OutgoingRequestFrame.WithParamList(prx,
                                                      _operationName,
                                                      _idempotent,
                                                      _compress,
                                                      _format,
                                                      context,
                                                      paramList,
                                                      _writer),
                   cancel);

        /// <summary>Invoke the request asynchronously.</summary>
        /// <param name="prx">The proxy used to send the request.</param>
        /// <param name="paramList">The request input parameters.</param>
        /// <param name="context">The Ice request context.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        public Task InvokeAsync(
            IObjectPrx prx,
            TParamList paramList,
            IReadOnlyDictionary<string, string>? context,
            IProgress<bool>? progress,
            CancellationToken cancel) =>
            InvokeAsync(prx,
                        OutgoingRequestFrame.WithParamList(prx,
                                                           _operationName,
                                                           _idempotent,
                                                           _compress,
                                                           _format,
                                                           context,
                                                           paramList,
                                                           _writer),
                        progress,
                        cancel);
    }

    /// <summary>Publicly visible Ice-internal type used to make void request with input parameters, the input
    /// parameters are pass with the `in' modifier, this helper is used when input parameter is a structure type, or
    /// when there is several input parameters as we pass them wrapped in a tuple.</summary>
    /// <typeparam name="TParamList">The input parameters type.</typeparam>
    public sealed class OutgoingRequestWithStructParam<TParamList>
        : OutgoingRequest where TParamList : struct
    {
        private readonly string _operationName;
        private readonly bool _idempotent;
        private readonly FormatType _format;
        private readonly OutputStreamValueWriter<TParamList> _writer;

        /// <summary>Constructs a new request.</summary>
        /// <param name="operationName">The request operation name.</param>
        /// <param name="idempotent"><c>True</c> if the request is idempotent, <c>False</c> otherwise.</param>
        /// <param name="oneway"><c>True</c> if the request use oneway invocation mode, <c>False</c> otherwise.</param>
        /// <param name="compress"><c>True</c> if the request params must use protocol compression, <c>False</c>
        /// otherwise.</param>
        /// <param name="format">The format used for marshal class parameters.</param>
        /// <param name="writer">The <see cref="OutputStream"/> writer used to write the request input parameters.</param>
        public OutgoingRequestWithStructParam(
            string operationName,
            bool idempotent,
            bool oneway,
            bool compress,
            FormatType format,
            OutputStreamValueWriter<TParamList> writer)
            : base(oneway, compress)
        {
            _operationName = operationName;
            _idempotent = idempotent;
            _format = format;
            _writer = writer;
        }

        /// <summary>Invoke the request synchronously.</summary>
        /// <param name="prx">The proxy used to send the request.</param>
        /// <param name="paramList">The request input parameters.</param>
        /// <param name="context">The Ice request context.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        public void Invoke(
            IObjectPrx prx,
            in TParamList paramList,
            IReadOnlyDictionary<string, string>? context,
            CancellationToken cancel) =>
            Invoke(prx,
                   OutgoingRequestFrame.WithParamList(prx,
                                                      _operationName,
                                                      _idempotent,
                                                      _compress,
                                                      _format,
                                                      context,
                                                      paramList,
                                                      _writer),
                   cancel);

        /// <summary>Invoke the request asynchronously.</summary>
        /// <param name="prx">The proxy used to send the request.</param>
        /// <param name="paramList">The request input parameters.</param>
        /// <param name="context">The Ice request context.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        public Task InvokeAsync(
            IObjectPrx prx, in TParamList paramList,
            IReadOnlyDictionary<string, string>? context,
            IProgress<bool>? progress,
            CancellationToken cancel) =>
            InvokeAsync(prx,
                        OutgoingRequestFrame.WithParamList(prx,
                                                           _operationName,
                                                           _idempotent,
                                                           _compress,
                                                           _format,
                                                           context,
                                                           paramList,
                                                           _writer),
                        progress,
                        cancel);
    }
}
