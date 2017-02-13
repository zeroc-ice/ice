// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Slice.macros;

import test.Slice.macros.Test.Default;
import test.Slice.macros.Test.NoDefault;
import test.Slice.macros.Test.JavaOnly;

public class Client
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        try
        {
            System.out.print("testing Slice predefined macros... ");
            Default d = new Default();
            test(d.x == 10);
            test(d.y == 10);

            NoDefault nd = new NoDefault();
            test(nd.x != 10);
            test(nd.y != 10);

            JavaOnly c = new JavaOnly();
            test(c.lang.equals("java"));
            test(c.version == Ice.Util.intVersion());
            System.out.println("ok");
        }
        catch(Exception ex)
        {
            ex.printStackTrace();
            status = 1;
        }
        System.gc();
        System.exit(status);
    }
}
