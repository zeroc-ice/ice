//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using System.Collections.Generic;

namespace Ice
{
    namespace objects
    {
        public sealed class InitialI : Test.Initial
        {
            public InitialI(Ice.ObjectAdapter adapter)
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

            public void getAll(out Test.B b1, out Test.B b2, out Test.C c, out Test.D d, Ice.Current current)
            {
                b1 = _b1;
                b2 = _b2;
                c = _c;
                d = _d;
            }

            public Test.B getB1(Ice.Current current)
            {
                return _b1;
            }

            public Test.B getB2(Ice.Current current)
            {
                return _b2;
            }

            public Test.C getC(Ice.Current current)
            {
                return _c;
            }

            public Test.D getD(Ice.Current current)
            {
                return _d;
            }

            public Test.K getK(Ice.Current current)
            {
                return new Test.K(new Test.L("l"));
            }

            public Ice.Value opValue(Ice.Value v1, out Ice.Value v2, Ice.Current current)
            {
                v2 = v1;
                return v1;
            }

            public Ice.Value[] opValueSeq(Ice.Value[] v1, out Ice.Value[] v2, Ice.Current current)
            {
                v2 = v1;
                return v1;
            }

            public Dictionary<string, Ice.Value>
            opValueMap(Dictionary<string, Ice.Value> v1, out Dictionary<string, Ice.Value> v2,
                       Ice.Current current)
            {
                v2 = v1;
                return v1;
            }

            public void setRecursive(Test.Recursive r, Ice.Current current)
            {
            }

            public bool supportsClassGraphDepthMax(Ice.Current current)
            {
                return true;
            }

            public Test.D1 getD1(Test.D1 d1, Ice.Current current)
            {
                return d1;
            }

            public void throwEDerived(Ice.Current current)
            {
                throw new Test.EDerived(new Test.A1("a1"), new Test.A1("a2"), new Test.A1("a3"), new Test.A1("a4"));
            }

            public void setG(Test.G theG, Ice.Current current)
            {
            }
            public Test.Base[] opBaseSeq(Test.Base[] inS, out Test.Base[] outS, Ice.Current current)
            {
                outS = inS;
                return inS;
            }

            public Test.Compact getCompact(Ice.Current current)
            {
                return new Test.CompactExt();
            }

            public void shutdown(Ice.Current current)
            {
                _adapter.Communicator.shutdown();
            }

            public Test.Inner.A
            getInnerA(Ice.Current current)
            {
                return new Test.Inner.A(_b1);
            }

            public Test.Inner.Sub.A
            getInnerSubA(Ice.Current current)
            {
                return new Test.Inner.Sub.A(new Test.Inner.A(_b1));
            }

            public void throwInnerEx(Ice.Current current)
            {
                throw new Test.Inner.Ex("Inner::Ex");
            }

            public void throwInnerSubEx(Ice.Current current)
            {
                throw new Test.Inner.Sub.Ex("Inner::Sub::Ex");
            }

            public Test.Initial_GetMBMarshaledResult getMB(Ice.Current current)
            {
                return new Test.Initial_GetMBMarshaledResult(_b1, current);
            }

            public Task<Test.Initial_GetAMDMBMarshaledResult> getAMDMBAsync(Ice.Current current)
            {
                return Task.FromResult(new Test.Initial_GetAMDMBMarshaledResult(_b1, current));
            }

            public Test.M
            opM(Test.M v1, out Test.M v2, Ice.Current current)
            {
                v2 = v1;
                return v1;
            }

            public Test.F1
            opF1(Test.F1 f11, out Test.F1 f12, Ice.Current current)
            {
                f12 = new Test.F1("F12");
                return f11;
            }

            public Test.F2Prx
            opF2(Test.F2Prx f21, out Test.F2Prx f22, Ice.Current current)
            {
                f22 = Test.F2Prx.Parse("F22", current.Adapter.Communicator);
                return f21;
            }

            public Test.F3
            opF3(Test.F3 f31, out Test.F3 f32, Ice.Current current)
            {
                f32 = new Test.F3(new Test.F1("F12"), Test.F2Prx.Parse("F22", current.Adapter.Communicator));
                return f31;
            }

            public bool
            hasF3(Ice.Current current)
            {
                return true;
            }

            private Ice.ObjectAdapter _adapter;
            private Test.B _b1;
            private Test.B _b2;
            private Test.C _c;
            private Test.D _d;
        }
    }
}
