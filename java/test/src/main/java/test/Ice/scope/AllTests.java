// Copyright (c) ZeroC, Inc.

package test.Ice.scope;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public static void allTests(test.TestHelper helper) {
        com.zeroc.Ice.Communicator communicator = helper.communicator();

        {
            var i =
                    test.Ice.scope.Test.MyInterfacePrx.createProxy(
                            communicator, "i1:" + helper.getTestEndpoint());

            var s1 = new test.Ice.scope.Test.MyStruct(0);
            test.Ice.scope.Test.MyInterface.OpMyStructResult opMyStructResult = i.opMyStruct(s1);
            test(s1.equals(opMyStructResult.returnValue));
            test(s1.equals(opMyStructResult.s2));

            var sseq1 = new test.Ice.scope.Test.MyStruct[]{s1};
            test.Ice.scope.Test.MyInterface.OpMyStructSeqResult opMyStructSeqResult =
                    i.opMyStructSeq(sseq1);
            test(opMyStructSeqResult.returnValue[0].equals(s1));
            test(opMyStructSeqResult.s2[0].equals(s1));

            var smap1 = new java.util.HashMap<String, test.Ice.scope.Test.MyStruct>();
            smap1.put("a", s1);
            test.Ice.scope.Test.MyInterface.OpMyStructMapResult opMyStructMapResult =
                    i.opMyStructMap(smap1);
            test(opMyStructMapResult.returnValue.get("a").equals(s1));
            test(opMyStructMapResult.s2.get("a").equals(s1));

            var c1 = new test.Ice.scope.Test.MyClass(s1);
            test.Ice.scope.Test.MyInterface.OpMyClassResult opMyClassResult = i.opMyClass(c1);
            test(c1.s.equals(opMyClassResult.returnValue.s));
            test(c1.s.equals(opMyClassResult.c2.s));

            var cseq1 = new test.Ice.scope.Test.MyClass[]{c1};
            test.Ice.scope.Test.MyInterface.OpMyClassSeqResult opMyClassSeqResult =
                    i.opMyClassSeq(cseq1);
            test(opMyClassSeqResult.returnValue[0].s.equals(s1));
            test(opMyClassSeqResult.c2[0].s.equals(s1));

            var cmap1 = new java.util.HashMap<String, test.Ice.scope.Test.MyClass>();
            cmap1.put("a", c1);
            test.Ice.scope.Test.MyInterface.OpMyClassMapResult opMyClassMapResult =
                    i.opMyClassMap(cmap1);
            test(opMyClassMapResult.returnValue.get("a").s.equals(s1));
            test(opMyClassMapResult.c2.get("a").s.equals(s1));

            test.Ice.scope.Test.MyEnum e = i.opMyEnum(test.Ice.scope.Test.MyEnum.v1);
            test(e == test.Ice.scope.Test.MyEnum.v1);

            test.Ice.scope.Test.MyOtherStruct s =
                    i.opMyOtherStruct(new test.Ice.scope.Test.MyOtherStruct("MyOtherStruct"));
            test("MyOtherStruct".equals(s.s));

            test.Ice.scope.Test.MyOtherClass c =
                    i.opMyOtherClass(new test.Ice.scope.Test.MyOtherClass("MyOtherClass"));
            test("MyOtherClass".equals(c.s));
        }

        {
            var i =
                    test.Ice.scope.Test.MyInterfacePrx.createProxy(
                            communicator, "i1:" + helper.getTestEndpoint());

            var s1 = new test.Ice.scope.Test.MyStruct(0);
            test.Ice.scope.Test.MyInterface.OpMyStructResult opMyStructResult =
                    i.opMyStructAsync(s1).join();
            test(s1.equals(opMyStructResult.returnValue));
            test(s1.equals(opMyStructResult.s2));

            var sseq1 = new test.Ice.scope.Test.MyStruct[]{s1};
            test.Ice.scope.Test.MyInterface.OpMyStructSeqResult opMyStructSeqResult =
                    i.opMyStructSeqAsync(sseq1).join();
            test(opMyStructSeqResult.returnValue[0].equals(s1));
            test(opMyStructSeqResult.s2[0].equals(s1));

            var smap1 = new java.util.HashMap<String, test.Ice.scope.Test.MyStruct>();
            smap1.put("a", s1);
            test.Ice.scope.Test.MyInterface.OpMyStructMapResult opMyStructMapResult =
                    i.opMyStructMapAsync(smap1).join();
            test(opMyStructMapResult.returnValue.get("a").equals(s1));
            test(opMyStructMapResult.s2.get("a").equals(s1));

            var c1 = new test.Ice.scope.Test.MyClass(s1);
            test.Ice.scope.Test.MyInterface.OpMyClassResult opMyClassResult =
                    i.opMyClassAsync(c1).join();
            test(c1.s.equals(opMyClassResult.returnValue.s));
            test(c1.s.equals(opMyClassResult.c2.s));

            var cseq1 = new test.Ice.scope.Test.MyClass[]{c1};
            test.Ice.scope.Test.MyInterface.OpMyClassSeqResult opMyClassSeqResult =
                    i.opMyClassSeqAsync(cseq1).join();
            test(opMyClassSeqResult.returnValue[0].s.equals(s1));
            test(opMyClassSeqResult.c2[0].s.equals(s1));

            var cmap1 = new java.util.HashMap<String, test.Ice.scope.Test.MyClass>();
            cmap1.put("a", c1);
            test.Ice.scope.Test.MyInterface.OpMyClassMapResult opMyClassMapResult =
                    i.opMyClassMapAsync(cmap1).join();
            test(opMyClassMapResult.returnValue.get("a").s.equals(s1));
            test(opMyClassMapResult.c2.get("a").s.equals(s1));

            test.Ice.scope.Test.MyEnum e = i.opMyEnumAsync(test.Ice.scope.Test.MyEnum.v1).join();
            test(e == test.Ice.scope.Test.MyEnum.v1);

            test.Ice.scope.Test.MyOtherStruct s =
                    i.opMyOtherStructAsync(new test.Ice.scope.Test.MyOtherStruct("MyOtherStruct"))
                            .join();
            test("MyOtherStruct".equals(s.s));

            test.Ice.scope.Test.MyOtherClass c =
                    i.opMyOtherClassAsync(new test.Ice.scope.Test.MyOtherClass("MyOtherClass"))
                            .join();
            test("MyOtherClass".equals(c.s));
        }

        {
            var i =
                    test.Ice.scope.Test.Inner.MyInterfacePrx.createProxy(
                            communicator, "i2:" + helper.getTestEndpoint());

            var s1 = new test.Ice.scope.Test.Inner.Inner2.MyStruct(0);
            test.Ice.scope.Test.Inner.MyInterface.OpMyStructResult opMyStructResult =
                    i.opMyStruct(s1);
            test(s1.equals(opMyStructResult.returnValue));
            test(s1.equals(opMyStructResult.s2));

            var sseq1 = new test.Ice.scope.Test.Inner.Inner2.MyStruct[]{s1};
            test.Ice.scope.Test.Inner.MyInterface.OpMyStructSeqResult opMyStructSeqResult =
                    i.opMyStructSeq(sseq1);
            test(opMyStructSeqResult.returnValue[0].equals(s1));
            test(opMyStructSeqResult.s2[0].equals(s1));

            var smap1 = new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.MyStruct>();
            smap1.put("a", s1);
            test.Ice.scope.Test.Inner.MyInterface.OpMyStructMapResult opMyStructMapResult =
                    i.opMyStructMap(smap1);
            test(opMyStructMapResult.returnValue.get("a").equals(s1));
            test(opMyStructMapResult.s2.get("a").equals(s1));

            var c1 = new test.Ice.scope.Test.Inner.Inner2.MyClass(s1);
            test.Ice.scope.Test.Inner.MyInterface.OpMyClassResult opMyClassResult = i.opMyClass(c1);
            test(c1.s.equals(opMyClassResult.returnValue.s));
            test(c1.s.equals(opMyClassResult.c2.s));

            var cseq1 = new test.Ice.scope.Test.Inner.Inner2.MyClass[]{c1};
            test.Ice.scope.Test.Inner.MyInterface.OpMyClassSeqResult opMyClassSeqResult =
                    i.opMyClassSeq(cseq1);
            test(opMyClassSeqResult.returnValue[0].s.equals(s1));
            test(opMyClassSeqResult.c2[0].s.equals(s1));

            var cmap1 = new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.MyClass>();
            cmap1.put("a", c1);
            test.Ice.scope.Test.Inner.MyInterface.OpMyClassMapResult opMyClassMapResult =
                    i.opMyClassMap(cmap1);
            test(opMyClassMapResult.returnValue.get("a").s.equals(s1));
            test(opMyClassMapResult.c2.get("a").s.equals(s1));
        }

        {
            var i =
                    test.Ice.scope.Test.Inner.MyInterfacePrx.createProxy(
                            communicator, "i2:" + helper.getTestEndpoint());

            var s1 = new test.Ice.scope.Test.Inner.Inner2.MyStruct(0);
            test.Ice.scope.Test.Inner.MyInterface.OpMyStructResult opMyStructResult =
                    i.opMyStructAsync(s1).join();
            test(s1.equals(opMyStructResult.returnValue));
            test(s1.equals(opMyStructResult.s2));

            var sseq1 = new test.Ice.scope.Test.Inner.Inner2.MyStruct[]{s1};
            test.Ice.scope.Test.Inner.MyInterface.OpMyStructSeqResult opMyStructSeqResult =
                    i.opMyStructSeqAsync(sseq1).join();
            test(opMyStructSeqResult.returnValue[0].equals(s1));
            test(opMyStructSeqResult.s2[0].equals(s1));

            var smap1 = new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.MyStruct>();
            smap1.put("a", s1);
            test.Ice.scope.Test.Inner.MyInterface.OpMyStructMapResult opMyStructMapResult =
                    i.opMyStructMapAsync(smap1).join();
            test(opMyStructMapResult.returnValue.get("a").equals(s1));
            test(opMyStructMapResult.s2.get("a").equals(s1));

            var c1 = new test.Ice.scope.Test.Inner.Inner2.MyClass(s1);
            test.Ice.scope.Test.Inner.MyInterface.OpMyClassResult opMyClassResult =
                    i.opMyClassAsync(c1).join();
            test(c1.s.equals(opMyClassResult.returnValue.s));
            test(c1.s.equals(opMyClassResult.c2.s));

            var cseq1 = new test.Ice.scope.Test.Inner.Inner2.MyClass[]{c1};
            test.Ice.scope.Test.Inner.MyInterface.OpMyClassSeqResult opMyClassSeqResult =
                    i.opMyClassSeqAsync(cseq1).join();
            test(opMyClassSeqResult.returnValue[0].s.equals(s1));
            test(opMyClassSeqResult.c2[0].s.equals(s1));

            var cmap1 = new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.MyClass>();
            cmap1.put("a", c1);
            test.Ice.scope.Test.Inner.MyInterface.OpMyClassMapResult opMyClassMapResult =
                    i.opMyClassMapAsync(cmap1).join();
            test(opMyClassMapResult.returnValue.get("a").s.equals(s1));
            test(opMyClassMapResult.c2.get("a").s.equals(s1));
        }

        {
            var i =
                    test.Ice.scope.Test.Inner.Inner2.MyInterfacePrx.createProxy(
                            communicator, "i3:" + helper.getTestEndpoint());

            var s1 = new test.Ice.scope.Test.Inner.Inner2.MyStruct(0);
            test.Ice.scope.Test.Inner.Inner2.MyInterface.OpMyStructResult opMyStructResult =
                    i.opMyStruct(s1);
            test(s1.equals(opMyStructResult.returnValue));
            test(s1.equals(opMyStructResult.s2));

            var sseq1 = new test.Ice.scope.Test.Inner.Inner2.MyStruct[]{s1};
            test.Ice.scope.Test.Inner.Inner2.MyInterface.OpMyStructSeqResult opMyStructSeqResult =
                    i.opMyStructSeq(sseq1);
            test(opMyStructSeqResult.returnValue[0].equals(s1));
            test(opMyStructSeqResult.s2[0].equals(s1));

            var smap1 = new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.MyStruct>();
            smap1.put("a", s1);
            test.Ice.scope.Test.Inner.Inner2.MyInterface.OpMyStructMapResult opMyStructMapResult =
                    i.opMyStructMap(smap1);
            test(opMyStructMapResult.returnValue.get("a").equals(s1));
            test(opMyStructMapResult.s2.get("a").equals(s1));

            var c1 = new test.Ice.scope.Test.Inner.Inner2.MyClass(s1);
            test.Ice.scope.Test.Inner.Inner2.MyInterface.OpMyClassResult opMyClassResult =
                    i.opMyClass(c1);
            test(c1.s.equals(opMyClassResult.returnValue.s));
            test(c1.s.equals(opMyClassResult.c2.s));

            var cseq1 = new test.Ice.scope.Test.Inner.Inner2.MyClass[]{c1};
            test.Ice.scope.Test.Inner.Inner2.MyInterface.OpMyClassSeqResult opMyClassSeqResult =
                    i.opMyClassSeq(cseq1);
            test(opMyClassSeqResult.returnValue[0].s.equals(s1));
            test(opMyClassSeqResult.c2[0].s.equals(s1));

            var cmap1 = new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.MyClass>();
            cmap1.put("a", c1);
            test.Ice.scope.Test.Inner.Inner2.MyInterface.OpMyClassMapResult opMyClassMapResult =
                    i.opMyClassMap(cmap1);
            test(opMyClassMapResult.returnValue.get("a").s.equals(s1));
            test(opMyClassMapResult.c2.get("a").s.equals(s1));
        }

        {
            var i =
                    test.Ice.scope.Test.Inner.Inner2.MyInterfacePrx.createProxy(
                            communicator, "i3:" + helper.getTestEndpoint());

            var s1 = new test.Ice.scope.Test.Inner.Inner2.MyStruct(0);
            test.Ice.scope.Test.Inner.Inner2.MyInterface.OpMyStructResult opMyStructResult =
                    i.opMyStructAsync(s1).join();
            test(s1.equals(opMyStructResult.returnValue));
            test(s1.equals(opMyStructResult.s2));

            var sseq1 = new test.Ice.scope.Test.Inner.Inner2.MyStruct[]{s1};
            test.Ice.scope.Test.Inner.Inner2.MyInterface.OpMyStructSeqResult opMyStructSeqResult =
                    i.opMyStructSeqAsync(sseq1).join();
            test(opMyStructSeqResult.returnValue[0].equals(s1));
            test(opMyStructSeqResult.s2[0].equals(s1));

            var smap1 = new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.MyStruct>();
            smap1.put("a", s1);
            test.Ice.scope.Test.Inner.Inner2.MyInterface.OpMyStructMapResult opMyStructMapResult =
                    i.opMyStructMapAsync(smap1).join();
            test(opMyStructMapResult.returnValue.get("a").equals(s1));
            test(opMyStructMapResult.s2.get("a").equals(s1));

            var c1 = new test.Ice.scope.Test.Inner.Inner2.MyClass(s1);
            test.Ice.scope.Test.Inner.Inner2.MyInterface.OpMyClassResult opMyClassResult =
                    i.opMyClassAsync(c1).join();
            test(c1.s.equals(opMyClassResult.returnValue.s));
            test(c1.s.equals(opMyClassResult.c2.s));

            var cseq1 = new test.Ice.scope.Test.Inner.Inner2.MyClass[]{c1};
            test.Ice.scope.Test.Inner.Inner2.MyInterface.OpMyClassSeqResult opMyClassSeqResult =
                    i.opMyClassSeqAsync(cseq1).join();
            test(opMyClassSeqResult.returnValue[0].s.equals(s1));
            test(opMyClassSeqResult.c2[0].s.equals(s1));

            var cmap1 = new java.util.HashMap<String, test.Ice.scope.Test.Inner.Inner2.MyClass>();
            cmap1.put("a", c1);
            test.Ice.scope.Test.Inner.Inner2.MyInterface.OpMyClassMapResult opMyClassMapResult =
                    i.opMyClassMapAsync(cmap1).join();
            test(opMyClassMapResult.returnValue.get("a").s.equals(s1));
            test(opMyClassMapResult.c2.get("a").s.equals(s1));
        }

        {
            var i =
                    test.Ice.scope.Inner.Test.Inner2.MyInterfacePrx.createProxy(
                            communicator, "i4:" + helper.getTestEndpoint());

            var s1 = new test.Ice.scope.Test.MyStruct(0);
            test.Ice.scope.Inner.Test.Inner2.MyInterface.OpMyStructResult opMyStructResult =
                    i.opMyStructAsync(s1).join();
            test(s1.equals(opMyStructResult.returnValue));
            test(s1.equals(opMyStructResult.s2));

            var sseq1 = new test.Ice.scope.Test.MyStruct[]{s1};
            test.Ice.scope.Inner.Test.Inner2.MyInterface.OpMyStructSeqResult opMyStructSeqResult =
                    i.opMyStructSeqAsync(sseq1).join();
            test(opMyStructSeqResult.returnValue[0].equals(s1));
            test(opMyStructSeqResult.s2[0].equals(s1));

            var smap1 = new java.util.HashMap<String, test.Ice.scope.Test.MyStruct>();
            smap1.put("a", s1);
            test.Ice.scope.Inner.Test.Inner2.MyInterface.OpMyStructMapResult opMyStructMapResult =
                    i.opMyStructMapAsync(smap1).join();
            test(opMyStructMapResult.returnValue.get("a").equals(s1));
            test(opMyStructMapResult.s2.get("a").equals(s1));

            var c1 = new test.Ice.scope.Test.MyClass(s1);
            test.Ice.scope.Inner.Test.Inner2.MyInterface.OpMyClassResult opMyClassResult =
                    i.opMyClassAsync(c1).join();
            test(c1.s.equals(opMyClassResult.returnValue.s));
            test(c1.s.equals(opMyClassResult.c2.s));

            var cseq1 = new test.Ice.scope.Test.MyClass[]{c1};
            test.Ice.scope.Inner.Test.Inner2.MyInterface.OpMyClassSeqResult opMyClassSeqResult =
                    i.opMyClassSeqAsync(cseq1).join();
            test(opMyClassSeqResult.returnValue[0].s.equals(s1));
            test(opMyClassSeqResult.c2[0].s.equals(s1));

            var cmap1 = new java.util.HashMap<String, test.Ice.scope.Test.MyClass>();
            cmap1.put("a", c1);
            test.Ice.scope.Inner.Test.Inner2.MyInterface.OpMyClassMapResult opMyClassMapResult =
                    i.opMyClassMapAsync(cmap1).join();
            test(opMyClassMapResult.returnValue.get("a").s.equals(s1));
            test(opMyClassMapResult.c2.get("a").s.equals(s1));
        }

        {
            var i =
                    test.Ice.scope.Test.MyInterfacePrx.createProxy(
                            communicator, "i1:" + helper.getTestEndpoint());
            i.shutdown();
        }
    }

    private AllTests() {
    }
}
