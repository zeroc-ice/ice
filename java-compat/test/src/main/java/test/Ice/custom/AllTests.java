// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.custom;

import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.nio.ByteBuffer;
import java.nio.ShortBuffer;
import java.nio.IntBuffer;
import java.nio.LongBuffer;
import java.nio.FloatBuffer;
import java.nio.DoubleBuffer;

import test.Ice.custom.Test.BoolSeqHolder;
import test.Ice.custom.Test.ByteSeqHolder;
import test.Ice.custom.Test.C;
import test.Ice.custom.Test.CArrayHolder;
import test.Ice.custom.Test.CListHolder;
import test.Ice.custom.Test.CSeqHolder;
import test.Ice.custom.Test.DSeqHolder;
import test.Ice.custom.Test.DoubleSeqHolder;
import test.Ice.custom.Test.E;
import test.Ice.custom.Test.ESeqHolder;
import test.Ice.custom.Test.FloatSeqHolder;
import test.Ice.custom.Test.IntSeqHolder;
import test.Ice.custom.Test.LongSeqHolder;
import test.Ice.custom.Test.S;
import test.Ice.custom.Test.SSeqHolder;
import test.Ice.custom.Test.ShortSeqHolder;
import test.Ice.custom.Test.StringSeqHolder;
import test.Ice.custom.Test.StringSeqSeqHolder;
import test.Ice.custom.Test.TestIntfPrx;
import test.Ice.custom.Test.TestIntfPrxHelper;

