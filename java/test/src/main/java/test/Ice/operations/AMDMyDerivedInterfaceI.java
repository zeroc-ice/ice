// Copyright (c) ZeroC, Inc.

package test.Ice.operations;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.OperationMode;

import test.Ice.operations.Test.AsyncMyDerivedInterface;
import test.Ice.operations.Test.MyClass1;
import test.Ice.operations.Test.MyEnum;
import test.Ice.operations.Test.MyInterface;
import test.Ice.operations.Test.MyInterfacePrx;
import test.Ice.operations.Test.MyStruct;
import test.Ice.operations.Test.MyStruct1;
import test.Ice.operations.Test.Structure;
import test.Ice.operations.Test.s0;
import test.Ice.operations.Test.s1;
import test.Ice.operations.Test.s10;
import test.Ice.operations.Test.s2;
import test.Ice.operations.Test.s3;
import test.Ice.operations.Test.s4;
import test.Ice.operations.Test.s5;
import test.Ice.operations.Test.s6;
import test.Ice.operations.Test.s7;
import test.Ice.operations.Test.s8;
import test.Ice.operations.Test.s9;
import test.Ice.operations.Test.ss0;
import test.Ice.operations.Test.ss1;
import test.Ice.operations.Test.ss2;
import test.Ice.operations.Test.ss3;
import test.Ice.operations.Test.ss4;
import test.Ice.operations.Test.ss5;
import test.Ice.operations.Test.su0;
import test.Ice.operations.Test.su1;
import test.Ice.operations.Test.su2;
import test.Ice.operations.Test.sw0;
import test.Ice.operations.Test.sw1;
import test.Ice.operations.Test.sw10;
import test.Ice.operations.Test.sw2;
import test.Ice.operations.Test.sw3;
import test.Ice.operations.Test.sw4;
import test.Ice.operations.Test.sw5;
import test.Ice.operations.Test.sw6;
import test.Ice.operations.Test.sw7;
import test.Ice.operations.Test.sw8;
import test.Ice.operations.Test.sw9;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

public final class AMDMyDerivedInterfaceI implements AsyncMyDerivedInterface {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    static class Thread_opVoid extends Thread {
        public Thread_opVoid(CompletableFuture<Void> f) {
            _future = f;
        }

        @Override
        public void run() {
            _future.complete((Void) null);
        }

        private CompletableFuture<Void> _future;
    }

