// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Demo;

public class Client
{
    public static void Main(string[] args)
    {
        int status = 0;
        Ice.Communicator ic = null;
        try {
            ic = Ice.Util.initialize(ref args);
            Ice.ObjectPrx obj = ic.stringToProxy(
                    "SimplePrinter:default -p 10000");
            PrinterPrx printer = PrinterPrxHelper.checkedCast(obj);
            if (printer == null)
                throw new ApplicationException("Invalid proxy");

            printer.printString("Hello World!");
        } catch (Exception e) {
            Console.Error.WriteLine(e);
            status = 1;
        }
        if (ic != null) {
            // Clean up
            //
            try {
                ic.destroy();
            } catch (Exception e) {
                Console.Error.WriteLine(e);
                status = 1;
            }
        }
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}
