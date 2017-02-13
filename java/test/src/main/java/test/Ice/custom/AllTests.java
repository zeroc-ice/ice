// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

import test.Ice.custom.Test.C;
import test.Ice.custom.Test.E;
import test.Ice.custom.Test.S;
import test.Ice.custom.Test.TestIntf;
import test.Ice.custom.Test.TestIntfPrx;

public class AllTests
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static TestIntfPrx allTests(test.Util.Application app)
    {
        com.zeroc.Ice.Communicator communicator=app.communicator();
        PrintWriter out = app.getWriter();
        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "test:" + app.getTestEndpoint(0);
        com.zeroc.Ice.ObjectPrx obj = communicator.stringToProxy(ref);
        test(obj != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        TestIntfPrx t = TestIntfPrx.checkedCast(obj);
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
            TestIntf.OpCSeqResult seqR = t.opCSeq(seq);
            test(seqR.returnValue.length == seq.length);
            test(seqR.outSeq.length == seq.length);
            for(int i = 1; i < seq.length; i++)
            {
                test(seqR.returnValue[i] != null);
                test(seqR.returnValue[i] == seqR.returnValue[0]);
                test(seqR.returnValue[i] == seqR.outSeq[i]);
            }

            ArrayList<C> arr = new ArrayList<>(Arrays.asList(seq));
            TestIntf.OpCArrayResult arrR = t.opCArray(arr);
            test(arrR.returnValue.size() == arr.size());
            test(arrR.outSeq.size() == arr.size());
            for(int i = 1; i < arr.size(); i++)
            {
                test(arrR.returnValue.get(i) != null);
                test(arrR.returnValue.get(i) == arrR.returnValue.get(0));
                test(arrR.returnValue.get(i) == arrR.outSeq.get(i));
            }

            LinkedList<C> list = new LinkedList<>(Arrays.asList(seq));
            TestIntf.OpCListResult listR = t.opCList(list);
            test(listR.returnValue.size() == list.size());
            test(listR.outSeq.size() == list.size());
            for(int i = 1; i < list.size(); i++)
            {
                test(listR.returnValue.get(i) != null);
                test(listR.returnValue.get(i) == listR.returnValue.get(0));
                test(listR.returnValue.get(i) == listR.outSeq.get(i));
            }
        }

        {
            final Boolean[] seq = { Boolean.TRUE, Boolean.FALSE, Boolean.TRUE, Boolean.FALSE, Boolean.TRUE };
            ArrayList<Boolean> list = new ArrayList<>(Arrays.asList(seq));
            TestIntf.OpBoolSeqResult listR = t.opBoolSeq(list);
            test(listR.outSeq.equals(listR.returnValue));
            test(listR.outSeq.equals(list));
        }

        {
            final Byte[] seq = { new Byte((byte)0), new Byte((byte)1), new Byte((byte)2), new Byte((byte)3) };
            ArrayList<Byte> list = new ArrayList<>(Arrays.asList(seq));
            TestIntf.OpByteSeqResult listR = t.opByteSeq(list);
            test(listR.outSeq.equals(listR.returnValue));
            test(listR.outSeq.equals(list));
        }

        {
            final Short[] seq = { new Short((short)0), new Short((short)1), new Short((short)2), new Short((short)3) };
            ArrayList<Short> list = new ArrayList<>(Arrays.asList(seq));
            TestIntf.OpShortSeqResult listR = t.opShortSeq(list);
            test(listR.outSeq.equals(listR.returnValue));
            test(listR.outSeq.equals(list));
        }

        {
            final Integer[] seq = { new Integer(0), new Integer(1), new Integer(2), new Integer(3) };
            ArrayList<Integer> list = new ArrayList<>(Arrays.asList(seq));
            TestIntf.OpIntSeqResult listR = t.opIntSeq(list);
            test(listR.outSeq.equals(listR.returnValue));
            test(listR.outSeq.equals(list));
        }

        {
            final Long[] seq = { new Long(0), new Long(1), new Long(2), new Long(3) };
            ArrayList<Long> list = new ArrayList<>(Arrays.asList(seq));
            TestIntf.OpLongSeqResult listR = t.opLongSeq(list);
            test(listR.outSeq.equals(listR.returnValue));
            test(listR.outSeq.equals(list));
        }

        {
            final Float[] seq = { new Float(0), new Float(1), new Float(2), new Float(3) };
            ArrayList<Float> list = new ArrayList<>(Arrays.asList(seq));
            TestIntf.OpFloatSeqResult listR = t.opFloatSeq(list);
            test(listR.outSeq.equals(listR.returnValue));
            test(listR.outSeq.equals(list));
        }

        {
            final Double[] seq = { new Double(0), new Double(1), new Double(2), new Double(3) };
            ArrayList<Double> list = new ArrayList<>(Arrays.asList(seq));
            TestIntf.OpDoubleSeqResult listR = t.opDoubleSeq(list);
            test(listR.outSeq.equals(listR.returnValue));
            test(listR.outSeq.equals(list));
        }

        {
            final String[] seq = { "0", "1", "2", "3", "4" };
            ArrayList<String> list = new ArrayList<>(Arrays.asList(seq));
            TestIntf.OpStringSeqResult listR = t.opStringSeq(list);
            test(listR.outSeq.equals(listR.returnValue));
            test(listR.outSeq.equals(list));
        }

        {
            final E[] seq = { E.E1, E.E2, E.E3 };
            ArrayList<E> list = new ArrayList<>(Arrays.asList(seq));
            TestIntf.OpESeqResult listR = t.opESeq(list);
            test(listR.outSeq.equals(listR.returnValue));
            test(listR.outSeq.equals(list));
        }

        {
            S[] seq = new S[5];
            for(int i = 0; i < seq.length; i++)
            {
                seq[i] = new S();
                seq[i].en = E.values()[i % 3];
            }
            ArrayList<S> list = new ArrayList<>(Arrays.asList(seq));
            TestIntf.OpSSeqResult listR = t.opSSeq(list);
            test(listR.outSeq.equals(listR.returnValue));
            test(listR.outSeq.equals(list));
        }

        {
            ArrayList<Map<Integer, String>> list = new ArrayList<>();
            for(int i = 0; i < 5; i++)
            {
                Map<Integer, String> m = new HashMap<>();
                for(int j = 0; j < 4; j++)
                {
                    m.put(j, "" + j);
                }
                list.add(m);
            }
            TestIntf.OpDSeqResult listR = t.opDSeq(list);
            test(listR.outSeq.equals(listR.returnValue));
            test(listR.outSeq.equals(list));
        }

        {
            List<List<String>> seq = new LinkedList<>();
            for(int i = 0; i < 5; i++)
            {
                final String[] arr = { "0", "1", "2", "3", "4" };
                seq.add(new ArrayList<>(Arrays.asList(arr)));
            }
            TestIntf.OpStringSeqSeqResult listR = t.opStringSeqSeq(seq);
            test(listR.outSeq.equals(listR.returnValue));
            test(listR.outSeq.equals(seq));
        }

        {
            final byte[] fullSeq = new byte[] {0, 1, 2, 3, 4, 5, 6, 7};
            final byte[] usedSeq = new byte[] {2, 3, 4, 5};

            ByteBuffer buffer = ByteBuffer.wrap(fullSeq, 2, 4);
            TestIntf.OpByteBufferSeqResult bufferR = t.opByteBufferSeq(buffer);

            byte[] arr = new byte[bufferR.outSeq.limit()];
            bufferR.outSeq.get(arr, 0, bufferR.outSeq.limit());
            test(Arrays.equals(arr, usedSeq));
            arr = new byte[bufferR.returnValue.limit()];
            bufferR.returnValue.get(arr, 0, bufferR.returnValue.limit());
            test(Arrays.equals(arr, usedSeq));
        }

        {
            final short[] fullSeq = new short[] {0, 1, 2, 3, 4, 5, 6, 7};
            final short[] usedSeq = new short[] {2, 3, 4, 5};

            ShortBuffer buffer = ShortBuffer.wrap(fullSeq, 2, 4);
            TestIntf.OpShortBufferSeqResult bufferR = t.opShortBufferSeq(buffer);

            short[] arr = new short[bufferR.outSeq.limit()];
            bufferR.outSeq.get(arr, 0, bufferR.outSeq.limit());
            test(Arrays.equals(arr, usedSeq));
            arr = new short[bufferR.returnValue.limit()];
            bufferR.returnValue.get(arr, 0, bufferR.returnValue.limit());
            test(Arrays.equals(arr, usedSeq));
        }

        {
            final int[] fullSeq = new int[] {0, 1, 2, 3, 4, 5, 6, 7};
            final int[] usedSeq = new int[] {2, 3, 4, 5};

            IntBuffer buffer = IntBuffer.wrap(fullSeq, 2, 4);
            TestIntf.OpIntBufferSeqResult bufferR = t.opIntBufferSeq(buffer);

            int[] arr = new int[bufferR.outSeq.limit()];
            bufferR.outSeq.get(arr, 0, bufferR.outSeq.limit());
            test(Arrays.equals(arr, usedSeq));
            arr = new int[bufferR.returnValue.limit()];
            bufferR.returnValue.get(arr, 0, bufferR.returnValue.limit());
            test(Arrays.equals(arr, usedSeq));
        }

        {
            final long[] fullSeq = new long[] {0L, 1L, 2L, 3L, 4L, 5L, 6L, 7L};
            final long[] usedSeq = new long[] {2L, 3L, 4L, 5L};

            LongBuffer buffer = LongBuffer.wrap(fullSeq, 2, 4);
            TestIntf.OpLongBufferSeqResult bufferR = t.opLongBufferSeq(buffer);

            long[] arr = new long[bufferR.outSeq.limit()];
            bufferR.outSeq.get(arr, 0, bufferR.outSeq.limit());
            test(Arrays.equals(arr, usedSeq));
            arr = new long[bufferR.returnValue.limit()];
            bufferR.returnValue.get(arr, 0, bufferR.returnValue.limit());
            test(Arrays.equals(arr, usedSeq));
        }

        {
            final float[] fullSeq = new float[] {0, 1, 2, 3, 4, 5, 6, 7};
            final float[] usedSeq = new float[] {2, 3, 4, 5};

            FloatBuffer buffer = FloatBuffer.wrap(fullSeq, 2, 4);
            TestIntf.OpFloatBufferSeqResult bufferR = t.opFloatBufferSeq(buffer);

            float[] arr = new float[bufferR.outSeq.limit()];
            bufferR.outSeq.get(arr, 0, bufferR.outSeq.limit());
            test(Arrays.equals(arr, usedSeq));
            arr = new float[bufferR.returnValue.limit()];
            bufferR.returnValue.get(arr, 0, bufferR.returnValue.limit());
            test(Arrays.equals(arr, usedSeq));
        }

        {
            final double[] fullSeq = new double[] {0, 1, 2, 3, 4, 5, 6, 7};
            final double[] usedSeq = new double[] {2, 3, 4, 5};

            DoubleBuffer buffer = DoubleBuffer.wrap(fullSeq, 2, 4);
            TestIntf.OpDoubleBufferSeqResult bufferR = t.opDoubleBufferSeq(buffer);

            double[] arr = new double[bufferR.outSeq.limit()];
            bufferR.outSeq.get(arr, 0, bufferR.outSeq.limit());
            test(Arrays.equals(arr, usedSeq));
            arr = new double[bufferR.returnValue.limit()];
            bufferR.returnValue.get(arr, 0, bufferR.returnValue.limit());
            test(Arrays.equals(arr, usedSeq));
        }

        out.println("ok");

        return t;
    }
}
