// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


using System;
using System.Collections.Generic;
using System.Diagnostics;


public class AllTests : TestCommon.AllTests
{
    private static string testString = "This is a test string";

    private class Cookie
    {
        public string getString()
        {
            return testString;
        }
    }

    private class CallbackBase
    {
        internal CallbackBase()
        {
            _called = false;
        }

        public virtual void check()
        {
            lock(this)
            {
                while(!_called)
                {
                    System.Threading.Monitor.Wait(this);
                }

                _called = false;
            }
        }

        public virtual void called()
        {
            lock(this)
            {
                Debug.Assert(!_called);
                _called = true;
                System.Threading.Monitor.Pulse(this);
            }
        }

        private bool _called;
    }

    private class Callback
    {
        public Callback(Ice.Communicator communicator, bool useCookie)
        {
            _communicator = communicator;
            _useCookie = useCookie;
        }

        public void opString(Ice.AsyncResult result)
        {
            string cmp = testString;
            if(_useCookie)
            {
                Cookie cookie = (Cookie)result.AsyncState;
                cmp = cookie.getString();
            }

            byte[] outEncaps;
            if(result.getProxy().end_ice_invoke(out outEncaps, result))
            {
                Ice.InputStream inS = new Ice.InputStream(_communicator, outEncaps);
                inS.startEncapsulation();
                string s = inS.readString();
                test(s.Equals(cmp));
                s = inS.readString();
                test(s.Equals(cmp));
                inS.endEncapsulation();
                callback.called();
            }
            else
            {
                test(false);
            }
        }

        public void opStringNC(bool ok, byte[] outEncaps)
        {
            if(ok)
            {
                Ice.InputStream inS = new Ice.InputStream(_communicator, outEncaps);
                inS.startEncapsulation();
                string s = inS.readString();
                test(s.Equals(testString));
                s = inS.readString();
                test(s.Equals(testString));
                inS.endEncapsulation();
                callback.called();
            }
            else
            {
                test(false);
            }
        }

        public void opException(Ice.AsyncResult result)
        {
            if(_useCookie)
            {
                Cookie cookie = (Cookie)result.AsyncState;
                test(cookie.getString().Equals(testString));
            }

            byte[] outEncaps;
            if(result.getProxy().end_ice_invoke(out outEncaps, result))
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
                catch(Test.MyException)
                {
                    inS.endEncapsulation();
                    callback.called();
                }
                catch(Exception)
                {
                    test(false);
                }
            }
        }

