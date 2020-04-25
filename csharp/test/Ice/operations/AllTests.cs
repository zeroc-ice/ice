//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice.operations
{
    public class AllTests
    {
        public static Test.IMyClassPrx allTests(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            System.IO.TextWriter output = helper.GetWriter();

            var cl = Test.IMyClassPrx.Parse($"test:{helper.GetTestEndpoint(0)}", communicator);
            var derivedProxy = Test.IMyDerivedClassPrx.Parse($"test:{helper.GetTestEndpoint(0)}", communicator);

            output.Write("testing twoway operations... ");
            output.Flush();
            Twoways.twoways(helper, cl);
            Twoways.twoways(helper, derivedProxy);
            derivedProxy.opDerived();
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
            OnewaysAMI.onewaysAMI(helper, cl);
            output.WriteLine("ok");

            return cl;
        }
    }
}
