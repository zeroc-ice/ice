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

        public void check()
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

        public void called()
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

        public void check()
        {
            callback.check();
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
            test(ex is Ice.ConnectionLostException);
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    public static Test.RetryPrx allTests(Ice.Communicator communicator)
    {
        Console.Out.Write("testing stringToProxy... ");
        Console.Out.Flush();
        string rf = "retry:default -p 12010";
        Ice.ObjectPrx base1 = communicator.stringToProxy(rf);
        test(base1 != null);
        Ice.ObjectPrx base2 = communicator.stringToProxy(rf);
        test(base2 != null);
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing checked cast... ");
        Console.Out.Flush();
        Test.RetryPrx retry1 = Test.RetryPrxHelper.checkedCast(base1);
        test(retry1 != null);
        test(retry1.Equals(base1));
        Test.RetryPrx retry2 = Test.RetryPrxHelper.checkedCast(base2);
        test(retry2 != null);
        test(retry2.Equals(base2));
        Console.Out.WriteLine("ok");

        Console.Out.Write("calling regular operation with first proxy... ");
        Console.Out.Flush();
        retry1.op(false);
        Console.Out.WriteLine("ok");

        Console.Out.Write("calling operation to kill connection with second proxy... ");
        Console.Out.Flush();
        try
        {
            retry2.op(true);
            test(false);
        }
        catch(Ice.ConnectionLostException)
        {
            Console.Out.WriteLine("ok");
        }

        Console.Out.Write("calling regular operation with first proxy again... ");
        Console.Out.Flush();
        retry1.op(false);
        Console.Out.WriteLine("ok");

        AMIRegular cb1 = new AMIRegular();
        AMIException cb2 = new AMIException();

        Console.Out.Write("calling regular AMI operation with first proxy... ");
        retry1.begin_op(false).whenCompleted(cb1.response, cb1.exception);
        cb1.check();
        Console.Out.WriteLine("ok");

        Console.Out.Write("calling AMI operation to kill connection with second proxy... ");
        retry2.begin_op(true).whenCompleted(cb2.response, cb2.exception);
        cb2.check();
        Console.Out.WriteLine("ok");

        Console.Out.Write("calling regular AMI operation with first proxy again... ");
        retry1.begin_op(false).whenCompleted(cb1.response, cb1.exception);
        cb1.check();
        Console.Out.WriteLine("ok");

        return retry1;
    }
}
