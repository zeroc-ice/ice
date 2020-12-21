// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Admin
{
    public class TestFacet : ITestFacet
    {
        public void Op(Current current, CancellationToken cancel)
        {
        }
    }

    public class RemoteCommunicator : IRemoteCommunicator
    {
        private volatile IReadOnlyDictionary<string, string>? _changes;
        private readonly Communicator _communicator;

        public RemoteCommunicator(Communicator communicator) => _communicator = communicator;

        public IObjectPrx? GetAdmin(Current current, CancellationToken cancel) =>
            _communicator.GetAdminAsync(cancel: cancel).GetAwaiter().GetResult();

        public IReadOnlyDictionary<string, string> GetChanges(Current current, CancellationToken cancel) =>
            new Dictionary<string, string>(_changes!);

        public void Print(string message, Current current, CancellationToken cancel) =>
            _communicator.Logger.Print(message);

        public void Trace(string category, string message, Current current, CancellationToken cancel) =>
            _communicator.Logger.Trace(category, message);

        public void Warning(string message, Current current, CancellationToken cancel) =>
            _communicator.Logger.Warning(message);

        public void Error(string message, Current current, CancellationToken cancel) =>
            _communicator.Logger.Error(message);

        public void Shutdown(Current current, CancellationToken cancel) => _ = _communicator.ShutdownAsync();

        // Note that we are executing in a thread of the *main* communicator, not the one that is being shut down.
        public void WaitForShutdown(Current current, CancellationToken cancel) =>
            _communicator.WaitForShutdownAsync().Wait(cancel);

        public void Destroy(Current current, CancellationToken cancel) => _communicator.Dispose();

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

            if (communicator.FindAdminFacet("Properties") is IPropertiesAdmin admin)
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
