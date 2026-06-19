// Copyright (c) ZeroC, Inc.

package test.Ice.customDictionary;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ObjectPrx;

import test.Ice.customDictionary.Test.A;
import test.Ice.customDictionary.Test.C;
import test.Ice.customDictionary.Test.E;
import test.Ice.customDictionary.Test.TestIntf;
import test.Ice.customDictionary.Test.TestIntfPrx;
import test.TestHelper;

import java.io.PrintWriter;
import java.util.Arrays;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.Map;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public static TestIntfPrx allTests(TestHelper helper) {
        Communicator communicator = helper.communicator();
        String ref = "test:" + helper.getTestEndpoint(0);
        ObjectPrx obj = communicator.stringToProxy(ref);
        test(obj != null);
        TestIntfPrx prx = TestIntfPrx.checkedCast(obj);
        test(prx != null);

        PrintWriter out = helper.getWriter();
        out.print("testing custom dictionaries... ");
        out.flush();

        {
            Map<Byte, Boolean> inDict = new LinkedHashMap<>();
            inDict.put((byte) 10, Boolean.TRUE);
            inDict.put((byte) 11, Boolean.FALSE);
            inDict.put((byte) 101, Boolean.TRUE);

            TestIntf.OpByteBoolLinkedDictResult r = prx.opByteBoolLinkedDict(inDict);

            test(r.outDict.getClass() == LinkedHashMap.class);
            test(r.outDict.equals(inDict));
            test(r.returnValue.getClass() == LinkedHashMap.class);
            test(r.returnValue.equals(inDict));
        }

        {
            Map<Short, Integer> inDict = new LinkedHashMap<>();
            inDict.put((short) 110, -1);
            inDict.put((short) 111, -100);
            inDict.put((short) 1101, 0);

            TestIntf.OpShortIntLinkedDictResult r = prx.opShortIntLinkedDict(inDict);

            test(r.outDict.getClass() == LinkedHashMap.class);
            test(r.outDict.equals(inDict));
            test(r.returnValue.getClass() == LinkedHashMap.class);
            test(r.returnValue.equals(inDict));
        }

        {
            LinkedHashMap<Long, Float> inDict = new LinkedHashMap<>();
            inDict.put(999999110L, -1.1f);
            inDict.put(999999120L, -100.4f);
            inDict.put(999999130L, 0.5f);

            TestIntf.OpLongFloatLinkedDictResult r = prx.opLongFloatLinkedDict(inDict);

            test(r.outDict.getClass() == LinkedHashMap.class);
            test(r.outDict.equals(inDict));
            test(r.returnValue.getClass() == LinkedHashMap.class);
            test(r.returnValue.equals(inDict));
        }

        {
            LinkedHashMap<String, Double> inDict = new LinkedHashMap<>();
            inDict.put("foo", -1.1d);
            inDict.put("FOO", -100.4d);
            inDict.put("BAR", 0.5d);

            TestIntf.OpStringDoubleLinkedDictResult r = prx.opStringDoubleLinkedDict(inDict);

            test(r.outDict.getClass() == LinkedHashMap.class);
            test(r.outDict.equals(inDict));
            test(r.returnValue.getClass() == LinkedHashMap.class);
            test(r.returnValue.equals(inDict));
        }

        {
            Map<A, Integer> inDict = new HashMap<>();
            inDict.put(new A(1), 2);
            inDict.put(new A(2), 2);
            inDict.put(new A(2), 3);

            TestIntf.OpAIntDictResult r = prx.opAIntDict(inDict);

            test(r.outDict.getClass() == HashMap.class);
            test(r.outDict.equals(inDict));
            test(r.returnValue.getClass() == HashMap.class);
            test(r.returnValue.equals(inDict));
        }

        {
            Map<A, Integer> inDict = new LinkedHashMap<>();
            inDict.put(new A(2), 3);
            inDict.put(new A(3), 3);
            inDict.put(new A(3), 4);

            TestIntf.OpAIntLinkedDictResult r = prx.opAIntLinkedDict(inDict);

            test(r.outDict.getClass() == LinkedHashMap.class);
            test(r.outDict.equals(inDict));
            test(r.returnValue.getClass() == LinkedHashMap.class);
            test(r.returnValue.equals(inDict));
        }

        {
            LinkedHashMap<A, Integer> inDict = new LinkedHashMap<>();
            inDict.put(new A(3), 4);
            inDict.put(new A(4), 4);
            inDict.put(new A(4), 5);

            TestIntf.OpAIntFormalLinkedDictResult r = prx.opAIntFormalLinkedDict(inDict);

            test(r.outDict.getClass() == LinkedHashMap.class);
            test(r.outDict.equals(inDict));
            test(r.returnValue.getClass() == LinkedHashMap.class);
            test(r.returnValue.equals(inDict));
        }

        {
            Map<Short, C> inDict = new HashMap<>();
            inDict.put((short) 0, new C("abc"));
            inDict.put((short) 1, new C("Hello!!"));
            inDict.put((short) 2, new C("qwerty"));

            TestIntf.OpShortCDictResult r = prx.opShortCDict(inDict);

            test(r.outDict.getClass() == HashMap.class);
            test(r.outDict.size() == 3);
            test("abc".equals(r.outDict.get((short) 0).s));
            test("Hello!!".equals(r.outDict.get((short) 1).s));
            test("qwerty".equals(r.outDict.get((short) 2).s));
            test(r.returnValue.getClass() == HashMap.class);
            test(r.returnValue.size() == 3);
            test("abc".equals(r.returnValue.get((short) 0).s));
            test("Hello!!".equals(r.returnValue.get((short) 1).s));
            test("qwerty".equals(r.returnValue.get((short) 2).s));
        }

        {
            Map<Short, C> inDict = new LinkedHashMap<>();
            inDict.put((short) 10, new C("abc!"));
            inDict.put((short) 11, new C("Hello!!!"));
            inDict.put((short) 12, new C("qwerty!"));

            TestIntf.OpShortCLinkedDictResult r = prx.opShortCLinkedDict(inDict);

            test(r.outDict.getClass() == LinkedHashMap.class);
            test(r.outDict.size() == 3);
            test("abc!".equals(r.outDict.get((short) 10).s));
            test("Hello!!!".equals(r.outDict.get((short) 11).s));
            test("qwerty!".equals(r.outDict.get((short) 12).s));
            test(r.returnValue.getClass() == LinkedHashMap.class);
            test(r.returnValue.size() == 3);
            test("abc!".equals(r.returnValue.get((short) 10).s));
            test("Hello!!!".equals(r.returnValue.get((short) 11).s));
            test("qwerty!".equals(r.returnValue.get((short) 12).s));
        }

        {
            LinkedHashMap<Short, C> inDict = new LinkedHashMap<>();
            inDict.put((short) 210, new C("abc!!"));
            inDict.put((short) 211, new C("Hello!!!!"));
            inDict.put((short) 212, new C("qwerty!!"));

            TestIntf.OpShortCFormalLinkedDictResult r = prx.opShortCFormalLinkedDict(inDict);

            test(r.outDict.getClass() == LinkedHashMap.class);
            test(r.outDict.size() == 3);
            test("abc!!".equals(r.outDict.get((short) 210).s));
            test("Hello!!!!".equals(r.outDict.get((short) 211).s));
            test("qwerty!!".equals(r.outDict.get((short) 212).s));
            test(r.returnValue.getClass() == LinkedHashMap.class);
            test(r.returnValue.size() == 3);
            test("abc!!".equals(r.returnValue.get((short) 210).s));
            test("Hello!!!!".equals(r.returnValue.get((short) 211).s));
            test("qwerty!!".equals(r.returnValue.get((short) 212).s));
        }

        {
            Map<String, E> inDict = new HashMap<>();
            inDict.put("i", E.E1);
            inDict.put("c", E.E2);
            inDict.put("e", E.E3);

            TestIntf.OpStringEDictResult r = prx.opStringEDict(inDict);

            test(r.outDict.getClass() == HashMap.class);
            test(r.outDict.equals(inDict));
            test(r.returnValue.getClass() == HashMap.class);
            test(r.returnValue.equals(inDict));
        }

        {
            Map<String, E> inDict = new LinkedHashMap<>();
            inDict.put("c", E.E2);
            inDict.put("e", E.E3);
            inDict.put("i", E.E1);

            TestIntf.OpStringELinkedDictResult r = prx.opStringELinkedDict(inDict);

            test(r.outDict.getClass() == LinkedHashMap.class);
            test(r.outDict.equals(inDict));
            test(r.returnValue.getClass() == LinkedHashMap.class);
            test(r.returnValue.equals(inDict));
        }

        {
            LinkedHashMap<String, E> inDict = new LinkedHashMap<>();
            inDict.put("e", E.E3);
            inDict.put("i", E.E1);
            inDict.put("c", E.E2);

            TestIntf.OpStringEFormalLinkedDictResult r = prx.opStringEFormalLinkedDict(inDict);

            test(r.outDict.getClass() == LinkedHashMap.class);
            test(r.outDict.equals(inDict));
            test(r.returnValue.getClass() == LinkedHashMap.class);
            test(r.returnValue.equals(inDict));
        }

        {
            String[] array1 = {"hello", "world", "!"};
            String[] array2 = {"!", "hello", "world"};
            String[] array3 = {"world", "!", "hello"};

            Map<E, String[]> inDict = new HashMap<>();
            inDict.put(E.E1, array1);
            inDict.put(E.E2, array2);
            inDict.put(E.E3, array3);

            TestIntf.OpEStringSeqDictResult r = prx.opEStringSeqDict(inDict);

            test(r.outDict.getClass() == HashMap.class);
            test(r.outDict.size() == 3);
            test(Arrays.equals(r.outDict.get(E.E1), array1));
            test(Arrays.equals(r.outDict.get(E.E2), array2));
            test(Arrays.equals(r.outDict.get(E.E3), array3));
            test(r.returnValue.getClass() == HashMap.class);
            test(r.returnValue.size() == 3);
            test(Arrays.equals(r.returnValue.get(E.E1), array1));
            test(Arrays.equals(r.returnValue.get(E.E2), array2));
            test(Arrays.equals(r.returnValue.get(E.E3), array3));
        }

        {
            String[] array1 = {"hello", "world", "!"};
            String[] array2 = {"!", "hello", "world"};
            String[] array3 = {"world", "!", "hello"};

            Map<E, String[]> inDict = new LinkedHashMap<>();
            inDict.put(E.E1, array1);
            inDict.put(E.E2, array2);
            inDict.put(E.E3, array3);

            TestIntf.OpEStringSeqLinkedDictResult r = prx.opEStringSeqLinkedDict(inDict);

            test(r.outDict.getClass() == LinkedHashMap.class);
            test(r.outDict.size() == 3);
            test(Arrays.equals(r.outDict.get(E.E1), array1));
            test(Arrays.equals(r.outDict.get(E.E2), array2));
            test(Arrays.equals(r.outDict.get(E.E3), array3));
            test(r.returnValue.getClass() == LinkedHashMap.class);
            test(r.returnValue.size() == 3);
            test(Arrays.equals(r.returnValue.get(E.E1), array1));
            test(Arrays.equals(r.returnValue.get(E.E2), array2));
            test(Arrays.equals(r.returnValue.get(E.E3), array3));
        }

        {
            String[] array1 = {"hello", "world", "!"};
            String[] array2 = {"!", "hello", "world"};
            String[] array3 = {"world", "!", "hello"};

            LinkedHashMap<E, String[]> inDict = new LinkedHashMap<>();
            inDict.put(E.E1, array1);
            inDict.put(E.E2, array2);
            inDict.put(E.E3, array3);

            TestIntf.OpEStringSeqFormalLinkedDictResult r = prx.opEStringSeqFormalLinkedDict(inDict);

            test(r.outDict.getClass() == LinkedHashMap.class);
            test(r.outDict.size() == 3);
            test(Arrays.equals(r.outDict.get(E.E1), array1));
            test(Arrays.equals(r.outDict.get(E.E2), array2));
            test(Arrays.equals(r.outDict.get(E.E3), array3));
            test(r.returnValue.getClass() == LinkedHashMap.class);
            test(r.returnValue.size() == 3);
            test(Arrays.equals(r.returnValue.get(E.E1), array1));
            test(Arrays.equals(r.returnValue.get(E.E2), array2));
            test(Arrays.equals(r.returnValue.get(E.E3), array3));
        }

        out.println("ok");
        out.print("testing optional custom dictionaries... ");
        out.flush();

        {
            Map<Byte, Boolean> inDict = new LinkedHashMap<>();
            inDict.put((byte) 10, Boolean.TRUE);
            inDict.put((byte) 11, Boolean.FALSE);
            inDict.put((byte) 101, Boolean.TRUE);

            TestIntf.OpOptByteBoolLinkedDictResult r = prx.opOptByteBoolLinkedDict(inDict);

            test(r.outDict.get().getClass() == LinkedHashMap.class);
            test(r.outDict.get().equals(inDict));
            test(r.returnValue.get().getClass() == LinkedHashMap.class);
            test(r.returnValue.get().equals(inDict));
        }

        {
            Map<Short, Integer> inDict = new LinkedHashMap<>();
            inDict.put((short) 110, -1);
            inDict.put((short) 111, -100);
            inDict.put((short) 1101, 0);

            TestIntf.OpOptShortIntLinkedDictResult r = prx.opOptShortIntLinkedDict(inDict);

            test(r.outDict.get().getClass() == LinkedHashMap.class);
            test(r.outDict.get().equals(inDict));
            test(r.returnValue.get().getClass() == LinkedHashMap.class);
            test(r.returnValue.get().equals(inDict));
        }

        {
            LinkedHashMap<Long, Float> inDict = new LinkedHashMap<>();
            inDict.put(999999110L, -1.1f);
            inDict.put(999999120L, -100.4f);
            inDict.put(999999130L, 0.5f);

            TestIntf.OpOptLongFloatLinkedDictResult r = prx.opOptLongFloatLinkedDict(inDict);

            test(r.outDict.get().getClass() == LinkedHashMap.class);
            test(r.outDict.get().equals(inDict));
            test(r.returnValue.get().getClass() == LinkedHashMap.class);
            test(r.returnValue.get().equals(inDict));
        }

        {
            LinkedHashMap<String, Double> inDict = new LinkedHashMap<>();
            inDict.put("foo", -1.1d);
            inDict.put("FOO", -100.4d);
            inDict.put("BAR", 0.5d);

            TestIntf.OpOptStringDoubleLinkedDictResult r = prx.opOptStringDoubleLinkedDict(inDict);

            test(r.outDict.get().getClass() == LinkedHashMap.class);
            test(r.outDict.get().equals(inDict));
            test(r.returnValue.get().getClass() == LinkedHashMap.class);
            test(r.returnValue.get().equals(inDict));
        }

        {
            Map<A, Integer> inDict = new HashMap<>();
            inDict.put(new A(1), 2);
            inDict.put(new A(2), 2);
            inDict.put(new A(2), 3);

            TestIntf.OpOptAIntDictResult r = prx.opOptAIntDict(inDict);

            test(r.outDict.get().getClass() == HashMap.class);
            test(r.outDict.get().equals(inDict));
            test(r.returnValue.get().getClass() == HashMap.class);
            test(r.returnValue.get().equals(inDict));
        }

        {
            Map<A, Integer> inDict = new LinkedHashMap<>();
            inDict.put(new A(2), 3);
            inDict.put(new A(3), 3);
            inDict.put(new A(3), 4);

            TestIntf.OpOptAIntLinkedDictResult r = prx.opOptAIntLinkedDict(inDict);

            test(r.outDict.get().getClass() == LinkedHashMap.class);
            test(r.outDict.get().equals(inDict));
            test(r.returnValue.get().getClass() == LinkedHashMap.class);
            test(r.returnValue.get().equals(inDict));
        }

        {
            LinkedHashMap<A, Integer> inDict = new LinkedHashMap<>();
            inDict.put(new A(3), 4);
            inDict.put(new A(4), 4);
            inDict.put(new A(4), 5);

            TestIntf.OpOptAIntFormalLinkedDictResult r = prx.opOptAIntFormalLinkedDict(inDict);

            test(r.outDict.get().getClass() == LinkedHashMap.class);
            test(r.outDict.get().equals(inDict));
            test(r.returnValue.get().getClass() == LinkedHashMap.class);
            test(r.returnValue.get().equals(inDict));
        }

        {
            Map<String, E> inDict = new HashMap<>();
            inDict.put("i", E.E1);
            inDict.put("c", E.E2);
            inDict.put("e", E.E3);

            TestIntf.OpOptStringEDictResult r = prx.opOptStringEDict(inDict);

            test(r.outDict.get().getClass() == HashMap.class);
            test(r.outDict.get().equals(inDict));
            test(r.returnValue.get().getClass() == HashMap.class);
            test(r.returnValue.get().equals(inDict));
        }

        {
            Map<String, E> inDict = new LinkedHashMap<>();
            inDict.put("c", E.E2);
            inDict.put("e", E.E3);
            inDict.put("i", E.E1);

            TestIntf.OpOptStringELinkedDictResult r = prx.opOptStringELinkedDict(inDict);

            test(r.outDict.get().getClass() == LinkedHashMap.class);
            test(r.outDict.get().equals(inDict));
            test(r.returnValue.get().getClass() == LinkedHashMap.class);
            test(r.returnValue.get().equals(inDict));
        }

        {
            LinkedHashMap<String, E> inDict = new LinkedHashMap<>();
            inDict.put("e", E.E3);
            inDict.put("i", E.E1);
            inDict.put("c", E.E2);

            TestIntf.OpOptStringEFormalLinkedDictResult r = prx.opOptStringEFormalLinkedDict(inDict);

            test(r.outDict.get().getClass() == LinkedHashMap.class);
            test(r.outDict.get().equals(inDict));
            test(r.returnValue.get().getClass() == LinkedHashMap.class);
            test(r.returnValue.get().equals(inDict));
        }

        {
            String[] array1 = {"hello", "world", "!"};
            String[] array2 = {"!", "hello", "world"};
            String[] array3 = {"world", "!", "hello"};

            Map<E, String[]> inDict = new HashMap<>();
            inDict.put(E.E1, array1);
            inDict.put(E.E2, array2);
            inDict.put(E.E3, array3);

            TestIntf.OpOptEStringSeqDictResult r = prx.opOptEStringSeqDict(inDict);

            test(r.outDict.get().getClass() == HashMap.class);
            test(r.outDict.get().size() == 3);
            test(Arrays.equals(r.outDict.get().get(E.E1), array1));
            test(Arrays.equals(r.outDict.get().get(E.E2), array2));
            test(Arrays.equals(r.outDict.get().get(E.E3), array3));
            test(r.returnValue.get().getClass() == HashMap.class);
            test(r.returnValue.get().size() == 3);
            test(Arrays.equals(r.returnValue.get().get(E.E1), array1));
            test(Arrays.equals(r.returnValue.get().get(E.E2), array2));
            test(Arrays.equals(r.returnValue.get().get(E.E3), array3));
        }

        {
            String[] array1 = {"hello", "world", "!"};
            String[] array2 = {"!", "hello", "world"};
            String[] array3 = {"world", "!", "hello"};

            Map<E, String[]> inDict = new LinkedHashMap<>();
            inDict.put(E.E1, array1);
            inDict.put(E.E2, array2);
            inDict.put(E.E3, array3);

            TestIntf.OpOptEStringSeqLinkedDictResult r = prx.opOptEStringSeqLinkedDict(inDict);

            test(r.outDict.get().getClass() == LinkedHashMap.class);
            test(r.outDict.get().size() == 3);
            test(Arrays.equals(r.outDict.get().get(E.E1), array1));
            test(Arrays.equals(r.outDict.get().get(E.E2), array2));
            test(Arrays.equals(r.outDict.get().get(E.E3), array3));
            test(r.returnValue.get().getClass() == LinkedHashMap.class);
            test(r.returnValue.get().size() == 3);
            test(Arrays.equals(r.returnValue.get().get(E.E1), array1));
            test(Arrays.equals(r.returnValue.get().get(E.E2), array2));
            test(Arrays.equals(r.returnValue.get().get(E.E3), array3));
        }

        {
            String[] array1 = {"hello", "world", "!"};
            String[] array2 = {"!", "hello", "world"};
            String[] array3 = {"world", "!", "hello"};

            LinkedHashMap<E, String[]> inDict = new LinkedHashMap<>();
            inDict.put(E.E1, array1);
            inDict.put(E.E2, array2);
            inDict.put(E.E3, array3);

            TestIntf.OpOptEStringSeqFormalLinkedDictResult r = prx.opOptEStringSeqFormalLinkedDict(inDict);

            test(r.outDict.get().getClass() == LinkedHashMap.class);
            test(r.outDict.get().size() == 3);
            test(Arrays.equals(r.outDict.get().get(E.E1), array1));
            test(Arrays.equals(r.outDict.get().get(E.E2), array2));
            test(Arrays.equals(r.outDict.get().get(E.E3), array3));
            test(r.returnValue.get().getClass() == LinkedHashMap.class);
            test(r.returnValue.get().size() == 3);
            test(Arrays.equals(r.returnValue.get().get(E.E1), array1));
            test(Arrays.equals(r.returnValue.get().get(E.E2), array2));
            test(Arrays.equals(r.returnValue.get().get(E.E3), array3));
        }

        out.println("ok");
        out.print("testing custom dictionaries with marshal result... ");
        out.flush();

        {
            LinkedHashMap<A, Integer> inDict = new LinkedHashMap<>();
            inDict.put(new A(1), 2);
            inDict.put(new A(2), 2);
            inDict.put(new A(2), 3);

            TestIntf.OpMAIntFormalLinkedDictResult r = prx.opMAIntFormalLinkedDict(inDict);

            test(r.outDict.getClass() == LinkedHashMap.class);
            test(r.outDict.equals(inDict));
            test(r.returnValue.getClass() == LinkedHashMap.class);
            test(r.returnValue.equals(inDict));
        }

        {
            String[] array1 = {"hello", "world", "!"};
            String[] array2 = {"!", "hello", "world"};
            String[] array3 = {"world", "!", "hello"};

            LinkedHashMap<E, String[]> inDict = new LinkedHashMap<>();
            inDict.put(E.E1, array1);
            inDict.put(E.E2, array2);
            inDict.put(E.E3, array3);

            TestIntf.OpMOptEStringSeqFormalLinkedDictResult r = prx.opMOptEStringSeqFormalLinkedDict(inDict);

            test(r.outDict.get().getClass() == LinkedHashMap.class);
            test(r.outDict.get().size() == 3);
            test(Arrays.equals(r.outDict.get().get(E.E1), array1));
            test(Arrays.equals(r.outDict.get().get(E.E2), array2));
            test(Arrays.equals(r.outDict.get().get(E.E3), array3));
            test(r.returnValue.get().getClass() == LinkedHashMap.class);
            test(r.returnValue.get().size() == 3);
            test(Arrays.equals(r.returnValue.get().get(E.E1), array1));
            test(Arrays.equals(r.returnValue.get().get(E.E2), array2));
            test(Arrays.equals(r.returnValue.get().get(E.E3), array3));
        }

        out.println("ok");

        return prx;
    }

    private AllTests() {}
}
