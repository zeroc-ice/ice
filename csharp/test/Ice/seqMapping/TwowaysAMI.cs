//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Threading.Tasks;

namespace Ice
{
    namespace seqMapping
    {
        public class TwowaysAMI
        {
            private static void test(bool b)
            {
                if(!b)
                {
                    throw new System.SystemException();
                }
            }

            static readonly int _length = 100;

            internal static async Task twowaysAMI(Communicator communicator, Test.MyClassPrx p)
            {
                {
                    byte[] i = new byte[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] =(byte)c;
                    }
                    var result = await p.opAByteSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new List<byte>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add((byte)c);
                    }

                    var result = await p.opLByteSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new LinkedList<byte>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.AddLast((byte)c);
                    }

                    var result = await p.opKByteSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Queue<byte>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Enqueue((byte)c);
                    }

                    var result = await p.opQByteSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Stack<byte>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Push((byte)c);
                    }

                    var result = await p.opSByteSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    bool[] i = new bool[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] = c % 1 == 1;
                    }

                    var result = await p.opABoolSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    List<bool> i = new List<bool>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add(c % 1 == 1);
                    }

                    var result = await p.opLBoolSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new LinkedList<bool>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.AddLast(c % 1 == 1);
                    }

                    var result = await p.opKBoolSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Queue<bool>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Enqueue(c % 1 == 1);
                    }

                    var result = await p.opQBoolSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Stack<bool>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Push(c % 1 == 1);
                    }

                    var result = await p.opSBoolSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    short[] i = new short[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] =(short)c;
                    }

                    var result = await p.opAShortSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new List<short>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add((short)c);
                    }

                    var result = await p.opLShortSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new LinkedList<short>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.AddLast((short)c);
                    }

                    var result = await p.opKShortSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Queue<short>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Enqueue((short)c);
                    }

                    var result = await p.opQShortSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Stack<short>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Push((short)c);
                    }

                    var result = await p.opSShortSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    int[] i = new int[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] =c;
                    }

                    var result = await p.opAIntSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new List<int>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add((int)c);
                    }

                    var result = await p.opLIntSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new LinkedList<int>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.AddLast(c);
                    }

                    var result = await p.opKIntSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Queue<int>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Enqueue(c);
                    }

                    var result = await p.opQIntSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Stack<int>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Push(c);
                    }

                    var result = await p.opSIntSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    long[] i = new long[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] = c;
                    }

                    var result = await p.opALongSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new List<long>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add(c);
                    }

                    var result = await p.opLLongSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new LinkedList<long>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.AddLast((long)c);
                    }

                    var result = await p.opKLongSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Queue<long>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Enqueue(c);
                    }

                    var result = await p.opQLongSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Stack<long>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Push(c);
                    }

                    var result = await p.opSLongSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    float[] i = new float[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] = c;
                    }

                    var result = await p.opAFloatSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new List<float>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add(c);
                    }

                    var result = await p.opLFloatSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new LinkedList<float>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.AddLast(c);
                    }

                    var result = await p.opKFloatSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Queue<float>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Enqueue(c);
                    }

                    var result = await p.opQFloatSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Stack<float>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Push(c);
                    }

                    var result = await p.opSFloatSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    double[] i = new double[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] =c;
                    }

                    var result = await p.opADoubleSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new List<double>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add(c);
                    }

                    var result = await p.opLDoubleSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new LinkedList<double>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.AddLast(c);
                    }

                    var result = await p.opKDoubleSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Queue<double>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Enqueue(c);
                    }

                    var result = await p.opQDoubleSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Stack<double>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Push(c);
                    }

                    var result = await p.opSDoubleSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    string[] i = new string[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] = c.ToString();
                    }

                    var result = await p.opAStringSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new List<string>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add(c.ToString());
                    }

                    var result = await p.opLStringSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new LinkedList<string>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.AddLast(c.ToString());
                    }

                    var result = await p.opKStringSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Queue<string>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Enqueue(c.ToString());
                    }

                    var result = await p.opQStringSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Stack<string>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Push(c.ToString());
                    }

                    var result = await p.opSStringSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    Value[] i = new Value[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] = new Test.CV(c);
                    }

                    var result = await p.opAObjectSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new List<Value>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add(new Test.CV(c));
                    }

                    var result = await p.opLObjectSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    ObjectPrx[] i = new ObjectPrx[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] = communicator.stringToProxy(c.ToString());
                    }

                    var result = await p.opAObjectPrxSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new List<ObjectPrx>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add(communicator.stringToProxy(c.ToString()));
                    }

                    var result = await p.opLObjectPrxSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new LinkedList<Ice.ObjectPrx>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.AddLast(communicator.stringToProxy(c.ToString()));
                    }

                    var result = await p.opKObjectPrxSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Queue<ObjectPrx>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Enqueue(communicator.stringToProxy(c.ToString()));
                    }

                    var result = await p.opQObjectPrxSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Stack<ObjectPrx>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Push(communicator.stringToProxy(c.ToString()));
                    }

                    var result = await p.opSObjectPrxSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Test.S[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c].i = c;
                    }

                    var result = await p.opAStructSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new List<Test.S>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add(new Test.S(c));
                    }

                    var result = await p.opLStructSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new LinkedList<Test.S>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.AddLast(new Test.S(c));
                    }

                    var result = await p.opKStructSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Queue<Test.S>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Enqueue(new Test.S(c));
                    }

                    var result = await p.opQStructSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Stack<Test.S>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Push(new Test.S(c));
                    }

                    var result = await p.opSStructSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Test.CV[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] = new Test.CV(c);
                    }

                    var result = await p.opACVSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new List<Test.CV>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add(new Test.CV(c));
                    }

                    var result = await p.opLCVSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    Test.IPrx[] i = new Test.IPrx[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] = Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString()));
                    }

                    var result = await p.opAIPrxSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new List<Test.IPrx>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add(Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
                    }

                    var result = await p.opLIPrxSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new LinkedList<Test.IPrx>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.AddLast(Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
                    }

                    var result = await p.opKIPrxSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Queue<Test.IPrx>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Enqueue(Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
                    }

                    var result = await p.opQIPrxSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    Stack<Test.IPrx> i = new Stack<Test.IPrx>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Push(Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
                    }

                    var result = await p.opSIPrxSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Test.CR[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] = new Test.CR(new Test.CV(c));
                    }

                    var result = await p.opACRSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new List<Test.CR>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add(new Test.CR(new Test.CV(c)));
                    }

                    var result = await p.opLCRSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Test.En[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] =(Test.En)(c % 3);
                    }

                    var result = await p.opAEnSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new List<Test.En>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add((Test.En)(c % 3));
                    }

                    var result = await p.opLEnSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new LinkedList<Test.En>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.AddLast((Test.En)(c % 3));
                    }

                    var result = await p.opKEnSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Queue<Test.En>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Enqueue((Test.En)(c % 3));
                    }

                    var result = await p.opQEnSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Stack<Test.En>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Push((Test.En)(c % 3));
                    }

                    var result = await p.opSEnSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Custom<int>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add(c);
                    }

                    var result = await p.opCustomIntSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Custom<Test.CV>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add(new Test.CV(c));
                    }

                    var result = await p.opCustomCVSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
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

                    var result = await p.opCustomIntSSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }

                {
                    var i = new Custom<Custom<Test.CV>>();
                    for(int c = 0; c < _length; ++c)
                    {
                        var inner = new Custom<Test.CV>();
                        for(int j = 0; j < c; ++j)
                        {
                            inner.Add(new Test.CV(j));
                        }
                        i.Add(inner);
                    }

                    var result = await p.opCustomCVSSAsync(i);
                    test(CollectionComparer.Equals(i, result.o));
                    test(CollectionComparer.Equals(i, result.returnValue));
                }
            }
        }
    }
}
