// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.invoke.lambda;
import java.io.PrintWriter;

import test.Ice.invoke.Test.MyClassPrx;
import test.Ice.invoke.Test.MyClassPrxHelper;
import test.Ice.invoke.Test.MyException;

public class AllTests
{
    final static String testString = "This is a test string";

    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private static class Callback
    {
        Callback()
        {
            _called = false;
        }

        public synchronized void check()
        {
            while(!_called)
            {
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                }
            }

            _called = false;
        }

        public synchronized void called()
        {
            assert(!_called);
            _called = true;
            notify();
        }

        private boolean _called;
    }

    private static class Callback_Object_opStringI
    {
        public Callback_Object_opStringI(Ice.Communicator communicator)
        {
            _communicator = communicator;
        }

        public void response(boolean ok, byte[] outEncaps)
        {
            if(ok)
            {
                Ice.InputStream inS = new Ice.InputStream(_communicator, outEncaps);
                inS.startEncapsulation();
                String s = inS.readString();
                test(s.equals(testString));
                s = inS.readString();
                test(s.equals(testString));
                inS.endEncapsulation();
                callback.called();
            }
            else
            {
                test(false);
            }
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public void sent(boolean sent)
        {
        }

        public void check()
        {
            callback.check();
        }

        private Ice.Communicator _communicator;
        private Callback callback = new Callback();
    }

    private static class Callback_Object_opExceptionI
    {
        public Callback_Object_opExceptionI(Ice.Communicator communicator)
        {
            _communicator = communicator;
        }

        public void response(boolean ok, byte[] outEncaps)
        {
            if(ok)
            {
                test(false);
            }
            else
            {
                Ice.InputStream inS = new Ice.InputStream(_communicator, outEncaps);
                inS.startEncapsulation();
                try
                {
                    inS.throwException();
                }
                catch(MyException ex)
                {
                    inS.endEncapsulation();
                    callback.called();
                }
                catch(java.lang.Exception ex)
                {
                    test(false);
                }
            }
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        public void sent(boolean sent)
        {
        }

        private Ice.Communicator _communicator;
        private Callback callback = new Callback();
    }

    public static MyClassPrx
    allTests(test.Util.Application app)
    {
        Ice.Communicator communicator = app.communicator();
        PrintWriter out = app.getWriter();
        String ref = "test:" + app.getTestEndpoint(0);
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        MyClassPrx cl = MyClassPrxHelper.checkedCast(base);
        MyClassPrx oneway = MyClassPrxHelper.uncheckedCast(cl.ice_oneway());

        out.print("testing asynchronous ice_invoke with lambda callbacks... ");
        out.flush();

        {
            Ice.OutputStream outS = new Ice.OutputStream(communicator);
            outS.startEncapsulation();
            outS.writeString(testString);
            outS.endEncapsulation();
            byte[] inEncaps = outS.finished();

            // begin_ice_invoke with Callback_Object_ice_invoke
            Callback_Object_opStringI cb2 = new Callback_Object_opStringI(communicator);
            cl.begin_ice_invoke("opString", Ice.OperationMode.Normal, inEncaps,
                (boolean ret, byte[] outParams) -> cb2.response(ret, outParams),
                (Ice.Exception ex) -> cb2.exception(ex),
                (boolean sent) -> cb2.sent(sent));
            cb2.check();
        }

        {
            // begin_ice_invoke with Callback_Object_ice_invoke
            Callback_Object_opExceptionI cb2 = new Callback_Object_opExceptionI(communicator);
            cl.begin_ice_invoke("opException", Ice.OperationMode.Normal, null,
                (boolean ret, byte[] outParams) -> cb2.response(ret, outParams),
                (Ice.Exception ex) -> cb2.exception(ex),
                (boolean sent) -> cb2.sent(sent));
            cb2.check();
        }

        out.println("ok");

        return cl;
    }
}
