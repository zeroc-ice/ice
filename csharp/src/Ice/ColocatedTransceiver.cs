//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Threading.Tasks.Sources;
using System.Threading.Channels;

namespace ZeroC.Ice
{
    internal class ColocatedStream : Stream, IValueTaskSource<object>
    {
        protected override ReadOnlyMemory<byte> Header => ArraySegment<byte>.Empty;
        private ManualResetValueTaskSourceCore<object> _source;
        private CancellationTokenSource? _cancelSource;
        private readonly ColocatedTransceiver _transceiver;

        public override void Abort(Exception ex)
        {
            try
            {
                _source.SetException(ex);
            }
            catch
            {
            }
        }

        protected override ValueTask<bool> ReceiveAsync(ArraySegment<byte> buffer, CancellationToken cancel) =>
            // This is never called because we override the default ReceiveFrameAsync implementation
            throw new NotImplementedException();

        protected override ValueTask ResetAsync()
        {
            _cancelSource!.Cancel();
            return default;
        }

        protected override ValueTask SendAsync(IList<ArraySegment<byte>> buffer, bool fin, CancellationToken cancel) =>
            // This is never called because we override the default Async implementation
            throw new NotImplementedException();

        object IValueTaskSource<object>.GetResult(short token)
        {
            Debug.Assert(token == _source.Version);
            try
            {
                return _source.GetResult(token);
            }
            finally
            {
                _source.Reset();
            }
        }

        ValueTaskSourceStatus IValueTaskSource<object>.GetStatus(short token) => _source.GetStatus(token);

        void IValueTaskSource<object>.OnCompleted(
            Action<object?> continuation,
            object? state,
            short token,
            ValueTaskSourceOnCompletedFlags flags) => _source.OnCompleted(continuation, state, token, flags);

        internal ColocatedStream(long streamId, ColocatedTransceiver transceiver) : base(streamId, transceiver) =>
            _transceiver = transceiver;

        internal override void CancelSourceIfStreamReset(CancellationTokenSource source) => _cancelSource = source;

        internal void ReceivedFrame(object frame) => _source.SetResult(frame);

        internal override async ValueTask<(long, string)> ReceiveCloseFrameAsync()
        {
            try
            {
                await _transceiver.Reader.Completion.ConfigureAwait(false);
            }
            catch (ObjectDisposedException)
            {
            }
            return (0, "connection gracefully closed by peer");
        }

        internal override ValueTask SendCloseFrameAsync(long streamId, string reason, CancellationToken cancel)
        {
            _transceiver.Writer.Complete();
            return default;
        }

        private protected override async ValueTask<(ArraySegment<byte>, bool)> ReceiveFrameAsync(
            byte expectedFrameType,
            CancellationToken cancel)
        {
            object frame = await new ValueTask<object>(this, _source.Version).ConfigureAwait(false);
            if (frame is OutgoingRequestFrame request)
            {
                return (request.Data.AsArraySegment(), true);
            }
            else if (frame is OutgoingResponseFrame response)
            {
                return (response.Data.AsArraySegment(), true);
            }
            else
            {
                Debug.Assert(false);
                throw new InvalidDataException("unexpected frame");
            }
        }

        private protected override ValueTask SendFrameAsync(OutgoingFrame frame, bool fin, CancellationToken cancel)
        {
            if (_transceiver.Endpoint.Communicator.TraceLevels.Protocol >= 1)
            {
                ProtocolTrace.TraceFrame(_transceiver.Endpoint, Id, frame);
            }
            return _transceiver.SendFrameAsync(Id, frame, cancel);
        }
    }

    internal class ColocatedTransceiver : MultiStreamTransceiver
    {
        internal ChannelReader<(long, object)> Reader { get; }
        internal ChannelWriter<(long, object)> Writer { get; }
        private readonly object _mutex = new object();
        private long _nextBidirectionalId;
        private long _nextUnidirectionalId;

        public override async ValueTask<Stream> AcceptStreamAsync(CancellationToken cancel)
        {
            while (true)
            {
                (long streamId, object frame) = await Reader.ReadAsync(cancel).ConfigureAwait(false);
                if (frame is OutgoingRequestFrame)
                {
                    var stream = new ColocatedStream(streamId, this);
                    stream.ReceivedFrame(frame);
                    return stream;
                }
                else if (TryGetStream(streamId, out ColocatedStream? stream))
                {
                    stream.ReceivedFrame(frame);
                }
                else
                {
                    // Canceled request.
                }
            }
        }

        public override ValueTask CloseAsync(Exception exception, CancellationToken cancel)
        {
            Writer.Complete();
            return default;
        }

        public override Stream CreateStream(bool bidirectional)
        {
            lock (_mutex)
            {
                Stream stream;
                if (bidirectional)
                {
                    stream = new ColocatedStream(_nextBidirectionalId, this);
                    _nextBidirectionalId += 4;
                }
                else
                {
                    stream = new ColocatedStream(_nextUnidirectionalId, this);
                    _nextUnidirectionalId += 4;
                }
                return stream;
            }
        }

        public override ValueTask InitializeAsync(CancellationToken cancel) => default;

        public override ValueTask PingAsync(CancellationToken cancel) => default;

        public override string ToString() =>
            $"object adapter = {((ColocatedEndpoint)Endpoint).Adapter.Name}\nincoming = {IsIncoming}";

        protected override void Dispose(bool disposing)
        {
        }

        internal ColocatedTransceiver(ColocatedEndpoint endpoint,
                                      ChannelWriter<(long, object)> writer,
                                      ChannelReader<(long, object)> reader,
                                      bool isIncoming) :
            base(endpoint, isIncoming ? endpoint.Adapter : null)
        {
            // We use the same stream ID numbering scheme as Quic
            if (IsIncoming)
            {
                _nextBidirectionalId = 1;
                _nextUnidirectionalId = 3;
            }
            else
            {
                _nextBidirectionalId = 0;
                _nextUnidirectionalId = 2;
            }
            Writer = writer;
            Reader = reader;
        }

        internal override ValueTask<Stream> ReceiveInitializeFrameAsync(CancellationToken cancel) =>
            new ValueTask<Stream>(new ColocatedStream(IsIncoming ? 2 : 3, this));

        internal ValueTask SendFrameAsync(long streamId, object frame, CancellationToken cancel) =>
            Writer.WriteAsync((streamId, frame), cancel);

        internal override ValueTask<Stream> SendInitializeFrameAsync(CancellationToken cancel) =>
            new ValueTask<Stream>(new ColocatedStream(IsIncoming ? 3 : 2, this));
    }

}
