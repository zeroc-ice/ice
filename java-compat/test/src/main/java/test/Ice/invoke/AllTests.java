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

    private static class opStringI extends Ice.Callback
    {
        public opStringI(Ice.Communicator communicator)
        {
            _communicator = communicator;
        }

        @Override
        public void completed(Ice.AsyncResult result)
        {
            Ice.ByteSeqHolder outEncaps = new Ice.ByteSeqHolder();
            if(result.getProxy().end_ice_invoke(outEncaps, result))
            {
                Ice.InputStream inS = new Ice.InputStream(_communicator, outEncaps.value);
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

        public void check()
        {
            callback.check();
        }

        private Ice.Communicator _communicator;
        private Callback callback = new Callback();
    }

    private static class opExceptionI extends Ice.Callback
    {
        public opExceptionI(Ice.Communicator communicator)
        {
            _communicator = communicator;
        }

        @Override
        public void completed(Ice.AsyncResult result)
        {
            Ice.ByteSeqHolder outEncaps = new Ice.ByteSeqHolder();
            if(result.getProxy().end_ice_invoke(outEncaps, result))
            {
                test(false);
            }
            else
            {
                Ice.InputStream inS = new Ice.InputStream(_communicator, outEncaps.value);
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

        public void check()
        {
            callback.check();
        }

        private Ice.Communicator _communicator;
        private Callback callback = new Callback();
    }

    private static class Callback_Object_opStringI extends Ice.Callback_Object_ice_invoke
    {
        public Callback_Object_opStringI(Ice.Communicator communicator)
        {
            _communicator = communicator;
        }

        @Override
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

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Ice.Communicator _communicator;
        private Callback callback = new Callback();
    }

    private static class Callback_Object_opExceptionI extends Ice.Callback_Object_ice_invoke
    {
        public Callback_Object_opExceptionI(Ice.Communicator communicator)
        {
            _communicator = communicator;
        }

        @Override
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

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Ice.Communicator _communicator;
        private Callback callback = new Callback();
    }

    public static MyClassPrx
    allTests(Ice.Communicator communicator, PrintWriter out)
    {
        String ref = "test:default -p 12010";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        MyClassPrx cl = MyClassPrxHelper.checkedCast(base);
        MyClassPrx oneway = MyClassPrxHelper.uncheckedCast(cl.ice_oneway());
        MyClassPrx batchOneway = MyClassPrxHelper.uncheckedCast(cl.ice_batchOneway());

        out.print("testing ice_invoke... ");
        out.flush();

        {
            if(!oneway.ice_invoke("opOneway", Ice.OperationMode.Normal, null, null))
            {
                test(false);
            }

            test(batchOneway.ice_invoke("opOneway", Ice.OperationMode.Normal, null, null));
            test(batchOneway.ice_invoke("opOneway", Ice.OperationMode.Normal, null, null));
            test(batchOneway.ice_invoke("opOneway", Ice.OperationMode.Normal, null, null));
            test(batchOneway.ice_invoke("opOneway", Ice.OperationMode.Normal, null, null));
            batchOneway.ice_flushBatchRequests();

            Ice.OutputStream outS = new Ice.OutputStream(communicator);
            outS.startEncapsulation();
            outS.writeString(testString);
            outS.endEncapsulation();
            byte[] inEncaps = outS.finished();
            Ice.ByteSeqHolder outEncaps = new Ice.ByteSeqHolder();
            if(cl.ice_invoke("opString", Ice.OperationMode.Normal, inEncaps, outEncaps))
            {
                Ice.InputStream inS = new Ice.InputStream(communicator, outEncaps.value);
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
            Ice.ByteSeqHolder outEncaps = new Ice.ByteSeqHolder();
            if(cl.ice_invoke("opException", Ice.OperationMode.Normal, null, outEncaps))
            {
                test(false);
            }
            else
            {
                Ice.InputStream inS = new Ice.InputStream(communicator, outEncaps.value);
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
            Ice.AsyncResult result = oneway.begin_ice_invoke("opOneway", Ice.OperationMode.Normal, null);
            Ice.ByteSeqHolder outEncaps = new Ice.ByteSeqHolder();
            if(!oneway.end_ice_invoke(outEncaps, result))
            {
                test(false);
            }

            Ice.OutputStream outS = new Ice.OutputStream(communicator);
            outS.startEncapsulation();
            outS.writeString(testString);
            outS.endEncapsulation();
            byte[] inEncaps = outS.finished();

            // begin_ice_invoke with no callback
            result = cl.begin_ice_invoke("opString", Ice.OperationMode.Normal, inEncaps);
            if(cl.end_ice_invoke(outEncaps, result))
            {
                Ice.InputStream inS = new Ice.InputStream(communicator, outEncaps.value);
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

            // begin_ice_invoke with Callback
            opStringI cb1 = new opStringI(communicator);
            cl.begin_ice_invoke("opString", Ice.OperationMode.Normal, inEncaps, cb1);
            cb1.check();

            // begin_ice_invoke with Callback_Object_ice_invoke
            Callback_Object_opStringI cb2 = new Callback_Object_opStringI(communicator);
            cl.begin_ice_invoke("opString", Ice.OperationMode.Normal, inEncaps, cb2);
            cb2.check();
        }

        {
            // begin_ice_invoke with no callback
            Ice.AsyncResult result = cl.begin_ice_invoke("opException", Ice.OperationMode.Normal, null);
            Ice.ByteSeqHolder outEncaps = new Ice.ByteSeqHolder();
            if(cl.end_ice_invoke(outEncaps, result))
            {
                test(false);
            }
            else
            {
                Ice.InputStream inS = new Ice.InputStream(communicator, outEncaps.value);
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

            // begin_ice_invoke with Callback
            opExceptionI cb1 = new opExceptionI(communicator);
            cl.begin_ice_invoke("opException", Ice.OperationMode.Normal, null, cb1);
            cb1.check();

            // begin_ice_invoke with Callback_Object_ice_invoke
            Callback_Object_opExceptionI cb2 = new Callback_Object_opExceptionI(communicator);
            cl.begin_ice_invoke("opException", Ice.OperationMode.Normal, null, cb2);
            cb2.check();
        }

        out.println("ok");

        return cl;
    }
}
