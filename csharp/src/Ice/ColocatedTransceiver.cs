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
    internal class ColocatedStream : Stream, IValueTaskSource<(object, bool)>
    {
        protected override ReadOnlyMemory<byte> Header => ArraySegment<byte>.Empty;
        private ManualResetValueTaskSourceCore<(object, bool)> _source;
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
            _transceiver.SendFrameAsync(Id, buffer, fin, cancel);

        (object, bool) IValueTaskSource<(object, bool)>.GetResult(short token)
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

        ValueTaskSourceStatus IValueTaskSource<(object, bool)>.GetStatus(short token) => _source.GetStatus(token);

        void IValueTaskSource<(object, bool)>.OnCompleted(
            Action<object?> continuation,
            object? state,
            short token,
            ValueTaskSourceOnCompletedFlags flags) => _source.OnCompleted(continuation, state, token, flags);

        internal ColocatedStream(long streamId, ColocatedTransceiver transceiver) : base(streamId, transceiver) =>
            _transceiver = transceiver;

        internal override void CancelSourceIfStreamReset(CancellationTokenSource source) => _cancelSource = source;

        internal void ReceivedFrame(object frame, bool fin) => _source.SetResult((frame, fin));

        private protected override async ValueTask<(ArraySegment<byte>, bool)> ReceiveFrameAsync(
            byte expectedFrameType,
            CancellationToken cancel)
        {
            (object frame, bool fin) = await new ValueTask<(object, bool)>(this, _source.Version).ConfigureAwait(false);
            if (frame is OutgoingRequestFrame request)
            {
                return (request.Data.AsArraySegment(), fin);
            }
            else if (frame is OutgoingResponseFrame response)
            {
                return (response.Data.AsArraySegment(), fin);
            }
            else if (frame is List<ArraySegment<byte>> data)
            {
                if (_transceiver.Endpoint.Protocol == Protocol.Ice1)
                {
                    Debug.Assert(expectedFrameType == data[0][8]);
                    return (ArraySegment<byte>.Empty, fin);
                }
                else
                {
                    Debug.Assert(expectedFrameType == data[0][0]);
                    (int size, int sizeLength) = data[0].Slice(1).AsReadOnlySpan().ReadSize20();
                    return (data[0].Slice(1 + sizeLength, size), fin);
                }
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
            return _transceiver.SendFrameAsync(Id, frame, fin, cancel);
        }
    }

    internal class ColocatedTransceiver : MultiStreamTransceiver
    {
        private readonly object _mutex = new object();
        private long _nextBidirectionalId;
        private long _nextUnidirectionalId;
        private readonly ChannelReader<(long, object, bool)> _reader;
        private readonly ChannelWriter<(long, object, bool)> _writer;

        public override async ValueTask<Stream> AcceptStreamAsync(CancellationToken cancel)
        {
            while (true)
            {
                try
                {
                    (long streamId, object frame, bool fin) = await _reader.ReadAsync(cancel).ConfigureAwait(false);
                    if (TryGetStream(streamId, out ColocatedStream? stream))
                    {
                        stream.ReceivedFrame(frame, fin);
                    }
                    else if (frame is OutgoingRequestFrame || streamId == (IsIncoming ? 2 : 3))
                    {
                        stream = new ColocatedStream(streamId, this);
                        stream.ReceivedFrame(frame, fin);
                        return stream;
                    }
                    else
                    {
                        // Canceled request.
                    }
                }
                catch (ChannelClosedException exception)
                {
                    throw new Ice.ConnectionLostException(exception);
                }
            }
        }

        public override async ValueTask CloseAsync(Exception exception, CancellationToken cancel)
        {
            _writer.Complete();
            await _reader.Completion.ConfigureAwait(false);
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

        internal ColocatedTransceiver(ColocatedEndpoint endpoint,
                                      ChannelWriter<(long, object, bool)> writer,
                                      ChannelReader<(long, object, bool)> reader,
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
            _writer = writer;
            _reader = reader;
        }

        internal ValueTask SendFrameAsync(long streamId, object frame, bool fin, CancellationToken cancel) =>
            _writer.WriteAsync((streamId, frame, fin), cancel);
    }
}
