//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace inheritance
    {
        public class AllTests : global::Test.AllTests
        {
            public static Test.InitialPrx allTests(global::Test.TestHelper helper)
            {
                Ice.Communicator communicator = helper.communicator();
                var output = helper.getWriter();
                output.Write("testing stringToProxy... ");
                output.Flush();
                string ref_Renamed = "initial:" + helper.getTestEndpoint(0);
                Ice.ObjectPrx @base = communicator.stringToProxy(ref_Renamed);
                test(@base != null);
                output.WriteLine("ok");

                output.Write("testing checked cast... ");
                output.Flush();
                var initial = Test.InitialPrxHelper.checkedCast(@base);
                test(initial != null);
                test(initial.Equals(@base));
                output.WriteLine("ok");

                output.Write("getting proxies for interface hierarchy... ");
                output.Flush();
                var ia = initial.iaop();
                var ib1 = initial.ib1op();
                var ib2 = initial.ib2op();
                var ic = initial.icop();
                test(ia != ib1);
                test(ia != ib2);
                test(ia != ic);
                test(ib1 != ic);
                test(ib2 != ic);
                output.WriteLine("ok");

                output.Write("invoking proxy operations on interface hierarchy... ");
                output.Flush();
                Test.MA.IAPrx iao;
                Test.MB.IB1Prx ib1o;
                Test.MB.IB2Prx ib2o;
                Test.MA.ICPrx ico;

                iao = ia.iaop(ia);
                test(iao.Equals(ia));
                iao = ia.iaop(ib1);
                test(iao.Equals(ib1));
                iao = ia.iaop(ib2);
                test(iao.Equals(ib2));
                iao = ia.iaop(ic);
                test(iao.Equals(ic));
                iao = ib1.iaop(ia);
                test(iao.Equals(ia));
                iao = ib1.iaop(ib1);
                test(iao.Equals(ib1));
                iao = ib1.iaop(ib2);
                test(iao.Equals(ib2));
                iao = ib1.iaop(ic);
                test(iao.Equals(ic));
                iao = ib2.iaop(ia);
                test(iao.Equals(ia));
                iao = ib2.iaop(ib1);
                test(iao.Equals(ib1));
                iao = ib2.iaop(ib2);
                test(iao.Equals(ib2));
                iao = ib2.iaop(ic);
                test(iao.Equals(ic));
                iao = ic.iaop(ia);
                test(iao.Equals(ia));
                iao = ic.iaop(ib1);
                test(iao.Equals(ib1));
                iao = ic.iaop(ib2);
                test(iao.Equals(ib2));
                iao = ic.iaop(ic);
                test(iao.Equals(ic));

                iao = ib1.ib1op(ib1);
                test(iao.Equals(ib1));
                ib1o = ib1.ib1op(ib1);
                test(ib1o.Equals(ib1));
                iao = ib1.ib1op(ic);
                test(iao.Equals(ic));
                ib1o = ib1.ib1op(ic);
                test(ib1o.Equals(ic));
                iao = ic.ib1op(ib1);
                test(iao.Equals(ib1));
                ib1o = ic.ib1op(ib1);
                test(ib1o.Equals(ib1));
                iao = ic.ib1op(ic);
                test(iao.Equals(ic));
                ib1o = ic.ib1op(ic);
                test(ib1o.Equals(ic));

                iao = ib2.ib2op(ib2);
                test(iao.Equals(ib2));
                ib2o = ib2.ib2op(ib2);
                test(ib2o.Equals(ib2));
                iao = ib2.ib2op(ic);
                test(iao.Equals(ic));
                ib2o = ib2.ib2op(ic);
                test(ib2o.Equals(ic));
                iao = ic.ib2op(ib2);
                test(iao.Equals(ib2));
                ib2o = ic.ib2op(ib2);
                test(ib2o.Equals(ib2));
                iao = ic.ib2op(ic);
                test(iao.Equals(ic));
                ib2o = ic.ib2op(ic);
                test(ib2o.Equals(ic));

                iao = ic.icop(ic);
                test(iao.Equals(ic));
                ib1o = ic.icop(ic);
                test(ib1o.Equals(ic));
                ib2o = ic.icop(ic);
                test(ib2o.Equals(ic));
                ico = ic.icop(ic);
                test(ico.Equals(ic));
                output.WriteLine("ok");
                return initial;
            }
        }
    }
}
