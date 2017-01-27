// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using Test;

class Twoways
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new Exception();
        }
    }

    static int _length = 100;

    internal static void twoways(Ice.Communicator communicator, Test.MyClassPrx p)
    {
        {
            byte[] i = new byte[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = (byte)c;
            }
            byte[] o;
            byte[] r;

            r = p.opAByteS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            List<byte> i = new List<byte>(_length);
            for(int c = 0; c < _length; ++c)
            {
                i.Add((byte)c);
            }
            List<byte> o;
            List<byte> r;

            r = p.opLByteS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            LinkedList<byte> i = new LinkedList<byte>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast((byte)c);
            }
            LinkedList<byte> o;
            LinkedList<byte> r;

            r = p.opKByteS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Queue<byte> i = new Queue<byte>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue((byte)c);
            }
            Queue<byte> o;
            Queue<byte> r;

            r = p.opQByteS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Stack<byte> i = new Stack<byte>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push((byte)c);
            }
            Stack<byte> o;
            Stack<byte> r;

            r = p.opSByteS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            bool[] i = new bool[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = c % 1 == 1;
            }
            bool[] o;
            bool[] r;

            r = p.opABoolS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            List<bool> i = new List<bool>(_length);
            for(int c = 0; c < _length; ++c)
            {
                i.Add(c % 1 == 1);
            }
            List<bool> o;
            List<bool> r;

            r = p.opLBoolS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            LinkedList<bool> i = new LinkedList<bool>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast(c % 1 == 1);
            }
            LinkedList<bool> o;
            LinkedList<bool> r;

            r = p.opKBoolS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Queue<bool> i = new Queue<bool>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue(c % 1 == 1);
            }
            Queue<bool> o;
            Queue<bool> r;

            r = p.opQBoolS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Stack<bool> i = new Stack<bool>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push(c % 1 == 1);
            }
            Stack<bool> o;
            Stack<bool> r;

            r = p.opSBoolS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            short[] i = new short[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = (short)c;
            }
            short[] o;
            short[] r;

            {
                r = p.opAShortS(i, out o);
            }
            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            List<short> i = new List<short>(_length);
            for(int c = 0; c < _length; ++c)
            {
                i.Add((short)c);
            }
            List<short> o;
            List<short> r;

            r = p.opLShortS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            LinkedList<short> i = new LinkedList<short>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast((short)c);
            }
            LinkedList<short> o;
            LinkedList<short> r;

            r = p.opKShortS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Queue<short> i = new Queue<short>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue((short)c);
            }
            Queue<short> o;
            Queue<short> r;

            r = p.opQShortS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Stack<short> i = new Stack<short>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push((short)c);
            }
            Stack<short> o;
            Stack<short> r;

            r = p.opSShortS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            int[] i = new int[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = (int)c;
            }
            int[] o;
            int[] r;

            r = p.opAIntS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            List<int> i = new List<int>(_length);
            for(int c = 0; c < _length; ++c)
            {
                i.Add((int)c);
            }
            List<int> o;
            List<int> r;

            r = p.opLIntS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            LinkedList<int> i = new LinkedList<int>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast((int)c);
            }
            LinkedList<int> o;
            LinkedList<int> r;

            r = p.opKIntS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Queue<int> i = new Queue<int>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue((int)c);
            }
            Queue<int> o;
            Queue<int> r;

            r = p.opQIntS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Stack<int> i = new Stack<int>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push((int)c);
            }
            Stack<int> o;
            Stack<int> r;

            r = p.opSIntS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            long[] i = new long[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = (long)c;
            }
            long[] o;
            long[] r;

            r = p.opALongS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            List<long> i = new List<long>(_length);
            for(int c = 0; c < _length; ++c)
            {
                i.Add((long)c);
            }
            List<long> o;
            List<long> r;

            r = p.opLLongS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            LinkedList<long> i = new LinkedList<long>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast((long)c);
            }
            LinkedList<long> o;
            LinkedList<long> r;

            r = p.opKLongS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Queue<long> i = new Queue<long>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue((long)c);
            }
            Queue<long> o;
            Queue<long> r;

            r = p.opQLongS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Stack<long> i = new Stack<long>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push((long)c);
            }
            Stack<long> o;
            Stack<long> r;

            r = p.opSLongS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            float[] i = new float[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = (float)c;
            }
            float[] o;
            float[] r;

            r = p.opAFloatS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            List<float> i = new List<float>(_length);
            for(int c = 0; c < _length; ++c)
            {
                i.Add((float)c);
            }
            List<float> o;
            List<float> r;

            r = p.opLFloatS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            LinkedList<float> i = new LinkedList<float>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast((float)c);
            }
            LinkedList<float> o;
            LinkedList<float> r;

            r = p.opKFloatS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Queue<float> i = new Queue<float>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue((float)c);
            }
            Queue<float> o;
            Queue<float> r;

            r = p.opQFloatS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Stack<float> i = new Stack<float>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push((float)c);
            }
            Stack<float> o;
            Stack<float> r;

            r = p.opSFloatS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            double[] i = new double[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = (double)c;
            }
            double[] o;
            double[] r;

            r = p.opADoubleS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            List<double> i = new List<double>(_length);
            for(int c = 0; c < _length; ++c)
            {
                i.Add((double)c);
            }
            List<double> o;
            List<double> r;

            r = p.opLDoubleS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            LinkedList<double> i = new LinkedList<double>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast((double)c);
            }
            LinkedList<double> o;
            LinkedList<double> r;

            r = p.opKDoubleS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Queue<double> i = new Queue<double>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue((double)c);
            }
            Queue<double> o;
            Queue<double> r;

            r = p.opQDoubleS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Stack<double> i = new Stack<double>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push((double)c);
            }
            Stack<double> o;
            Stack<double> r;

            r = p.opSDoubleS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            string[] i = new string[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = c.ToString();
            }
            string[] o;
            string[] r;

            r = p.opAStringS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            List<string> i = new List<string>(_length);
            for(int c = 0; c < _length; ++c)
            {
                i.Add(c.ToString());
            }
            List<string> o;
            List<string> r;

            r = p.opLStringS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            LinkedList<string> i = new LinkedList<string>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast(c.ToString());
            }
            LinkedList<string> o;
            LinkedList<string> r;

            r = p.opKStringS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Queue<string> i = new Queue<string>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue(c.ToString());
            }
            Queue<string> o;
            Queue<string> r;

            r = p.opQStringS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Stack<string> i = new Stack<string>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push(c.ToString());
            }
            Stack<string> o;
            Stack<string> r;

            r = p.opSStringS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Ice.Value[] i = new CV[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = new CV(c);
            }
            Ice.Value[] o;
            Ice.Value[] r;

            r = p.opAObjectS(i, out o);

            System.Collections.IEnumerator eo = o.GetEnumerator();
            System.Collections.IEnumerator er = r.GetEnumerator();
            foreach(CV obj in i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.i == ((CV)eo.Current).i);
                test(obj.i == ((CV)er.Current).i);
            }
        }

        {
            var i = new List<Ice.Value>(_length);
            for(int c = 0; c < _length; ++c)
            {
                i.Add(new CV(c));
            }
            List<Ice.Value> o;
            List<Ice.Value> r;

            r = p.opLObjectS(i, out o);

            var eo = o.GetEnumerator();
            var er = r.GetEnumerator();
            foreach(CV obj in i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.i == ((CV)eo.Current).i);
                test(obj.i == ((CV)er.Current).i);
            }
        }

        {
            Ice.ObjectPrx[] i = new Ice.ObjectPrx[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = communicator.stringToProxy(c.ToString());
            }
            Ice.ObjectPrx[] o;
            Ice.ObjectPrx[] r;

            r = p.opAObjectPrxS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            List<Ice.ObjectPrx> i = new List<Ice.ObjectPrx>(_length);
            for(int c = 0; c < _length; ++c)
            {
                i.Add(communicator.stringToProxy(c.ToString()));
            }
            List<Ice.ObjectPrx> o;
            List<Ice.ObjectPrx> r;

            r = p.opLObjectPrxS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            LinkedList<Ice.ObjectPrx> i = new LinkedList<Ice.ObjectPrx>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast(communicator.stringToProxy(c.ToString()));
            }
            LinkedList<Ice.ObjectPrx> o;
            LinkedList<Ice.ObjectPrx> r;

            r = p.opKObjectPrxS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Queue<Ice.ObjectPrx> i = new Queue<Ice.ObjectPrx>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue(communicator.stringToProxy(c.ToString()));
            }
            Queue<Ice.ObjectPrx> o;
            Queue<Ice.ObjectPrx> r;

            r = p.opQObjectPrxS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Stack<Ice.ObjectPrx> i = new Stack<Ice.ObjectPrx>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push(communicator.stringToProxy(c.ToString()));
            }
            Stack<Ice.ObjectPrx> o;
            Stack<Ice.ObjectPrx> r;

            r = p.opSObjectPrxS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            S[] i = new S[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c].i = c;
            }
            S[] o;
            S[] r;

            r = p.opAStructS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            List<S> i = new List<S>(_length);
            for(int c = 0; c < _length; ++c)
            {
                i.Add(new S(c));
            }
            List<S> o;
            List<S> r;

            r = p.opLStructS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            LinkedList<S> i = new LinkedList<S>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast(new S(c));
            }
            LinkedList<S> o;
            LinkedList<S> r;

            r = p.opKStructS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Queue<S> i = new Queue<S>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue(new S(c));
            }
            Queue<S> o;
            Queue<S> r;

            r = p.opQStructS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Stack<S> i = new Stack<S>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push(new S(c));
            }
            Stack<S> o;
            Stack<S> r;


            r = p.opSStructS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            SD[] i = new SD[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = new SD(c);
            }
            SD[] o;
            SD[] r;

            r = p.opAStructSD(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            List<SD> i = new List<SD>(_length);
            for(int c = 0; c < _length; ++c)
            {
                i.Add(new SD(c));
            }
            List<SD> o;
            List<SD> r;

            r = p.opLStructSD(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            LinkedList<SD> i = new LinkedList<SD>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast(new SD(c));
            }
            LinkedList<SD> o;
            LinkedList<SD> r;

            r = p.opKStructSD(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Queue<SD> i = new Queue<SD>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue(new SD(c));
            }
            Queue<SD> o;
            Queue<SD> r;

            r = p.opQStructSD(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Stack<SD> i = new Stack<SD>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push(new SD(c));
            }
            Stack<SD> o;
            Stack<SD> r;


            r = p.opSStructSD(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            CV[] i = new CV[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = new CV(c);
            }
            CV[] o;
            CV[] r;

            r = p.opACVS(i, out o);

            System.Collections.IEnumerator eo = o.GetEnumerator();
            System.Collections.IEnumerator er = r.GetEnumerator();
            foreach(CV obj in i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.i == ((CV)eo.Current).i);
                test(obj.i == ((CV)er.Current).i);
            }
        }

        {
            List<CV> i = new List<CV>(_length);
            for(int c = 0; c < _length; ++c)
            {
                i.Add(new CV(c));
            }
            List<CV> o;
            List<CV> r;

            r = p.opLCVS(i, out o);

            IEnumerator<CV> eo = o.GetEnumerator();
            IEnumerator<CV> er = r.GetEnumerator();
            foreach(CV obj in i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.i == eo.Current.i);
                test(obj.i == er.Current.i);
            }
        }

        {
            CR[] i = new CR[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = new CR(new CV(c));
            }
            CR[] o;
            CR[] r;

            r = p.opACRS(i, out o);

            System.Collections.IEnumerator eo = o.GetEnumerator();
            System.Collections.IEnumerator er = r.GetEnumerator();
            foreach(CR obj in i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.v.i == ((CR)eo.Current).v.i);
                test(obj.v.i == ((CR)er.Current).v.i);
            }
        }

        {
            List<CR> i = new List<CR>(_length);
            for(int c = 0; c < _length; ++c)
            {
                i.Add(new CR(new CV(c)));
            }
            List<CR> o;
            List<CR> r;

            r = p.opLCRS(i, out o);

            IEnumerator<CR> eo = o.GetEnumerator();
            IEnumerator<CR> er = r.GetEnumerator();
            foreach(CR obj in i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.v.i == eo.Current.v.i);
                test(obj.v.i == er.Current.v.i);
            }
        }

        {
            En[] i = new En[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = (En)(c % 3);
            }
            En[] o;
            En[] r;

            r = p.opAEnS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            List<En> i = new List<En>(_length);
            for(int c = 0; c < _length; ++c)
            {
                i.Add((En)(c % 3));
            }
            List<En> o;
            List<En> r;

            r = p.opLEnS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            LinkedList<En> i = new LinkedList<En>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast((En)(c % 3));
            }
            LinkedList<En> o;
            LinkedList<En> r;

            r = p.opKEnS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Queue<En> i = new Queue<En>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue((En)(c % 3));
            }
            Queue<En> o;
            Queue<En> r;

            r = p.opQEnS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Stack<En> i = new Stack<En>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push((En)(c % 3));
            }
            Stack<En> o;
            Stack<En> r;

            r = p.opSEnS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Ice.ObjectPrx[] i = new Ice.ObjectPrx[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = communicator.stringToProxy(c.ToString());
            }
            Ice.ObjectPrx[] o;
            Ice.ObjectPrx[] r;

            r = p.opACVPrxS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            List<Ice.ObjectPrx> i = new List<Ice.ObjectPrx>(_length);
            for(int c = 0; c < _length; ++c)
            {
                i.Add(communicator.stringToProxy(c.ToString()));
            }
            List<Ice.ObjectPrx> o;
            List<Ice.ObjectPrx> r;

            r = p.opLCVPrxS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            LinkedList<Ice.ObjectPrx> i = new LinkedList<Ice.ObjectPrx>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast(communicator.stringToProxy(c.ToString()));
            }
            LinkedList<Ice.ObjectPrx> o;
            LinkedList<Ice.ObjectPrx> r;

            r = p.opKCVPrxS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Queue<Ice.ObjectPrx> i = new Queue<Ice.ObjectPrx>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue(communicator.stringToProxy(c.ToString()));
            }
            Queue<Ice.ObjectPrx> o;
            Queue<Ice.ObjectPrx> r;

            r = p.opQCVPrxS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Stack<Ice.ObjectPrx> i = new Stack<Ice.ObjectPrx>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push(communicator.stringToProxy(c.ToString()));
            }
            Stack<Ice.ObjectPrx> o;
            Stack<Ice.ObjectPrx> r;

            r = p.opSCVPrxS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Custom<int> i = new Custom<int>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(c);
            }
            Custom<int> o;
            Custom<int> r;

            r = p.opCustomIntS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Custom<CV> i = new Custom<CV>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(new CV(c));
            }
            i.Add(null);
            Custom<CV> o;
            Custom<CV> r;

            r = p.opCustomCVS(i, out o);

            IEnumerator<CV> eo = (IEnumerator<CV>)o.GetEnumerator();
            IEnumerator<CV> er = (IEnumerator<CV>)r.GetEnumerator();
            foreach(CV obj in i)
            {
                eo.MoveNext();
                er.MoveNext();
                if(obj == null)
                {
                    test(eo.Current == null);
                    test(er.Current == null);
                }
                else
                {
                    test(obj.i == eo.Current.i);
                    test(obj.i == er.Current.i);
                }
            }
        }

        {
            Custom<Custom<int>> i = new Custom<Custom<int>>();
            for(int c = 0; c < _length; ++c)
            {
                Custom<int> inner = new Custom<int>();
                for(int j = 0; j < c; ++j)
                {
                    inner.Add(j);
                }
                i.Add(inner);
            }
            Custom<Custom<int>> o;
            Custom<Custom<int>> r;

            r = p.opCustomIntSS(i, out o);

            test(Ice.CollectionComparer.Equals(i, o));
            test(Ice.CollectionComparer.Equals(i, r));
        }

        {
            Custom<Custom<CV>> i = new Custom<Custom<CV>>();
            for(int c = 0; c < _length; ++c)
            {
                Custom<CV> inner = new Custom<CV>();
                for(int j = 0; j < c; ++j)
                {
                    inner.Add(new CV(j));
                }
                i.Add(inner);
            }
            Custom<Custom<CV>> o;
            Custom<Custom<CV>> r;

            r = p.opCustomCVSS(i, out o);

            IEnumerator<Custom<CV>> eo = (IEnumerator<Custom<CV>>)o.GetEnumerator();
            IEnumerator<Custom<CV>> er = (IEnumerator<Custom<CV>>)r.GetEnumerator();
            foreach(Custom<CV> s in i)
            {
                eo.MoveNext();
                er.MoveNext();
                IEnumerator<CV> io = (IEnumerator<CV>)eo.Current.GetEnumerator();
                IEnumerator<CV> ir = (IEnumerator<CV>)er.Current.GetEnumerator();
                foreach(CV obj in s)
                {
                    io.MoveNext();
                    ir.MoveNext();
                    if(obj == null)
                    {
                        test(io.Current == null);
                        test(ir.Current == null);
                    }
                    else
                    {
                        test(obj.i == io.Current.i);
                        test(obj.i == ir.Current.i);
                    }
                }
            }
        }

        {
            Serialize.Small i = null;
            Serialize.Small o;
            Serialize.Small r;

            r = p.opSerialSmallCSharp(i, out o);

            test(o == null);
            test(r == null);
        }

        {
            Serialize.Small i = new Serialize.Small();
            i.i = 99;
            Serialize.Small o;
            Serialize.Small r;

            try
            {
                r = p.opSerialSmallCSharp(i, out o);

                test(o.i == 99);
                test(r.i == 99);
            }
            catch(Ice.OperationNotExistException)
            {
                // OK, talking to non-C# server.
            }
        }

        {
            Serialize.Large i = new Serialize.Large();
            i.d1 = 1.0;
            i.d2 = 2.0;
            i.d3 = 3.0;
            i.d4 = 4.0;
            i.d5 = 5.0;
            i.d6 = 6.0;
            i.d7 = 7.0;
            i.d8 = 8.0;
            i.d9 = 9.0;
            i.d10 = 10.0;
            Serialize.Large o;
            Serialize.Large r;

            try
            {
                r = p.opSerialLargeCSharp(i, out o);

                test(o.d1 == 1.0);
                test(o.d2 == 2.0);
                test(o.d3 == 3.0);
                test(o.d4 == 4.0);
                test(o.d5 == 5.0);
                test(o.d6 == 6.0);
                test(o.d7 == 7.0);
                test(o.d8 == 8.0);
                test(o.d9 == 9.0);
                test(o.d10 == 10.0);
                test(r.d1 == 1.0);
                test(r.d2 == 2.0);
                test(r.d3 == 3.0);
                test(r.d4 == 4.0);
                test(r.d5 == 5.0);
                test(r.d6 == 6.0);
                test(r.d7 == 7.0);
                test(r.d8 == 8.0);
                test(r.d9 == 9.0);
                test(r.d10 == 10.0);
            }
            catch(Ice.OperationNotExistException)
            {
                // OK, talking to non-C# server.
            }
        }

        {
            Serialize.Struct i = new Serialize.Struct();
            i.o = null;
            i.o2 = i;
            i.s = null;
            i.s2 = "Hello";
            Serialize.Struct o;
            Serialize.Struct r;

            try
            {
                r = p.opSerialStructCSharp(i, out o);

                test(o.o == null);
                test(o.o2 != null);
                test(((Serialize.Struct)(o.o2)).o == null);
                test(((Serialize.Struct)(o.o2)).o2 == o.o2);
                test(o.s == null);
                test(o.s2.Equals("Hello"));
                test(r.o == null);
                test(r.o2 != null);
                test(((Serialize.Struct)(r.o2)).o == null);
                test(((Serialize.Struct)(r.o2)).o2 == r.o2);
                test(r.s == null);
                test(r.s2.Equals("Hello"));
            }
            catch(Ice.OperationNotExistException)
            {
                // OK, talking to non-C# server.
            }
        }
    }
}
