// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Text;
using System.Threading;
using System.Threading.Channels;
using System.Threading.Tasks;
using ZeroC.Ice.Instrumentation;
using ColocatedChannelReader = System.Threading.Channels.ChannelReader<(long StreamId, object? Frame, bool Fin)>;
using ColocatedChannelWriter = System.Threading.Channels.ChannelWriter<(long StreamId, object? Frame, bool Fin)>;

namespace ZeroC.Ice
{
    /// <summary>The Endpoint class for the colocated transport.</summary>
    internal class ColocatedEndpoint : Endpoint
    {
        public override bool IsAlwaysSecure => true;

        protected internal override bool HasOptions => false;
        protected internal override ushort DefaultPort => 0;

        internal ObjectAdapter Adapter { get; }

        private readonly Channel<(long, ColocatedChannelWriter, ColocatedChannelReader)> _channel;

        public override IAcceptor Acceptor(ObjectAdapter adapter) =>
            new ColocatedAcceptor(this, adapter, _channel.Writer, _channel.Reader);

        public override bool IsLocal(Endpoint endpoint) =>
            endpoint is ColocatedEndpoint colocatedEndpoint && colocatedEndpoint.Adapter == Adapter;

        protected internal override void WriteOptions(OutputStream ostr) =>
            throw new NotSupportedException("colocated endpoint can't be marshaled");

        public override Connection CreateDatagramServerConnection(ObjectAdapter adapter) =>
            throw new InvalidOperationException();

        private long _nextId;

        protected internal override void AppendOptions(StringBuilder sb, char optionSeparator)
        {
        }

        protected internal async override Task<Connection> ConnectAsync(
            NonSecure preferNonSecure,
            object? label,
            CancellationToken cancel)
        {
            IObserver? observer = Communicator.Observer?.GetConnectionEstablishmentObserver(
                this,
                Adapter.Name.Length == 0 ? "unnamed adapter" : Adapter.Name);
            try
            {
                observer?.Attach();
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

                if (!_channel.Writer.TryWrite((id, writer.Writer, reader.Reader)))
                {
                    throw new ConnectionRefusedException();
                }

                var connection = new ColocatedConnection(
                    this,
                    new ColocatedSocket(this, id, reader.Writer, writer.Reader, false),
                    label,
                    adapter: null);
                await connection.InitializeAsync(cancel).ConfigureAwait(false);
                return connection;
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
        protected internal override Endpoint GetPublishedEndpoint(string serverName) =>
            throw new NotSupportedException("cannot create published endpoint for colocated endpoint");

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
        }
    }
}
