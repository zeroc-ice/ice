// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Collections.Generic;

using Test;

public class Client
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.Exception();
        }
    }

    public static int Main(string[] args)
    {
        int status = 0;
        try
        {
            Console.Out.Write("testing Slice predefined macros... ");
            Console.Out.Flush();
            Default d = new Default();
            test(d.x == 10);
            test(d.y == 10);

            NoDefault nd = new NoDefault();
            test(nd.x != 10);
            test(nd.y != 10);

            CsOnly c = new CsOnly();
            test(c.lang.Equals("cs"));
            test(c.version == Ice.Util.intVersion());
            Console.Out.WriteLine("ok");
        }
        catch(System.Exception ex)
        {
            Console.Error.WriteLine(ex);
            status = 1;
        }
        return status;
    }
}
