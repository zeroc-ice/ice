//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using Test;
using Ice.scope.Test;
using Ice.scope.Test.Inner;
using Ice.scope.Test.Inner.Inner2;
using Inner.Test.Inner2;

namespace Ice.scope
{
    public class Server : TestHelper
    {
        class I1 : Test.II
        {
            public (Test.S, Test.S) opS(Test.S s1, Current current) => (s1, s1);

            public (Test.S[], Test.S[]) opSSeq(Test.S[] s1, Current current) => (s1, s1);

            public (Dictionary<string, Test.S>, Dictionary<string, Test.S>)
            opSMap(Dictionary<string, Test.S> s1, Current current) => (s1, s1);

            public (Test.C, Test.C) opC(Test.C c1, Current current) => (c1, c1);

            public (Test.C[], Test.C[]) opCSeq(Test.C[] c1, Current current) => (c1, c1);

            public (Dictionary<string, Test.C>, Dictionary<string, Test.C>)
            opCMap(Dictionary<string, Test.C> c1, Current current) => (c1, c1);

            public E1 opE1(E1 e1, Current current) => e1;

            public S1 opS1(S1 s1, Current current) => s1;

            public C1 opC1(C1 c1, Current current) => c1;

            public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();
        }

        class I2 : Test.Inner.II
        {
            public (Test.Inner.Inner2.S, Test.Inner.Inner2.S)
            opS(Test.Inner.Inner2.S s1, Current current) => (s1, s1);

            public (Test.Inner.Inner2.S[], Test.Inner.Inner2.S[])
            opSSeq(Test.Inner.Inner2.S[] s1, Current current) => (s1, s1);

            public (Dictionary<string, Test.Inner.Inner2.S>, Dictionary<string, Test.Inner.Inner2.S>)
            opSMap(Dictionary<string, Test.Inner.Inner2.S> s1, Current current) => (s1, s1);

            public (Test.Inner.Inner2.C, Test.Inner.Inner2.C)
            opC(Test.Inner.Inner2.C c1, Current current) => (c1, c1);

            public (Test.Inner.Inner2.C[], Test.Inner.Inner2.C[])
            opCSeq(Test.Inner.Inner2.C[] c1, Current current) => (c1, c1);

            public (Dictionary<string, Test.Inner.Inner2.C>, Dictionary<string, Test.Inner.Inner2.C>)
            opCMap(Dictionary<string, Test.Inner.Inner2.C> c1, Current current) => (c1, c1);

            public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();
        }

        class I3 : Test.Inner.Inner2.II
        {
            public (Test.Inner.Inner2.S, Test.Inner.Inner2.S)
            opS(Test.Inner.Inner2.S s1, Current current) => (s1, s1);

            public (Test.Inner.Inner2.S[], Test.Inner.Inner2.S[])
            opSSeq(Test.Inner.Inner2.S[] s1, Current current) => (s1, s1);

            public (Dictionary<string, Test.Inner.Inner2.S>, Dictionary<string, Test.Inner.Inner2.S>)
            opSMap(Dictionary<string, Test.Inner.Inner2.S> s1, Current current) => (s1, s1);

            public (Test.Inner.Inner2.C, Test.Inner.Inner2.C)
            opC(Test.Inner.Inner2.C c1, Current current) => (c1, c1);

            public (Test.Inner.Inner2.C[], Test.Inner.Inner2.C[])
            opCSeq(Test.Inner.Inner2.C[] c1, Current current) => (c1, c1);

            public (Dictionary<string, Test.Inner.Inner2.C>, Dictionary<string, Test.Inner.Inner2.C>)
            opCMap(Dictionary<string, Test.Inner.Inner2.C> c1, Current current) => (c1, c1);

            public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();
        }

        class I4 : Inner.Test.Inner2.II
        {
            public (Test.S, Test.S) opS(Test.S s1, Current current) => (s1, s1);

            public (Test.S[], Test.S[]) opSSeq(Test.S[] s1, Current current) => (s1, s1);

            public (Dictionary<string, Test.S>, Dictionary<string, Test.S>)
            opSMap(Dictionary<string, Test.S> s1, Current current) => (s1, s1);

            public (Test.C, Test.C) opC(Test.C c1, Current current) => (c1, c1);

            public (Test.C[], Test.C[]) opCSeq(Test.C[] c1, Current current) => (c1, c1);

            public (Dictionary<string, Test.C>, Dictionary<string, Test.C>)
            opCMap(Dictionary<string, Test.C> c1, Current current) => (c1, c1);

            public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();
        }

        public override void run(string[] args)
        {
            using var communicator = initialize(createTestProperties(ref args),
                typeIdNamespaces: new string[] { "Ice.scope.TypeId" });
            communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("i1", new I1());
            adapter.Add("i2", new I2());
            adapter.Add("i3", new I3());
            adapter.Add("i4", new I4());
            adapter.Activate();
            serverReady();
            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.runTest<Server>(args);
    }
}
