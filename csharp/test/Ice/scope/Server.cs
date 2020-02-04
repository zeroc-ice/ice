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
            public Test.II.OpSReturnValue
            opS(Test.S s1, Current current) => new Test.II.OpSReturnValue(s1, s1);

            public Test.II.OpSSeqReturnValue
            opSSeq(Test.S[] s1, Current current) => new Test.II.OpSSeqReturnValue(s1, s1);

            public Test.II.OpSMapReturnValue
            opSMap(Dictionary<string, Test.S> s1, Current current) => new Test.II.OpSMapReturnValue(s1, s1);

            public Test.II.OpCReturnValue
            opC(Test.C c1, Current current) => new Test.II.OpCReturnValue(c1, c1);

            public Test.II.OpCSeqReturnValue
            opCSeq(Test.C[] c1, Current current) => new Test.II.OpCSeqReturnValue(c1, c1);

            public Test.II.OpCMapReturnValue
            opCMap(Dictionary<string, Test.C> c1, Current current) => new Test.II.OpCMapReturnValue(c1, c1);

            public E1 opE1(E1 e1, Current current) => e1;

            public S1 opS1(S1 s1, Current current) => s1;

            public C1 opC1(C1 c1, Current current) => c1;

            public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();
        }

        class I2 : Test.Inner.II
        {
            public Test.Inner.II.OpSReturnValue
            opS(Test.Inner.Inner2.S s1, Current current) => new Test.Inner.II.OpSReturnValue(s1, s1);

            public Test.Inner.II.OpSSeqReturnValue
            opSSeq(Test.Inner.Inner2.S[] s1, Current current) => new Test.Inner.II.OpSSeqReturnValue(s1, s1);

            public Test.Inner.II.OpSMapReturnValue
            opSMap(Dictionary<string, Test.Inner.Inner2.S> s1, Current current) =>
                new Test.Inner.II.OpSMapReturnValue(s1, s1);

            public Test.Inner.II.OpCReturnValue
            opC(Test.Inner.Inner2.C c1, Current current) => new Test.Inner.II.OpCReturnValue(c1, c1);

            public Test.Inner.II.OpCSeqReturnValue
            opCSeq(Test.Inner.Inner2.C[] c1, Current current) => new Test.Inner.II.OpCSeqReturnValue(c1, c1);

            public Test.Inner.II.OpCMapReturnValue
            opCMap(Dictionary<string, Test.Inner.Inner2.C> c1, Current current) =>
                new Test.Inner.II.OpCMapReturnValue(c1, c1);

            public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();
        }

        class I3 : Test.Inner.Inner2.II
        {
            public Test.Inner.Inner2.II.OpSReturnValue
            opS(Test.Inner.Inner2.S s1, Current current) => new Test.Inner.Inner2.II.OpSReturnValue(s1, s1);

            public Test.Inner.Inner2.II.OpSSeqReturnValue
            opSSeq(Test.Inner.Inner2.S[] s1, Current current) => new Test.Inner.Inner2.II.OpSSeqReturnValue(s1, s1);

            public Test.Inner.Inner2.II.OpSMapReturnValue
            opSMap(Dictionary<string, Test.Inner.Inner2.S> s1, Current current) =>
                new Test.Inner.Inner2.II.OpSMapReturnValue(s1, s1);

            public Test.Inner.Inner2.II.OpCReturnValue
            opC(Test.Inner.Inner2.C c1, Current current) => new Test.Inner.Inner2.II.OpCReturnValue(c1, c1);

            public Test.Inner.Inner2.II.OpCSeqReturnValue
            opCSeq(Test.Inner.Inner2.C[] c1, Current current) => new Test.Inner.Inner2.II.OpCSeqReturnValue(c1, c1);

            public Test.Inner.Inner2.II.OpCMapReturnValue
            opCMap(Dictionary<string, Test.Inner.Inner2.C> c1, Current current) =>
                new Test.Inner.Inner2.II.OpCMapReturnValue(c1, c1);

            public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();
        }

        class I4 : Inner.Test.Inner2.II
        {
            public Inner.Test.Inner2.II.OpSReturnValue
            opS(Test.S s1, Current current) => new Inner.Test.Inner2.II.OpSReturnValue(s1, s1);

            public Inner.Test.Inner2.II.OpSSeqReturnValue
            opSSeq(Test.S[] s1, Current current) => new Inner.Test.Inner2.II.OpSSeqReturnValue(s1, s1);

            public Inner.Test.Inner2.II.OpSMapReturnValue
            opSMap(Dictionary<string, Test.S> s1, Current current) =>
                new Inner.Test.Inner2.II.OpSMapReturnValue(s1, s1);

            public Inner.Test.Inner2.II.OpCReturnValue
            opC(Test.C c1, Current current) => new Inner.Test.Inner2.II.OpCReturnValue(c1, c1);

            public Inner.Test.Inner2.II.OpCSeqReturnValue
            opCSeq(Test.C[] c1, Current current) => new Inner.Test.Inner2.II.OpCSeqReturnValue(c1, c1);

            public Inner.Test.Inner2.II.OpCMapReturnValue
            opCMap(Dictionary<string, Test.C> c1, Current current) =>
                new Inner.Test.Inner2.II.OpCMapReturnValue(c1, c1);

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
