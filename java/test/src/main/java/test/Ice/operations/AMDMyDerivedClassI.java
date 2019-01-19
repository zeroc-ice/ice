//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.operations;

import java.util.*;
import java.util.concurrent.CompletionStage;
import java.util.concurrent.CompletableFuture;

import com.zeroc.Ice.Current;

import test.Ice.operations.AMD.Test.*;

public final class AMDMyDerivedClassI implements MyDerivedClass
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    static class Thread_opVoid extends Thread
    {
        public Thread_opVoid(CompletableFuture<Void> f)
        {
            _future = f;
        }

        @Override
        public void run()
        {
            _future.complete((Void)null);
        }

        private CompletableFuture<Void> _future;
    }

    //
    // Override the Object "pseudo" operations to verify the operation mode.
    //

    @Override
    public boolean ice_isA(String id, Current current)
    {
        test(current.mode == com.zeroc.Ice.OperationMode.Nonmutating);
        return MyDerivedClass.super.ice_isA(id, current);
    }

    @Override
    public void ice_ping(Current current)
    {
        test(current.mode == com.zeroc.Ice.OperationMode.Nonmutating);
        MyDerivedClass.super.ice_ping(current);
    }

    @Override
    public String[] ice_ids(Current current)
    {
        test(current.mode == com.zeroc.Ice.OperationMode.Nonmutating);
        return MyDerivedClass.super.ice_ids(current);
    }

    @Override
    public String ice_id(Current current)
    {
        test(current.mode == com.zeroc.Ice.OperationMode.Nonmutating);
        return MyDerivedClass.super.ice_id(current);
    }

    @Override
    synchronized public CompletionStage<Void> shutdownAsync(Current current)
    {
        while(_opVoidThread != null)
        {
            try
            {
                _opVoidThread.join();
                _opVoidThread = null;
            }
            catch(java.lang.InterruptedException ex)
            {
            }
        }

        current.adapter.getCommunicator().shutdown();
        return CompletableFuture.completedFuture((Void)null);
    }

    @Override
    synchronized public CompletionStage<Boolean> supportsCompressAsync(Current current)
    {
        return CompletableFuture.completedFuture(true);
    }

    @Override
    synchronized public CompletionStage<Void> opVoidAsync(Current current)
    {
        test(current.mode == com.zeroc.Ice.OperationMode.Normal);

        while(_opVoidThread != null)
        {
            try
            {
                _opVoidThread.join();
                _opVoidThread = null;
            }
            catch(java.lang.InterruptedException ex)
            {
            }
        }

        CompletableFuture<Void> f = new CompletableFuture<>();
        _opVoidThread = new Thread_opVoid(f);
        _opVoidThread.start();
        return f;
    }

    @Override
    public CompletionStage<MyClass.OpBoolResult> opBoolAsync(boolean p1, boolean p2, Current current)
    {
        return CompletableFuture.completedFuture(new MyClass.OpBoolResult(p2, p1));
    }

    @Override
    public CompletionStage<MyClass.OpBoolSResult> opBoolSAsync(boolean[] p1, boolean[] p2, Current current)
    {
        MyClass.OpBoolSResult r = new MyClass.OpBoolSResult();
        r.p3 = new boolean[p1.length + p2.length];
        System.arraycopy(p1, 0, r.p3, 0, p1.length);
        System.arraycopy(p2, 0, r.p3, p1.length, p2.length);

        r.returnValue = new boolean[p1.length];
        for(int i = 0; i < p1.length; i++)
        {
            r.returnValue[i] = p1[p1.length - (i + 1)];
        }
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpBoolSSResult> opBoolSSAsync(boolean[][] p1, boolean[][] p2, Current current)
    {
        MyClass.OpBoolSSResult r = new MyClass.OpBoolSSResult();
        r.p3 = new boolean[p1.length + p2.length][];
        System.arraycopy(p1, 0, r.p3, 0, p1.length);
        System.arraycopy(p2, 0, r.p3, p1.length, p2.length);

        r.returnValue = new boolean[p1.length][];
        for(int i = 0; i < p1.length; i++)
        {
            r.returnValue[i] = p1[p1.length - (i + 1)];
        }
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpByteResult> opByteAsync(byte p1, byte p2, Current current)
    {
        return CompletableFuture.completedFuture(new MyClass.OpByteResult(p1, (byte) (p1 ^ p2)));
    }

    @Override
    public CompletionStage<MyClass.OpByteBoolDResult> opByteBoolDAsync(java.util.Map<Byte, Boolean> p1,
                                                                       java.util.Map<Byte, Boolean> p2,
                                                                       Current current)
    {
        MyClass.OpByteBoolDResult r = new MyClass.OpByteBoolDResult();
        r.p3 = p1;
        r.returnValue = new java.util.HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpByteSResult> opByteSAsync(byte[] p1, byte[] p2, Current current)
    {
        MyClass.OpByteSResult r = new MyClass.OpByteSResult();
        r.p3 = new byte[p1.length];
        for(int i = 0; i < p1.length; i++)
        {
            r.p3[i] = p1[p1.length - (i + 1)];
        }

        r.returnValue = new byte[p1.length + p2.length];
        System.arraycopy(p1, 0, r.returnValue, 0, p1.length);
        System.arraycopy(p2, 0, r.returnValue, p1.length, p2.length);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpByteSSResult> opByteSSAsync(byte[][] p1, byte[][] p2, Current current)
    {
        MyClass.OpByteSSResult r = new MyClass.OpByteSSResult();
        r.p3 = new byte[p1.length][];
        for(int i = 0; i < p1.length; i++)
        {
            r.p3[i] = p1[p1.length - (i + 1)];
        }

        r.returnValue = new byte[p1.length + p2.length][];
        System.arraycopy(p1, 0, r.returnValue, 0, p1.length);
        System.arraycopy(p2, 0, r.returnValue, p1.length, p2.length);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpFloatDoubleResult> opFloatDoubleAsync(float p1, double p2, Current current)
    {
        return CompletableFuture.completedFuture(new MyClass.OpFloatDoubleResult(p2, p1, p2));
    }

    @Override
    public CompletionStage<MyClass.OpFloatDoubleSResult> opFloatDoubleSAsync(float[] p1, double[] p2, Current current)
    {
        MyClass.OpFloatDoubleSResult r = new MyClass.OpFloatDoubleSResult();
        r.p3 = p1;
        r.p4 = new double[p2.length];
        for(int i = 0; i < p2.length; i++)
        {
            r.p4[i] = p2[p2.length - (i + 1)];
        }
        r.returnValue = new double[p2.length + p1.length];
        System.arraycopy(p2, 0, r.returnValue, 0, p2.length);
        for(int i = 0; i < p1.length; i++)
        {
            r.returnValue[p2.length + i] = p1[i];
        }
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpFloatDoubleSSResult> opFloatDoubleSSAsync(float[][] p1, double[][] p2,
                                                                               Current current)
    {
        MyClass.OpFloatDoubleSSResult r = new MyClass.OpFloatDoubleSSResult();
        r.p3 = p1;
        r.p4 = new double[p2.length][];
        for(int i = 0; i < p2.length; i++)
        {
            r.p4[i] = p2[p2.length - (i + 1)];
        }
        r.returnValue = new double[p2.length * 2][];
        System.arraycopy(p2, 0, r.returnValue, 0, p2.length);
        System.arraycopy(p2, 0, r.returnValue, p2.length, p2.length);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpLongFloatDResult> opLongFloatDAsync(java.util.Map<Long, Float> p1,
                                                                         java.util.Map<Long, Float> p2,
                                                                         Current current)
    {
        MyClass.OpLongFloatDResult r = new MyClass.OpLongFloatDResult();
        r.p3 = p1;
        r.returnValue = new java.util.HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpMyClassResult> opMyClassAsync(MyClassPrx p1, Current current)
    {
        MyClass.OpMyClassResult r = new MyClass.OpMyClassResult();
        r.p2 = p1;
        r.p3 = MyClassPrx.uncheckedCast(
                current.adapter.createProxy(com.zeroc.Ice.Util.stringToIdentity("noSuchIdentity")));
        r.returnValue = MyClassPrx.uncheckedCast(current.adapter.createProxy(current.id));
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpMyEnumResult> opMyEnumAsync(MyEnum p1, Current current)
    {
        return CompletableFuture.completedFuture(new MyClass.OpMyEnumResult(MyEnum.enum3, p1));
    }

    @Override
    public CompletionStage<MyClass.OpShortIntDResult> opShortIntDAsync(java.util.Map<Short, Integer> p1,
                                                                       java.util.Map<Short, Integer> p2,
                                                                       Current current)
    {
        MyClass.OpShortIntDResult r = new MyClass.OpShortIntDResult();
        r.p3 = p1;
        r.returnValue = new java.util.HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpShortIntLongResult> opShortIntLongAsync(short p1, int p2, long p3,
                                                                             Current current)
    {
        return CompletableFuture.completedFuture(new MyClass.OpShortIntLongResult(p3, p1, p2, p3));
    }

    @Override
    public CompletionStage<MyClass.OpShortIntLongSResult> opShortIntLongSAsync(short[] p1, int[] p2, long[] p3,
                                                                               Current current)
    {
        MyClass.OpShortIntLongSResult r = new MyClass.OpShortIntLongSResult();
        r.p4 = p1;
        r.p5 = new int[p2.length];
        for(int i = 0; i < p2.length; i++)
        {
            r.p5[i] = p2[p2.length - (i + 1)];
        }
        r.p6 = new long[p3.length * 2];
        System.arraycopy(p3, 0, r.p6, 0, p3.length);
        System.arraycopy(p3, 0, r.p6, p3.length, p3.length);
        r.returnValue = p3;
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpShortIntLongSSResult> opShortIntLongSSAsync(short[][] p1, int[][] p2, long[][] p3,
                                                                                 Current current)
    {
        MyClass.OpShortIntLongSSResult r = new MyClass.OpShortIntLongSSResult();
        r.p4 = p1;
        r.p5 = new int[p2.length][];
        for(int i = 0; i < p2.length; i++)
        {
            r.p5[i] = p2[p2.length - (i + 1)];
        }
        r.p6 = new long[p3.length * 2][];
        System.arraycopy(p3, 0, r.p6, 0, p3.length);
        System.arraycopy(p3, 0, r.p6, p3.length, p3.length);
        r.returnValue = p3;
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpStringResult> opStringAsync(String p1, String p2, Current current)
    {
        MyClass.OpStringResult r = new MyClass.OpStringResult();
        r.p3 = p2 + " " + p1;
        r.returnValue = p1 + " " + p2;
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpStringMyEnumDResult> opStringMyEnumDAsync(java.util.Map<String, MyEnum> p1,
                                                                               java.util.Map<String, MyEnum> p2,
                                                                               Current current)
    {
        MyClass.OpStringMyEnumDResult r = new MyClass.OpStringMyEnumDResult();
        r.p3 = p1;
        r.returnValue = new java.util.HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpMyEnumStringDResult> opMyEnumStringDAsync(java.util.Map<MyEnum, String> p1,
                                                                               java.util.Map<MyEnum, String> p2,
                                                                               Current current)
    {
        MyClass.OpMyEnumStringDResult r = new MyClass.OpMyEnumStringDResult();
        r.p3 = p1;
        r.returnValue = new java.util.HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpMyStructMyEnumDResult> opMyStructMyEnumDAsync(java.util.Map<MyStruct, MyEnum> p1,
                                                                                   java.util.Map<MyStruct, MyEnum> p2,
                                                                                   Current current)
    {
        MyClass.OpMyStructMyEnumDResult r = new MyClass.OpMyStructMyEnumDResult();
        r.p3 = p1;
        r.returnValue = new java.util.HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpByteBoolDSResult> opByteBoolDSAsync(List<Map<Byte, Boolean>> p1,
                                                                         List<Map<Byte, Boolean>> p2,
                                                                         Current current)
    {
        MyClass.OpByteBoolDSResult r = new MyClass.OpByteBoolDSResult();
        r.p3 = new ArrayList<>();
        r.p3.addAll(p2);
        r.p3.addAll(p1);

        r.returnValue = new ArrayList<>(p1);
        Collections.reverse(r.returnValue);

        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpShortIntDSResult> opShortIntDSAsync(List<Map<Short, Integer>> p1,
                                                                         List<Map<Short, Integer>> p2,
                                                                         Current current)
    {
        MyClass.OpShortIntDSResult r = new MyClass.OpShortIntDSResult();
        r.p3 = new ArrayList<>();
        r.p3.addAll(p2);
        r.p3.addAll(p1);

        r.returnValue = new ArrayList<>(p1);
        Collections.reverse(r.returnValue);

        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpLongFloatDSResult> opLongFloatDSAsync(List<Map<Long, Float>> p1,
                                                                           List<Map<Long, Float>> p2,
                                                                           Current current)
    {
        MyClass.OpLongFloatDSResult r = new MyClass.OpLongFloatDSResult();
        r.p3 = new ArrayList<>();
        r.p3.addAll(p2);
        r.p3.addAll(p1);

        r.returnValue = new ArrayList<>(p1);
        Collections.reverse(r.returnValue);

        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpStringStringDSResult> opStringStringDSAsync(List<Map<String, String>> p1,
                                                                                 List<Map<String, String>> p2,
                                                                                 Current current)
    {
        MyClass.OpStringStringDSResult r = new MyClass.OpStringStringDSResult();
        r.p3 = new ArrayList<>();
        r.p3.addAll(p2);
        r.p3.addAll(p1);

        r.returnValue = new ArrayList<>(p1);
        Collections.reverse(r.returnValue);

        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpStringMyEnumDSResult> opStringMyEnumDSAsync(List<Map<String, MyEnum>> p1,
                                                                                 List<Map<String, MyEnum>> p2,
                                                                                 Current current)
    {
        MyClass.OpStringMyEnumDSResult r = new MyClass.OpStringMyEnumDSResult();
        r.p3 = new ArrayList<>();
        r.p3.addAll(p2);
        r.p3.addAll(p1);

        r.returnValue = new ArrayList<>(p1);
        Collections.reverse(r.returnValue);

        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpMyEnumStringDSResult> opMyEnumStringDSAsync(List<Map<MyEnum, String>> p1,
                                                                                 List<Map<MyEnum, String>> p2,
                                                                                 Current current)
    {
        MyClass.OpMyEnumStringDSResult r = new MyClass.OpMyEnumStringDSResult();
        r.p3 = new ArrayList<>();
        r.p3.addAll(p2);
        r.p3.addAll(p1);

        r.returnValue = new ArrayList<>(p1);
        Collections.reverse(r.returnValue);

        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpMyStructMyEnumDSResult> opMyStructMyEnumDSAsync(List<Map<MyStruct, MyEnum>> p1,
                                                                                     List<Map<MyStruct, MyEnum>> p2,
                                                                                     Current current)
    {
        MyClass.OpMyStructMyEnumDSResult r = new MyClass.OpMyStructMyEnumDSResult();
        r.p3 = new ArrayList<>();
        r.p3.addAll(p2);
        r.p3.addAll(p1);

        r.returnValue = new ArrayList<>(p1);
        Collections.reverse(r.returnValue);

        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpByteByteSDResult> opByteByteSDAsync(Map<Byte, byte[]> p1, Map<Byte, byte[]> p2,
                                                                         Current current)
    {
        MyClass.OpByteByteSDResult r = new MyClass.OpByteByteSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpBoolBoolSDResult> opBoolBoolSDAsync(Map<Boolean, boolean[]> p1,
                                                                         Map<Boolean, boolean[]> p2,
                                                                         Current current)
    {
        MyClass.OpBoolBoolSDResult r = new MyClass.OpBoolBoolSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpShortShortSDResult> opShortShortSDAsync(Map<Short, short[]> p1,
                                                                             Map<Short, short[]> p2,
                                                                             Current current)
    {
        MyClass.OpShortShortSDResult r = new MyClass.OpShortShortSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpIntIntSDResult> opIntIntSDAsync(Map<Integer, int[]> p1, Map<Integer, int[]> p2,
                                                                     Current current)
    {
        MyClass.OpIntIntSDResult r = new MyClass.OpIntIntSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpLongLongSDResult> opLongLongSDAsync(Map<Long, long[]> p1, Map<Long, long[]> p2,
                                                                         Current current)
    {
        MyClass.OpLongLongSDResult r = new MyClass.OpLongLongSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpStringFloatSDResult> opStringFloatSDAsync(Map<String, float[]> p1,
                                                                               Map<String, float[]> p2,
                                                                               Current current)
    {
        MyClass.OpStringFloatSDResult r = new MyClass.OpStringFloatSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpStringDoubleSDResult> opStringDoubleSDAsync(Map<String, double[]> p1,
                                                                                 Map<String, double[]> p2,
                                                                                 Current current)
    {
        MyClass.OpStringDoubleSDResult r = new MyClass.OpStringDoubleSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpStringStringSDResult> opStringStringSDAsync(Map<String, String[]> p1,
                                                                                 Map<String, String[]> p2,
                                                                                 Current current)
    {
        MyClass.OpStringStringSDResult r = new MyClass.OpStringStringSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpMyEnumMyEnumSDResult> opMyEnumMyEnumSDAsync(Map<MyEnum, MyEnum[]> p1,
                                                                                 Map<MyEnum, MyEnum[]> p2,
                                                                                 Current current)
    {
        MyClass.OpMyEnumMyEnumSDResult r = new MyClass.OpMyEnumMyEnumSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<int[]> opIntSAsync(int[] s, Current current)
    {
        int[] r = new int[s.length];
        for(int i = 0; i < r.length; ++i)
        {
            r[i] = -s[i];
        }
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public synchronized CompletionStage<Void> opByteSOnewayAsync(byte[] s, Current current)
    {
        ++_opByteSOnewayCallCount;
        return CompletableFuture.completedFuture((Void)null);
    }

    @Override
    public synchronized CompletionStage<Integer> opByteSOnewayCallCountAsync(Current current)
    {
        int count = _opByteSOnewayCallCount;
        _opByteSOnewayCallCount = 0;
        return CompletableFuture.completedFuture(count);
    }

    @Override
    public CompletionStage<Map<String, String>> opContextAsync(Current current)
    {
        return CompletableFuture.completedFuture(current.ctx);
    }

    @Override
    public CompletionStage<Void> opDoubleMarshalingAsync(double p1, double[] p2, Current current)
    {
        double d = 1278312346.0 / 13.0;
        test(p1 == d);
        for(int i = 0; i < p2.length; ++i)
        {
            test(p2[i] == d);
        }
        return CompletableFuture.completedFuture((Void)null);
    }

    @Override
    public CompletionStage<MyClass.OpStringSResult> opStringSAsync(String[] p1, String[] p2, Current current)
    {
        MyClass.OpStringSResult r = new MyClass.OpStringSResult();
        r.p3 = new String[p1.length + p2.length];
        System.arraycopy(p1, 0, r.p3, 0, p1.length);
        System.arraycopy(p2, 0, r.p3, p1.length, p2.length);

        r.returnValue = new String[p1.length];
        for(int i = 0; i < p1.length; i++)
        {
            r.returnValue[i] = p1[p1.length - (i + 1)];
        }
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpStringSSResult> opStringSSAsync(String[][] p1, String[][] p2, Current current)
    {
        MyClass.OpStringSSResult r = new MyClass.OpStringSSResult();
        r.p3 = new String[p1.length + p2.length][];
        System.arraycopy(p1, 0, r.p3, 0, p1.length);
        System.arraycopy(p2, 0, r.p3, p1.length, p2.length);

        r.returnValue = new String[p2.length][];
        for(int i = 0; i < p2.length; i++)
        {
            r.returnValue[i] = p2[p2.length - (i + 1)];
        }
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpStringSSSResult> opStringSSSAsync(String[][][] p1, String[][][] p2,
                                                                       Current current)
    {
        MyClass.OpStringSSSResult r = new MyClass.OpStringSSSResult();
        r.p3 = new String[p1.length + p2.length][][];
        System.arraycopy(p1, 0, r.p3, 0, p1.length);
        System.arraycopy(p2, 0, r.p3, p1.length, p2.length);

        r.returnValue = new String[p2.length][][];
        for(int i = 0; i < p2.length; i++)
        {
            r.returnValue[i] = p2[p2.length - (i + 1)];
        }
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpStringStringDResult> opStringStringDAsync(java.util.Map<String, String> p1,
                                                                               java.util.Map<String, String> p2,
                                                                               Current current)
    {
        MyClass.OpStringStringDResult r = new MyClass.OpStringStringDResult();
        r.p3 = p1;
        r.returnValue = new java.util.HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<MyClass.OpStructResult> opStructAsync(Structure p1, Structure p2, Current current)
    {
        MyClass.OpStructResult r = new MyClass.OpStructResult();
        r.p3 = p1;
        r.p3.s.s = "a new string";
        r.returnValue = p2;
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<Void> opIdempotentAsync(Current current)
    {
        test(current.mode == com.zeroc.Ice.OperationMode.Idempotent);
        return CompletableFuture.completedFuture((Void)null);
    }

    @Override
    public CompletionStage<Void> opNonmutatingAsync(Current current)
    {
        test(current.mode == com.zeroc.Ice.OperationMode.Nonmutating);
        return CompletableFuture.completedFuture((Void)null);
    }

    @Override
    public CompletionStage<Void> opDerivedAsync(Current current)
    {
        return CompletableFuture.completedFuture((Void)null);
    }

    @Override
    public CompletionStage<Byte> opByte1Async(byte value, Current current)
    {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<Short> opShort1Async(short value, Current current)
    {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<Integer> opInt1Async(int value, Current current)
    {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<Long> opLong1Async(long value, Current current)
    {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<Float> opFloat1Async(float value, Current current)
    {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<Double> opDouble1Async(double value, Current current)
    {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<String> opString1Async(String value, Current current)
    {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<String[]> opStringS1Async(String[] value, Current current)
    {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<Map<Byte, Boolean>> opByteBoolD1Async(Map<Byte, Boolean> value, Current current)
    {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<String[]> opStringS2Async(String[] value, Current current)
    {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<Map<Byte, Boolean>> opByteBoolD2Async(Map<Byte, Boolean> value, Current current)
    {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<MyClass1> opMyClass1Async(MyClass1 value, Current current)
    {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<MyStruct1> opMyStruct1Async(MyStruct1 value, Current current)
    {
        return CompletableFuture.completedFuture(value);
    }

    @Override
    public CompletionStage<String[]> opStringLiteralsAsync(Current current)
    {
        return CompletableFuture.completedFuture(new String[]
            {
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
    public CompletionStage<String[]> opWStringLiteralsAsync(Current current)
    {
        return opStringLiteralsAsync(current);
    }

    @Override
    public CompletionStage<OpMStruct1MarshaledResult> opMStruct1Async(Current current)
    {
        return CompletableFuture.completedFuture(new OpMStruct1MarshaledResult(new Structure(), current));
    }

    @Override
    public CompletionStage<OpMStruct2MarshaledResult> opMStruct2Async(Structure p1, Current current)
    {
        return CompletableFuture.completedFuture(new OpMStruct2MarshaledResult(p1, p1, current));
    }

    @Override
    public CompletionStage<OpMSeq1MarshaledResult> opMSeq1Async(Current current)
    {
        return CompletableFuture.completedFuture(new OpMSeq1MarshaledResult(new String[0], current));
    }

    @Override
    public CompletionStage<OpMSeq2MarshaledResult> opMSeq2Async(String[] p1, Current current)
    {
        return CompletableFuture.completedFuture(new OpMSeq2MarshaledResult(p1, p1, current));
    }

    @Override
    public CompletionStage<OpMDict1MarshaledResult> opMDict1Async(Current current)
    {
        return CompletableFuture.completedFuture(new OpMDict1MarshaledResult(new HashMap<>(), current));
    }

    @Override
    public CompletionStage<OpMDict2MarshaledResult> opMDict2Async(Map<String, String> p1, Current current)
    {
        return CompletableFuture.completedFuture(new OpMDict2MarshaledResult(p1, p1, current));
    }

    private Thread _opVoidThread;
    private int _opByteSOnewayCallCount = 0;
}
