// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

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
    allTests(Ice.Communicator communicator)
    {
        System.out.print("testing stringToProxy... ");
        System.out.flush();
        String ref = "test:default -p 12345 -t 10000";
        Ice.ObjectPrx obj = communicator.stringToProxy(ref);
        test(obj != null);
        System.out.println("ok");

        System.out.print("testing checked cast... ");
        System.out.flush();
        TestIntfPrx t = TestIntfPrxHelper.checkedCast(obj);
        test(t != null);
        test(t.equals(obj));
        System.out.println("ok");

        System.out.print("testing custom sequences... ");
        System.out.flush();

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

            java.util.ArrayList arr = new java.util.ArrayList(java.util.Arrays.asList(seq));
            CArrayHolder arrH = new CArrayHolder();
            java.util.ArrayList arrR = t.opCArray(arr, arrH);
            test(arrR.size() == arr.size());
            test(arrH.value.size() == arr.size());
            for(int i = 1; i < arr.size(); i++)
            {
                test(arrR.get(i) != null);
                test(arrR.get(i) == arrR.get(0));
                test(arrR.get(i) == arrH.value.get(i));
            }

            java.util.LinkedList list = new java.util.LinkedList(java.util.Arrays.asList(seq));
            CListHolder listH = new CListHolder();
            java.util.LinkedList listR = t.opCList(list, listH);
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
            java.util.ArrayList list = new java.util.ArrayList(java.util.Arrays.asList(seq));
            BoolSeqHolder listH = new BoolSeqHolder();
            java.util.ArrayList listR = t.opBoolSeq(list, listH);
            test(listH.value.equals(listR));
            test(listH.value.equals(list));
        }

        {
            final Byte[] seq = { new Byte((byte)0), new Byte((byte)1), new Byte((byte)2), new Byte((byte)3) };
            java.util.ArrayList list = new java.util.ArrayList(java.util.Arrays.asList(seq));
            ByteSeqHolder listH = new ByteSeqHolder();
            java.util.ArrayList listR = t.opByteSeq(list, listH);
            test(listH.value.equals(listR));
            test(listH.value.equals(list));
        }

        {
            final Short[] seq = { new Short((short)0), new Short((short)1), new Short((short)2), new Short((short)3) };
            java.util.ArrayList list = new java.util.ArrayList(java.util.Arrays.asList(seq));
            ShortSeqHolder listH = new ShortSeqHolder();
            java.util.ArrayList listR = t.opShortSeq(list, listH);
            test(listH.value.equals(listR));
            test(listH.value.equals(list));
        }

        {
            final Integer[] seq = { new Integer(0), new Integer(1), new Integer(2), new Integer(3) };
            java.util.ArrayList list = new java.util.ArrayList(java.util.Arrays.asList(seq));
            IntSeqHolder listH = new IntSeqHolder();
            java.util.ArrayList listR = t.opIntSeq(list, listH);
            test(listH.value.equals(listR));
            test(listH.value.equals(list));
        }

        {
            final Long[] seq = { new Long(0), new Long(1), new Long(2), new Long(3) };
            java.util.ArrayList list = new java.util.ArrayList(java.util.Arrays.asList(seq));
            LongSeqHolder listH = new LongSeqHolder();
            java.util.ArrayList listR = t.opLongSeq(list, listH);
            test(listH.value.equals(listR));
            test(listH.value.equals(list));
        }

        {
            final Float[] seq = { new Float(0), new Float(1), new Float(2), new Float(3) };
            java.util.ArrayList list = new java.util.ArrayList(java.util.Arrays.asList(seq));
            FloatSeqHolder listH = new FloatSeqHolder();
            java.util.ArrayList listR = t.opFloatSeq(list, listH);
            test(listH.value.equals(listR));
            test(listH.value.equals(list));
        }

        {
            final Double[] seq = { new Double(0), new Double(1), new Double(2), new Double(3) };
            java.util.ArrayList list = new java.util.ArrayList(java.util.Arrays.asList(seq));
            DoubleSeqHolder listH = new DoubleSeqHolder();
            java.util.ArrayList listR = t.opDoubleSeq(list, listH);
            test(listH.value.equals(listR));
            test(listH.value.equals(list));
        }

        {
            final String[] seq = { "0", "1", "2", "3", "4" };
            java.util.ArrayList list = new java.util.ArrayList(java.util.Arrays.asList(seq));
            StringSeqHolder listH = new StringSeqHolder();
            java.util.ArrayList listR = t.opStringSeq(list, listH);
            test(listH.value.equals(listR));
            test(listH.value.equals(list));
        }

        {
            final E[] seq = { E.E1, E.E2, E.E3 };
            java.util.ArrayList list = new java.util.ArrayList(java.util.Arrays.asList(seq));
            ESeqHolder listH = new ESeqHolder();
            java.util.ArrayList listR = t.opESeq(list, listH);
            test(listH.value.equals(listR));
            test(listH.value.equals(list));
        }

        {
            S[] seq = new S[5];
            for(int i = 0; i < seq.length; i++)
            {
                seq[i] = new S();
                seq[i].en = E.convert(i % 3);
            }
            java.util.ArrayList list = new java.util.ArrayList(java.util.Arrays.asList(seq));
            SSeqHolder listH = new SSeqHolder();
            java.util.ArrayList listR = t.opSSeq(list, listH);
            test(listH.value.equals(listR));
            test(listH.value.equals(list));
        }

        {
            java.util.ArrayList list = new java.util.ArrayList();
            for(int i = 0; i < 5; i++)
            {
                java.util.HashMap m = new java.util.HashMap();
                for(int j = 0; j < 4; j++)
                {
                    m.put(new Integer(j), "" + j);
                }
                list.add(m);
            }
            DSeqHolder listH = new DSeqHolder();
            java.util.ArrayList listR = t.opDSeq(list, listH);
            test(listH.value.equals(listR));
            test(listH.value.equals(list));
        }

        {
            java.util.ArrayList[] seq = new java.util.ArrayList[5];
            for(int i = 0; i < 5; i++)
            {
                final String[] arr = { "0", "1", "2", "3", "4" };
                seq[i] = new java.util.ArrayList(java.util.Arrays.asList(arr));
            }
            StringSeqSeqHolder listH = new StringSeqSeqHolder();
            java.util.ArrayList[] listR = t.opStringSeqSeq(seq, listH);
            test(java.util.Arrays.equals(listH.value, listR));
            test(java.util.Arrays.equals(listH.value, seq));
        }

        System.out.println("ok");

        return t;
    }
}


