//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.Test.Inheritance
{
    public class AllTests
    {
        public static IInitialPrx allTests(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            System.IO.TextWriter output = helper.GetWriter();
            var initial = IInitialPrx.Parse($"initial:{helper.GetTestEndpoint(0)}", communicator);

            output.Write("getting proxies for interface hierarchy... ");
            output.Flush();
            MA.IIAPrx? ia = initial.iaop();
            MB.IIB1Prx? ib1 = initial.ib1op();
            MB.IIB2Prx? ib2 = initial.ib2op();
            MA.IICPrx? ic = initial.icop();
            TestHelper.Assert(ia != null);
            TestHelper.Assert(ib1 != null);
            TestHelper.Assert(ib2 != null);
            TestHelper.Assert(ic != null);
            TestHelper.Assert(ia != ib1);
            TestHelper.Assert(ia != ib2);
            TestHelper.Assert(ia != ic);
            TestHelper.Assert(ib1 != ic);
            TestHelper.Assert(ib2 != ic);
            output.WriteLine("ok");

            output.Write("invoking proxy operations on interface hierarchy... ");
            output.Flush();
            MA.IIAPrx? iao;
            MB.IIB1Prx? ib1o;
            MB.IIB2Prx? ib2o;
            MA.IICPrx? ico;

            iao = ia.iaop(ia);
            TestHelper.Assert(iao!.Equals(ia));
            iao = ia.iaop(ib1);
            TestHelper.Assert(iao!.Equals(ib1));
            iao = ia.iaop(ib2);
            TestHelper.Assert(iao!.Equals(ib2));
            iao = ia.iaop(ic);
            TestHelper.Assert(iao!.Equals(ic));
            iao = ib1.iaop(ia);
            TestHelper.Assert(iao!.Equals(ia));
            iao = ib1.iaop(ib1);
            TestHelper.Assert(iao!.Equals(ib1));
            iao = ib1.iaop(ib2);
            TestHelper.Assert(iao!.Equals(ib2));
            iao = ib1.iaop(ic);
            TestHelper.Assert(iao!.Equals(ic));
            iao = ib2.iaop(ia);
            TestHelper.Assert(iao!.Equals(ia));
            iao = ib2.iaop(ib1);
            TestHelper.Assert(iao!.Equals(ib1));
            iao = ib2.iaop(ib2);
            TestHelper.Assert(iao!.Equals(ib2));
            iao = ib2.iaop(ic);
            TestHelper.Assert(iao!.Equals(ic));
            iao = ic.iaop(ia);
            TestHelper.Assert(iao!.Equals(ia));
            iao = ic.iaop(ib1);
            TestHelper.Assert(iao!.Equals(ib1));
            iao = ic.iaop(ib2);
            TestHelper.Assert(iao!.Equals(ib2));
            iao = ic.iaop(ic);
            TestHelper.Assert(iao!.Equals(ic));

            iao = ib1.ib1op(ib1);
            TestHelper.Assert(iao!.Equals(ib1));
            ib1o = ib1.ib1op(ib1);
            TestHelper.Assert(ib1o!.Equals(ib1));
            iao = ib1.ib1op(ic);
            TestHelper.Assert(iao!.Equals(ic));
            ib1o = ib1.ib1op(ic);
            TestHelper.Assert(ib1o!.Equals(ic));
            iao = ic.ib1op(ib1);
            TestHelper.Assert(iao!.Equals(ib1));
            ib1o = ic.ib1op(ib1);
            TestHelper.Assert(ib1o!.Equals(ib1));
            iao = ic.ib1op(ic);
            TestHelper.Assert(iao!.Equals(ic));
            ib1o = ic.ib1op(ic);
            TestHelper.Assert(ib1o!.Equals(ic));

            iao = ib2.ib2op(ib2);
            TestHelper.Assert(iao!.Equals(ib2));
            ib2o = ib2.ib2op(ib2);
            TestHelper.Assert(ib2o!.Equals(ib2));
            iao = ib2.ib2op(ic);
            TestHelper.Assert(iao!.Equals(ic));
            ib2o = ib2.ib2op(ic);
            TestHelper.Assert(ib2o!.Equals(ic));
            iao = ic.ib2op(ib2);
            TestHelper.Assert(iao!.Equals(ib2));
            ib2o = ic.ib2op(ib2);
            TestHelper.Assert(ib2o!.Equals(ib2));
            iao = ic.ib2op(ic);
            TestHelper.Assert(iao!.Equals(ic));
            ib2o = ic.ib2op(ic);
            TestHelper.Assert(ib2o!.Equals(ic));

            iao = ic.icop(ic);
            TestHelper.Assert(iao!.Equals(ic));
            ib1o = ic.icop(ic);
            TestHelper.Assert(ib1o!.Equals(ic));
            ib2o = ic.icop(ic);
            TestHelper.Assert(ib2o!.Equals(ic));
            ico = ic.icop(ic);
            TestHelper.Assert(ico!.Equals(ic));
            output.WriteLine("ok");
            return initial;
        }
    }
}
