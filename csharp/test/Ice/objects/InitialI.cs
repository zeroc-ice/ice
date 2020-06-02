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

            _b1.theA = _b2; // Cyclic reference to another B
            _b1.theB = _b1; // Self reference.
            _b1.theC = null; // Null reference.

            _b2.theA = _b2; // Self reference, using base.
            _b2.theB = _b1; // Cyclic reference to another B
            _b2.theC = _c; // Cyclic reference to a C.

            _c.theB = _b2; // Cyclic reference to a B.

            _d.theA = _b1; // Reference to a B.
            _d.theB = _b2; // Reference to a B.
            _d.theC = null; // Reference to a C.
        }

        public (B, B, C, D)
        getAll(Current current) => (_b1, _b2, _c, _d);

        public B getB1(Current current) => _b1;

        public B getB2(Current current) => _b2;

        public C getC(Current current) => _c;

        public D getD(Current current) => _d;

        public K getK(Current current) => new K(new L("l"));

        public (AnyClass?, AnyClass?) opClass(AnyClass? v1, Current current) => (v1, v1);

        public (IEnumerable<AnyClass?>, IEnumerable<AnyClass?>) opClassSeq(AnyClass?[] v1, Current current) => (v1, v1);

        public (IReadOnlyDictionary<string, AnyClass?>, IReadOnlyDictionary<string, AnyClass?>) opClassMap(
            Dictionary<string, AnyClass?> v1, Current current) => (v1, v1);

        public void setRecursive(Recursive? r, Current current)
        {
        }

        public bool supportsClassGraphDepthMax(Current current) => true;

        public D1? getD1(D1? d1, Current current) => d1;

        public void throwEDerived(Current current) =>
            throw new EDerived(new A1("a1"), new A1("a2"), new A1("a3"), new A1("a4"));

        public void setG(G? theG, Current current)
        {
        }
        public (IEnumerable<Base?>, IEnumerable<Base?>) opBaseSeq(Base?[] inS, Current current) =>
            (inS, inS);

        public Compact getCompact(Current current) => new CompactExt();

        public void shutdown(Current current) => _adapter.Communicator.Shutdown();

        public Inner.A
        getInnerA(Current current) => new Inner.A(_b1);

        public Inner.Sub.A
        getInnerSubA(Current current) => new Inner.Sub.A(new Inner.A(_b1));

        public void throwInnerEx(Current current) => throw new Inner.Ex("Inner::Ex");

        public void throwInnerSubEx(Current current) => throw new Inner.Sub.Ex("Inner::Sub::Ex");

        public IInitial.GetMBMarshaledReturnValue getMB(Current current) =>
            new IInitial.GetMBMarshaledReturnValue(_b1, current);

        public ValueTask<IInitial.GetAMDMBMarshaledReturnValue> getAMDMBAsync(Current current)
            => new ValueTask<IInitial.GetAMDMBMarshaledReturnValue>(
                new IInitial.GetAMDMBMarshaledReturnValue(_b1, current));

        public (M?, M?) opM(M? v1, Current current) => (v1, v1);

        public (F1?, F1?) opF1(F1? f11, Current current) => (f11, new F1("F12"));

        public (IF2Prx?, IF2Prx?) opF2(IF2Prx? f21, Current current) =>
            (f21, IF2Prx.Parse("F22", current.Adapter.Communicator));

        public (F3?, F3?) opF3(F3? f31, Current current) =>
            (f31, new F3(new F1("F12"), IF2Prx.Parse("F22", current.Adapter.Communicator)));

        public bool hasF3(Current current) => true;

        private ObjectAdapter _adapter;
        private B _b1;
        private B _b2;
        private C _c;
        private D _d;
    }
}
