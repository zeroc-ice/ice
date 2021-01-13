// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Perf
{
    public static class AllTests
    {
        public static void RunTest(System.IO.TextWriter output, int repetitions, string name, Action invocation,
            Action warmUpInvocation)
        {
            output.Write($"testing {name}... ");
            output.Flush();
            for (int i = 0; i < 1000; i++)
            {
                warmUpInvocation();
            }

            var watch = new Stopwatch();
            var collections = new int[GC.MaxGeneration].Select((v, i) => GC.CollectionCount(i)).ToArray();
            watch.Start();
            for (int i = 0; i < repetitions; i++)
            {
                invocation();
            }
            collections = collections.Select((v, i) => GC.CollectionCount(i) - v).ToArray();
            GC.Collect();
            watch.Stop();
            output.WriteLine($"{watch.ElapsedMilliseconds / (float)repetitions}ms, " +
                $"{string.Join("/", collections)} ({string.Join("/", collections.Select((_, i) => $"gen{i}"))}) GCs");
        }

        public static void RunTest(System.IO.TextWriter output, int repetitions, string name, Action invocation)
        {
            RunTest(output, repetitions, name, invocation, invocation);
        }

        public static void RunTest<T>(System.IO.TextWriter output, int repetitions, string name,
            Action<ReadOnlyMemory<T>> invocation, int size) where T : struct
        {
            var seq = new T[size];
            T[] emptySeq = Array.Empty<T>();
            RunTest(output, repetitions, name, () => invocation(seq), () => invocation(emptySeq));
        }

        public static void RunTest<T>(System.IO.TextWriter output, int repetitions, string name,
            Func<int, IEnumerable<T>> invocation, int size) where T : struct
        {
            RunTest(output, repetitions, name, () => invocation(size), () => invocation(0));
        }

        public static async Task RunAsync(TestHelper helper)
        {
            Communicator communicator = helper.Communicator!;

            System.IO.TextWriter output = helper.Output;

#if DEBUG
            output.WriteLine("warning: performance test built with DEBUG");
#endif

            var perf = IPerformancePrx.Parse(helper.GetTestProxy("perf", 0), communicator);

            RunTest(output, 10000, "latency", () => perf.IcePing());
            RunTest<byte>(output, 1000, "sending byte sequence", v => perf.SendBytes(v), Constants.ByteSeqSize);
            RunTest<byte>(output, 1000, "received byte sequence", sz => perf.ReceiveBytes(sz), Constants.ByteSeqSize);

            await perf.ShutdownAsync();
        }
    }
}
