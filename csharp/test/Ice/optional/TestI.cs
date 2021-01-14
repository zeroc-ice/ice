// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Optional
{
    internal class Test : ITest
    {
        public void Shutdown(Current current, CancellationToken cancel) =>
            current.Communicator.ShutdownAsync();

        public void OpInt(int? i1, Current current, CancellationToken cancel) =>
            TestHelper.Assert(i1 == null || i1.Value == 42);

        public void OpString(string? i1, Current current, CancellationToken cancel) =>
            TestHelper.Assert(i1 == null || i1 == "42");

        public int? OpReturnInt(Current current, CancellationToken cancel) => 42;

        public string? OpReturnString(Current current, CancellationToken cancel) => "42";

        public void OpBasic(int i1, int? i2, string? i3, string i4, Current current, CancellationToken cancel)
        {
            TestHelper.Assert(i2 == null || i2.Value == i1);
            TestHelper.Assert(i3 == null || i3 == i4);
        }

        public (int? R1, int R2, int? R3, string? R4) OpBasicReturnTuple(
            int i1,
            int? i2,
            string? i3,
            Current current,
            CancellationToken cancel) => (i2, i1, i2, i3);

        public IObjectPrx? OpObject(IObjectPrx i1, IObjectPrx? i2, Current current, CancellationToken cancel)
        {
            TestHelper.Assert(i2 == null || i2.Equals(i1));
            return i2;
        }

        public ITestPrx? OpTest(ITestPrx i1, ITestPrx? i2, Current current, CancellationToken cancel)
        {
            TestHelper.Assert(i2 == null || i2.Equals(i1));
            return i2;
        }

        public AnyClass? OpAnyClass(AnyClass i1, AnyClass? i2, Current current, CancellationToken cancel)
        {
            TestHelper.Assert(i2 == null || i2 == i1);
            return i2;
        }

        public C? OpC(C i1, C? i2, Current current, CancellationToken cancel)
        {
            TestHelper.Assert(i2 == null || i2 == i1);
            return i2;
        }

        public IEnumerable<int?> OpOptIntSeq(int?[] i1, Current current, CancellationToken cancel) => i1;

        public IEnumerable<int?>? OpTaggedOptIntSeq(int?[]? i1, Current current, CancellationToken cancel) => i1;

        public IEnumerable<string?> OpOptStringSeq(string?[] i1, Current current, CancellationToken cancel) => i1;

        public IEnumerable<string?>? OpTaggedOptStringSeq(string?[]? i1, Current current, CancellationToken cancel) =>
            i1;

        public IReadOnlyDictionary<int, int?> OpIntOptIntDict(
            Dictionary<int, int?> i1,
            Current current,
            CancellationToken cancel) => i1;

        public IReadOnlyDictionary<int, int?>? OpTaggedIntOptIntDict(
            Dictionary<int, int?>? i1,
            Current current,
            CancellationToken cancel) => i1;

        public IReadOnlyDictionary<int, string?> OpIntOptStringDict(
            Dictionary<int, string?> i1,
            Current current,
            CancellationToken cancel) =>
            i1;

        public IReadOnlyDictionary<int, string?>? OpTaggedIntOptStringDict(
            Dictionary<int, string?>? i1,
            Current current,
            CancellationToken cancel) => i1;

        public MyStruct OpMyStruct(MyStruct i1, Current current, CancellationToken cancel) => i1;

        public MyStruct? OpOptMyStruct(MyStruct? i1, Current current, CancellationToken cancel) => i1;

        public Derived OpDerived(Derived i1, Current current, CancellationToken cancel) => i1;

        public Derived? OpOptDerived(Derived? i1, Current current, CancellationToken cancel) => i1;

        public void OpDerivedEx(Current current, CancellationToken cancel)
        {
            if (current.Context.ContainsKey("all null"))
            {
                throw new DerivedEx();
            }
            else
            {
                throw new DerivedEx(null, 5, new string?[] { "foo", null, "bar" }, new C(42), "test");
            }
        }
    }
}
