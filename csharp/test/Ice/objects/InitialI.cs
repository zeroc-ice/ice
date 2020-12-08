// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Objects
{
    public sealed class Initial : IInitial
    {
        private ObjectAdapter _adapter;
        private readonly B _b1;
        private readonly B _b2;
        private readonly C _c;
        private readonly D _d;

        public Initial(ObjectAdapter adapter)
        {
            _adapter = adapter;
            _b1 = new B();
            _b2 = new B();
            _c = new C();
            _d = new D();

            _b1.TheA = _b2; // Cyclic reference to another B
            _b1.TheB = _b1; // Self reference.
            _b1.TheC = null; // Null reference.

            _b2.TheA = _b2; // Self reference, using base.
            _b2.TheB = _b1; // Cyclic reference to another B
            _b2.TheC = _c; // Cyclic reference to a C.

            _c.TheB = _b2; // Cyclic reference to a B.

            _d.TheA = _b1; // Reference to a B.
            _d.TheB = _b2; // Reference to a B.
            _d.TheC = null; // Reference to a C.
        }

        public (B, B, C, D) GetAll(Current current, CancellationToken cancel) => (_b1, _b2, _c, _d);

        public B GetB1(Current current, CancellationToken cancel) => _b1;

        public B GetB2(Current current, CancellationToken cancel) => _b2;

        public C GetC(Current current, CancellationToken cancel) => _c;

        public D GetD(Current current, CancellationToken cancel) => _d;

        public K GetK(Current current, CancellationToken cancel) => new(new L("l"));

        public (AnyClass?, AnyClass?) OpClass(AnyClass? v1, Current current, CancellationToken cancel) => (v1, v1);

        public (IEnumerable<AnyClass?>, IEnumerable<AnyClass?>) OpClassSeq(
            AnyClass?[] v1,
            Current current,
            CancellationToken cancel) =>
            (v1, v1);

        public (IReadOnlyDictionary<string, AnyClass?>, IReadOnlyDictionary<string, AnyClass?>) OpClassMap(
            Dictionary<string, AnyClass?> v1,
            Current current,
            CancellationToken cancel) => (v1, v1);

        public void SetRecursive(Recursive? r, Current current, CancellationToken cancel)
        {
        }

        public bool SupportsClassGraphMaxDepth(Current current, CancellationToken cancel) => true;

        public D1? GetD1(D1? d1, Current current, CancellationToken cancel) => d1;

        public void ThrowEDerived(Current current, CancellationToken cancel) =>
            throw new EDerived(new A1("a1"), new A1("a2"), new A1("a3"), new A1("a4"));

        public void SetG(G? theG, Current current, CancellationToken cancel)
        {
        }
        public (IEnumerable<Base?>, IEnumerable<Base?>) OpBaseSeq(
            Base?[] inS,
            Current current,
            CancellationToken cancel) =>
            (inS, inS);

        public Compact GetCompact(Current current, CancellationToken cancel) => new CompactExt();

        public void Shutdown(Current current, CancellationToken cancel) => _adapter.Communicator.ShutdownAsync();

        public Inner.A GetInnerA(Current current, CancellationToken cancel) => new(_b1);

        public Inner.Sub.A GetInnerSubA(Current current, CancellationToken cancel) => new(new Inner.A(_b1));

        public void ThrowInnerEx(Current current, CancellationToken cancel) => throw new Inner.Ex("Inner::Ex");

        public void ThrowInnerSubEx(Current current, CancellationToken cancel) =>
            throw new Inner.Sub.Ex("Inner::Sub::Ex");

        public IInitial.GetMBMarshaledReturnValue GetMB(Current current, CancellationToken cancel) =>
            new(_b1, current);

        public ValueTask<IInitial.GetAMDMBMarshaledReturnValue> GetAMDMBAsync(
            Current current,
            CancellationToken cancel) =>
            new(new IInitial.GetAMDMBMarshaledReturnValue(_b1, current));

        public (M?, M?) OpM(M? v1, Current current, CancellationToken cancel) => (v1, v1);

        public (F1?, F1?) OpF1(F1? f11, Current current, CancellationToken cancel) => (f11, new F1("F12"));

        public (IF2Prx?, IF2Prx?) OpF2(IF2Prx? f21, Current current, CancellationToken cancel) =>
            (f21, IF2Prx.Parse("F22", current.Communicator));

        public (F3?, F3?) OpF3(F3? f31, Current current, CancellationToken cancel) =>
            (f31, new F3(new F1("F12"), IF2Prx.Parse("F22", current.Communicator)));

        public bool HasF3(Current current, CancellationToken cancel) => true;
    }
}
