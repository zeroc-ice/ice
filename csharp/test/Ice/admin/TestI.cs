// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Admin
{
    public class TestFacet : ITestFacet
    {
        public void Op(Current current, CancellationToken cancel)
        {
        }
    }

    public class RemoteCommunicator : IAsyncRemoteCommunicator
    {
        private volatile IReadOnlyDictionary<string, string>? _changes;
        private readonly Communicator _communicator;

        public RemoteCommunicator(Communicator communicator) => _communicator = communicator;

        public async ValueTask<IObjectPrx?> GetAdminAsync(Current current, CancellationToken cancel) =>
            await _communicator.GetAdminAsync(cancel: cancel);

        public ValueTask<IReadOnlyDictionary<string, string>> GetChangesAsync(Current current, CancellationToken cancel)
        {
            return new(_changes!);
        }

        public ValueTask PrintAsync(string message, Current current, CancellationToken cancel)
        {
            _communicator.Logger.Print(message);
            return default;
        }

        public ValueTask TraceAsync(string category, string message, Current current, CancellationToken cancel)
        {
            _communicator.Logger.Trace(category, message);
            return default;
        }

        public ValueTask WarningAsync(string message, Current current, CancellationToken cancel)
        {
            _communicator.Logger.Warning(message);
            return default;
        }

        public ValueTask ErrorAsync(string message, Current current, CancellationToken cancel)
        {
            _communicator.Logger.Error(message);
            return default;
        }

        // We're not shutting down the communicator that dispatches the request!
        public async ValueTask ShutdownAsync(Current current, CancellationToken cancel) =>
            await _communicator.ShutdownAsync();

        public async ValueTask DestroyAsync(Current current, CancellationToken cancel) =>
            await _communicator.DestroyAsync();

        public void Updated(IReadOnlyDictionary<string, string> changes) => _changes = changes;
    }

    public class RemoteCommunicatorFactoryI : IRemoteCommunicatorFactory
    {
        public IRemoteCommunicatorPrx CreateCommunicator(
            Dictionary<string, string> props,
            Current current,
            CancellationToken cancel)
        {
            // Prepare the property set using the given properties.
            ILogger? logger = null;
            if (props.TryGetValue("NullLogger", out string? value) &&
                int.TryParse(value, out int nullLogger) && nullLogger > 0)
            {
                logger = new NullLogger();
            }

            // Initialize a new communicator.
            var communicator = new Communicator(props, logger: logger);

            // Install a custom admin facet.
            try
            {
                var testFacet = new TestFacet();
                communicator.AddAdminFacet("TestFacet", testFacet);
            }
            catch (System.ArgumentException)
            {
            }

            // The RemoteCommunicator servant also implements PropertiesAdminUpdateCallback.
            var servant = new RemoteCommunicator(communicator);

            if (communicator.FindAdminFacet("Properties") is IAsyncPropertiesAdmin admin)
            {
                admin.Updated += (_, updates) => servant.Updated(updates);
            }

            return current.Adapter.AddWithUUID(servant, IRemoteCommunicatorPrx.Factory);
        }

        public void Shutdown(Current current, CancellationToken cancel) => current.Communicator.ShutdownAsync();

        private class NullLogger : ILogger
        {
            public void Print(string message)
            {
            }

            public void Trace(string category, string message)
            {
            }

            public void Warning(string message)
            {
            }

            public void Error(string message)
            {
            }

            public string Prefix => "NullLogger";

            public ILogger CloneWithPrefix(string prefix) => this;
        }
    }
}
