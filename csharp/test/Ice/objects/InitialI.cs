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
            public InitialI(ObjectAdapter adapter)
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

            public void getAll(out Test.B b1, out Test.B b2, out Test.C c, out Test.D d, Current current)
            {
                b1 = _b1;
                b2 = _b2;
                c = _c;
                d = _d;
            }

            public Test.B getB1(Current current)
            {
                return _b1;
            }

            public Test.B getB2(Current current)
            {
                return _b2;
            }

            public Test.C getC(Current current)
            {
                return _c;
            }

            public Test.D getD(Current current)
            {
                return _d;
            }

            public Test.K getK(Current current)
            {
                return new Test.K(new Test.L("l"));
            }

            public Value opValue(Value v1, out Value v2, Current current)
            {
                v2 = v1;
                return v1;
            }

            public Value[] opValueSeq(Value[] v1, out Value[] v2, Current current)
            {
                v2 = v1;
                return v1;
            }

            public Dictionary<string, Value>
            opValueMap(Dictionary<string, Value> v1, out Dictionary<string, Value> v2,
                       Current current)
            {
                v2 = v1;
                return v1;
            }

            public void setRecursive(Test.Recursive r, Current current)
            {
            }

            public bool supportsClassGraphDepthMax(Current current)
            {
                return true;
            }

            public Test.D1 getD1(Test.D1 d1, Current current)
            {
                return d1;
            }

            public void throwEDerived(Current current)
            {
                throw new Test.EDerived(new Test.A1("a1"), new Test.A1("a2"), new Test.A1("a3"), new Test.A1("a4"));
            }

            public void setG(Test.G theG, Current current)
            {
            }
            public Test.Base[] opBaseSeq(Test.Base[] inS, out Test.Base[] outS, Current current)
            {
                outS = inS;
                return inS;
            }

            public Test.Compact getCompact(Current current)
            {
                return new Test.CompactExt();
            }

            public void shutdown(Current current)
            {
                _adapter.Communicator.shutdown();
            }

            public Test.Inner.A
            getInnerA(Current current)
            {
                return new Test.Inner.A(_b1);
            }

            public Test.Inner.Sub.A
            getInnerSubA(Current current)
            {
                return new Test.Inner.Sub.A(new Test.Inner.A(_b1));
            }

            public void throwInnerEx(Current current)
            {
                throw new Test.Inner.Ex("Inner::Ex");
            }

            public void throwInnerSubEx(Current current)
            {
                throw new Test.Inner.Sub.Ex("Inner::Sub::Ex");
            }

            public Test.Initial.getMBMarshaledResult getMB(Current current)
            {
                return new Test.Initial.getMBMarshaledResult(_b1, current);
            }

            public Task<Test.Initial.getAMDMBMarshaledResult> getAMDMBAsync(Current current)
            {
                return Task.FromResult(new Test.Initial.getAMDMBMarshaledResult(_b1, current));
            }

            public Test.M
            opM(Test.M v1, out Test.M v2, Current current)
            {
                v2 = v1;
                return v1;
            }

            public Test.F1
            opF1(Test.F1 f11, out Test.F1 f12, Current current)
            {
                f12 = new Test.F1("F12");
                return f11;
            }

            public Test.F2Prx
            opF2(Test.F2Prx f21, out Test.F2Prx f22, Current current)
            {
                f22 = Test.F2Prx.Parse("F22", current.Adapter.Communicator);
                return f21;
            }

            public Test.F3
            opF3(Test.F3 f31, out Test.F3 f32, Current current)
            {
                f32 = new Test.F3(new Test.F1("F12"), Test.F2Prx.Parse("F22", current.Adapter.Communicator));
                return f31;
            }

            public bool
            hasF3(Current current)
            {
                return true;
            }

            private ObjectAdapter _adapter;
            private Test.B _b1;
            private Test.B _b2;
            private Test.C _c;
            private Test.D _d;
        }
    }
}
