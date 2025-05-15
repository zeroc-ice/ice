// Copyright (c) ZeroC, Inc.

package test.Ice.packagemd;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.MarshalException;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.UnknownUserException;

import test.Ice.packagemd.Test.InitialPrx;
import test.Ice.packagemd.Test1.C1;
import test.Ice.packagemd.Test1.C2;
import test.Ice.packagemd.Test1.E1;
import test.Ice.packagemd.Test1.E2;
import test.Ice.packagemd.Test1.notify;
import test.TestHelper;

import java.io.PrintWriter;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public static InitialPrx allTests(TestHelper helper) {
        Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();
        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "initial:" + helper.getTestEndpoint(0);
        ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        InitialPrx initial = InitialPrx.checkedCast(base);
        test(initial != null);
        test(initial.equals(base));
        out.println("ok");

        {
            out.print("testing types without package... ");
            out.flush();
            C1 c1 = initial.getTest1C2AsC1();
            test(c1 != null);
            test(c1 instanceof C2);
            C2 c2 = initial.getTest1C2AsC2();
            test(c2 != null);
            try {
                initial.throwTest1E2AsE1();
                test(false);
            } catch (E1 ex) {
                test(ex instanceof E2);
            }
            try {
                initial.throwTest1E2AsE2();
                test(false);
            } catch (E2 ex) {
                // Expected
            }
            try {
                initial.throwTest1Notify();
                test(false);
            } catch (notify ex) {
                // Expected
            }
            out.println("ok");
        }

        {
            out.print("testing types with package... ");
            out.flush();

            {
                try {
                    initial.throwTest2E2AsE1();
                    test(false);
                } catch (UnknownUserException ex) {
                    // Expected
                } catch (MarshalException ex) {
                    // Expected
                } catch (test.Ice.packagemd.testpkg.Test2.E1 ex) {
                    test(false);
                }
                try {
                    initial.throwTest2E2AsE2();
                    test(false);
                } catch (UnknownUserException ex) {
                    // Expected
                } catch (MarshalException ex) {
                    // Expected
                } catch (test.Ice.packagemd.testpkg.Test2.E1 ex) {
                    test(false);
                }
            }

            {
                var initData = new InitializationData();
                initData.properties = communicator.getProperties()._clone();
                initData.properties.setProperty("Ice.Package.Test2", "test.Ice.packagemd.testpkg");
                initData.properties.setProperty("Ice.Default.Package", "test.Ice.packagemd.modpkg");

                try (Communicator configuredCommunicator = helper.initialize(initData)) {
                    var proxy = InitialPrx.createProxy(configuredCommunicator, ref);

                    test.Ice.packagemd.testpkg.Test2.C1 c1 = proxy.getTest2C2AsC1();
                    test(c1 != null);
                    test(c1 instanceof test.Ice.packagemd.testpkg.Test2.C2);
                    test.Ice.packagemd.testpkg.Test2.C2 c2 = proxy.getTest2C2AsC2();
                    test(c2 != null);
                    try {
                        proxy.throwTest2E2AsE1();
                        test(false);
                    } catch (test.Ice.packagemd.testpkg.Test2.E1 ex) {
                        test(ex instanceof test.Ice.packagemd.testpkg.Test2.E2);
                    }
                    try {
                        proxy.throwTest2E2AsE2();
                        test(false);
                    } catch (test.Ice.packagemd.testpkg.Test2.E2 ex) {
                        // Expected
                    }

                    test.Ice.packagemd.modpkg.Test3.C1 c1bis = proxy.getTest3C2AsC1();
                    test(c1bis != null);
                    test(c1bis instanceof test.Ice.packagemd.modpkg.Test3.C2);
                    test.Ice.packagemd.modpkg.Test3.C2 c2bis = proxy.getTest3C2AsC2();
                    test(c2bis != null);
                    try {
                        proxy.throwTest3E2AsE1();
                        test(false);
                    } catch (test.Ice.packagemd.modpkg.Test3.E1 ex) {
                        test(ex instanceof test.Ice.packagemd.modpkg.Test3.E2);
                    }
                    try {
                        proxy.throwTest3E2AsE2();
                        test(false);
                    } catch (test.Ice.packagemd.modpkg.Test3.E2 ex) {
                        // Expected
                    }
                }
            }

            out.println("ok");
        }

        return initial;
    }

    private AllTests() {}
}
