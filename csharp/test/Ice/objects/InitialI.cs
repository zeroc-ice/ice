//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using System.Collections.Generic;

namespace ZeroC.Ice.Test.Objects
{
    public sealed class Initial : IInitial
    {
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

        public (B, B, C, D) GetAll(Current current) => (_b1, _b2, _c, _d);

        public B GetB1(Current current) => _b1;

        public B GetB2(Current current) => _b2;

        public C GetC(Current current) => _c;

        public D GetD(Current current) => _d;

        public K GetK(Current current) => new K(new L("l"));

        public (AnyClass?, AnyClass?) OpClass(AnyClass? v1, Current current) => (v1, v1);

        public (IEnumerable<AnyClass?>, IEnumerable<AnyClass?>) OpClassSeq(AnyClass?[] v1, Current current) =>
            (v1, v1);

        public (IReadOnlyDictionary<string, AnyClass?>, IReadOnlyDictionary<string, AnyClass?>) OpClassMap(
            Dictionary<string, AnyClass?> v1, Current current) => (v1, v1);

        public void SetRecursive(Recursive? r, Current current)
        {
        }

        public bool SupportsClassGraphDepthMax(Current current) => true;

        public D1? GetD1(D1? d1, Current current) => d1;

        public void ThrowEDerived(Current current) =>
            throw new EDerived(new A1("a1"), new A1("a2"), new A1("a3"), new A1("a4"));

        public void SetG(G? theG, Current current)
        {
        }
        public (IEnumerable<Base?>, IEnumerable<Base?>) OpBaseSeq(Base?[] inS, Current current) =>
            (inS, inS);

        public Compact GetCompact(Current current) => new CompactExt();

        public void Shutdown(Current current) => _adapter.Communicator.ShutdownAsync();

        public Inner.A GetInnerA(Current current) => new Inner.A(_b1);

        public Inner.Sub.A GetInnerSubA(Current current) => new Inner.Sub.A(new Inner.A(_b1));

        public void ThrowInnerEx(Current current) => throw new Inner.Ex("Inner::Ex");

        public void ThrowInnerSubEx(Current current) => throw new Inner.Sub.Ex("Inner::Sub::Ex");

        public IInitial.GetMBMarshaledReturnValue GetMB(Current current) =>
            new IInitial.GetMBMarshaledReturnValue(_b1, current);

        public ValueTask<IInitial.GetAMDMBMarshaledReturnValue> GetAMDMBAsync(Current current) =>
            new ValueTask<IInitial.GetAMDMBMarshaledReturnValue>(
                new IInitial.GetAMDMBMarshaledReturnValue(_b1, current));

        public (M?, M?) OpM(M? v1, Current current) => (v1, v1);

        public (F1?, F1?) OpF1(F1? f11, Current current) => (f11, new F1("F12"));

        public (IF2Prx?, IF2Prx?) OpF2(IF2Prx? f21, Current current) =>
            (f21, IF2Prx.Parse("F22", current.Adapter.Communicator));

        public (F3?, F3?) OpF3(F3? f31, Current current) =>
            (f31, new F3(new F1("F12"), IF2Prx.Parse("F22", current.Adapter.Communicator)));

        public bool HasF3(Current current) => true;

        private ObjectAdapter _adapter;
        private B _b1;
        private B _b2;
        private C _c;
        private D _d;
    }
}
