// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Inheritance
{
    public static class AllTests
    {
        public static async Task RunAsync(TestHelper helper)
        {
            Communicator communicator = helper.Communicator;

            System.IO.TextWriter output = helper.Output;
            var initial = IInitialPrx.Parse(helper.GetTestProxy("initial", 0), communicator);

            output.Write("getting proxies for interface hierarchy... ");
            output.Flush();
            MA.IAPrx? ia = initial.Iaop();
            MB.IB1Prx? ib1 = initial.Ib1op();
            MB.IB2Prx? ib2 = initial.Ib2op();
            MA.ICPrx? ic = initial.Icop();
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
            MA.IAPrx? iao;
            MB.IB1Prx? ib1o;
            MB.IB2Prx? ib2o;
            MA.ICPrx? ico;

            iao = ia.Iaop(ia);
            TestHelper.Assert(iao!.Equals(ia));
            iao = ia.Iaop(ib1);
            TestHelper.Assert(iao!.Equals(ib1));
            iao = ia.Iaop(ib2);
            TestHelper.Assert(iao!.Equals(ib2));
            iao = ia.Iaop(ic);
            TestHelper.Assert(iao!.Equals(ic));
            iao = ib1.Iaop(ia);
            TestHelper.Assert(iao!.Equals(ia));
            iao = ib1.Iaop(ib1);
            TestHelper.Assert(iao!.Equals(ib1));
            iao = ib1.Iaop(ib2);
            TestHelper.Assert(iao!.Equals(ib2));
            iao = ib1.Iaop(ic);
            TestHelper.Assert(iao!.Equals(ic));
            iao = ib2.Iaop(ia);
            TestHelper.Assert(iao!.Equals(ia));
            iao = ib2.Iaop(ib1);
            TestHelper.Assert(iao!.Equals(ib1));
            iao = ib2.Iaop(ib2);
            TestHelper.Assert(iao!.Equals(ib2));
            iao = ib2.Iaop(ic);
            TestHelper.Assert(iao!.Equals(ic));
            iao = ic.Iaop(ia);
            TestHelper.Assert(iao!.Equals(ia));
            iao = ic.Iaop(ib1);
            TestHelper.Assert(iao!.Equals(ib1));
            iao = ic.Iaop(ib2);
            TestHelper.Assert(iao!.Equals(ib2));
            iao = ic.Iaop(ic);
            TestHelper.Assert(iao!.Equals(ic));

            iao = ib1.Ib1op(ib1);
            TestHelper.Assert(iao!.Equals(ib1));
            ib1o = ib1.Ib1op(ib1);
            TestHelper.Assert(ib1o!.Equals(ib1));
            iao = ib1.Ib1op(ic);
            TestHelper.Assert(iao!.Equals(ic));
            ib1o = ib1.Ib1op(ic);
            TestHelper.Assert(ib1o!.Equals(ic));
            iao = ic.Ib1op(ib1);
            TestHelper.Assert(iao!.Equals(ib1));
            ib1o = ic.Ib1op(ib1);
            TestHelper.Assert(ib1o!.Equals(ib1));
            iao = ic.Ib1op(ic);
            TestHelper.Assert(iao!.Equals(ic));
            ib1o = ic.Ib1op(ic);
            TestHelper.Assert(ib1o!.Equals(ic));

            iao = ib2.Ib2op(ib2);
            TestHelper.Assert(iao!.Equals(ib2));
            ib2o = ib2.Ib2op(ib2);
            TestHelper.Assert(ib2o!.Equals(ib2));
            iao = ib2.Ib2op(ic);
            TestHelper.Assert(iao!.Equals(ic));
            ib2o = ib2.Ib2op(ic);
            TestHelper.Assert(ib2o!.Equals(ic));
            iao = ic.Ib2op(ib2);
            TestHelper.Assert(iao!.Equals(ib2));
            ib2o = ic.Ib2op(ib2);
            TestHelper.Assert(ib2o!.Equals(ib2));
            iao = ic.Ib2op(ic);
            TestHelper.Assert(iao!.Equals(ic));
            ib2o = ic.Ib2op(ic);
            TestHelper.Assert(ib2o!.Equals(ic));

            iao = ic.Icop(ic);
            TestHelper.Assert(iao!.Equals(ic));
            ib1o = ic.Icop(ic);
            TestHelper.Assert(ib1o!.Equals(ic));
            ib2o = ic.Icop(ic);
            TestHelper.Assert(ib2o!.Equals(ic));
            ico = ic.Icop(ic);
            TestHelper.Assert(ico!.Equals(ic));
            output.WriteLine("ok");
            await initial.ShutdownAsync();
        }
    }
}
