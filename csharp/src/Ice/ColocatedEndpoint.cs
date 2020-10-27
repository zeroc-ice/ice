// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using System.Threading.Channels;
using System.Threading.Tasks;
using ColocatedChannelReader = System.Threading.Channels.ChannelReader<(long StreamId, object? Frame, bool Fin)>;
using ColocatedChannelWriter = System.Threading.Channels.ChannelWriter<(long StreamId, object? Frame, bool Fin)>;

namespace ZeroC.Ice
{
    /// <summary>The Endpoint class for the colocated transport.</summary>
    internal class ColocatedEndpoint : Endpoint
    {
        public override bool IsSecure => true;

        protected internal override bool HasOptions => false;
        protected internal override ushort DefaultPort => 0;

        internal ObjectAdapter Adapter { get; }

        private readonly Channel<(long, ColocatedChannelWriter, ColocatedChannelReader)> _channel;
        private readonly IEnumerable<IConnector> _connectors;

        public override IAcceptor Acceptor(IConnectionManager manager, ObjectAdapter adapter) =>
            new ColocatedAcceptor(this, manager, adapter, _channel.Writer, _channel.Reader);

        public override bool IsLocal(Endpoint endpoint) =>
            endpoint is ColocatedEndpoint colocatedEndpoint && colocatedEndpoint.Adapter == Adapter;

        protected internal override void WriteOptions(OutputStream ostr) =>
            throw new NotSupportedException("colocated endpoint can't be marshaled");

        public override ValueTask<IEnumerable<IConnector>> ConnectorsAsync(
            CancellationToken cancel) => new ValueTask<IEnumerable<IConnector>>(_connectors);

        public override Connection CreateDatagramServerConnection(ObjectAdapter adapter) =>
            throw new InvalidOperationException();

        public override IEnumerable<Endpoint> ExpandHost(out Endpoint? publish) =>
            throw new NotSupportedException("colocated endpoint can't be used for object adapter endpoints");

        public override IEnumerable<Endpoint> ExpandIfWildcard() => new Endpoint[] { this };

        protected internal override void AppendOptions(StringBuilder sb, char optionSeparator)
        {
        }

        internal ColocatedEndpoint(ObjectAdapter adapter)
            : base(new EndpointData(Transport.Colocated, host: adapter.Name, port: 0, Array.Empty<string>()),
                   adapter.Communicator,
                   adapter.Protocol)
        {
            Adapter = adapter;
            var options = new UnboundedChannelOptions
            {
                SingleReader = true,
                SingleWriter = true,
                AllowSynchronousContinuations = true
            };
            _channel = Channel.CreateUnbounded<(long, ColocatedChannelWriter, ColocatedChannelReader)>(options);
            _connectors = new IConnector[] { new ColocatedConnector(this, _channel.Writer) };
        }
    }
}
