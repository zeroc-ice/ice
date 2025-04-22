// Copyright (c) ZeroC, Inc.

package test.Ice.slicing.objects;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.MarshalException;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.OperationNotExistException;
import com.zeroc.Ice.SlicedData;
import com.zeroc.Ice.SliceLoader;
import com.zeroc.Ice.UnknownSlicedValue;
import com.zeroc.Ice.Util;
import com.zeroc.Ice.Value;

import test.TestHelper;
import test.Ice.slicing.objects.client.Test.B;
import test.Ice.slicing.objects.client.Test.BaseException;
import test.Ice.slicing.objects.client.Test.CompactPCDerived;
import test.Ice.slicing.objects.client.Test.D1;
import test.Ice.slicing.objects.client.Test.D3;
import test.Ice.slicing.objects.client.Test.DerivedException;
import test.Ice.slicing.objects.client.Test.Forward;
import test.Ice.slicing.objects.client.Test.PBase;
import test.Ice.slicing.objects.client.Test.PCDerived;
import test.Ice.slicing.objects.client.Test.PCDerived2;
import test.Ice.slicing.objects.client.Test.PCDerived3;
import test.Ice.slicing.objects.client.Test.PCUnknown;
import test.Ice.slicing.objects.client.Test.PDerived;
import test.Ice.slicing.objects.client.Test.PNode;
import test.Ice.slicing.objects.client.Test.Preserved;
import test.Ice.slicing.objects.client.Test.PreservedException;
import test.Ice.slicing.objects.client.Test.SBSKnownDerived;
import test.Ice.slicing.objects.client.Test.SBase;
import test.Ice.slicing.objects.client.Test.SS1;
import test.Ice.slicing.objects.client.Test.SS2;
import test.Ice.slicing.objects.client.Test.SS3;
import test.Ice.slicing.objects.client.Test.TestIntf;
import test.Ice.slicing.objects.client.Test.TestIntfPrx;

