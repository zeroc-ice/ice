// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Optional
{
    internal static class AllTests
    {
        // We need a separate, non-async function here, since Span<T> can't be used in async contexts.
        internal static void RunTest(ITestPrx test, TextWriter output)
        {
            output.Write("testing BitSequence and ReadOnlyBitSequence... ");

            Span<byte> span1 = stackalloc byte[7];
            Span<byte> span2 = stackalloc byte[3];
            var bitSequence = new BitSequence(span1, span2);
            TestHelper.Assert(bitSequence.Length == 80);
            var onBits = new int[] { 0, 9, 35, 69, 70, 71, 79 };

            foreach (int i in onBits)
            {
                bitSequence[i] = true;
            }
            bitSequence[69] = true; // double true

            for (int i = 0; i < bitSequence.Length; ++i)
            {
                TestHelper.Assert(bitSequence[i] == onBits.Contains(i));
                bitSequence[i] = !bitSequence[i];
            }
            for (int i = 0; i < bitSequence.Length; ++i)
            {
                TestHelper.Assert(bitSequence[i] != onBits.Contains(i));
                bitSequence[i] = !bitSequence[i]; // back to original value
            }

            try
            {
                bitSequence[81] = true;
                TestHelper.Assert(false);
            }
            catch (IndexOutOfRangeException)
            {
                // expected
            }

            try
            {
                bitSequence[-5] = true;
                TestHelper.Assert(false);
            }
            catch (IndexOutOfRangeException)
            {
                // expected
            }

            Span<byte> span = stackalloc byte[10];
            span1.CopyTo(span);
            span2.CopyTo(span.Slice(7));
            var roBitSequence = new ReadOnlyBitSequence(span);
            TestHelper.Assert(roBitSequence.Length == 80);
            for (int i = 0; i < roBitSequence.Length; ++i)
            {
                TestHelper.Assert(roBitSequence[i] == onBits.Contains(i));
            }

            try
            {
                bool _ = roBitSequence[80];
                TestHelper.Assert(false);
            }
            catch (IndexOutOfRangeException)
            {
                // expected
            }

            try
            {
                bool _ = roBitSequence[-5];
                TestHelper.Assert(false);
            }
            catch (IndexOutOfRangeException)
            {
                // expected
            }

            output.Flush();
            output.WriteLine("ok");

            output.Write("testing basic operations with optional parameters... ");

            test.OpInt(null);
            test.OpInt(test.OpReturnInt());
            test.OpString(null);
            test.OpString(test.OpReturnString());

            test.OpBasic(17, 17, "test", "test");
            test.OpBasic(17, 17, null, "test");
            test.OpBasic(17, null, null, "test");

            (int? r, int o1, int? o2, string? o3) = test.OpBasicReturnTuple(5, 15, "test");
            TestHelper.Assert(r!.Value == 15 && o1 == 5 && o2!.Value == 15 && o3! == "test");

            (r, o1, o2, o3) = test.OpBasicReturnTuple(6, null, null);
            TestHelper.Assert(r == null && o1 == 6 && o2 == null && o3 == null);
            output.WriteLine("ok");

            output.Write("testing operations with proxies and class parameters... ");
            TestHelper.Assert(test.OpObject(test, test)!.Equals(test));
            TestHelper.Assert(test.OpObject(test, null) == null);
            TestHelper.Assert(test.OpTest(test, test)!.Equals(test));
            TestHelper.Assert(test.OpTest(test, null) == null);

            var classInstance = new C(42);
            AnyClass? anyClass = test.OpAnyClass(classInstance, classInstance);
            TestHelper.Assert(anyClass != null && ((C)anyClass).X == 42);
            TestHelper.Assert(test.OpAnyClass(classInstance, null) == null);
            TestHelper.Assert(test.OpC(classInstance, classInstance)!.X == 42);
            TestHelper.Assert(test.OpC(classInstance, null) == null);

            try
            {
                test.OpObject(null!, null);
                TestHelper.Assert(false);
            }
            catch (NullReferenceException)
            {
            }

            try
            {
                test.OpTest(null!, null);
                TestHelper.Assert(false);
            }
            catch (NullReferenceException)
            {
            }

            // We detect null class instances through asserts during marshaling.

            output.WriteLine("ok");

            output.Write("testing operations with sequence<T?> parameters... ");
            int?[] intSeq = new int?[] { 1, -5, null, 19, -35000 };
            TestHelper.Assert(test.OpOptIntSeq(intSeq).SequenceEqual(intSeq));
            TestHelper.Assert(test.OpTaggedOptIntSeq(intSeq)!.SequenceEqual(intSeq));
            TestHelper.Assert(test.OpTaggedOptIntSeq(null) == null);

            string?[] stringSeq = new string?[] { "foo", "test", null, "", "bar" };
            TestHelper.Assert(test.OpOptStringSeq(stringSeq).SequenceEqual(stringSeq));
            TestHelper.Assert(test.OpTaggedOptStringSeq(stringSeq)!.SequenceEqual(stringSeq));
            TestHelper.Assert(test.OpTaggedOptStringSeq(null) == null);

            output.WriteLine("ok");

            output.Write("testing operations with dictionary<K, V?> parameters... ");
            Dictionary<int, int?> intIntDict = new Dictionary<int, int?> { { 1, -5 }, { 3, null }, { 5, 19 },
                { 7, -35000 }};
            TestHelper.Assert(test.OpIntOptIntDict(intIntDict).DictionaryEqual(intIntDict));
            TestHelper.Assert(test.OpTaggedIntOptIntDict(intIntDict)!.DictionaryEqual(intIntDict));
            TestHelper.Assert(test.OpTaggedIntOptIntDict(null) == null);

            Dictionary<int, string?> intStringDict = new Dictionary<int, string?> { { 1, "foo" }, { 3, "test" },
                { 5, null }, { 7, "bar" }};
            TestHelper.Assert(test.OpIntOptStringDict(intStringDict).DictionaryEqual(intStringDict));
            TestHelper.Assert(test.OpTaggedIntOptStringDict(intStringDict)!.DictionaryEqual(intStringDict));
            TestHelper.Assert(test.OpTaggedIntOptStringDict(null) == null);

            output.WriteLine("ok");

            output.Write("testing struct with optional data members... ");
            var myStruct = new MyStruct(test, null, new string?[] { "foo", null, "bar" });
            MyStruct myStructResult = test.OpMyStruct(myStruct);
            TestHelper.Assert(myStruct == myStructResult);

            myStructResult = test.OpOptMyStruct(myStruct)!.Value;
            TestHelper.Assert(myStruct == myStructResult);

            TestHelper.Assert(test.OpOptMyStruct(null) == null);
            output.WriteLine("ok");

            output.Write("testing class with optional data members... ");
            var derived = new Derived(test, null, new string?[] { "foo", null, "bar" }, null, "test");
            Derived derivedResult = test.OpDerived(derived);
            TestHelper.Assert(derivedResult.Proxy!.Equals(derived.Proxy) &&
                derivedResult.X == derived.X &&
                derivedResult.StringSeq!.SequenceEqual(derived.StringSeq!) &&
                derivedResult.SomeClass == null &&
                derivedResult.S == derived.S);

            derivedResult = test.OpOptDerived(derived)!;
            TestHelper.Assert(derivedResult.Proxy!.Equals(derived.Proxy) &&
                derivedResult.X == derived.X &&
                derivedResult.StringSeq!.SequenceEqual(derived.StringSeq) &&
                derivedResult.SomeClass == null &&
                derivedResult.S == derived.S);

            TestHelper.Assert(test.OpOptDerived(null) == null);
            output.WriteLine("ok");

            output.Write("testing exception with optional data members... ");
            try
            {
                test.OpDerivedEx();
                TestHelper.Assert(false);
            }
            catch (DerivedEx ex)
            {
                TestHelper.Assert(ex.Proxy == null &&
                    ex.X == 5 &&
                    ex.StringSeq!.SequenceEqual(new string?[] { "foo", null, "bar" }) &&
                    ex.SomeClass is C someClass && someClass.X == 42 &&
                    ex.S == "test");
            }

            try
            {
                test.OpDerivedEx(context: new Dictionary<string, string> { { "all null", "yes" } });
                TestHelper.Assert(false);
            }
            catch (DerivedEx ex)
            {
                TestHelper.Assert(ex.Proxy == null &&
                    ex.X == null &&
                    ex.StringSeq == null &&
                    ex.SomeClass == null &&
                    ex.S == null);
            }

            output.WriteLine("ok");
        }

        internal static async Task RunAsync(TestHelper helper)
        {
            Communicator communicator = helper.Communicator;

            TextWriter output = helper.Output;
            var test = ITestPrx.Parse(helper.GetTestProxy("test", 0), communicator);
            RunTest(test, output);

            await test.ShutdownAsync();
        }
    }
}