public class AllTests
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static TestIntfPrx
    allTests(test.TestHelper helper)
    {
        Ice.Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();
        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "test:" + helper.getTestEndpoint(0);
        Ice.ObjectPrx obj = communicator.stringToProxy(ref);
        test(obj != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        TestIntfPrx t = TestIntfPrxHelper.checkedCast(obj);
        test(t != null);
        test(t.equals(obj));
        out.println("ok");

        out.print("testing custom sequences... ");
        out.flush();

        {
            //
            // Create a sequence of C instances, where elements 1..n simply point to element 0.
            //
            C[] seq = new C[5];
            seq[0] = new C();
            for(int i = 1; i < seq.length; i++)
            {
                seq[i] = seq[0];
            }

            //
            // Invoke each operation and verify that the returned sequences have the same
            // structure as the original.
            //
            CSeqHolder seqH = new CSeqHolder();
            C[] seqR = t.opCSeq(seq, seqH);
            test(seqR.length == seq.length);
            test(seqH.value.length == seq.length);
            for(int i = 1; i < seq.length; i++)
            {
                test(seqR[i] != null);
                test(seqR[i] == seqR[0]);
                test(seqR[i] == seqH.value[i]);
            }

            ArrayList<C> arr = new ArrayList<C>(Arrays.asList(seq));
            CArrayHolder arrH = new CArrayHolder();
            List<C> arrR = t.opCArray(arr, arrH);
            test(arrR.size() == arr.size());
            test(arrH.value.size() == arr.size());
            for(int i = 1; i < arr.size(); i++)
            {
                test(arrR.get(i) != null);
                test(arrR.get(i) == arrR.get(0));
                test(arrR.get(i) == arrH.value.get(i));
            }

            LinkedList<C> list = new LinkedList<C>(Arrays.asList(seq));
            CListHolder listH = new CListHolder();
            List<C> listR = t.opCList(list, listH);
            test(listR.size() == list.size());
            test(listH.value.size() == list.size());
            for(int i = 1; i < list.size(); i++)
            {
                test(listR.get(i) != null);
                test(listR.get(i) == listR.get(0));
                test(listR.get(i) == listH.value.get(i));
            }
        }

        {
            final Boolean[] seq = { Boolean.TRUE, Boolean.FALSE, Boolean.TRUE, Boolean.FALSE, Boolean.TRUE };
            ArrayList<Boolean> list = new ArrayList<Boolean>(Arrays.asList(seq));
            BoolSeqHolder listH = new BoolSeqHolder();
            List<Boolean> listR = t.opBoolSeq(list, listH);
            test(listH.value.equals(listR));
            test(listH.value.equals(list));
        }

        {
            final Byte[] seq = { Byte.valueOf((byte)0), Byte.valueOf((byte)1), Byte.valueOf((byte)2),
                                 Byte.valueOf((byte)3) };
            ArrayList<Byte> list = new ArrayList<Byte>(Arrays.asList(seq));
            ByteSeqHolder listH = new ByteSeqHolder();
            List<Byte> listR = t.opByteSeq(list, listH);
            test(listH.value.equals(listR));
            test(listH.value.equals(list));
        }

        {
            final Short[] seq = { Short.valueOf((short)0), Short.valueOf((short)1), Short.valueOf((short)2),
                                  Short.valueOf((short)3) };
            ArrayList<Short> list = new ArrayList<Short>(Arrays.asList(seq));
            ShortSeqHolder listH = new ShortSeqHolder();
            List<Short> listR = t.opShortSeq(list, listH);
            test(listH.value.equals(listR));
            test(listH.value.equals(list));
        }

        {
            final Integer[] seq = { Integer.valueOf(0), Integer.valueOf(1), Integer.valueOf(2), Integer.valueOf(3) };
            ArrayList<Integer> list = new ArrayList<Integer>(Arrays.asList(seq));
            IntSeqHolder listH = new IntSeqHolder();
            List<Integer> listR = t.opIntSeq(list, listH);
            test(listH.value.equals(listR));
            test(listH.value.equals(list));
        }

        {
            final Long[] seq = { Long.valueOf(0), Long.valueOf(1), Long.valueOf(2), Long.valueOf(3) };
            ArrayList<Long> list = new ArrayList<Long>(Arrays.asList(seq));
            LongSeqHolder listH = new LongSeqHolder();
            List<Long> listR = t.opLongSeq(list, listH);
            test(listH.value.equals(listR));
            test(listH.value.equals(list));
        }

        {
            final Float[] seq = { Float.valueOf((float)0), Float.valueOf((float)1), Float.valueOf((float)2),
                                  Float.valueOf((float)3) };
            ArrayList<Float> list = new ArrayList<Float>(Arrays.asList(seq));
            FloatSeqHolder listH = new FloatSeqHolder();
            List<Float> listR = t.opFloatSeq(list, listH);
            test(listH.value.equals(listR));
            test(listH.value.equals(list));
        }

        {
            final Double[] seq = { Double.valueOf((double)0), Double.valueOf((double)1), Double.valueOf((double)2),
                                   Double.valueOf((double)3) };
            ArrayList<Double> list = new ArrayList<Double>(Arrays.asList(seq));
            DoubleSeqHolder listH = new DoubleSeqHolder();
            List<Double> listR = t.opDoubleSeq(list, listH);
            test(listH.value.equals(listR));
            test(listH.value.equals(list));
        }

        {
            final String[] seq = { "0", "1", "2", "3", "4" };
            ArrayList<String> list = new ArrayList<String>(Arrays.asList(seq));
            StringSeqHolder listH = new StringSeqHolder();
            List<String> listR = t.opStringSeq(list, listH);
            test(listH.value.equals(listR));
            test(listH.value.equals(list));
        }

        {
            final E[] seq = { E.E1, E.E2, E.E3 };
            ArrayList<E> list = new ArrayList<E>(Arrays.asList(seq));
            ESeqHolder listH = new ESeqHolder();
            List<E> listR = t.opESeq(list, listH);
            test(listH.value.equals(listR));
            test(listH.value.equals(list));
        }

        {
            S[] seq = new S[5];
            for(int i = 0; i < seq.length; i++)
            {
                seq[i] = new S();
                seq[i].en = E.values()[i % 3];
            }
            ArrayList<S> list = new ArrayList<S>(Arrays.asList(seq));
            SSeqHolder listH = new SSeqHolder();
            List<S> listR = t.opSSeq(list, listH);
            test(listH.value.equals(listR));
            test(listH.value.equals(list));
        }

        {
            ArrayList<Map<Integer, String>> list = new ArrayList<Map<Integer, String>>();
            for(int i = 0; i < 5; i++)
            {
                Map<Integer, String> m = new HashMap<Integer, String>();
                for(int j = 0; j < 4; j++)
                {
                    m.put(j, "" + j);
                }
                list.add(m);
            }
            DSeqHolder listH = new DSeqHolder();
            List<Map<Integer, String>> listR = t.opDSeq(list, listH);
            test(listH.value.equals(listR));
            test(listH.value.equals(list));
        }

        {
            List<List<String>> seq = new LinkedList<List<String>>();
            for(int i = 0; i < 5; i++)
            {
                final String[] arr = { "0", "1", "2", "3", "4" };
                seq.add(new ArrayList<String>(Arrays.asList(arr)));
            }
            StringSeqSeqHolder listH = new StringSeqSeqHolder();
            List<List<String>> listR = t.opStringSeqSeq(seq, listH);
            test(listH.value.equals(listR));
            test(listH.value.equals(seq));
        }

        {
            final byte[] fullSeq = new byte[] {0, 1, 2, 3, 4, 5, 6, 7};
            final byte[] usedSeq = new byte[] {2, 3, 4, 5};

            ByteBuffer buffer = ByteBuffer.wrap(fullSeq, 2, 4);
            Ice.Holder<ByteBuffer> bufferH = new Ice.Holder<ByteBuffer>();
            ByteBuffer bufferR = t.opByteBufferSeq(buffer, bufferH);

            byte[] arr = new byte[bufferH.value.limit()];
            bufferH.value.get(arr, 0, bufferH.value.limit());
            test(Arrays.equals(arr, usedSeq));
            arr = new byte[bufferR.limit()];
            bufferR.get(arr, 0, bufferR.limit());
            test(Arrays.equals(arr, usedSeq));
        }

        {
            final short[] fullSeq = new short[] {0, 1, 2, 3, 4, 5, 6, 7};
            final short[] usedSeq = new short[] {2, 3, 4, 5};

            ShortBuffer buffer = ShortBuffer.wrap(fullSeq, 2, 4);
            Ice.Holder<ShortBuffer> bufferH = new Ice.Holder<ShortBuffer>();
            ShortBuffer bufferR = t.opShortBufferSeq(buffer, bufferH);

            short[] arr = new short[bufferH.value.limit()];
            bufferH.value.get(arr, 0, bufferH.value.limit());
            test(Arrays.equals(arr, usedSeq));
            arr = new short[bufferR.limit()];
            bufferR.get(arr, 0, bufferR.limit());
            test(Arrays.equals(arr, usedSeq));
        }

        {
            final int[] fullSeq = new int[] {0, 1, 2, 3, 4, 5, 6, 7};
            final int[] usedSeq = new int[] {2, 3, 4, 5};

            IntBuffer buffer = IntBuffer.wrap(fullSeq, 2, 4);
            Ice.Holder<IntBuffer> bufferH = new Ice.Holder<IntBuffer>();
            IntBuffer bufferR = t.opIntBufferSeq(buffer, bufferH);

            int[] arr = new int[bufferH.value.limit()];
            bufferH.value.get(arr, 0, bufferH.value.limit());
            test(Arrays.equals(arr, usedSeq));
            arr = new int[bufferR.limit()];
            bufferR.get(arr, 0, bufferR.limit());
            test(Arrays.equals(arr, usedSeq));
        }

        {
            final long[] fullSeq = new long[] {0L, 1L, 2L, 3L, 4L, 5L, 6L, 7L};
            final long[] usedSeq = new long[] {2L, 3L, 4L, 5L};

            LongBuffer buffer = LongBuffer.wrap(fullSeq, 2, 4);
            Ice.Holder<LongBuffer> bufferH = new Ice.Holder<LongBuffer>();
            LongBuffer bufferR = t.opLongBufferSeq(buffer, bufferH);

            long[] arr = new long[bufferH.value.limit()];
            bufferH.value.get(arr, 0, bufferH.value.limit());
            test(Arrays.equals(arr, usedSeq));
            arr = new long[bufferR.limit()];
            bufferR.get(arr, 0, bufferR.limit());
            test(Arrays.equals(arr, usedSeq));
        }

        {
            final float[] fullSeq = new float[] {0, 1, 2, 3, 4, 5, 6, 7};
            final float[] usedSeq = new float[] {2, 3, 4, 5};

            FloatBuffer buffer = FloatBuffer.wrap(fullSeq, 2, 4);
            Ice.Holder<FloatBuffer> bufferH = new Ice.Holder<FloatBuffer>();
            FloatBuffer bufferR = t.opFloatBufferSeq(buffer, bufferH);

            float[] arr = new float[bufferH.value.limit()];
            bufferH.value.get(arr, 0, bufferH.value.limit());
            test(Arrays.equals(arr, usedSeq));
            arr = new float[bufferR.limit()];
            bufferR.get(arr, 0, bufferR.limit());
            test(Arrays.equals(arr, usedSeq));
        }

        {
            final double[] fullSeq = new double[] {0, 1, 2, 3, 4, 5, 6, 7};
            final double[] usedSeq = new double[] {2, 3, 4, 5};

            DoubleBuffer buffer = DoubleBuffer.wrap(fullSeq, 2, 4);
            Ice.Holder<DoubleBuffer> bufferH = new Ice.Holder<DoubleBuffer>();
            DoubleBuffer bufferR = t.opDoubleBufferSeq(buffer, bufferH);

            double[] arr = new double[bufferH.value.limit()];
            bufferH.value.get(arr, 0, bufferH.value.limit());
            test(Arrays.equals(arr, usedSeq));
            arr = new double[bufferR.limit()];
            bufferR.get(arr, 0, bufferR.limit());
            test(Arrays.equals(arr, usedSeq));
        }

        out.println("ok");

        return t;
    }
}