import java.io.PrintWriter;
import java.util.HashMap;
import java.util.IdentityHashMap;
import java.util.Map;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    private static class Callback {
        Callback() {
            _called = false;
        }

        public synchronized void check() {
            while (!_called) {
                try {
                    wait();
                } catch (InterruptedException ex) {}
            }

            _called = false;
        }

        public synchronized void called() {
            assert (!_called);
            _called = true;
            notify();
        }

        private boolean _called;
    }

    public static class CustomSliceLoader implements SliceLoader {
        @Override
        public java.lang.Object newInstance(String typeId) {
            if (typeId.equals(PNode.ice_staticId())) {
                return new PNodeI();
            } else if (typeId.equals(Preserved.ice_staticId())) {
                return new PreservedI();
            }
            return null;
        }
    }

    private static class PNodeI extends PNode {
        public PNodeI() {
            ++counter;
        }

        static int counter;
    }

    private static class PreservedI extends Preserved {
        public PreservedI() {
            ++counter;
        }

        static int counter;
    }

    private static class Wrapper<T> {
        public T v;
    }

    public static TestIntfPrx allTests(TestHelper helper, boolean collocated) {
        PrintWriter out = helper.getWriter();
        Communicator communicator = helper.communicator();

        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "Test:" + helper.getTestEndpoint(0) + " -t 10000";
        ObjectPrx base = communicator.stringToProxy(ref);
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
            Value o;
            SBase sb = null;
            try {
                o = test.SBaseAsObject();
                test(o != null);
                test("::Test::SBase".equals(o.ice_id()));
                sb = (SBase) o;
            } catch (Exception ex) {
                ex.printStackTrace();
                test(false);
            }
            test(sb != null);
            test("SBase.sb".equals(sb.sb));
        }
        out.println("ok");

        out.print("base as Object (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.SBaseAsObjectAsync()
                .whenComplete(
                    (result, ex) -> {
                        test(ex == null);
                        test(result != null);
                        test("::Test::SBase".equals(result.ice_id()));
                        SBase sb = (SBase) result;
                        test(sb != null);
                        test("SBase.sb".equals(sb.sb));
                        cb.called();
                    });
            cb.check();
        }
        out.println("ok");

        out.print("base as base... ");
        out.flush();
        {
            SBase sb;
            try {
                sb = test.SBaseAsSBase();
                test("SBase.sb".equals(sb.sb));
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("base as base (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.SBaseAsSBaseAsync()
                .whenComplete(
                    (result, ex) -> {
                        test(ex == null);
                        test("SBase.sb".equals(result.sb));
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
            try {
                sb = test.SBSKnownDerivedAsSBase();
                test("SBSKnownDerived.sb".equals(sb.sb));
                sbskd = (SBSKnownDerived) sb;
            } catch (Exception ex) {
                test(false);
            }
            test(sbskd != null);
            test("SBSKnownDerived.sbskd".equals(sbskd.sbskd));
        }
        out.println("ok");

        out.print("base with known derived as base (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.SBSKnownDerivedAsSBaseAsync()
                .whenComplete(
                    (result, ex) -> {
                        test(ex == null);
                        test("SBSKnownDerived.sb".equals(result.sb));
                        SBSKnownDerived sbskd = (SBSKnownDerived) result;
                        test(sbskd != null);
                        test("SBSKnownDerived.sbskd".equals(sbskd.sbskd));
                        cb.called();
                    });
            cb.check();
        }
        out.println("ok");

        out.print("base with known derived as known derived... ");
        out.flush();
        {
            SBSKnownDerived sbskd;
            try {
                sbskd = test.SBSKnownDerivedAsSBSKnownDerived();
                test("SBSKnownDerived.sbskd".equals(sbskd.sbskd));
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("base with known derived as known derived (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.SBSKnownDerivedAsSBSKnownDerivedAsync()
                .whenComplete(
                    (result, ex) -> {
                        test(ex == null);
                        test("SBSKnownDerived.sbskd".equals(result.sbskd));
                        cb.called();
                    });
            cb.check();
        }
        out.println("ok");

        out.print("base with unknown derived as base... ");
        out.flush();
        {
            try {
                SBase sb = test.SBSUnknownDerivedAsSBase();
                test("SBSUnknownDerived.sb".equals(sb.sb));
            } catch (Exception ex) {
                test(false);
            }
        }
        if (test.ice_getEncodingVersion().equals(Util.Encoding_1_0)) {
            try {
                SBase sb = test.SBSUnknownDerivedAsSBaseCompact();
                test("SBSUnknownDerived.sb".equals(sb.sb));
            } catch (OperationNotExistException ex) {} catch (Exception ex) {
                test(false);
            }
        } else {
            try {
                //
                // This test fails when using the compact format because the instance cannot be
                // sliced to a known type.
                //
                test.SBSUnknownDerivedAsSBaseCompact();
                test(false);
            } catch (MarshalException ex) {
                // Expected.
            } catch (OperationNotExistException ex) {} catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("base with unknown derived as base (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.SBSUnknownDerivedAsSBaseAsync()
                .whenComplete(
                    (result, ex) -> {
                        test(ex == null);
                        test("SBSUnknownDerived.sb".equals(result.sb));
                        cb.called();
                    });
            cb.check();
        }
        if (test.ice_getEncodingVersion().equals(Util.Encoding_1_0)) {
            //
            // This test succeeds for the 1.0 encoding.
            //
            Callback cb = new Callback();
            test.SBSUnknownDerivedAsSBaseCompactAsync()
                .whenComplete(
                    (result, ex) -> {
                        if (ex != null) {
                            test(ex instanceof OperationNotExistException);
                        } else {
                            test("SBSUnknownDerived.sb".equals(result.sb));
                        }
                        cb.called();
                    });
            cb.check();
        } else {
            //
            // This test fails when using the compact format because the instance cannot be sliced
            // to a known type.
            //
            Callback cb = new Callback();
            test.SBSUnknownDerivedAsSBaseCompactAsync()
                .whenComplete(
                    (result, ex) -> {
                        test(ex != null);
                        test(
                            ex instanceof OperationNotExistException
                                || ex instanceof MarshalException);
                        cb.called();
                    });
            cb.check();
        }
        out.println("ok");

        out.print("unknown with Object as Object... ");
        out.flush();
        {
            Value o;
            try {
                o = test.SUnknownAsObject();
                test(!test.ice_getEncodingVersion().equals(Util.Encoding_1_0));
                test(o instanceof UnknownSlicedValue);
                test("::Test::SUnknown".equals(((UnknownSlicedValue) o).ice_id()));
                test(((UnknownSlicedValue) o).ice_getSlicedData() != null);
                test.checkSUnknown(o);
            } catch (MarshalException ex) {
                test(test.ice_getEncodingVersion().equals(Util.Encoding_1_0));
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("unknown with Object as Object (AMI)... ");
        out.flush();
        {
            if (test.ice_getEncodingVersion().equals(Util.Encoding_1_0)) {
                Callback cb = new Callback();
                test.SUnknownAsObjectAsync()
                    .whenComplete(
                        (result, ex) -> {
                            test(ex != null);
                            test(ex instanceof MarshalException);
                            cb.called();
                        });
                cb.check();
            } else {
                Callback cb = new Callback();
                test.SUnknownAsObjectAsync()
                    .whenComplete(
                        (result, ex) -> {
                            test(ex == null);
                            test(result instanceof UnknownSlicedValue);
                            test(
                                "::Test::SUnknown"
                                    .equals(((UnknownSlicedValue) result)
                                        .ice_id()));
                            cb.called();
                        });
                cb.check();
            }
        }
        out.println("ok");

        out.print("one-element cycle... ");
        out.flush();
        {
            try {
                B b = test.oneElementCycle();
                test(b != null);
                test("::Test::B".equals(b.ice_id()));
                test("B1.sb".equals(b.sb));
                test(b.pb == b);
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("one-element cycle (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.oneElementCycleAsync()
                .whenComplete(
                    (result, ex) -> {
                        test(ex == null);
                        test(result != null);
                        test("::Test::B".equals(result.ice_id()));
                        test("B1.sb".equals(result.sb));
                        test(result.pb == result);
                        cb.called();
                    });
            cb.check();
        }
        out.println("ok");

        out.print("two-element cycle... ");
        out.flush();
        {
            try {
                B b1 = test.twoElementCycle();
                test(b1 != null);
                test("::Test::B".equals(b1.ice_id()));
                test("B1.sb".equals(b1.sb));

                B b2 = b1.pb;
                test(b2 != null);
                test("::Test::B".equals(b2.ice_id()));
                test("B2.sb".equals(b2.sb));
                test(b2.pb == b1);
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("two-element cycle (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.twoElementCycleAsync()
                .whenComplete(
                    (result, ex) -> {
                        test(ex == null);
                        test(result != null);
                        test("::Test::B".equals(result.ice_id()));
                        test("B1.sb".equals(result.sb));

                        B b2 = result.pb;
                        test(b2 != null);
                        test("::Test::B".equals(b2.ice_id()));
                        test("B2.sb".equals(b2.sb));
                        test(b2.pb == result);
                        cb.called();
                    });
            cb.check();
        }
        out.println("ok");

        out.print("known derived pointer slicing as base... ");
        out.flush();
        {
            try {
                B b1;
                b1 = test.D1AsB();
                test(b1 != null);
                test("::Test::D1".equals(b1.ice_id()));
                test("D1.sb".equals(b1.sb));
                test(b1.pb != null);
                test(b1.pb != b1);
                D1 d1 = (D1) b1;
                test(d1 != null);
                test("D1.sd1".equals(d1.sd1));
                test(d1.pd1 != null);
                test(d1.pd1 != b1);
                test(b1.pb == d1.pd1);

                B b2 = b1.pb;
                test(b2 != null);
                test(b2.pb == b1);
                test("D2.sb".equals(b2.sb));
                test("::Test::B".equals(b2.ice_id()));
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("known derived pointer slicing as base (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.D1AsBAsync()
                .whenComplete(
                    (result, ex) -> {
                        test(ex == null);
                        test(result != null);
                        test("::Test::D1".equals(result.ice_id()));
                        test("D1.sb".equals(result.sb));
                        test(result.pb != null);
                        test(result.pb != result);
                        D1 d1 = (D1) result;
                        test(d1 != null);
                        test("D1.sd1".equals(d1.sd1));
                        test(d1.pd1 != null);
                        test(d1.pd1 != result);
                        test(result.pb == d1.pd1);

                        B b2 = result.pb;
                        test(b2 != null);
                        test(b2.pb == result);
                        test("D2.sb".equals(b2.sb));
                        test("::Test::B".equals(b2.ice_id()));
                        cb.called();
                    });
            cb.check();
        }
        out.println("ok");

        out.print("known derived pointer slicing as derived... ");
        out.flush();
        {
            try {
                D1 d1;
                d1 = test.D1AsD1();
                test(d1 != null);
                test("::Test::D1".equals(d1.ice_id()));
                test("D1.sb".equals(d1.sb));
                test(d1.pb != null);
                test(d1.pb != d1);

                B b2 = d1.pb;
                test(b2 != null);
                test("::Test::B".equals(b2.ice_id()));
                test("D2.sb".equals(b2.sb));
                test(b2.pb == d1);
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("known derived pointer slicing as derived (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.D1AsD1Async()
                .whenComplete(
                    (result, ex) -> {
                        test(ex == null);
                        test(result != null);
                        test("::Test::D1".equals(result.ice_id()));
                        test("D1.sb".equals(result.sb));
                        test(result.pb != null);
                        test(result.pb != result);

                        B b2 = result.pb;
                        test(b2 != null);
                        test("::Test::B".equals(b2.ice_id()));
                        test("D2.sb".equals(b2.sb));
                        test(b2.pb == result);
                        cb.called();
                    });
            cb.check();
        }
        out.println("ok");

        out.print("unknown derived pointer slicing as base... ");
        out.flush();
        {
            try {
                B b2;
                b2 = test.D2AsB();
                test(b2 != null);
                test("::Test::B".equals(b2.ice_id()));
                test("D2.sb".equals(b2.sb));
                test(b2.pb != null);
                test(b2.pb != b2);

                B b1 = b2.pb;
                test(b1 != null);
                test("::Test::D1".equals(b1.ice_id()));
                test("D1.sb".equals(b1.sb));
                test(b1.pb == b2);
                D1 d1 = (D1) b1;
                test(d1 != null);
                test("D1.sd1".equals(d1.sd1));
                test(d1.pd1 == b2);
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("unknown derived pointer slicing as base (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.D2AsBAsync()
                .whenComplete(
                    (result, ex) -> {
                        test(ex == null);
                        test(result != null);
                        test("::Test::B".equals(result.ice_id()));
                        test("D2.sb".equals(result.sb));
                        test(result.pb != null);
                        test(result.pb != result);

                        B b1 = result.pb;
                        test(b1 != null);
                        test("::Test::D1".equals(b1.ice_id()));
                        test("D1.sb".equals(b1.sb));
                        test(b1.pb == result);
                        D1 d1 = (D1) b1;
                        test(d1 != null);
                        test("D1.sd1".equals(d1.sd1));
                        test(d1.pd1 == result);
                        cb.called();
                    });
            cb.check();
        }
        out.println("ok");

        out.print("param ptr slicing with known first... ");
        out.flush();
        {
            try {
                TestIntf.ParamTest1Result r = test.paramTest1();

                test(r.p1 != null);
                test("::Test::D1".equals(r.p1.ice_id()));
                test("D1.sb".equals(r.p1.sb));
                test(r.p1.pb == r.p2);
                D1 d1 = (D1) r.p1;
                test(d1 != null);
                test("D1.sd1".equals(d1.sd1));
                test(d1.pd1 == r.p2);

                test(r.p2 != null);
                test("::Test::B".equals(r.p2.ice_id())); // No factory, must be sliced
                test("D2.sb".equals(r.p2.sb));
                test(r.p2.pb == r.p1);
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("param ptr slicing with known first (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.paramTest1Async()
                .whenComplete(
                    (result, ex) -> {
                        test(ex == null);
                        test(result.p1 != null);
                        test("::Test::D1".equals(result.p1.ice_id()));
                        test("D1.sb".equals(result.p1.sb));
                        test(result.p1.pb == result.p2);
                        D1 d1 = (D1) result.p1;
                        test(d1 != null);
                        test("D1.sd1".equals(d1.sd1));
                        test(d1.pd1 == result.p2);

                        test(result.p2 != null);
                        test(
                            "::Test::B"
                                .equals(result.p2
                                    .ice_id())); // No factory, must be sliced
                        test("D2.sb".equals(result.p2.sb));
                        test(result.p2.pb == result.p1);
                        cb.called();
                    });
            cb.check();
        }
        out.println("ok");

        out.print("param ptr slicing with unknown first... ");
        out.flush();
        {
            try {
                TestIntf.ParamTest2Result r = test.paramTest2();

                test(r.p1 != null);
                test("::Test::D1".equals(r.p1.ice_id()));
                test("D1.sb".equals(r.p1.sb));
                test(r.p1.pb == r.p2);
                D1 d1 = (D1) r.p1;
                test(d1 != null);
                test("D1.sd1".equals(d1.sd1));
                test(d1.pd1 == r.p2);

                test(r.p2 != null);
                test("::Test::B".equals(r.p2.ice_id())); // No factory, must be sliced
                test("D2.sb".equals(r.p2.sb));
                test(r.p2.pb == r.p1);
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("param ptr slicing with unknown first (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.paramTest2Async()
                .whenComplete(
                    (result, ex) -> {
                        test(ex == null);
                        test(result.p1 != null);
                        test("::Test::D1".equals(result.p1.ice_id()));
                        test("D1.sb".equals(result.p1.sb));
                        test(result.p1.pb == result.p2);
                        D1 d1 = (D1) result.p1;
                        test(d1 != null);
                        test("D1.sd1".equals(d1.sd1));
                        test(d1.pd1 == result.p2);

                        test(result.p2 != null);
                        test(
                            "::Test::B"
                                .equals(result.p2
                                    .ice_id())); // No factory, must be sliced
                        test("D2.sb".equals(result.p2.sb));
                        test(result.p2.pb == result.p1);
                        cb.called();
                    });
            cb.check();
        }
        out.println("ok");

        out.print("return value identity with known first... ");
        out.flush();
        {
            try {
                TestIntf.ReturnTest1Result r = test.returnTest1();
                test(r.returnValue == r.p1);
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("return value identity with known first (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.returnTest1Async()
                .whenComplete(
                    (result, ex) -> {
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
            try {
                TestIntf.ReturnTest2Result r = test.returnTest2();
                test(r.returnValue == r.p2);
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("return value identity with unknown first (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.returnTest2Async()
                .whenComplete(
                    (result, ex) -> {
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
            test("D1.sb".equals(b1.sb));
            test("::Test::D1".equals(b1.ice_id()));
            D1 p1 = (D1) b1;
            test(p1 != null);
            test("D1.sd1".equals(p1.sd1));
            test(p1.pd1 == b1.pb);

            B b2 = b1.pb;
            test(b2 != null);
            test("D3.sb".equals(b2.sb));
            test(b2.pb == b1);

            if (test.ice_getEncodingVersion().equals(Util.Encoding_1_0)) {
                test(!(b2 instanceof D3));
            } else {
                test(b2 instanceof D3);
                D3 p3 = (D3) b2;
                test(p3.pd3 == p1);
                test("D3.sd3".equals(p3.sd3));
            }

            test(b1 != d1);
            test(b1 != d3);
            test(b2 != d1);
            test(b2 != d3);
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
            test.returnTest3Async(d1, d3)
                .whenComplete(
                    (result, ex) -> {
                        test(ex == null);
                        w.v = result;
                        cb.called();
                    });
            cb.check();

            B b1 = w.v;
            test(b1 != null);
            test("D1.sb".equals(b1.sb));
            test("::Test::D1".equals(b1.ice_id()));
            D1 p1 = (D1) b1;
            test(p1 != null);
            test("D1.sd1".equals(p1.sd1));
            test(p1.pd1 == b1.pb);

            B b2 = b1.pb;
            test(b2 != null);
            test("D3.sb".equals(b2.sb));
            test(b2.pb == b1);

            if (test.ice_getEncodingVersion().equals(Util.Encoding_1_0)) {
                test(!(b2 instanceof D3));
            } else {
                test(b2 instanceof D3);
                D3 p3 = (D3) b2;
                test(p3.pd3 == p1);
                test("D3.sd3".equals(p3.sd3));
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
            try {
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
                test("D3.sb".equals(b1.sb));

                B b2 = b1.pb;
                test(b2 != null);
                test("D1.sb".equals(b2.sb));
                test("::Test::D1".equals(b2.ice_id()));
                test(b2.pb == b1);
                D1 p3 = (D1) b2;
                test(p3 != null);
                test("D1.sd1".equals(p3.sd1));
                test(p3.pd1 == b1);

                if (test.ice_getEncodingVersion().equals(Util.Encoding_1_0)) {
                    test(!(b1 instanceof D3));
                } else {
                    test(b1 instanceof D3);
                    D3 p1 = (D3) b1;
                    test("D3.sd3".equals(p1.sd3));
                    test(p1.pd3 == b2);
                }

                test(b1 != d1);
                test(b1 != d3);
                test(b2 != d1);
                test(b2 != d3);
            } catch (Exception ex) {
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
            test.returnTest3Async(d3, d1)
                .whenComplete(
                    (result, ex) -> {
                        test(ex == null);
                        w.v = result;
                        cb.called();
                    });
            cb.check();

            B b1 = w.v;
            test(b1 != null);
            test("D3.sb".equals(b1.sb));

            B b2 = b1.pb;
            test(b2 != null);
            test("D1.sb".equals(b2.sb));
            test("::Test::D1".equals(b2.ice_id()));
            test(b2.pb == b1);
            D1 p3 = (D1) b2;
            test(p3 != null);
            test("D1.sd1".equals(p3.sd1));
            test(p3.pd1 == b1);

            if (test.ice_getEncodingVersion().equals(Util.Encoding_1_0)) {
                test(!(b1 instanceof D3));
            } else {
                test(b1 instanceof D3);
                D3 p1 = (D3) b1;
                test("D3.sd3".equals(p1.sd3));
                test(p1.pd3 == b2);
            }

            test(b1 != d1);
            test(b1 != d3);
            test(b2 != d1);
            test(b2 != d3);
        }
        out.println("ok");

        out.print("remainder unmarshaling (3 instances)... ");
        out.flush();
        {
            try {
                TestIntf.ParamTest3Result r = test.paramTest3();

                test(r.p1 != null);
                test("D2.sb (p1 1)".equals(r.p1.sb));
                test(r.p1.pb == null);
                test("::Test::B".equals(r.p1.ice_id()));

                test(r.p2 != null);
                test("D2.sb (p2 1)".equals(r.p2.sb));
                test(r.p2.pb == null);
                test("::Test::B".equals(r.p2.ice_id()));

                test(r.returnValue != null);
                test("D1.sb (p2 2)".equals(r.returnValue.sb));
                test(r.returnValue.pb == null);
                test("::Test::D1".equals(r.returnValue.ice_id()));
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("remainder unmarshaling (3 instances) (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.paramTest3Async()
                .whenComplete(
                    (result, ex) -> {
                        test(ex == null);
                        test(result.p1 != null);
                        test("D2.sb (p1 1)".equals(result.p1.sb));
                        test(result.p1.pb == null);
                        test("::Test::B".equals(result.p1.ice_id()));

                        test(result.p2 != null);
                        test("D2.sb (p2 1)".equals(result.p2.sb));
                        test(result.p2.pb == null);
                        test("::Test::B".equals(result.p2.ice_id()));

                        test(result.returnValue != null);
                        test("D1.sb (p2 2)".equals(result.returnValue.sb));
                        test(result.returnValue.pb == null);
                        test("::Test::D1".equals(result.returnValue.ice_id()));
                        cb.called();
                    });
            cb.check();
        }
        out.println("ok");

        out.print("remainder unmarshaling (4 instances)... ");
        out.flush();
        {
            try {
                TestIntf.ParamTest4Result r = test.paramTest4();

                test(r.p != null);
                test("D4.sb (1)".equals(r.p.sb));
                test(r.p.pb == null);
                test("::Test::B".equals(r.p.ice_id()));

                test(r.returnValue != null);
                test("B.sb (2)".equals(r.returnValue.sb));
                test(r.returnValue.pb == null);
                test("::Test::B".equals(r.returnValue.ice_id()));
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("remainder unmarshaling (4 instances) (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.paramTest4Async()
                .whenComplete(
                    (result, ex) -> {
                        test(ex == null);
                        test(result.p != null);
                        test("D4.sb (1)".equals(result.p.sb));
                        test(result.p.pb == null);
                        test("::Test::B".equals(result.p.ice_id()));

                        test(result.returnValue != null);
                        test("B.sb (2)".equals(result.returnValue.sb));
                        test(result.returnValue.pb == null);
                        test("::Test::B".equals(result.returnValue.ice_id()));
                        cb.called();
                    });
            cb.check();
        }
        out.println("ok");

        out.print("param ptr slicing, instance marshaled in unknown derived as base... ");
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

            B r = test.returnTest3(d3, b2);

            test(r != null);
            test("D3.sb".equals(r.sb));
            test(r.pb == r);
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
            test.returnTest3Async(d3, b2)
                .whenComplete(
                    (result, ex) -> {
                        test(ex == null);
                        w.v = result;
                        cb.called();
                    });
            cb.check();

            B r = w.v;
            test(r != null);
            test("D3.sb".equals(r.sb));
            test(r.pb == r);

            if (test.ice_getEncodingVersion().equals(Util.Encoding_1_0)) {
                test(!(r instanceof D3));
            } else {
                test(r instanceof D3);
                D3 p3 = (D3) r;
                test("D3.sd3".equals(p3.sd3));
                test("::Test::B".equals(p3.pd3.ice_id()));
                test("B.sb(1)".equals(p3.pd3.sb));
                test(p3.pd3.pb == p3.pd3);
            }
        }
        out.println("ok");

        out.print("param ptr slicing, instance marshaled in unknown derived as derived... ");
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

            B r = test.returnTest3(d3, d12);
            test(r != null);
            test("D3.sb".equals(r.sb));
            test(r.pb == r);
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
            test.returnTest3Async(d3, d12)
                .whenComplete(
                    (result, ex) -> {
                        test(ex == null);
                        w.v = result;
                        cb.called();
                    });
            cb.check();

            B r = w.v;
            test(r != null);
            test("D3.sb".equals(r.sb));
            test(r.pb == r);
        }
        out.println("ok");

        out.print("sequence slicing... ");
        out.flush();
        {
            try {
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

                test("::Test::B".equals(ss1b.ice_id()));
                test("::Test::D1".equals(ss1d1.ice_id()));

                test("::Test::B".equals(ss2b.ice_id()));
                test("::Test::D1".equals(ss2d1.ice_id()));

                test("::Test::B".equals(ss1b.ice_id()));
                test("::Test::D1".equals(ss1d1.ice_id()));

                test("::Test::B".equals(ss2b.ice_id()));
                test("::Test::D1".equals(ss2d1.ice_id()));

                if (test.ice_getEncodingVersion().equals(Util.Encoding_1_0)) {
                    test("::Test::B".equals(ss1d3.ice_id()));
                    test("::Test::B".equals(ss2d3.ice_id()));
                } else {
                    test("::Test::D3".equals(ss1d3.ice_id()));
                    test("::Test::D3".equals(ss2d3.ice_id()));
                }
            } catch (Exception ex) {
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
                test.sequenceTestAsync(ss1, ss2)
                    .whenComplete(
                        (result, ex) -> {
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

            test("::Test::B".equals(ss1b.ice_id()));
            test("::Test::D1".equals(ss1d1.ice_id()));

            test("::Test::B".equals(ss2b.ice_id()));
            test("::Test::D1".equals(ss2d1.ice_id()));

            if (test.ice_getEncodingVersion().equals(Util.Encoding_1_0)) {
                test("::Test::B".equals(ss1d3.ice_id()));
                test("::Test::B".equals(ss2d3.ice_id()));
            } else {
                test("::Test::D3".equals(ss1d3.ice_id()));
                test("::Test::D3".equals(ss2d3.ice_id()));
            }
        }
        out.println("ok");

        out.print("dictionary slicing... ");
        out.flush();
        {
            try {
                IdentityHashMap<Integer, B> bin = new IdentityHashMap<>();
                int i;
                for (i = 0; i < 10; i++) {
                    String s = "D1." + Integer.valueOf(i).toString();
                    D1 d1 = new D1();
                    d1.sb = s;
                    d1.pb = d1;
                    d1.sd1 = s;
                    bin.put(i, d1);
                }

                TestIntf.DictionaryTestResult r = test.dictionaryTest(bin);

                test(r.bout.size() == 10);
                for (i = 0; i < 10; i++) {
                    B b = r.bout.get(i * 10);
                    test(b != null);
                    String s = "D1." + Integer.valueOf(i).toString();
                    test(b.sb.equals(s));
                    test(b.pb != null);
                    test(b.pb != b);
                    test(b.pb.sb.equals(s));
                    test(b.pb.pb == b.pb);
                }

                test(r.returnValue.size() == 10);
                for (i = 0; i < 10; i++) {
                    B b = r.returnValue.get(i * 20);
                    test(b != null);
                    String s = "D1." + Integer.valueOf(i * 20).toString();
                    test(b.sb.equals(s));
                    test(b.pb == (i == 0 ? null : r.returnValue.get((i - 1) * 20)));
                    D1 d1 = (D1) b;
                    test(d1 != null);
                    test(d1.sd1.equals(s));
                    test(d1.pd1 == d1);
                }
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("dictionary slicing (AMI)... ");
        out.flush();
        {
            Map<Integer, B> bin = new HashMap<>();
            int i;
            for (i = 0; i < 10; i++) {
                String s = "D1." + Integer.valueOf(i).toString();
                D1 d1 = new D1();
                d1.sb = s;
                d1.pb = d1;
                d1.sd1 = s;
                bin.put(i, d1);
            }

            final Wrapper<Map<Integer, B>> wbout = new Wrapper<>();
            final Wrapper<Map<Integer, B>> wr = new Wrapper<>();
            Callback cb = new Callback();
            test.dictionaryTestAsync(bin)
                .whenComplete(
                    (result, ex) -> {
                        test(ex == null);
                        wbout.v = result.bout;
                        wr.v = result.returnValue;
                        cb.called();
                    });
            cb.check();

            Map<Integer, B> bout = wbout.v;
            Map<Integer, B> r = wr.v;
            test(bout.size() == 10);
            for (i = 0; i < 10; i++) {
                B b = bout.get(i * 10);
                test(b != null);
                String s = "D1." + Integer.valueOf(i).toString();
                test(b.sb.equals(s));
                test(b.pb != null);
                test(b.pb != b);
                test(b.pb.sb.equals(s));
                test(b.pb.pb == b.pb);
            }

            test(r.size() == 10);
            for (i = 0; i < 10; i++) {
                B b = r.get(i * 20);
                test(b != null);
                String s = "D1." + Integer.valueOf(i * 20).toString();
                test(b.sb.equals(s));
                test(b.pb == (i == 0 ? null : r.get((i - 1) * 20)));
                D1 d1 = (D1) b;
                test(d1 != null);
                test(d1.sd1.equals(s));
                test(d1.pd1 == d1);
            }
        }
        out.println("ok");

        out.print("base exception thrown as base exception... ");
        out.flush();
        {
            try {
                test.throwBaseAsBase();
                test(false);
            } catch (BaseException e) {
                test("::Test::BaseException".equals(e.ice_id()));
                test("sbe".equals(e.sbe));
                test(e.pb != null);
                test("sb".equals(e.pb.sb));
                test(e.pb.pb == e.pb);
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("base exception thrown as base exception (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.throwBaseAsBaseAsync()
                .whenComplete(
                    (result, ex) -> {
                        test(ex != null);
                        try {
                            BaseException e = (BaseException) ex;
                            test("::Test::BaseException".equals(e.ice_id()));
                            test("sbe".equals(e.sbe));
                            test(e.pb != null);
                            test("sb".equals(e.pb.sb));
                            test(e.pb.pb == e.pb);
                        } catch (Exception e) {
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
            try {
                test.throwDerivedAsBase();
                test(false);
            } catch (DerivedException e) {
                test("::Test::DerivedException".equals(e.ice_id()));
                test("sbe".equals(e.sbe));
                test(e.pb != null);
                test("sb1".equals(e.pb.sb));
                test(e.pb.pb == e.pb);
                test("sde1".equals(e.sde));
                test(e.pd1 != null);
                test("sb2".equals(e.pd1.sb));
                test(e.pd1.pb == e.pd1);
                test("sd2".equals(e.pd1.sd1));
                test(e.pd1.pd1 == e.pd1);
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("derived exception thrown as base exception (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.throwDerivedAsBaseAsync()
                .whenComplete(
                    (result, ex) -> {
                        test(ex != null);
                        try {
                            DerivedException e = (DerivedException) ex;
                            test("::Test::DerivedException".equals(e.ice_id()));
                            test("sbe".equals(e.sbe));
                            test(e.pb != null);
                            test("sb1".equals(e.pb.sb));
                            test(e.pb.pb == e.pb);
                            test("sde1".equals(e.sde));
                            test(e.pd1 != null);
                            test("sb2".equals(e.pd1.sb));
                            test(e.pd1.pb == e.pd1);
                            test("sd2".equals(e.pd1.sd1));
                            test(e.pd1.pd1 == e.pd1);
                        } catch (Exception e) {
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
            try {
                test.throwDerivedAsDerived();
                test(false);
            } catch (DerivedException e) {
                test("::Test::DerivedException".equals(e.ice_id()));
                test("sbe".equals(e.sbe));
                test(e.pb != null);
                test("sb1".equals(e.pb.sb));
                test(e.pb.pb == e.pb);
                test("sde1".equals(e.sde));
                test(e.pd1 != null);
                test("sb2".equals(e.pd1.sb));
                test(e.pd1.pb == e.pd1);
                test("sd2".equals(e.pd1.sd1));
                test(e.pd1.pd1 == e.pd1);
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("derived exception thrown as derived exception (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.throwDerivedAsDerivedAsync()
                .whenComplete(
                    (result, ex) -> {
                        test(ex != null);
                        try {
                            DerivedException e = (DerivedException) ex;
                            test("::Test::DerivedException".equals(e.ice_id()));
                            test("sbe".equals(e.sbe));
                            test(e.pb != null);
                            test("sb1".equals(e.pb.sb));
                            test(e.pb.pb == e.pb);
                            test("sde1".equals(e.sde));
                            test(e.pd1 != null);
                            test("sb2".equals(e.pd1.sb));
                            test(e.pd1.pb == e.pd1);
                            test("sd2".equals(e.pd1.sd1));
                            test(e.pd1.pd1 == e.pd1);
                        } catch (Exception e) {
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
            try {
                test.throwUnknownDerivedAsBase();
                test(false);
            } catch (BaseException e) {
                test("::Test::BaseException".equals(e.ice_id()));
                test("sbe".equals(e.sbe));
                test(e.pb != null);
                test("sb d2".equals(e.pb.sb));
                test(e.pb.pb == e.pb);
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("unknown derived exception thrown as base exception (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.throwUnknownDerivedAsBaseAsync()
                .whenComplete(
                    (result, ex) -> {
                        test(ex != null);
                        try {
                            BaseException e = (BaseException) ex;
                            test("::Test::BaseException".equals(e.ice_id()));
                            test("sbe".equals(e.sbe));
                            test(e.pb != null);
                            test("sb d2".equals(e.pb.sb));
                            test(e.pb.pb == e.pb);
                        } catch (Exception e) {
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
            try {
                Forward f = test.useForward();
                test(f != null);
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("forward-declared class (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.useForwardAsync()
                .whenComplete(
                    (result, ex) -> {
                        test(ex == null);
                        test(result != null);
                        cb.called();
                    });
            cb.check();
        }
        out.println("ok");

        out.print("preserved classes... ");
        out.flush();

        try {
            //
            // Server knows the most-derived class PDerived.
            //
            PDerived pd = new PDerived();
            pd.pi = 3;
            pd.ps = "preserved";
            pd.pb = pd;

            PBase r = test.exchangePBase(pd);
            PDerived p2 = (PDerived) r;
            test(p2.pi == 3);
            test("preserved".equals(p2.ps));
            test(p2.pb == p2);
        } catch (OperationNotExistException ex) {}

        try {
            //
            // Server only knows the base (non-preserved) type, so the object is sliced.
            //
            PCUnknown pu = new PCUnknown();
            pu.pi = 3;
            pu.pu = "preserved";

            PBase r = test.exchangePBase(pu);

            test(r.pi == 3);

            if (test.ice_getEncodingVersion().equals(Util.Encoding_1_0)) {
                test(!(r instanceof PCUnknown));
            } else {
                test(r instanceof PCUnknown);
                PCUnknown p2 = (PCUnknown) r;
                test(p2 != null);
                test("preserved".equals(p2.pu));
            }
        } catch (OperationNotExistException ex) {}

        try {
            //
            // Server only knows the intermediate type Preserved. The object will be sliced to
            // Preserved for the 1.0 encoding; otherwise it should be returned intact.
            //
            PCDerived pcd = new PCDerived();
            pcd.pi = 3;
            pcd.pbs = new PBase[]{pcd};

            PBase r = test.exchangePBase(pcd);
            if (test.ice_getEncodingVersion().equals(Util.Encoding_1_0)) {
                test(!(r instanceof PCDerived));
                test(r.pi == 3);
            } else {
                PCDerived p2 = (PCDerived) r;
                test(p2.pi == 3);
                test(p2.pbs[0] == p2);
            }
        } catch (OperationNotExistException ex) {}

        try {
            //
            // Server only knows the intermediate type Preserved. The object will be sliced to
            // Preserved for the 1.0 encoding; otherwise it should be returned intact.
            //
            CompactPCDerived pcd = new CompactPCDerived();
            pcd.pi = 3;
            pcd.pbs = new PBase[]{pcd};

            PBase r = test.exchangePBase(pcd);
            if (test.ice_getEncodingVersion().equals(Util.Encoding_1_0)) {
                test(!(r instanceof CompactPCDerived));
                test(r.pi == 3);
            } else {
                CompactPCDerived p2 = (CompactPCDerived) r;
                test(p2.pi == 3);
                test(p2.pbs[0] == p2);
            }
        } catch (OperationNotExistException ex) {}

        try {
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
            for (i = 0; i < 300; i++) {
                PCDerived2 p2 = new PCDerived2();
                p2.pi = i;
                p2.pbs =
                    new PBase[]{
                        null
                    }; // Nil reference. This slice should not have an indirection table.
                p2.pcd2 = i;
                pcd.pbs[i] = p2;
            }
            pcd.pcd2 = pcd.pi;
            pcd.pcd3 = pcd.pbs[10];

            PBase r = test.exchangePBase(pcd);
            if (test.ice_getEncodingVersion().equals(Util.Encoding_1_0)) {
                test(!(r instanceof PCDerived3));
                test(r instanceof Preserved);
                test(r.pi == 3);
            } else {
                PCDerived3 p3 = (PCDerived3) r;
                test(p3.pi == 3);
                for (i = 0; i < 300; i++) {
                    PCDerived2 p2 = (PCDerived2) p3.pbs[i];
                    test(p2.pi == i);
                    test(p2.pbs.length == 1);
                    test(p2.pbs[0] == null);
                    test(p2.pcd2 == i);
                }
                test(p3.pcd2 == p3.pi);
                test(p3.pcd3 == p3.pbs[10]);
            }
        } catch (OperationNotExistException ex) {}

        try {
            //
            // Obtain an object with preserved slices and send it back to the server. The preserved
            // slices should be excluded for the 1.0 encoding, otherwise they should be included.
            //
            Preserved p = test.PBSUnknownAsPreserved();
            test.checkPBSUnknown(p);
            if (!test.ice_getEncodingVersion().equals(Util.Encoding_1_0)) {
                SlicedData slicedData = p.ice_getSlicedData();
                test(slicedData != null);
                test(slicedData.slices.length == 1);
                test("::Test::PSUnknown".equals(slicedData.slices[0].typeId));
                test.ice_encodingVersion(Util.Encoding_1_0).checkPBSUnknown(p);
            } else {
                test(p.ice_getSlicedData() == null);
            }
        } catch (OperationNotExistException ex) {}
        out.println("ok");

        out.print("preserved classes (AMI)... ");
        out.flush();
        try {
            //
            // Server knows the most-derived class PDerived.
            //
            PDerived pd = new PDerived();
            pd.pi = 3;
            pd.ps = "preserved";
            pd.pb = pd;

            Callback cb = new Callback();
            test.exchangePBaseAsync(pd)
                .whenComplete(
                    (result, ex) -> {
                        if (ex != null) {
                            test(ex instanceof OperationNotExistException);
                        } else {
                            PDerived p2 = (PDerived) result;
                            test(p2.pi == 3);
                            test("preserved".equals(p2.ps));
                            test(p2.pb == p2);
                        }
                        cb.called();
                    });
            cb.check();
        } catch (OperationNotExistException ex) {}

        try {
            //
            // Server only knows the base (non-preserved) type, so the object is sliced.
            //
            PCUnknown pu = new PCUnknown();
            pu.pi = 3;
            pu.pu = "preserved";

            Callback cb = new Callback();
            test.exchangePBaseAsync(pu)
                .whenComplete(
                    (result, ex) -> {
                        if (ex != null) {
                            test(ex instanceof OperationNotExistException);
                        } else {
                            if (test.ice_getEncodingVersion().equals(Util.Encoding_1_0)) {
                                test(!(result instanceof PCUnknown));
                            } else {
                                test(result instanceof PCUnknown);
                                PCUnknown p2 = (PCUnknown) result;
                                test(p2 != null);
                                test("preserved".equals(p2.pu));
                            }
                        }
                        cb.called();
                    });
            cb.check();
        } catch (OperationNotExistException ex) {}

        try {
            //
            // Server only knows the intermediate type Preserved. The object will be sliced to
            // Preserved for the 1.0 encoding; otherwise it should be returned intact.
            //
            PCDerived pcd = new PCDerived();
            pcd.pi = 3;
            pcd.pbs = new PBase[]{pcd};

            if (test.ice_getEncodingVersion().equals(Util.Encoding_1_0)) {
                Callback cb = new Callback();
                test.exchangePBaseAsync(pcd)
                    .whenComplete(
                        (result, ex) -> {
                            if (ex != null) {
                                test(ex instanceof OperationNotExistException);
                            } else {
                                test(!(result instanceof PCDerived));
                                test(result.pi == 3);
                            }
                            cb.called();
                        });
                cb.check();
            } else {
                Callback cb = new Callback();
                test.exchangePBaseAsync(pcd)
                    .whenComplete(
                        (result, ex) -> {
                            if (ex != null) {
                                test(ex instanceof OperationNotExistException);
                            } else {
                                PCDerived p2 = (PCDerived) result;
                                test(p2.pi == 3);
                                test(p2.pbs[0] == p2);
                            }
                            cb.called();
                        });
                cb.check();
            }
        } catch (OperationNotExistException ex) {}

        try {
            //
            // Server only knows the intermediate type Preserved. The object will be sliced to
            // Preserved for the 1.0 encoding; otherwise it should be returned intact.
            //
            CompactPCDerived pcd = new CompactPCDerived();
            pcd.pi = 3;
            pcd.pbs = new PBase[]{pcd};

            if (test.ice_getEncodingVersion().equals(Util.Encoding_1_0)) {
                Callback cb = new Callback();
                test.exchangePBaseAsync(pcd)
                    .whenComplete(
                        (result, ex) -> {
                            if (ex != null) {
                                test(ex instanceof OperationNotExistException);
                            } else {
                                test(!(result instanceof CompactPCDerived));
                                test(result.pi == 3);
                            }
                            cb.called();
                        });
                cb.check();
            } else {
                Callback cb = new Callback();
                test.exchangePBaseAsync(pcd)
                    .whenComplete(
                        (result, ex) -> {
                            if (ex != null) {
                                test(ex instanceof OperationNotExistException);
                            } else {
                                CompactPCDerived p2 = (CompactPCDerived) result;
                                test(p2.pi == 3);
                                test(p2.pbs[0] == p2);
                            }
                            cb.called();
                        });
                cb.check();
            }
        } catch (OperationNotExistException ex) {}

        try {
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
            for (i = 0; i < 300; i++) {
                PCDerived2 p2 = new PCDerived2();
                p2.pi = i;
                p2.pbs =
                    new PBase[]{
                        null
                    }; // Nil reference. This slice should not have an indirection table.
                p2.pcd2 = i;
                pcd.pbs[i] = p2;
            }
            pcd.pcd2 = pcd.pi;
            pcd.pcd3 = pcd.pbs[10];

            if (test.ice_getEncodingVersion().equals(Util.Encoding_1_0)) {
                Callback cb = new Callback();
                test.exchangePBaseAsync(pcd)
                    .whenComplete(
                        (result, ex) -> {
                            if (ex != null) {
                                test(ex instanceof OperationNotExistException);
                            } else {
                                test(!(result instanceof PCDerived3));
                                test(result instanceof Preserved);
                                test(result.pi == 3);
                            }
                            cb.called();
                        });
                cb.check();
            } else {
                Callback cb = new Callback();
                test.exchangePBaseAsync(pcd)
                    .whenComplete(
                        (result, ex) -> {
                            if (ex != null) {
                                test(ex instanceof OperationNotExistException);
                            } else {
                                PCDerived3 p3 = (PCDerived3) result;
                                test(p3.pi == 3);
                                for (int j = 0; j < 300; j++) {
                                    PCDerived2 p2 = (PCDerived2) p3.pbs[j];
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
        } catch (OperationNotExistException ex) {}

        try {
            //
            // Obtain an object with preserved slices and send it back to the server. The preserved
            // slices should be excluded for the 1.0 encoding, otherwise they should be included.
            //
            Preserved p = test.PBSUnknownAsPreserved();
            test.checkPBSUnknown(p);
            if (!test.ice_getEncodingVersion().equals(Util.Encoding_1_0)) {
                test.ice_encodingVersion(Util.Encoding_1_0).checkPBSUnknown(p);
            }
        } catch (OperationNotExistException ex) {}

        out.println("ok");

        out.print("garbage collection for preserved classes... ");
        out.flush();
        try {
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
            // Obtain a preserved object from the server where the most-derived type is unknown. A
            // data member in the preserved slice refers to the outer object, so the chain of
            // references looks like this:
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
            // Throw a preserved exception where the most-derived type is unknown. The preserved
            // exception slice contains a class data member. This object is also preserved, and its
            // most-derived type is also unknown. The preserved slice of the object contains a class
            // data member that refers to itself.
            //
            // The chain of references looks like this:
            //
            // ex.slicedData.obj.slicedData.obj
            //
            test(PreservedI.counter == 0);

            try {
                test.throwPreservedException();
            } catch (PreservedException ex) {
                test(PreservedI.counter == 1);
            }

            PreservedI.counter = 0;
        } catch (OperationNotExistException ex) {}

        out.println("ok");

        return test;
    }

    private AllTests() {}
}
