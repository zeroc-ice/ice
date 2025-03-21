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
            byte[] o;
            byte[] r;

            r = p.opAByteS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            List<byte> i = new List<byte>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add((byte)c);
            }
            List<byte> o;
            List<byte> r;

            r = p.opLByteS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            LinkedList<byte> i = new LinkedList<byte>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast((byte)c);
            }
            LinkedList<byte> o;
            LinkedList<byte> r;

            r = p.opKByteS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            Queue<byte> i = new Queue<byte>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue((byte)c);
            }
            Queue<byte> o;
            Queue<byte> r;

            r = p.opQByteS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            Stack<byte> i = new Stack<byte>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push((byte)c);
            }
            Stack<byte> o;
            Stack<byte> r;

            r = p.opSByteS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            bool[] i = new bool[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = c % 1 == 1;
            }
            bool[] o;
            bool[] r;

            r = p.opABoolS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            List<bool> i = new List<bool>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add(c % 1 == 1);
            }
            List<bool> o;
            List<bool> r;

            r = p.opLBoolS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            LinkedList<bool> i = new LinkedList<bool>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast(c % 1 == 1);
            }
            LinkedList<bool> o;
            LinkedList<bool> r;

            r = p.opKBoolS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            Queue<bool> i = new Queue<bool>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue(c % 1 == 1);
            }
            Queue<bool> o;
            Queue<bool> r;

            r = p.opQBoolS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            Stack<bool> i = new Stack<bool>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push(c % 1 == 1);
            }
            Stack<bool> o;
            Stack<bool> r;

            r = p.opSBoolS(i, out o);

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
            List<short> i = new List<short>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add((short)c);
            }
            List<short> o;
            List<short> r;

            r = p.opLShortS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            LinkedList<short> i = new LinkedList<short>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast((short)c);
            }
            LinkedList<short> o;
            LinkedList<short> r;

            r = p.opKShortS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            Queue<short> i = new Queue<short>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue((short)c);
            }
            Queue<short> o;
            Queue<short> r;

            r = p.opQShortS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            Stack<short> i = new Stack<short>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push((short)c);
            }
            Stack<short> o;
            Stack<short> r;

            r = p.opSShortS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            int[] i = new int[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = (int)c;
            }
            int[] o;
            int[] r;

            r = p.opAIntS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            List<int> i = new List<int>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add((int)c);
            }
            List<int> o;
            List<int> r;

            r = p.opLIntS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            LinkedList<int> i = new LinkedList<int>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast((int)c);
            }
            LinkedList<int> o;
            LinkedList<int> r;

            r = p.opKIntS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            Queue<int> i = new Queue<int>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue((int)c);
            }
            Queue<int> o;
            Queue<int> r;

            r = p.opQIntS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            Stack<int> i = new Stack<int>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push((int)c);
            }
            Stack<int> o;
            Stack<int> r;

            r = p.opSIntS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            long[] i = new long[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = (long)c;
            }
            long[] o;
            long[] r;

            r = p.opALongS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            List<long> i = new List<long>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add((long)c);
            }
            List<long> o;
            List<long> r;

            r = p.opLLongS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            LinkedList<long> i = new LinkedList<long>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast((long)c);
            }
            LinkedList<long> o;
            LinkedList<long> r;

            r = p.opKLongS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            Queue<long> i = new Queue<long>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue((long)c);
            }
            Queue<long> o;
            Queue<long> r;

            r = p.opQLongS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            Stack<long> i = new Stack<long>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push((long)c);
            }
            Stack<long> o;
            Stack<long> r;

            r = p.opSLongS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            float[] i = new float[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = (float)c;
            }
            float[] o;
            float[] r;

            r = p.opAFloatS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            List<float> i = new List<float>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add((float)c);
            }
            List<float> o;
            List<float> r;

            r = p.opLFloatS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            LinkedList<float> i = new LinkedList<float>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast((float)c);
            }
            LinkedList<float> o;
            LinkedList<float> r;

            r = p.opKFloatS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            Queue<float> i = new Queue<float>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue((float)c);
            }
            Queue<float> o;
            Queue<float> r;

            r = p.opQFloatS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            Stack<float> i = new Stack<float>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push((float)c);
            }
            Stack<float> o;
            Stack<float> r;

            r = p.opSFloatS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            double[] i = new double[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = (double)c;
            }
            double[] o;
            double[] r;

            r = p.opADoubleS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            List<double> i = new List<double>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add((double)c);
            }
            List<double> o;
            List<double> r;

            r = p.opLDoubleS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            LinkedList<double> i = new LinkedList<double>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast((double)c);
            }
            LinkedList<double> o;
            LinkedList<double> r;

            r = p.opKDoubleS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            Queue<double> i = new Queue<double>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue((double)c);
            }
            Queue<double> o;
            Queue<double> r;

            r = p.opQDoubleS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            Stack<double> i = new Stack<double>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push((double)c);
            }
            Stack<double> o;
            Stack<double> r;

            r = p.opSDoubleS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            string[] i = new string[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = c.ToString();
            }
            string[] o;
            string[] r;

            r = p.opAStringS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            List<string> i = new List<string>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add(c.ToString());
            }
            List<string> o;
            List<string> r;

            r = p.opLStringS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            LinkedList<string> i = new LinkedList<string>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast(c.ToString());
            }
            LinkedList<string> o;
            LinkedList<string> r;

            r = p.opKStringS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            Queue<string> i = new Queue<string>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue(c.ToString());
            }
            Queue<string> o;
            Queue<string> r;

            r = p.opQStringS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            Stack<string> i = new Stack<string>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push(c.ToString());
            }
            Stack<string> o;
            Stack<string> r;

            r = p.opSStringS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            Ice.Value[] i = new Test.CV[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = new Test.CV(c);
            }
            Ice.Value[] o;
            Ice.Value[] r;

            r = p.opAObjectS(i, out o);

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
            List<Ice.Value> o;
            List<Ice.Value> r;

            r = p.opLObjectS(i, out o);

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
            Ice.ObjectPrx[] i = new Ice.ObjectPrx[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = communicator.stringToProxy(c.ToString());
            }
            Ice.ObjectPrx[] o;
            Ice.ObjectPrx[] r;

            r = p.opAObjectPrxS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            List<Ice.ObjectPrx> i = new List<Ice.ObjectPrx>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add(communicator.stringToProxy(c.ToString()));
            }
            List<Ice.ObjectPrx> o;
            List<Ice.ObjectPrx> r;

            r = p.opLObjectPrxS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            LinkedList<Ice.ObjectPrx> i = new LinkedList<Ice.ObjectPrx>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast(communicator.stringToProxy(c.ToString()));
            }
            LinkedList<Ice.ObjectPrx> o;
            LinkedList<Ice.ObjectPrx> r;

            r = p.opKObjectPrxS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            Queue<Ice.ObjectPrx> i = new Queue<Ice.ObjectPrx>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue(communicator.stringToProxy(c.ToString()));
            }
            Queue<Ice.ObjectPrx> o;
            Queue<Ice.ObjectPrx> r;

            r = p.opQObjectPrxS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            Stack<Ice.ObjectPrx> i = new Stack<Ice.ObjectPrx>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push(communicator.stringToProxy(c.ToString()));
            }
            Stack<Ice.ObjectPrx> o;
            Stack<Ice.ObjectPrx> r;

            r = p.opSObjectPrxS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Test.S[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c].i = c;
            }
            Test.S[] o;
            Test.S[] r;

            r = p.opAStructS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new List<Test.S>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add(new Test.S(c));
            }
            List<Test.S> o;
            List<Test.S> r;

            r = p.opLStructS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new LinkedList<Test.S>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast(new Test.S(c));
            }
            LinkedList<Test.S> o;
            LinkedList<Test.S> r;

            r = p.opKStructS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Queue<Test.S>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue(new Test.S(c));
            }
            Queue<Test.S> o;
            Queue<Test.S> r;

            r = p.opQStructS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Stack<Test.S>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push(new Test.S(c));
            }
            Stack<Test.S> o;
            Stack<Test.S> r;

            r = p.opSStructS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Test.SD[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = new Test.SD(c);
            }
            Test.SD[] o;
            Test.SD[] r;

            r = p.opAStructSD(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new List<Test.SD>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add(new Test.SD(c));
            }
            List<Test.SD> o;
            List<Test.SD> r;

            r = p.opLStructSD(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new LinkedList<Test.SD>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast(new Test.SD(c));
            }
            LinkedList<Test.SD> o;
            LinkedList<Test.SD> r;

            r = p.opKStructSD(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Queue<Test.SD>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue(new Test.SD(c));
            }
            Queue<Test.SD> o;
            Queue<Test.SD> r;

            r = p.opQStructSD(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Stack<Test.SD>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push(new Test.SD(c));
            }
            Stack<Test.SD> o;
            Stack<Test.SD> r;

            r = p.opSStructSD(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Test.CV[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = new Test.CV(c);
            }
            Test.CV[] o;
            Test.CV[] r;

            r = p.opACVS(i, out o);

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
            List<Test.CV> o;
            List<Test.CV> r;

            r = p.opLCVS(i, out o);

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
            Test.CR[] o;
            Test.CR[] r;

            r = p.opACRS(i, out o);

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
            List<Test.CR> o;
            List<Test.CR> r;

            r = p.opLCRS(i, out o);

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
            Test.En[] o;
            Test.En[] r;

            r = p.opAEnS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new List<Test.En>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add((Test.En)(c % 3));
            }
            List<Test.En> o;
            List<Test.En> r;

            r = p.opLEnS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new LinkedList<Test.En>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast((Test.En)(c % 3));
            }
            LinkedList<Test.En> o;
            LinkedList<Test.En> r;

            r = p.opKEnS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Queue<Test.En>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue((Test.En)(c % 3));
            }
            Queue<Test.En> o;
            Queue<Test.En> r;

            r = p.opQEnS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            var i = new Stack<Test.En>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push((Test.En)(c % 3));
            }
            Stack<Test.En> o;
            Stack<Test.En> r;

            r = p.opSEnS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            Test.IPrx[] i = new Test.IPrx[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString()));
            }
            Test.IPrx[] o;
            Test.IPrx[] r;

            r = p.opAIPrxS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            List<Test.IPrx> i = new List<Test.IPrx>(_length);
            for (int c = 0; c < _length; ++c)
            {
                i.Add(Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
            }
            List<Test.IPrx> o;
            List<Test.IPrx> r;

            r = p.opLIPrxS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            LinkedList<Test.IPrx> i = new LinkedList<Test.IPrx>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast(Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
            }
            LinkedList<Test.IPrx> o;
            LinkedList<Test.IPrx> r;

            r = p.opKIPrxS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            Queue<Test.IPrx> i = new Queue<Test.IPrx>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue(Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
            }
            Queue<Test.IPrx> o;
            Queue<Test.IPrx> r;

            r = p.opQIPrxS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            Stack<Test.IPrx> i = new Stack<Test.IPrx>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push(Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
            }
            Stack<Test.IPrx> o;
            Stack<Test.IPrx> r;

            r = p.opSIPrxS(i, out o);

            test(Enumerable.SequenceEqual(i, o));
            test(Enumerable.SequenceEqual(i, r));
        }

        {
            Custom<int> i = new Custom<int>();
            for (int c = 0; c < _length; ++c)
            {
                i.Add(c);
            }
            Custom<int> o;
            Custom<int> r;

            r = p.opCustomIntS(i, out o);

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
            Custom<Test.CV> o;
            Custom<Test.CV> r;

            r = p.opCustomCVS(i, out o);

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
            Custom<Custom<int>> i = new Custom<Custom<int>>();
            for (int c = 0; c < _length; ++c)
            {
                Custom<int> inner = new Custom<int>();
                for (int j = 0; j < c; ++j)
                {
                    inner.Add(j);
                }
                i.Add(inner);
            }
            Custom<Custom<int>> o;
            Custom<Custom<int>> r;

            r = p.opCustomIntSS(i, out o);

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
            Custom<Custom<Test.CV>> o;
            Custom<Custom<Test.CV>> r;

            r = p.opCustomCVSS(i, out o);

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
