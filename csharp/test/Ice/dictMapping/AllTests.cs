//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace dictMapping
    {
        public class AllTests : global::Test.AllTests
        {
            public static Test.IMyClassPrx allTests(global::Test.TestHelper helper, bool collocated)
            {
                var communicator = helper.communicator();
                var output = helper.getWriter();

                var cl = Test.IMyClassPrx.Parse($"test:{helper.getTestEndpoint(0)}", communicator);

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
}
