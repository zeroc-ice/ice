// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.invoke;

import java.io.PrintWriter;
import java.util.concurrent.CompletableFuture;

import com.zeroc.Ice.InputStream;
import com.zeroc.Ice.OperationMode;
import com.zeroc.Ice.OutputStream;

import test.Ice.invoke.Test.MyClassPrx;
import test.Ice.invoke.Test.MyException;

public class AllTests
{
    final static String testString = "This is a test string";

    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static MyClassPrx allTests(com.zeroc.Ice.Communicator communicator, PrintWriter out)
    {
        String ref = "test:default -p 12010";
        com.zeroc.Ice.ObjectPrx base = communicator.stringToProxy(ref);
        MyClassPrx cl = MyClassPrx.checkedCast(base);
        MyClassPrx oneway = cl.ice_oneway();
        MyClassPrx batchOneway = cl.ice_batchOneway();

        out.print("testing ice_invoke... ");
        out.flush();

        {
            com.zeroc.Ice.Object.Ice_invokeResult r;

            r = oneway.ice_invoke("opOneway", OperationMode.Normal, null);
            test(r.returnValue);
            test(batchOneway.ice_invoke("opOneway", OperationMode.Normal, null).returnValue);
            test(batchOneway.ice_invoke("opOneway", OperationMode.Normal, null).returnValue);
            test(batchOneway.ice_invoke("opOneway", OperationMode.Normal, null).returnValue);
            test(batchOneway.ice_invoke("opOneway", OperationMode.Normal, null).returnValue);
            batchOneway.ice_flushBatchRequests();

            OutputStream outS = new OutputStream(communicator);
            outS.startEncapsulation();
            outS.writeString(testString);
            outS.endEncapsulation();
            byte[] inEncaps = outS.finished();
            r = cl.ice_invoke("opString", OperationMode.Normal, inEncaps);
            if(r.returnValue)
            {
                InputStream inS = new InputStream(communicator, r.outParams);
                inS.startEncapsulation();
                String s = inS.readString();
                test(s.equals(testString));
                s = inS.readString();
                inS.endEncapsulation();
                test(s.equals(testString));
            }
            else
            {
                test(false);
            }
        }

        for(int i = 0; i < 2; ++i)
        {
            java.util.Map<String, String> context = null;
            if(i == 1)
            {
                context = new java.util.HashMap<String, String>();
                context.put("raise", "");
            }
            com.zeroc.Ice.Object.Ice_invokeResult r = cl.ice_invoke("opException", OperationMode.Normal, null, context);
            if(r.returnValue)
            {
                test(false);
            }
            else
            {
                InputStream inS = new InputStream(communicator, r.outParams);
                inS.startEncapsulation();
                try
                {
                    inS.throwException();
                }
                catch(MyException ex)
                {
                }
                catch(java.lang.Exception ex)
                {
                    test(false);
                }
                inS.endEncapsulation();
            }
        }

        out.println("ok");

        out.print("testing asynchronous ice_invoke... ");
        out.flush();

        {
            CompletableFuture<com.zeroc.Ice.Object.Ice_invokeResult> f =
                oneway.ice_invokeAsync("opOneway", OperationMode.Normal, null);
            com.zeroc.Ice.Object.Ice_invokeResult r = f.join();
            test(r.returnValue);

            OutputStream outS = new OutputStream(communicator);
            outS.startEncapsulation();
            outS.writeString(testString);
            outS.endEncapsulation();
            byte[] inEncaps = outS.finished();

            f = cl.ice_invokeAsync("opString", OperationMode.Normal, inEncaps);
            r = f.join();
            if(r.returnValue)
            {
                InputStream inS = new InputStream(communicator, r.outParams);
                inS.startEncapsulation();
                String s = inS.readString();
                test(s.equals(testString));
                s = inS.readString();
                inS.endEncapsulation();
                test(s.equals(testString));
            }
            else
            {
                test(false);
            }
        }

        {
            CompletableFuture<com.zeroc.Ice.Object.Ice_invokeResult> f =
                cl.ice_invokeAsync("opException", OperationMode.Normal, null);
            com.zeroc.Ice.Object.Ice_invokeResult r = f.join();
            if(r.returnValue)
            {
                test(false);
            }
            else
            {
                InputStream inS = new InputStream(communicator, r.outParams);
                inS.startEncapsulation();
                try
                {
                    inS.throwException();
                }
                catch(MyException ex)
                {
                }
                catch(java.lang.Exception ex)
                {
                    test(false);
                }
                inS.endEncapsulation();
            }
        }

        out.println("ok");

        return cl;
    }
}
