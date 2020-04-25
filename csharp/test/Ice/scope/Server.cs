//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice.scope.Test;
using System.Collections.Generic;
using Test;

namespace Ice.scope
{
    public class Server : TestHelper
    {
        private class I1 : II
        {
            public (S, S) opS(S s1, Current current) => (s1, s1);

            public (IEnumerable<S>, IEnumerable<S>) opSSeq(S[] s1, Current current) => (s1, s1);

            public (IReadOnlyDictionary<string, S>, IReadOnlyDictionary<string, S>)
            opSMap(Dictionary<string, S> s1, Current current) => (s1, s1);

            public (C?, C?) opC(C? c1, Current current) => (c1, c1);

            public (IEnumerable<C?>, IEnumerable<C?>) opCSeq(C?[] c1, Current current) => (c1, c1);

            public (IReadOnlyDictionary<string, C?>, IReadOnlyDictionary<string, C?>)
            opCMap(Dictionary<string, C?> c1, Current current) => (c1, c1);

            public E1 opE1(E1 e1, Current current) => e1;

            public S1 opS1(S1 s1, Current current) => s1;

            public C1? opC1(C1? c1, Current current) => c1;

            public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();
        }

        private class I2 : Test.Inner.II
        {
            public (Test.Inner.Inner2.S, Test.Inner.Inner2.S)
            opS(Test.Inner.Inner2.S s1, Current current) => (s1, s1);

            public (IEnumerable<Test.Inner.Inner2.S>, IEnumerable<Test.Inner.Inner2.S>)
            opSSeq(Test.Inner.Inner2.S[] s1, Current current) => (s1, s1);

            public (IReadOnlyDictionary<string, Test.Inner.Inner2.S>, IReadOnlyDictionary<string, Test.Inner.Inner2.S>)
            opSMap(Dictionary<string, Test.Inner.Inner2.S> s1, Current current) => (s1, s1);

            public (Test.Inner.Inner2.C?, Test.Inner.Inner2.C?)
            opC(Test.Inner.Inner2.C? c1, Current current) => (c1, c1);

            public (IEnumerable<Test.Inner.Inner2.C?>, IEnumerable<Test.Inner.Inner2.C?>)
            opCSeq(Test.Inner.Inner2.C?[] c1, Current current) => (c1, c1);

            public (IReadOnlyDictionary<string, Test.Inner.Inner2.C?>, IReadOnlyDictionary<string, Test.Inner.Inner2.C?>)
            opCMap(Dictionary<string, Test.Inner.Inner2.C?> c1, Current current) => (c1, c1);

            public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();
        }

        private class I3 : Test.Inner.Inner2.II
        {
            public (Test.Inner.Inner2.S, Test.Inner.Inner2.S)
            opS(Test.Inner.Inner2.S s1, Current current) => (s1, s1);

            public (IEnumerable<Test.Inner.Inner2.S>, IEnumerable<Test.Inner.Inner2.S>)
            opSSeq(Test.Inner.Inner2.S[] s1, Current current) => (s1, s1);

            public (IReadOnlyDictionary<string, Test.Inner.Inner2.S>, IReadOnlyDictionary<string, Test.Inner.Inner2.S>)
            opSMap(Dictionary<string, Test.Inner.Inner2.S> s1, Current current) => (s1, s1);

            public (Test.Inner.Inner2.C?, Test.Inner.Inner2.C?)
            opC(Test.Inner.Inner2.C? c1, Current current) => (c1, c1);

            public (IEnumerable<Test.Inner.Inner2.C?>, IEnumerable<Test.Inner.Inner2.C?>)
            opCSeq(Test.Inner.Inner2.C?[] c1, Current current) => (c1, c1);

            public (IReadOnlyDictionary<string, Test.Inner.Inner2.C?>, IReadOnlyDictionary<string, Test.Inner.Inner2.C?>)
            opCMap(Dictionary<string, Test.Inner.Inner2.C?> c1, Current current) => (c1, c1);

            public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();
        }

        private class I4 : Inner.Test.Inner2.II
        {
            public (S, S) opS(S s1, Current current) => (s1, s1);

            public (IEnumerable<S>, IEnumerable<S>) opSSeq(S[] s1, Current current) => (s1, s1);

            public (IReadOnlyDictionary<string, S>, IReadOnlyDictionary<string, S>)
            opSMap(Dictionary<string, S> s1, Current current) => (s1, s1);

            public (C?, C?) opC(C? c1, Current current) => (c1, c1);

            public (IEnumerable<C?>, IEnumerable<C?>) opCSeq(C?[] c1, Current current) => (c1, c1);

            public (IReadOnlyDictionary<string, C?>, IReadOnlyDictionary<string, C?>)
            opCMap(Dictionary<string, C?> c1, Current current) => (c1, c1);

            public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();
        }

        public override void Run(string[] args)
        {
            using Communicator communicator = Initialize(CreateTestProperties(ref args),
                typeIdNamespaces: new string[] { "Ice.scope.TypeId" });
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("i1", new I1());
            adapter.Add("i2", new I2());
            adapter.Add("i3", new I3());
            adapter.Add("i4", new I4());
            adapter.Activate();
            ServerReady();
            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
    }
}
