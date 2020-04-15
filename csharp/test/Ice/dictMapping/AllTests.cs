//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;
using System.IO;
using Test;

namespace Ice.dictMapping
{
    public class AllTests
    {
        public static Test.IMyClassPrx allTests(TestHelper helper, bool collocated)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            TextWriter output = helper.GetWriter();

            var cl = Test.IMyClassPrx.Parse($"test:{helper.GetTestEndpoint(0)}", communicator);

            output.Write("testing twoway operations... ");
            output.Flush();
            Twoways.twoways(cl);
            output.WriteLine("ok");

            if (!collocated)
            {
                output.Write("testing twoway operations with AMI... ");
                output.Flush();
                TwowaysAMI.twowaysAMI(cl);
                output.WriteLine("ok");
            }
            return cl;
        }
    }
}
