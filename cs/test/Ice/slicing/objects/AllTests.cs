// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using Test;

public class AllTests
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.Exception();
        }
    }

    private class Callback
    {
        internal Callback()
        {
            _called = false;
        }

        public virtual void check()
        {
            _m.Lock();
            try
            {
                while(!_called)
                {
                    _m.Wait();
                }

                _called = false;
            }
            finally
            {
                _m.Unlock();
            }
        }

        public virtual void called()
        {
            _m.Lock();
            try
            {
                Debug.Assert(!_called);
                _called = true;
                _m.Notify();
            }
            finally
            {
                _m.Unlock();
            }
        }

        private bool _called;
        private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
    }

    private class AsyncCallback
    {
        public void response_SBaseAsObject(Ice.Object o)
        {
            AllTests.test(o != null);
            AllTests.test(o.ice_id().Equals("::Test::SBase"));
            SBase sb = (SBase) o;
            AllTests.test(sb != null);
            AllTests.test(sb.sb.Equals("SBase.sb"));
            callback.called();
        }

        public void response_SBaseAsSBase(SBase sb)
        {
            AllTests.test(sb.sb.Equals("SBase.sb"));
            callback.called();
        }

        public void response_SBSKnownDerivedAsSBase(SBase sb)
        {
            AllTests.test(sb.sb.Equals("SBSKnownDerived.sb"));
            SBSKnownDerived sbskd = (SBSKnownDerived) sb;
            AllTests.test(sbskd != null);
            AllTests.test(sbskd.sbskd.Equals("SBSKnownDerived.sbskd"));
            callback.called();
        }

        public void response_SBSKnownDerivedAsSBSKnownDerived(SBSKnownDerived sbskd)
        {
            AllTests.test(sbskd.sbskd.Equals("SBSKnownDerived.sbskd"));
            callback.called();
        }

        public void response_SBSUnknownDerivedAsSBase(SBase sb)
        {
            AllTests.test(sb.sb.Equals("SBSUnknownDerived.sb"));
            callback.called();
        }

        public void response_SUnknownAsObject(Ice.Object o)
        {
            AllTests.test(false);
        }

        public void exception_SUnknownAsObject(Ice.Exception exc)
        {
            AllTests.test(exc.GetType().FullName.Equals("Ice.NoObjectFactoryException"));
            callback.called();
        }

        public void response_oneElementCycle(B b)
        {
            AllTests.test(b != null);
            AllTests.test(b.ice_id().Equals("::Test::B"));
            AllTests.test(b.sb.Equals("B1.sb"));
            AllTests.test(b.pb == b);
            callback.called();
        }

        public void response_twoElementCycle(B b1)
        {
            AllTests.test(b1 != null);
            AllTests.test(b1.ice_id().Equals("::Test::B"));
            AllTests.test(b1.sb.Equals("B1.sb"));

            B b2 = b1.pb;
            AllTests.test(b2 != null);
            AllTests.test(b2.ice_id().Equals("::Test::B"));
            AllTests.test(b2.sb.Equals("B2.sb"));
            AllTests.test(b2.pb == b1);
            callback.called();
        }

        public void response_D1AsB(B b1)
        {
            AllTests.test(b1 != null);
            AllTests.test(b1.ice_id().Equals("::Test::D1"));
            AllTests.test(b1.sb.Equals("D1.sb"));
            AllTests.test(b1.pb != null);
            AllTests.test(b1.pb != b1);
            D1 d1 = (D1) b1;
            AllTests.test(d1 != null);
            AllTests.test(d1.sd1.Equals("D1.sd1"));
            AllTests.test(d1.pd1 != null);
            AllTests.test(d1.pd1 != b1);
            AllTests.test(b1.pb == d1.pd1);

            B b2 = b1.pb;
            AllTests.test(b2 != null);
            AllTests.test(b2.pb == b1);
            AllTests.test(b2.sb.Equals("D2.sb"));
            AllTests.test(b2.ice_id().Equals("::Test::B"));
            callback.called();
        }

        public void response_D1AsD1(D1 d1)
        {
            AllTests.test(d1 != null);
            AllTests.test(d1.ice_id().Equals("::Test::D1"));
            AllTests.test(d1.sb.Equals("D1.sb"));
            AllTests.test(d1.pb != null);
            AllTests.test(d1.pb != d1);

            B b2 = d1.pb;
            AllTests.test(b2 != null);
            AllTests.test(b2.ice_id().Equals("::Test::B"));
            AllTests.test(b2.sb.Equals("D2.sb"));
            AllTests.test(b2.pb == d1);
            callback.called();
        }

        public void response_D2AsB(B b2)
        {
            AllTests.test(b2 != null);
            AllTests.test(b2.ice_id().Equals("::Test::B"));
            AllTests.test(b2.sb.Equals("D2.sb"));
            AllTests.test(b2.pb != null);
            AllTests.test(b2.pb != b2);

            B b1 = b2.pb;
            AllTests.test(b1 != null);
            AllTests.test(b1.ice_id().Equals("::Test::D1"));
            AllTests.test(b1.sb.Equals("D1.sb"));
            AllTests.test(b1.pb == b2);
            D1 d1 = (D1) b1;
            AllTests.test(d1 != null);
            AllTests.test(d1.sd1.Equals("D1.sd1"));
            AllTests.test(d1.pd1 == b2);
            callback.called();
        }

        public void response_paramTest1(B b1, B b2)
        {
            AllTests.test(b1 != null);
            AllTests.test(b1.ice_id().Equals("::Test::D1"));
            AllTests.test(b1.sb.Equals("D1.sb"));
            AllTests.test(b1.pb == b2);
            D1 d1 = (D1) b1;
            AllTests.test(d1 != null);
            AllTests.test(d1.sd1.Equals("D1.sd1"));
            AllTests.test(d1.pd1 == b2);

            AllTests.test(b2 != null);
            AllTests.test(b2.ice_id().Equals("::Test::B")); // No factory, must be sliced
            AllTests.test(b2.sb.Equals("D2.sb"));
            AllTests.test(b2.pb == b1);
            callback.called();
        }

        public void response_paramTest2(B b2, B b1)
        {
            AllTests.test(b1 != null);
            AllTests.test(b1.ice_id().Equals("::Test::D1"));
            AllTests.test(b1.sb.Equals("D1.sb"));
            AllTests.test(b1.pb == b2);
            D1 d1 = (D1) b1;
            AllTests.test(d1 != null);
            AllTests.test(d1.sd1.Equals("D1.sd1"));
            AllTests.test(d1.pd1 == b2);

            AllTests.test(b2 != null);
            AllTests.test(b2.ice_id().Equals("::Test::B")); // No factory, must be sliced
            AllTests.test(b2.sb.Equals("D2.sb"));
            AllTests.test(b2.pb == b1);
            callback.called();
        }

        public void response_returnTest1(B r, B p1, B p2)
        {
            AllTests.test(r == p1);
            callback.called();
        }

        public void response_returnTest2(B r, B p1, B p2)
        {
            AllTests.test(r == p1);
            callback.called();
        }

        public void response_returnTest3(B b)
        {
            rb = b;
            callback.called();
        }

        public void response_paramTest3(B ret, B p1, B p2)
        {
            AllTests.test(p1 != null);
            AllTests.test(p1.sb.Equals("D2.sb (p1 1)"));
            AllTests.test(p1.pb == null);
            AllTests.test(p1.ice_id().Equals("::Test::B"));

            AllTests.test(p2 != null);
            AllTests.test(p2.sb.Equals("D2.sb (p2 1)"));
            AllTests.test(p2.pb == null);
            AllTests.test(p2.ice_id().Equals("::Test::B"));

            AllTests.test(ret != null);
            AllTests.test(ret.sb.Equals("D1.sb (p2 2)"));
            AllTests.test(ret.pb == null);
            AllTests.test(ret.ice_id().Equals("::Test::D1"));
            callback.called();
        }

        public void response_paramTest4(B ret, B b)
        {
            AllTests.test(b != null);
            AllTests.test(b.sb.Equals("D4.sb (1)"));
            AllTests.test(b.pb == null);
            AllTests.test(b.ice_id().Equals("::Test::B"));

            AllTests.test(ret != null);
            AllTests.test(ret.sb.Equals("B.sb (2)"));
            AllTests.test(ret.pb == null);
            AllTests.test(ret.ice_id().Equals("::Test::B"));
            callback.called();
        }

        public void response_sequenceTest(SS ss)
        {
            rss = ss;
            callback.called();
        }

        public void response_dictionaryTest(Dictionary<int, B> r, Dictionary<int, B> bout)
        {
            this.rbdict = (Dictionary<int, B>)r;
            this.obdict = (Dictionary<int, B>)bout;
            callback.called();
        }

        public void exception_throwBaseAsBase(Ice.Exception exc)
        {
            try
            {
                BaseException e = (BaseException)exc;
                AllTests.test(e.sbe.Equals("sbe"));
                AllTests.test(e.pb != null);
                AllTests.test(e.pb.sb.Equals("sb"));
                AllTests.test(e.pb.pb == e.pb);
            }
            catch(Exception)
            {
                AllTests.test(false);
            }
            callback.called();
        }

        public void exception_throwDerivedAsBase(Ice.Exception exc)
        {
            try
            {
                DerivedException e = (DerivedException)exc;
                AllTests.test(e.sbe.Equals("sbe"));
                AllTests.test(e.pb != null);
                AllTests.test(e.pb.sb.Equals("sb1"));
                AllTests.test(e.pb.pb == e.pb);
                AllTests.test(e.sde.Equals("sde1"));
                AllTests.test(e.pd1 != null);
                AllTests.test(e.pd1.sb.Equals("sb2"));
                AllTests.test(e.pd1.pb == e.pd1);
                AllTests.test(e.pd1.sd1.Equals("sd2"));
                AllTests.test(e.pd1.pd1 == e.pd1);
            }
            catch(Exception)
            {
                AllTests.test(false);
            }
            callback.called();
        }

        public void exception_throwDerivedAsDerived(Ice.Exception exc)
        {
            try
            {
                DerivedException e = (DerivedException)exc;
                AllTests.test(e.sbe.Equals("sbe"));
                AllTests.test(e.pb != null);
                AllTests.test(e.pb.sb.Equals("sb1"));
                AllTests.test(e.pb.pb == e.pb);
                AllTests.test(e.sde.Equals("sde1"));
                AllTests.test(e.pd1 != null);
                AllTests.test(e.pd1.sb.Equals("sb2"));
                AllTests.test(e.pd1.pb == e.pd1);
                AllTests.test(e.pd1.sd1.Equals("sd2"));
                AllTests.test(e.pd1.pd1 == e.pd1);
            }
            catch(Exception)
            {
                AllTests.test(false);
            }
            callback.called();
        }

        public void exception_throwUnknownDerivedAsBase(Ice.Exception exc)
        {
            try
            {
                BaseException e = (BaseException)exc;
                AllTests.test(e.sbe.Equals("sbe"));
                AllTests.test(e.pb != null);
                AllTests.test(e.pb.sb.Equals("sb d2"));
                AllTests.test(e.pb.pb == e.pb);
            }
            catch(Exception)
            {
                AllTests.test(false);
            }
            callback.called();
        }

        public void response_useForward(Forward f)
        {
            AllTests.test(f != null);
            callback.called();
        }

        public void response()
        {
            AllTests.test(false);
        }

        public void exception(Ice.Exception exc)
        {
            AllTests.test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        public B rb;
        public SS rss;
        public Dictionary<int, B> rbdict;
        public Dictionary<int, B> obdict;

        private Callback callback = new Callback();
    }

    public static TestIntfPrx allTests(Ice.Communicator communicator, bool collocated)
    {
        Console.Out.Write("testing stringToProxy... ");
        Console.Out.Flush();
        string r = "Test:default -p 12010 -t 2000";
        Ice.ObjectPrx basePrx = communicator.stringToProxy(r);
        test(basePrx != null);
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing checked cast... ");
        Console.Out.Flush();
        TestIntfPrx testPrx = TestIntfPrxHelper.checkedCast(basePrx);
        test(testPrx != null);
        test(testPrx.Equals(basePrx));
        Console.Out.WriteLine("ok");

        Console.Out.Write("base as Object... ");
        Console.Out.Flush();
        {
            Ice.Object o;
            SBase sb = null;
            try
            {
                o = testPrx.SBaseAsObject();
                test(o != null);
                test(o.ice_id().Equals("::Test::SBase"));
                sb = (SBase) o;
            }
            catch(Exception)
            {
                test(false);
            }
            test(sb != null);
            test(sb.sb.Equals("SBase.sb"));
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("base as Object (AMI)... ");
        Console.Out.Flush();
        {
            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_SBaseAsObject().whenCompleted(cb.response_SBaseAsObject, cb.exception);
            cb.check();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("base as base... ");
        Console.Out.Flush();
        {
            SBase sb;
            try
            {
                sb = testPrx.SBaseAsSBase();
                test(sb.sb.Equals("SBase.sb"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("base as base (AMI)... ");
        Console.Out.Flush();
        {
            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_SBaseAsSBase().whenCompleted(cb.response_SBaseAsSBase, cb.exception);
            cb.check();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("base with known derived as base... ");
        Console.Out.Flush();
        {
            SBase sb;
            SBSKnownDerived sbskd = null;
            try
            {
                sb = testPrx.SBSKnownDerivedAsSBase();
                test(sb.sb.Equals("SBSKnownDerived.sb"));
                sbskd = (SBSKnownDerived) sb;
            }
            catch(Exception)
            {
                test(false);
            }
            test(sbskd != null);
            test(sbskd.sbskd.Equals("SBSKnownDerived.sbskd"));
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("base with known derived as base (AMI)... ");
        Console.Out.Flush();
        {
            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_SBSKnownDerivedAsSBase().whenCompleted(cb.response_SBSKnownDerivedAsSBase, cb.exception);
            cb.check();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("base with known derived as known derived... ");
        Console.Out.Flush();
        {
            SBSKnownDerived sbskd;
            try
            {
                sbskd = testPrx.SBSKnownDerivedAsSBSKnownDerived();
                test(sbskd.sbskd.Equals("SBSKnownDerived.sbskd"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("base with known derived as known derived (AMI)... ");
        Console.Out.Flush();
        {
            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_SBSKnownDerivedAsSBSKnownDerived().whenCompleted(
                        cb.response_SBSKnownDerivedAsSBSKnownDerived, cb.exception);
            cb.check();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("base with unknown derived as base... ");
        Console.Out.Flush();
        {
            SBase sb;
            try
            {
                sb = testPrx.SBSUnknownDerivedAsSBase();
                test(sb.sb.Equals("SBSUnknownDerived.sb"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("base with unknown derived as base (AMI)... ");
        Console.Out.Flush();
        {
            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_SBSUnknownDerivedAsSBase().whenCompleted(
                        cb.response_SBSUnknownDerivedAsSBase, cb.exception);
            cb.check();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("unknown with Object as Object... ");
        Console.Out.Flush();
        {
            try
            {
                testPrx.SUnknownAsObject();
                test(false);
            }
            catch(Ice.NoObjectFactoryException)
            {
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("unknown with Object as Object (AMI)... ");
        Console.Out.Flush();
        {
            try
            {
                AsyncCallback cb = new AsyncCallback();
                testPrx.begin_SUnknownAsObject().whenCompleted(
                        cb.response_SUnknownAsObject, cb.exception_SUnknownAsObject);
                cb.check();
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("one-element cycle... ");
        Console.Out.Flush();
        {
            try
            {
                B b = testPrx.oneElementCycle();
                test(b != null);
                test(b.ice_id().Equals("::Test::B"));
                test(b.sb.Equals("B1.sb"));
                test(b.pb == b);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("one-element cycle (AMI)... ");
        Console.Out.Flush();
        {
            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_oneElementCycle().whenCompleted(
                        cb.response_oneElementCycle, cb.exception);
            cb.check();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("two-element cycle... ");
        Console.Out.Flush();
        {
            try
            {
                B b1 = testPrx.twoElementCycle();
                test(b1 != null);
                test(b1.ice_id().Equals("::Test::B"));
                test(b1.sb.Equals("B1.sb"));

                B b2 = b1.pb;
                test(b2 != null);
                test(b2.ice_id().Equals("::Test::B"));
                test(b2.sb.Equals("B2.sb"));
                test(b2.pb == b1);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("two-element cycle (AMI)... ");
        Console.Out.Flush();
        {
            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_twoElementCycle().whenCompleted(
                        cb.response_twoElementCycle, cb.exception);
            cb.check();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("known derived pointer slicing as base... ");
        Console.Out.Flush();
        {
            try
            {
                B b1;
                b1 = testPrx.D1AsB();
                test(b1 != null);
                test(b1.ice_id().Equals("::Test::D1"));
                test(b1.sb.Equals("D1.sb"));
                test(b1.pb != null);
                test(b1.pb != b1);
                D1 d1 = (D1) b1;
                test(d1 != null);
                test(d1.sd1.Equals("D1.sd1"));
                test(d1.pd1 != null);
                test(d1.pd1 != b1);
                test(b1.pb == d1.pd1);

                B b2 = b1.pb;
                test(b2 != null);
                test(b2.pb == b1);
                test(b2.sb.Equals("D2.sb"));
                test(b2.ice_id().Equals("::Test::B"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("known derived pointer slicing as base (AMI)... ");
        Console.Out.Flush();
        {
            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_D1AsB().whenCompleted(cb.response_D1AsB, cb.exception);
            cb.check();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("known derived pointer slicing as derived... ");
        Console.Out.Flush();
        {
            try
            {
                D1 d1;
                d1 = testPrx.D1AsD1();
                test(d1 != null);
                test(d1.ice_id().Equals("::Test::D1"));
                test(d1.sb.Equals("D1.sb"));
                test(d1.pb != null);
                test(d1.pb != d1);

                B b2 = d1.pb;
                test(b2 != null);
                test(b2.ice_id().Equals("::Test::B"));
                test(b2.sb.Equals("D2.sb"));
                test(b2.pb == d1);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("known derived pointer slicing as derived (AMI)... ");
        Console.Out.Flush();
        {
            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_D1AsD1().whenCompleted(cb.response_D1AsD1, cb.exception);
            cb.check();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("unknown derived pointer slicing as base... ");
        Console.Out.Flush();
        {
            try
            {
                B b2;
                b2 = testPrx.D2AsB();
                test(b2 != null);
                test(b2.ice_id().Equals("::Test::B"));
                test(b2.sb.Equals("D2.sb"));
                test(b2.pb != null);
                test(b2.pb != b2);

                B b1 = b2.pb;
                test(b1 != null);
                test(b1.ice_id().Equals("::Test::D1"));
                test(b1.sb.Equals("D1.sb"));
                test(b1.pb == b2);
                D1 d1 = (D1) b1;
                test(d1 != null);
                test(d1.sd1.Equals("D1.sd1"));
                test(d1.pd1 == b2);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("unknown derived pointer slicing as base (AMI)... ");
        Console.Out.Flush();
        {
            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_D2AsB().whenCompleted(cb.response_D2AsB, cb.exception);
            cb.check();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("param ptr slicing with known first... ");
        Console.Out.Flush();
        {
            try
            {
                B b1;
                B b2;
                testPrx.paramTest1(out b1, out b2);

                test(b1 != null);
                test(b1.ice_id().Equals("::Test::D1"));
                test(b1.sb.Equals("D1.sb"));
                test(b1.pb == b2);
                D1 d1 = (D1) b1;
                test(d1 != null);
                test(d1.sd1.Equals("D1.sd1"));
                test(d1.pd1 == b2);

                test(b2 != null);
                test(b2.ice_id().Equals("::Test::B")); // No factory, must be sliced
                test(b2.sb.Equals("D2.sb"));
                test(b2.pb == b1);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("param ptr slicing with known first (AMI)... ");
        Console.Out.Flush();
        {
            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_paramTest1().whenCompleted(cb.response_paramTest1, cb.exception);
            cb.check();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("param ptr slicing with unknown first... ");
        Console.Out.Flush();
        {
            try
            {
                B b2;
                B b1;
                testPrx.paramTest2(out b2, out b1);

                test(b1 != null);
                test(b1.ice_id().Equals("::Test::D1"));
                test(b1.sb.Equals("D1.sb"));
                test(b1.pb == b2);
                D1 d1 = (D1) b1;
                test(d1 != null);
                test(d1.sd1.Equals("D1.sd1"));
                test(d1.pd1 == b2);

                test(b2 != null);
                test(b2.ice_id().Equals("::Test::B")); // No factory, must be sliced
                test(b2.sb.Equals("D2.sb"));
                test(b2.pb == b1);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("param ptr slicing with unknown first (AMI)... ");
        Console.Out.Flush();
        {
            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_paramTest2().whenCompleted(cb.response_paramTest2, cb.exception);
            cb.check();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("return value identity with known first... ");
        Console.Out.Flush();
        {
            try
            {
                B p1;
                B p2;
                B ret = testPrx.returnTest1(out p1, out p2);
                test(ret == p1);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("return value identity with known first (AMI)... ");
        Console.Out.Flush();
        {
            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_returnTest1().whenCompleted(cb.response_returnTest1, cb.exception);
            cb.check();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("return value identity with unknown first... ");
        Console.Out.Flush();
        {
            try
            {
                B p1;
                B p2;
                B ret = testPrx.returnTest2(out p1, out p2);
                test(ret == p1);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("return value identity with unknown first (AMI)... ");
        Console.Out.Flush();
        {
            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_returnTest2().whenCompleted(cb.response_returnTest2, cb.exception);
            cb.check();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("return value identity for input params known first... ");
        Console.Out.Flush();
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

                B b1 = testPrx.returnTest3(d1, d3);

                test(b1 != null);
                test(b1.sb.Equals("D1.sb"));
                test(b1.ice_id().Equals("::Test::D1"));
                D1 p1 = (D1) b1;
                test(p1 != null);
                test(p1.sd1.Equals("D1.sd1"));
                test(p1.pd1 == b1.pb);

                B b2 = b1.pb;
                test(b2 != null);
                test(b2.sb.Equals("D3.sb"));
                test(b2.ice_id().Equals("::Test::B")); // Sliced by server
                test(b2.pb == b1);
                try
                {
                    D3 p3 = (D3) b2;
                    test(false);
                    D3 tmp = p3; p3 = tmp; // Stop compiler warning about unused variable.
                }
                catch(InvalidCastException)
                {
                }

                test(b1 != d1);
                test(b1 != d3);
                test(b2 != d1);
                test(b2 != d3);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("return value identity for input params known first (AMI)... ");
        Console.Out.Flush();
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

            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_returnTest3(d1, d3).whenCompleted(cb.response_returnTest3, cb.exception);
            cb.check();
            B b1 = cb.rb;

            test(b1 != null);
            test(b1.sb.Equals("D1.sb"));
            test(b1.ice_id().Equals("::Test::D1"));
            D1 p1 = (D1) b1;
            test(p1 != null);
            test(p1.sd1.Equals("D1.sd1"));
            test(p1.pd1 == b1.pb);

            B b2 = b1.pb;
            test(b2 != null);
            test(b2.sb.Equals("D3.sb"));
            test(b2.ice_id().Equals("::Test::B")); // Sliced by server
            test(b2.pb == b1);
            try
            {
                D3 p3 = (D3) b2;
                test(false);
                D3 tmp = p3; p3 = tmp; // Stop compiler warning about unused variable.
            }
            catch(InvalidCastException)
            {
            }

            test(b1 != d1);
            test(b1 != d3);
            test(b2 != d1);
            test(b2 != d3);
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("return value identity for input params unknown first... ");
        Console.Out.Flush();
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

                B b1 = testPrx.returnTest3(d3, d1);

                test(b1 != null);
                test(b1.sb.Equals("D3.sb"));
                test(b1.ice_id().Equals("::Test::B")); // Sliced by server

                try
                {
                    D3 p1 = (D3) b1;
                    test(false);
                    D3 tmp = p1; p1 = tmp; // Stop compiler warning about unused variable.
                }
                catch(InvalidCastException)
                {
                }

                B b2 = b1.pb;
                test(b2 != null);
                test(b2.sb.Equals("D1.sb"));
                test(b2.ice_id().Equals("::Test::D1"));
                test(b2.pb == b1);
                D1 p3 = (D1) b2;
                test(p3 != null);
                test(p3.sd1.Equals("D1.sd1"));
                test(p3.pd1 == b1);

                test(b1 != d1);
                test(b1 != d3);
                test(b2 != d1);
                test(b2 != d3);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("return value identity for input params unknown first (AMI)... ");
        Console.Out.Flush();
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

            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_returnTest3(d3, d1).whenCompleted(cb.response_returnTest3, cb.exception);
            cb.check();
            B b1 = cb.rb;

            test(b1 != null);
            test(b1.sb.Equals("D3.sb"));
            test(b1.ice_id().Equals("::Test::B")); // Sliced by server

            try
            {
                D3 p1 = (D3) b1;
                test(false);
                D3 tmp = p1; p1 = tmp; // Stop compiler warning about unused variable.
            }
            catch(InvalidCastException)
            {
            }

            B b2 = b1.pb;
            test(b2 != null);
            test(b2.sb.Equals("D1.sb"));
            test(b2.ice_id().Equals("::Test::D1"));
            test(b2.pb == b1);
            D1 p3 = (D1) b2;
            test(p3 != null);
            test(p3.sd1.Equals("D1.sd1"));
            test(p3.pd1 == b1);

            test(b1 != d1);
            test(b1 != d3);
            test(b2 != d1);
            test(b2 != d3);
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("remainder unmarshaling (3 instances)... ");
        Console.Out.Flush();
        {
            try
            {
                B p1;
                B p2;
                B ret = testPrx.paramTest3(out p1, out p2);

                test(p1 != null);
                test(p1.sb.Equals("D2.sb (p1 1)"));
                test(p1.pb == null);
                test(p1.ice_id().Equals("::Test::B"));

                test(p2 != null);
                test(p2.sb.Equals("D2.sb (p2 1)"));
                test(p2.pb == null);
                test(p2.ice_id().Equals("::Test::B"));

                test(ret != null);
                test(ret.sb.Equals("D1.sb (p2 2)"));
                test(ret.pb == null);
                test(ret.ice_id().Equals("::Test::D1"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("remainder unmarshaling (3 instances) (AMI)... ");
        Console.Out.Flush();
        {
            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_paramTest3().whenCompleted(cb.response_paramTest3, cb.exception);
            cb.check();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("remainder unmarshaling (4 instances)... ");
        Console.Out.Flush();
        {
            try
            {
                B b;
                B ret = testPrx.paramTest4(out b);

                test(b != null);
                test(b.sb.Equals("D4.sb (1)"));
                test(b.pb == null);
                test(b.ice_id().Equals("::Test::B"));

                test(ret != null);
                test(ret.sb.Equals("B.sb (2)"));
                test(ret.pb == null);
                test(ret.ice_id().Equals("::Test::B"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("remainder unmarshaling (4 instances) (AMI)... ");
        Console.Out.Flush();
        {
            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_paramTest4().whenCompleted(cb.response_paramTest4, cb.exception);
            cb.check();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("param ptr slicing, instance marshaled in unknown derived as base... ");
        Console.Out.Flush();
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

                B ret = testPrx.returnTest3(d3, b2);

                test(ret != null);
                test(ret.ice_id().Equals("::Test::B"));
                test(ret.sb.Equals("D3.sb"));
                test(ret.pb == ret);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("param ptr slicing, instance marshaled in unknown derived as base (AMI)... ");
        Console.Out.Flush();
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

            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_returnTest3(d3, b2).whenCompleted(cb.response_returnTest3, cb.exception);
            cb.check();
            B rv = cb.rb;

            test(rv != null);
            test(rv.ice_id().Equals("::Test::B"));
            test(rv.sb.Equals("D3.sb"));
            test(rv.pb == rv);
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("param ptr slicing, instance marshaled in unknown derived as derived... ");
        Console.Out.Flush();
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

                B ret = testPrx.returnTest3(d3, d12);
                test(ret != null);
                test(ret.ice_id().Equals("::Test::B"));
                test(ret.sb.Equals("D3.sb"));
                test(ret.pb == ret);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("param ptr slicing, instance marshaled in unknown derived as derived (AMI)... ");
        Console.Out.Flush();
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

            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_returnTest3(d3, d12).whenCompleted(cb.response_returnTest3, cb.exception);
            cb.check();
            B rv = cb.rb;

            test(rv != null);
            test(rv.ice_id().Equals("::Test::B"));
            test(rv.sb.Equals("D3.sb"));
            test(rv.pb == rv);
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("sequence slicing... ");
        Console.Out.Flush();
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
                    ss1.s = new BSeq(3);
                    ss1.s.Add(ss1b);
                    ss1.s.Add(ss1d1);
                    ss1.s.Add(ss1d3);

                    SS2 ss2 = new SS2();
                    ss2.s = new BSeq(3);
                    ss2.s.Add(ss2b);
                    ss2.s.Add(ss2d1);
                    ss2.s.Add(ss2d3);

                    ss = testPrx.sequenceTest(ss1, ss2);
                }

                test(ss.c1 != null);
                B ss1b2 = ss.c1.s[0];
                B ss1d2 = ss.c1.s[1];
                test(ss.c2 != null);
                B ss1d4 = ss.c1.s[2];

                test(ss.c2 != null);
                B ss2b2 = ss.c2.s[0];
                B ss2d2 = ss.c2.s[1];
                B ss2d4 = ss.c2.s[2];

                test(ss1b2.pb == ss1b2);
                test(ss1d2.pb == ss1b2);
                test(ss1d4.pb == ss1b2);

                test(ss2b2.pb == ss1b2);
                test(ss2d2.pb == ss2b2);
                test(ss2d4.pb == ss2b2);

                test(ss1b2.ice_id().Equals("::Test::B"));
                test(ss1d2.ice_id().Equals("::Test::D1"));
                test(ss1d4.ice_id().Equals("::Test::B"));

                test(ss2b2.ice_id().Equals("::Test::B"));
                test(ss2d2.ice_id().Equals("::Test::D1"));
                test(ss2d4.ice_id().Equals("::Test::B"));
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("sequence slicing (AMI)... ");
        Console.Out.Flush();
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
                ss1.s = new BSeq();
                ss1.s.Add(ss1b);
                ss1.s.Add(ss1d1);
                ss1.s.Add(ss1d3);

                SS2 ss2 = new SS2();
                ss2.s = new BSeq();
                ss2.s.Add(ss2b);
                ss2.s.Add(ss2d1);
                ss2.s.Add(ss2d3);

                AsyncCallback cb = new AsyncCallback();
                testPrx.begin_sequenceTest(ss1, ss2).whenCompleted(cb.response_sequenceTest, cb.exception);
                cb.check();
                ss = cb.rss;
            }
            test(ss.c1 != null);
            B ss1b3 = ss.c1.s[0];
            B ss1d5 = ss.c1.s[1];
            test(ss.c2 != null);
            B ss1d6 = ss.c1.s[2];

            test(ss.c2 != null);
            B ss2b3 = ss.c2.s[0];
            B ss2d5 = ss.c2.s[1];
            B ss2d6 = ss.c2.s[2];

            test(ss1b3.pb == ss1b3);
            test(ss1d6.pb == ss1b3);
            test(ss1d6.pb == ss1b3);

            test(ss2b3.pb == ss1b3);
            test(ss2d6.pb == ss2b3);
            test(ss2d6.pb == ss2b3);

            test(ss1b3.ice_id().Equals("::Test::B"));
            test(ss1d5.ice_id().Equals("::Test::D1"));
            test(ss1d6.ice_id().Equals("::Test::B"));

            test(ss2b3.ice_id().Equals("::Test::B"));
            test(ss2d5.ice_id().Equals("::Test::D1"));
            test(ss2d6.ice_id().Equals("::Test::B"));
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("dictionary slicing... ");
        Console.Out.Flush();
        {
            try
            {
                Dictionary<int, B> bin = new Dictionary<int, B>();
                Dictionary<int, B> bout;
                Dictionary<int, B> ret;
                int i;
                for(i = 0; i < 10; ++i)
                {
                    string s = "D1." + i.ToString();
                    D1 d1 = new D1();
                    d1.sb = s;
                    d1.pb = d1;
                    d1.sd1 = s;
                    bin[i] = d1;
                }

                ret = testPrx.dictionaryTest(bin, out bout);

                test(bout.Count == 10);
                for(i = 0; i < 10; ++i)
                {
                    B b = bout[i * 10];
                    test(b != null);
                    string s = "D1." + i.ToString();
                    test(b.sb.Equals(s));
                    test(b.pb != null);
                    test(b.pb != b);
                    test(b.pb.sb.Equals(s));
                    test(b.pb.pb == b.pb);
                }

                test(ret.Count == 10);
                for(i = 0; i < 10; ++i)
                {
                    B b = ret[i * 20];
                    test(b != null);
                    string s = "D1." + (i * 20).ToString();
                    test(b.sb.Equals(s));
                    test(b.pb == (i == 0 ? (B)null : ret[(i - 1) * 20]));
                    D1 d1 = (D1) b;
                    test(d1 != null);
                    test(d1.sd1.Equals(s));
                    test(d1.pd1 == d1);
                }
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("dictionary slicing (AMI)... ");
        Console.Out.Flush();
        {
            Dictionary<int, B> bin = new Dictionary<int, B>();
            Dictionary<int, B> bout;
            Dictionary<int, B> rv;
            int i;
            for(i = 0; i < 10; ++i)
            {
                string s = "D1." + i.ToString();
                D1 d1 = new D1();
                d1.sb = s;
                d1.pb = d1;
                d1.sd1 = s;
                bin[i] = d1;
            }

            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_dictionaryTest(bin).whenCompleted(cb.response_dictionaryTest, cb.exception);
            cb.check();
            bout = cb.obdict;
            rv = cb.rbdict;

            test(bout.Count == 10);
            for(i = 0; i < 10; ++i)
            {
                B b = bout[i * 10];
                test(b != null);
                string s = "D1." + i.ToString();
                test(b.sb.Equals(s));
                test(b.pb != null);
                test(b.pb != b);
                test(b.pb.sb.Equals(s));
                test(b.pb.pb == b.pb);
            }

            test(rv.Count == 10);
            for(i = 0; i < 10; ++i)
            {
                B b = rv[i * 20];
                test(b != null);
                string s = "D1." + (i * 20).ToString();
                test(b.sb.Equals(s));
                test(b.pb == (i == 0 ? (B)null : rv[(i - 1) * 20]));
                D1 d1 = (D1) b;
                test(d1 != null);
                test(d1.sd1.Equals(s));
                test(d1.pd1 == d1);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("base exception thrown as base exception... ");
        Console.Out.Flush();
        {
            try
            {
                testPrx.throwBaseAsBase();
                test(false);
            }
            catch(BaseException e)
            {
                test(e.GetType().FullName.Equals("Test.BaseException"));
                test(e.sbe.Equals("sbe"));
                test(e.pb != null);
                test(e.pb.sb.Equals("sb"));
                test(e.pb.pb == e.pb);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("base exception thrown as base exception (AMI)... ");
        Console.Out.Flush();
        {
            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_throwBaseAsBase().whenCompleted(cb.response, cb.exception_throwBaseAsBase);
            cb.check();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("derived exception thrown as base exception... ");
        Console.Out.Flush();
        {
            try
            {
                testPrx.throwDerivedAsBase();
                test(false);
            }
            catch(DerivedException e)
            {
                test(e.GetType().FullName.Equals("Test.DerivedException"));
                test(e.sbe.Equals("sbe"));
                test(e.pb != null);
                test(e.pb.sb.Equals("sb1"));
                test(e.pb.pb == e.pb);
                test(e.sde.Equals("sde1"));
                test(e.pd1 != null);
                test(e.pd1.sb.Equals("sb2"));
                test(e.pd1.pb == e.pd1);
                test(e.pd1.sd1.Equals("sd2"));
                test(e.pd1.pd1 == e.pd1);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("derived exception thrown as base exception (AMI)... ");
        Console.Out.Flush();
        {
            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_throwDerivedAsBase().whenCompleted(cb.response, cb.exception_throwDerivedAsBase);
            cb.check();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("derived exception thrown as derived exception... ");
        Console.Out.Flush();
        {
            try
            {
                testPrx.throwDerivedAsDerived();
                test(false);
            }
            catch(DerivedException e)
            {
                test(e.GetType().FullName.Equals("Test.DerivedException"));
                test(e.sbe.Equals("sbe"));
                test(e.pb != null);
                test(e.pb.sb.Equals("sb1"));
                test(e.pb.pb == e.pb);
                test(e.sde.Equals("sde1"));
                test(e.pd1 != null);
                test(e.pd1.sb.Equals("sb2"));
                test(e.pd1.pb == e.pd1);
                test(e.pd1.sd1.Equals("sd2"));
                test(e.pd1.pd1 == e.pd1);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("derived exception thrown as derived exception (AMI)... ");
        Console.Out.Flush();
        {
            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_throwDerivedAsDerived().whenCompleted(cb.response, cb.exception_throwDerivedAsDerived);
            cb.check();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("unknown derived exception thrown as base exception... ");
        Console.Out.Flush();
        {
            try
            {
                testPrx.throwUnknownDerivedAsBase();
                test(false);
            }
            catch(BaseException e)
            {
                test(e.GetType().FullName.Equals("Test.BaseException"));
                test(e.sbe.Equals("sbe"));
                test(e.pb != null);
                test(e.pb.sb.Equals("sb d2"));
                test(e.pb.pb == e.pb);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("unknown derived exception thrown as base exception (AMI)... ");
        Console.Out.Flush();
        {
            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_throwUnknownDerivedAsBase().whenCompleted(
                        cb.response, cb.exception_throwUnknownDerivedAsBase);
            cb.check();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("forward-declared class... ");
        Console.Out.Flush();
        {
            try
            {
                Forward f;
                testPrx.useForward(out f);
                test(f != null);
            }
            catch(Exception)
            {
                test(false);
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("forward-declared class (AMI)... ");
        Console.Out.Flush();
        {
            AsyncCallback cb = new AsyncCallback();
            testPrx.begin_useForward().whenCompleted(cb.response_useForward, cb.exception);
            cb.check();
        }
        Console.Out.WriteLine("ok");

        return testPrx;
    }
}
