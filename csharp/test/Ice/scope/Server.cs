// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice
{
    namespace scope
    {
        public class Server : TestHelper
        {
            private class MyInterface1 : Test.MyInterfaceDisp_
            {
                public override Test.MyStruct
                opMyStruct(Test.MyStruct s1, out Test.MyStruct s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public override Test.MyStruct[]
                opMyStructSeq(Test.MyStruct[] s1, out Test.MyStruct[] s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public override Dictionary<String, Test.MyStruct>
                opMyStructMap(Dictionary<String, Test.MyStruct> s1, out Dictionary<String, Test.MyStruct> s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public override Test.MyClass
                opMyClass(Test.MyClass c1, out Test.MyClass c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public override Test.MyClass[]
                opMyClassSeq(Test.MyClass[] c1, out Test.MyClass[] c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public override Dictionary<String, Test.MyClass>
                opMyClassMap(Dictionary<String, Test.MyClass> c1, out Dictionary<String, Test.MyClass> c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public override Test.MyEnum
                opMyEnum(Test.MyEnum e1, Ice.Current current)
                {
                    return e1;
                }

                public override Test.MyOtherStruct
                opMyOtherStruct(Test.MyOtherStruct s1, Ice.Current current)
                {
                    return s1;
                }

                public override Test.MyOtherClass
                opMyOtherClass(Test.MyOtherClass c1, Ice.Current current)
                {
                    return c1;
                }

                override
                public void shutdown(Ice.Current current)
                {
                    current.adapter.getCommunicator().shutdown();
                }
            }

            private class MyInterface2 : Test.Inner.MyInterfaceDisp_
            {
                public override Test.Inner.Inner2.MyStruct
                opMyStruct(Test.Inner.Inner2.MyStruct s1, out Test.Inner.Inner2.MyStruct s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public override Test.Inner.Inner2.MyStruct[]
                opMyStructSeq(Test.Inner.Inner2.MyStruct[] s1, out Test.Inner.Inner2.MyStruct[] s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public override Dictionary<String, Test.Inner.Inner2.MyStruct>
                opMyStructMap(Dictionary<String, Test.Inner.Inner2.MyStruct> s1, out Dictionary<String, Test.Inner.Inner2.MyStruct> s2,
                       Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public override Test.Inner.Inner2.MyClass
                opMyClass(Test.Inner.Inner2.MyClass c1, out Test.Inner.Inner2.MyClass c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public override Test.Inner.Inner2.MyClass[]
                opMyClassSeq(Test.Inner.Inner2.MyClass[] c1, out Test.Inner.Inner2.MyClass[] c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public override Dictionary<String, Test.Inner.Inner2.MyClass>
                opMyClassMap(Dictionary<String, Test.Inner.Inner2.MyClass> c1, out Dictionary<String, Test.Inner.Inner2.MyClass> c2,
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

            private class MyInterface3 : Test.Inner.Inner2.MyInterfaceDisp_
            {
                public override Test.Inner.Inner2.MyStruct
                opMyStruct(Test.Inner.Inner2.MyStruct s1, out Test.Inner.Inner2.MyStruct s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public override Test.Inner.Inner2.MyStruct[]
                opMyStructSeq(Test.Inner.Inner2.MyStruct[] s1, out Test.Inner.Inner2.MyStruct[] s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public override Dictionary<String, Test.Inner.Inner2.MyStruct>
                opMyStructMap(Dictionary<String, Test.Inner.Inner2.MyStruct> s1, out Dictionary<String, Test.Inner.Inner2.MyStruct> s2,
                       Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public override Test.Inner.Inner2.MyClass
                opMyClass(Test.Inner.Inner2.MyClass c1, out Test.Inner.Inner2.MyClass c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public override Test.Inner.Inner2.MyClass[]
                opMyClassSeq(Test.Inner.Inner2.MyClass[] c1, out Test.Inner.Inner2.MyClass[] c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public override Dictionary<String, Test.Inner.Inner2.MyClass>
                opMyClassMap(Dictionary<String, Test.Inner.Inner2.MyClass> c1, out Dictionary<String, Test.Inner.Inner2.MyClass> c2,
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

            private class MyInterface4 : Inner.Test.Inner2.MyInterfaceDisp_
            {
                public override Test.MyStruct
                opMyStruct(Test.MyStruct s1, out Test.MyStruct s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public override Test.MyStruct[]
                opMyStructSeq(Test.MyStruct[] s1, out Test.MyStruct[] s2, Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public override Dictionary<String, Test.MyStruct>
                opMyStructMap(Dictionary<String, Test.MyStruct> s1, out Dictionary<String, Test.MyStruct> s2,
                       Ice.Current current)
                {
                    s2 = s1;
                    return s1;
                }

                public override Test.MyClass
                opMyClass(Test.MyClass c1, out Test.MyClass c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public override Test.MyClass[]
                opMyClassSeq(Test.MyClass[] c1, out Test.MyClass[] c2, Ice.Current current)
                {
                    c2 = c1;
                    return c1;
                }

                public override Dictionary<String, Test.MyClass>
                opMyClassMap(Dictionary<String, Test.MyClass> c1, out Dictionary<String, Test.MyClass> c2,
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
                initData.properties = createTestProperties(ref args);
                using (var communicator = initialize(initData))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    adapter.add(new MyInterface1(), Ice.Util.stringToIdentity("i1"));
                    adapter.add(new MyInterface2(), Ice.Util.stringToIdentity("i2"));
                    adapter.add(new MyInterface3(), Ice.Util.stringToIdentity("i3"));
                    adapter.add(new MyInterface4(), Ice.Util.stringToIdentity("i4"));
                    adapter.activate();
                    serverReady();
                    communicator.waitForShutdown();
                }
            }

            public static Task<int> Main(string[] args) =>
                TestDriver.runTestAsync<Server>(args);
        }
    }
}
