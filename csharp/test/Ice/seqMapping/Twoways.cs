// Copyright (c) ZeroC, Inc.

namespace Ice.seqMapping;

internal class Twoways
{
    private static void test(bool b) => global::Test.TestHelper.test(b);

    private static readonly int _length = 100;

    internal static void twoways(Ice.Communicator communicator, Test.MyClassPrx p)
    {
        {
            byte[] i = new byte[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = (byte)c;
            }
            byte[] r;

            r = p.opAByteS(i, out byte[] o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new List<byte>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add((byte)c);
            }
            List<byte> r;

            r = p.opLByteS(i, out List<byte> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new LinkedList<byte>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast((byte)c);
            }
            LinkedList<byte> r;

            r = p.opKByteS(i, out LinkedList<byte> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Queue<byte>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue((byte)c);
            }
            Queue<byte> r;

            r = p.opQByteS(i, out Queue<byte> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Stack<byte>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push((byte)c);
            }
            Stack<byte> r;

            r = p.opSByteS(i, out Stack<byte> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            bool[] i = new bool[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = c % 1 == 1;
            }
            bool[] r;

            r = p.opABoolS(i, out bool[] o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new List<bool>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add(c % 1 == 1);
            }
            List<bool> r;

            r = p.opLBoolS(i, out List<bool> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new LinkedList<bool>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast(c % 1 == 1);
            }
            LinkedList<bool> r;

            r = p.opKBoolS(i, out LinkedList<bool> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Queue<bool>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue(c % 1 == 1);
            }
            Queue<bool> r;

            r = p.opQBoolS(i, out Queue<bool> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Stack<bool>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push(c % 1 == 1);
            }
            Stack<bool> r;

            r = p.opSBoolS(i, out Stack<bool> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            short[] i = new short[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = (short)c;
            }
            short[] o;
            short[] r;

            {
                r = p.opAShortS(i, out o);
            }
            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new List<short>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add((short)c);
            }
            List<short> r;

            r = p.opLShortS(i, out List<short> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new LinkedList<short>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast((short)c);
            }
            LinkedList<short> r;

            r = p.opKShortS(i, out LinkedList<short> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Queue<short>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue((short)c);
            }
            Queue<short> r;

            r = p.opQShortS(i, out Queue<short> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Stack<short>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push((short)c);
            }
            Stack<short> r;

            r = p.opSShortS(i, out Stack<short> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            int[] i = new int[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = (int)c;
            }
            int[] r;

            r = p.opAIntS(i, out int[] o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new List<int>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add((int)c);
            }
            List<int> r;

            r = p.opLIntS(i, out List<int> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new LinkedList<int>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast((int)c);
            }
            LinkedList<int> r;

            r = p.opKIntS(i, out LinkedList<int> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Queue<int>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue((int)c);
            }
            Queue<int> r;

            r = p.opQIntS(i, out Queue<int> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Stack<int>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push((int)c);
            }
            Stack<int> r;

            r = p.opSIntS(i, out Stack<int> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            long[] i = new long[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = (long)c;
            }
            long[] r;

            r = p.opALongS(i, out long[] o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new List<long>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add((long)c);
            }
            List<long> r;

            r = p.opLLongS(i, out List<long> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new LinkedList<long>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast((long)c);
            }
            LinkedList<long> r;

            r = p.opKLongS(i, out LinkedList<long> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Queue<long>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue((long)c);
            }
            Queue<long> r;

            r = p.opQLongS(i, out Queue<long> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Stack<long>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push((long)c);
            }
            Stack<long> r;

            r = p.opSLongS(i, out Stack<long> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            float[] i = new float[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = (float)c;
            }
            float[] r;

            r = p.opAFloatS(i, out float[] o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new List<float>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add((float)c);
            }
            List<float> r;

            r = p.opLFloatS(i, out List<float> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new LinkedList<float>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast((float)c);
            }
            LinkedList<float> r;

            r = p.opKFloatS(i, out LinkedList<float> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Queue<float>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue((float)c);
            }
            Queue<float> r;

            r = p.opQFloatS(i, out Queue<float> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Stack<float>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push((float)c);
            }
            Stack<float> r;

            r = p.opSFloatS(i, out Stack<float> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            double[] i = new double[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = (double)c;
            }
            double[] r;

            r = p.opADoubleS(i, out double[] o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new List<double>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add((double)c);
            }
            List<double> r;

            r = p.opLDoubleS(i, out List<double> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new LinkedList<double>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast((double)c);
            }
            LinkedList<double> r;

            r = p.opKDoubleS(i, out LinkedList<double> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Queue<double>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue((double)c);
            }
            Queue<double> r;

            r = p.opQDoubleS(i, out Queue<double> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Stack<double>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push((double)c);
            }
            Stack<double> r;

            r = p.opSDoubleS(i, out Stack<double> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            string[] i = new string[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = c.ToString();
            }
            string[] r;

            r = p.opAStringS(i, out string[] o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new List<string>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add(c.ToString());
            }
            List<string> r;

            r = p.opLStringS(i, out List<string> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new LinkedList<string>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast(c.ToString());
            }
            LinkedList<string> r;

            r = p.opKStringS(i, out LinkedList<string> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Queue<string>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue(c.ToString());
            }
            Queue<string> r;

            r = p.opQStringS(i, out Queue<string> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Stack<string>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push(c.ToString());
            }
            Stack<string> r;

            r = p.opSStringS(i, out Stack<string> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            Ice.Value[] i = new Test.CV[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = new Test.CV(c);
            }
            Ice.Value[] r;

            r = p.opAObjectS(i, out Value[] o);

            System.Collections.IEnumerator eo = o.GetEnumerator();
            System.Collections.IEnumerator er = r.GetEnumerator();
            foreach (Test.CV obj in i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.i == ((Test.CV)eo.Current).i);
                test(obj.i == ((Test.CV)er.Current).i);
            }
        }

        {
            var i = new List<Ice.Value>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add(new Test.CV(c));
            }
            List<Ice.Value> r;

            r = p.opLObjectS(i, out List<Value> o);

            var eo = o.GetEnumerator();
            var er = r.GetEnumerator();
            foreach (Test.CV obj in i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.i == ((Test.CV)eo.Current).i);
                test(obj.i == ((Test.CV)er.Current).i);
            }
        }

        {
            var i = new Ice.ObjectPrx[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = communicator.stringToProxy(c.ToString());
            }
            Ice.ObjectPrx[] r;

            r = p.opAObjectPrxS(i, out ObjectPrx[] o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new List<Ice.ObjectPrx>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add(communicator.stringToProxy(c.ToString()));
            }
            List<Ice.ObjectPrx> r;

            r = p.opLObjectPrxS(i, out List<ObjectPrx> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new LinkedList<Ice.ObjectPrx>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast(communicator.stringToProxy(c.ToString()));
            }
            LinkedList<Ice.ObjectPrx> r;

            r = p.opKObjectPrxS(i, out LinkedList<ObjectPrx> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Queue<Ice.ObjectPrx>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue(communicator.stringToProxy(c.ToString()));
            }
            Queue<Ice.ObjectPrx> r;

            r = p.opQObjectPrxS(i, out Queue<ObjectPrx> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Stack<Ice.ObjectPrx>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push(communicator.stringToProxy(c.ToString()));
            }
            Stack<Ice.ObjectPrx> r;

            r = p.opSObjectPrxS(i, out Stack<ObjectPrx> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Test.S[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c].i = c;
            }
            Test.S[] r;

            r = p.opAStructS(i, out Test.S[] o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new List<Test.S>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add(new Test.S(c));
            }
            List<Test.S> r;

            r = p.opLStructS(i, out List<Test.S> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new LinkedList<Test.S>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast(new Test.S(c));
            }
            LinkedList<Test.S> r;

            r = p.opKStructS(i, out LinkedList<Test.S> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Queue<Test.S>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue(new Test.S(c));
            }
            Queue<Test.S> r;

            r = p.opQStructS(i, out Queue<Test.S> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Stack<Test.S>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push(new Test.S(c));
            }
            Stack<Test.S> r;

            r = p.opSStructS(i, out Stack<Test.S> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Test.SD[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = new Test.SD(c);
            }
            Test.SD[] r;

            r = p.opAStructSD(i, out Test.SD[] o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new List<Test.SD>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add(new Test.SD(c));
            }
            List<Test.SD> r;

            r = p.opLStructSD(i, out List<Test.SD> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new LinkedList<Test.SD>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast(new Test.SD(c));
            }
            LinkedList<Test.SD> r;

            r = p.opKStructSD(i, out LinkedList<Test.SD> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Queue<Test.SD>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue(new Test.SD(c));
            }
            Queue<Test.SD> r;

            r = p.opQStructSD(i, out Queue<Test.SD> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Stack<Test.SD>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push(new Test.SD(c));
            }
            Stack<Test.SD> r;

            r = p.opSStructSD(i, out Stack<Test.SD> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Test.CV[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = new Test.CV(c);
            }
            Test.CV[] r;

            r = p.opACVS(i, out Test.CV[] o);

            System.Collections.IEnumerator eo = o.GetEnumerator();
            System.Collections.IEnumerator er = r.GetEnumerator();
            foreach (Test.CV obj in i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.i == ((Test.CV)eo.Current).i);
                test(obj.i == ((Test.CV)er.Current).i);
            }
        }

        {
            var i = new List<Test.CV>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add(new Test.CV(c));
            }
            List<Test.CV> r;

            r = p.opLCVS(i, out List<Test.CV> o);

            IEnumerator<Test.CV> eo = o.GetEnumerator();
            IEnumerator<Test.CV> er = r.GetEnumerator();
            foreach (Test.CV obj in i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.i == eo.Current.i);
                test(obj.i == er.Current.i);
            }
        }

        {
            var i = new Test.CR[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = new Test.CR(new Test.CV(c));
            }
            Test.CR[] r;

            r = p.opACRS(i, out Test.CR[] o);

            System.Collections.IEnumerator eo = o.GetEnumerator();
            System.Collections.IEnumerator er = r.GetEnumerator();
            foreach (Test.CR obj in i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.v.i == ((Test.CR)eo.Current).v.i);
                test(obj.v.i == ((Test.CR)er.Current).v.i);
            }
        }

        {
            var i = new List<Test.CR>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add(new Test.CR(new Test.CV(c)));
            }
            List<Test.CR> r;

            r = p.opLCRS(i, out List<Test.CR> o);

            IEnumerator<Test.CR> eo = o.GetEnumerator();
            IEnumerator<Test.CR> er = r.GetEnumerator();
            foreach (Test.CR obj in i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.v.i == eo.Current.v.i);
                test(obj.v.i == er.Current.v.i);
            }
        }

        {
            var i = new Test.En[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = (Test.En)(c % 3);
            }
            Test.En[] r;

            r = p.opAEnS(i, out Test.En[] o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new List<Test.En>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add((Test.En)(c % 3));
            }
            List<Test.En> r;

            r = p.opLEnS(i, out List<Test.En> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new LinkedList<Test.En>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast((Test.En)(c % 3));
            }
            LinkedList<Test.En> r;

            r = p.opKEnS(i, out LinkedList<Test.En> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Queue<Test.En>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue((Test.En)(c % 3));
            }
            Queue<Test.En> r;

            r = p.opQEnS(i, out Queue<Test.En> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Stack<Test.En>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push((Test.En)(c % 3));
            }
            Stack<Test.En> r;

            r = p.opSEnS(i, out Stack<Test.En> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Test.IPrx[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString()));
            }
            Test.IPrx[] r;

            r = p.opAIPrxS(i, out Test.IPrx[] o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new List<Test.IPrx>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add(Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
            }
            List<Test.IPrx> r;

            r = p.opLIPrxS(i, out List<Test.IPrx> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new LinkedList<Test.IPrx>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast(Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
            }
            LinkedList<Test.IPrx> r;

            r = p.opKIPrxS(i, out LinkedList<Test.IPrx> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Queue<Test.IPrx>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue(Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
            }
            Queue<Test.IPrx> r;

            r = p.opQIPrxS(i, out Queue<Test.IPrx> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Stack<Test.IPrx>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push(Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
            }
            Stack<Test.IPrx> r;

            r = p.opSIPrxS(i, out Stack<Test.IPrx> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Custom<int>();
            for (int c = 0; c < _length; ++c)
            {
                i.Add(c);
            }
            Custom<int> r;

            r = p.opCustomIntS(i, out Custom<int> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Custom<Test.CV>();
            for (int c = 0; c < _length; ++c)
            {
                i.Add(new Test.CV(c));
            }
            i.Add(null);
            Custom<Test.CV> r;

            r = p.opCustomCVS(i, out Custom<Test.CV> o);

            var eo = o.GetEnumerator();
            var er = r.GetEnumerator();
            foreach (Test.CV obj in i)
            {
                eo.MoveNext();
                er.MoveNext();
                if (obj == null)
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
            var i = new Custom<Custom<int>>();
            for (int c = 0; c < _length; ++c)
            {
                var inner = new Custom<int>();
                for (int j = 0; j < c; ++j)
                {
                    inner.Add(j);
                }
                i.Add(inner);
            }
            Custom<Custom<int>> r;

            r = p.opCustomIntSS(i, out Custom<Custom<int>> o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Custom<Custom<Test.CV>>();
            for (int c = 0; c < _length; ++c)
            {
                var inner = new Custom<Test.CV>();
                for (int j = 0; j < c; ++j)
                {
                    inner.Add(new Test.CV(j));
                }
                i.Add(inner);
            }
            Custom<Custom<Test.CV>> r;

            r = p.opCustomCVSS(i, out Custom<Custom<Test.CV>> o);

            var eo = o.GetEnumerator();
            var er = r.GetEnumerator();
            foreach (Custom<Test.CV> s in i)
            {
                eo.MoveNext();
                er.MoveNext();
                IEnumerator<Test.CV> io = eo.Current.GetEnumerator();
                IEnumerator<Test.CV> ir = er.Current.GetEnumerator();
                foreach (Test.CV obj in s)
                {
                    io.MoveNext();
                    ir.MoveNext();
                    if (obj == null)
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
    }
}
