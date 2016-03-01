// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.checksum;

import java.io.PrintWriter;
import test.Ice.checksum.Test.*;

public class AllTests
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static ChecksumPrx
    allTests(Ice.Communicator communicator, boolean collocated, PrintWriter out)
    {
        String ref = "test:default -p 12010";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);

        ChecksumPrx checksum = ChecksumPrxHelper.checkedCast(base);
        test(checksum != null);

        //
        // Verify that no checksums are present for local types.
        //
        out.print("testing checksums... ");
        out.flush();
        for(java.util.Map.Entry<String, String> p : SliceChecksums.checksums.entrySet())
        {
            String key = p.getKey();
            int pos = key.indexOf("Local");
            test(pos == -1);
        }

        //
        // Get server's Slice checksums.
        //
        java.util.Map<String, String> d = checksum.getSliceChecksums();

        //
        // Compare the checksums. For a type FooN whose name ends in an integer N,
        // we assume that the server's type does not change for N = 1, and does
        // change for N > 1.
        //
        java.util.regex.Pattern patt = java.util.regex.Pattern.compile("\\d+");
        for(java.util.Map.Entry<String, String> p : d.entrySet())
        {
            int n = 0;
            String key = p.getKey();
            java.util.regex.Matcher m = patt.matcher(key);
            if(m.find())
            {
                try
                {
                    n = Integer.parseInt(key.substring(m.start(), m.end()));
                }
                catch(NumberFormatException ex)
                {
                    test(false);
                }
            }

            String value = SliceChecksums.checksums.get(key);
            test(value != null);

            if(n <= 1)
            {
                test(value.equals(p.getValue()));
            }
            else
            {
                test(!value.equals(p.getValue()));
            }
        }
        out.println("ok");

        return checksum;
    }
}
