// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Threading;
using System.Threading.Channels;

using ColocatedChannelReader = System.Threading.Channels.ChannelReader<(long StreamId, object? Frame, bool Fin)>;
using ColocatedChannelWriter = System.Threading.Channels.ChannelWriter<(long StreamId, object? Frame, bool Fin)>;

namespace ZeroC.Ice
{
    /// <summary>The IConnector implementation for the colocated transport.</summary>
    internal class ColocatedConnector : Connector
    {
        private readonly ColocatedEndpoint _endpoint;
        private long _nextId;
        private readonly ChannelWriter<(long, ColocatedChannelWriter, ColocatedChannelReader)> _writer;

        public override Endpoint Endpoint => _endpoint;
        public override Connection Connect(string connectionId)
        {
            var readerOptions = new UnboundedChannelOptions
            {
                SingleReader = true,
                SingleWriter = true,
                AllowSynchronousContinuations = false
            };
            var reader = Channel.CreateUnbounded<(long, object?, bool)>(readerOptions);

            var writerOptions = new UnboundedChannelOptions
            {
                SingleReader = true,
                SingleWriter = true,
                AllowSynchronousContinuations = false
            };
            var writer = Channel.CreateUnbounded<(long, object?, bool)>(writerOptions);

            long id = Interlocked.Increment(ref _nextId);

            if (!_writer.TryWrite((id, writer.Writer, reader.Reader)))
            {
                throw new ConnectionRefusedException();
            }

            return new ColocatedConnection(_endpoint.Communicator.OutgoingConnectionFactory,
                                           _endpoint,
                                           new ColocatedTransceiver(_endpoint, id, reader.Writer, writer.Reader, false),
                                           this,
                                           connectionId,
                                           null);
        }

        public override string ToString() =>
            _endpoint.Adapter.Name.Length == 0 ? "unnamed adapter" : _endpoint.Adapter.Name;
        public override bool Equals(Connector? obj) => ReferenceEquals(this, obj);
        public override int GetHashCode() => _endpoint.GetHashCode();

        internal ColocatedConnector(
            ColocatedEndpoint endpoint,
            ChannelWriter<(long, ColocatedChannelWriter, ColocatedChannelReader)> writer)
        {
            _endpoint = endpoint;
            _writer = writer;
        }
    }
}
