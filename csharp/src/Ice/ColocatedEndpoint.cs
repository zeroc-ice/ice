//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Threading.Channels;

namespace ZeroC.Ice
{
    internal class ColocatedConnector : IConnector
    {
        private readonly ColocatedEndpoint _endpoint;
        private readonly ChannelWriter<(ChannelWriter<(long, object, bool)>, ChannelReader<(long, object, bool)>)> _writer;

        public Connection Connect(string connectionId)
        {
            var reader = Channel.CreateUnbounded<(long, object, bool)>();
            var writer = Channel.CreateUnbounded<(long, object, bool)>();

            if (!_writer.TryWrite((writer.Writer, reader.Reader)))
            {
                throw new ConnectionRefusedException();
            }

            return new ColocatedConnection(_endpoint.Communicator.OutgoingConnectionFactory,
                                           _endpoint,
                                           new ColocatedTransceiver(_endpoint, reader.Writer, writer.Reader, false),
                                           this,
                                           connectionId,
                                           null);
        }

        public override string ToString() => _endpoint.Adapter.Name;

        internal ColocatedConnector(
            ColocatedEndpoint endpoint,
            ChannelWriter<(ChannelWriter<(long, object, bool)>, ChannelReader<(long, object, bool)>)> writer)
        {
            _endpoint = endpoint;
            _writer = writer;
        }
    }

    internal class ColocatedAcceptor : IAcceptor
    {
        public Endpoint Endpoint => _endpoint;

        private readonly ColocatedEndpoint _endpoint;
        private readonly IConnectionManager _manager;
        private readonly ObjectAdapter _adapter;
        private readonly ChannelReader<(ChannelWriter<(long, object, bool)>, ChannelReader<(long, object, bool)>)> _reader;
        private readonly ChannelWriter<(ChannelWriter<(long, object, bool)>, ChannelReader<(long, object, bool)>)> _writer;

        public async ValueTask<Connection> AcceptAsync()
        {
            (ChannelWriter<(long, object, bool)> writer, ChannelReader<(long, object, bool)> reader) =
                await _reader.ReadAsync().ConfigureAwait(false);

            return new ColocatedConnection(_manager,
                                           _endpoint,
                                           new ColocatedTransceiver(_endpoint, writer, reader, true),
                                           null,
                                           "",
                                           _adapter);
        }

        public void Dispose() => _writer.Complete();

        public string ToDetailedString() => ToString();

        public override string ToString() => _endpoint.Adapter.Name;

        internal ColocatedAcceptor(
            ColocatedEndpoint endpoint,
            IConnectionManager manager,
            ObjectAdapter adapter,
            ChannelWriter<(ChannelWriter<(long, object, bool)>, ChannelReader<(long, object, bool)>)> writer,
            ChannelReader<(ChannelWriter<(long, object, bool)>, ChannelReader<(long, object, bool)>)> reader)
        {
            _endpoint = endpoint;
            _manager = manager;
            _adapter = adapter;
            _writer = writer;
            _reader = reader;
        }
    }

    /// <summary>The base class for IP-based endpoints: TcpEndpoint, UdpEndpoint.</summary>
    internal class ColocatedEndpoint : Endpoint
    {
        public override bool IsSecure => true;
        public override string Host => Adapter.Name;
        public override ushort Port => 0;
        public override Transport Transport => Transport.Colocated;

        protected internal override bool HasOptions => false;
        protected internal override ushort DefaultPort => 0;

        internal ObjectAdapter Adapter { get; }

        private readonly Channel<(ChannelWriter<(long, object, bool)>, ChannelReader<(long, object, bool)>)> _channel;
        private readonly IEnumerable<IConnector> _connectors;

        public override IAcceptor Acceptor(IConnectionManager manager, ObjectAdapter adapter) =>
            new ColocatedAcceptor(this, manager, adapter, _channel.Writer, _channel.Reader);

        public override bool IsLocal(Endpoint endpoint) =>
            endpoint is ColocatedEndpoint colocatedEndpoint && colocatedEndpoint.Adapter == Adapter;

        protected internal override void WriteOptions(OutputStream ostr) =>
            throw new NotSupportedException("colocated endpoint can't be marshaled");

        public override ValueTask<IEnumerable<IConnector>> ConnectorsAsync(
            EndpointSelectionType endptSelection,
            CancellationToken cancel) => new ValueTask<IEnumerable<IConnector>>(_connectors);

        public override Connection CreateDatagramServerConnection(ObjectAdapter adapter) =>
            throw new InvalidOperationException();

        public override IEnumerable<Endpoint> ExpandHost(out Endpoint? publish) =>
            throw new NotSupportedException("colocated endpoint can't be used for object adapter endpoints");

        public override IEnumerable<Endpoint> ExpandIfWildcard() => new Endpoint[] { this };

        protected internal override void AppendOptions(StringBuilder sb, char optionSeparator)
        {
        }

        internal ColocatedEndpoint(ObjectAdapter adapter) : base(adapter.Communicator, adapter.Protocol)
        {
            Adapter = adapter;
            _channel = Channel.CreateUnbounded<(ChannelWriter<(long, object, bool)>,
                                                ChannelReader<(long, object, bool)>)>();
            _connectors = new IConnector[] { new ColocatedConnector(this, _channel.Writer) };
        }
    }
}
