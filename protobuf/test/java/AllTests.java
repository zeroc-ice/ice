// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Protobuf is licensed to you under the terms
// described in the ICE_PROTOBUF_LICENSE file included in this
// distribution.
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

        public synchronized void
        check()
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

        public synchronized void
        called()
        {
            assert(!_called);
            _called = true;
            notify();
        }

        private boolean _called;
    }

    private static class AMI_MyClass_opMessage extends Test.AMI_MyClass_opMessage
    {
        public void
        ice_response(test.TestPB.Message r, test.TestPB.Message o)
        {
            test(o.getI() == 99);
            test(r.getI() == 99);
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opMessageAMD extends Test.AMI_MyClass_opMessageAMD
    {
        public void
        ice_response(test.TestPB.Message r, test.TestPB.Message o)
        {
            test(o.getI() == 99);
            test(r.getI() == 99);
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    public static Test.MyClassPrx
    allTests(Ice.Communicator communicator, boolean collocated)
    {
        String ref = "test:default -p 12010";
        Ice.ObjectPrx baseProxy = communicator.stringToProxy(ref);
        Test.MyClassPrx cl = Test.MyClassPrxHelper.checkedCast(baseProxy);

        System.out.print("testing twoway operations... ");
        {
            test.TestPB.Message i = test.TestPB.Message.newBuilder().setI(99).build();
            Ice.Holder<test.TestPB.Message> o = new Ice.Holder<test.TestPB.Message>();
            test.TestPB.Message r;

            r = cl.opMessage(i, o);

            test(o.value.getI() == 99);
            test(r.getI() == 99);
        }
        {
            test.TestPB.Message i = test.TestPB.Message.newBuilder().setI(99).build();
            Ice.Holder<test.TestPB.Message> o = new Ice.Holder<test.TestPB.Message>();
            test.TestPB.Message r;

            r = cl.opMessageAMD(i, o);

            test(o.value.getI() == 99);
            test(r.getI() == 99);
        }
        System.out.println("ok");

        System.out.print("testing twoway AMI operations... ");
        {
            test.TestPB.Message i = test.TestPB.Message.newBuilder().setI(99).build();

            AMI_MyClass_opMessage cb = new AMI_MyClass_opMessage();
            cl.opMessage_async(cb, i);
            cb.check();
        }
        {
            test.TestPB.Message i = test.TestPB.Message.newBuilder().setI(99).build();

            AMI_MyClass_opMessageAMD cb = new AMI_MyClass_opMessageAMD();
            cl.opMessageAMD_async(cb, i);
            cb.check();
        }
        System.out.println("ok");

        return cl;
    }
}
