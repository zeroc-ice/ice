// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using ZeroC.Ice;
using Test;
using System.Threading;

namespace ZeroC.IceDiscovery.Test.Simple
{
    public sealed class Controller : IController
    {
        public void ActivateObjectAdapter(
            string name,
            string adapterId,
            string replicaGroupId,
            Current current,
            CancellationToken cancel)
        {
            Communicator communicator = current.Adapter.Communicator;
            bool ice1 = TestHelper.GetTestProtocol(communicator.GetProperties()) == Protocol.Ice1;
            communicator.SetProperty($"{name}.AdapterId", adapterId);
            communicator.SetProperty($"{name}.ReplicaGroupId", replicaGroupId);
            communicator.SetProperty($"{name}.Endpoints", ice1 ? "tcp -h 127.0.0.1" : "ice+tcp://127.0.0.1:0");
            ObjectAdapter oa = communicator.CreateObjectAdapter(name);
            _adapters[name] = oa;
            oa.Activate();
        }

        public void DeactivateObjectAdapter(string name, Current current, CancellationToken cancel)
        {
            _adapters[name].Dispose();
            _adapters.Remove(name);
        }

        public void AddObject(string oaName, string id, Current current, CancellationToken cancel)
        {
            TestHelper.Assert(_adapters.ContainsKey(oaName));
            _adapters[oaName].Add(id, new TestIntf());
        }

        public void RemoveObject(string oaName, string id, Current current, CancellationToken cancel)
        {
            TestHelper.Assert(_adapters.ContainsKey(oaName));
            _adapters[oaName].Remove(id);
        }

        public void Shutdown(Current current, CancellationToken cancel) =>
            current.Adapter.Communicator.ShutdownAsync();

        private readonly Dictionary<string, ObjectAdapter> _adapters = new Dictionary<string, ObjectAdapter>();
    }

    public sealed class TestIntf : ITestIntf
    {
        public string GetAdapterId(Current current, CancellationToken cancel) =>
            current.Adapter.Communicator.GetProperty($"{current.Adapter.Name}.AdapterId") ?? "";
    }
}