    @Override
    public synchronized CompletionStage<Void> shutdownAsync(Current current) {
        while (_opVoidThread != null) {
            try {
                _opVoidThread.join();
                _opVoidThread = null;
            } catch (InterruptedException ex) {}
        }

        current.adapter.getCommunicator().shutdown();
        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public synchronized CompletionStage<Boolean> supportsCompressAsync(Current current) {
        return CompletableFuture.completedFuture(true);
    }

    @Override
    public synchronized CompletionStage<Void> opVoidAsync(Current current) {
        test(current.mode == OperationMode.Normal);

        while (_opVoidThread != null) {
            try {
                _opVoidThread.join();
                _opVoidThread = null;
            } catch (InterruptedException ex) {}
        }

        CompletableFuture<Void> f = new CompletableFuture<>();
        _opVoidThread = new Thread_opVoid(f);
        _opVoidThread.start();
        return f;
    }

    @Override
    public CompletionStage<MyInterface.OpBoolResult> opBoolAsync(
            boolean p1, boolean p2, Current current) {
        return CompletableFuture.completedFuture(new MyInterface.OpBoolResult(p2, p1));
    }

    @Override
    public CompletionStage<MyInterface.OpBoolSResult> opBoolSAsync(
            boolean[] p1, boolean[] p2, Current current) {
        MyInterface.OpBoolSResult r = new MyInterface.OpBoolSResult();
        r.p3 = new boolean[p1.length + p2.length];
        System.arraycopy(p1, 0, r.p3, 0, p1.length);
        System.arraycopy(p2, 0, r.p3, p1.length, p2.length);

        r.returnValue = new boolean[p1.length];
        for (int i = 0; i < p1.length; i++) {
            r.returnValue[i] = p1[p1.length - (i + 1)];
        }
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpBoolSSResult> opBoolSSAsync(
            boolean[][] p1, boolean[][] p2, Current current) {
        MyInterface.OpBoolSSResult r = new MyInterface.OpBoolSSResult();
        r.p3 = new boolean[p1.length + p2.length][];
        System.arraycopy(p1, 0, r.p3, 0, p1.length);
        System.arraycopy(p2, 0, r.p3, p1.length, p2.length);

        r.returnValue = new boolean[p1.length][];
        for (int i = 0; i < p1.length; i++) {
            r.returnValue[i] = p1[p1.length - (i + 1)];
        }
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpByteResult> opByteAsync(byte p1, byte p2, Current current) {
        return CompletableFuture.completedFuture(new MyInterface.OpByteResult(p1, (byte) (p1 ^ p2)));
    }

    @Override
    public CompletionStage<MyInterface.OpByteBoolDResult> opByteBoolDAsync(
            Map<Byte, Boolean> p1, Map<Byte, Boolean> p2, Current current) {
        MyInterface.OpByteBoolDResult r = new MyInterface.OpByteBoolDResult();
        r.p3 = p1;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpByteSResult> opByteSAsync(
            byte[] p1, byte[] p2, Current current) {
        MyInterface.OpByteSResult r = new MyInterface.OpByteSResult();
        r.p3 = new byte[p1.length];
        for (int i = 0; i < p1.length; i++) {
            r.p3[i] = p1[p1.length - (i + 1)];
        }

        r.returnValue = new byte[p1.length + p2.length];
        System.arraycopy(p1, 0, r.returnValue, 0, p1.length);
        System.arraycopy(p2, 0, r.returnValue, p1.length, p2.length);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpByteSSResult> opByteSSAsync(
            byte[][] p1, byte[][] p2, Current current) {
        MyInterface.OpByteSSResult r = new MyInterface.OpByteSSResult();
        r.p3 = new byte[p1.length][];
        for (int i = 0; i < p1.length; i++) {
            r.p3[i] = p1[p1.length - (i + 1)];
        }

        r.returnValue = new byte[p1.length + p2.length][];
        System.arraycopy(p1, 0, r.returnValue, 0, p1.length);
        System.arraycopy(p2, 0, r.returnValue, p1.length, p2.length);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpFloatDoubleResult> opFloatDoubleAsync(
            float p1, double p2, Current current) {
        return CompletableFuture.completedFuture(new MyInterface.OpFloatDoubleResult(p2, p1, p2));
    }

    @Override
    public CompletionStage<MyInterface.OpFloatDoubleSResult> opFloatDoubleSAsync(
            float[] p1, double[] p2, Current current) {
        MyInterface.OpFloatDoubleSResult r = new MyInterface.OpFloatDoubleSResult();
        r.p3 = p1;
        r.p4 = new double[p2.length];
        for (int i = 0; i < p2.length; i++) {
            r.p4[i] = p2[p2.length - (i + 1)];
        }
        r.returnValue = new double[p2.length + p1.length];
        System.arraycopy(p2, 0, r.returnValue, 0, p2.length);
        for (int i = 0; i < p1.length; i++) {
            r.returnValue[p2.length + i] = p1[i];
        }
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpFloatDoubleSSResult> opFloatDoubleSSAsync(
            float[][] p1, double[][] p2, Current current) {
        MyInterface.OpFloatDoubleSSResult r = new MyInterface.OpFloatDoubleSSResult();
        r.p3 = p1;
        r.p4 = new double[p2.length][];
        for (int i = 0; i < p2.length; i++) {
            r.p4[i] = p2[p2.length - (i + 1)];
        }
        r.returnValue = new double[p2.length * 2][];
        System.arraycopy(p2, 0, r.returnValue, 0, p2.length);
        System.arraycopy(p2, 0, r.returnValue, p2.length, p2.length);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpLongFloatDResult> opLongFloatDAsync(
            Map<Long, Float> p1, Map<Long, Float> p2, Current current) {
        MyInterface.OpLongFloatDResult r = new MyInterface.OpLongFloatDResult();
        r.p3 = p1;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpMyInterfaceResult> opMyInterfaceAsync(MyInterfacePrx p1, Current current) {
        MyInterface.OpMyInterfaceResult r = new MyInterface.OpMyInterfaceResult();
        r.p2 = p1;
        r.p3 =
            MyInterfacePrx.uncheckedCast(
                current.adapter.createProxy(
                    new Identity("noSuchIdentity", "")));
        r.returnValue = MyInterfacePrx.uncheckedCast(current.adapter.createProxy(current.id));
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpMyEnumResult> opMyEnumAsync(MyEnum p1, Current current) {
        return CompletableFuture.completedFuture(new MyInterface.OpMyEnumResult(MyEnum.enum3, p1));
    }

    @Override
    public CompletionStage<MyInterface.OpShortIntDResult> opShortIntDAsync(
            Map<Short, Integer> p1, Map<Short, Integer> p2, Current current) {
        MyInterface.OpShortIntDResult r = new MyInterface.OpShortIntDResult();
        r.p3 = p1;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpShortIntLongResult> opShortIntLongAsync(
            short p1, int p2, long p3, Current current) {
        return CompletableFuture.completedFuture(new MyInterface.OpShortIntLongResult(p3, p1, p2, p3));
    }

    @Override
    public CompletionStage<MyInterface.OpShortIntLongSResult> opShortIntLongSAsync(
            short[] p1, int[] p2, long[] p3, Current current) {
        MyInterface.OpShortIntLongSResult r = new MyInterface.OpShortIntLongSResult();
        r.p4 = p1;
        r.p5 = new int[p2.length];
        for (int i = 0; i < p2.length; i++) {
            r.p5[i] = p2[p2.length - (i + 1)];
        }
        r.p6 = new long[p3.length * 2];
        System.arraycopy(p3, 0, r.p6, 0, p3.length);
        System.arraycopy(p3, 0, r.p6, p3.length, p3.length);
        r.returnValue = p3;
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpShortIntLongSSResult> opShortIntLongSSAsync(
            short[][] p1, int[][] p2, long[][] p3, Current current) {
        MyInterface.OpShortIntLongSSResult r = new MyInterface.OpShortIntLongSSResult();
        r.p4 = p1;
        r.p5 = new int[p2.length][];
        for (int i = 0; i < p2.length; i++) {
            r.p5[i] = p2[p2.length - (i + 1)];
        }
        r.p6 = new long[p3.length * 2][];
        System.arraycopy(p3, 0, r.p6, 0, p3.length);
        System.arraycopy(p3, 0, r.p6, p3.length, p3.length);
        r.returnValue = p3;
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpStringResult> opStringAsync(
            String p1, String p2, Current current) {
        MyInterface.OpStringResult r = new MyInterface.OpStringResult();
        r.p3 = p2 + " " + p1;
        r.returnValue = p1 + " " + p2;
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpStringMyEnumDResult> opStringMyEnumDAsync(
            Map<String, MyEnum> p1, Map<String, MyEnum> p2, Current current) {
        MyInterface.OpStringMyEnumDResult r = new MyInterface.OpStringMyEnumDResult();
        r.p3 = p1;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpMyEnumStringDResult> opMyEnumStringDAsync(
            Map<MyEnum, String> p1, Map<MyEnum, String> p2, Current current) {
        MyInterface.OpMyEnumStringDResult r = new MyInterface.OpMyEnumStringDResult();
        r.p3 = p1;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpMyStructMyEnumDResult> opMyStructMyEnumDAsync(
            Map<MyStruct, MyEnum> p1,
            Map<MyStruct, MyEnum> p2,
            Current current) {
        MyInterface.OpMyStructMyEnumDResult r = new MyInterface.OpMyStructMyEnumDResult();
        r.p3 = p1;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpByteBoolDSResult> opByteBoolDSAsync(
            List<Map<Byte, Boolean>> p1, List<Map<Byte, Boolean>> p2, Current current) {
        MyInterface.OpByteBoolDSResult r = new MyInterface.OpByteBoolDSResult();
        r.p3 = new ArrayList<>();
        r.p3.addAll(p2);
        r.p3.addAll(p1);

        r.returnValue = new ArrayList<>(p1);
        Collections.reverse(r.returnValue);

        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpShortIntDSResult> opShortIntDSAsync(
            List<Map<Short, Integer>> p1, List<Map<Short, Integer>> p2, Current current) {
        MyInterface.OpShortIntDSResult r = new MyInterface.OpShortIntDSResult();
        r.p3 = new ArrayList<>();
        r.p3.addAll(p2);
        r.p3.addAll(p1);

        r.returnValue = new ArrayList<>(p1);
        Collections.reverse(r.returnValue);

        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpLongFloatDSResult> opLongFloatDSAsync(
            List<Map<Long, Float>> p1, List<Map<Long, Float>> p2, Current current) {
        MyInterface.OpLongFloatDSResult r = new MyInterface.OpLongFloatDSResult();
        r.p3 = new ArrayList<>();
        r.p3.addAll(p2);
        r.p3.addAll(p1);

        r.returnValue = new ArrayList<>(p1);
        Collections.reverse(r.returnValue);

        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpStringStringDSResult> opStringStringDSAsync(
            List<Map<String, String>> p1, List<Map<String, String>> p2, Current current) {
        MyInterface.OpStringStringDSResult r = new MyInterface.OpStringStringDSResult();
        r.p3 = new ArrayList<>();
        r.p3.addAll(p2);
        r.p3.addAll(p1);

        r.returnValue = new ArrayList<>(p1);
        Collections.reverse(r.returnValue);

        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpStringMyEnumDSResult> opStringMyEnumDSAsync(
            List<Map<String, MyEnum>> p1, List<Map<String, MyEnum>> p2, Current current) {
        MyInterface.OpStringMyEnumDSResult r = new MyInterface.OpStringMyEnumDSResult();
        r.p3 = new ArrayList<>();
        r.p3.addAll(p2);
        r.p3.addAll(p1);

        r.returnValue = new ArrayList<>(p1);
        Collections.reverse(r.returnValue);

        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpMyEnumStringDSResult> opMyEnumStringDSAsync(
            List<Map<MyEnum, String>> p1, List<Map<MyEnum, String>> p2, Current current) {
        MyInterface.OpMyEnumStringDSResult r = new MyInterface.OpMyEnumStringDSResult();
        r.p3 = new ArrayList<>();
        r.p3.addAll(p2);
        r.p3.addAll(p1);

        r.returnValue = new ArrayList<>(p1);
        Collections.reverse(r.returnValue);

        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpMyStructMyEnumDSResult> opMyStructMyEnumDSAsync(
            List<Map<MyStruct, MyEnum>> p1, List<Map<MyStruct, MyEnum>> p2, Current current) {
        MyInterface.OpMyStructMyEnumDSResult r = new MyInterface.OpMyStructMyEnumDSResult();
        r.p3 = new ArrayList<>();
        r.p3.addAll(p2);
        r.p3.addAll(p1);

        r.returnValue = new ArrayList<>(p1);
        Collections.reverse(r.returnValue);

        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpByteByteSDResult> opByteByteSDAsync(
            Map<Byte, byte[]> p1, Map<Byte, byte[]> p2, Current current) {
        MyInterface.OpByteByteSDResult r = new MyInterface.OpByteByteSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpBoolBoolSDResult> opBoolBoolSDAsync(
            Map<Boolean, boolean[]> p1, Map<Boolean, boolean[]> p2, Current current) {
        MyInterface.OpBoolBoolSDResult r = new MyInterface.OpBoolBoolSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpShortShortSDResult> opShortShortSDAsync(
            Map<Short, short[]> p1, Map<Short, short[]> p2, Current current) {
        MyInterface.OpShortShortSDResult r = new MyInterface.OpShortShortSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpIntIntSDResult> opIntIntSDAsync(
            Map<Integer, int[]> p1, Map<Integer, int[]> p2, Current current) {
        MyInterface.OpIntIntSDResult r = new MyInterface.OpIntIntSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpLongLongSDResult> opLongLongSDAsync(
            Map<Long, long[]> p1, Map<Long, long[]> p2, Current current) {
        MyInterface.OpLongLongSDResult r = new MyInterface.OpLongLongSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpStringFloatSDResult> opStringFloatSDAsync(
            Map<String, float[]> p1, Map<String, float[]> p2, Current current) {
        MyInterface.OpStringFloatSDResult r = new MyInterface.OpStringFloatSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpStringDoubleSDResult> opStringDoubleSDAsync(
            Map<String, double[]> p1, Map<String, double[]> p2, Current current) {
        MyInterface.OpStringDoubleSDResult r = new MyInterface.OpStringDoubleSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpStringStringSDResult> opStringStringSDAsync(
            Map<String, String[]> p1, Map<String, String[]> p2, Current current) {
        MyInterface.OpStringStringSDResult r = new MyInterface.OpStringStringSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpMyEnumMyEnumSDResult> opMyEnumMyEnumSDAsync(
            Map<MyEnum, MyEnum[]> p1, Map<MyEnum, MyEnum[]> p2, Current current) {
        MyInterface.OpMyEnumMyEnumSDResult r = new MyInterface.OpMyEnumMyEnumSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<int[]> opIntSAsync(int[] s, Current current) {
        int[] r = new int[s.length];
        for (int i = 0; i < r.length; i++) {
            r[i] = -s[i];
        }
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public synchronized CompletionStage<Void> opByteSOnewayAsync(byte[] s, Current current) {
        ++_opByteSOnewayCallCount;
        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public synchronized CompletionStage<Integer> opByteSOnewayCallCountAsync(Current current) {
        int count = _opByteSOnewayCallCount;
        _opByteSOnewayCallCount = 0;
        return CompletableFuture.completedFuture(count);
    }

    @Override
    public CompletionStage<Map<String, String>> opContextAsync(Current current) {
        return CompletableFuture.completedFuture(current.ctx);
    }

    @Override
    public CompletionStage<Void> opDoubleMarshalingAsync(double p1, double[] p2, Current current) {
        double d = 1278312346.0 / 13.0;
        test(p1 == d);
        for (int i = 0; i < p2.length; i++) {
            test(p2[i] == d);
        }
        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public CompletionStage<MyInterface.OpStringSResult> opStringSAsync(
            String[] p1, String[] p2, Current current) {
        MyInterface.OpStringSResult r = new MyInterface.OpStringSResult();
        r.p3 = new String[p1.length + p2.length];
        System.arraycopy(p1, 0, r.p3, 0, p1.length);
        System.arraycopy(p2, 0, r.p3, p1.length, p2.length);

        r.returnValue = new String[p1.length];
        for (int i = 0; i < p1.length; i++) {
            r.returnValue[i] = p1[p1.length - (i + 1)];
        }
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpStringSSResult> opStringSSAsync(
            String[][] p1, String[][] p2, Current current) {
        MyInterface.OpStringSSResult r = new MyInterface.OpStringSSResult();
        r.p3 = new String[p1.length + p2.length][];
        System.arraycopy(p1, 0, r.p3, 0, p1.length);
        System.arraycopy(p2, 0, r.p3, p1.length, p2.length);

        r.returnValue = new String[p2.length][];
        for (int i = 0; i < p2.length; i++) {
            r.returnValue[i] = p2[p2.length - (i + 1)];
        }
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpStringSSSResult> opStringSSSAsync(
            String[][][] p1, String[][][] p2, Current current) {
        MyInterface.OpStringSSSResult r = new MyInterface.OpStringSSSResult();
        r.p3 = new String[p1.length + p2.length][][];
        System.arraycopy(p1, 0, r.p3, 0, p1.length);
        System.arraycopy(p2, 0, r.p3, p1.length, p2.length);

        r.returnValue = new String[p2.length][][];
        for (int i = 0; i < p2.length; i++) {
            r.returnValue[i] = p2[p2.length - (i + 1)];
        }
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpStringStringDResult> opStringStringDAsync(
            Map<String, String> p1, Map<String, String> p2, Current current) {
        MyInterface.OpStringStringDResult r = new MyInterface.OpStringStringDResult();
        r.p3 = p1;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyInterface.OpStructResult> opStructAsync(
            Structure p1, Structure p2, Current current) {
        MyInterface.OpStructResult r = new MyInterface.OpStructResult();
        r.p3 = p1;
        r.p3.s.s = "a new string";
        r.returnValue = p2;
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<Void> opIdempotentAsync(Current current) {
        test(current.mode == OperationMode.Idempotent);
        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public CompletionStage<Void> opOnewayAsync(Current current) {
        test(current.requestId == 0);
        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public CompletionStage<Void> opDerivedAsync(Current current) {
        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public CompletionStage<Byte> opByte1Async(byte value, Current current) {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<Short> opShort1Async(short value, Current current) {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<Integer> opInt1Async(int value, Current current) {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<Long> opLong1Async(long value, Current current) {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<Float> opFloat1Async(float value, Current current) {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<Double> opDouble1Async(double value, Current current) {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<String> opString1Async(String value, Current current) {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<String[]> opStringS1Async(String[] value, Current current) {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<Map<Byte, Boolean>> opByteBoolD1Async(
            Map<Byte, Boolean> value, Current current) {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<String[]> opStringS2Async(String[] value, Current current) {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<Map<Byte, Boolean>> opByteBoolD2Async(
            Map<Byte, Boolean> value, Current current) {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<MyClass1> opMyClass1Async(MyClass1 value, Current current) {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<MyStruct1> opMyStruct1Async(MyStruct1 value, Current current) {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<String[]> opStringLiteralsAsync(Current current) {
        return CompletableFuture.completedFuture(
            new String[]{
                s0.value,
                s1.value,
                s2.value,
                s3.value,
                s4.value,
                s5.value,
                s6.value,
                s7.value,
                s8.value,
                s9.value,
                s10.value,
                sw0.value,
                sw1.value,
                sw2.value,
                sw3.value,
                sw4.value,
                sw5.value,
                sw6.value,
                sw7.value,
                sw8.value,
                sw9.value,
                sw10.value,
                ss0.value,
                ss1.value,
                ss2.value,
                ss3.value,
                ss4.value,
                ss5.value,
                su0.value,
                su1.value,
                su2.value
            });
    }

    @Override
    public CompletionStage<String[]> opWStringLiteralsAsync(Current current) {
        return opStringLiteralsAsync(current);
    }

    @Override
    public CompletionStage<OpMStruct1MarshaledResult> opMStruct1Async(Current current) {
        return CompletableFuture.completedFuture(
            new OpMStruct1MarshaledResult(new Structure(), current));
    }

    @Override
    public CompletionStage<OpMStruct2MarshaledResult> opMStruct2Async(
            Structure p1, Current current) {
        return CompletableFuture.completedFuture(new OpMStruct2MarshaledResult(p1, p1, current));
    }

    @Override
    public CompletionStage<OpMSeq1MarshaledResult> opMSeq1Async(Current current) {
        return CompletableFuture.completedFuture(
            new OpMSeq1MarshaledResult(new String[0], current));
    }

    @Override
    public CompletionStage<OpMSeq2MarshaledResult> opMSeq2Async(String[] p1, Current current) {
        return CompletableFuture.completedFuture(new OpMSeq2MarshaledResult(p1, p1, current));
    }

    @Override
    public CompletionStage<OpMDict1MarshaledResult> opMDict1Async(Current current) {
        return CompletableFuture.completedFuture(
            new OpMDict1MarshaledResult(new HashMap<>(), current));
    }

    @Override
    public CompletionStage<OpMDict2MarshaledResult> opMDict2Async(
            Map<String, String> p1, Current current) {
        return CompletableFuture.completedFuture(new OpMDict2MarshaledResult(p1, p1, current));
    }

    private Thread _opVoidThread;
    private int _opByteSOnewayCallCount;
}
