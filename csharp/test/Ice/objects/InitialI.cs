// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Threading.Tasks;
using System.Collections.Generic;

namespace Ice
{
    namespace objects
    {
        public sealed class InitialI : Test.InitialDisp_
        {
            public InitialI(Ice.ObjectAdapter adapter)
            {
                _adapter = adapter;
                _b1 = new BI();
                _b2 = new BI();
                _c = new CI();
                _d = new DI();
                _e = new EI();
                _f = new FI(_e);

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

            public override void getAll(out Test.B b1, out Test.B b2, out Test.C c, out Test.D d, Ice.Current current)
            {
                b1 = _b1;
                b2 = _b2;
                c = _c;
                d = _d;
            }

            public override Test.B getB1(Ice.Current current)
            {
                return _b1;
            }

            public override Test.B getB2(Ice.Current current)
            {
                return _b2;
            }

            public override Test.C getC(Ice.Current current)
            {
                return _c;
            }

            public override Test.D getD(Ice.Current current)
            {
                return _d;
            }

            public override Test.E getE(Ice.Current current)
            {
                return _e;
            }

            public override Test.F getF(Ice.Current current)
            {
                return _f;
            }

            public override Ice.Value getI(Ice.Current current)
            {
                return new II();
            }

            public override Ice.Value getJ(Ice.Current current)
            {
                return new JI();
            }

            public override Ice.Value getH(Ice.Current current)
            {
                return new HI();
            }

            public override Test.K getK(Ice.Current current)
            {
                return new Test.K(new Test.L("l"));
            }

            public override Ice.Value opValue(Ice.Value v1, out Ice.Value v2, Ice.Current current)
            {
                v2 = v1;
                return v1;
            }

            public override Ice.Value[] opValueSeq(Ice.Value[] v1, out Ice.Value[] v2, Ice.Current current)
            {
                v2 = v1;
                return v1;
            }

            public override Dictionary<string, Ice.Value>
            opValueMap(Dictionary<string, Ice.Value> v1, out Dictionary<string, Ice.Value> v2,
                       Ice.Current current)
            {
                v2 = v1;
                return v1;
            }

            public override void setRecursive(Test.Recursive r, Ice.Current current)
            {
            }

            public override bool supportsClassGraphDepthMax(Ice.Current current)
            {
                return true;
            }

            public override Test.D1 getD1(Test.D1 d1, Ice.Current current)
            {
                return d1;
            }

            public override void throwEDerived(Ice.Current current)
            {
                throw new Test.EDerived(new Test.A1("a1"), new Test.A1("a2"), new Test.A1("a3"), new Test.A1("a4"));
            }

            public override void setG(Test.G theG, Ice.Current current)
            {
            }

            public override void setI(Ice.Value theI, Ice.Current current)
            {
            }

            public override Test.Base[] opBaseSeq(Test.Base[] inS, out Test.Base[] outS, Ice.Current current)
            {
                outS = inS;
                return inS;
            }

            public override Test.Compact getCompact(Ice.Current current)
            {
                return new Test.CompactExt();
            }

            public override void shutdown(Ice.Current current = null)
            {
                _adapter.getCommunicator().shutdown();
            }

            public override Test.Inner.A
            getInnerA(Ice.Current current)
            {
                return new Test.Inner.A(_b1);
            }

            public override Test.Inner.Sub.A
            getInnerSubA(Ice.Current current)
            {
                return new Test.Inner.Sub.A(new Test.Inner.A(_b1));
            }

            public override void throwInnerEx(Ice.Current current)
            {
                throw new Test.Inner.Ex("Inner::Ex");
            }

            public override void throwInnerSubEx(Ice.Current current)
            {
                throw new Test.Inner.Sub.Ex("Inner::Sub::Ex");
            }

            public override Test.Initial_GetMBMarshaledResult getMB(Ice.Current current)
            {
                return new Test.Initial_GetMBMarshaledResult(_b1, current);
            }

            public override Task<Test.Initial_GetAMDMBMarshaledResult> getAMDMBAsync(Ice.Current current)
            {
                return Task.FromResult(new Test.Initial_GetAMDMBMarshaledResult(_b1, current));
            }

            public override Test.M
            opM(Test.M v1, out Test.M v2, Ice.Current current)
            {
                v2 = v1;
                return v1;
            }

            private Ice.ObjectAdapter _adapter;
            private Test.B _b1;
            private Test.B _b2;
            private Test.C _c;
            private Test.D _d;
            private Test.E _e;
            private Test.F _f;
        }
    }
}
