// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.slicing.objects;

import java.io.PrintStream;
import java.io.PrintWriter;

import test.Ice.slicing.objects.client.Test.Callback_TestIntf_D1AsB;
import test.Ice.slicing.objects.client.Test.Callback_TestIntf_D1AsD1;
import test.Ice.slicing.objects.client.Test.Callback_TestIntf_D2AsB;
import test.Ice.slicing.objects.client.Test.Callback_TestIntf_SBSKnownDerivedAsSBSKnownDerived;
import test.Ice.slicing.objects.client.Test.Callback_TestIntf_SBSKnownDerivedAsSBase;
import test.Ice.slicing.objects.client.Test.Callback_TestIntf_SBSUnknownDerivedAsSBase;
import test.Ice.slicing.objects.client.Test.Callback_TestIntf_SBaseAsObject;
import test.Ice.slicing.objects.client.Test.Callback_TestIntf_SBaseAsSBase;
import test.Ice.slicing.objects.client.Test.Callback_TestIntf_SUnknownAsObject;
import test.Ice.slicing.objects.client.Test.Callback_TestIntf_dictionaryTest;
import test.Ice.slicing.objects.client.Test.Callback_TestIntf_oneElementCycle;
import test.Ice.slicing.objects.client.Test.Callback_TestIntf_paramTest1;
import test.Ice.slicing.objects.client.Test.Callback_TestIntf_paramTest2;
import test.Ice.slicing.objects.client.Test.Callback_TestIntf_paramTest3;
import test.Ice.slicing.objects.client.Test.Callback_TestIntf_paramTest4;
import test.Ice.slicing.objects.client.Test.Callback_TestIntf_returnTest1;
import test.Ice.slicing.objects.client.Test.Callback_TestIntf_returnTest2;
import test.Ice.slicing.objects.client.Test.Callback_TestIntf_returnTest3;
import test.Ice.slicing.objects.client.Test.Callback_TestIntf_sequenceTest;
import test.Ice.slicing.objects.client.Test.Callback_TestIntf_throwBaseAsBase;
import test.Ice.slicing.objects.client.Test.Callback_TestIntf_throwDerivedAsBase;
import test.Ice.slicing.objects.client.Test.Callback_TestIntf_throwDerivedAsDerived;
import test.Ice.slicing.objects.client.Test.Callback_TestIntf_throwUnknownDerivedAsBase;
import test.Ice.slicing.objects.client.Test.Callback_TestIntf_twoElementCycle;
import test.Ice.slicing.objects.client.Test.Callback_TestIntf_useForward;
import test.Ice.slicing.objects.client.Test.B;
import test.Ice.slicing.objects.client.Test.BDictHolder;
import test.Ice.slicing.objects.client.Test.BHolder;
import test.Ice.slicing.objects.client.Test.BaseException;
import test.Ice.slicing.objects.client.Test.D1;
import test.Ice.slicing.objects.client.Test.D3;
import test.Ice.slicing.objects.client.Test.DerivedException;
import test.Ice.slicing.objects.client.Test.SBSKnownDerived;
import test.Ice.slicing.objects.client.Test.SBase;
import test.Ice.slicing.objects.client.Test.SS;
import test.Ice.slicing.objects.client.Test.SS1;
import test.Ice.slicing.objects.client.Test.SS2;
import test.Ice.slicing.objects.client.Test.TestIntfPrx;
import test.Ice.slicing.objects.client.Test.TestIntfPrxHelper;
import test.Ice.slicing.objects.client.Test.Forward;
import test.Ice.slicing.objects.client.Test.ForwardHolder;

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

    private static class Callback_TestIntf_SBaseAsObjectI extends Callback_TestIntf_SBaseAsObject
    {
        public void
        response(Ice.Object o)
        {
            test(o != null);
            test(o.ice_id().equals("::Test::SBase"));
            SBase sb = (SBase)o;
            test(sb != null);
            test(sb.sb.equals("SBase.sb"));
            callback.called();
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
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

    private static class Callback_TestIntf_SBaseAsSBaseI extends Callback_TestIntf_SBaseAsSBase
    {
        public void
        response(SBase sb)
        {
            test(sb.sb.equals("SBase.sb"));
            callback.called();
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
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

    private static class Callback_TestIntf_SBSKnownDerivedAsSBaseI extends Callback_TestIntf_SBSKnownDerivedAsSBase
    {
        public void
        response(SBase sb)
        {
            test(sb.sb.equals("SBSKnownDerived.sb"));
            SBSKnownDerived sbskd = (SBSKnownDerived)sb;
            test(sbskd != null);
            test(sbskd.sbskd.equals("SBSKnownDerived.sbskd"));
            callback.called();
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
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

    private static class Callback_TestIntf_SBSKnownDerivedAsSBSKnownDerivedI
        extends Callback_TestIntf_SBSKnownDerivedAsSBSKnownDerived
    {
        public void
        response(SBSKnownDerived sbskd)
        {
            test(sbskd.sbskd.equals("SBSKnownDerived.sbskd"));
            callback.called();
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
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

    private static class Callback_TestIntf_SBSUnknownDerivedAsSBaseI extends Callback_TestIntf_SBSUnknownDerivedAsSBase
    {
        public void
        response(SBase sb)
        {
            test(sb.sb.equals("SBSUnknownDerived.sb"));
            callback.called();
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
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

    private static class Callback_TestIntf_SUnknownAsObjectI extends Callback_TestIntf_SUnknownAsObject
    {
        public void
        response(Ice.Object o)
        {
            test(false);
        }

        public void
        exception(Ice.LocalException exc)
        {

            test(exc.ice_name().equals("Ice::NoObjectFactoryException"));
            callback.called();
        }

        public void
        exception(Ice.UserException exc)
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

    private static class Callback_TestIntf_oneElementCycleI extends Callback_TestIntf_oneElementCycle
    {
        public void
        response(B b)
        {
            test(b != null);
            test(b.ice_id().equals("::Test::B"));
            test(b.sb.equals("B1.sb"));
            test(b.pb == b);
            callback.called();
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
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

    private static class Callback_TestIntf_twoElementCycleI extends Callback_TestIntf_twoElementCycle
    {
        public void
        response(B b1)
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
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
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

    private static class Callback_TestIntf_D1AsBI extends Callback_TestIntf_D1AsB
    {
        public void
        response(B b1)
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
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
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

    private static class Callback_TestIntf_D1AsD1I extends Callback_TestIntf_D1AsD1
    {
        public void
        response(D1 d1)
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
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
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

    private static class Callback_TestIntf_D2AsBI extends Callback_TestIntf_D2AsB
    {
        public void
        response(B b2)
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
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
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

    private static class Callback_TestIntf_paramTest1I extends Callback_TestIntf_paramTest1
    {
        public void
        response(B b1, B b2)
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
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
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

    private static class Callback_TestIntf_paramTest2I extends Callback_TestIntf_paramTest2
    {
        public void
        response(B b2, B b1)
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
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
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

    private static class Callback_TestIntf_returnTest1I extends Callback_TestIntf_returnTest1
    {
        public void
        response(B r, B p1, B p2)
        {
            test(r == p1);
            callback.called();
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
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

    private static class Callback_TestIntf_returnTest2I extends Callback_TestIntf_returnTest2
    {
        public void
        response(B r, B p1, B p2)
        {
            test(r == p1);
            callback.called();
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
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

    private static class Callback_TestIntf_returnTest3I extends Callback_TestIntf_returnTest3
    {
        public void
        response(B b)
        {
            r = b;
            callback.called();
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
        {
            test(false);
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();

        public B r;
    }

    private static class Callback_TestIntf_paramTest3I extends Callback_TestIntf_paramTest3
    {
        public void
        response(B ret, B p1, B p2)
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
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
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

    private static class Callback_TestIntf_paramTest4I extends Callback_TestIntf_paramTest4
    {
        public void
        response(B ret, B b)
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
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
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

    private static class Callback_TestIntf_sequenceTestI extends Callback_TestIntf_sequenceTest
    {
        public void
        response(SS ss)
        {
            r = ss;
            callback.called();
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
        {
            test(false);
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();

        public SS r;
    }

    private static class Callback_TestIntf_dictionaryTestI extends Callback_TestIntf_dictionaryTest
    {
        public void
        response(java.util.Map<Integer, B> r, java.util.Map<Integer, B> bout)
        {
            this.r = r;
            this.bout = bout;
            callback.called();
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
        {
            test(false);
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();

        public java.util.Map<Integer, B> r;
        public java.util.Map<Integer, B> bout;
    }

    private static class Callback_TestIntf_throwBaseAsBaseI extends Callback_TestIntf_throwBaseAsBase
    {
        public void
        response()
        {
            test(false);
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
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

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class Callback_TestIntf_throwDerivedAsBaseI extends Callback_TestIntf_throwDerivedAsBase
    {
        public void
        response()
        {
            test(false);
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
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

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class Callback_TestIntf_throwDerivedAsDerivedI extends Callback_TestIntf_throwDerivedAsDerived
    {
        public void
        response()
        {
            test(false);
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
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

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class Callback_TestIntf_throwUnknownDerivedAsBaseI 
        extends Callback_TestIntf_throwUnknownDerivedAsBase
    {
        public void
        response()
        {
            test(false);
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
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

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class Callback_TestIntf_useForwardI extends Callback_TestIntf_useForward
    {
        public void
        response(Forward f)
        {
             test(f != null);
             callback.called();
        }

        public void
        exception(Ice.LocalException exc)
        {
            test(false);
        }

        public void
        exception(Ice.UserException exc)
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

    public static TestIntfPrx
    allTests(Ice.Communicator communicator, boolean collocated, PrintWriter out)
    {
        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "Test:default -p 12010 -t 10000";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        TestIntfPrx test = TestIntfPrxHelper.checkedCast(base);
        test(test != null);
        test(test.equals(base));
        out.println("ok");

        out.print("base as Object... ");
        out.flush();
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
        out.println("ok");

        out.print("base as Object (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_SBaseAsObjectI cb = new Callback_TestIntf_SBaseAsObjectI();
            test.begin_SBaseAsObject(cb);
            cb.check();
        }
        out.println("ok");

        out.print("base as base... ");
        out.flush();
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
        out.println("ok");

        out.print("base as base (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_SBaseAsSBaseI cb = new Callback_TestIntf_SBaseAsSBaseI();
            test.begin_SBaseAsSBase(cb);
            cb.check();
        }
        out.println("ok");

        out.print("base with known derived as base... ");
        out.flush();
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
        out.println("ok");

        out.print("base with known derived as base (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_SBSKnownDerivedAsSBaseI cb = new Callback_TestIntf_SBSKnownDerivedAsSBaseI();
            test.begin_SBSKnownDerivedAsSBase(cb);
            cb.check();
        }
        out.println("ok");

        out.print("base with known derived as known derived... ");
        out.flush();
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
        out.println("ok");

        out.print("base with known derived as known derived (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_SBSKnownDerivedAsSBSKnownDerivedI cb =
                new Callback_TestIntf_SBSKnownDerivedAsSBSKnownDerivedI();
            test.begin_SBSKnownDerivedAsSBSKnownDerived(cb);
            cb.check();
        }
        out.println("ok");

        out.print("base with unknown derived as base... ");
        out.flush();
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
        out.println("ok");

        out.print("base with unknown derived as base (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_SBSUnknownDerivedAsSBaseI cb = new Callback_TestIntf_SBSUnknownDerivedAsSBaseI();
            test.begin_SBSUnknownDerivedAsSBase(cb);
            cb.check();
        }
        out.println("ok");

        out.print("unknown with Object as Object... ");
        out.flush();
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
        out.println("ok");

        out.print("unknown with Object as Object (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_SUnknownAsObjectI cb = new Callback_TestIntf_SUnknownAsObjectI();
            test.begin_SUnknownAsObject(cb);
            cb.check();
        }
        out.println("ok");

        out.print("one-element cycle... ");
        out.flush();
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
        out.println("ok");

        out.print("one-element cycle (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_oneElementCycleI cb = new Callback_TestIntf_oneElementCycleI();
            test.begin_oneElementCycle(cb);
            cb.check();
        }
        out.println("ok");

        out.print("two-element cycle... ");
        out.flush();
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
        out.println("ok");

        out.print("two-element cycle (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_twoElementCycleI cb = new Callback_TestIntf_twoElementCycleI();
            test.begin_twoElementCycle(cb);
            cb.check();
        }
        out.println("ok");

        out.print("known derived pointer slicing as base... ");
        out.flush();
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
        out.println("ok");

        out.print("known derived pointer slicing as base (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_D1AsBI cb = new Callback_TestIntf_D1AsBI();
            test.begin_D1AsB(cb);
            cb.check();
        }
        out.println("ok");

        out.print("known derived pointer slicing as derived... ");
        out.flush();
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
        out.println("ok");

        out.print("known derived pointer slicing as derived (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_D1AsD1I cb = new Callback_TestIntf_D1AsD1I();
            test.begin_D1AsD1(cb);
            cb.check();
        }
        out.println("ok");

        out.print("unknown derived pointer slicing as base... ");
        out.flush();
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
        out.println("ok");

        out.print("unknown derived pointer slicing as base (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_D2AsBI cb = new Callback_TestIntf_D2AsBI();
            test.begin_D2AsB(cb);
            cb.check();
        }
        out.println("ok");

        out.print("param ptr slicing with known first... ");
        out.flush();
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
        out.println("ok");

        out.print("param ptr slicing with known first (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_paramTest1I cb = new Callback_TestIntf_paramTest1I();
            test.begin_paramTest1(cb);
            cb.check();
        }
        out.println("ok");

        out.print("param ptr slicing with unknown first... ");
        out.flush();
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
        out.println("ok");

        out.print("param ptr slicing with unknown first (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_paramTest2I cb = new Callback_TestIntf_paramTest2I();
            test.begin_paramTest2(cb);
            cb.check();
        }
        out.println("ok");

        out.print("return value identity with known first... ");
        out.flush();
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
        out.println("ok");

        out.print("return value identity with known first (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_returnTest1I cb = new Callback_TestIntf_returnTest1I();
            test.begin_returnTest1(cb);
            cb.check();
        }
        out.println("ok");

        out.print("return value identity with unknown first... ");
        out.flush();
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
        out.println("ok");

        out.print("return value identity with unknown first (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_returnTest2I cb = new Callback_TestIntf_returnTest2I();
            test.begin_returnTest2(cb);
            cb.check();
        }
        out.println("ok");

        out.print("return value identity for input params known first... ");
        out.flush();
        {
            //try
            //{
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
            //}
            //catch(Exception ex)
            //{
                //test(false);
            //}
        }
        out.println("ok");

        out.print("return value identity for input params known first (AMI)... ");
        out.flush();
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

            Callback_TestIntf_returnTest3I cb = new Callback_TestIntf_returnTest3I();
            test.begin_returnTest3(d1, d3, cb);
            cb.check();
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
        out.println("ok");

        out.print("return value identity for input params unknown first... ");
        out.flush();
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
        out.println("ok");

        out.print("return value identity for input params unknown first (AMI)... ");
        out.flush();
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

            Callback_TestIntf_returnTest3I cb = new Callback_TestIntf_returnTest3I();
            test.begin_returnTest3(d3, d1, cb);
            cb.check();
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
        out.println("ok");

        out.print("remainder unmarshaling (3 instances)... ");
        out.flush();
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
        out.println("ok");

        out.print("remainder unmarshaling (3 instances) (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_paramTest3I cb = new Callback_TestIntf_paramTest3I();
            test.begin_paramTest3(cb);
            cb.check();
        }
        out.println("ok");

        out.print("remainder unmarshaling (4 instances)... ");
        out.flush();
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
        out.println("ok");

        out.print("remainder unmarshaling (4 instances) (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_paramTest4I cb = new Callback_TestIntf_paramTest4I();
            test.begin_paramTest4(cb);
            cb.check();
        }
        out.println("ok");

        out.print("param ptr slicing, instance marshaled in unknown derived as base... ");
        out.flush();
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
        out.println("ok");

        out.print("param ptr slicing, instance marshaled in unknown derived as base (AMI)... ");
        out.flush();
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

            Callback_TestIntf_returnTest3I cb = new Callback_TestIntf_returnTest3I();
            test.begin_returnTest3(d3, b2, cb);
            cb.check();
            B r = cb.r;

            test(r != null);
            test(r.ice_id().equals("::Test::B"));
            test(r.sb.equals("D3.sb"));
            test(r.pb == r);
        }
        out.println("ok");

        out.print("param ptr slicing, instance marshaled in unknown derived as derived... ");
        out.flush();
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
        out.println("ok");

        out.print("param ptr slicing, instance marshaled in unknown derived as derived (AMI)... ");
        out.flush();
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

            Callback_TestIntf_returnTest3I cb = new Callback_TestIntf_returnTest3I();
            test.begin_returnTest3(d3, d12, cb);
            cb.check();
            B r = cb.r;

            test(r != null);
            test(r.ice_id().equals("::Test::B"));
            test(r.sb.equals("D3.sb"));
            test(r.pb == r);
        }
        out.println("ok");

        out.print("sequence slicing... ");
        out.flush();
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
        out.println("ok");

        out.print("sequence slicing (AMI)... ");
        out.flush();
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

                Callback_TestIntf_sequenceTestI cb = new Callback_TestIntf_sequenceTestI();
                test.begin_sequenceTest(ss1, ss2, cb);
                cb.check();
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
        out.println("ok");

        out.print("dictionary slicing... ");
        out.flush();
        {
            try
            {
                java.util.IdentityHashMap<Integer, B> bin = new java.util.IdentityHashMap<Integer, B>();
                BDictHolder boutH = new BDictHolder();
                java.util.Map<Integer, B> r;
                int i;
                for(i = 0; i < 10; ++i)
                {
                    String s = "D1." + new Integer(i).toString();
                    D1 d1 = new D1();
                    d1.sb = s;
                    d1.pb = d1;
                    d1.sd1 = s;
                    bin.put(i, d1);
                }

                r = test.dictionaryTest(bin, boutH);

                test(boutH.value.size() == 10);
                for(i = 0; i < 10; ++i)
                {
                    B b = boutH.value.get(i * 10);
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
                    B b = r.get(i * 20);
                    test(b != null);
                    String s = "D1." + new Integer(i * 20).toString();
                    test(b.sb.equals(s));
                    test(b.pb == (i == 0 ? null : r.get((i - 1) * 20)));
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
        out.println("ok");

        out.print("dictionary slicing (AMI)... ");
        out.flush();
        {
            java.util.Map<Integer, B> bin = new java.util.HashMap<Integer, B>();
            java.util.Map<Integer, B> bout;
            java.util.Map<Integer, B> r;
            int i;
            for(i = 0; i < 10; ++i)
            {
                String s = "D1." + new Integer(i).toString();
                D1 d1 = new D1();
                d1.sb = s;
                d1.pb = d1;
                d1.sd1 = s;
                bin.put(i, d1);
            }

            Callback_TestIntf_dictionaryTestI cb = new Callback_TestIntf_dictionaryTestI();
            test.begin_dictionaryTest(bin, cb);
            cb.check();
            bout = cb.bout;
            r = cb.r;

            test(bout.size() == 10);
            for(i = 0; i < 10; ++i)
            {
                B b = bout.get(i * 10);
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
                B b = r.get(i * 20);
                test(b != null);
                String s = "D1." + new Integer(i * 20).toString();
                test(b.sb.equals(s));
                test(b.pb == (i == 0 ? null : r.get((i - 1) * 20)));
                D1 d1 = (D1)b;
                test(d1 != null);
                test(d1.sd1.equals(s));
                test(d1.pd1 == d1);
            }
        }
        out.println("ok");

        out.print("base exception thrown as base exception... ");
        out.flush();
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
        out.println("ok");

        out.print("base exception thrown as base exception (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_throwBaseAsBaseI cb = new Callback_TestIntf_throwBaseAsBaseI();
            test.begin_throwBaseAsBase(cb);
            cb.check();
        }
        out.println("ok");

        out.print("derived exception thrown as base exception... ");
        out.flush();
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
        out.println("ok");

        out.print("derived exception thrown as base exception (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_throwDerivedAsBaseI cb = new Callback_TestIntf_throwDerivedAsBaseI();
            test.begin_throwDerivedAsBase(cb);
            cb.check();
        }
        out.println("ok");

        out.print("derived exception thrown as derived exception... ");
        out.flush();
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
        out.println("ok");

        out.print("derived exception thrown as derived exception (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_throwDerivedAsDerivedI cb = new Callback_TestIntf_throwDerivedAsDerivedI();
            test.begin_throwDerivedAsDerived(cb);
            cb.check();
        }
        out.println("ok");

        out.print("unknown derived exception thrown as base exception... ");
        out.flush();
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
        out.println("ok");

        out.print("unknown derived exception thrown as base exception (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_throwUnknownDerivedAsBaseI cb = new Callback_TestIntf_throwUnknownDerivedAsBaseI();
            test.begin_throwUnknownDerivedAsBase(cb);
            cb.check();
        }
        out.println("ok");

        out.print("forward-declared class... ");
        out.flush();
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
        out.println("ok");

        out.print("forward-declared class (AMI)... ");
        out.flush();
        {
            Callback_TestIntf_useForwardI cb = new Callback_TestIntf_useForwardI();
            test.begin_useForward(cb);
            cb.check();
        }
        out.println("ok");

        return test;
    }
}
