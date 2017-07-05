// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.slicing.objects;

import java.io.PrintWriter;

import com.zeroc.Ice.Util;

import test.Ice.slicing.objects.client.Test.*;

public class AllTests
{
    private static void test(boolean b)
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

    private static class PNodeI extends PNode
    {
        public PNodeI()
        {
            ++counter;
        }

        static int counter = 0;
    }

    private static class NodeFactoryI implements com.zeroc.Ice.ValueFactory
    {
        @Override
        public com.zeroc.Ice.Value create(String id)
        {
            if(id.equals(PNode.ice_staticId()))
            {
                return new PNodeI();
            }
            return null;
        }
    }

    private static class PreservedI extends Preserved
    {
        public PreservedI()
        {
            ++counter;
        }

        static int counter = 0;
    }

    private static class PreservedFactoryI implements com.zeroc.Ice.ValueFactory
    {
        @Override
        public com.zeroc.Ice.Value create(String id)
        {
            if(id.equals(Preserved.ice_staticId()))
            {
                return new PreservedI();
            }
            return null;
        }
    }

    private static class Wrapper<T>
    {
        public T v;
    }

    public static TestIntfPrx allTests(test.Util.Application app, boolean collocated)
    {
        PrintWriter out = app.getWriter();
        com.zeroc.Ice.Communicator communicator = app.communicator();

        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "Test:" + app.getTestEndpoint(0) + " -t 10000";
        com.zeroc.Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        TestIntfPrx test = TestIntfPrx.checkedCast(base);
        test(test != null);
        test(test.equals(base));
        out.println("ok");

        out.print("base as Object... ");
        out.flush();
        {
            com.zeroc.Ice.Value o;
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
                ex.printStackTrace();
                test(false);
            }
            test(sb != null);
            test(sb.sb.equals("SBase.sb"));
        }
        out.println("ok");

