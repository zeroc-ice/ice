//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using System.Collections.Generic;

namespace Ice.objects
{
    public sealed class Initial : Test.IInitial
    {
        public Initial(ObjectAdapter adapter)
        {
            _adapter = adapter;
            _b1 = new Test.B();
            _b2 = new Test.B();
            _c = new Test.C();
            _d = new Test.D();

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

        public (Test.B, Test.B, Test.C, Test.D)
        getAll(Current current) => (_b1, _b2, _c, _d);

        public Test.B getB1(Current current) => _b1;

        public Test.B getB2(Current current) => _b2;

        public Test.C getC(Current current) => _c;

        public Test.D getD(Current current) => _d;

        public Test.K getK(Current current) => new Test.K(new Test.L("l"));

        public (AnyClass?, AnyClass?) opClass(AnyClass? v1, Current current) => (v1, v1);

        public (IEnumerable<AnyClass?>, IEnumerable<AnyClass?>) opClassSeq(AnyClass?[] v1, Current current) => (v1, v1);

        public (IReadOnlyDictionary<string, AnyClass?>, IReadOnlyDictionary<string, AnyClass?>) opClassMap(
            Dictionary<string, AnyClass?> v1, Current current) => (v1, v1);

        public void setRecursive(Test.Recursive? r, Current current)
        {
        }

        public bool supportsClassGraphDepthMax(Current current) => true;

        public Test.D1? getD1(Test.D1? d1, Current current) => d1;

        public void throwEDerived(Current current) =>
            throw new Test.EDerived(new Test.A1("a1"), new Test.A1("a2"), new Test.A1("a3"), new Test.A1("a4"));

        public void setG(Test.G? theG, Current current)
        {
        }
        public (IEnumerable<Test.Base?>, IEnumerable<Test.Base?>) opBaseSeq(Test.Base?[] inS, Current current) =>
            (inS, inS);

        public Test.Compact getCompact(Current current) => new Test.CompactExt();

        public void shutdown(Current current) => _adapter.Communicator.Shutdown();

        public Test.Inner.A
        getInnerA(Current current) => new Test.Inner.A(_b1);

        public Test.Inner.Sub.A
        getInnerSubA(Current current) => new Test.Inner.Sub.A(new Test.Inner.A(_b1));

        public void throwInnerEx(Current current) => throw new Test.Inner.Ex("Inner::Ex");

        public void throwInnerSubEx(Current current) => throw new Test.Inner.Sub.Ex("Inner::Sub::Ex");

        public Test.IInitial.GetMBMarshaledReturnValue getMB(Current current) =>
            new Test.IInitial.GetMBMarshaledReturnValue(_b1, current);

        public ValueTask<Test.IInitial.GetAMDMBMarshaledReturnValue> getAMDMBAsync(Current current)
            => new ValueTask<Test.IInitial.GetAMDMBMarshaledReturnValue>(
                new Test.IInitial.GetAMDMBMarshaledReturnValue(_b1, current));

        public (Test.M?, Test.M?) opM(Test.M? v1, Current current) => (v1, v1);

        public (Test.F1?, Test.F1?) opF1(Test.F1? f11, Current current) => (f11, new Test.F1("F12"));

        public (Test.IF2Prx?, Test.IF2Prx?) opF2(Test.IF2Prx? f21, Current current) =>
            (f21, Test.IF2Prx.Parse("F22", current.Adapter.Communicator));

        public (Test.F3?, Test.F3?) opF3(Test.F3? f31, Current current) =>
            (f31, new Test.F3(new Test.F1("F12"), Test.IF2Prx.Parse("F22", current.Adapter.Communicator)));

        public bool hasF3(Current current) => true;

        private ObjectAdapter _adapter;
        private Test.B _b1;
        private Test.B _b2;
        private Test.C _c;
        private Test.D _d;
    }
}
