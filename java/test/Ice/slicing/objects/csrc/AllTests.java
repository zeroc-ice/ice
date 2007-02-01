// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

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

    private static class AMI_Test_SBaseAsObjectI extends AMI_TestIntf_SBaseAsObject
    {
        public void
        ice_response(Ice.Object o)
        {
            test(o != null);
            test(o.ice_id().equals("::Test::SBase"));
            SBase sb = (SBase)o;
            test(sb != null);
            test(sb.sb.equals("SBase.sb"));
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
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

    private static class AMI_Test_SBaseAsSBaseI extends AMI_TestIntf_SBaseAsSBase
    {
        public void
        ice_response(SBase sb)
        {
            test(sb.sb.equals("SBase.sb"));
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
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

    private static class AMI_Test_SBSKnownDerivedAsSBaseI extends AMI_TestIntf_SBSKnownDerivedAsSBase
    {
        public void
        ice_response(SBase sb)
        {
            test(sb.sb.equals("SBSKnownDerived.sb"));
            SBSKnownDerived sbskd = (SBSKnownDerived)sb;
            test(sbskd != null);
            test(sbskd.sbskd.equals("SBSKnownDerived.sbskd"));
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
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

    private static class AMI_Test_SBSKnownDerivedAsSBSKnownDerivedI
        extends AMI_TestIntf_SBSKnownDerivedAsSBSKnownDerived
    {
        public void
        ice_response(SBSKnownDerived sbskd)
        {
            test(sbskd.sbskd.equals("SBSKnownDerived.sbskd"));
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
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

    private static class AMI_Test_SBSUnknownDerivedAsSBaseI extends AMI_TestIntf_SBSUnknownDerivedAsSBase
    {
        public void
        ice_response(SBase sb)
        {
            test(sb.sb.equals("SBSUnknownDerived.sb"));
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
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

    private static class AMI_Test_SUnknownAsObjectI extends AMI_TestIntf_SUnknownAsObject
    {
        public void
        ice_response(Ice.Object o)
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException exc)
        {

            test(exc.ice_name().equals("Ice::NoObjectFactoryException"));
            callback.called();
        }

        public void
        ice_exception(Ice.UserException exc)
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

    private static class AMI_Test_oneElementCycleI extends AMI_TestIntf_oneElementCycle
    {
        public void
        ice_response(B b)
        {
            test(b != null);
            test(b.ice_id().equals("::Test::B"));
            test(b.sb.equals("B1.sb"));
            test(b.pb == b);
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
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

    private static class AMI_Test_twoElementCycleI extends AMI_TestIntf_twoElementCycle
    {
        public void
        ice_response(B b1)
        {
            test(b1 != null);
            test(b1.ice_id().equals("::Test::B"));
            test(b1.sb.equals("B1.sb"));

            B b2 = b1.pb;
            test(b2 != null);
            test(b2.ice_id().equals("::Test::B"));
            test(b2.sb.equals("B2.sb"));
            test(b2.pb == b1);
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
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

    private static class AMI_Test_D1AsBI extends AMI_TestIntf_D1AsB
    {
        public void
        ice_response(B b1)
        {
            test(b1 != null);
            test(b1.ice_id().equals("::Test::D1"));
            test(b1.sb.equals("D1.sb"));
            test(b1.pb != null);
            test(b1.pb != b1);
            D1 d1 = (D1)b1;
            test(d1 != null);
            test(d1.sd1.equals("D1.sd1"));
            test(d1.pd1 != null);
            test(d1.pd1 != b1);
            test(b1.pb == d1.pd1);

            B b2 = b1.pb;
            test(b2 != null);
            test(b2.pb == b1);
            test(b2.sb.equals("D2.sb"));
            test(b2.ice_id().equals("::Test::B"));
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
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

    private static class AMI_Test_D1AsD1I extends AMI_TestIntf_D1AsD1
    {
        public void
        ice_response(D1 d1)
        {
            test(d1 != null);
            test(d1.ice_id().equals("::Test::D1"));
            test(d1.sb.equals("D1.sb"));
            test(d1.pb != null);
            test(d1.pb != d1);

            B b2 = d1.pb;
            test(b2 != null);
            test(b2.ice_id().equals("::Test::B"));
            test(b2.sb.equals("D2.sb"));
            test(b2.pb == d1);
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
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

    private static class AMI_Test_D2AsBI extends AMI_TestIntf_D2AsB
    {
        public void
        ice_response(B b2)
        {
            test(b2 != null);
            test(b2.ice_id().equals("::Test::B"));
            test(b2.sb.equals("D2.sb"));
            test(b2.pb != null);
            test(b2.pb != b2);

            B b1 = b2.pb;
            test(b1 != null);
            test(b1.ice_id().equals("::Test::D1"));
            test(b1.sb.equals("D1.sb"));
            test(b1.pb == b2);
            D1 d1 = (D1)b1;
            test(d1 != null);
            test(d1.sd1.equals("D1.sd1"));
            test(d1.pd1 == b2);
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
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

    private static class AMI_Test_paramTest1I extends AMI_TestIntf_paramTest1
    {
        public void
        ice_response(B b1, B b2)
        {
            test(b1 != null);
            test(b1.ice_id().equals("::Test::D1"));
            test(b1.sb.equals("D1.sb"));
            test(b1.pb == b2);
            D1 d1 = (D1)b1;
            test(d1 != null);
            test(d1.sd1.equals("D1.sd1"));
            test(d1.pd1 == b2);

            test(b2 != null);
            test(b2.ice_id().equals("::Test::B"));      // No factory, must be sliced
            test(b2.sb.equals("D2.sb"));
            test(b2.pb == b1);
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
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

    private static class AMI_Test_paramTest2I extends AMI_TestIntf_paramTest2
    {
        public void
        ice_response(B b2, B b1)
        {
            test(b1 != null);
            test(b1.ice_id().equals("::Test::D1"));
            test(b1.sb.equals("D1.sb"));
            test(b1.pb == b2);
            D1 d1 = (D1)b1;
            test(d1 != null);
            test(d1.sd1.equals("D1.sd1"));
            test(d1.pd1 == b2);

            test(b2 != null);
            test(b2.ice_id().equals("::Test::B"));      // No factory, must be sliced
            test(b2.sb.equals("D2.sb"));
            test(b2.pb == b1);
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
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

    private static class AMI_Test_returnTest1I extends AMI_TestIntf_returnTest1
    {
        public void
        ice_response(B r, B p1, B p2)
        {
            test(r == p1);
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
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

    private static class AMI_Test_returnTest2I extends AMI_TestIntf_returnTest2
    {
        public void
        ice_response(B r, B p1, B p2)
        {
            test(r == p1);
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
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

    private static class AMI_Test_returnTest3I extends AMI_TestIntf_returnTest3
    {
        public void
        ice_response(B b)
        {
            r = b;
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
        {
            test(false);
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();

        public B r;
    }

    private static class AMI_Test_paramTest3I extends AMI_TestIntf_paramTest3
    {
        public void
        ice_response(B ret, B p1, B p2)
        {
            test(p1 != null);
            test(p1.sb.equals("D2.sb (p1 1)"));
            test(p1.pb == null);
            test(p1.ice_id().equals("::Test::B"));

            test(p2 != null);
            test(p2.sb.equals("D2.sb (p2 1)"));
            test(p2.pb == null);
            test(p2.ice_id().equals("::Test::B"));

            test(ret != null);
            test(ret.sb.equals("D1.sb (p2 2)"));
            test(ret.pb == null);
            test(ret.ice_id().equals("::Test::D1"));
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
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

    private static class AMI_Test_paramTest4I extends AMI_TestIntf_paramTest4
    {
        public void
        ice_response(B ret, B b)
        {
            test(b != null);
            test(b.sb.equals("D4.sb (1)"));
            test(b.pb == null);
            test(b.ice_id().equals("::Test::B"));

            test(ret != null);
            test(ret.sb.equals("B.sb (2)"));
            test(ret.pb == null);
            test(ret.ice_id().equals("::Test::B"));
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
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

    private static class AMI_Test_sequenceTestI extends AMI_TestIntf_sequenceTest
    {
        public void
        ice_response(SS ss)
        {
            r = ss;
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
        {
            test(false);
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();

        public SS r;
    }

    private static class AMI_Test_dictionaryTestI extends AMI_TestIntf_dictionaryTest
    {
        public void
        ice_response(java.util.Map r, java.util.Map bout)
        {
            this.r = r;
            this.bout = bout;
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
        {
            test(false);
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();

        public java.util.Map r;
        public java.util.Map bout;
    }

    private static class AMI_Test_throwBaseAsBaseI extends AMI_TestIntf_throwBaseAsBase
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
        {
            try
            {
                BaseException e = (BaseException)exc;
                test(e.ice_name().equals("Test::BaseException"));
                test(e.sbe.equals("sbe"));
                test(e.pb != null);
                test(e.pb.sb.equals("sb"));
                test(e.pb.pb == e.pb);
            }
            catch(Exception e)
            {
                test(false);
            }
            callback.called();
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_Test_throwDerivedAsBaseI extends AMI_TestIntf_throwDerivedAsBase
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
        {
            try
            {
                DerivedException e = (DerivedException)exc;
                test(e.ice_name().equals("Test::DerivedException"));
                test(e.sbe.equals("sbe"));
                test(e.pb != null);
                test(e.pb.sb.equals("sb1"));
                test(e.pb.pb == e.pb);
                test(e.sde.equals("sde1"));
                test(e.pd1 != null);
                test(e.pd1.sb.equals("sb2"));
                test(e.pd1.pb == e.pd1);
                test(e.pd1.sd1.equals("sd2"));
                test(e.pd1.pd1 == e.pd1);
            }
            catch(Exception e)
            {
                test(false);
            }
            callback.called();
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_Test_throwDerivedAsDerivedI extends AMI_TestIntf_throwDerivedAsDerived
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
        {
            try
            {
                DerivedException e = (DerivedException)exc;
                test(e.ice_name().equals("Test::DerivedException"));
                test(e.sbe.equals("sbe"));
                test(e.pb != null);
                test(e.pb.sb.equals("sb1"));
                test(e.pb.pb == e.pb);
                test(e.sde.equals("sde1"));
                test(e.pd1 != null);
                test(e.pd1.sb.equals("sb2"));
                test(e.pd1.pb == e.pd1);
                test(e.pd1.sd1.equals("sd2"));
                test(e.pd1.pd1 == e.pd1);
            }
            catch(Exception e)
            {
                test(false);
            }
            callback.called();
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_Test_throwUnknownDerivedAsBaseI extends AMI_TestIntf_throwUnknownDerivedAsBase
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
        {
            try
            {
                BaseException e = (BaseException)exc;
                test(e.ice_name().equals("Test::BaseException"));
                test(e.sbe.equals("sbe"));
                test(e.pb != null);
                test(e.pb.sb.equals("sb d2"));
                test(e.pb.pb == e.pb);
            }
            catch(Exception e)
            {
                test(false);
            }
            callback.called();
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_Test_useForwardI extends AMI_TestIntf_useForward
    {
        public void
        ice_response(Forward f)
        {
             test(f != null);
             callback.called();
        }

        public void
        ice_exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        ice_exception(Ice.UserException exc)
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

    public static TestIntfPrx
    allTests(Ice.Communicator communicator, boolean collocated)
    {
        System.out.print("testing stringToProxy... ");
        System.out.flush();
        String ref = "Test:default -p 12010 -t 10000";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        System.out.println("ok");

        System.out.print("testing checked cast... ");
        System.out.flush();
        TestIntfPrx test = TestIntfPrxHelper.checkedCast(base);
        test(test != null);
        test(test.equals(base));
        System.out.println("ok");

        System.out.print("base as Object... ");
        System.out.flush();
        {
            Ice.Object o;
            SBase sb = null;
            try
            {
                o = test.SBaseAsObject();
                test(o != null);
                test(o.ice_id().equals("::Test::SBase"));
                sb = (SBase)o;
            }
            catch(Exception ex)
            {
                test(false);
            }
            test(sb != null);
            test(sb.sb.equals("SBase.sb"));
        }
        System.out.println("ok");

        System.out.print("base as Object (AMI)... ");
        System.out.flush();
        {
            AMI_Test_SBaseAsObjectI cb = new AMI_Test_SBaseAsObjectI();
            test.SBaseAsObject_async(cb);
            test(cb.check());
        }
        System.out.println("ok");

        System.out.print("base as base... ");
        System.out.flush();
        {
            SBase sb;
            try
            {
                sb = test.SBaseAsSBase();
                test(sb.sb.equals("SBase.sb"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("base as base (AMI)... ");
        System.out.flush();
        {
            AMI_Test_SBaseAsSBaseI cb = new AMI_Test_SBaseAsSBaseI();
            test.SBaseAsSBase_async(cb);
            test(cb.check());
        }
        System.out.println("ok");

        System.out.print("base with known derived as base... ");
        System.out.flush();
        {
            SBase sb;
            SBSKnownDerived sbskd = null;
            try
            {
                sb = test.SBSKnownDerivedAsSBase();
                test(sb.sb.equals("SBSKnownDerived.sb"));
                sbskd = (SBSKnownDerived)sb;
            }
            catch(Exception ex)
            {
                test(false);
            }
            test(sbskd != null);
            test(sbskd.sbskd.equals("SBSKnownDerived.sbskd"));
        }
        System.out.println("ok");

        System.out.print("base with known derived as base (AMI)... ");
        System.out.flush();
        {
            AMI_Test_SBSKnownDerivedAsSBaseI cb = new AMI_Test_SBSKnownDerivedAsSBaseI();
            test.SBSKnownDerivedAsSBase_async(cb);
            test(cb.check());
        }
        System.out.println("ok");

        System.out.print("base with known derived as known derived... ");
        System.out.flush();
        {
            SBSKnownDerived sbskd;
            try
            {
                sbskd = test.SBSKnownDerivedAsSBSKnownDerived();
                test(sbskd.sbskd.equals("SBSKnownDerived.sbskd"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("base with known derived as known derived (AMI)... ");
        System.out.flush();
        {
            AMI_Test_SBSKnownDerivedAsSBSKnownDerivedI cb = new AMI_Test_SBSKnownDerivedAsSBSKnownDerivedI();
            test.SBSKnownDerivedAsSBSKnownDerived_async(cb);
            test(cb.check());
        }
        System.out.println("ok");

        System.out.print("base with unknown derived as base... ");
        System.out.flush();
        {
            SBase sb;
            try
            {
                sb = test.SBSUnknownDerivedAsSBase();
                test(sb.sb.equals("SBSUnknownDerived.sb"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("base with unknown derived as base (AMI)... ");
        System.out.flush();
        {
            AMI_Test_SBSUnknownDerivedAsSBaseI cb = new AMI_Test_SBSUnknownDerivedAsSBaseI();
            test.SBSUnknownDerivedAsSBase_async(cb);
            test(cb.check());
        }
        System.out.println("ok");

        System.out.print("unknown with Object as Object... ");
        System.out.flush();
        {
            Ice.Object o;
            try
            {
                o = test.SUnknownAsObject();
                test(false);
            }
            catch(Ice.NoObjectFactoryException ex)
            {
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("unknown with Object as Object (AMI)... ");
        System.out.flush();
        {
            AMI_Test_SUnknownAsObjectI cb = new AMI_Test_SUnknownAsObjectI();
            test.SUnknownAsObject_async(cb);
            test(cb.check());
        }
        System.out.println("ok");

        System.out.print("one-element cycle... ");
        System.out.flush();
        {
            try
            {
                B b = test.oneElementCycle();
                test(b != null);
                test(b.ice_id().equals("::Test::B"));
                test(b.sb.equals("B1.sb"));
                test(b.pb == b);
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("one-element cycle (AMI)... ");
        System.out.flush();
        {
            AMI_Test_oneElementCycleI cb = new AMI_Test_oneElementCycleI();
            test.oneElementCycle_async(cb);
            test(cb.check());
        }
        System.out.println("ok");

        System.out.print("two-element cycle... ");
        System.out.flush();
        {
            try
            {
                B b1 = test.twoElementCycle();
                test(b1 != null);
                test(b1.ice_id().equals("::Test::B"));
                test(b1.sb.equals("B1.sb"));

                B b2 = b1.pb;
                test(b2 != null);
                test(b2.ice_id().equals("::Test::B"));
                test(b2.sb.equals("B2.sb"));
                test(b2.pb == b1);
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("two-element cycle (AMI)... ");
        System.out.flush();
        {
            AMI_Test_twoElementCycleI cb = new AMI_Test_twoElementCycleI();
            test.twoElementCycle_async(cb);
            test(cb.check());
        }
        System.out.println("ok");

        System.out.print("known derived pointer slicing as base... ");
        System.out.flush();
        {
            try
            {
                B b1;
                b1 = test.D1AsB();
                test(b1 != null);
                test(b1.ice_id().equals("::Test::D1"));
                test(b1.sb.equals("D1.sb"));
                test(b1.pb != null);
                test(b1.pb != b1);
                D1 d1 = (D1)b1;
                test(d1 != null);
                test(d1.sd1.equals("D1.sd1"));
                test(d1.pd1 != null);
                test(d1.pd1 != b1);
                test(b1.pb == d1.pd1);

                B b2 = b1.pb;
                test(b2 != null);
                test(b2.pb == b1);
                test(b2.sb.equals("D2.sb"));
                test(b2.ice_id().equals("::Test::B"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("known derived pointer slicing as base (AMI)... ");
        System.out.flush();
        {
            AMI_Test_D1AsBI cb = new AMI_Test_D1AsBI();
            test.D1AsB_async(cb);
            test(cb.check());
        }
        System.out.println("ok");

        System.out.print("known derived pointer slicing as derived... ");
        System.out.flush();
        {
            try
            {
                D1 d1;
                d1 = test.D1AsD1();
                test(d1 != null);
                test(d1.ice_id().equals("::Test::D1"));
                test(d1.sb.equals("D1.sb"));
                test(d1.pb != null);
                test(d1.pb != d1);

                B b2 = d1.pb;
                test(b2 != null);
                test(b2.ice_id().equals("::Test::B"));
                test(b2.sb.equals("D2.sb"));
                test(b2.pb == d1);
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("known derived pointer slicing as derived (AMI)... ");
        System.out.flush();
        {
            AMI_Test_D1AsD1I cb = new AMI_Test_D1AsD1I();
            test.D1AsD1_async(cb);
            test(cb.check());
        }
        System.out.println("ok");

        System.out.print("unknown derived pointer slicing as base... ");
        System.out.flush();
        {
            try
            {
                B b2;
                b2 = test.D2AsB();
                test(b2 != null);
                test(b2.ice_id().equals("::Test::B"));
                test(b2.sb.equals("D2.sb"));
                test(b2.pb != null);
                test(b2.pb != b2);

                B b1 = b2.pb;
                test(b1 != null);
                test(b1.ice_id().equals("::Test::D1"));
                test(b1.sb.equals("D1.sb"));
                test(b1.pb == b2);
                D1 d1 = (D1)b1;
                test(d1 != null);
                test(d1.sd1.equals("D1.sd1"));
                test(d1.pd1 == b2);
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("unknown derived pointer slicing as base (AMI)... ");
        System.out.flush();
        {
            AMI_Test_D2AsBI cb = new AMI_Test_D2AsBI();
            test.D2AsB_async(cb);
            test(cb.check());
        }
        System.out.println("ok");

        System.out.print("param ptr slicing with known first... ");
        System.out.flush();
        {
            try
            {
                BHolder b1 = new BHolder();
                BHolder b2 = new BHolder();
                test.paramTest1(b1, b2);

                test(b1.value != null);
                test(b1.value.ice_id().equals("::Test::D1"));
                test(b1.value.sb.equals("D1.sb"));
                test(b1.value.pb == b2.value);
                D1 d1 = (D1)b1.value;
                test(d1 != null);
                test(d1.sd1.equals("D1.sd1"));
                test(d1.pd1 == b2.value);

                test(b2.value != null);
                test(b2.value.ice_id().equals("::Test::B"));    // No factory, must be sliced
                test(b2.value.sb.equals("D2.sb"));
                test(b2.value.pb == b1.value);
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("param ptr slicing with known first (AMI)... ");
        System.out.flush();
        {
            AMI_Test_paramTest1I cb = new AMI_Test_paramTest1I();
            test.paramTest1_async(cb);
            test(cb.check());
        }
        System.out.println("ok");

        System.out.print("param ptr slicing with unknown first... ");
        System.out.flush();
        {
            try
            {
                BHolder b2 = new BHolder();
                BHolder b1 = new BHolder();
                test.paramTest2(b2, b1);

                test(b1.value != null);
                test(b1.value.ice_id().equals("::Test::D1"));
                test(b1.value.sb.equals("D1.sb"));
                test(b1.value.pb == b2.value);
                D1 d1 = (D1)b1.value;
                test(d1 != null);
                test(d1.sd1.equals("D1.sd1"));
                test(d1.pd1 == b2.value);

                test(b2.value != null);
                test(b2.value.ice_id().equals("::Test::B"));    // No factory, must be sliced
                test(b2.value.sb.equals("D2.sb"));
                test(b2.value.pb == b1.value);
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("param ptr slicing with unknown first (AMI)... ");
        System.out.flush();
        {
            AMI_Test_paramTest2I cb = new AMI_Test_paramTest2I();
            test.paramTest2_async(cb);
            test(cb.check());
        }
        System.out.println("ok");

        System.out.print("return value identity with known first... ");
        System.out.flush();
        {
            try
            {
                BHolder p1 = new BHolder();
                BHolder p2 = new BHolder();
                B r = test.returnTest1(p1, p2);
                test(r == p1.value);
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("return value identity with known first (AMI)... ");
        System.out.flush();
        {
            AMI_Test_returnTest1I cb = new AMI_Test_returnTest1I();
            test.returnTest1_async(cb);
            test(cb.check());
        }
        System.out.println("ok");

        System.out.print("return value identity with unknown first... ");
        System.out.flush();
        {
            try
            {
                BHolder p1 = new BHolder();
                BHolder p2 = new BHolder();
                B r = test.returnTest2(p1, p2);
                test(r == p1.value);
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("return value identity with unknown first (AMI)... ");
        System.out.flush();
        {
            AMI_Test_returnTest2I cb = new AMI_Test_returnTest2I();
            test.returnTest2_async(cb);
            test(cb.check());
        }
        System.out.println("ok");

        System.out.print("return value identity for input params known first... ");
        System.out.flush();
        {
            try
            {
                D1 d1 = new D1();
                d1.sb = "D1.sb";
                d1.sd1 = "D1.sd1";
                D3 d3 = new D3();
                d3.pb = d1;
                d3.sb = "D3.sb";
                d3.sd3 = "D3.sd3";
                d3.pd3 = d1;
                d1.pb = d3;
                d1.pd1 = d3;

                B b1 = test.returnTest3(d1, d3);

                test(b1 != null);
                test(b1.sb.equals("D1.sb"));
                test(b1.ice_id().equals("::Test::D1"));
                D1 p1 = (D1)b1;
                test(p1 != null);
                test(p1.sd1.equals("D1.sd1"));
                test(p1.pd1 == b1.pb);

                B b2 = b1.pb;
                test(b2 != null);
                test(b2.sb.equals("D3.sb"));
                test(b2.ice_id().equals("::Test::B"));  // Sliced by server
                test(b2.pb == b1);
                try
                {
                    D3 p3 = (D3)b2;
                    test(false);
                }
                catch(ClassCastException ex)
                {
                }

                test(b1 != d1);
                test(b1 != d3);
                test(b2 != d1);
                test(b2 != d3);
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("return value identity for input params known first (AMI)... ");
        System.out.flush();
        {
            D1 d1 = new D1();
            d1.sb = "D1.sb";
            d1.sd1 = "D1.sd1";
            D3 d3 = new D3();
            d3.pb = d1;
            d3.sb = "D3.sb";
            d3.sd3 = "D3.sd3";
            d3.pd3 = d1;
            d1.pb = d3;
            d1.pd1 = d3;

            AMI_Test_returnTest3I cb = new AMI_Test_returnTest3I();
            test.returnTest3_async(cb, d1, d3);
            test(cb.check());
            B b1 = cb.r;

            test(b1 != null);
            test(b1.sb.equals("D1.sb"));
            test(b1.ice_id().equals("::Test::D1"));
            D1 p1 = (D1)b1;
            test(p1 != null);
            test(p1.sd1.equals("D1.sd1"));
            test(p1.pd1 == b1.pb);

            B b2 = b1.pb;
            test(b2 != null);
            test(b2.sb.equals("D3.sb"));
            test(b2.ice_id().equals("::Test::B"));      // Sliced by server
            test(b2.pb == b1);
            try
            {
                D3 p3 = (D3)b2;
                test(false);
            }
            catch(ClassCastException ex)
            {
            }

            test(b1 != d1);
            test(b1 != d3);
            test(b2 != d1);
            test(b2 != d3);
        }
        System.out.println("ok");

        System.out.print("return value identity for input params unknown first... ");
        System.out.flush();
        {
            try
            {
                D1 d1 = new D1();
                d1.sb = "D1.sb";
                d1.sd1 = "D1.sd1";
                D3 d3 = new D3();
                d3.pb = d1;
                d3.sb = "D3.sb";
                d3.sd3 = "D3.sd3";
                d3.pd3 = d1;
                d1.pb = d3;
                d1.pd1 = d3;

                B b1 = test.returnTest3(d3, d1);

                test(b1 != null);
                test(b1.sb.equals("D3.sb"));
                test(b1.ice_id().equals("::Test::B"));  // Sliced by server

                try
                {
                    D3 p1 = (D3)b1;
                    test(false);
                }
                catch(ClassCastException ex)
                {
                }

                B b2 = b1.pb;
                test(b2 != null);
                test(b2.sb.equals("D1.sb"));
                test(b2.ice_id().equals("::Test::D1"));
                test(b2.pb == b1);
                D1 p3 = (D1)b2;
                test(p3 != null);
                test(p3.sd1.equals("D1.sd1"));
                test(p3.pd1 == b1);

                test(b1 != d1);
                test(b1 != d3);
                test(b2 != d1);
                test(b2 != d3);
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("return value identity for input params unknown first (AMI)... ");
        System.out.flush();
        {
            D1 d1 = new D1();
            d1.sb = "D1.sb";
            d1.sd1 = "D1.sd1";
            D3 d3 = new D3();
            d3.pb = d1;
            d3.sb = "D3.sb";
            d3.sd3 = "D3.sd3";
            d3.pd3 = d1;
            d1.pb = d3;
            d1.pd1 = d3;

            AMI_Test_returnTest3I cb = new AMI_Test_returnTest3I();
            test.returnTest3_async(cb, d3, d1);
            test(cb.check());
            B b1 = cb.r;

            test(b1 != null);
            test(b1.sb.equals("D3.sb"));
            test(b1.ice_id().equals("::Test::B"));      // Sliced by server

            try
            {
                D3 p1 = (D3)b1;
                test(false);
            }
            catch(ClassCastException ex)
            {
            }

            B b2 = b1.pb;
            test(b2 != null);
            test(b2.sb.equals("D1.sb"));
            test(b2.ice_id().equals("::Test::D1"));
            test(b2.pb == b1);
            D1 p3 = (D1)b2;
            test(p3 != null);
            test(p3.sd1.equals("D1.sd1"));
            test(p3.pd1 == b1);

            test(b1 != d1);
            test(b1 != d3);
            test(b2 != d1);
            test(b2 != d3);
        }
        System.out.println("ok");

        System.out.print("remainder unmarshaling (3 instances)... ");
        System.out.flush();
        {
            try
            {
                BHolder p1 = new BHolder();
                BHolder p2 = new BHolder();
                B ret = test.paramTest3(p1, p2);

                test(p1.value != null);
                test(p1.value.sb.equals("D2.sb (p1 1)"));
                test(p1.value.pb == null);
                test(p1.value.ice_id().equals("::Test::B"));

                test(p2.value != null);
                test(p2.value.sb.equals("D2.sb (p2 1)"));
                test(p2.value.pb == null);
                test(p2.value.ice_id().equals("::Test::B"));

                test(ret != null);
                test(ret.sb.equals("D1.sb (p2 2)"));
                test(ret.pb == null);
                test(ret.ice_id().equals("::Test::D1"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("remainder unmarshaling (3 instances) (AMI)... ");
        System.out.flush();
        {
            AMI_Test_paramTest3I cb = new AMI_Test_paramTest3I();
            test.paramTest3_async(cb);
            test(cb.check());
        }
        System.out.println("ok");

        System.out.print("remainder unmarshaling (4 instances)... ");
        System.out.flush();
        {
            try
            {
                BHolder b = new BHolder();
                B ret = test.paramTest4(b);

                test(b.value != null);
                test(b.value.sb.equals("D4.sb (1)"));
                test(b.value.pb == null);
                test(b.value.ice_id().equals("::Test::B"));

                test(ret != null);
                test(ret.sb.equals("B.sb (2)"));
                test(ret.pb == null);
                test(ret.ice_id().equals("::Test::B"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("remainder unmarshaling (4 instances) (AMI)... ");
        System.out.flush();
        {
            AMI_Test_paramTest4I cb = new AMI_Test_paramTest4I();
            test.paramTest4_async(cb);
            test(cb.check());
        }
        System.out.println("ok");

        System.out.print("param ptr slicing, instance marshaled in unknown derived as base... ");
        System.out.flush();
        {
            try
            {
                B b1 = new B();
                b1.sb = "B.sb(1)";
                b1.pb = b1;

                D3 d3 = new D3();
                d3.sb = "D3.sb";
                d3.pb = d3;
                d3.sd3 = "D3.sd3";
                d3.pd3 = b1;

                B b2 = new B();
                b2.sb = "B.sb(2)";
                b2.pb = b1;

                B r = test.returnTest3(d3, b2);

                test(r != null);
                test(r.ice_id().equals("::Test::B"));
                test(r.sb.equals("D3.sb"));
                test(r.pb == r);
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("param ptr slicing, instance marshaled in unknown derived as base (AMI)... ");
        System.out.flush();
        {
            B b1 = new B();
            b1.sb = "B.sb(1)";
            b1.pb = b1;

            D3 d3 = new D3();
            d3.sb = "D3.sb";
            d3.pb = d3;
            d3.sd3 = "D3.sd3";
            d3.pd3 = b1;

            B b2 = new B();
            b2.sb = "B.sb(2)";
            b2.pb = b1;

            AMI_Test_returnTest3I cb = new AMI_Test_returnTest3I();
            test.returnTest3_async(cb, d3, b2);
            test(cb.check());
            B r = cb.r;

            test(r != null);
            test(r.ice_id().equals("::Test::B"));
            test(r.sb.equals("D3.sb"));
            test(r.pb == r);
        }
        System.out.println("ok");

        System.out.print("param ptr slicing, instance marshaled in unknown derived as derived... ");
        System.out.flush();
        {
            try
            {
                D1 d11 = new D1();
                d11.sb = "D1.sb(1)";
                d11.pb = d11;
                d11.sd1 = "D1.sd1(1)";

                D3 d3 = new D3();
                d3.sb = "D3.sb";
                d3.pb = d3;
                d3.sd3 = "D3.sd3";
                d3.pd3 = d11;

                D1 d12 = new D1();
                d12.sb = "D1.sb(2)";
                d12.pb = d12;
                d12.sd1 = "D1.sd1(2)";
                d12.pd1 = d11;

                B r = test.returnTest3(d3, d12);
                test(r != null);
                test(r.ice_id().equals("::Test::B"));
                test(r.sb.equals("D3.sb"));
                test(r.pb == r);
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("param ptr slicing, instance marshaled in unknown derived as derived (AMI)... ");
        System.out.flush();
        {
            D1 d11 = new D1();
            d11.sb = "D1.sb(1)";
            d11.pb = d11;
            d11.sd1 = "D1.sd1(1)";

            D3 d3 = new D3();
            d3.sb = "D3.sb";
            d3.pb = d3;
            d3.sd3 = "D3.sd3";
            d3.pd3 = d11;

            D1 d12 = new D1();
            d12.sb = "D1.sb(2)";
            d12.pb = d12;
            d12.sd1 = "D1.sd1(2)";
            d12.pd1 = d11;

            AMI_Test_returnTest3I cb = new AMI_Test_returnTest3I();
            test.returnTest3_async(cb, d3, d12);
            test(cb.check());
            B r = cb.r;

            test(r != null);
            test(r.ice_id().equals("::Test::B"));
            test(r.sb.equals("D3.sb"));
            test(r.pb == r);
        }
        System.out.println("ok");

        System.out.print("sequence slicing... ");
        System.out.flush();
        {
            try
            {
                SS ss;
                {
                    B ss1b = new B();
                    ss1b.sb = "B.sb";
                    ss1b.pb = ss1b;

                    D1 ss1d1 = new D1();
                    ss1d1.sb = "D1.sb";
                    ss1d1.sd1 = "D1.sd1";
                    ss1d1.pb = ss1b;

                    D3 ss1d3 = new D3();
                    ss1d3.sb = "D3.sb";
                    ss1d3.sd3 = "D3.sd3";
                    ss1d3.pb = ss1b;

                    B ss2b = new B();
                    ss2b.sb = "B.sb";
                    ss2b.pb = ss1b;

                    D1 ss2d1 = new D1();
                    ss2d1.sb = "D1.sb";
                    ss2d1.sd1 = "D1.sd1";
                    ss2d1.pb = ss2b;

                    D3 ss2d3 = new D3();
                    ss2d3.sb = "D3.sb";
                    ss2d3.sd3 = "D3.sd3";
                    ss2d3.pb = ss2b;

                    ss1d1.pd1 = ss2b;
                    ss1d3.pd3 = ss2d1;

                    ss2d1.pd1 = ss1d3;
                    ss2d3.pd3 = ss1d1;

                    SS1 ss1 = new SS1();
                    ss1.s = new B[3];
                    ss1.s[0] = ss1b;
                    ss1.s[1] = ss1d1;
                    ss1.s[2] = ss1d3;

                    SS2 ss2 = new SS2();
                    ss2.s = new B[3];
                    ss2.s[0] = ss2b;
                    ss2.s[1] = ss2d1;
                    ss2.s[2] = ss2d3;

                    ss = test.sequenceTest(ss1, ss2);
                }

                test(ss.c1 != null);
                B ss1b = ss.c1.s[0];
                B ss1d1 = ss.c1.s[1];
                test(ss.c2 != null);
                B ss1d3 = ss.c1.s[2];

                test(ss.c2 != null);
                B ss2b = ss.c2.s[0];
                B ss2d1 = ss.c2.s[1];
                B ss2d3 = ss.c2.s[2];

                test(ss1b.pb == ss1b);
                test(ss1d1.pb == ss1b);
                test(ss1d3.pb == ss1b);

                test(ss2b.pb == ss1b);
                test(ss2d1.pb == ss2b);
                test(ss2d3.pb == ss2b);

                test(ss1b.ice_id().equals("::Test::B"));
                test(ss1d1.ice_id().equals("::Test::D1"));
                test(ss1d3.ice_id().equals("::Test::B"));

                test(ss2b.ice_id().equals("::Test::B"));
                test(ss2d1.ice_id().equals("::Test::D1"));
                test(ss2d3.ice_id().equals("::Test::B"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("sequence slicing (AMI)... ");
        System.out.flush();
        {
            SS ss;
            {
                B ss1b = new B();
                ss1b.sb = "B.sb";
                ss1b.pb = ss1b;

                D1 ss1d1 = new D1();
                ss1d1.sb = "D1.sb";
                ss1d1.sd1 = "D1.sd1";
                ss1d1.pb = ss1b;

                D3 ss1d3 = new D3();
                ss1d3.sb = "D3.sb";
                ss1d3.sd3 = "D3.sd3";
                ss1d3.pb = ss1b;

                B ss2b = new B();
                ss2b.sb = "B.sb";
                ss2b.pb = ss1b;

                D1 ss2d1 = new D1();
                ss2d1.sb = "D1.sb";
                ss2d1.sd1 = "D1.sd1";
                ss2d1.pb = ss2b;

                D3 ss2d3 = new D3();
                ss2d3.sb = "D3.sb";
                ss2d3.sd3 = "D3.sd3";
                ss2d3.pb = ss2b;

                ss1d1.pd1 = ss2b;
                ss1d3.pd3 = ss2d1;

                ss2d1.pd1 = ss1d3;
                ss2d3.pd3 = ss1d1;

                SS1 ss1 = new SS1();
                ss1.s = new B[3];
                ss1.s[0] = ss1b;
                ss1.s[1] = ss1d1;
                ss1.s[2] = ss1d3;

                SS2 ss2 = new SS2();
                ss2.s = new B[3];
                ss2.s[0] = ss2b;
                ss2.s[1] = ss2d1;
                ss2.s[2] = ss2d3;

                AMI_Test_sequenceTestI cb = new AMI_Test_sequenceTestI();
                test.sequenceTest_async(cb, ss1, ss2);
                test(cb.check());
                ss = cb.r;
            }
            test(ss.c1 != null);
            B ss1b = ss.c1.s[0];
            B ss1d1 = ss.c1.s[1];
            test(ss.c2 != null);
            B ss1d3 = ss.c1.s[2];

            test(ss.c2 != null);
            B ss2b = ss.c2.s[0];
            B ss2d1 = ss.c2.s[1];
            B ss2d3 = ss.c2.s[2];

            test(ss1b.pb == ss1b);
            test(ss1d1.pb == ss1b);
            test(ss1d3.pb == ss1b);

            test(ss2b.pb == ss1b);
            test(ss2d1.pb == ss2b);
            test(ss2d3.pb == ss2b);

            test(ss1b.ice_id().equals("::Test::B"));
            test(ss1d1.ice_id().equals("::Test::D1"));
            test(ss1d3.ice_id().equals("::Test::B"));

            test(ss2b.ice_id().equals("::Test::B"));
            test(ss2d1.ice_id().equals("::Test::D1"));
            test(ss2d3.ice_id().equals("::Test::B"));
        }
        System.out.println("ok");

        System.out.print("dictionary slicing... ");
        System.out.flush();
        {
            try
            {
                java.util.IdentityHashMap bin = new java.util.IdentityHashMap();
                BDictHolder bout = new BDictHolder();
                java.util.Map r;
                int i;
                for(i = 0; i < 10; ++i)
                {
                    String s = "D1." + new Integer(i).toString();
                    D1 d1 = new D1();
                    d1.sb = s;
                    d1.pb = d1;
                    d1.sd1 = s;
                    bin.put(new Integer(i), d1);
                }

                r = test.dictionaryTest(bin, bout);

                test(bout.value.size() == 10);
                for(i = 0; i < 10; ++i)
                {
                    B b = (B)bout.value.get(new Integer(i * 10));
                    test(b != null);
                    String s = "D1." + new Integer(i).toString();
                    test(b.sb.equals(s));
                    test(b.pb != null);
                    test(b.pb != b);
                    test(b.pb.sb.equals(s));
                    test(b.pb.pb == b.pb);
                }

                test(r.size() == 10);
                for(i = 0; i < 10; ++i)
                {
                    B b = (B)r.get(new Integer(i * 20));
                    test(b != null);
                    String s = "D1." + new Integer(i * 20).toString();
                    test(b.sb.equals(s));
                    test(b.pb == (i == 0 ? (B)null : (B)r.get(new Integer((i - 1) * 20))));
                    D1 d1 = (D1)b;
                    test(d1 != null);
                    test(d1.sd1.equals(s));
                    test(d1.pd1 == d1);
                }
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("dictionary slicing (AMI)... ");
        System.out.flush();
        {
            java.util.Map bin = new java.util.HashMap();
            java.util.Map bout;
            java.util.Map r;
            int i;
            for(i = 0; i < 10; ++i)
            {
                String s = "D1." + new Integer(i).toString();
                D1 d1 = new D1();
                d1.sb = s;
                d1.pb = d1;
                d1.sd1 = s;
                bin.put(new Integer(i), d1);
            }

            AMI_Test_dictionaryTestI cb = new AMI_Test_dictionaryTestI();
            test.dictionaryTest_async(cb, bin);
            test(cb.check());
            bout = cb.bout;
            r = cb.r;

            test(bout.size() == 10);
            for(i = 0; i < 10; ++i)
            {
                B b = (B)bout.get(new Integer(i * 10));
                test(b != null);
                String s = "D1." + new Integer(i).toString();
                test(b.sb.equals(s));
                test(b.pb != null);
                test(b.pb != b);
                test(b.pb.sb.equals(s));
                test(b.pb.pb == b.pb);
            }

            test(r.size() == 10);
            for(i = 0; i < 10; ++i)
            {
                B b = (B)r.get(new Integer(i * 20));
                test(b != null);
                String s = "D1." + new Integer(i * 20).toString();
                test(b.sb.equals(s));
                test(b.pb == (i == 0 ? (B)null : (B)r.get(new Integer((i - 1) * 20))));
                D1 d1 = (D1)b;
                test(d1 != null);
                test(d1.sd1.equals(s));
                test(d1.pd1 == d1);
            }
        }
        System.out.println("ok");

        System.out.print("base exception thrown as base exception... ");
        System.out.flush();
        {
            try
            {
                test.throwBaseAsBase();
                test(false);
            }
            catch(BaseException e)
            {
                test(e.ice_name().equals("Test::BaseException"));
                test(e.sbe.equals("sbe"));
                test(e.pb != null);
                test(e.pb.sb.equals("sb"));
                test(e.pb.pb == e.pb);
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("base exception thrown as base exception (AMI)... ");
        System.out.flush();
        {
            AMI_Test_throwBaseAsBaseI cb = new AMI_Test_throwBaseAsBaseI();
            test.throwBaseAsBase_async(cb);
            test(cb.check());
        }
        System.out.println("ok");

        System.out.print("derived exception thrown as base exception... ");
        System.out.flush();
        {
            try
            {
                test.throwDerivedAsBase();
                test(false);
            }
            catch(DerivedException e)
            {
                test(e.ice_name().equals("Test::DerivedException"));
                test(e.sbe.equals("sbe"));
                test(e.pb != null);
                test(e.pb.sb.equals("sb1"));
                test(e.pb.pb == e.pb);
                test(e.sde.equals("sde1"));
                test(e.pd1 != null);
                test(e.pd1.sb.equals("sb2"));
                test(e.pd1.pb == e.pd1);
                test(e.pd1.sd1.equals("sd2"));
                test(e.pd1.pd1 == e.pd1);
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("derived exception thrown as base exception (AMI)... ");
        System.out.flush();
        {
            AMI_Test_throwDerivedAsBaseI cb = new AMI_Test_throwDerivedAsBaseI();
            test.throwDerivedAsBase_async(cb);
            test(cb.check());
        }
        System.out.println("ok");

        System.out.print("derived exception thrown as derived exception... ");
        System.out.flush();
        {
            try
            {
                test.throwDerivedAsDerived();
                test(false);
            }
            catch(DerivedException e)
            {
                test(e.ice_name().equals("Test::DerivedException"));
                test(e.sbe.equals("sbe"));
                test(e.pb != null);
                test(e.pb.sb.equals("sb1"));
                test(e.pb.pb == e.pb);
                test(e.sde.equals("sde1"));
                test(e.pd1 != null);
                test(e.pd1.sb.equals("sb2"));
                test(e.pd1.pb == e.pd1);
                test(e.pd1.sd1.equals("sd2"));
                test(e.pd1.pd1 == e.pd1);
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("derived exception thrown as derived exception (AMI)... ");
        System.out.flush();
        {
            AMI_Test_throwDerivedAsDerivedI cb = new AMI_Test_throwDerivedAsDerivedI();
            test.throwDerivedAsDerived_async(cb);
            test(cb.check());
        }
        System.out.println("ok");

        System.out.print("unknown derived exception thrown as base exception... ");
        System.out.flush();
        {
            try
            {
                test.throwUnknownDerivedAsBase();
                test(false);
            }
            catch(BaseException e)
            {
                test(e.ice_name().equals("Test::BaseException"));
                test(e.sbe.equals("sbe"));
                test(e.pb != null);
                test(e.pb.sb.equals("sb d2"));
                test(e.pb.pb == e.pb);
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("unknown derived exception thrown as base exception (AMI)... ");
        System.out.flush();
        {
            AMI_Test_throwUnknownDerivedAsBaseI cb = new AMI_Test_throwUnknownDerivedAsBaseI();
            test.throwUnknownDerivedAsBase_async(cb);
            test(cb.check());
        }
        System.out.println("ok");

        System.out.print("forward-declared class... ");
        System.out.flush();
        {
            try
            {
                ForwardHolder f = new ForwardHolder();
                test.useForward(f);
                test(f.value != null);
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        System.out.println("ok");

        System.out.print("forward-declared class (AMI)... ");
        System.out.flush();
        {
            AMI_Test_useForwardI cb = new AMI_Test_useForwardI();
            test.useForward_async(cb);
            test(cb.check());
        }
        System.out.println("ok");

        return test;
    }
}