        public void opExceptionNC(bool ok, byte[] outEncaps)
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
                catch(Test.MyException)
                {
                    inS.endEncapsulation();
                    callback.called();
                }
                catch(Exception)
                {
                    test(false);
                }
            }
        }

        public virtual void check()
        {
            callback.check();
        }

        private Ice.Communicator _communicator;
        private bool _useCookie;

        private CallbackBase callback = new CallbackBase();
    }

    public static Test.MyClassPrx allTests(TestCommon.Application app)
    {
        Ice.Communicator communicator = app.communicator();
        Ice.ObjectPrx baseProxy = communicator.stringToProxy("test:" + app.getTestEndpoint(0));
        Test.MyClassPrx cl = Test.MyClassPrxHelper.checkedCast(baseProxy);
        Test.MyClassPrx oneway = Test.MyClassPrxHelper.uncheckedCast(cl.ice_oneway());
        Test.MyClassPrx batchOneway = Test.MyClassPrxHelper.uncheckedCast(cl.ice_batchOneway());

        Write("testing ice_invoke... ");
        Flush();

        {
            byte[] inEncaps, outEncaps;
            if(!oneway.ice_invoke("opOneway", Ice.OperationMode.Normal, null, out outEncaps))
            {
                test(false);
            }

            test(batchOneway.ice_invoke("opOneway", Ice.OperationMode.Normal, null, out outEncaps));
            test(batchOneway.ice_invoke("opOneway", Ice.OperationMode.Normal, null, out outEncaps));
            test(batchOneway.ice_invoke("opOneway", Ice.OperationMode.Normal, null, out outEncaps));
            test(batchOneway.ice_invoke("opOneway", Ice.OperationMode.Normal, null, out outEncaps));
            batchOneway.ice_flushBatchRequests();

            Ice.OutputStream outS = new Ice.OutputStream(communicator);
            outS.startEncapsulation();
            outS.writeString(testString);
            outS.endEncapsulation();
            inEncaps = outS.finished();

            if(cl.ice_invoke("opString", Ice.OperationMode.Normal, inEncaps, out outEncaps))
            {
                Ice.InputStream inS = new Ice.InputStream(communicator, outEncaps);
                inS.startEncapsulation();
                string s = inS.readString();
                test(s.Equals(testString));
                s = inS.readString();
                inS.endEncapsulation();
                test(s.Equals(testString));
            }
            else
            {
                test(false);
            }
        }

        for(int i = 0; i < 2; ++i)
        {
            byte[] outEncaps;
            Dictionary<string, string> ctx = null;
            if(i == 1)
            {
                ctx = new Dictionary<string, string>();
                ctx["raise"] = "";
            }

            if(cl.ice_invoke("opException", Ice.OperationMode.Normal, null, out outEncaps, ctx))
            {
                test(false);
            }
            else
            {
                Ice.InputStream inS = new Ice.InputStream(communicator, outEncaps);
                inS.startEncapsulation();
                try
                {
                    inS.throwException();
                }
                catch(Test.MyException)
                {
                    inS.endEncapsulation();
                }
                catch(Exception)
                {
                    test(false);
                }
            }
        }

        WriteLine("ok");

        Write("testing asynchronous ice_invoke with Async Task API... ");
        Flush();

        {
            try
            {
                oneway.ice_invokeAsync("opOneway", Ice.OperationMode.Normal, null).Wait();
            }
            catch(Exception)
            {
                test(false);
            }


            Ice.OutputStream outS = new Ice.OutputStream(communicator);
            outS.startEncapsulation();
            outS.writeString(testString);
            outS.endEncapsulation();
            byte[] inEncaps = outS.finished();

            // begin_ice_invoke with no callback
            var result = cl.ice_invokeAsync("opString", Ice.OperationMode.Normal, inEncaps).Result;
            if(result.returnValue)
            {
                Ice.InputStream inS = new Ice.InputStream(communicator, result.outEncaps);
                inS.startEncapsulation();
                string s = inS.readString();
                test(s.Equals(testString));
                s = inS.readString();
                inS.endEncapsulation();
                test(s.Equals(testString));
            }
            else
            {
                test(false);
            }
        }

        {
            var result = cl.ice_invokeAsync("opException", Ice.OperationMode.Normal, null).Result;
            if(result.returnValue)
            {
                test(false);
            }
            else
            {
                Ice.InputStream inS = new Ice.InputStream(communicator, result.outEncaps);
                inS.startEncapsulation();
                try
                {
                    inS.throwException();
                }
                catch(Test.MyException)
                {
                    inS.endEncapsulation();
                }
                catch(Exception)
                {
                    test(false);
                }
            }
        }

        WriteLine("ok");

        Write("testing asynchronous ice_invoke with AsyncResult API... ");
        Flush();

        {
            byte[] inEncaps, outEncaps;
            Ice.AsyncResult result = oneway.begin_ice_invoke("opOneway", Ice.OperationMode.Normal, null);
            if(!oneway.end_ice_invoke(out outEncaps, result))
            {
                test(false);
            }

            Ice.OutputStream outS = new Ice.OutputStream(communicator);
            outS.startEncapsulation();
            outS.writeString(testString);
            outS.endEncapsulation();
            inEncaps = outS.finished();

            // begin_ice_invoke with no callback
            result = cl.begin_ice_invoke("opString", Ice.OperationMode.Normal, inEncaps);
            if(cl.end_ice_invoke(out outEncaps, result))
            {
                Ice.InputStream inS = new Ice.InputStream(communicator, outEncaps);
                inS.startEncapsulation();
                string s = inS.readString();
                test(s.Equals(testString));
                s = inS.readString();
                inS.endEncapsulation();
                test(s.Equals(testString));
            }
            else
            {
                test(false);
            }

            // begin_ice_invoke with Callback
            Callback cb = new Callback(communicator, false);
            cl.begin_ice_invoke("opString", Ice.OperationMode.Normal, inEncaps, cb.opString, null);
            cb.check();

            // begin_ice_invoke with Callback with cookie
            cb = new Callback(communicator, true);
            cl.begin_ice_invoke("opString", Ice.OperationMode.Normal, inEncaps, cb.opString, new Cookie());
            cb.check();

            // begin_ice_invoke with Callback_Object_ice_invoke
            cb = new Callback(communicator, true);
            cl.begin_ice_invoke("opString", Ice.OperationMode.Normal, inEncaps).whenCompleted(cb.opStringNC, null);
            cb.check();
        }

        {
            // begin_ice_invoke with no callback
            Ice.AsyncResult result = cl.begin_ice_invoke("opException", Ice.OperationMode.Normal, null);
            byte[] outEncaps;
            if(cl.end_ice_invoke(out outEncaps, result))
            {
                test(false);
            }
            else
            {
                Ice.InputStream inS = new Ice.InputStream(communicator, outEncaps);
                inS.startEncapsulation();
                try
                {
                    inS.throwException();
                }
                catch(Test.MyException)
                {
                    inS.endEncapsulation();
                }
                catch(Exception)
                {
                    test(false);
                }
            }

            // begin_ice_invoke with Callback
            Callback cb = new Callback(communicator, false);
            cl.begin_ice_invoke("opException", Ice.OperationMode.Normal, null, cb.opException, null);
            cb.check();

            // begin_ice_invoke with Callback with cookie
            cb = new Callback(communicator, true);
            cl.begin_ice_invoke("opException", Ice.OperationMode.Normal, null, cb.opException, new Cookie());
            cb.check();

            // begin_ice_invoke with Callback_Object_ice_invoke
            cb = new Callback(communicator, true);
            cl.begin_ice_invoke("opException", Ice.OperationMode.Normal, null).whenCompleted(cb.opExceptionNC, null);
            cb.check();
        }

        WriteLine("ok");
        return cl;
    }
}
