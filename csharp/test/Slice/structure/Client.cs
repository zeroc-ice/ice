// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Test;
using ZeroC.Ice;

namespace ZeroC.Slice.Test.Structure
{
    // Use reference equality for IntList
    public partial struct S3
    {
        public readonly bool Equals(S3 other) => S == other.S && IntList == other.IntList;
        public readonly override int GetHashCode() => HashCode.Combine(S, IntList);
    }

    public class Client : TestHelper
    {
        private static void Run(Communicator communicator)
        {
            Console.Out.Write("testing Equals() for Slice structures... ");
            Console.Out.Flush();

            // Define some default values.
            var def_cls = new C(5);
            var def_s = new S1("name");
            string[] def_ss = new string[] { "one", "two", "three" };
            int[] def_il = new int[3] { 1, 2, 3 };
            var def_sd = new Dictionary<string, string>
            {
                { "abc", "def" }
            };
            var def_prx = IObjectPrx.Parse("test", communicator);
            _ = new S2(true, 98, 99, 100, 101, 1.0f, 2.0, "string", def_ss, def_il, def_sd, def_s, def_cls, def_prx);

            // Compare default-constructed structures.
            Assert(new S2() == new S2());

            var s3 = new S3("foo", new int[] { 4, 7, 10 });
            var s3Copy = s3;
            Assert(s3 == s3Copy);
            Assert(s3.GetHashCode() == s3Copy.GetHashCode());
            s3Copy.IntList = new int[] { 4, 7, 10 }; // same values, different object
            Assert(s3 != s3Copy);
            Assert(s3.GetHashCode() != s3Copy.GetHashCode()); // can be the same, just unlikely
            Console.Out.WriteLine("ok");
        }

        public override async Task RunAsync(string[] args)
        {
            await using Communicator communicator = Initialize(ref args);
            Run(communicator);
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
