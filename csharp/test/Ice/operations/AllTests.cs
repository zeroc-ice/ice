//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.Test.Operations
{
    public class AllTests
    {
        public static IMyClassPrx allTests(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            System.IO.TextWriter output = helper.GetWriter();

            var cl = IMyClassPrx.Parse($"test:{helper.GetTestEndpoint(0)}", communicator);
            var derivedProxy = IMyDerivedClassPrx.Parse($"test:{helper.GetTestEndpoint(0)}", communicator);

            output.Write("testing twoway operations... ");
            output.Flush();
            Twoways.twoways(helper, cl);
            Twoways.twoways(helper, derivedProxy);
            derivedProxy.OpDerived();
            output.WriteLine("ok");

            output.Write("testing oneway operations... ");
            output.Flush();
            Oneways.oneways(cl);
            output.WriteLine("ok");

            output.Write("testing twoway operations with AMI... ");
            output.Flush();
            TwowaysAMI.twowaysAMI(helper, cl);
            TwowaysAMI.twowaysAMI(helper, derivedProxy);
            output.WriteLine("ok");

            output.Write("testing oneway operations with AMI... ");
            output.Flush();
            OnewaysAMI.Run(helper, cl);
            output.WriteLine("ok");

            return cl;
        }
    }
}
