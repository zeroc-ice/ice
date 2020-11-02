// Copyright (c) ZeroC, Inc. All rights reserved.

using Test;
using System.IO;

namespace ZeroC.Ice.Test.SeqMapping
{
    public static class AllTests
    {
        public static IMyClassPrx Run(TestHelper helper, bool collocated)
        {
            Communicator? communicator = helper.Communicator;
            TestHelper.Assert(communicator != null);
            TextWriter output = helper.Output;
            var cl = IMyClassPrx.Parse(helper.GetTestProxy("test", 0), communicator);
            output.Write("testing twoway operations... ");
            output.Flush();
            Twoways.Run(communicator, cl);
            output.WriteLine("ok");

            if (!collocated)
            {
                output.Write("testing twoway operations with AMI... ");
                output.Flush();
                TwowaysAMI.Run(communicator, cl);
                output.WriteLine("ok");
            }
            return cl;
        }
    }
}
