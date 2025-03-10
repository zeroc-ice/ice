// Copyright (c) ZeroC, Inc.

namespace Ice.seqMapping
{
        public class AllTests : global::Test.AllTests
        {
            public static async Task<Test.MyClassPrx> allTests(global::Test.TestHelper helper, bool collocated)
            {
                var communicator = helper.communicator();
                var output = helper.getWriter();
                output.Flush();
                string rf = "test:" + helper.getTestEndpoint(0);
                var baseProxy = communicator.stringToProxy(rf);
                Test.MyClassPrx cl = Test.MyClassPrxHelper.checkedCast(baseProxy);

                output.Write("testing twoway operations... ");
                output.Flush();
                Twoways.twoways(communicator, cl);
                output.WriteLine("ok");

                if (!collocated)
                {
                    output.Write("testing twoway operations with AMI... ");
                    output.Flush();
                    await TwowaysAMI.twowaysAMI(communicator, cl);
                    output.WriteLine("ok");
                }
                return cl;
            }
        }
    }

