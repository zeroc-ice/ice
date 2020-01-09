//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using Test;
using Ice.scope.Test;
using Ice.scope.Test.Inner;
using Ice.scope.Test.Inner.Inner2;
using Inner.Test.Inner2;

namespace Ice
{
    namespace scope
    {
        public class Server : TestHelper
        {
            class I1 : Test.I
            {
                public Test.I.OpSReturnValue
                opS(Test.S s1, Current current) => new Test.I.OpSReturnValue(s1, s1);

                public Test.I.OpSSeqReturnValue
                opSSeq(Test.S[] s1, Current current) => new Test.I.OpSSeqReturnValue(s1, s1);

                public Test.I.OpSMapReturnValue
                opSMap(Dictionary<string, Test.S> s1, Current current) => new Test.I.OpSMapReturnValue(s1, s1);

                public Test.I.OpCReturnValue
                opC(Test.C c1, Current current) => new Test.I.OpCReturnValue(c1, c1);

                public Test.I.OpCSeqReturnValue
                opCSeq(Test.C[] c1, Current current) => new Test.I.OpCSeqReturnValue(c1, c1);

                public Test.I.OpCMapReturnValue
                opCMap(Dictionary<string, Test.C> c1, Current current) => new Test.I.OpCMapReturnValue(c1, c1);

                public E1 opE1(E1 e1, Current current) => e1;

                public S1 opS1(S1 s1, Current current) => s1;

                public C1 opC1(C1 c1, Current current) => c1;

                public void shutdown(Current current) => current.Adapter.Communicator.shutdown();
            }

            class I2 : Test.Inner.I
            {
                public Test.Inner.I.OpSReturnValue
                opS(Test.Inner.Inner2.S s1, Current current) => new Test.Inner.I.OpSReturnValue(s1, s1);

                public Test.Inner.I.OpSSeqReturnValue
                opSSeq(Test.Inner.Inner2.S[] s1, Current current) => new Test.Inner.I.OpSSeqReturnValue(s1, s1);

                public Test.Inner.I.OpSMapReturnValue
                opSMap(Dictionary<string, Test.Inner.Inner2.S> s1, Current current) =>
                    new Test.Inner.I.OpSMapReturnValue(s1, s1);

                public Test.Inner.I.OpCReturnValue
                opC(Test.Inner.Inner2.C c1, Current current) => new Test.Inner.I.OpCReturnValue(c1, c1);

                public Test.Inner.I.OpCSeqReturnValue
                opCSeq(Test.Inner.Inner2.C[] c1, Current current) => new Test.Inner.I.OpCSeqReturnValue(c1, c1);

                public Test.Inner.I.OpCMapReturnValue
                opCMap(Dictionary<string, Test.Inner.Inner2.C> c1, Current current) =>
                    new Test.Inner.I.OpCMapReturnValue(c1, c1);

                public void shutdown(Current current) => current.Adapter.Communicator.shutdown();
            }

            class I3 : Test.Inner.Inner2.I
            {
                public Test.Inner.Inner2.I.OpSReturnValue
                opS(Test.Inner.Inner2.S s1, Current current) => new Test.Inner.Inner2.I.OpSReturnValue(s1, s1);

                public Test.Inner.Inner2.I.OpSSeqReturnValue
                opSSeq(Test.Inner.Inner2.S[] s1, Current current) => new Test.Inner.Inner2.I.OpSSeqReturnValue(s1, s1);

                public Test.Inner.Inner2.I.OpSMapReturnValue
                opSMap(Dictionary<string, Test.Inner.Inner2.S> s1, Current current) =>
                    new Test.Inner.Inner2.I.OpSMapReturnValue(s1, s1);

                public Test.Inner.Inner2.I.OpCReturnValue
                opC(Test.Inner.Inner2.C c1, Current current) => new Test.Inner.Inner2.I.OpCReturnValue(c1, c1);

                public Test.Inner.Inner2.I.OpCSeqReturnValue
                opCSeq(Test.Inner.Inner2.C[] c1, Current current) => new Test.Inner.Inner2.I.OpCSeqReturnValue(c1, c1);

                public Test.Inner.Inner2.I.OpCMapReturnValue
                opCMap(Dictionary<string, Test.Inner.Inner2.C> c1, Current current) =>
                    new Test.Inner.Inner2.I.OpCMapReturnValue(c1, c1);

                public void shutdown(Current current) => current.Adapter.Communicator.shutdown();
            }

            class I4 : Inner.Test.Inner2.I
            {
                public Inner.Test.Inner2.I.OpSReturnValue
                opS(Test.S s1, Current current) => new Inner.Test.Inner2.I.OpSReturnValue(s1, s1);

                public Inner.Test.Inner2.I.OpSSeqReturnValue
                opSSeq(Test.S[] s1, Current current) => new Inner.Test.Inner2.I.OpSSeqReturnValue(s1, s1);

                public Inner.Test.Inner2.I.OpSMapReturnValue
                opSMap(Dictionary<string, Test.S> s1, Current current) =>
                    new Inner.Test.Inner2.I.OpSMapReturnValue(s1, s1);

                public Inner.Test.Inner2.I.OpCReturnValue
                opC(Test.C c1, Current current) => new Inner.Test.Inner2.I.OpCReturnValue(c1, c1);

                public Inner.Test.Inner2.I.OpCSeqReturnValue
                opCSeq(Test.C[] c1, Current current) => new Inner.Test.Inner2.I.OpCSeqReturnValue(c1, c1);

                public Inner.Test.Inner2.I.OpCMapReturnValue
                opCMap(Dictionary<string, Test.C> c1, Current current) =>
                    new Inner.Test.Inner2.I.OpCMapReturnValue(c1, c1);

                public void shutdown(Current current) => current.Adapter.Communicator.shutdown();
            }

            public override void run(string[] args)
            {
                using var communicator = initialize(createTestProperties(ref args),
                    typeIdNamespaces: new string[] { "Ice.scope.TypeId" });
                communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                adapter.Add(new I1(), "i1");
                adapter.Add(new I2(), "i2");
                adapter.Add(new I3(), "i3");
                adapter.Add(new I4(), "i4");
                adapter.Activate();
                serverReady();
                communicator.waitForShutdown();
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Server>(args);
            }
        }
    }
}
