//
// Copyright(c) ZeroC, Inc. All rights reserved.
//

using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using Test;

namespace ZeroC.Ice.Test.Stream
{
    public class AllTests
    {
        //
        // There does not appear to be any way to compare collections
        // in either C# or with the .NET framework. Something like
        // C++ STL EqualRange would be n..
        //
        private static bool Compare(ICollection c1, ICollection c2)
        {
            if (c1 == null)
            {
                return c2 == null;
            }
            if (c2 == null)
            {
                return false;
            }
            if (!c1.GetType().Equals(c2.GetType()))
            {
                return false;
            }

            if (c1.Count != c2.Count)
            {
                return false;
            }

            IEnumerator i1 = c1.GetEnumerator();
            IEnumerator i2 = c2.GetEnumerator();
            while (i1.MoveNext())
            {
                i2.MoveNext();
                TestHelper.Assert(i1.Current != null);
                if (i1.Current is ICollection)
                {
                    TestHelper.Assert(i2.Current is ICollection);
                    if (!Compare((ICollection)i1.Current, (ICollection)i2.Current))
                    {
                        return false;
                    }
                }
                else if (!i1.Current.Equals(i2.Current))
                {
                    return false;
                }
            }
            return true;
        }

        public static int allTests(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();

            System.IO.TextWriter output = helper.GetWriter();
            output.Write("testing primitive types... ");
            output.Flush();

            // TODO rework this once the new streaming API is finished
            /*{
                ostr = new OutputStream(communicator);
                ostr.StartEncapsulation();
                ostr.WriteBool(true);
                ostr.EndEncapsulation();
                var data = ostr.ToArray();

                istr = new InputStream(communicator, data);
                istr.StartEncapsulation();
                test(istr.ReadBool());
                istr.EndEncapsulation();

                istr = new InputStream(communicator, data);
                istr.StartEncapsulation();
                test(istr.ReadBool());
                istr.EndEncapsulation();
            }

            {
                var data = new byte[0];
                istr = new InputStream(communicator, data);
                try
                {
                    istr.ReadBool();
                    test(false);
                }
                catch (System.ArgumentOutOfRangeException)
                {
                }
            }

            {
                ostr = new OutputStream(communicator);
                ostr.WriteBool(true);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                test(istr.ReadBool());
            }

            {
                ostr = new OutputStream(communicator);
                ostr.WriteByte(1);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                test(istr.ReadByte() == 1);
            }

            {
                ostr = new OutputStream(communicator);
                ostr.WriteShort(2);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                test(istr.ReadShort() == 2);
            }

            {
                ostr = new OutputStream(communicator);
                ostr.WriteInt(3);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                test(istr.ReadInt() == 3);
            }

            {
                ostr = new OutputStream(communicator);
                ostr.WriteLong(4);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                test(istr.ReadLong() == 4);
            }

            {
                ostr = new OutputStream(communicator);
                ostr.WriteFloat((float)5.0);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                test(istr.ReadFloat() == (float)5.0);
            }

            {
                ostr = new OutputStream(communicator);
                ostr.WriteDouble(6.0);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                test(istr.ReadDouble() == 6.0);
            }

            {
                ostr = new OutputStream(communicator);
                ostr.WriteString("hello world");
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                test(istr.ReadString().Equals("hello world"));
            }

            output.WriteLine("ok");

            output.Write("testing constructed types... ");
            output.Flush();

            {
                ostr = new OutputStream(communicator);
                ostr.Write(MyEnum.enum3);
                byte[] data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                MyEnum e = istr.ReadMyEnum();
                test(e == MyEnum.enum3);
            }

            {
                ostr = new OutputStream(communicator);
                var s = new SmallStruct();
                s.bo = true;
                s.by = 1;
                s.sh = 2;
                s.i = 3;
                s.l = 4;
                s.f = 5.0f;
                s.d = 6.0;
                s.str = "7";
                s.e = MyEnum.enum2;
                s.p = IMyInterfacePrx.Parse("test:default", communicator);
                ostr.WriteStruct(s);
                var data = ostr.ToArray();
                var s2 = new SmallStruct(new InputStream(communicator, data));
                test(s2.Equals(s));
            }

            {
                ostr = new OutputStream(communicator);
                var o = new ClassWithTaggedMembers();
                o.bo = true;
                o.by = 5;
                o.sh = 4;
                o.i = 3;
                // Can only read/write classes within encaps
                ostr.StartEncapsulation();
                ostr.WriteClass(o);
                ostr.EndEncapsulation();
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                istr.StartEncapsulation();
                var o2 = istr.ReadClass<ClassWithTaggedMembers>();
                istr.EndEncapsulation();
                test(o2.bo == o.bo);
                test(o2.by == o.by);
                test(o2.sh == o.sh);
                test(o2.i == o.i);
            }

            {
                bool[] arr = { true, false, true, false };
                ostr = new OutputStream(communicator);
                ostr.WriteBoolSeq(arr);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var arr2 = istr.ReadBoolArray();
                test(Compare(arr2, arr));

                bool[][] arrS = { arr, new bool[0], arr };
                ostr = new OutputStream(communicator);
                ostr.Write(arrS);
                data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var arr2S = istr.ReadBoolSS();
                test(Compare(arr2S, arrS));
            }

            {
                byte[] arr = { 0x01, 0x11, 0x12, 0x22 };
                ostr = new OutputStream(communicator);
                ostr.WriteByteSeq(arr);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var arr2 = istr.ReadByteArray();
                test(Compare(arr2, arr));

                byte[][] arrS = { arr, new byte[0], arr };
                ostr = new OutputStream(communicator);
                ostr.Write(arrS);
                data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var arr2S = istr.ReadByteSS();
                test(Compare(arr2S, arrS));
            }

            {
                var small = new Serialize.Small();
                small.i = 99;
                ostr = new OutputStream(communicator);
                ostr.WriteSerializable(small);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var small2 = (Serialize.Small)istr.ReadSerializable();
                test(small2.i == 99);
            }

            {
                short[] arr = { 0x01, 0x11, 0x12, 0x22 };
                ostr = new OutputStream(communicator);
                ostr.WriteShortSeq(arr);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var arr2 = istr.ReadShortArray();
                test(Compare(arr2, arr));

                short[][] arrS = { arr, new short[0], arr };
                ostr = new OutputStream(communicator);
                ostr.Write(arrS);
                data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var arr2S = istr.ReadShortSS();
                test(Compare(arr2S, arrS));
            }

            {
                int[] arr = { 0x01, 0x11, 0x12, 0x22 };
                ostr = new OutputStream(communicator);
                ostr.WriteIntSeq(arr);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var arr2 = istr.ReadIntArray();
                test(Compare(arr2, arr));

                int[][] arrS = { arr, new int[0], arr };
                ostr = new OutputStream(communicator);
                ostr.Write(arrS);
                data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var arr2S = istr.ReadIntSS();
                test(Compare(arr2S, arrS));
            }

            {
                long[] arr = { 0x01, 0x11, 0x12, 0x22 };
                ostr = new OutputStream(communicator);
                ostr.WriteLongSeq(arr);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var arr2 = istr.ReadLongArray();
                test(Compare(arr2, arr));

                long[][] arrS = { arr, new long[0], arr };
                ostr = new OutputStream(communicator);
                ostr.Write(arrS);
                data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var arr2S = istr.ReadLongSS();
                test(Compare(arr2S, arrS));
            }

            {
                float[] arr = { 1, 2, 3, 4 };
                ostr = new OutputStream(communicator);
                ostr.WriteFloatSeq(arr);
                byte[] data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                float[] arr2 = istr.ReadFloatArray();
                test(Compare(arr2, arr));

                float[][] arrS = { arr, new float[0], arr };
                ostr = new OutputStream(communicator);
                ostr.Write(arrS);
                data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var arr2S = istr.ReadFloatSS();
                test(Compare(arr2S, arrS));
            }

            {
                double[] arr =
                    {
                        1,
                        2,
                        3,
                        4
                    };
                ostr = new OutputStream(communicator);
                ostr.WriteDoubleSeq(arr);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var arr2 = istr.ReadDoubleArray();
                test(Compare(arr2, arr));

                double[][] arrS = { arr, new double[0], arr };
                ostr = new OutputStream(communicator);
                ostr.Write(arrS);
                data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var arr2S = istr.ReadDoubleSS();
                test(Compare(arr2S, arrS));
            }

            {
                string[] arr = { "string1", "string2", "string3", "string4" };
                ostr = new OutputStream(communicator);
                ostr.WriteStringSeq(arr);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var arr2 = istr.ReadStringArray();
                test(Compare(arr2, arr));

                string[][] arrS = { arr, new string[0], arr };
                ostr = new OutputStream(communicator);
                ostr.Write(arrS);
                data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var arr2S = istr.ReadStringSS();
                test(Compare(arr2S, arrS));
            }

            {
                MyEnum[] arr = {
                    MyEnum.enum3,
                    MyEnum.enum2,
                    MyEnum.enum1,
                    MyEnum.enum2
                };
                ostr = new OutputStream(communicator);
                ostr.Write(arr);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var arr2 = istr.ReadMyEnumS();
                test(Compare(arr2, arr));

                MyEnum[][] arrS = { arr, new MyEnum[0], arr };
                ostr = new OutputStream(communicator);
                ostr.Write(arrS);
                data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var arr2S = istr.ReadMyEnumSS();
                test(Compare(arr2S, arrS));
            }

            var smallStructArray = new SmallStruct[3];
            for (int i = 0; i < smallStructArray.Length; ++i)
            {
                smallStructArray[i] = new SmallStruct();
                smallStructArray[i].bo = true;
                smallStructArray[i].by = 1;
                smallStructArray[i].sh = 2;
                smallStructArray[i].i = 3;
                smallStructArray[i].l = 4;
                smallStructArray[i].f = 5.0f;
                smallStructArray[i].d = 6.0;
                smallStructArray[i].str = "7";
                smallStructArray[i].e = MyEnum.enum2;
                smallStructArray[i].p = IMyInterfacePrx.Parse("test:default", communicator);
            }

            var myClassArray = new MyClass[4];
            for (int i = 0; i < myClassArray.Length; ++i)
            {
                myClassArray[i] = new MyClass();
                myClassArray[i].c = myClassArray[i];
                myClassArray[i].o = myClassArray[i];
                myClassArray[i].s = new SmallStruct(false, 0, 0, 0, 0, 0.0f, 0.0, "", MyEnum.enum1, null);
                myClassArray[i].s.e = MyEnum.enum2;
                myClassArray[i].seq1 = new bool[] { true, false, true, false };
                myClassArray[i].seq2 = new byte[] { 1, 2, 3, 4 };
                myClassArray[i].seq3 = new short[] { 1, 2, 3, 4 };
                myClassArray[i].seq4 = new int[] { 1, 2, 3, 4 };
                myClassArray[i].seq5 = new long[] { 1, 2, 3, 4 };
                myClassArray[i].seq6 = new float[] { 1, 2, 3, 4 };
                myClassArray[i].seq7 = new double[] { 1, 2, 3, 4 };
                myClassArray[i].seq8 = new string[] { "string1", "string2", "string3", "string4" };
                myClassArray[i].seq9 = new MyEnum[] { MyEnum.enum3, MyEnum.enum2, MyEnum.enum1 };
                myClassArray[i].seq10 = new MyClass[4]; // null elements.
                myClassArray[i].d = new Dictionary<string, MyClass>();
                myClassArray[i].d["hi"] = myClassArray[i];
            }

            {
                ostr = new OutputStream(communicator);
                ostr.StartEncapsulation();
                ostr.Write(myClassArray);
                ostr.EndEncapsulation();
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                istr.StartEncapsulation();
                var arr2 = istr.ReadMyClassS();
                istr.EndEncapsulation();
                test(arr2.Length == myClassArray.Length);
                for (int i = 0; i < arr2.Length; ++i)
                {
                    test(arr2[i] != null);
                    test(arr2[i].c == arr2[i]);
                    test(arr2[i].o == arr2[i]);
                    test(arr2[i].s.e == MyEnum.enum2);
                    test(Compare(arr2[i].seq1, myClassArray[i].seq1));
                    test(Compare(arr2[i].seq2, myClassArray[i].seq2));
                    test(Compare(arr2[i].seq3, myClassArray[i].seq3));
                    test(Compare(arr2[i].seq4, myClassArray[i].seq4));
                    test(Compare(arr2[i].seq5, myClassArray[i].seq5));
                    test(Compare(arr2[i].seq6, myClassArray[i].seq6));
                    test(Compare(arr2[i].seq7, myClassArray[i].seq7));
                    test(Compare(arr2[i].seq8, myClassArray[i].seq8));
                    test(Compare(arr2[i].seq9, myClassArray[i].seq9));
                    test(arr2[i].d["hi"].Equals(arr2[i]));
                }

                MyClass[][] arrS = { myClassArray, new MyClass[0], myClassArray };
                ostr = new OutputStream(communicator);
                ostr.StartEncapsulation();
                ostr.Write(arrS);
                ostr.EndEncapsulation();
                data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                istr.StartEncapsulation();
                var arr2S = istr.ReadMyClassSS();
                istr.EndEncapsulation();
                test(arr2S.Length == arrS.Length);
                test(arr2S[0].Length == arrS[0].Length);
                test(arr2S[1].Length == arrS[1].Length);
                test(arr2S[2].Length == arrS[2].Length);

                for (int j = 0; j < arr2S.Length; ++j)
                {
                    for (int k = 0; k < arr2S[j].Length; ++k)
                    {
                        test(arr2S[j][k].c == arr2S[j][k]);
                        test(arr2S[j][k].o == arr2S[j][k]);
                        test(arr2S[j][k].s.e == MyEnum.enum2);
                        test(Compare(arr2S[j][k].seq1, myClassArray[k].seq1));
                        test(Compare(arr2S[j][k].seq2, myClassArray[k].seq2));
                        test(Compare(arr2S[j][k].seq3, myClassArray[k].seq3));
                        test(Compare(arr2S[j][k].seq4, myClassArray[k].seq4));
                        test(Compare(arr2S[j][k].seq5, myClassArray[k].seq5));
                        test(Compare(arr2S[j][k].seq6, myClassArray[k].seq6));
                        test(Compare(arr2S[j][k].seq7, myClassArray[k].seq7));
                        test(Compare(arr2S[j][k].seq8, myClassArray[k].seq8));
                        test(Compare(arr2S[j][k].seq9, myClassArray[k].seq9));
                        test(arr2S[j][k].d["hi"].Equals(arr2S[j][k]));
                    }
                }
            }

            {
                ostr = new OutputStream(communicator);
                var obj = new MyClass();
                obj.s = new SmallStruct(false, 0, 0, 0, 0, 0.0f, 0.0, "", MyEnum.enum1, null);
                obj.s.e = MyEnum.enum2;
                ostr.StartEncapsulation();
                ostr.WriteClass(obj);
                ostr.EndEncapsulation();
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                istr.StartEncapsulation();
                var robj = istr.ReadClass<MyClass>();
                istr.EndEncapsulation();
                test(robj != null);
                test(robj.s.e == MyEnum.enum2);
            }

            {
                ostr = new OutputStream(communicator);
                ostr.StartEncapsulation(ostr.Encoding, FormatType.Sliced);
                var ex = new MyException();

                var c = new MyClass();
                c.c = c;
                c.o = c;
                c.s = new SmallStruct(false, 0, 0, 0, 0, 0.0f, 0.0, "", MyEnum.enum1, null);
                c.s.e = MyEnum.enum2;
                c.seq1 = new bool[] { true, false, true, false };
                c.seq2 = new byte[] { 1, 2, 3, 4 };
                c.seq3 = new short[] { 1, 2, 3, 4 };
                c.seq4 = new int[] { 1, 2, 3, 4 };
                c.seq5 = new long[] { 1, 2, 3, 4 };
                c.seq6 = new float[] { 1, 2, 3, 4 };
                c.seq7 = new double[] { 1, 2, 3, 4 };
                c.seq8 = new string[] { "string1", "string2", "string3", "string4" };
                c.seq9 = new MyEnum[] { MyEnum.enum3, MyEnum.enum2, MyEnum.enum1 };
                c.seq10 = new MyClass[4]; // null elements.
                c.d = new Dictionary<string, MyClass>();
                c.d.Add("hi", c);

                ex.c = c;

                ostr.WriteException(ex);
                ostr.EndEncapsulation();
                var data = ostr.ToArray();

                istr = new InputStream(communicator, data);
                istr.StartEncapsulation();
                try
                {
                    istr.ThrowException();
                    test(false);
                }
                catch (MyException ex1)
                {
                    test(ex1.c.s.e == c.s.e);
                    test(Compare(ex1.c.seq1, c.seq1));
                    test(Compare(ex1.c.seq2, c.seq2));
                    test(Compare(ex1.c.seq3, c.seq3));
                    test(Compare(ex1.c.seq4, c.seq4));
                    test(Compare(ex1.c.seq5, c.seq5));
                    test(Compare(ex1.c.seq6, c.seq6));
                    test(Compare(ex1.c.seq7, c.seq7));
                    test(Compare(ex1.c.seq8, c.seq8));
                    test(Compare(ex1.c.seq9, c.seq9));
                }
                catch (RemoteException)
                {
                    test(false);
                }
                istr.EndEncapsulation();
            }

            {
                var dict = new Dictionary<byte, bool>();
                dict.Add(4, true);
                dict.Add(1, false);
                ostr = new OutputStream(communicator);
                ostr.Write(dict);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var dict2 = istr.ReadByteBoolD();
                test(Collections.Equals(dict2, dict));
            }

            {
                var dict = new Dictionary<short, int>();
                dict.Add(1, 9);
                dict.Add(4, 8);
                ostr = new OutputStream(communicator);
                ostr.Write(dict);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var dict2 = istr.ReadShortIntD();
                test(Collections.Equals(dict2, dict));
            }

            {
                var dict = new Dictionary<long, float>();
                dict.Add(123809828, 0.51f);
                dict.Add(123809829, 0.56f);
                ostr = new OutputStream(communicator);
                ostr.Write(dict);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var dict2 = istr.ReadLongFloatD();
                test(Collections.Equals(dict2, dict));
            }

            {
                var dict = new Dictionary<string, string>();
                dict.Add("key1", "value1");
                dict.Add("key2", "value2");
                ostr = new OutputStream(communicator);
                StringStringDHelper.Write(ostr, dict);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var dict2 = istr.ReadStringStringD();
                test(Collections.Equals(dict2, dict));
            }

            {
                var dict = new Dictionary<string, MyClass>();
                var c = new MyClass();
                c.s = new SmallStruct(false, 0, 0, 0, 0, 0.0f, 0.0, "", MyEnum.enum1, null);
                c.s.e = MyEnum.enum2;
                dict.Add("key1", c);
                c = new MyClass();
                c.s = new SmallStruct(false, 0, 0, 0, 0, 0.0f, 0.0, "", MyEnum.enum1, null);
                c.s.e = MyEnum.enum3;
                dict.Add("key2", c);
                ostr = new OutputStream(communicator);
                ostr.StartEncapsulation();
                ostr.Write(dict);
                ostr.EndEncapsulation();
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                istr.StartEncapsulation();
                var dict2 = istr.ReadStringMyClassD();
                istr.EndEncapsulation();
                test(dict2.Count == dict.Count);
                test(dict2["key1"].s.e == MyEnum.enum2);
                test(dict2["key2"].s.e == MyEnum.enum3);
            }

            {
                bool[] arr = { true, false, true, false };
                ostr = new OutputStream(communicator);
                var l = new List<bool>(arr);
                ostr.StartEncapsulation();
                ostr.WriteBoolSeq(l);
                ostr.EndEncapsulation();
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                istr.StartEncapsulation();
                var l2 = new List<bool>(istr.ReadBoolArray());
                istr.EndEncapsulation();
                test(Compare(l, l2));
            }

            {
                byte[] arr = { 0x01, 0x11, 0x12, 0x22 };
                ostr = new OutputStream(communicator);
                var l = new List<byte>(arr);
                ostr.WriteByteSeq(l);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var l2 = new List<byte>(istr.ReadByteArray());
                test(Compare(l2, l));
            }

            {
                MyEnum[] arr = { MyEnum.enum3, MyEnum.enum2, MyEnum.enum1, MyEnum.enum2 };
                ostr = new OutputStream(communicator);
                var l = new List<MyEnum>(arr);
                ostr.Write(l);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var l2 = istr.ReadMyEnumList();
                test(Compare(l2, l));
            }

            {
                ostr = new OutputStream(communicator);
                var l = new List<SmallStruct>(smallStructArray);
                ostr.Write(l);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var l2 = istr.ReadSmallStructList();
                test(l2.Count == l.Count);
                for (int i = 0; i < l2.Count; ++i)
                {
                    test(l2[i].Equals(smallStructArray[i]));
                }
            }

            {
                ostr = new OutputStream(communicator);
                ostr.StartEncapsulation();
                var l = new List<MyClass>(myClassArray);
                ostr.Write(l);
                ostr.EndEncapsulation();
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                istr.StartEncapsulation();
                var l2 = istr.ReadMyClassList();
                istr.EndEncapsulation();
                test(l2.Count == l.Count);
                for (int i = 0; i < l2.Count; ++i)
                {
                    test(l2[i] != null);
                    test(l2[i].c == l2[i]);
                    test(l2[i].o == l2[i]);
                    test(l2[i].s.e == MyEnum.enum2);
                    test(Compare(l2[i].seq1, l[i].seq1));
                    test(Compare(l2[i].seq2, l[i].seq2));
                    test(Compare(l2[i].seq3, l[i].seq3));
                    test(Compare(l2[i].seq4, l[i].seq4));
                    test(Compare(l2[i].seq5, l[i].seq5));
                    test(Compare(l2[i].seq6, l[i].seq6));
                    test(Compare(l2[i].seq7, l[i].seq7));
                    test(Compare(l2[i].seq8, l[i].seq8));
                    test(Compare(l2[i].seq9, l[i].seq9));
                    test(l2[i].d["hi"].Equals(l2[i]));
                }
            }

            {
                var arr = new IObjectPrx[2];
                arr[0] = IObjectPrx.Parse("zero", communicator);
                arr[1] = IObjectPrx.Parse("one", communicator);
                ostr = new OutputStream(communicator);
                var l = new List<IObjectPrx>(arr);
                ostr.Write(arr);
                byte[] data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var l2 = new List<IObjectPrx>(istr.ReadObjectProxySeq());
                test(Compare(l2, l));
            }

            {
                var arr = new IMyInterfacePrx[2];
                arr[0] = IMyInterfacePrx.Parse("zero", communicator);
                arr[1] = IMyInterfacePrx.Parse("one", communicator);
                ostr = new OutputStream(communicator);
                var l = new List<IMyInterfacePrx>(arr);
                ostr.WriteProxySeq(l);
                byte[] data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var l2 = istr.ReadMyInterfaceProxyList();
                test(Compare(l2, l));
            }

            {
                short[] arr = { 0x01, 0x11, 0x12, 0x22 };
                ostr = new OutputStream(communicator);
                var l = new LinkedList<short>(arr);
                ostr.WriteShortSeq(l);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var l2 = new LinkedList<short>(istr.ReadShortArray());
                test(Compare(l2, l));
            }

            {
                int[] arr = { 0x01, 0x11, 0x12, 0x22 };
                ostr = new OutputStream(communicator);
                var l = new LinkedList<int>(arr);
                ostr.WriteIntSeq(l);
                byte[] data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var l2 = new LinkedList<int>(istr.ReadIntArray());
                test(Compare(l2, l));
            }

            {
                MyEnum[] arr = { MyEnum.enum3, MyEnum.enum2, MyEnum.enum1, MyEnum.enum2 };
                ostr = new OutputStream(communicator);
                var l = new LinkedList<MyEnum>(arr);
                ostr.Write(l);
                byte[] data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var l2 = istr.ReadMyEnumLinkedList();
                test(Compare(l2, l));
            }

            {
                ostr = new OutputStream(communicator);
                var l = new LinkedList<SmallStruct>(smallStructArray);
                ostr.Write(l);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var l2 = istr.ReadSmallStructLinkedList();
                test(l2.Count == l.Count);
                var e = l.GetEnumerator();
                var e2 = l2.GetEnumerator();
                while (e.MoveNext() && e2.MoveNext())
                {
                    test(e.Current.Equals(e2.Current));
                }
            }

            {
                long[] arr = { 0x01, 0x11, 0x12, 0x22 };
                ostr = new OutputStream(communicator);
                var l = new Stack<long>(arr);
                ostr.WriteLongSeq(l);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var l2 = new Stack<long>(istr.ReadLongArray().Reverse());
                test(Compare(l2, l));
            }

            {
                float[] arr = { 1, 2, 3, 4 };
                ostr = new OutputStream(communicator);
                var l = new Stack<float>(arr);
                ostr.WriteFloatSeq(l);
                byte[] data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var l2 = new Stack<float>(istr.ReadFloatArray().Reverse());
                test(Compare(l2, l));
            }

            {
                ostr = new OutputStream(communicator);
                var l = new Stack<SmallStruct>(smallStructArray);
                ostr.Write(l);
                byte[] data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var l2 = istr.ReadSmallStructStack();
                test(l2.Count == l.Count);
                var e = l.GetEnumerator();
                var e2 = l2.GetEnumerator();
                while (e.MoveNext() && e2.MoveNext())
                {
                    test(e.Current.Equals(e2.Current));
                }
            }

            {
                var arr = new IMyInterfacePrx[2];
                arr[0] = IMyInterfacePrx.Parse("zero", communicator);
                arr[1] = IMyInterfacePrx.Parse("one", communicator);
                ostr = new OutputStream(communicator);
                var l = new Stack<IMyInterfacePrx>(arr);
                ostr.WriteProxySeq(l);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var l2 = istr.ReadMyInterfaceProxyStack();
                test(Compare(l2, l));
            }

            {
                double[] arr = { 1, 2, 3, 4 };
                ostr = new OutputStream(communicator);
                var l = new Queue<double>(arr);
                ostr.WriteDoubleSeq(l);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var l2 = new Queue<double>(istr.ReadDoubleArray());
                test(Compare(l2, l));
            }

            {
                string[] arr = { "string1", "string2", "string3", "string4" };
                ostr = new OutputStream(communicator);
                var l = new Queue<string>(arr);
                ostr.WriteStringSeq(l);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var l2 = new Queue<string>(istr.ReadStringCollection());
                test(Compare(l2, l));
            }

            {
                ostr = new OutputStream(communicator);
                var l = new Queue<SmallStruct>(smallStructArray);
                ostr.Write(l);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var l2 = istr.ReadSmallStructQueue();
                test(l2.Count == l.Count);
                var e = l.GetEnumerator();
                var e2 = l2.GetEnumerator();
                while (e.MoveNext() && e2.MoveNext())
                {
                    test(e.Current.Equals(e2.Current));
                }
            }

            {
                string[] arr = { "string1", "string2", "string3", "string4" };
                string[][] arrS = { arr, new string[0], arr };
                ostr = new OutputStream(communicator);
                var l = new List<string[]>(arrS);
                ostr.Write(l);
                byte[] data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var l2 = istr.ReadStringSList();
                test(Compare(l2, l));
            }

            {
                string[] arr = { "string1", "string2", "string3", "string4" };
                string[][] arrS = { arr, new string[0], arr };
                ostr = new OutputStream(communicator);
                var l = new Stack<string[]>(arrS);
                ostr.Write(l);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var l2 = istr.ReadStringSStack();
                test(Compare(l2, l));
            }

            {
                var dict = new SortedDictionary<string, string>();
                dict.Add("key1", "value1");
                dict.Add("key2", "value2");
                ostr = new OutputStream(communicator);
                Ice.ContextHelper.Write(ostr, dict);
                var data = ostr.ToArray();
                istr = new InputStream(communicator, data);
                var dict2 = istr.ReadSortedStringStringD();
                test(Collections.Equals(dict2, dict));
            }*/

            output.WriteLine("ok");
            return 0;
        }
    }
}
