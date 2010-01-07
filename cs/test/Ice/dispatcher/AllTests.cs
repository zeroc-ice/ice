// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using Test;

public class AllTests
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.Exception();
        }
    }

    public static void allTests(Ice.Communicator communicator)
    {
        string sref = "test:default -p 12010";
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        Test.TestIntfPrx p = Test.TestIntfPrxHelper.uncheckedCast(obj);

        sref = "testController:tcp -p 12011";
        obj = communicator.stringToProxy(sref);
        test(obj != null);

        Test.TestIntfControllerPrx testController = Test.TestIntfControllerPrxHelper.uncheckedCast(obj);

        Console.Out.Write("testing dispatcher... ");
        Console.Out.Flush();
        {
            p.op();

            p.begin_op().whenCompleted(delegate() { test(Dispatcher.isDispatcherThread()); }, null);

            TestIntfPrx i = (TestIntfPrx)p.ice_adapterId("dummy");
            i.begin_op().whenCompleted(delegate(Ice.Exception ex) 
                                       {
                                           test(ex is Ice.NoEndpointException);
                                           test(Dispatcher.isDispatcherThread());
                                       });

            testController.holdAdapter();
            Test.Callback_TestIntf_opWithPayload rcb = delegate() { test(Dispatcher.isDispatcherThread()); };
            Ice.SentCallback scb = delegate(bool sentSynchronously) 
                {
                    test(sentSynchronously || Dispatcher.isDispatcherThread());
                };

            byte[] seq = new byte[10 * 1024];
            (new System.Random()).NextBytes(seq);
            while(p.begin_opWithPayload(seq).whenCompleted(rcb, null).whenSent(scb).sentSynchronously());
            testController.resumeAdapter();
        }
        Console.Out.WriteLine("ok");

        p.shutdown();        
    }
}
