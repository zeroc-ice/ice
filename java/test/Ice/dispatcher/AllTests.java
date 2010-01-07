// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.dispatcher;

import java.io.PrintWriter;

import test.Ice.dispatcher.Test.TestIntfPrx;
import test.Ice.dispatcher.Test.TestIntfPrxHelper;
import test.Ice.dispatcher.Test.TestIntfControllerPrx;
import test.Ice.dispatcher.Test.TestIntfControllerPrxHelper;
import test.Ice.dispatcher.Test.Callback_TestIntf_op;
import test.Ice.dispatcher.Test.Callback_TestIntf_opWithPayload;

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

    public static void
    allTests(Ice.Communicator communicator, PrintWriter out)
    {
        String sref = "test:default -p 12010";
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        TestIntfPrx p = TestIntfPrxHelper.uncheckedCast(obj);

        sref = "testController:tcp -p 12011";
        obj = communicator.stringToProxy(sref);
        test(obj != null);

        TestIntfControllerPrx testController = TestIntfControllerPrxHelper.uncheckedCast(obj);

        out.print("testing dispatcher... ");
        out.flush();
        {
            p.op();

            p.begin_op(new Callback_TestIntf_op()
                {
                    public void
                    response()
                    {
                        test(Dispatcher.isDispatcherThread());
                    }
                    
                    public void 
                    exception(Ice.LocalException ex)
                    {
                        ex.printStackTrace();
                        test(false);
                    }
                });

            TestIntfPrx i = (TestIntfPrx)p.ice_adapterId("dummy");
            i.begin_op(new Callback_TestIntf_op()
                {
                    public void
                    response()
                    {
                        test(false);
                    }
                    
                    public void 
                    exception(Ice.LocalException ex)
                    {
                        test(ex instanceof Ice.NoEndpointException);
                        test(Dispatcher.isDispatcherThread());
                    }
                });

            testController.holdAdapter();
            Callback_TestIntf_opWithPayload callback = new Callback_TestIntf_opWithPayload()
                {
                    public void
                    response()
                    {
                        test(Dispatcher.isDispatcherThread());
                    }
                    
                    public void 
                    exception(Ice.LocalException ex)
                    {
                        test(false);
                    }

                    public void 
                    sent(boolean sentSynchronously)
                    {
                        test(sentSynchronously || Dispatcher.isDispatcherThread());
                    }
                };

            byte[] seq = new byte[10 * 1024];
            new java.util.Random().nextBytes(seq); // Make sure the request doesn't compress too well.
            while(p.begin_opWithPayload(seq, callback).sentSynchronously());
            testController.resumeAdapter();
        }
        out.println("ok");

        p.shutdown();        
    }
}
