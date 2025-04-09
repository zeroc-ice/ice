// Copyright (c) ZeroC, Inc.

package test.Ice.scope;

import com.zeroc.Ice.Communicator;

import test.TestHelper;
import test.Ice.scope.Test.MyClass;
import test.Ice.scope.Test.MyEnum;
import test.Ice.scope.Test.MyInterface;
import test.Ice.scope.Test.MyInterfacePrx;
import test.Ice.scope.Test.MyOtherClass;
import test.Ice.scope.Test.MyOtherStruct;
import test.Ice.scope.Test.MyStruct;

import java.util.HashMap;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public static void allTests(TestHelper helper) {
        Communicator communicator = helper.communicator();

        {
            var i =
                MyInterfacePrx.createProxy(
                    communicator, "i1:" + helper.getTestEndpoint());

            var s1 = new MyStruct(0);
            MyInterface.OpMyStructResult opMyStructResult = i.opMyStruct(s1);
            test(s1.equals(opMyStructResult.returnValue));
            test(s1.equals(opMyStructResult.s2));

            var sseq1 = new MyStruct[]{s1};
            MyInterface.OpMyStructSeqResult opMyStructSeqResult =
                i.opMyStructSeq(sseq1);
            test(opMyStructSeqResult.returnValue[0].equals(s1));
            test(opMyStructSeqResult.s2[0].equals(s1));

            var smap1 = new HashMap<String, MyStruct>();
            smap1.put("a", s1);
            MyInterface.OpMyStructMapResult opMyStructMapResult =
                i.opMyStructMap(smap1);
            test(opMyStructMapResult.returnValue.get("a").equals(s1));
            test(opMyStructMapResult.s2.get("a").equals(s1));

            var c1 = new MyClass(s1);
            MyInterface.OpMyClassResult opMyClassResult = i.opMyClass(c1);
            test(c1.s.equals(opMyClassResult.returnValue.s));
            test(c1.s.equals(opMyClassResult.c2.s));

            var cseq1 = new MyClass[]{c1};
            MyInterface.OpMyClassSeqResult opMyClassSeqResult =
                i.opMyClassSeq(cseq1);
            test(opMyClassSeqResult.returnValue[0].s.equals(s1));
            test(opMyClassSeqResult.c2[0].s.equals(s1));

            var cmap1 = new HashMap<String, MyClass>();
            cmap1.put("a", c1);
            MyInterface.OpMyClassMapResult opMyClassMapResult =
                i.opMyClassMap(cmap1);
            test(opMyClassMapResult.returnValue.get("a").s.equals(s1));
            test(opMyClassMapResult.c2.get("a").s.equals(s1));

            MyEnum e = i.opMyEnum(MyEnum.v1);
            test(e == MyEnum.v1);

            MyOtherStruct s =
                i.opMyOtherStruct(new MyOtherStruct("MyOtherStruct"));
            test("MyOtherStruct".equals(s.s));

            MyOtherClass c =
                i.opMyOtherClass(new MyOtherClass("MyOtherClass"));
            test("MyOtherClass".equals(c.s));
        }

        {
            var i =
                MyInterfacePrx.createProxy(
                    communicator, "i1:" + helper.getTestEndpoint());

            var s1 = new MyStruct(0);
            MyInterface.OpMyStructResult opMyStructResult =
                i.opMyStructAsync(s1).join();
            test(s1.equals(opMyStructResult.returnValue));
            test(s1.equals(opMyStructResult.s2));

            var sseq1 = new MyStruct[]{s1};
            MyInterface.OpMyStructSeqResult opMyStructSeqResult =
                i.opMyStructSeqAsync(sseq1).join();
            test(opMyStructSeqResult.returnValue[0].equals(s1));
            test(opMyStructSeqResult.s2[0].equals(s1));

            var smap1 = new HashMap<String, MyStruct>();
            smap1.put("a", s1);
            MyInterface.OpMyStructMapResult opMyStructMapResult =
                i.opMyStructMapAsync(smap1).join();
            test(opMyStructMapResult.returnValue.get("a").equals(s1));
            test(opMyStructMapResult.s2.get("a").equals(s1));

            var c1 = new MyClass(s1);
            MyInterface.OpMyClassResult opMyClassResult =
                i.opMyClassAsync(c1).join();
            test(c1.s.equals(opMyClassResult.returnValue.s));
            test(c1.s.equals(opMyClassResult.c2.s));

            var cseq1 = new MyClass[]{c1};
            MyInterface.OpMyClassSeqResult opMyClassSeqResult =
                i.opMyClassSeqAsync(cseq1).join();
            test(opMyClassSeqResult.returnValue[0].s.equals(s1));
            test(opMyClassSeqResult.c2[0].s.equals(s1));

            var cmap1 = new HashMap<String, MyClass>();
            cmap1.put("a", c1);
            MyInterface.OpMyClassMapResult opMyClassMapResult =
                i.opMyClassMapAsync(cmap1).join();
            test(opMyClassMapResult.returnValue.get("a").s.equals(s1));
            test(opMyClassMapResult.c2.get("a").s.equals(s1));

            MyEnum e = i.opMyEnumAsync(MyEnum.v1).join();
            test(e == MyEnum.v1);

            MyOtherStruct s =
                i.opMyOtherStructAsync(new MyOtherStruct("MyOtherStruct"))
                    .join();
            test("MyOtherStruct".equals(s.s));

            MyOtherClass c =
                i.opMyOtherClassAsync(new MyOtherClass("MyOtherClass"))
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

            var smap1 = new HashMap<String, test.Ice.scope.Test.Inner.Inner2.MyStruct>();
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

            var cmap1 = new HashMap<String, test.Ice.scope.Test.Inner.Inner2.MyClass>();
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

            var smap1 = new HashMap<String, test.Ice.scope.Test.Inner.Inner2.MyStruct>();
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

            var cmap1 = new HashMap<String, test.Ice.scope.Test.Inner.Inner2.MyClass>();
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

            var smap1 = new HashMap<String, test.Ice.scope.Test.Inner.Inner2.MyStruct>();
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

            var cmap1 = new HashMap<String, test.Ice.scope.Test.Inner.Inner2.MyClass>();
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

            var smap1 = new HashMap<String, test.Ice.scope.Test.Inner.Inner2.MyStruct>();
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

            var cmap1 = new HashMap<String, test.Ice.scope.Test.Inner.Inner2.MyClass>();
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

            var s1 = new MyStruct(0);
            test.Ice.scope.Inner.Test.Inner2.MyInterface.OpMyStructResult opMyStructResult =
                i.opMyStructAsync(s1).join();
            test(s1.equals(opMyStructResult.returnValue));
            test(s1.equals(opMyStructResult.s2));

            var sseq1 = new MyStruct[]{s1};
            test.Ice.scope.Inner.Test.Inner2.MyInterface.OpMyStructSeqResult opMyStructSeqResult =
                i.opMyStructSeqAsync(sseq1).join();
            test(opMyStructSeqResult.returnValue[0].equals(s1));
            test(opMyStructSeqResult.s2[0].equals(s1));

            var smap1 = new HashMap<String, MyStruct>();
            smap1.put("a", s1);
            test.Ice.scope.Inner.Test.Inner2.MyInterface.OpMyStructMapResult opMyStructMapResult =
                i.opMyStructMapAsync(smap1).join();
            test(opMyStructMapResult.returnValue.get("a").equals(s1));
            test(opMyStructMapResult.s2.get("a").equals(s1));

            var c1 = new MyClass(s1);
            test.Ice.scope.Inner.Test.Inner2.MyInterface.OpMyClassResult opMyClassResult =
                i.opMyClassAsync(c1).join();
            test(c1.s.equals(opMyClassResult.returnValue.s));
            test(c1.s.equals(opMyClassResult.c2.s));

            var cseq1 = new MyClass[]{c1};
            test.Ice.scope.Inner.Test.Inner2.MyInterface.OpMyClassSeqResult opMyClassSeqResult =
                i.opMyClassSeqAsync(cseq1).join();
            test(opMyClassSeqResult.returnValue[0].s.equals(s1));
            test(opMyClassSeqResult.c2[0].s.equals(s1));

            var cmap1 = new HashMap<String, MyClass>();
            cmap1.put("a", c1);
            test.Ice.scope.Inner.Test.Inner2.MyInterface.OpMyClassMapResult opMyClassMapResult =
                i.opMyClassMapAsync(cmap1).join();
            test(opMyClassMapResult.returnValue.get("a").s.equals(s1));
            test(opMyClassMapResult.c2.get("a").s.equals(s1));
        }

        {
            var i =
                MyInterfacePrx.createProxy(
                    communicator, "i1:" + helper.getTestEndpoint());
            i.shutdown();
        }
    }

    private AllTests() {
    }
}
