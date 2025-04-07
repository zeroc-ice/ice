// Copyright (c) ZeroC, Inc.

package test.Ice.slicing.exceptions;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ObjectPrx;

import test.Ice.slicing.exceptions.client.Test.*;
import test.TestHelper;

import java.io.PrintWriter;

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
                } catch (InterruptedException ex) {
                }
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

        out.print("base... ");
        out.flush();
        {
            try {
                test.baseAsBase();
                test(false);
            } catch (Base b) {
                test("Base.b".equals(b.b));
                test("::Test::Base".equals(b.ice_id()));
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("base (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.baseAsBaseAsync()
                    .whenComplete(
                            (result, ex) -> {
                                test(ex != null && ex instanceof Base);
                                Base b = (Base) ex;
                                test("Base.b".equals(b.b));
                                test("::Test::Base".equals(b.ice_id()));
                                cb.called();
                            });
            cb.check();
        }
        out.println("ok");

        out.print("slicing of unknown derived... ");
        out.flush();
        {
            try {
                test.unknownDerivedAsBase();
                test(false);
            } catch (Base b) {
                test("UnknownDerived.b".equals(b.b));
                test("::Test::Base".equals(b.ice_id()));
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("slicing of unknown derived (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.unknownDerivedAsBaseAsync()
                    .whenComplete(
                            (result, ex) -> {
                                test(ex != null && ex instanceof Base);
                                Base b = (Base) ex;
                                test("UnknownDerived.b".equals(b.b));
                                test("::Test::Base".equals(b.ice_id()));
                                cb.called();
                            });
            cb.check();
        }
        out.println("ok");

        out.print("non-slicing of known derived as base... ");
        out.flush();
        {
            try {
                test.knownDerivedAsBase();
                test(false);
            } catch (KnownDerived k) {
                test("KnownDerived.b".equals(k.b));
                test("KnownDerived.kd".equals(k.kd));
                test("::Test::KnownDerived".equals(k.ice_id()));
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("non-slicing of known derived as base (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.knownDerivedAsBaseAsync()
                    .whenComplete(
                            (result, ex) -> {
                                test(ex != null && ex instanceof KnownDerived);
                                KnownDerived k = (KnownDerived) ex;
                                test("KnownDerived.b".equals(k.b));
                                test("KnownDerived.kd".equals(k.kd));
                                test("::Test::KnownDerived".equals(k.ice_id()));
                                cb.called();
                            });
            cb.check();
        }
        out.println("ok");

        out.print("non-slicing of known derived as derived... ");
        out.flush();
        {
            try {
                test.knownDerivedAsKnownDerived();
                test(false);
            } catch (KnownDerived k) {
                test("KnownDerived.b".equals(k.b));
                test("KnownDerived.kd".equals(k.kd));
                test("::Test::KnownDerived".equals(k.ice_id()));
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("non-slicing of known derived as derived (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.knownDerivedAsKnownDerivedAsync()
                    .whenComplete(
                            (result, ex) -> {
                                test(ex != null && ex instanceof KnownDerived);
                                KnownDerived k = (KnownDerived) ex;
                                test("KnownDerived.b".equals(k.b));
                                test("KnownDerived.kd".equals(k.kd));
                                test("::Test::KnownDerived".equals(k.ice_id()));
                                cb.called();
                            });
            cb.check();
        }
        out.println("ok");

        out.print("slicing of unknown intermediate as base... ");
        out.flush();
        {
            try {
                test.unknownIntermediateAsBase();
                test(false);
            } catch (Base b) {
                test("UnknownIntermediate.b".equals(b.b));
                test("::Test::Base".equals(b.ice_id()));
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("slicing of unknown intermediate as base (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.unknownIntermediateAsBaseAsync()
                    .whenComplete(
                            (result, ex) -> {
                                test(ex != null && ex instanceof Base);
                                Base b = (Base) ex;
                                test("UnknownIntermediate.b".equals(b.b));
                                test("::Test::Base".equals(b.ice_id()));
                                cb.called();
                            });
            cb.check();
        }
        out.println("ok");

        out.print("slicing of known intermediate as base... ");
        out.flush();
        {
            try {
                test.knownIntermediateAsBase();
                test(false);
            } catch (KnownIntermediate ki) {
                test("KnownIntermediate.b".equals(ki.b));
                test("KnownIntermediate.ki".equals(ki.ki));
                test("::Test::KnownIntermediate".equals(ki.ice_id()));
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("slicing of known intermediate as base (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.knownIntermediateAsBaseAsync()
                    .whenComplete(
                            (result, ex) -> {
                                test(ex != null && ex instanceof KnownIntermediate);
                                KnownIntermediate ki = (KnownIntermediate) ex;
                                test("KnownIntermediate.b".equals(ki.b));
                                test("KnownIntermediate.ki".equals(ki.ki));
                                test("::Test::KnownIntermediate".equals(ki.ice_id()));
                                cb.called();
                            });
            cb.check();
        }
        out.println("ok");

        out.print("slicing of known most derived as base... ");
        out.flush();
        {
            try {
                test.knownMostDerivedAsBase();
                test(false);
            } catch (KnownMostDerived kmd) {
                test("KnownMostDerived.b".equals(kmd.b));
                test("KnownMostDerived.ki".equals(kmd.ki));
                test("KnownMostDerived.kmd".equals(kmd.kmd));
                test("::Test::KnownMostDerived".equals(kmd.ice_id()));
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("slicing of known most derived as base (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.knownMostDerivedAsBaseAsync()
                    .whenComplete(
                            (result, ex) -> {
                                test(ex != null && ex instanceof KnownMostDerived);
                                KnownMostDerived kmd = (KnownMostDerived) ex;
                                test("KnownMostDerived.b".equals(kmd.b));
                                test("KnownMostDerived.ki".equals(kmd.ki));
                                test("KnownMostDerived.kmd".equals(kmd.kmd));
                                test("::Test::KnownMostDerived".equals(kmd.ice_id()));
                                cb.called();
                            });
            cb.check();
        }
        out.println("ok");

        out.print("non-slicing of known intermediate as intermediate... ");
        out.flush();
        {
            try {
                test.knownIntermediateAsKnownIntermediate();
                test(false);
            } catch (KnownIntermediate ki) {
                test("KnownIntermediate.b".equals(ki.b));
                test("KnownIntermediate.ki".equals(ki.ki));
                test("::Test::KnownIntermediate".equals(ki.ice_id()));
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("non-slicing of known intermediate as intermediate (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.knownIntermediateAsKnownIntermediateAsync()
                    .whenComplete(
                            (result, ex) -> {
                                test(ex != null && ex instanceof KnownIntermediate);
                                KnownIntermediate ki = (KnownIntermediate) ex;
                                test("KnownIntermediate.b".equals(ki.b));
                                test("KnownIntermediate.ki".equals(ki.ki));
                                test("::Test::KnownIntermediate".equals(ki.ice_id()));
                                cb.called();
                            });
            cb.check();
        }
        out.println("ok");

        out.print("non-slicing of known most derived as intermediate... ");
        out.flush();
        {
            try {
                test.knownMostDerivedAsKnownIntermediate();
                test(false);
            } catch (KnownMostDerived kmd) {
                test("KnownMostDerived.b".equals(kmd.b));
                test("KnownMostDerived.ki".equals(kmd.ki));
                test("KnownMostDerived.kmd".equals(kmd.kmd));
                test("::Test::KnownMostDerived".equals(kmd.ice_id()));
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("non-slicing of known most derived as intermediate (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.knownMostDerivedAsKnownIntermediateAsync()
                    .whenComplete(
                            (result, ex) -> {
                                test(ex != null && ex instanceof KnownMostDerived);
                                KnownMostDerived kmd = (KnownMostDerived) ex;
                                test("KnownMostDerived.b".equals(kmd.b));
                                test("KnownMostDerived.ki".equals(kmd.ki));
                                test("KnownMostDerived.kmd".equals(kmd.kmd));
                                test("::Test::KnownMostDerived".equals(kmd.ice_id()));
                                cb.called();
                            });
            cb.check();
        }
        out.println("ok");

        out.print("non-slicing of known most derived as most derived... ");
        out.flush();
        {
            try {
                test.knownMostDerivedAsKnownMostDerived();
                test(false);
            } catch (KnownMostDerived kmd) {
                test("KnownMostDerived.b".equals(kmd.b));
                test("KnownMostDerived.ki".equals(kmd.ki));
                test("KnownMostDerived.kmd".equals(kmd.kmd));
                test("::Test::KnownMostDerived".equals(kmd.ice_id()));
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("non-slicing of known most derived as most derived (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.knownMostDerivedAsKnownMostDerivedAsync()
                    .whenComplete(
                            (result, ex) -> {
                                test(ex != null && ex instanceof KnownMostDerived);
                                KnownMostDerived kmd = (KnownMostDerived) ex;
                                test("KnownMostDerived.b".equals(kmd.b));
                                test("KnownMostDerived.ki".equals(kmd.ki));
                                test("KnownMostDerived.kmd".equals(kmd.kmd));
                                test("::Test::KnownMostDerived".equals(kmd.ice_id()));
                                cb.called();
                            });
            cb.check();
        }
        out.println("ok");

        out.print("slicing of unknown most derived, known intermediate as base... ");
        out.flush();
        {
            try {
                test.unknownMostDerived1AsBase();
                test(false);
            } catch (KnownIntermediate ki) {
                test("UnknownMostDerived1.b".equals(ki.b));
                test("UnknownMostDerived1.ki".equals(ki.ki));
                test("::Test::KnownIntermediate".equals(ki.ice_id()));
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("slicing of unknown most derived, known intermediate as base (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.unknownMostDerived1AsBaseAsync()
                    .whenComplete(
                            (result, ex) -> {
                                test(ex != null && ex instanceof KnownIntermediate);
                                KnownIntermediate ki = (KnownIntermediate) ex;
                                test("UnknownMostDerived1.b".equals(ki.b));
                                test("UnknownMostDerived1.ki".equals(ki.ki));
                                test("::Test::KnownIntermediate".equals(ki.ice_id()));
                                cb.called();
                            });
            cb.check();
        }
        out.println("ok");

        out.print("slicing of unknown most derived, known intermediate as intermediate... ");
        out.flush();
        {
            try {
                test.unknownMostDerived1AsKnownIntermediate();
                test(false);
            } catch (KnownIntermediate ki) {
                test("UnknownMostDerived1.b".equals(ki.b));
                test("UnknownMostDerived1.ki".equals(ki.ki));
                test("::Test::KnownIntermediate".equals(ki.ice_id()));
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("slicing of unknown most derived, known intermediate as intermediate (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.unknownMostDerived1AsKnownIntermediateAsync()
                    .whenComplete(
                            (result, ex) -> {
                                test(ex != null && ex instanceof KnownIntermediate);
                                KnownIntermediate ki = (KnownIntermediate) ex;
                                test("UnknownMostDerived1.b".equals(ki.b));
                                test("UnknownMostDerived1.ki".equals(ki.ki));
                                test("::Test::KnownIntermediate".equals(ki.ice_id()));
                                cb.called();
                            });
            cb.check();
        }
        out.println("ok");

        out.print("slicing of unknown most derived, unknown intermediate thrown as base... ");
        out.flush();
        {
            try {
                test.unknownMostDerived2AsBase();
                test(false);
            } catch (Base b) {
                test("UnknownMostDerived2.b".equals(b.b));
                test("::Test::Base".equals(b.ice_id()));
            } catch (Exception ex) {
                test(false);
            }
        }
        out.println("ok");

        out.print("slicing of unknown most derived, unknown intermediate thrown as base (AMI)... ");
        out.flush();
        {
            Callback cb = new Callback();
            test.unknownMostDerived2AsBaseAsync()
                    .whenComplete(
                            (result, ex) -> {
                                test(ex != null && ex instanceof Base);
                                Base b = (Base) ex;
                                test("UnknownMostDerived2.b".equals(b.b));
                                test("::Test::Base".equals(b.ice_id()));
                                cb.called();
                            });
            cb.check();
        }
        out.println("ok");

        return test;
    }

    private AllTests() {
    }
}
