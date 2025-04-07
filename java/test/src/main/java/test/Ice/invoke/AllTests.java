// Copyright (c) ZeroC, Inc.

package test.Ice.invoke;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InputStream;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.OperationMode;
import com.zeroc.Ice.OutputStream;

import test.Ice.invoke.Test.MyClassPrx;
import test.Ice.invoke.Test.MyException;
import test.TestHelper;

import java.io.PrintWriter;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.CompletableFuture;

public class AllTests {
    static final String testString = "This is a test string";

    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public static MyClassPrx allTests(TestHelper helper) {
        Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();
        String ref = "test:" + helper.getTestEndpoint(0);
        ObjectPrx base = communicator.stringToProxy(ref);
        MyClassPrx cl = MyClassPrx.checkedCast(base);
        MyClassPrx oneway = cl.ice_oneway();
        MyClassPrx batchOneway = cl.ice_batchOneway();

        out.print("testing ice_invoke... ");
        out.flush();
        {
            Object.Ice_invokeResult r;

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
            if (r.returnValue) {
                InputStream inS = new InputStream(communicator, r.outParams);
                inS.startEncapsulation();
                String s = inS.readString();
                test(testString.equals(s));
                s = inS.readString();
                inS.endEncapsulation();
                test(testString.equals(s));
            } else {
                test(false);
            }
        }

        for (int i = 0; i < 2; i++) {
            Map<String, String> context = null;
            if (i == 1) {
                context = new HashMap<String, String>();
                context.put("raise", "");
            }
            Object.Ice_invokeResult r =
                    cl.ice_invoke("opException", OperationMode.Normal, null, context);
            if (r.returnValue) {
                test(false);
            } else {
                InputStream inS = new InputStream(communicator, r.outParams);
                inS.startEncapsulation();
                try {
                    inS.throwException();
                } catch (MyException ex) {
                } catch (Exception ex) {
                    test(false);
                }
                inS.endEncapsulation();
            }
        }

        out.println("ok");

        out.print("testing asynchronous ice_invoke... ");
        out.flush();

        {
            CompletableFuture<Object.Ice_invokeResult> f =
                    oneway.ice_invokeAsync("opOneway", OperationMode.Normal, null);
            Object.Ice_invokeResult r = f.join();
            test(r.returnValue);

            OutputStream outS = new OutputStream(communicator);
            outS.startEncapsulation();
            outS.writeString(testString);
            outS.endEncapsulation();
            byte[] inEncaps = outS.finished();

            f = cl.ice_invokeAsync("opString", OperationMode.Normal, inEncaps);
            r = f.join();
            if (r.returnValue) {
                InputStream inS = new InputStream(communicator, r.outParams);
                inS.startEncapsulation();
                String s = inS.readString();
                test(testString.equals(s));
                s = inS.readString();
                inS.endEncapsulation();
                test(testString.equals(s));
            } else {
                test(false);
            }
        }

        {
            CompletableFuture<Object.Ice_invokeResult> f =
                    cl.ice_invokeAsync("opException", OperationMode.Normal, null);
            Object.Ice_invokeResult r = f.join();
            if (r.returnValue) {
                test(false);
            } else {
                InputStream inS = new InputStream(communicator, r.outParams);
                inS.startEncapsulation();
                try {
                    inS.throwException();
                } catch (MyException ex) {
                } catch (Exception ex) {
                    test(false);
                }
                inS.endEncapsulation();
            }
        }

        out.println("ok");

        return cl;
    }

    private AllTests() {
    }
}
