//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using Test;

namespace ZeroC.Ice.Test.Scope
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

        private class I2 : Inner.II
        {
            public (Inner.Inner2.S, Inner.Inner2.S)
            opS(Inner.Inner2.S s1, Current current) => (s1, s1);

            public (IEnumerable<Inner.Inner2.S>, IEnumerable<Inner.Inner2.S>)
            opSSeq(Inner.Inner2.S[] s1, Current current) => (s1, s1);

            public (IReadOnlyDictionary<string, Inner.Inner2.S>, IReadOnlyDictionary<string, Inner.Inner2.S>)
            opSMap(Dictionary<string, Inner.Inner2.S> s1, Current current) => (s1, s1);

            public (Inner.Inner2.C?, Inner.Inner2.C?)
            opC(Inner.Inner2.C? c1, Current current) => (c1, c1);

            public (IEnumerable<Inner.Inner2.C?>, IEnumerable<Inner.Inner2.C?>)
            opCSeq(Inner.Inner2.C?[] c1, Current current) => (c1, c1);

            public (IReadOnlyDictionary<string, Inner.Inner2.C?>, IReadOnlyDictionary<string, Inner.Inner2.C?>)
            opCMap(Dictionary<string, Inner.Inner2.C?> c1, Current current) => (c1, c1);

            public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();
        }

        private class I3 : Inner.Inner2.II
        {
            public (Inner.Inner2.S, Inner.Inner2.S)
            opS(Inner.Inner2.S s1, Current current) => (s1, s1);

            public (IEnumerable<Inner.Inner2.S>, IEnumerable<Inner.Inner2.S>)
            opSSeq(Inner.Inner2.S[] s1, Current current) => (s1, s1);

            public (IReadOnlyDictionary<string, Inner.Inner2.S>, IReadOnlyDictionary<string, Inner.Inner2.S>)
            opSMap(Dictionary<string, Inner.Inner2.S> s1, Current current) => (s1, s1);

            public (Inner.Inner2.C?, Inner.Inner2.C?)
            opC(Inner.Inner2.C? c1, Current current) => (c1, c1);

            public (IEnumerable<Inner.Inner2.C?>, IEnumerable<Inner.Inner2.C?>)
            opCSeq(Inner.Inner2.C?[] c1, Current current) => (c1, c1);

            public (IReadOnlyDictionary<string, Inner.Inner2.C?>, IReadOnlyDictionary<string, Inner.Inner2.C?>)
            opCMap(Dictionary<string, Inner.Inner2.C?> c1, Current current) => (c1, c1);

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
            using Communicator communicator = Initialize(ref args);
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
