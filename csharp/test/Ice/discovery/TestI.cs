// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading;
using Test;

namespace ZeroC.Ice.Test.Discovery
{
    public sealed class Controller : IController
    {
        private readonly Dictionary<string, ObjectAdapter> _adapters = new();

        public void ActivateObjectAdapter(
            string name,
            string adapterId,
            string replicaGroupId,
            Current current,
            CancellationToken cancel)
        {
            Communicator communicator = current.Communicator;
            bool ice1 = TestHelper.GetTestProtocol(communicator.GetProperties()) == Protocol.Ice1;
            string transport = TestHelper.GetTestTransport(communicator.GetProperties());

            communicator.SetProperty($"{name}.AdapterId", adapterId);
            communicator.SetProperty($"{name}.ReplicaGroupId", replicaGroupId);
            communicator.SetProperty($"{name}.Endpoints", ice1 ? $"{transport} -h 127.0.0.1" :
                $"ice+{transport}://127.0.0.1:0");

            communicator.SetProperty($"{name}.ServerName", "localhost");
            ObjectAdapter oa = communicator.CreateObjectAdapter(name);
            _adapters[name] = oa;
            oa.Activate();
        }

        public void DeactivateObjectAdapter(string name, Current current, CancellationToken cancel)
        {
            _adapters[name].Dispose();
            _adapters.Remove(name);
        }

        public void AddObject(string oaName, string identityAndFacet, Current current, CancellationToken cancel)
        {
            TestHelper.Assert(_adapters.ContainsKey(oaName));
            _adapters[oaName].Add(identityAndFacet, new TestIntf());
        }

        public void RemoveObject(string oaName, string identityAndFacet, Current current, CancellationToken cancel)
        {
            TestHelper.Assert(_adapters.ContainsKey(oaName));
            _adapters[oaName].Remove(identityAndFacet);
        }

        public void Shutdown(Current current, CancellationToken cancel) =>
            current.Communicator.ShutdownAsync();
    }

    public sealed class TestIntf : ITestIntf
    {
        public string GetAdapterId(Current current, CancellationToken cancel) =>
            current.Communicator.GetProperty($"{current.Adapter.Name}.AdapterId") ?? "";
    }
}
