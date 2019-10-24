//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace Ice
{
    namespace checksum
    {
        public class AllTests : global::Test.AllTests
        {
            public static Test.ChecksumPrx
            allTests(global::Test.TestHelper helper, bool collocated)
            {
                var output = helper.getWriter();
                Ice.Communicator communicator = helper.communicator();
                string rf = "test:" + helper.getTestEndpoint(0);
                Ice.ObjectPrx baseProxy = communicator.stringToProxy(rf);
                test(baseProxy != null);

                Test.ChecksumPrx checksum = Test.ChecksumPrxHelper.checkedCast(baseProxy);
                test(checksum != null);

                //
                // Verify that no checksums are present for local types.
                //
                output.Write("testing checksums... ");
                output.Flush();
                foreach (KeyValuePair<string, string> entry in Ice.SliceChecksums.checksums)
                {
                    string key = entry.Key;
                    int pos = key.IndexOf("Local");
                    test(pos == -1);
                }

                //
                // Get server's Slice checksums.
                //
                Dictionary<string, string> d = checksum.getSliceChecksums();

                //
                // Compare the checksums. For a type FooN whose name ends in an integer N,
                // we assume that the server's type does not change for N = 1, and does
                // change for N > 1.
                //
                char[] digits = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
                foreach (KeyValuePair<string, string> entry in d)
                {
                    string key = entry.Key;
                    int start = key.IndexOfAny(digits);
                    if (start != -1)
                    {
                        int end = start;
                        while (end < key.Length && char.IsDigit(key[end]))
                        {
                            end++;
                        }
                        int n = int.Parse(key.Substring(start, end - start));

                        string value = Ice.SliceChecksums.checksums[key];
                        test(value != null);

                        if (n <= 1)
                        {
                            test(value.Equals(entry.Value));
                        }
                        else
                        {
                            test(!value.Equals(entry.Value));
                        }
                    }
                }

                output.WriteLine("ok");

                return checksum;
            }
        }
    }
}
