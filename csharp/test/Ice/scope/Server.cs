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
                public Test.S
                opS(Test.S s1, out Test.S s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public Test.S[]
                opSSeq(Test.S[] s1, out Test.S[] s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public Dictionary<String, Test.S>
                opSMap(Dictionary<String, Test.S> s1, out Dictionary<String, Test.S> s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public Test.C
                opC(Test.C c1, out Test.C c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public Test.C[]
                opCSeq(Test.C[] c1, out Test.C[] c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public Dictionary<String, Test.C>
                opCMap(Dictionary<String, Test.C> c1, out Dictionary<String, Test.C> c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public Test.E1
                opE1(Test.E1 e1, Ice.Current current)
                {
                    return e1;
                }

                public Test.S1
                opS1(Test.S1 s1, Ice.Current current)
                {
                    return s1;
                }

                public Test.C1
                opC1(Test.C1 c1, Ice.Current current)
                {
                    return c1;
                }

                public void shutdown(Ice.Current current)
                {
                    current.Adapter.Communicator.shutdown();
                }
            }

            class I2 : Test.Inner.I
            {
                public Test.Inner.Inner2.S
                opS(Test.Inner.Inner2.S s1, out Test.Inner.Inner2.S s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public Test.Inner.Inner2.S[]
                opSSeq(Test.Inner.Inner2.S[] s1, out Test.Inner.Inner2.S[] s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public Dictionary<String, Test.Inner.Inner2.S>
                opSMap(Dictionary<String, Test.Inner.Inner2.S> s1, out Dictionary<String, Test.Inner.Inner2.S> s2,
                       Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public Test.Inner.Inner2.C
                opC(Test.Inner.Inner2.C c1, out Test.Inner.Inner2.C c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public Test.Inner.Inner2.C[]
                opCSeq(Test.Inner.Inner2.C[] c1, out Test.Inner.Inner2.C[] c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public Dictionary<String, Test.Inner.Inner2.C>
                opCMap(Dictionary<String, Test.Inner.Inner2.C> c1, out Dictionary<String, Test.Inner.Inner2.C> c2,
                       Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public void shutdown(Ice.Current current)
                {
                    current.Adapter.Communicator.shutdown();
                }
            }

            class I3 : Test.Inner.Inner2.I
            {
                public Test.Inner.Inner2.S
                opS(Test.Inner.Inner2.S s1, out Test.Inner.Inner2.S s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public Test.Inner.Inner2.S[]
                opSSeq(Test.Inner.Inner2.S[] s1, out Test.Inner.Inner2.S[] s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public Dictionary<String, Test.Inner.Inner2.S>
                opSMap(Dictionary<String, Test.Inner.Inner2.S> s1, out Dictionary<String, Test.Inner.Inner2.S> s2,
                       Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public Test.Inner.Inner2.C
                opC(Test.Inner.Inner2.C c1, out Test.Inner.Inner2.C c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public Test.Inner.Inner2.C[]
                opCSeq(Test.Inner.Inner2.C[] c1, out Test.Inner.Inner2.C[] c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public Dictionary<String, Test.Inner.Inner2.C>
                opCMap(Dictionary<String, Test.Inner.Inner2.C> c1, out Dictionary<String, Test.Inner.Inner2.C> c2,
                       Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public void shutdown(Ice.Current current)
                {
                    current.Adapter.Communicator.shutdown();
                }
            }

            class I4 : Inner.Test.Inner2.I
            {
                public Test.S
                opS(Test.S s1, out Test.S s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public Test.S[]
                opSSeq(Test.S[] s1, out Test.S[] s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public Dictionary<String, Test.S>
                opSMap(Dictionary<String, Test.S> s1, out Dictionary<String, Test.S> s2,
                       Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public Test.C
                opC(Test.C c1, out Test.C c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public Test.C[]
                opCSeq(Test.C[] c1, out Test.C[] c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public Dictionary<String, Test.C>
                opCMap(Dictionary<String, Test.C> c1, out Dictionary<String, Test.C> c2,
                       Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public void shutdown(Ice.Current current)
                {
                    current.Adapter.Communicator.shutdown();
                }
            }

            public override void run(string[] args)
            {
                var initData = new InitializationData();
                initData.typeIdNamespaces = new string[] { "Ice.scope.TypeId" };
                initData.properties = createTestProperties(ref args);
                using (var communicator = initialize(initData))
                {
                    communicator.Properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    adapter.Add(new I1(), "i1");
                    adapter.Add(new I2(), "i2");
                    adapter.Add(new I3(), "i3");
                    adapter.Add(new I4(), "i4");
                    adapter.Activate();
                    serverReady();
                    communicator.waitForShutdown();
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Server>(args);
            }
        }
    }
}