        out.print("base as Object (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.SBaseAsObjectAsync().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result != null);
                    test(result.ice_id().equals("::Test::SBase"));
                    SBase sb = (SBase)result;
                    test(sb != null);
                    test(sb.sb.equals("SBase.sb"));
                    cb.called();
                });
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
            Callback cb = new Callback();
            test.SBaseAsSBaseAsync().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.sb.equals("SBase.sb"));
                    cb.called();
                });
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
            Callback cb = new Callback();
            test.SBSKnownDerivedAsSBaseAsync().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.sb.equals("SBSKnownDerived.sb"));
                    SBSKnownDerived sbskd = (SBSKnownDerived)result;
                    test(sbskd != null);
                    test(sbskd.sbskd.equals("SBSKnownDerived.sbskd"));
                    cb.called();
                });
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
            Callback cb = new Callback();
            test.SBSKnownDerivedAsSBSKnownDerivedAsync().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.sbskd.equals("SBSKnownDerived.sbskd"));
                    cb.called();
                });
            cb.check();
        }
        out.println("ok");

        out.print("base with unknown derived as base... ");
        out.flush();
        {
            try
            {
                SBase sb = test.SBSUnknownDerivedAsSBase();
                test(sb.sb.equals("SBSUnknownDerived.sb"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        if(test.ice_getEncodingVersion().equals(Util.Encoding_1_0))
        {
            try
            {
                SBase sb = test.SBSUnknownDerivedAsSBaseCompact();
                test(sb.sb.equals("SBSUnknownDerived.sb"));
            }
            catch(com.zeroc.Ice.OperationNotExistException ex)
            {
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        else
        {
            try
            {
                //
                // This test fails when using the compact format because the instance cannot
                // be sliced to a known type.
                //
                test.SBSUnknownDerivedAsSBaseCompact();
                test(false);
            }
            catch(com.zeroc.Ice.NoValueFactoryException ex)
            {
                // Expected.
            }
            catch(com.zeroc.Ice.OperationNotExistException ex)
            {
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
            Callback cb = new Callback();
            test.SBSUnknownDerivedAsSBaseAsync().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.sb.equals("SBSUnknownDerived.sb"));
                    cb.called();
                });
            cb.check();
        }
        if(test.ice_getEncodingVersion().equals(Util.Encoding_1_0))
        {
            //
            // This test succeeds for the 1.0 encoding.
            //
            Callback cb = new Callback();
            test.SBSUnknownDerivedAsSBaseCompactAsync().whenComplete((result, ex) ->
                {
                    if(ex != null)
                    {
                        test(ex instanceof com.zeroc.Ice.OperationNotExistException);
                    }
                    else
                    {
                        test(result.sb.equals("SBSUnknownDerived.sb"));
                    }
                    cb.called();
                });
            cb.check();
        }
        else
        {
            //
            // This test fails when using the compact format because the instance cannot
            // be sliced to a known type.
            //
            Callback cb = new Callback();
            test.SBSUnknownDerivedAsSBaseCompactAsync().whenComplete((result, ex) ->
                {
                    test(ex != null);
                    test(ex instanceof com.zeroc.Ice.OperationNotExistException ||
                         ex instanceof com.zeroc.Ice.NoValueFactoryException);
                    cb.called();
                });
            cb.check();
        }
        out.println("ok");

        out.print("unknown with Object as Object... ");
        out.flush();
        {
            com.zeroc.Ice.Value o;
            try
            {
                o = test.SUnknownAsObject();
                test(!test.ice_getEncodingVersion().equals(Util.Encoding_1_0));
                test(o instanceof com.zeroc.Ice.UnknownSlicedValue);
                test(((com.zeroc.Ice.UnknownSlicedValue)o).getUnknownTypeId().equals("::Test::SUnknown"));
                test(((com.zeroc.Ice.UnknownSlicedValue)o).ice_getSlicedData() != null);
                test.checkSUnknown(o);
            }
            catch(com.zeroc.Ice.NoValueFactoryException ex)
            {
                test(test.ice_getEncodingVersion().equals(Util.Encoding_1_0));
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
            if(test.ice_getEncodingVersion().equals(Util.Encoding_1_0))
            {
                Callback cb = new Callback();
                test.SUnknownAsObjectAsync().whenComplete((result, ex) ->
                    {
                        test(ex != null);
                        test(((com.zeroc.Ice.LocalException)ex).ice_id().equals("::Ice::NoValueFactoryException"));
                        cb.called();
                    });
                cb.check();
            }
            else
            {
                Callback cb = new Callback();
                test.SUnknownAsObjectAsync().whenComplete((result, ex) ->
                    {
                        test(ex == null);
                        test(result instanceof com.zeroc.Ice.UnknownSlicedValue);
                        test(((com.zeroc.Ice.UnknownSlicedValue)result).getUnknownTypeId().equals("::Test::SUnknown"));
                        cb.called();
                    });
                cb.check();
            }
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
            Callback cb = new Callback();
            test.oneElementCycleAsync().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result != null);
                    test(result.ice_id().equals("::Test::B"));
                    test(result.sb.equals("B1.sb"));
                    test(result.pb == result);
                    cb.called();
                });
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
            Callback cb = new Callback();
            test.twoElementCycleAsync().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result != null);
                    test(result.ice_id().equals("::Test::B"));
                    test(result.sb.equals("B1.sb"));

                    B b2 = result.pb;
                    test(b2 != null);
                    test(b2.ice_id().equals("::Test::B"));
                    test(b2.sb.equals("B2.sb"));
                    test(b2.pb == result);
                    cb.called();
                });
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
            Callback cb = new Callback();
            test.D1AsBAsync().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result != null);
                    test(result.ice_id().equals("::Test::D1"));
                    test(result.sb.equals("D1.sb"));
                    test(result.pb != null);
                    test(result.pb != result);
                    D1 d1 = (D1)result;
                    test(d1 != null);
                    test(d1.sd1.equals("D1.sd1"));
                    test(d1.pd1 != null);
                    test(d1.pd1 != result);
                    test(result.pb == d1.pd1);

                    B b2 = result.pb;
                    test(b2 != null);
                    test(b2.pb == result);
                    test(b2.sb.equals("D2.sb"));
                    test(b2.ice_id().equals("::Test::B"));
                    cb.called();
                });
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
            Callback cb = new Callback();
            test.D1AsD1Async().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result != null);
                    test(result.ice_id().equals("::Test::D1"));
                    test(result.sb.equals("D1.sb"));
                    test(result.pb != null);
                    test(result.pb != result);

                    B b2 = result.pb;
                    test(b2 != null);
                    test(b2.ice_id().equals("::Test::B"));
                    test(b2.sb.equals("D2.sb"));
                    test(b2.pb == result);
                    cb.called();
                });
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
            Callback cb = new Callback();
            test.D2AsBAsync().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result != null);
                    test(result.ice_id().equals("::Test::B"));
                    test(result.sb.equals("D2.sb"));
                    test(result.pb != null);
                    test(result.pb != result);

                    B b1 = result.pb;
                    test(b1 != null);
                    test(b1.ice_id().equals("::Test::D1"));
                    test(b1.sb.equals("D1.sb"));
                    test(b1.pb == result);
                    D1 d1 = (D1)b1;
                    test(d1 != null);
                    test(d1.sd1.equals("D1.sd1"));
                    test(d1.pd1 == result);
                    cb.called();
                });
            cb.check();
        }
        out.println("ok");

        out.print("param ptr slicing with known first... ");
        out.flush();
        {
            try
            {
                TestIntf.ParamTest1Result r = test.paramTest1();

                test(r.p1 != null);
                test(r.p1.ice_id().equals("::Test::D1"));
                test(r.p1.sb.equals("D1.sb"));
                test(r.p1.pb == r.p2);
                D1 d1 = (D1)r.p1;
                test(d1 != null);
                test(d1.sd1.equals("D1.sd1"));
                test(d1.pd1 == r.p2);

                test(r.p2 != null);
                test(r.p2.ice_id().equals("::Test::B"));    // No factory, must be sliced
                test(r.p2.sb.equals("D2.sb"));
                test(r.p2.pb == r.p1);
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
            Callback cb = new Callback();
            test.paramTest1Async().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p1 != null);
                    test(result.p1.ice_id().equals("::Test::D1"));
                    test(result.p1.sb.equals("D1.sb"));
                    test(result.p1.pb == result.p2);
                    D1 d1 = (D1)result.p1;
                    test(d1 != null);
                    test(d1.sd1.equals("D1.sd1"));
                    test(d1.pd1 == result.p2);

                    test(result.p2 != null);
                    test(result.p2.ice_id().equals("::Test::B"));      // No factory, must be sliced
                    test(result.p2.sb.equals("D2.sb"));
                    test(result.p2.pb == result.p1);
                    cb.called();
                });
            cb.check();
        }
        out.println("ok");

        out.print("param ptr slicing with unknown first... ");
        out.flush();
        {
            try
            {
                TestIntf.ParamTest2Result r = test.paramTest2();

                test(r.p1 != null);
                test(r.p1.ice_id().equals("::Test::D1"));
                test(r.p1.sb.equals("D1.sb"));
                test(r.p1.pb == r.p2);
                D1 d1 = (D1)r.p1;
                test(d1 != null);
                test(d1.sd1.equals("D1.sd1"));
                test(d1.pd1 == r.p2);

                test(r.p2 != null);
                test(r.p2.ice_id().equals("::Test::B"));    // No factory, must be sliced
                test(r.p2.sb.equals("D2.sb"));
                test(r.p2.pb == r.p1);
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
            Callback cb = new Callback();
            test.paramTest2Async().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p1 != null);
                    test(result.p1.ice_id().equals("::Test::D1"));
                    test(result.p1.sb.equals("D1.sb"));
                    test(result.p1.pb == result.p2);
                    D1 d1 = (D1)result.p1;
                    test(d1 != null);
                    test(d1.sd1.equals("D1.sd1"));
                    test(d1.pd1 == result.p2);

                    test(result.p2 != null);
                    test(result.p2.ice_id().equals("::Test::B"));      // No factory, must be sliced
                    test(result.p2.sb.equals("D2.sb"));
                    test(result.p2.pb == result.p1);
                    cb.called();
                });
            cb.check();
        }
        out.println("ok");

        out.print("return value identity with known first... ");
        out.flush();
        {
            try
            {
                TestIntf.ReturnTest1Result r = test.returnTest1();
                test(r.returnValue == r.p1);
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
            Callback cb = new Callback();
            test.returnTest1Async().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.returnValue == result.p1);
                    cb.called();
                });
            cb.check();
        }
        out.println("ok");

        out.print("return value identity with unknown first... ");
        out.flush();
        {
            try
            {
                TestIntf.ReturnTest2Result r = test.returnTest2();
                test(r.returnValue == r.p2);
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
            Callback cb = new Callback();
            test.returnTest2Async().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.returnValue == result.p2);
                    cb.called();
                });
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
                    test(p3 != null);
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

            final Wrapper<B> w = new Wrapper<>();
            Callback cb = new Callback();
            test.returnTest3Async(d1, d3).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    w.v = result;
                    cb.called();
                });
            cb.check();

            B b1 = w.v;
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
                test(p3 != null);
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
                    test(p1 != null);
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

            final Wrapper<B> w = new Wrapper<>();
            Callback cb = new Callback();
            test.returnTest3Async(d3, d1).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    w.v = result;
                    cb.called();
                });
            cb.check();

            B b1 = w.v;
            test(b1 != null);
            test(b1.sb.equals("D3.sb"));
            test(b1.ice_id().equals("::Test::B"));      // Sliced by server

            try
            {
                D3 p1 = (D3)b1;
                test(p1 != null);
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
                TestIntf.ParamTest3Result r = test.paramTest3();

                test(r.p1 != null);
                test(r.p1.sb.equals("D2.sb (p1 1)"));
                test(r.p1.pb == null);
                test(r.p1.ice_id().equals("::Test::B"));

                test(r.p2 != null);
                test(r.p2.sb.equals("D2.sb (p2 1)"));
                test(r.p2.pb == null);
                test(r.p2.ice_id().equals("::Test::B"));

                test(r.returnValue != null);
                test(r.returnValue.sb.equals("D1.sb (p2 2)"));
                test(r.returnValue.pb == null);
                test(r.returnValue.ice_id().equals("::Test::D1"));
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
            Callback cb = new Callback();
            test.paramTest3Async().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p1 != null);
                    test(result.p1.sb.equals("D2.sb (p1 1)"));
                    test(result.p1.pb == null);
                    test(result.p1.ice_id().equals("::Test::B"));

                    test(result.p2 != null);
                    test(result.p2.sb.equals("D2.sb (p2 1)"));
                    test(result.p2.pb == null);
                    test(result.p2.ice_id().equals("::Test::B"));

                    test(result.returnValue != null);
                    test(result.returnValue.sb.equals("D1.sb (p2 2)"));
                    test(result.returnValue.pb == null);
                    test(result.returnValue.ice_id().equals("::Test::D1"));
                    cb.called();
                });
            cb.check();
        }
        out.println("ok");

        out.print("remainder unmarshaling (4 instances)... ");
        out.flush();
        {
            try
            {
                TestIntf.ParamTest4Result r = test.paramTest4();

                test(r.p != null);
                test(r.p.sb.equals("D4.sb (1)"));
                test(r.p.pb == null);
                test(r.p.ice_id().equals("::Test::B"));

                test(r.returnValue != null);
                test(r.returnValue.sb.equals("B.sb (2)"));
                test(r.returnValue.pb == null);
                test(r.returnValue.ice_id().equals("::Test::B"));
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
            Callback cb = new Callback();
            test.paramTest4Async().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p != null);
                    test(result.p.sb.equals("D4.sb (1)"));
                    test(result.p.pb == null);
                    test(result.p.ice_id().equals("::Test::B"));

                    test(result.returnValue != null);
                    test(result.returnValue.sb.equals("B.sb (2)"));
                    test(result.returnValue.pb == null);
                    test(result.returnValue.ice_id().equals("::Test::B"));
                    cb.called();
                });
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

            final Wrapper<B> w = new Wrapper<>();
            Callback cb = new Callback();
            test.returnTest3Async(d3, b2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    w.v = result;
                    cb.called();
                });
            cb.check();

            B r = w.v;
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

            final Wrapper<B> w = new Wrapper<>();
            Callback cb = new Callback();
            test.returnTest3Async(d3, d12).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    w.v = result;
                    cb.called();
                });
            cb.check();

            B r = w.v;
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
                SS3 ss;
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
            final Wrapper<SS3> w = new Wrapper<>();
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

                Callback cb = new Callback();
                test.sequenceTestAsync(ss1, ss2).whenComplete((result, ex) ->
                    {
                        test(ex == null);
                        w.v = result;
                        cb.called();
                    });
                cb.check();
            }
            SS3 ss = w.v;
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
                java.util.IdentityHashMap<Integer, B> bin = new java.util.IdentityHashMap<>();
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

                TestIntf.DictionaryTestResult r = test.dictionaryTest(bin);

                test(r.bout.size() == 10);
                for(i = 0; i < 10; ++i)
                {
                    B b = r.bout.get(i * 10);
                    test(b != null);
                    String s = "D1." + new Integer(i).toString();
                    test(b.sb.equals(s));
                    test(b.pb != null);
                    test(b.pb != b);
                    test(b.pb.sb.equals(s));
                    test(b.pb.pb == b.pb);
                }

                test(r.returnValue.size() == 10);
                for(i = 0; i < 10; ++i)
                {
                    B b = r.returnValue.get(i * 20);
                    test(b != null);
                    String s = "D1." + new Integer(i * 20).toString();
                    test(b.sb.equals(s));
                    test(b.pb == (i == 0 ? null : r.returnValue.get((i - 1) * 20)));
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
            java.util.Map<Integer, B> bin = new java.util.HashMap<>();
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

            final Wrapper<java.util.Map<Integer, B>> wbout = new Wrapper<>();
            final Wrapper<java.util.Map<Integer, B>> wr = new Wrapper<>();
            Callback cb = new Callback();
            test.dictionaryTestAsync(bin).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    wbout.v = result.bout;
                    wr.v = result.returnValue;
                    cb.called();
                });
            cb.check();

            java.util.Map<Integer, B> bout = wbout.v;
            java.util.Map<Integer, B> r = wr.v;
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
                test(e.ice_id().equals("::Test::BaseException"));
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
            Callback cb = new Callback();
            test.throwBaseAsBaseAsync().whenComplete((result, ex) ->
                {
                    test(ex != null);
                    try
                    {
                        BaseException e = (BaseException)ex;
                        test(e.ice_id().equals("::Test::BaseException"));
                        test(e.sbe.equals("sbe"));
                        test(e.pb != null);
                        test(e.pb.sb.equals("sb"));
                        test(e.pb.pb == e.pb);
                    }
                    catch(Exception e)
                    {
                        test(false);
                    }
                    cb.called();
                });
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
                test(e.ice_id().equals("::Test::DerivedException"));
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
            Callback cb = new Callback();
            test.throwDerivedAsBaseAsync().whenComplete((result, ex) ->
                {
                    test(ex != null);
                    try
                    {
                        DerivedException e = (DerivedException)ex;
                        test(e.ice_id().equals("::Test::DerivedException"));
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
                    cb.called();
                });
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
                test(e.ice_id().equals("::Test::DerivedException"));
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
            Callback cb = new Callback();
            test.throwDerivedAsDerivedAsync().whenComplete((result, ex) ->
                {
                    test(ex != null);
                    try
                    {
                        DerivedException e = (DerivedException)ex;
                        test(e.ice_id().equals("::Test::DerivedException"));
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
                    cb.called();
                });
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
                test(e.ice_id().equals("::Test::BaseException"));
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
            Callback cb = new Callback();
            test.throwUnknownDerivedAsBaseAsync().whenComplete((result, ex) ->
                {
                    test(ex != null);
                    try
                    {
                        BaseException e = (BaseException)ex;
                        test(e.ice_id().equals("::Test::BaseException"));
                        test(e.sbe.equals("sbe"));
                        test(e.pb != null);
                        test(e.pb.sb.equals("sb d2"));
                        test(e.pb.pb == e.pb);
                    }
                    catch(Exception e)
                    {
                        test(false);
                    }
                    cb.called();
                });
            cb.check();
        }
        out.println("ok");

        out.print("forward-declared class... ");
        out.flush();
        {
            try
            {
                Forward f = test.useForward();
                test(f != null);
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
            Callback cb = new Callback();
            test.useForwardAsync().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result != null);
                    cb.called();
                });
            cb.check();
        }
        out.println("ok");

        out.print("preserved classes... ");
        out.flush();

        //
        // Register a factory in order to substitute our own subclass of Preserved. This provides
        // an easy way to determine how many unmarshaled instances currently exist.
        //
        // TODO: We have to install this now (even though it's not necessary yet), because otherwise
        // the Ice run time will install its own internal factory for Preserved upon receiving the
        // first instance.
        //
        communicator.getValueFactoryManager().add(new PreservedFactoryI(), Preserved.ice_staticId());

        try
        {
            //
            // Server knows the most-derived class PDerived.
            //
            PDerived pd = new PDerived();
            pd.pi = 3;
            pd.ps = "preserved";
            pd.pb = pd;

            PBase r = test.exchangePBase(pd);
            PDerived p2 = (PDerived)r;
            test(p2.pi == 3);
            test(p2.ps.equals("preserved"));
            test(p2.pb == p2);
        }
        catch(com.zeroc.Ice.OperationNotExistException ex)
        {
        }

        try
        {
            //
            // Server only knows the base (non-preserved) type, so the object is sliced.
            //
            PCUnknown pu = new PCUnknown();
            pu.pi = 3;
            pu.pu = "preserved";

            PBase r = test.exchangePBase(pu);
            test(!(r instanceof PCUnknown));
            test(r.pi == 3);
        }
        catch(com.zeroc.Ice.OperationNotExistException ex)
        {
        }

        try
        {
            //
            // Server only knows the intermediate type Preserved. The object will be sliced to
            // Preserved for the 1.0 encoding; otherwise it should be returned intact.
            //
            PCDerived pcd = new PCDerived();
            pcd.pi = 3;
            pcd.pbs = new PBase[] { pcd };

            PBase r = test.exchangePBase(pcd);
            if(test.ice_getEncodingVersion().equals(Util.Encoding_1_0))
            {
                test(!(r instanceof PCDerived));
                test(r.pi == 3);
            }
            else
            {
                PCDerived p2 = (PCDerived)r;
                test(p2.pi == 3);
                test(p2.pbs[0] == p2);
            }
        }
        catch(com.zeroc.Ice.OperationNotExistException ex)
        {
        }

        try
        {
            //
            // Server only knows the intermediate type Preserved. The object will be sliced to
            // Preserved for the 1.0 encoding; otherwise it should be returned intact.
            //
            CompactPCDerived pcd = new CompactPCDerived();
            pcd.pi = 3;
            pcd.pbs = new PBase[] { pcd };

            PBase r = test.exchangePBase(pcd);
            if(test.ice_getEncodingVersion().equals(Util.Encoding_1_0))
            {
                test(!(r instanceof CompactPCDerived));
                test(r.pi == 3);
            }
            else
            {
                CompactPCDerived p2 = (CompactPCDerived)r;
                test(p2.pi == 3);
                test(p2.pbs[0] == p2);
            }
        }
        catch(com.zeroc.Ice.OperationNotExistException ex)
        {
        }

        try
        {
            //
            // Send an object that will have multiple preserved slices in the server.
            // The object will be sliced to Preserved for the 1.0 encoding.
            //
            PCDerived3 pcd = new PCDerived3();
            pcd.pi = 3;
            //
            // Sending more than 254 objects exercises the encoding for object ids.
            //
            pcd.pbs = new PBase[300];
            int i;
            for(i = 0; i < 300; ++i)
            {
                PCDerived2 p2 = new PCDerived2();
                p2.pi = i;
                p2.pbs = new PBase[] { null }; // Nil reference. This slice should not have an indirection table.
                p2.pcd2 = i;
                pcd.pbs[i] = p2;
            }
            pcd.pcd2 = pcd.pi;
            pcd.pcd3 = pcd.pbs[10];

            PBase r = test.exchangePBase(pcd);
            if(test.ice_getEncodingVersion().equals(Util.Encoding_1_0))
            {
                test(!(r instanceof PCDerived3));
                test(r instanceof Preserved);
                test(r.pi == 3);
            }
            else
            {
                PCDerived3 p3 = (PCDerived3)r;
                test(p3.pi == 3);
                for(i = 0; i < 300; ++i)
                {
                    PCDerived2 p2 = (PCDerived2)p3.pbs[i];
                    test(p2.pi == i);
                    test(p2.pbs.length == 1);
                    test(p2.pbs[0] == null);
                    test(p2.pcd2 == i);
                }
                test(p3.pcd2 == p3.pi);
                test(p3.pcd3 == p3.pbs[10]);
            }
        }
        catch(com.zeroc.Ice.OperationNotExistException ex)
        {
        }

        try
        {
            //
            // Obtain an object with preserved slices and send it back to the server.
            // The preserved slices should be excluded for the 1.0 encoding, otherwise
            // they should be included.
            //
            Preserved p = test.PBSUnknownAsPreserved();
            test.checkPBSUnknown(p);
            if(!test.ice_getEncodingVersion().equals(Util.Encoding_1_0))
            {
                com.zeroc.Ice.SlicedData slicedData = p.ice_getSlicedData();
                test(slicedData != null);
                test(slicedData.slices.length == 1);
                test(slicedData.slices[0].typeId.equals("::Test::PSUnknown"));
                test.ice_encodingVersion(Util.Encoding_1_0).checkPBSUnknown(p);
            }
            else
            {
                test(p.ice_getSlicedData() == null);
            }
        }
        catch(com.zeroc.Ice.OperationNotExistException ex)
        {
        }
        out.println("ok");

        out.print("preserved classes (AMI)... ");
        out.flush();
        try
        {
            //
            // Server knows the most-derived class PDerived.
            //
            PDerived pd = new PDerived();
            pd.pi = 3;
            pd.ps = "preserved";
            pd.pb = pd;

            Callback cb = new Callback();
            test.exchangePBaseAsync(pd).whenComplete((result, ex) ->
                {
                    if(ex != null)
                    {
                        test(ex instanceof com.zeroc.Ice.OperationNotExistException);
                    }
                    else
                    {
                        PDerived p2 = (PDerived)result;
                        test(p2.pi == 3);
                        test(p2.ps.equals("preserved"));
                        test(p2.pb == p2);
                    }
                    cb.called();
                });
            cb.check();
        }
        catch(com.zeroc.Ice.OperationNotExistException ex)
        {
        }

        try
        {
            //
            // Server only knows the base (non-preserved) type, so the object is sliced.
            //
            PCUnknown pu = new PCUnknown();
            pu.pi = 3;
            pu.pu = "preserved";

            Callback cb = new Callback();
            test.exchangePBaseAsync(pu).whenComplete((result, ex) ->
                {
                    if(ex != null)
                    {
                        test(ex instanceof com.zeroc.Ice.OperationNotExistException);
                    }
                    else
                    {
                        test(!(result instanceof PCUnknown));
                        test(result.pi == 3);
                    }
                    cb.called();
                });
            cb.check();
        }
        catch(com.zeroc.Ice.OperationNotExistException ex)
        {
        }

        try
        {
            //
            // Server only knows the intermediate type Preserved. The object will be sliced to
            // Preserved for the 1.0 encoding; otherwise it should be returned intact.
            //
            PCDerived pcd = new PCDerived();
            pcd.pi = 3;
            pcd.pbs = new PBase[] { pcd };

            if(test.ice_getEncodingVersion().equals(Util.Encoding_1_0))
            {
                Callback cb = new Callback();
                test.exchangePBaseAsync(pcd).whenComplete((result, ex) ->
                    {
                        if(ex != null)
                        {
                            test(ex instanceof com.zeroc.Ice.OperationNotExistException);
                        }
                        else
                        {
                            test(!(result instanceof PCDerived));
                            test(result.pi == 3);
                        }
                        cb.called();
                    });
                cb.check();
            }
            else
            {
                Callback cb = new Callback();
                test.exchangePBaseAsync(pcd).whenComplete((result, ex) ->
                    {
                        if(ex != null)
                        {
                            test(ex instanceof com.zeroc.Ice.OperationNotExistException);
                        }
                        else
                        {
                            PCDerived p2 = (PCDerived)result;
                            test(p2.pi == 3);
                            test(p2.pbs[0] == p2);
                        }
                        cb.called();
                    });
                cb.check();
            }
        }
        catch(com.zeroc.Ice.OperationNotExistException ex)
        {
        }

        try
        {
            //
            // Server only knows the intermediate type Preserved. The object will be sliced to
            // Preserved for the 1.0 encoding; otherwise it should be returned intact.
            //
            CompactPCDerived pcd = new CompactPCDerived();
            pcd.pi = 3;
            pcd.pbs = new PBase[] { pcd };

            if(test.ice_getEncodingVersion().equals(Util.Encoding_1_0))
            {
                Callback cb = new Callback();
                test.exchangePBaseAsync(pcd).whenComplete((result, ex) ->
                    {
                        if(ex != null)
                        {
                            test(ex instanceof com.zeroc.Ice.OperationNotExistException);
                        }
                        else
                        {
                            test(!(result instanceof CompactPCDerived));
                            test(result.pi == 3);
                        }
                        cb.called();
                    });
                cb.check();
            }
            else
            {
                Callback cb = new Callback();
                test.exchangePBaseAsync(pcd).whenComplete((result, ex) ->
                    {
                        if(ex != null)
                        {
                            test(ex instanceof com.zeroc.Ice.OperationNotExistException);
                        }
                        else
                        {
                            CompactPCDerived p2 = (CompactPCDerived)result;
                            test(p2.pi == 3);
                            test(p2.pbs[0] == p2);
                        }
                        cb.called();
                    });
                cb.check();
            }
        }
        catch(com.zeroc.Ice.OperationNotExistException ex)
        {
        }

        try
        {
            //
            // Send an object that will have multiple preserved slices in the server.
            // The object will be sliced to Preserved for the 1.0 encoding.
            //
            PCDerived3 pcd = new PCDerived3();
            pcd.pi = 3;
            //
            // Sending more than 254 objects exercises the encoding for object ids.
            //
            pcd.pbs = new PBase[300];
            int i;
            for(i = 0; i < 300; ++i)
            {
                PCDerived2 p2 = new PCDerived2();
                p2.pi = i;
                p2.pbs = new PBase[] { null }; // Nil reference. This slice should not have an indirection table.
                p2.pcd2 = i;
                pcd.pbs[i] = p2;
            }
            pcd.pcd2 = pcd.pi;
            pcd.pcd3 = pcd.pbs[10];

            if(test.ice_getEncodingVersion().equals(Util.Encoding_1_0))
            {
                Callback cb = new Callback();
                test.exchangePBaseAsync(pcd).whenComplete((result, ex) ->
                    {
                        if(ex != null)
                        {
                            test(ex instanceof com.zeroc.Ice.OperationNotExistException);
                        }
                        else
                        {
                            test(!(result instanceof PCDerived3));
                            test(result instanceof Preserved);
                            test(result.pi == 3);
                        }
                        cb.called();
                    });
                cb.check();
            }
            else
            {
                Callback cb = new Callback();
                test.exchangePBaseAsync(pcd).whenComplete((result, ex) ->
                    {
                        if(ex != null)
                        {
                            test(ex instanceof com.zeroc.Ice.OperationNotExistException);
                        }
                        else
                        {
                            PCDerived3 p3 = (PCDerived3)result;
                            test(p3.pi == 3);
                            for(int j = 0; j < 300; ++j)
                            {
                                PCDerived2 p2 = (PCDerived2)p3.pbs[j];
                                test(p2.pi == j);
                                test(p2.pbs.length == 1);
                                test(p2.pbs[0] == null);
                                test(p2.pcd2 == j);
                            }
                            test(p3.pcd2 == p3.pi);
                            test(p3.pcd3 == p3.pbs[10]);
                        }
                        cb.called();
                    });
                cb.check();
            }
        }
        catch(com.zeroc.Ice.OperationNotExistException ex)
        {
        }

        try
        {
            //
            // Obtain an object with preserved slices and send it back to the server.
            // The preserved slices should be excluded for the 1.0 encoding, otherwise
            // they should be included.
            //
            Preserved p = test.PBSUnknownAsPreserved();
            test.checkPBSUnknown(p);
            if(!test.ice_getEncodingVersion().equals(Util.Encoding_1_0))
            {
                test.ice_encodingVersion(Util.Encoding_1_0).checkPBSUnknown(p);
            }
        }
        catch(com.zeroc.Ice.OperationNotExistException ex)
        {
        }

        out.println("ok");

        out.print("garbage collection for preserved classes... ");
        out.flush();
        try
        {
            //
            // Register a factory in order to substitute our own subclass of PNode. This provides
            // an easy way to determine how many unmarshaled instances currently exist.
            //
            communicator.getValueFactoryManager().add(new NodeFactoryI(), PNode.ice_staticId());

            //
            // Relay a graph through the server.
            //
            {
                PNode c = new PNode();
                c.next = new PNode();
                c.next.next = new PNode();
                c.next.next.next = c;

                test(PNodeI.counter == 0);
                test.exchangePNode(c);

                test(PNodeI.counter == 3);
                PNodeI.counter = 0;
            }

            //
            // Obtain a preserved object from the server where the most-derived
            // type is unknown. The preserved slice refers to a graph of PNode
            // objects.
            //
            {
                test(PNodeI.counter == 0);
                Preserved p = test.PBSUnknownAsPreservedWithGraph();
                test.checkPBSUnknownWithGraph(p);
                test(PNodeI.counter == 3);
                PNodeI.counter = 0;
            }

            //
            // Obtain a preserved object from the server where the most-derived
            // type is unknown. A data member in the preserved slice refers to the
            // outer object, so the chain of references looks like this:
            //
            // outer.slicedData.outer
            //
            {
                PreservedI.counter = 0;
                Preserved p = test.PBSUnknown2AsPreservedWithGraph();
                test.checkPBSUnknown2WithGraph(p);
                test(PreservedI.counter == 1);
                PreservedI.counter = 0;
            }

            //
            // Throw a preserved exception where the most-derived type is unknown.
            // The preserved exception slice contains a class data member. This
            // object is also preserved, and its most-derived type is also unknown.
            // The preserved slice of the object contains a class data member that
            // refers to itself.
            //
            // The chain of references looks like this:
            //
            // ex.slicedData.obj.slicedData.obj
            //
            try
            {
                test(PreservedI.counter == 0);

                try
                {
                    test.throwPreservedException();
                }
                catch(PreservedException ex)
                {
                    test(PreservedI.counter == 1);
                }

                PreservedI.counter = 0;
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        catch(com.zeroc.Ice.OperationNotExistException ex)
        {
        }

        out.println("ok");

        return test;
    }
}
