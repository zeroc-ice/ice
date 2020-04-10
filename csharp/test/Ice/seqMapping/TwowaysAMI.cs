//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Linq;
using System.Collections.Generic;

namespace Ice.seqMapping
{
    public class TwowaysAMI
    {
        private static void test(bool b)
        {
            if (!b)
            {
                throw new System.SystemException();
            }
        }

        const int Length = 100;

        internal static void twowaysAMI(Ice.Communicator communicator, Test.IMyClassPrx p)
        {
            {
                var i = Enumerable.Range(0, Length).Select(x => (byte)x).ToArray();
                var r = p.opAByteSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (byte)x).ToList();
                var r = p.opLByteSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                var r = p.opKByteSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                var r = p.opQByteSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                var r = p.opSByteSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => x % 1 == 1).ToArray();
                var r = p.opABoolSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => x % 1 == 1).ToList();
                var r = p.opLBoolSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<bool>(Enumerable.Range(0, Length).Select(x => x % 1 == 1).ToArray());
                var r = p.opKBoolSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<bool>(Enumerable.Range(0, Length).Select(x => x % 1 == 1).ToArray());
                var r = p.opQBoolSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<bool>(Enumerable.Range(0, Length).Select(x => x % 1 == 1).ToArray());
                var r = p.opSBoolSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (short)x).ToArray();
                var r = p.opAShortSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (short)x).ToList();
                var r = p.opLShortSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                var r = p.opKShortSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                var r = p.opQShortSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                var r = p.opSShortSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).ToArray();
                var r = p.opAIntSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).ToList();
                var r = p.opLIntSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<int>(Enumerable.Range(0, Length).ToArray());
                var r = p.opKIntSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<int>(Enumerable.Range(0, Length).ToArray());
                var r = p.opQIntSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<int>(Enumerable.Range(0, Length).ToArray());
                var r = p.opSIntSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (long)x).ToArray();
                var r = p.opALongSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (long)x).ToList();
                var r = p.opLLongSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                var r = p.opKLongSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                var r = p.opQLongSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                var r = p.opSLongSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (float)x).ToArray();
                var r = p.opAFloatSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (float)x).ToList();
                var r = p.opLFloatSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                var r = p.opKFloatSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                var r = p.opQFloatSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                var r = p.opSFloatSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (double)x).ToArray();
                var r = p.opADoubleSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (double)x).ToList();
                var r = p.opLDoubleSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                var r = p.opKDoubleSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                var r = p.opQDoubleSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                var r = p.opSDoubleSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray();
                var r = p.opAStringSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => x.ToString()).ToList();
                var r = p.opLStringSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                var r = p.opKStringSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                var r = p.opQStringSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                var r = p.opSStringSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.CV(x) as AnyClass).ToArray();
                var r = p.opAObjectSAsync(i).Result;
                test(r.o.SequenceEqual(i, new CVComparer()));
                test(r.ReturnValue.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.CV(x) as AnyClass).ToList();
                var r = p.opLObjectSAsync(i).Result;
                test(r.o.SequenceEqual(i, new CVComparer()));
                test(r.ReturnValue.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray();
                var r = p.opAObjectPrxSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToList();
                var r = p.opLObjectPrxSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<IObjectPrx>(Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray());
                var r = p.opQObjectPrxSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<IObjectPrx>(Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray());
                var r = p.opSObjectPrxSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.S(x)).ToArray();
                var r = p.opAStructSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.S(x)).ToList();
                var r = p.opLStructSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<Test.S>(Enumerable.Range(0, Length).Select(x => new Test.S(x)).ToArray());
                var r = p.opKStructSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<Test.S>(Enumerable.Range(0, Length).Select(x => new Test.S(x)).ToArray());
                var r = p.opQStructSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<Test.S>(Enumerable.Range(0, Length).Select(x => new Test.S(x)).ToArray());
                var r = p.opSStructSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.CV(x)).ToArray();
                var r = p.opACVSAsync(i).Result;
                test(r.o.SequenceEqual(i, new CVComparer()));
                test(r.ReturnValue.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.CV(x)).ToList();
                var r = p.opLCVSAsync(i).Result;
                test(r.o.SequenceEqual(i, new CVComparer()));
                test(r.ReturnValue.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => Test.IIPrx.Parse(x.ToString(), communicator)).ToArray();
                var r = p.opAIPrxSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => Test.IIPrx.Parse(x.ToString(), communicator)).ToList();
                var r = p.opLIPrxSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<Test.IIPrx>(Enumerable.Range(0, Length).Select(
                    x => Test.IIPrx.Parse(x.ToString(), communicator)).ToArray());
                var r = p.opKIPrxSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<Test.IIPrx>(Enumerable.Range(0, Length).Select(
                    x => Test.IIPrx.Parse(x.ToString(), communicator)).ToArray());
                var r = p.opQIPrxSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<Test.IIPrx>(Enumerable.Range(0, Length).Select(
                    x => Test.IIPrx.Parse(x.ToString(), communicator)).ToArray());
                var r = p.opSIPrxSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.CR(new Test.CV(x))).ToArray();
                var r = p.opACRSAsync(i).Result;
                test(r.o.SequenceEqual(i, new CRComparer()));
                test(r.ReturnValue.SequenceEqual(i, new CRComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.CR(new Test.CV(x))).ToList();
                var r = p.opLCRSAsync(i).Result;
                test(r.o.SequenceEqual(i, new CRComparer()));
                test(r.ReturnValue.SequenceEqual(i, new CRComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (Test.En)(x % 3)).ToArray();
                var r = p.opAEnSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (Test.En)(x % 3)).ToList();
                var r = p.opLEnSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<Test.En>(Enumerable.Range(0, Length).Select(
                    x => (Test.En)(x % 3)).ToArray());
                var r = p.opKEnSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<Test.En>(Enumerable.Range(0, Length).Select(x => (Test.En)(x % 3)).ToArray());
                var r = p.opQEnSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<Test.En>(Enumerable.Range(0, Length).Select(x => (Test.En)(x % 3)).ToArray());
                var r = p.opSEnSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<int>(Enumerable.Range(0, Length).ToList());
                var r = p.opCustomIntSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<Test.CV>(Enumerable.Range(0, Length).Select(x => new Test.CV(x)).ToList());
                var r = p.opCustomCVSAsync(i).Result;
                test(r.o.SequenceEqual(i, new CVComparer()));
                test(r.ReturnValue.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = new Custom<Custom<int>>();
                for (int c = 0; c < Length; ++c)
                {
                    i.Add(new Custom<int>(Enumerable.Range(0, Length).ToList()));
                }
                var r = p.opCustomIntSSAsync(i).Result;
                test(r.o.SequenceEqual(i));
                test(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<Custom<Test.CV>>();
                for (int c = 0; c < Length; ++c)
                {
                    i.Add(new Custom<Test.CV>(Enumerable.Range(0, Length).Select(x => new Test.CV(x)).ToList()));
                }
                var r = p.opCustomCVSSAsync(i).Result;
                for (int c = 0; c < Length; ++c)
                {
                    test(r.o[c].SequenceEqual(i[c], new CVComparer()));
                    test(r.ReturnValue[c].SequenceEqual(i[c], new CVComparer()));
                }
            }

            {
                var r = p.opSerialSmallCSharpAsync(null).Result;
                test(r.o == null);
                test(r.ReturnValue == null);
            }

            {
                var i = new Serialize.Small();
                i.i = 99;

                var r = p.opSerialSmallCSharpAsync(i).Result;
                test(r.o.i == 99);
                test(r.ReturnValue.i == 99);
            }

            {
                var i = new Serialize.Large();
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
                i.d11 = 11.0;
                i.s1 = Serialize.Large.LargeString;

                var r = p.opSerialLargeCSharpAsync(i).Result;

                test(r.o.d1 == 1.0);
                test(r.o.d2 == 2.0);
                test(r.o.d3 == 3.0);
                test(r.o.d4 == 4.0);
                test(r.o.d5 == 5.0);
                test(r.o.d6 == 6.0);
                test(r.o.d7 == 7.0);
                test(r.o.d8 == 8.0);
                test(r.o.d9 == 9.0);
                test(r.o.d10 == 10.0);
                test(r.o.d11 == 11.0);
                test(r.o.s1 == Serialize.Large.LargeString);

                test(r.ReturnValue.d1 == 1.0);
                test(r.ReturnValue.d2 == 2.0);
                test(r.ReturnValue.d3 == 3.0);
                test(r.ReturnValue.d4 == 4.0);
                test(r.ReturnValue.d5 == 5.0);
                test(r.ReturnValue.d6 == 6.0);
                test(r.ReturnValue.d7 == 7.0);
                test(r.ReturnValue.d8 == 8.0);
                test(r.ReturnValue.d9 == 9.0);
                test(r.ReturnValue.d10 == 10.0);
                test(r.ReturnValue.d11 == 11.0);
                test(r.ReturnValue.s1 == Serialize.Large.LargeString);
            }

            {
                var i = new Serialize.Struct();
                i.o = null;
                i.o2 = i;
                i.s = null;
                i.s2 = "Hello";

                var r = p.opSerialStructCSharpAsync(i).Result;

                test(r.o.o == null);
                test(r.o.o2 == r.o);
                test(r.o.s == null);
                test(r.o.s2 == "Hello");

                test(r.ReturnValue.o == null);
                test(r.ReturnValue.o2 == r.ReturnValue);
                test(r.ReturnValue.s == null);
                test(r.ReturnValue.s2 == "Hello");
            }
        }
    }

    class ValueComparer<T> : IEqualityComparer<T>
    {
        public bool Equals(T x, T y)
        {
            if (ReferenceEquals(x, y))
            {
                return true;
            }

            if (x == null || y == null)
            {
                return false;
            }

            return x.Equals(y);
        }

        public int GetHashCode(T obj) => obj.GetHashCode();
    }

    class CVComparer : IEqualityComparer<AnyClass>
    {
        public bool Equals(AnyClass x, AnyClass y)
        {
            if (x == y)
            {
                return true;
            }

            if (x == null || y == null)
            {
                return false;
            }
            return (x as Test.CV).i == (y as Test.CV).i;
        }

        public int GetHashCode(AnyClass obj)
        {
            return obj.GetHashCode();
        }
    }

    class CRComparer : IEqualityComparer<Test.CR>
    {
        public bool Equals(Test.CR x, Test.CR y)
        {
            if (x == y)
            {
                return true;
            }

            if (x == null || y == null)
            {
                return false;
            }
            return x.v.i == y.v.i;
        }

        public int GetHashCode(Test.CR obj) => obj.GetHashCode();
    }
}
