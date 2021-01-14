// Copyright (c) ZeroC, Inc. All rights reserved.

using System.IO;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.DictMapping
{
    public static class AllTests
    {
        public static async Task RunAsync(TestHelper helper, bool collocated)
        {
            Communicator communicator = helper.Communicator;

            TextWriter output = helper.Output;

            var cl = IMyClassPrx.Parse(helper.GetTestProxy("test", 0), communicator);

            output.Write("testing twoway operations... ");
            output.Flush();
            await Twoways.RunAsync(cl);
            output.WriteLine("ok");

            if (!collocated)
            {
                output.Write("testing twoway operations with AMI... ");
                output.Flush();
                await TwowaysAMI.RunAsync(cl);
                output.WriteLine("ok");
            }

            output.Write("shutting down server... ");
            output.Flush();
            await cl.ShutdownAsync();
            output.WriteLine("ok");
        }
    }
}
