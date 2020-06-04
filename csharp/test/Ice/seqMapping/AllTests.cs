//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.Test.SeqMapping
{
    public class AllTests
    {
        public static IMyClassPrx allTests(TestHelper helper, bool collocated)
        {
            var communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            var output = helper.GetWriter();
            var cl = IMyClassPrx.Parse($"test:{helper.GetTestEndpoint(0)}", communicator);
            output.Write("testing twoway operations... ");
            output.Flush();
            Twoways.twoways(communicator, cl);
            output.WriteLine("ok");

            if (!collocated)
            {
                output.Write("testing twoway operations with AMI... ");
                output.Flush();
                TwowaysAMI.twowaysAMI(communicator, cl);
                output.WriteLine("ok");
            }
            return cl;
        }
    }
}
