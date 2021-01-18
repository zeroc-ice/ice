// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Linq;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Compress
{
    public static class AllTests
    {
        public static async Task RunAsync(Communicator communicator, bool shutdown)
        {
            var prx1 =
                ITestIntfPrx.Parse(TestHelper.GetTestProxy("test-1", communicator.GetProperties()), communicator);
            var prx2 =
                ITestIntfPrx.Parse(TestHelper.GetTestProxy("test-2", communicator.GetProperties()), communicator);

            for (int size = 1024; size <= 4096; size *= 2)
            {
                byte[] p1 = Enumerable.Range(0, size).Select(i => (byte)i).ToArray();
                prx1.OpCompressArgs(size, p1);

                byte[] p2 = prx1.OpCompressArgsAndReturn(p1);
                TestHelper.Assert(p1.SequenceEqual(p2));

                p2 = prx1.OpCompressReturn(size);
                TestHelper.Assert(p1.SequenceEqual(p2));

                try
                {
                    prx1.OpWithUserException(size);
                    TestHelper.Assert(false);
                }
                catch (MyException ex)
                {
                    TestHelper.Assert(ex.Bytes.SequenceEqual(p1));
                }
            }

            for (int size = 2; size < 1024; size *= 2)
            {
                byte[] p1 = Enumerable.Range(0, size).Select(i => (byte)i).ToArray();
                prx2.OpCompressArgs(size, p1);

                byte[] p2 = prx2.OpCompressArgsAndReturn(p1);
                TestHelper.Assert(p1.SequenceEqual(p2));

                p2 = prx2.OpCompressReturn(size);
                TestHelper.Assert(p1.SequenceEqual(p2));

                try
                {
                    prx1.OpWithUserException(size);
                    TestHelper.Assert(false);
                }
                catch (MyException ex)
                {
                    TestHelper.Assert(ex.Bytes.SequenceEqual(p1));
                }
            }

            if (shutdown)
            {
                await prx1.ShutdownAsync();
            }
        }
    }
}
