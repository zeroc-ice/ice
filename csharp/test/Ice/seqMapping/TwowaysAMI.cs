//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;
using System.Linq;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;

namespace Ice
{
    namespace seqMapping
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

            internal static void twowaysAMI(Ice.Communicator communicator, Test.MyClassPrx p)
            {
                {
                    var i = Enumerable.Range(0, Length).Select(x => (byte)x).ToArray();
                    var r = p.opAByteSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => (byte)x).ToList();
                    var r = p.opLByteSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new LinkedList<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                    var r = p.opKByteSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Queue<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                    var r = p.opQByteSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Stack<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                    var r = p.opSByteSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => x % 1 == 1).ToArray();
                    var r = p.opABoolSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => x % 1 == 1).ToList();
                    var r = p.opLBoolSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new LinkedList<bool>(Enumerable.Range(0, Length).Select(x => x % 1 == 1).ToArray());
                    var r = p.opKBoolSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Queue<bool>(Enumerable.Range(0, Length).Select(x => x % 1 == 1).ToArray());
                    var r = p.opQBoolSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Stack<bool>(Enumerable.Range(0, Length).Select(x => x % 1 == 1).ToArray());
                    var r = p.opSBoolSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => (short)x).ToArray();
                    var r = p.opAShortSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => (short)x).ToList();
                    var r = p.opLShortSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new LinkedList<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                    var r = p.opKShortSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Queue<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                    var r = p.opQShortSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Stack<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                    var r = p.opSShortSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).ToArray();
                    var r = p.opAIntSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).ToList();
                    var r = p.opLIntSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new LinkedList<int>(Enumerable.Range(0, Length).ToArray());
                    var r = p.opKIntSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Queue<int>(Enumerable.Range(0, Length).ToArray());
                    var r = p.opQIntSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Stack<int>(Enumerable.Range(0, Length).ToArray());
                    var r = p.opSIntSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => (long)x).ToArray();
                    var r = p.opALongSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => (long)x).ToList();
                    var r = p.opLLongSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new LinkedList<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                    var r = p.opKLongSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Queue<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                    var r = p.opQLongSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Stack<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                    var r = p.opSLongSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => (float)x).ToArray();
                    var r = p.opAFloatSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => (float)x).ToList();
                    var r = p.opLFloatSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new LinkedList<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                    var r = p.opKFloatSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Queue<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                    var r = p.opQFloatSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Stack<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                    var r = p.opSFloatSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => (double)x).ToArray();
                    var r = p.opADoubleSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => (double)x).ToList();
                    var r = p.opLDoubleSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new LinkedList<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                    var r = p.opKDoubleSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Queue<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                    var r = p.opQDoubleSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Stack<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                    var r = p.opSDoubleSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray();
                    var r = p.opAStringSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => x.ToString()).ToList();
                    var r = p.opLStringSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new LinkedList<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                    var r = p.opKStringSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Queue<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                    var r = p.opQStringSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Stack<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                    var r = p.opSStringSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => new Test.CV(x) as Value).ToArray();
                    var r = p.opAObjectSAsync(i).Result;
                    test(r.o.SequenceEqual(i, new CVComparer()));
                    test(r.returnValue.SequenceEqual(i, new CVComparer()));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => new Test.CV(x) as Value).ToList();
                    var r = p.opLObjectSAsync(i).Result;
                    test(r.o.SequenceEqual(i, new CVComparer()));
                    test(r.returnValue.SequenceEqual(i, new CVComparer()));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(
                        x => communicator.stringToProxy(x.ToString())).ToArray();
                    var r = p.opAObjectPrxSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(
                        x => communicator.stringToProxy(x.ToString())).ToList();
                    var r = p.opLObjectPrxSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Queue<ObjectPrx>(Enumerable.Range(0, Length).Select(
                        x => communicator.stringToProxy(x.ToString())).ToArray());
                    var r = p.opQObjectPrxSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Stack<ObjectPrx>(Enumerable.Range(0, Length).Select(
                        x => communicator.stringToProxy(x.ToString())).ToArray());
                    var r = p.opSObjectPrxSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => new Test.S(x)).ToArray();
                    var r = p.opAStructSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => new Test.S(x)).ToList();
                    var r = p.opLStructSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new LinkedList<Test.S>(Enumerable.Range(0, Length).Select(x => new Test.S(x)).ToArray());
                    var r = p.opKStructSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Queue<Test.S>(Enumerable.Range(0, Length).Select(x => new Test.S(x)).ToArray());
                    var r = p.opQStructSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Stack<Test.S>(Enumerable.Range(0, Length).Select(x => new Test.S(x)).ToArray());
                    var r = p.opSStructSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => new Test.CV(x)).ToArray();
                    var r = p.opACVSAsync(i).Result;
                    test(r.o.SequenceEqual(i, new CVComparer()));
                    test(r.returnValue.SequenceEqual(i, new CVComparer()));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => new Test.CV(x)).ToList();
                    var r = p.opLCVSAsync(i).Result;
                    test(r.o.SequenceEqual(i, new CVComparer()));
                    test(r.returnValue.SequenceEqual(i, new CVComparer()));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(
                        x => Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(x.ToString()))).ToArray();
                    var r = p.opAIPrxSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(
                        x => Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(x.ToString()))).ToList();
                    var r = p.opLIPrxSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new LinkedList<Test.IPrx>(Enumerable.Range(0, Length).Select(
                        x => Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(x.ToString()))).ToArray());
                    var r = p.opKIPrxSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Queue<Test.IPrx>(Enumerable.Range(0, Length).Select(
                        x => Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(x.ToString()))).ToArray());
                    var r = p.opQIPrxSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Stack<Test.IPrx>(Enumerable.Range(0, Length).Select(
                        x => Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(x.ToString()))).ToArray());
                    var r = p.opSIPrxSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => new Test.CR(new Test.CV(x))).ToArray();
                    var r = p.opACRSAsync(i).Result;
                    test(r.o.SequenceEqual(i, new CRComparer()));
                    test(r.returnValue.SequenceEqual(i, new CRComparer()));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => new Test.CR(new Test.CV(x))).ToList();
                    var r = p.opLCRSAsync(i).Result;
                    test(r.o.SequenceEqual(i, new CRComparer()));
                    test(r.returnValue.SequenceEqual(i, new CRComparer()));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => (Test.En)(x % 3)).ToArray();
                    var r = p.opAEnSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => (Test.En)(x % 3)).ToList();
                    var r = p.opLEnSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new LinkedList<Test.En>(Enumerable.Range(0, Length).Select(
                        x => (Test.En)(x % 3)).ToArray());
                    var r = p.opKEnSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Queue<Test.En>(Enumerable.Range(0, Length).Select(x => (Test.En)(x % 3)).ToArray());
                    var r = p.opQEnSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Stack<Test.En>(Enumerable.Range(0, Length).Select(x => (Test.En)(x % 3)).ToArray());
                    var r = p.opSEnSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Custom<int>(Enumerable.Range(0, Length).ToList());
                    var r = p.opCustomIntSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
                }

                {
                    var i = new Custom<Test.CV>(Enumerable.Range(0, Length).Select(x => new Test.CV(x)).ToList());
                    var r = p.opCustomCVSAsync(i).Result;
                    test(r.o.SequenceEqual(i, new CVComparer()));
                    test(r.returnValue.SequenceEqual(i, new CVComparer()));
                }

                {
                    var i = new Custom<Custom<int>>();
                    for (int c = 0; c < Length; ++c)
                    {
                        i.Add(new Custom<int>(Enumerable.Range(0, Length).ToList()));
                    }
                    var r = p.opCustomIntSSAsync(i).Result;
                    test(r.o.SequenceEqual(i));
                    test(r.returnValue.SequenceEqual(i));
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
                        test(r.returnValue[c].SequenceEqual(i[c], new CVComparer()));
                    }
                }

                {
                    var r = p.opSerialSmallCSharpAsync(null).Result;
                    test(r.o == null);
                    test(r.returnValue == null);
                }

                {
                    var i = new Serialize.Small();
                    i.i = 99;

                    var r = p.opSerialSmallCSharpAsync(i).Result;
                    test(r.o.i == 99);
                    test(r.returnValue.i == 99);
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

                    test(r.returnValue.d1 == 1.0);
                    test(r.returnValue.d2 == 2.0);
                    test(r.returnValue.d3 == 3.0);
                    test(r.returnValue.d4 == 4.0);
                    test(r.returnValue.d5 == 5.0);
                    test(r.returnValue.d6 == 6.0);
                    test(r.returnValue.d7 == 7.0);
                    test(r.returnValue.d8 == 8.0);
                    test(r.returnValue.d9 == 9.0);
                    test(r.returnValue.d10 == 10.0);
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

                    test(r.returnValue.o == null);
                    test(r.returnValue.o2 == r.returnValue);
                    test(r.returnValue.s == null);
                    test(r.returnValue.s2 == "Hello");
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

            public int GetHashCode([DisallowNull] T obj)
            {
                return obj.GetHashCode();
            }
        }

        class CVComparer : IEqualityComparer<Value>
        {
            public bool Equals([AllowNull] Value x, [AllowNull] Value y)
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

            public int GetHashCode([DisallowNull] Value obj)
            {
                return obj.GetHashCode();
            }
        }

        class CRComparer : IEqualityComparer<Test.CR>
        {
            public bool Equals([AllowNull] Test.CR x, [AllowNull] Test.CR y)
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

            public int GetHashCode([DisallowNull] Test.CR obj)
            {
                return obj.GetHashCode();
            }
        }
    }
}
