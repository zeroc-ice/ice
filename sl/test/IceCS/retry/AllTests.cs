// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Threading;
using System.Collections.Generic;

public class AllTests
{
    public class LoggerI : Ice.Logger
    {
        public LoggerI()
        {
        }
        
        public void print(string message)
        {
        }

        public void trace(string category, string message)
        {
            _messages.Add(message);
        }
        
        public void warning(string message)
        {
        }
        
        public void error(string message)
        {
        }

        public List<string> messages()
        {
            List<string> messages = new List<string>(_messages);
            _messages.Clear();
            return messages;
        }

        List<string> _messages = new List<String>();
    }

    private static void test(bool b)
    {
        if(!b)
        {
            throw new Exception();
        }
    }

    private class Callback
    {
        internal Callback()
        {
            _called = false;
        }

        public bool check()
        {
            int cnt = 0;
            do
            {
                lock(this)
                {
                    if(_called)
                    {
                        _called = false;
                        return true;
                    }
                }
                Thread.Sleep(100);
            }
            while(++cnt < 50);

            return false; // Must be timeout
        }

        public void called()
        {
            lock(this)
            {
                Debug.Assert(!_called);
                _called = true;
            }
        }

        private bool _called;
    }

    private class AMIRegular 
    {
        public void response()
        {
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public bool check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMIException 
    {
        public void response()
        {
            test(false);
        }

        public void exception(Ice.Exception ex)
        {
            test(ex is Ice.UnknownException);
            callback.called();
        }

        public bool check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMIExceptionError 
    {
        public void response()
        {
            test(false);
        }

        public void exception(Ice.Exception ex)
        {
            test(ex is Ice.SocketException);
            callback.called();
        }

        public bool check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    public static void allTests(Ice.Communicator communicator, LoggerI logger)
    {
        Console.Out.Write("testing stringToProxy... ");
        Console.Out.Flush();
        string rf = "retry:default -p 12010 -t 10000";
        Ice.ObjectPrx base1 = communicator.stringToProxy(rf);
        test(base1 != null);
        Ice.ObjectPrx base2 = communicator.stringToProxy(rf);
        test(base2 != null);
        Console.Out.WriteLine("ok");

        Console.Out.Flush();
        Test.RetryPrx retry1 = Test.RetryPrxHelper.uncheckedCast(base1);
        test(retry1 != null);
        test(retry1.Equals(base1));
	// Set a locator proxy so we'll cause a retry.
        Test.RetryPrx retry2 = Test.RetryPrxHelper.uncheckedCast(base2); //.ice_locator(retry1));
        test(retry2 != null);
        test(retry2.Equals(base2));

        Console.Out.Write("calling regular operation... ");
        Console.Out.Flush();
        retry1.op();
        Console.Out.WriteLine("ok");

        Console.Out.Write("calling operation to cause retry... ");
        Console.Out.Flush();
        try
        {
            retry2.opkill();
            test(false);
        }
        catch(Ice.UnknownException)
        {
            Console.Out.WriteLine("ok");
        }

        List<string> messages = logger.messages();
        test(messages.Count == 2);
        test(messages[0].StartsWith("retrying operation"));
        test(messages[1].StartsWith("cannot retry"));

        Console.Out.Write("calling regular operation again... ");
        Console.Out.Flush();
        retry1.op();
        Console.Out.WriteLine("ok");

        Console.Out.Write("calling operation to cause error... ");
        Console.Out.Flush();
        try
        {
            retry2.operror();
            test(false);
        }
        catch(Ice.SocketException)
        {
            Console.Out.WriteLine("ok");
        }

        messages = logger.messages();
        test(messages.Count == 0);

        Console.Out.Write("calling regular operation again... ");
        Console.Out.Flush();
        retry1.op();
        Console.Out.WriteLine("ok");

        AMIRegular cb1 = new AMIRegular();
        AMIException cb2 = new AMIException();
        AMIExceptionError cb3 = new AMIExceptionError();

        Console.Out.Write("calling regular AMI operation... ");
        retry1.op_async(cb1.response, cb1.exception);
        test(cb1.check());
        Console.Out.WriteLine("ok");

        Console.Out.Write("calling AMI operation to cause retry... ");
        retry2.opkill_async(cb2.response, cb2.exception);
        test(cb2.check());
        Console.Out.WriteLine("ok");

        messages = logger.messages();
        test(messages.Count == 2);
        test(messages[0].StartsWith("retrying operation"));
        test(messages[1].StartsWith("cannot retry"));

        Console.Out.Write("calling regular AMI operation again... ");
        retry1.op_async(cb1.response, cb1.exception);
        test(cb1.check());
        Console.Out.WriteLine("ok");

        Console.Out.Write("calling AMI operation to cause error... ");
        retry2.operror_async(cb3.response, cb3.exception);
        test(cb3.check());
        Console.Out.WriteLine("ok");

        messages = logger.messages();
        test(messages.Count == 0);

        Console.Out.Write("calling regular AMI operation again... ");
        retry1.op_async(cb1.response, cb1.exception);
        test(cb1.check());
        Console.Out.WriteLine("ok");
    }
}
