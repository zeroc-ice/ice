// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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

    public static Test.ChecksumPrx
    allTests(Ice.Communicator communicator, boolean collocated)
    {
        String ref = "test:default -p 12010 -t 10000";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);

        Test.ChecksumPrx checksum = Test.ChecksumPrxHelper.checkedCast(base);
        test(checksum != null);

        //
        // Verify that no checksums are present for local types.
        //
        System.out.print("testing checksums... ");
        System.out.flush();
        java.util.Iterator p = SliceChecksums.checksums.entrySet().iterator();
        while(p.hasNext())
        {
            java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
            String key = (String)entry.getKey();
            int pos = key.indexOf("Local");
            test(pos == -1);
        }

        //
        // Get server's Slice checksums.
        //
        java.util.Map d = checksum.getSliceChecksums();

        //
        // Compare the checksums. For a type FooN whose name ends in an integer N,
        // we assume that the server's type does not change for N = 1, and does
        // change for N > 1.
        //
        p = d.entrySet().iterator();
        java.util.regex.Pattern patt = java.util.regex.Pattern.compile("\\d+");
        while(p.hasNext())
        {
            int n = 0;
            java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
            String key = (String)entry.getKey();
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

            String value = (String)SliceChecksums.checksums.get(key);
            test(value != null);

            if(n <= 1)
            {
                test(value.equals(entry.getValue()));
            }
            else
            {
                test(!value.equals(entry.getValue()));
            }
        }
        System.out.println("ok");

        return checksum;
    }
}
