// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;

public class AllTests : TestCommon.AllTests
{
    public static Test.ChecksumPrx
    allTests(TestCommon.Application app, bool collocated)
    {
        Ice.Communicator communicator = app.communicator();
        string rf = "test:" + app.getTestEndpoint(0);
        Ice.ObjectPrx baseProxy = communicator.stringToProxy(rf);
        test(baseProxy != null);

        Test.ChecksumPrx checksum = Test.ChecksumPrxHelper.checkedCast(baseProxy);
        test(checksum != null);

        //
        // Verify that no checksums are present for local types.
        //
        Console.Out.Write("testing checksums... ");
        Console.Out.Flush();
        foreach(KeyValuePair<string, string> entry in Ice.SliceChecksums.checksums)
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
        char[] digits = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
        foreach(KeyValuePair<string, string> entry in d)
        {
            string key = entry.Key;
            int start = key.IndexOfAny(digits);
            if(start != -1)
            {
                int end = start;
                while(end < key.Length && Char.IsDigit(key[end]))
                {
                    end++;
                }
                int n = Int32.Parse(key.Substring(start, end - start));

                string value = (string)Ice.SliceChecksums.checksums[key];
                test(value != null);

                if(n <= 1)
                {
                    test(value.Equals(entry.Value));
                }
                else
                {
                    test(!value.Equals(entry.Value));
                }
            }
        }

        Console.Out.WriteLine("ok");

        return checksum;
    }
}
