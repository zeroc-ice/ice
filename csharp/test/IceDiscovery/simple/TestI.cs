//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using ZeroC.Ice;
using Test;

namespace ZeroC.IceDiscovery.Test.Simple
{
    public sealed class Controller : IController
    {
        public void ActivateObjectAdapter(string name, string adapterId, string replicaGroupId, Current current)
        {
            Communicator communicator = current.Adapter.Communicator;
            communicator.SetProperty($"{name}.AdapterId", adapterId);
            communicator.SetProperty($"{name}.ReplicaGroupId", replicaGroupId);
            communicator.SetProperty($"{name}.Endpoints", "default");
            ObjectAdapter oa = communicator.CreateObjectAdapter(name);
            _adapters[name] = oa;
            oa.Activate();
        }

        public void DeactivateObjectAdapter(string name, Current current)
        {
            _adapters[name].Dispose();
            _adapters.Remove(name);
        }

        public void AddObject(string oaName, string id, Current current)
        {
            TestHelper.Assert(_adapters.ContainsKey(oaName));
            _adapters[oaName].Add(id, new TestIntf());
        }

        public void RemoveObject(string oaName, string id, Current current)
        {
            TestHelper.Assert(_adapters.ContainsKey(oaName));
            _adapters[oaName].Remove(id);
        }

        public void Shutdown(Current current) => current.Adapter.Communicator.ShutdownAsync();

        private Dictionary<string, ObjectAdapter> _adapters = new Dictionary<string, ObjectAdapter>();
    }

    public sealed class TestIntf : ITestIntf
    {
        public string GetAdapterId(Current current) =>
            current.Adapter.Communicator.GetProperty($"{current.Adapter.Name}.AdapterId") ?? "";
    }
}
