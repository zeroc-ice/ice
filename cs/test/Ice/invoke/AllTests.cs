// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Threading;

public class AllTests
{
    private static string testString = "This is a test string";

    private class Cookie
    {
        public string getString()
        {
            return testString;
        }
    }

    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.Exception();
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
                    Monitor.Wait(this);
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
                Monitor.Pulse(this);
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

            byte[] outParams;
            if(result.getProxy().end_ice_invoke(out outParams, result))
            {
                Ice.InputStream inS = Ice.Util.createInputStream(_communicator, outParams);
                string s = inS.readString();
                test(s.Equals(cmp));
                s = inS.readString();
                test(s.Equals(cmp));
                callback.called();
            }
            else
            {
                test(false);
            }
        }

        public void opStringNC(bool ok, byte[] outParams)
        {
            if(ok)
            {
                Ice.InputStream inS = Ice.Util.createInputStream(_communicator, outParams);
                string s = inS.readString();
                test(s.Equals(testString));
                s = inS.readString();
                test(s.Equals(testString));
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

            byte[] outParams;
            if(result.getProxy().end_ice_invoke(out outParams, result))
            {
                test(false);
            }
            else
            {
                Ice.InputStream inS = Ice.Util.createInputStream(_communicator, outParams);
                try
                {
                    inS.throwException();
                }
                catch(Test.MyException)
                {
                    callback.called();
                }
                catch(System.Exception)
                {
                    test(false);
                }
            }
        }

        public void opExceptionNC(bool ok, byte[] outParams)
        {
            if(ok)
            {
                test(false);
            }
            else
            {
                Ice.InputStream inS = Ice.Util.createInputStream(_communicator, outParams);
                try
                {
                    inS.throwException();
                }
                catch(Test.MyException)
                {
                    callback.called();
                }
                catch(System.Exception)
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

    public static Test.MyClassPrx allTests(Ice.Communicator communicator)
    {
        Ice.ObjectPrx baseProxy = communicator.stringToProxy("test:default -p 12010");
        Test.MyClassPrx cl = Test.MyClassPrxHelper.checkedCast(baseProxy);

        Console.Out.Write("testing ice_invoke... ");
        Console.Out.Flush();

        {
            byte[] inParams, outParams;
            Ice.OutputStream outS = Ice.Util.createOutputStream(communicator);
            outS.writeString(testString);
            inParams = outS.finished();

            if(cl.ice_invoke("opString", Ice.OperationMode.Normal, inParams, out outParams))
            {
                Ice.InputStream inS = Ice.Util.createInputStream(communicator, outParams);
                string s = inS.readString();
                test(s.Equals(testString));
                s = inS.readString();
                test(s.Equals(testString));
            }
            else
            {
                test(false);
            }
        }

        {
            byte[] outParams;
            if(cl.ice_invoke("opException", Ice.OperationMode.Normal, null, out outParams))
            {
                test(false);
            }
            else
            {
                Ice.InputStream inS = Ice.Util.createInputStream(communicator, outParams);
                try
                {
                    inS.throwException();
                }
                catch(Test.MyException)
                {
                }
                catch(System.Exception)
                {
                    test(false);
                }
            }
        }

        Console.Out.WriteLine("ok");

        Console.Out.Write("testing asynchronous ice_invoke... ");
        Console.Out.Flush();

        {
            byte[] inParams, outParams;
            Ice.OutputStream outS = Ice.Util.createOutputStream(communicator);
            outS.writeString(testString);
            inParams = outS.finished();

            // begin_ice_invoke with no callback
            Ice.AsyncResult result = cl.begin_ice_invoke("opString", Ice.OperationMode.Normal, inParams);
            if(cl.end_ice_invoke(out outParams, result))
            {
                Ice.InputStream inS = Ice.Util.createInputStream(communicator, outParams);
                string s = inS.readString();
                test(s.Equals(testString));
                s = inS.readString();
                test(s.Equals(testString));
            }
            else
            {
                test(false);
            }

            // begin_ice_invoke with Callback
            Callback cb = new Callback(communicator, false);
            cl.begin_ice_invoke("opString", Ice.OperationMode.Normal, inParams, cb.opString, null);
            cb.check();

            // begin_ice_invoke with Callback with cookie
            cb = new Callback(communicator, true);
            cl.begin_ice_invoke("opString", Ice.OperationMode.Normal, inParams, cb.opString, new Cookie());
            cb.check();

            // begin_ice_invoke with Callback_Object_ice_invoke
            cb = new Callback(communicator, true);
            cl.begin_ice_invoke("opString", Ice.OperationMode.Normal, inParams).whenCompleted(cb.opStringNC, null);
            cb.check();
        }

        {
            // begin_ice_invoke with no callback
            Ice.AsyncResult result = cl.begin_ice_invoke("opException", Ice.OperationMode.Normal, null);
            byte[] outParams;
            if(cl.end_ice_invoke(out outParams, result))
            {
                test(false);
            }
            else
            {
                Ice.InputStream inS = Ice.Util.createInputStream(communicator, outParams);
                try
                {
                    inS.throwException();
                }
                catch(Test.MyException)
                {
                }
                catch(System.Exception)
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

        Console.Out.WriteLine("ok");

        return cl;
    }
}
