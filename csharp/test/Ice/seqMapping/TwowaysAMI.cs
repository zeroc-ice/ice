// Copyright (c) ZeroC, Inc.

namespace Ice.seqMapping;

public static class TwowaysAMI
{
    private static void test(bool b) => global::Test.TestHelper.test(b);

    private const int _length = 100;

    internal static async Task twowaysAMI(Communicator communicator, Test.MyInterfacePrx p)
    {
        {
            byte[] i = new byte[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = (byte)c;
            }
            Test.MyInterface_OpAByteSResult result = await p.opAByteSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new List<byte>();
            for (int c = 0; c < _length; ++c)
            {
                i.Add((byte)c);
            }

            Test.MyInterface_OpLByteSResult result = await p.opLByteSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new LinkedList<byte>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast((byte)c);
            }

            Test.MyInterface_OpKByteSResult result = await p.opKByteSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Queue<byte>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue((byte)c);
            }

            Test.MyInterface_OpQByteSResult result = await p.opQByteSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Stack<byte>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push((byte)c);
            }

            Test.MyInterface_OpSByteSResult result = await p.opSByteSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            bool[] i = new bool[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = c % 1 == 1;
            }

            Test.MyInterface_OpABoolSResult result = await p.opABoolSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new List<bool>();
            for (int c = 0; c < _length; ++c)
            {
                i.Add(c % 1 == 1);
            }

            Test.MyInterface_OpLBoolSResult result = await p.opLBoolSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new LinkedList<bool>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast(c % 1 == 1);
            }

            Test.MyInterface_OpKBoolSResult result = await p.opKBoolSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Queue<bool>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue(c % 1 == 1);
            }

            Test.MyInterface_OpQBoolSResult result = await p.opQBoolSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Stack<bool>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push(c % 1 == 1);
            }

            Test.MyInterface_OpSBoolSResult result = await p.opSBoolSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            short[] i = new short[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = (short)c;
            }

            Test.MyInterface_OpAShortSResult result = await p.opAShortSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new List<short>();
            for (int c = 0; c < _length; ++c)
            {
                i.Add((short)c);
            }

            Test.MyInterface_OpLShortSResult result = await p.opLShortSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new LinkedList<short>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast((short)c);
            }

            Test.MyInterface_OpKShortSResult result = await p.opKShortSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Queue<short>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue((short)c);
            }

            Test.MyInterface_OpQShortSResult result = await p.opQShortSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Stack<short>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push((short)c);
            }

            Test.MyInterface_OpSShortSResult result = await p.opSShortSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            int[] i = new int[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = c;
            }

            Test.MyInterface_OpAIntSResult result = await p.opAIntSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new List<int>();
            for (int c = 0; c < _length; ++c)
            {
                i.Add((int)c);
            }

            Test.MyInterface_OpLIntSResult result = await p.opLIntSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new LinkedList<int>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast(c);
            }

            Test.MyInterface_OpKIntSResult result = await p.opKIntSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Queue<int>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue(c);
            }

            Test.MyInterface_OpQIntSResult result = await p.opQIntSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Stack<int>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push(c);
            }

            Test.MyInterface_OpSIntSResult result = await p.opSIntSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            long[] i = new long[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = c;
            }

            Test.MyInterface_OpALongSResult result = await p.opALongSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new List<long>();
            for (int c = 0; c < _length; ++c)
            {
                i.Add(c);
            }

            Test.MyInterface_OpLLongSResult result = await p.opLLongSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new LinkedList<long>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast((long)c);
            }

            Test.MyInterface_OpKLongSResult result = await p.opKLongSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Queue<long>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue(c);
            }

            Test.MyInterface_OpQLongSResult result = await p.opQLongSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Stack<long>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push(c);
            }

            Test.MyInterface_OpSLongSResult result = await p.opSLongSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            float[] i = new float[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = c;
            }

            Test.MyInterface_OpAFloatSResult result = await p.opAFloatSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new List<float>();
            for (int c = 0; c < _length; ++c)
            {
                i.Add(c);
            }

            Test.MyInterface_OpLFloatSResult result = await p.opLFloatSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new LinkedList<float>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast(c);
            }

            Test.MyInterface_OpKFloatSResult result = await p.opKFloatSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Queue<float>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue(c);
            }

            Test.MyInterface_OpQFloatSResult result = await p.opQFloatSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Stack<float>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push(c);
            }

            Test.MyInterface_OpSFloatSResult result = await p.opSFloatSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            double[] i = new double[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = c;
            }

            Test.MyInterface_OpADoubleSResult result = await p.opADoubleSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new List<double>();
            for (int c = 0; c < _length; ++c)
            {
                i.Add(c);
            }

            Test.MyInterface_OpLDoubleSResult result = await p.opLDoubleSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new LinkedList<double>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast(c);
            }

            Test.MyInterface_OpKDoubleSResult result = await p.opKDoubleSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Queue<double>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue(c);
            }

            Test.MyInterface_OpQDoubleSResult result = await p.opQDoubleSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Stack<double>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push(c);
            }

            Test.MyInterface_OpSDoubleSResult result = await p.opSDoubleSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            string[] i = new string[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = $"{c}";
            }

            Test.MyInterface_OpAStringSResult result = await p.opAStringSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new List<string>();
            for (int c = 0; c < _length; ++c)
            {
                i.Add($"{c}");
            }

            Test.MyInterface_OpLStringSResult result = await p.opLStringSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new LinkedList<string>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast($"{c}");
            }

            Test.MyInterface_OpKStringSResult result = await p.opKStringSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Queue<string>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue($"{c}");
            }

            Test.MyInterface_OpQStringSResult result = await p.opQStringSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Stack<string>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push($"{c}");
            }

            Test.MyInterface_OpSStringSResult result = await p.opSStringSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Value[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = new Test.CV(c);
            }

            Test.MyInterface_OpAObjectSResult result = await p.opAObjectSAsync(i);
            System.Collections.IEnumerator eo = result.o.GetEnumerator();
            System.Collections.IEnumerator er = result.returnValue.GetEnumerator();
            foreach (Test.CV obj in i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.i == ((Test.CV)eo.Current).i);
                test(obj.i == ((Test.CV)er.Current).i);
            }
        }

        {
            var i = new List<Value>();
            for (int c = 0; c < _length; ++c)
            {
                i.Add(new Test.CV(c));
            }

            Test.MyInterface_OpLObjectSResult result = await p.opLObjectSAsync(i);
            System.Collections.IEnumerator eo = result.o.GetEnumerator();
            System.Collections.IEnumerator er = result.returnValue.GetEnumerator();
            foreach (Test.CV obj in i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.i == ((Test.CV)eo.Current).i);
                test(obj.i == ((Test.CV)er.Current).i);
            }
        }

        {
            var i = new ObjectPrx[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = communicator.stringToProxy($"{c}");
            }

            Test.MyInterface_OpAObjectPrxSResult result = await p.opAObjectPrxSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new List<ObjectPrx>();
            for (int c = 0; c < _length; ++c)
            {
                i.Add(communicator.stringToProxy($"{c}"));
            }

            Test.MyInterface_OpLObjectPrxSResult result = await p.opLObjectPrxSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new LinkedList<Ice.ObjectPrx>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast(communicator.stringToProxy($"{c}"));
            }

            Test.MyInterface_OpKObjectPrxSResult result = await p.opKObjectPrxSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Queue<ObjectPrx>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue(communicator.stringToProxy($"{c}"));
            }

            Test.MyInterface_OpQObjectPrxSResult result = await p.opQObjectPrxSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Stack<ObjectPrx>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push(communicator.stringToProxy($"{c}"));
            }

            Test.MyInterface_OpSObjectPrxSResult result = await p.opSObjectPrxSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Test.S[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c].i = c;
            }

            Test.MyInterface_OpAStructSResult result = await p.opAStructSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new List<Test.S>();
            for (int c = 0; c < _length; ++c)
            {
                i.Add(new Test.S(c));
            }

            Test.MyInterface_OpLStructSResult result = await p.opLStructSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new LinkedList<Test.S>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast(new Test.S(c));
            }

            Test.MyInterface_OpKStructSResult result = await p.opKStructSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Queue<Test.S>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue(new Test.S(c));
            }

            Test.MyInterface_OpQStructSResult result = await p.opQStructSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Stack<Test.S>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push(new Test.S(c));
            }

            Test.MyInterface_OpSStructSResult result = await p.opSStructSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Test.CV[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = new Test.CV(c);
            }

            Test.MyInterface_OpACVSResult result = await p.opACVSAsync(i);
            System.Collections.IEnumerator eo = result.o.GetEnumerator();
            System.Collections.IEnumerator er = result.returnValue.GetEnumerator();
            foreach (Test.CV obj in i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.i == ((Test.CV)eo.Current).i);
                test(obj.i == ((Test.CV)er.Current).i);
            }
        }

        {
            var i = new List<Test.CV>();
            for (int c = 0; c < _length; ++c)
            {
                i.Add(new Test.CV(c));
            }

            Test.MyInterface_OpLCVSResult result = await p.opLCVSAsync(i);
            System.Collections.IEnumerator eo = result.o.GetEnumerator();
            System.Collections.IEnumerator er = result.returnValue.GetEnumerator();
            foreach (Test.CV obj in i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.i == ((Test.CV)eo.Current).i);
                test(obj.i == ((Test.CV)er.Current).i);
            }
        }

        {
            var i = new Test.IPrx[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = Test.IPrxHelper.uncheckedCast(communicator.stringToProxy($"{c}"));
            }

            Test.MyInterface_OpAIPrxSResult result = await p.opAIPrxSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new List<Test.IPrx>();
            for (int c = 0; c < _length; ++c)
            {
                i.Add(Test.IPrxHelper.uncheckedCast(communicator.stringToProxy($"{c}")));
            }

            Test.MyInterface_OpLIPrxSResult result = await p.opLIPrxSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new LinkedList<Test.IPrx>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast(Test.IPrxHelper.uncheckedCast(communicator.stringToProxy($"{c}")));
            }

            Test.MyInterface_OpKIPrxSResult result = await p.opKIPrxSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Queue<Test.IPrx>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue(Test.IPrxHelper.uncheckedCast(communicator.stringToProxy($"{c}")));
            }

            Test.MyInterface_OpQIPrxSResult result = await p.opQIPrxSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Stack<Test.IPrx>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push(Test.IPrxHelper.uncheckedCast(communicator.stringToProxy($"{c}")));
            }

            Test.MyInterface_OpSIPrxSResult result = await p.opSIPrxSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Test.CR[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = new Test.CR(new Test.CV(c));
            }

            Test.MyInterface_OpACRSResult result = await p.opACRSAsync(i);
            System.Collections.IEnumerator eo = result.o.GetEnumerator();
            System.Collections.IEnumerator er = result.returnValue.GetEnumerator();
            foreach (Test.CR obj in i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.v.i == ((Test.CR)eo.Current).v.i);
                test(obj.v.i == ((Test.CR)er.Current).v.i);
            }
        }

        {
            var i = new List<Test.CR>();
            for (int c = 0; c < _length; ++c)
            {
                i.Add(new Test.CR(new Test.CV(c)));
            }

            Test.MyInterface_OpLCRSResult result = await p.opLCRSAsync(i);
            System.Collections.IEnumerator eo = result.o.GetEnumerator();
            System.Collections.IEnumerator er = result.returnValue.GetEnumerator();
            foreach (Test.CR obj in i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.v.i == ((Test.CR)eo.Current).v.i);
                test(obj.v.i == ((Test.CR)er.Current).v.i);
            }
        }

        {
            var i = new Test.En[_length];
            for (int c = 0; c < _length; ++c)
            {
                i[c] = (Test.En)(c % 3);
            }

            Test.MyInterface_OpAEnSResult result = await p.opAEnSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new List<Test.En>();
            for (int c = 0; c < _length; ++c)
            {
                i.Add((Test.En)(c % 3));
            }

            Test.MyInterface_OpLEnSResult result = await p.opLEnSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new LinkedList<Test.En>();
            for (int c = 0; c < _length; ++c)
            {
                i.AddLast((Test.En)(c % 3));
            }

            Test.MyInterface_OpKEnSResult result = await p.opKEnSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Queue<Test.En>();
            for (int c = 0; c < _length; ++c)
            {
                i.Enqueue((Test.En)(c % 3));
            }

            Test.MyInterface_OpQEnSResult result = await p.opQEnSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Stack<Test.En>();
            for (int c = 0; c < _length; ++c)
            {
                i.Push((Test.En)(c % 3));
            }

            Test.MyInterface_OpSEnSResult result = await p.opSEnSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Custom<int>();
            for (int c = 0; c < _length; ++c)
            {
                i.Add(c);
            }

            Test.MyInterface_OpCustomIntSResult result = await p.opCustomIntSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
        }

        {
            var i = new Custom<Test.CV>();
            for (int c = 0; c < _length; ++c)
            {
                i.Add(new Test.CV(c));
            }

            Test.MyInterface_OpCustomCVSResult result = await p.opCustomCVSAsync(i);
            System.Collections.IEnumerator eo = result.o.GetEnumerator();
            System.Collections.IEnumerator er = result.returnValue.GetEnumerator();
            foreach (Test.CV obj in i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.i == ((Test.CV)eo.Current).i);
                test(obj.i == ((Test.CV)er.Current).i);
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

            Test.MyInterface_OpCustomIntSSResult result = await p.opCustomIntSSAsync(i);
            test(Enumerable.SequenceEqual(i, result.o));
            test(Enumerable.SequenceEqual(i, result.returnValue));
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

            Test.MyInterface_OpCustomCVSSResult result = await p.opCustomCVSSAsync(i);
            IEnumerator<Custom<Test.CV>> eo = result.o.GetEnumerator();
            IEnumerator<Custom<Test.CV>> er = result.returnValue.GetEnumerator();
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
