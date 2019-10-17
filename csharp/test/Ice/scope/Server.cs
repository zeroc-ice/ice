//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using Test;

namespace Ice
{
    namespace scope
    {
        public class Server : TestHelper
        {
            class I1 : Test.IDisp_
            {
                public override Test.S
                opS(Test.S s1, out Test.S s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public override Test.S[]
                opSSeq(Test.S[] s1, out Test.S[] s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public override Dictionary<String, Test.S>
                opSMap(Dictionary<String, Test.S> s1, out Dictionary<String, Test.S> s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public override Test.C
                opC(Test.C c1, out Test.C c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public override Test.C[]
                opCSeq(Test.C[] c1, out Test.C[] c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public override Dictionary<String, Test.C>
                opCMap(Dictionary<String, Test.C> c1, out Dictionary<String, Test.C> c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public override Test.E1
                opE1(Test.E1 e1, Ice.Current current)
                {
                    return e1;
                }

                public override Test.S1
                opS1(Test.S1 s1, Ice.Current current)
                {
                    return s1;
                }

                public override Test.C1
                opC1(Test.C1 c1, Ice.Current current)
                {
                    return c1;
                }

                override
                public void shutdown(Ice.Current current)
                {
                    current.adapter.getCommunicator().shutdown();
                }
            }

            class I2 : Test.Inner.IDisp_
            {
                public override Test.Inner.Inner2.S
                opS(Test.Inner.Inner2.S s1, out Test.Inner.Inner2.S s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public override Test.Inner.Inner2.S[]
                opSSeq(Test.Inner.Inner2.S[] s1, out Test.Inner.Inner2.S[] s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public override Dictionary<String, Test.Inner.Inner2.S>
                opSMap(Dictionary<String, Test.Inner.Inner2.S> s1, out Dictionary<String, Test.Inner.Inner2.S> s2,
                       Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public override Test.Inner.Inner2.C
                opC(Test.Inner.Inner2.C c1, out Test.Inner.Inner2.C c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public override Test.Inner.Inner2.C[]
                opCSeq(Test.Inner.Inner2.C[] c1, out Test.Inner.Inner2.C[] c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public override Dictionary<String, Test.Inner.Inner2.C>
                opCMap(Dictionary<String, Test.Inner.Inner2.C> c1, out Dictionary<String, Test.Inner.Inner2.C> c2,
                       Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                override
                public void shutdown(Ice.Current current)
                {
                    current.adapter.getCommunicator().shutdown();
                }
            }

            class I3 : Test.Inner.Inner2.IDisp_
            {
                public override Test.Inner.Inner2.S
                opS(Test.Inner.Inner2.S s1, out Test.Inner.Inner2.S s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public override Test.Inner.Inner2.S[]
                opSSeq(Test.Inner.Inner2.S[] s1, out Test.Inner.Inner2.S[] s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public override Dictionary<String, Test.Inner.Inner2.S>
                opSMap(Dictionary<String, Test.Inner.Inner2.S> s1, out Dictionary<String, Test.Inner.Inner2.S> s2,
                       Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public override Test.Inner.Inner2.C
                opC(Test.Inner.Inner2.C c1, out Test.Inner.Inner2.C c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public override Test.Inner.Inner2.C[]
                opCSeq(Test.Inner.Inner2.C[] c1, out Test.Inner.Inner2.C[] c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public override Dictionary<String, Test.Inner.Inner2.C>
                opCMap(Dictionary<String, Test.Inner.Inner2.C> c1, out Dictionary<String, Test.Inner.Inner2.C> c2,
                       Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                override
                public void shutdown(Ice.Current current)
                {
                    current.adapter.getCommunicator().shutdown();
                }
            }

            class I4 : Inner.Test.Inner2.IDisp_
            {
                public override Test.S
                opS(Test.S s1, out Test.S s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public override Test.S[]
                opSSeq(Test.S[] s1, out Test.S[] s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public override Dictionary<String, Test.S>
                opSMap(Dictionary<String, Test.S> s1, out Dictionary<String, Test.S> s2,
                       Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public override Test.C
                opC(Test.C c1, out Test.C c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public override Test.C[]
                opCSeq(Test.C[] c1, out Test.C[] c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public override Dictionary<String, Test.C>
                opCMap(Dictionary<String, Test.C> c1, out Dictionary<String, Test.C> c2,
                       Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public override void shutdown(Ice.Current current)
                {
                    current.adapter.getCommunicator().shutdown();
                }
            }

            public override void run(string[] args)
            {
                var initData = new InitializationData();
                initData.typeIdNamespaces = new string[]{"Ice.scope.TypeId"};
                initData.properties = createTestProperties(ref args);
                using(var communicator = initialize(initData))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    adapter.add(new I1(), Ice.Util.stringToIdentity("i1"));
                    adapter.add(new I2(), Ice.Util.stringToIdentity("i2"));
                    adapter.add(new I3(), Ice.Util.stringToIdentity("i3"));
                    adapter.add(new I4(), Ice.Util.stringToIdentity("i4"));
                    adapter.activate();
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
