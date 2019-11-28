//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace operations
    {
        public class AllTests : global::Test.AllTests
        {
            public static Test.MyClassPrx allTests(global::Test.TestHelper helper)
            {
                Ice.Communicator communicator = helper.communicator();
                var output = helper.getWriter();

                var cl = Test.MyClassPrx.Parse($"test:{helper.getTestEndpoint(0)}", communicator);
                var derivedProxy = Test.MyDerivedClassPrx.CheckedCast(cl);

                output.Write("testing twoway operations... ");
                output.Flush();
                Twoways.twoways(helper, cl);
                Twoways.twoways(helper, derivedProxy);
                derivedProxy.opDerived();
                output.WriteLine("ok");

                output.Write("testing oneway operations... ");
                output.Flush();
                Oneways.oneways(helper, cl);
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
}
