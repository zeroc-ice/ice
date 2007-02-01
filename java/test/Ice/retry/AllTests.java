// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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

    private static class Callback
    {
        Callback()
        {
            _called = false;
        }

        public synchronized boolean
        check()
        {
            while(!_called)
            {
                try
                {
                    wait(5000);
                }
                catch(InterruptedException ex)
                {
                    continue;
                }

                if(!_called)
                {
                    return false; // Must be timeout.
                }
            }

            _called = false;
            return true;
        }

        public synchronized void
        called()
        {
            assert(!_called);
            _called = true;
            notify();
        }

        private boolean _called;
    }

    private static class AMIRegular extends Test.AMI_Retry_op
    {
        public void
        ice_response()
        {
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMIException extends Test.AMI_Retry_op
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(ex instanceof Ice.ConnectionLostException);
            callback.called();
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    public static Test.RetryPrx
    allTests(Ice.Communicator communicator, java.io.PrintStream out)
    {
        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "retry:default -p 12010 -t 10000";
        Ice.ObjectPrx base1 = communicator.stringToProxy(ref);
        test(base1 != null);
        Ice.ObjectPrx base2 = communicator.stringToProxy(ref);
        test(base2 != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        Test.RetryPrx retry1 = Test.RetryPrxHelper.checkedCast(base1);
        test(retry1 != null);
        test(retry1.equals(base1));
        Test.RetryPrx retry2 = Test.RetryPrxHelper.checkedCast(base2);
        test(retry2 != null);
        test(retry2.equals(base2));
        out.println("ok");

        out.print("calling regular operation with first proxy... ");
        out.flush();
        retry1.op(false);
        out.println("ok");

        out.print("calling operation to kill connection with second proxy... ");
        out.flush();
        try
        {
            retry2.op(true);
            test(false);
        }
        catch(Ice.ConnectionLostException ex)
        {
            out.println("ok");
        }

        out.print("calling regular operation with first proxy again... ");
        out.flush();
        retry1.op(false);
        out.println("ok");

        AMIRegular cb1 = new AMIRegular();
        AMIException cb2 = new AMIException();

        out.print("calling regular AMI operation with first proxy... ");
        retry1.op_async(cb1, false);
        test(cb1.check());
        out.println("ok");

        out.print("calling AMI operation to kill connection with second proxy... ");
        retry2.op_async(cb2, true);
        test(cb2.check());
        out.println("ok");

        out.print("calling regular AMI operation with first proxy again... ");
        retry1.op_async(cb1, false);
        test(cb1.check());
        out.println("ok");

        return retry1;
    }
}
