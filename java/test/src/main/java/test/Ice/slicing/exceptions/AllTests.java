// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.slicing.exceptions;

import test.Ice.slicing.exceptions.client.Test.*;

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

    private static class RelayI implements Relay
    {
        @Override
        public void knownPreservedAsBase(com.zeroc.Ice.Current current)
            throws Base
        {
            KnownPreservedDerived ex = new KnownPreservedDerived();
            ex.b = "base";
            ex.kp = "preserved";
            ex.kpd = "derived";
            throw ex;
        }

        @Override
        public void knownPreservedAsKnownPreserved(com.zeroc.Ice.Current current)
            throws KnownPreserved
        {
            KnownPreservedDerived ex = new KnownPreservedDerived();
            ex.b = "base";
            ex.kp = "preserved";
            ex.kpd = "derived";
            throw ex;
        }

        @Override
        public void unknownPreservedAsBase(com.zeroc.Ice.Current current)
            throws Base
        {
            Preserved2 ex = new Preserved2();
            ex.b = "base";
            ex.kp = "preserved";
            ex.kpd = "derived";
            ex.p1 = new PreservedClass("bc", "pc");
            ex.p2 = ex.p1;
            throw ex;
        }

        @Override
        public void unknownPreservedAsKnownPreserved(com.zeroc.Ice.Current current)
            throws KnownPreserved
        {
            Preserved2 ex = new Preserved2();
            ex.b = "base";
            ex.kp = "preserved";
            ex.kpd = "derived";
            ex.p1 = new PreservedClass("bc", "pc");
            ex.p2 = ex.p1;
            throw ex;
        }
    }

    public static TestIntfPrx allTests(test.TestHelper helper, boolean collocated)
    {
        java.io.PrintWriter out = helper.getWriter();
        com.zeroc.Ice.Communicator communicator = helper.communicator();

        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "Test:" + helper.getTestEndpoint(0) + " -t 10000";
        com.zeroc.Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        TestIntfPrx test = TestIntfPrx.checkedCast(base);
        test(test != null);
        test(test.equals(base));
        out.println("ok");

        out.print("base... ");
        out.flush();
        {
            try
            {
                test.baseAsBase();
                test(false);
            }
            catch(Base b)
            {
                test(b.b.equals("Base.b"));
                test(b.ice_id().equals("::Test::Base"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("base (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.baseAsBaseAsync().whenComplete((result, ex) ->
                {
                    test(ex != null && ex instanceof Base);
                    Base b = (Base)ex;
                    test(b.b.equals("Base.b"));
                    test(b.ice_id().equals("::Test::Base"));
                    cb.called();
                });
            cb.check();
        }
        out.println("ok");

        out.print("slicing of unknown derived... ");
        out.flush();
        {
            try
            {
                test.unknownDerivedAsBase();
                test(false);
            }
            catch(Base b)
            {
                test(b.b.equals("UnknownDerived.b"));
                test(b.ice_id().equals("::Test::Base"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("slicing of unknown derived (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.unknownDerivedAsBaseAsync().whenComplete((result, ex) ->
                {
                    test(ex != null && ex instanceof Base);
                    Base b = (Base)ex;
                    test(b.b.equals("UnknownDerived.b"));
                    test(b.ice_id().equals("::Test::Base"));
                    cb.called();
                });
            cb.check();
        }
        out.println("ok");

        out.print("non-slicing of known derived as base... ");
        out.flush();
        {
            try
            {
                test.knownDerivedAsBase();
                test(false);
            }
            catch(KnownDerived k)
            {
                test(k.b.equals("KnownDerived.b"));
                test(k.kd.equals("KnownDerived.kd"));
                test(k.ice_id().equals("::Test::KnownDerived"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("non-slicing of known derived as base (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.knownDerivedAsBaseAsync().whenComplete((result, ex) ->
                {
                    test(ex != null && ex instanceof KnownDerived);
                    KnownDerived k = (KnownDerived)ex;
                    test(k.b.equals("KnownDerived.b"));
                    test(k.kd.equals("KnownDerived.kd"));
                    test(k.ice_id().equals("::Test::KnownDerived"));
                    cb.called();
                });
            cb.check();
        }
        out.println("ok");

        out.print("non-slicing of known derived as derived... ");
        out.flush();
        {
            try
            {
                test.knownDerivedAsKnownDerived();
                test(false);
            }
            catch(KnownDerived k)
            {
                test(k.b.equals("KnownDerived.b"));
                test(k.kd.equals("KnownDerived.kd"));
                test(k.ice_id().equals("::Test::KnownDerived"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("non-slicing of known derived as derived (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.knownDerivedAsKnownDerivedAsync().whenComplete((result, ex) ->
                {
                    test(ex != null && ex instanceof KnownDerived);
                    KnownDerived k = (KnownDerived)ex;
                    test(k.b.equals("KnownDerived.b"));
                    test(k.kd.equals("KnownDerived.kd"));
                    test(k.ice_id().equals("::Test::KnownDerived"));
                    cb.called();
                });
            cb.check();
        }
        out.println("ok");

        out.print("slicing of unknown intermediate as base... ");
        out.flush();
        {
            try
            {
                test.unknownIntermediateAsBase();
                test(false);
            }
            catch(Base b)
            {
                test(b.b.equals("UnknownIntermediate.b"));
                test(b.ice_id().equals("::Test::Base"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("slicing of unknown intermediate as base (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.unknownIntermediateAsBaseAsync().whenComplete((result, ex) ->
                {
                    test(ex != null && ex instanceof Base);
                    Base b = (Base)ex;
                    test(b.b.equals("UnknownIntermediate.b"));
                    test(b.ice_id().equals("::Test::Base"));
                    cb.called();
                });
            cb.check();
        }
        out.println("ok");

        out.print("slicing of known intermediate as base... ");
        out.flush();
        {
            try
            {
                test.knownIntermediateAsBase();
                test(false);
            }
            catch(KnownIntermediate ki)
            {
                test(ki.b.equals("KnownIntermediate.b"));
                test(ki.ki.equals("KnownIntermediate.ki"));
                test(ki.ice_id().equals("::Test::KnownIntermediate"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("slicing of known intermediate as base (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.knownIntermediateAsBaseAsync().whenComplete((result, ex) ->
                {
                    test(ex != null && ex instanceof KnownIntermediate);
                    KnownIntermediate ki = (KnownIntermediate)ex;
                    test(ki.b.equals("KnownIntermediate.b"));
                    test(ki.ki.equals("KnownIntermediate.ki"));
                    test(ki.ice_id().equals("::Test::KnownIntermediate"));
                    cb.called();
                });
            cb.check();
        }
        out.println("ok");

        out.print("slicing of known most derived as base... ");
        out.flush();
        {
            try
            {
                test.knownMostDerivedAsBase();
                test(false);
            }
            catch(KnownMostDerived kmd)
            {
                test(kmd.b.equals("KnownMostDerived.b"));
                test(kmd.ki.equals("KnownMostDerived.ki"));
                test(kmd.kmd.equals("KnownMostDerived.kmd"));
                test(kmd.ice_id().equals("::Test::KnownMostDerived"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("slicing of known most derived as base (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.knownMostDerivedAsBaseAsync().whenComplete((result, ex) ->
                {
                    test(ex != null && ex instanceof KnownMostDerived);
                    KnownMostDerived kmd = (KnownMostDerived)ex;
                    test(kmd.b.equals("KnownMostDerived.b"));
                    test(kmd.ki.equals("KnownMostDerived.ki"));
                    test(kmd.kmd.equals("KnownMostDerived.kmd"));
                    test(kmd.ice_id().equals("::Test::KnownMostDerived"));
                    cb.called();
                });
            cb.check();
        }
        out.println("ok");

        out.print("non-slicing of known intermediate as intermediate... ");
        out.flush();
        {
            try
            {
                test.knownIntermediateAsKnownIntermediate();
                test(false);
            }
            catch(KnownIntermediate ki)
            {
                test(ki.b.equals("KnownIntermediate.b"));
                test(ki.ki.equals("KnownIntermediate.ki"));
                test(ki.ice_id().equals("::Test::KnownIntermediate"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("non-slicing of known intermediate as intermediate (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.knownIntermediateAsKnownIntermediateAsync().whenComplete((result, ex) ->
                {
                    test(ex != null && ex instanceof KnownIntermediate);
                    KnownIntermediate ki = (KnownIntermediate)ex;
                    test(ki.b.equals("KnownIntermediate.b"));
                    test(ki.ki.equals("KnownIntermediate.ki"));
                    test(ki.ice_id().equals("::Test::KnownIntermediate"));
                    cb.called();
                });
            cb.check();
        }
        out.println("ok");

        out.print("non-slicing of known most derived as intermediate... ");
        out.flush();
        {
            try
            {
                test.knownMostDerivedAsKnownIntermediate();
                test(false);
            }
            catch(KnownMostDerived kmd)
            {
                test(kmd.b.equals("KnownMostDerived.b"));
                test(kmd.ki.equals("KnownMostDerived.ki"));
                test(kmd.kmd.equals("KnownMostDerived.kmd"));
                test(kmd.ice_id().equals("::Test::KnownMostDerived"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("non-slicing of known most derived as intermediate (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.knownMostDerivedAsKnownIntermediateAsync().whenComplete((result, ex) ->
                {
                    test(ex != null && ex instanceof KnownMostDerived);
                    KnownMostDerived kmd = (KnownMostDerived)ex;
                    test(kmd.b.equals("KnownMostDerived.b"));
                    test(kmd.ki.equals("KnownMostDerived.ki"));
                    test(kmd.kmd.equals("KnownMostDerived.kmd"));
                    test(kmd.ice_id().equals("::Test::KnownMostDerived"));
                    cb.called();
                });
            cb.check();
        }
        out.println("ok");

        out.print("non-slicing of known most derived as most derived... ");
        out.flush();
        {
            try
            {
                test.knownMostDerivedAsKnownMostDerived();
                test(false);
            }
            catch(KnownMostDerived kmd)
            {
                test(kmd.b.equals("KnownMostDerived.b"));
                test(kmd.ki.equals("KnownMostDerived.ki"));
                test(kmd.kmd.equals("KnownMostDerived.kmd"));
                test(kmd.ice_id().equals("::Test::KnownMostDerived"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("non-slicing of known most derived as most derived (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.knownMostDerivedAsKnownMostDerivedAsync().whenComplete((result, ex) ->
                {
                    test(ex != null && ex instanceof KnownMostDerived);
                    KnownMostDerived kmd = (KnownMostDerived)ex;
                    test(kmd.b.equals("KnownMostDerived.b"));
                    test(kmd.ki.equals("KnownMostDerived.ki"));
                    test(kmd.kmd.equals("KnownMostDerived.kmd"));
                    test(kmd.ice_id().equals("::Test::KnownMostDerived"));
                    cb.called();
                });
            cb.check();
        }
        out.println("ok");

        out.print("slicing of unknown most derived, known intermediate as base... ");
        out.flush();
        {
            try
            {
                test.unknownMostDerived1AsBase();
                test(false);
            }
            catch(KnownIntermediate ki)
            {
                test(ki.b.equals("UnknownMostDerived1.b"));
                test(ki.ki.equals("UnknownMostDerived1.ki"));
                test(ki.ice_id().equals("::Test::KnownIntermediate"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("slicing of unknown most derived, known intermediate as base (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.unknownMostDerived1AsBaseAsync().whenComplete((result, ex) ->
                {
                    test(ex != null && ex instanceof KnownIntermediate);
                    KnownIntermediate ki = (KnownIntermediate)ex;
                    test(ki.b.equals("UnknownMostDerived1.b"));
                    test(ki.ki.equals("UnknownMostDerived1.ki"));
                    test(ki.ice_id().equals("::Test::KnownIntermediate"));
                    cb.called();
                });
            cb.check();
        }
        out.println("ok");

        out.print("slicing of unknown most derived, known intermediate as intermediate... ");
        out.flush();
        {
            try
            {
                test.unknownMostDerived1AsKnownIntermediate();
                test(false);
            }
            catch(KnownIntermediate ki)
            {
                test(ki.b.equals("UnknownMostDerived1.b"));
                test(ki.ki.equals("UnknownMostDerived1.ki"));
                test(ki.ice_id().equals("::Test::KnownIntermediate"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("slicing of unknown most derived, known intermediate as intermediate (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.unknownMostDerived1AsKnownIntermediateAsync().whenComplete((result, ex) ->
                {
                    test(ex != null && ex instanceof KnownIntermediate);
                    KnownIntermediate ki = (KnownIntermediate)ex;
                    test(ki.b.equals("UnknownMostDerived1.b"));
                    test(ki.ki.equals("UnknownMostDerived1.ki"));
                    test(ki.ice_id().equals("::Test::KnownIntermediate"));
                    cb.called();
                });
            cb.check();
        }
        out.println("ok");

        out.print("slicing of unknown most derived, unknown intermediate thrown as base... ");
        out.flush();
        {
            try
            {
                test.unknownMostDerived2AsBase();
                test(false);
            }
            catch(Base b)
            {
                test(b.b.equals("UnknownMostDerived2.b"));
                test(b.ice_id().equals("::Test::Base"));
            }
            catch(Exception ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("slicing of unknown most derived, unknown intermediate thrown as base (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.unknownMostDerived2AsBaseAsync().whenComplete((result, ex) ->
                {
                    test(ex != null && ex instanceof Base);
                    Base b = (Base)ex;
                    test(b.b.equals("UnknownMostDerived2.b"));
                    test(b.ice_id().equals("::Test::Base"));
                    cb.called();
                });
            cb.check();
        }
        out.println("ok");

        out.print("unknown most derived in compact format... ");
        out.flush();
        {
            try
            {
                test.unknownMostDerived2AsBaseCompact();
                test(false);
            }
            catch(Base ex)
            {
                //
                // For the 1.0 encoding, the unknown exception is sliced to Base.
                //
                test(test.ice_getEncodingVersion().equals(com.zeroc.Ice.Util.Encoding_1_0));
            }
            catch(com.zeroc.Ice.UnknownUserException ex)
            {
                //
                // An UnknownUserException is raised for the compact format because the
                // most-derived type is unknown and the exception cannot be sliced.
                //
                test(!test.ice_getEncodingVersion().equals(com.zeroc.Ice.Util.Encoding_1_0));
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

        out.print("preserved exceptions... ");
        out.flush();
        {
            try
            {
                test.unknownPreservedAsBase();
                test(false);
            }
            catch(Base ex)
            {
                if(test.ice_getEncodingVersion().equals(com.zeroc.Ice.Util.Encoding_1_0))
                {
                    test(ex.ice_getSlicedData() == null);
                }
                else
                {
                    com.zeroc.Ice.SlicedData slicedData = ex.ice_getSlicedData();
                    test(slicedData != null);
                    test(slicedData.slices.length == 2);
                    test(slicedData.slices[1].typeId.equals("::Test::SPreserved1"));
                    test(slicedData.slices[0].typeId.equals("::Test::SPreserved2"));
                }
            }

            try
            {
                test.unknownPreservedAsKnownPreserved();
                test(false);
            }
            catch(KnownPreserved ex)
            {
                test(ex.kp.equals("preserved"));
                if(test.ice_getEncodingVersion().equals(com.zeroc.Ice.Util.Encoding_1_0))
                {
                    test(ex.ice_getSlicedData() == null);
                }
                else
                {
                    com.zeroc.Ice.SlicedData slicedData = ex.ice_getSlicedData();
                    test(slicedData != null);
                    test(slicedData.slices.length == 2);
                    test(slicedData.slices[1].typeId.equals("::Test::SPreserved1"));
                    test(slicedData.slices[0].typeId.equals("::Test::SPreserved2"));
                }
            }

            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("");
            RelayPrx relay = RelayPrx.uncheckedCast(adapter.addWithUUID(new RelayI()));
            adapter.activate();
            test.ice_getConnection().setAdapter(adapter);

            try
            {
                test.relayKnownPreservedAsBase(relay);
                test(false);
            }
            catch(KnownPreservedDerived ex)
            {
                test(ex.b.equals("base"));
                test(ex.kp.equals("preserved"));
                test(ex.kpd.equals("derived"));
            }
            catch(com.zeroc.Ice.OperationNotExistException ex)
            {
            }
            catch(Exception ex)
            {
                test(false);
            }

            try
            {
                test.relayKnownPreservedAsKnownPreserved(relay);
                test(false);
            }
            catch(KnownPreservedDerived ex)
            {
                test(ex.b.equals("base"));
                test(ex.kp.equals("preserved"));
                test(ex.kpd.equals("derived"));
            }
            catch(com.zeroc.Ice.OperationNotExistException ex)
            {
            }
            catch(Exception ex)
            {
                test(false);
            }

            try
            {
                test.relayUnknownPreservedAsBase(relay);
                test(false);
            }
            catch(Preserved2 ex)
            {
                test(ex.b.equals("base"));
                test(ex.kp.equals("preserved"));
                test(ex.kpd.equals("derived"));
                test(ex.p1.ice_id().equals(PreservedClass.ice_staticId()));
                PreservedClass pc = (PreservedClass)ex.p1;
                test(pc.bc.equals("bc"));
                test(pc.pc.equals("pc"));
                test(ex.p2 == ex.p1);
            }
            catch(KnownPreservedDerived ex)
            {
                //
                // For the 1.0 encoding, the unknown exception is sliced to KnownPreserved.
                //
                test(test.ice_getEncodingVersion().equals(com.zeroc.Ice.Util.Encoding_1_0));
                test(ex.b.equals("base"));
                test(ex.kp.equals("preserved"));
                test(ex.kpd.equals("derived"));
            }
            catch(com.zeroc.Ice.OperationNotExistException ex)
            {
            }
            catch(Exception ex)
            {
                test(false);
            }

            try
            {
                test.relayUnknownPreservedAsKnownPreserved(relay);
                test(false);
            }
            catch(Preserved2 ex)
            {
                test(ex.b.equals("base"));
                test(ex.kp.equals("preserved"));
                test(ex.kpd.equals("derived"));
                test(ex.p1.ice_id().equals(PreservedClass.ice_staticId()));
                PreservedClass pc = (PreservedClass)ex.p1;
                test(pc.bc.equals("bc"));
                test(pc.pc.equals("pc"));
                test(ex.p2 == ex.p1);
            }
            catch(KnownPreservedDerived ex)
            {
                //
                // For the 1.0 encoding, the unknown exception is sliced to KnownPreserved.
                //
                test(test.ice_getEncodingVersion().equals(com.zeroc.Ice.Util.Encoding_1_0));
                test(ex.b.equals("base"));
                test(ex.kp.equals("preserved"));
                test(ex.kpd.equals("derived"));
            }
            catch(com.zeroc.Ice.OperationNotExistException ex)
            {
            }
            catch(Exception ex)
            {
                test(false);
            }

            adapter.destroy();
        }
        out.println("ok");

        return test;
    }
}
