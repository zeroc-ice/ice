// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Operations
{
    public static class AllTests
    {
        public static async Task RunAsync(TestHelper helper)
        {
            Communicator communicator = helper.Communicator;
            System.IO.TextWriter output = helper.Output;

            var cl = IMyClassPrx.Parse(helper.GetTestProxy("test", 0), communicator);
            var derivedProxy = IMyDerivedClassPrx.Parse(helper.GetTestProxy("test", 0), communicator);

            output.Write("testing twoway operations... ");
            output.Flush();
            Twoways.Run(helper, cl);
            Twoways.Run(helper, derivedProxy);
            derivedProxy.OpDerived();
            output.WriteLine("ok");

            output.Write("testing oneway operations... ");
            output.Flush();
            Oneways.Run(cl);
            output.WriteLine("ok");

            output.Write("testing twoway operations with AMI... ");
            output.Flush();
            TwowaysAMI.Run(helper, cl);
            TwowaysAMI.Run(helper, derivedProxy);
            output.WriteLine("ok");

            output.Write("testing oneway operations with AMI... ");
            output.Flush();
            OnewaysAMI.Run(helper, cl);
            output.WriteLine("ok");

            await cl.ShutdownAsync();
        }
    }
}
