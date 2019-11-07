//
// Copyright(c) ZeroC, Inc. All rights reserved.
//

using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;

namespace Ice
{
    namespace stream
    {
        public class AllTests : global::Test.AllTests
        {
            //
            // There does not appear to be any way to compare collections
            // in either C# or with the .NET framework. Something like
            // C++ STL EqualRange would be nice...
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
                    if (i1.Current is ICollection)
                    {
                        Debug.Assert(i2.Current is ICollection);
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

            private class TestValueWriter : Ice.ValueWriter
            {
                public TestValueWriter(Test.MyClass obj)
                {
                    this.obj = obj;
                }

                public override void write(Ice.OutputStream outS)
                {
                    obj.iceWrite(outS);
                    called = true;
                }

                internal Test.MyClass obj;
                internal bool called = false;
            }

            private class TestReadValueCallback
            {
                public void invoke(Ice.Value obj)
                {
                    this.obj = obj;
                }

                internal Ice.Value obj;
            }

            static public int allTests(global::Test.TestHelper helper)
            {
                var communicator = helper.communicator();
                Ice.InputStream inS;
                Ice.OutputStream outS;

                var output = helper.getWriter();
                output.Write("testing primitive types... ");
                output.Flush();

                {
                    byte[] data = new byte[0];
                    inS = new Ice.InputStream(communicator, data);
                }

                {
                    outS = new Ice.OutputStream(communicator);
                    outS.startEncapsulation();
                    outS.writeBool(true);
                    outS.endEncapsulation();
                    var data = outS.finished();

                    inS = new Ice.InputStream(communicator, data);
                    inS.startEncapsulation();
                    test(inS.readBool());
                    inS.endEncapsulation();

                    inS = new Ice.InputStream(communicator, data);
                    inS.startEncapsulation();
                    test(inS.readBool());
                    inS.endEncapsulation();
                }

                {
                    var data = new byte[0];
                    inS = new Ice.InputStream(communicator, data);
                    try
                    {
                        inS.readBool();
                        test(false);
                    }
                    catch (Ice.UnmarshalOutOfBoundsException)
                    {
                    }
                }

                {
                    outS = new Ice.OutputStream(communicator);
                    outS.writeBool(true);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    test(inS.readBool());
                }

                {
                    outS = new Ice.OutputStream(communicator);
                    outS.writeByte(1);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    test(inS.readByte() == 1);
                }

                {
                    outS = new Ice.OutputStream(communicator);
                    outS.writeShort(2);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    test(inS.readShort() == 2);
                }

                {
                    outS = new Ice.OutputStream(communicator);
                    outS.writeInt(3);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    test(inS.readInt() == 3);
                }

                {
                    outS = new Ice.OutputStream(communicator);
                    outS.writeLong(4);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    test(inS.readLong() == 4);
                }

                {
                    outS = new Ice.OutputStream(communicator);
                    outS.writeFloat((float)5.0);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    test(inS.readFloat() == (float)5.0);
                }

                {
                    outS = new Ice.OutputStream(communicator);
                    outS.writeDouble(6.0);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    test(inS.readDouble() == 6.0);
                }

                {
                    outS = new Ice.OutputStream(communicator);
                    outS.writeString("hello world");
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    test(inS.readString().Equals("hello world"));
                }

                output.WriteLine("ok");

                output.Write("testing constructed types... ");
                output.Flush();

                {
                    outS = new Ice.OutputStream(communicator);
                    Test.MyEnumHelper.write(outS, Test.MyEnum.enum3);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var e = Test.MyEnumHelper.read(inS);
                    test(e == Test.MyEnum.enum3);
                }

                {
                    outS = new Ice.OutputStream(communicator);
                    var s = new Test.SmallStruct();
                    s.bo = true;
                    s.by = 1;
                    s.sh = 2;
                    s.i = 3;
                    s.l = 4;
                    s.f = 5.0f;
                    s.d = 6.0;
                    s.str = "7";
                    s.e = Test.MyEnum.enum2;
                    s.p = Test.MyInterfacePrxHelper.uncheckedCast(communicator.stringToProxy("test:default"));
                    Test.SmallStruct.ice_write(outS, s);
                    var data = outS.finished();
                    var s2 = Test.SmallStruct.ice_read(new Ice.InputStream(communicator, data));
                    test(s2.Equals(s));
                }

                {
                    outS = new Ice.OutputStream(communicator);
                    var o = new Test.OptionalClass();
                    o.bo = true;
                    o.by = 5;
                    o.sh = 4;
                    o.i = 3;
                    outS.writeValue(o);
                    outS.writePendingValues();
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    TestReadValueCallback cb = new TestReadValueCallback();
                    inS.readValue(cb.invoke);
                    inS.readPendingValues();
                    var o2 = (Test.OptionalClass)cb.obj;
                    test(o2.bo == o.bo);
                    test(o2.by == o.by);
                    if (communicator.getProperties().getProperty("Ice.Default.EncodingVersion").Equals("1.0"))
                    {
                        test(!o2.sh.HasValue);
                        test(!o2.i.HasValue);
                    }
                    else
                    {
                        test(o2.sh.Value == o.sh.Value);
                        test(o2.i.Value == o.i.Value);
                    }
                }

                {
                    outS = new Ice.OutputStream(communicator, Ice.Util.Encoding_1_0);
                    var o = new Test.OptionalClass();
                    o.bo = true;
                    o.by = 5;
                    o.sh = 4;
                    o.i = 3;
                    outS.writeValue(o);
                    outS.writePendingValues();
                    byte[] data = outS.finished();
                    inS = new Ice.InputStream(communicator, Ice.Util.Encoding_1_0, data);
                    var cb = new TestReadValueCallback();
                    inS.readValue(cb.invoke);
                    inS.readPendingValues();
                    var o2 = (Test.OptionalClass)cb.obj;
                    test(o2.bo == o.bo);
                    test(o2.by == o.by);
                    test(!o2.sh.HasValue);
                    test(!o2.i.HasValue);
                }

                {
                    bool[] arr = { true, false, true, false };
                    outS = new Ice.OutputStream(communicator);
                    Ice.BoolSeqHelper.write(outS, arr);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var arr2 = Ice.BoolSeqHelper.read(inS);
                    test(Compare(arr2, arr));

                    bool[][] arrS = { arr, new bool[0], arr };
                    outS = new Ice.OutputStream(communicator);
                    Test.BoolSSHelper.write(outS, arrS);
                    data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var arr2S = Test.BoolSSHelper.read(inS);
                    test(Compare(arr2S, arrS));
                }

                {
                    byte[] arr = { 0x01, 0x11, 0x12, 0x22 };
                    outS = new Ice.OutputStream(communicator);
                    Ice.ByteSeqHelper.write(outS, arr);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var arr2 = Ice.ByteSeqHelper.read(inS);
                    test(Compare(arr2, arr));

                    byte[][] arrS = { arr, new byte[0], arr };
                    outS = new Ice.OutputStream(communicator);
                    Test.ByteSSHelper.write(outS, arrS);
                    data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var arr2S = Test.ByteSSHelper.read(inS);
                    test(Compare(arr2S, arrS));
                }

                {
                    Serialize.Small small = new Serialize.Small();
                    small.i = 99;
                    outS = new Ice.OutputStream(communicator);
                    outS.writeSerializable(small);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var small2 = (Serialize.Small)inS.readSerializable();
                    test(small2.i == 99);
                }

                {
                    short[] arr = { 0x01, 0x11, 0x12, 0x22 };
                    outS = new Ice.OutputStream(communicator);
                    Ice.ShortSeqHelper.write(outS, arr);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var arr2 = Ice.ShortSeqHelper.read(inS);
                    test(Compare(arr2, arr));

                    short[][] arrS = { arr, new short[0], arr };
                    outS = new Ice.OutputStream(communicator);
                    Test.ShortSSHelper.write(outS, arrS);
                    data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var arr2S = Test.ShortSSHelper.read(inS);
                    test(Compare(arr2S, arrS));
                }

                {
                    int[] arr = { 0x01, 0x11, 0x12, 0x22 };
                    outS = new Ice.OutputStream(communicator);
                    Ice.IntSeqHelper.write(outS, arr);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var arr2 = Ice.IntSeqHelper.read(inS);
                    test(Compare(arr2, arr));

                    int[][] arrS = { arr, new int[0], arr };
                    outS = new Ice.OutputStream(communicator);
                    Test.IntSSHelper.write(outS, arrS);
                    data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var arr2S = Test.IntSSHelper.read(inS);
                    test(Compare(arr2S, arrS));
                }

                {
                    long[] arr = { 0x01, 0x11, 0x12, 0x22 };
                    outS = new Ice.OutputStream(communicator);
                    Ice.LongSeqHelper.write(outS, arr);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var arr2 = Ice.LongSeqHelper.read(inS);
                    test(Compare(arr2, arr));

                    long[][] arrS = { arr, new long[0], arr };
                    outS = new Ice.OutputStream(communicator);
                    Test.LongSSHelper.write(outS, arrS);
                    data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var arr2S = Test.LongSSHelper.read(inS);
                    test(Compare(arr2S, arrS));
                }

                {
                    float[] arr = { 1, 2, 3, 4 };
                    outS = new Ice.OutputStream(communicator);
                    Ice.FloatSeqHelper.write(outS, arr);
                    byte[] data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    float[] arr2 = Ice.FloatSeqHelper.read(inS);
                    test(Compare(arr2, arr));

                    float[][] arrS = { arr, new float[0], arr };
                    outS = new Ice.OutputStream(communicator);
                    Test.FloatSSHelper.write(outS, arrS);
                    data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var arr2S = Test.FloatSSHelper.read(inS);
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
                    outS = new Ice.OutputStream(communicator);
                    Ice.DoubleSeqHelper.write(outS, arr);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var arr2 = Ice.DoubleSeqHelper.read(inS);
                    test(Compare(arr2, arr));

                    double[][] arrS = { arr, new double[0], arr };
                    outS = new Ice.OutputStream(communicator);
                    Test.DoubleSSHelper.write(outS, arrS);
                    data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var arr2S = Test.DoubleSSHelper.read(inS);
                    test(Compare(arr2S, arrS));
                }

                {
                    string[] arr = { "string1", "string2", "string3", "string4" };
                    outS = new Ice.OutputStream(communicator);
                    Ice.StringSeqHelper.write(outS, arr);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var arr2 = Ice.StringSeqHelper.read(inS);
                    test(Compare(arr2, arr));

                    string[][] arrS = { arr, new string[0], arr };
                    outS = new Ice.OutputStream(communicator);
                    Test.StringSSHelper.write(outS, arrS);
                    data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var arr2S = Test.StringSSHelper.read(inS);
                    test(Compare(arr2S, arrS));
                }

                {
                    Test.MyEnum[] arr = {
                            Test.MyEnum.enum3,
                            Test.MyEnum.enum2,
                            Test.MyEnum.enum1,
                            Test.MyEnum.enum2
                        };
                    outS = new Ice.OutputStream(communicator);
                    Test.MyEnumSHelper.write(outS, arr);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var arr2 = Test.MyEnumSHelper.read(inS);
                    test(Compare(arr2, arr));

                    Test.MyEnum[][] arrS = { arr, new Test.MyEnum[0], arr };
                    outS = new Ice.OutputStream(communicator);
                    Test.MyEnumSSHelper.write(outS, arrS);
                    data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var arr2S = Test.MyEnumSSHelper.read(inS);
                    test(Compare(arr2S, arrS));
                }

                var smallStructArray = new Test.SmallStruct[3];
                for (int i = 0; i < smallStructArray.Length; ++i)
                {
                    smallStructArray[i] = new Test.SmallStruct();
                    smallStructArray[i].bo = true;
                    smallStructArray[i].by = 1;
                    smallStructArray[i].sh = 2;
                    smallStructArray[i].i = 3;
                    smallStructArray[i].l = 4;
                    smallStructArray[i].f = 5.0f;
                    smallStructArray[i].d = 6.0;
                    smallStructArray[i].str = "7";
                    smallStructArray[i].e = Test.MyEnum.enum2;
                    smallStructArray[i].p = Test.MyInterfacePrxHelper.uncheckedCast(communicator.stringToProxy("test:default"));
                }

                var myClassArray = new Test.MyClass[4];
                for (int i = 0; i < myClassArray.Length; ++i)
                {
                    myClassArray[i] = new Test.MyClass();
                    myClassArray[i].c = myClassArray[i];
                    myClassArray[i].o = myClassArray[i];
                    myClassArray[i].s = new Test.SmallStruct();
                    myClassArray[i].s.e = Test.MyEnum.enum2;
                    myClassArray[i].seq1 = new bool[] { true, false, true, false };
                    myClassArray[i].seq2 = new byte[] { 1, 2, 3, 4 };
                    myClassArray[i].seq3 = new short[] { 1, 2, 3, 4 };
                    myClassArray[i].seq4 = new int[] { 1, 2, 3, 4 };
                    myClassArray[i].seq5 = new long[] { 1, 2, 3, 4 };
                    myClassArray[i].seq6 = new float[] { 1, 2, 3, 4 };
                    myClassArray[i].seq7 = new double[] { 1, 2, 3, 4 };
                    myClassArray[i].seq8 = new string[] { "string1", "string2", "string3", "string4" };
                    myClassArray[i].seq9 = new Test.MyEnum[] { Test.MyEnum.enum3, Test.MyEnum.enum2, Test.MyEnum.enum1 };
                    myClassArray[i].seq10 = new Test.MyClass[4]; // null elements.
                    myClassArray[i].d = new Dictionary<string, Test.MyClass>();
                    myClassArray[i].d["hi"] = myClassArray[i];
                }

                {
                    outS = new Ice.OutputStream(communicator);
                    Test.MyClassSHelper.write(outS, myClassArray);
                    outS.writePendingValues();
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var arr2 = Test.MyClassSHelper.read(inS);
                    inS.readPendingValues();
                    test(arr2.Length == myClassArray.Length);
                    for (int i = 0; i < arr2.Length; ++i)
                    {
                        test(arr2[i] != null);
                        test(arr2[i].c == arr2[i]);
                        test(arr2[i].o == arr2[i]);
                        test(arr2[i].s.e == Test.MyEnum.enum2);
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

                    Test.MyClass[][] arrS = { myClassArray, new Test.MyClass[0], myClassArray };
                    outS = new Ice.OutputStream(communicator);
                    Test.MyClassSSHelper.write(outS, arrS);
                    outS.writePendingValues();
                    data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var arr2S = Test.MyClassSSHelper.read(inS);
                    inS.readPendingValues();
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
                            test(arr2S[j][k].s.e == Test.MyEnum.enum2);
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
                    outS = new Ice.OutputStream(communicator);
                    var obj = new Test.MyClass();
                    obj.s = new Test.SmallStruct();
                    obj.s.e = Test.MyEnum.enum2;
                    var writer = new TestValueWriter(obj);
                    outS.writeValue(writer);
                    outS.writePendingValues();
                    var data = outS.finished();
                    test(writer.called);
                    inS = new Ice.InputStream(communicator, data);
                    var cb = new TestReadValueCallback();
                    inS.readValue(cb.invoke);
                    inS.readPendingValues();
                    test(cb.obj != null);
                    test(cb.obj is Test.MyClass);
                    var robj = (Test.MyClass)cb.obj;
                    test(robj != null);
                    test(robj.s.e == Test.MyEnum.enum2);
                }

                {
                    outS = new Ice.OutputStream(communicator);
                    var ex = new Test.MyException();

                    var c = new Test.MyClass();
                    c.c = c;
                    c.o = c;
                    c.s = new Test.SmallStruct();
                    c.s.e = Test.MyEnum.enum2;
                    c.seq1 = new bool[] { true, false, true, false };
                    c.seq2 = new byte[] { 1, 2, 3, 4 };
                    c.seq3 = new short[] { 1, 2, 3, 4 };
                    c.seq4 = new int[] { 1, 2, 3, 4 };
                    c.seq5 = new long[] { 1, 2, 3, 4 };
                    c.seq6 = new float[] { 1, 2, 3, 4 };
                    c.seq7 = new double[] { 1, 2, 3, 4 };
                    c.seq8 = new string[] { "string1", "string2", "string3", "string4" };
                    c.seq9 = new Test.MyEnum[] { Test.MyEnum.enum3, Test.MyEnum.enum2, Test.MyEnum.enum1 };
                    c.seq10 = new Test.MyClass[4]; // null elements.
                    c.d = new Dictionary<string, Test.MyClass>();
                    c.d.Add("hi", c);

                    ex.c = c;

                    outS.writeException(ex);
                    var data = outS.finished();

                    inS = new Ice.InputStream(communicator, data);
                    try
                    {
                        inS.throwException();
                        test(false);
                    }
                    catch (Test.MyException ex1)
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
                    catch (Ice.UserException)
                    {
                        test(false);
                    }
                }

                {
                    var dict = new Dictionary<byte, bool>();
                    dict.Add(4, true);
                    dict.Add(1, false);
                    outS = new Ice.OutputStream(communicator);
                    Test.ByteBoolDHelper.write(outS, dict);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var dict2 = Test.ByteBoolDHelper.read(inS);
                    test(Ice.CollectionComparer.Equals(dict2, dict));
                }

                {
                    var dict = new Dictionary<short, int>();
                    dict.Add(1, 9);
                    dict.Add(4, 8);
                    outS = new Ice.OutputStream(communicator);
                    Test.ShortIntDHelper.write(outS, dict);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var dict2 = Test.ShortIntDHelper.read(inS);
                    test(Ice.CollectionComparer.Equals(dict2, dict));
                }

                {
                    var dict = new Dictionary<long, float>();
                    dict.Add(123809828, 0.51f);
                    dict.Add(123809829, 0.56f);
                    outS = new Ice.OutputStream(communicator);
                    Test.LongFloatDHelper.write(outS, dict);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var dict2 = Test.LongFloatDHelper.read(inS);
                    test(Ice.CollectionComparer.Equals(dict2, dict));
                }

                {
                    var dict = new Dictionary<string, string>();
                    dict.Add("key1", "value1");
                    dict.Add("key2", "value2");
                    outS = new Ice.OutputStream(communicator);
                    Test.StringStringDHelper.write(outS, dict);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var dict2 = Test.StringStringDHelper.read(inS);
                    test(Ice.CollectionComparer.Equals(dict2, dict));
                }

                {
                    var dict = new Dictionary<string, Test.MyClass>();
                    var c = new Test.MyClass();
                    c.s = new Test.SmallStruct();
                    c.s.e = Test.MyEnum.enum2;
                    dict.Add("key1", c);
                    c = new Test.MyClass();
                    c.s = new Test.SmallStruct();
                    c.s.e = Test.MyEnum.enum3;
                    dict.Add("key2", c);
                    outS = new Ice.OutputStream(communicator);
                    Test.StringMyClassDHelper.write(outS, dict);
                    outS.writePendingValues();
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var dict2 = Test.StringMyClassDHelper.read(inS);
                    inS.readPendingValues();
                    test(dict2.Count == dict.Count);
                    test(dict2["key1"].s.e == Test.MyEnum.enum2);
                    test(dict2["key2"].s.e == Test.MyEnum.enum3);
                }

                {
                    bool[] arr = { true, false, true, false };
                    outS = new Ice.OutputStream(communicator);
                    var l = new List<bool>(arr);
                    Test.BoolListHelper.write(outS, l);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var l2 = Test.BoolListHelper.read(inS);
                    test(Compare(l, l2));
                }

                {
                    byte[] arr = { 0x01, 0x11, 0x12, 0x22 };
                    outS = new Ice.OutputStream(communicator);
                    var l = new List<byte>(arr);
                    Test.ByteListHelper.write(outS, l);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var l2 = Test.ByteListHelper.read(inS);
                    test(Compare(l2, l));
                }

                {
                    Test.MyEnum[] arr = { Test.MyEnum.enum3, Test.MyEnum.enum2, Test.MyEnum.enum1, Test.MyEnum.enum2 };
                    outS = new Ice.OutputStream(communicator);
                    var l = new List<Test.MyEnum>(arr);
                    Test.MyEnumListHelper.write(outS, l);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var l2 = Test.MyEnumListHelper.read(inS);
                    test(Compare(l2, l));
                }

                {
                    outS = new Ice.OutputStream(communicator);
                    var l = new List<Test.SmallStruct>(smallStructArray);
                    Test.SmallStructListHelper.write(outS, l);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var l2 = Test.SmallStructListHelper.read(inS);
                    test(l2.Count == l.Count);
                    for (int i = 0; i < l2.Count; ++i)
                    {
                        test(l2[i].Equals(smallStructArray[i]));
                    }
                }

                {
                    outS = new Ice.OutputStream(communicator);
                    var l = new List<Test.MyClass>(myClassArray);
                    Test.MyClassListHelper.write(outS, l);
                    outS.writePendingValues();
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var l2 = Test.MyClassListHelper.read(inS);
                    inS.readPendingValues();
                    test(l2.Count == l.Count);
                    for (int i = 0; i < l2.Count; ++i)
                    {
                        test(l2[i] != null);
                        test(l2[i].c == l2[i]);
                        test(l2[i].o == l2[i]);
                        test(l2[i].s.e == Test.MyEnum.enum2);
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
                    var arr = new Ice.ObjectPrx[2];
                    arr[0] = communicator.stringToProxy("zero");
                    arr[1] = communicator.stringToProxy("one");
                    outS = new Ice.OutputStream(communicator);
                    var l = new List<Ice.ObjectPrx>(arr);
                    Test.MyClassProxyListHelper.write(outS, l);
                    byte[] data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var l2 = Test.MyClassProxyListHelper.read(inS);
                    test(Compare(l2, l));
                }

                {
                    var arr = new Test.MyInterfacePrx[2];
                    arr[0] = Test.MyInterfacePrxHelper.uncheckedCast(communicator.stringToProxy("zero"));
                    arr[1] = Test.MyInterfacePrxHelper.uncheckedCast(communicator.stringToProxy("one"));
                    outS = new Ice.OutputStream(communicator);
                    var l = new List<Test.MyInterfacePrx>(arr);
                    Test.MyInterfaceProxyListHelper.write(outS, l);
                    byte[] data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var l2 = Test.MyInterfaceProxyListHelper.read(inS);
                    test(Compare(l2, l));
                }

                {
                    short[] arr = { 0x01, 0x11, 0x12, 0x22 };
                    outS = new Ice.OutputStream(communicator);
                    var l = new LinkedList<short>(arr);
                    Test.ShortLinkedListHelper.write(outS, l);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var l2 = Test.ShortLinkedListHelper.read(inS);
                    test(Compare(l2, l));
                }

                {
                    int[] arr = { 0x01, 0x11, 0x12, 0x22 };
                    outS = new Ice.OutputStream(communicator);
                    LinkedList<int> l = new LinkedList<int>(arr);
                    Test.IntLinkedListHelper.write(outS, l);
                    byte[] data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    LinkedList<int> l2 = Test.IntLinkedListHelper.read(inS);
                    test(Compare(l2, l));
                }

                {
                    Test.MyEnum[] arr = { Test.MyEnum.enum3, Test.MyEnum.enum2, Test.MyEnum.enum1, Test.MyEnum.enum2 };
                    outS = new Ice.OutputStream(communicator);
                    LinkedList<Test.MyEnum> l = new LinkedList<Test.MyEnum>(arr);
                    Test.MyEnumLinkedListHelper.write(outS, l);
                    byte[] data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    LinkedList<Test.MyEnum> l2 = Test.MyEnumLinkedListHelper.read(inS);
                    test(Compare(l2, l));
                }

                {
                    outS = new Ice.OutputStream(communicator);
                    var l = new LinkedList<Test.SmallStruct>(smallStructArray);
                    Test.SmallStructLinkedListHelper.write(outS, l);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var l2 = Test.SmallStructLinkedListHelper.read(inS);
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
                    outS = new Ice.OutputStream(communicator);
                    var l = new Stack<long>(arr);
                    Test.LongStackHelper.write(outS, l);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var l2 = Test.LongStackHelper.read(inS);
                    test(Compare(l2, l));
                }

                {
                    float[] arr = { 1, 2, 3, 4 };
                    outS = new Ice.OutputStream(communicator);
                    var l = new Stack<float>(arr);
                    Test.FloatStackHelper.write(outS, l);
                    byte[] data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var l2 = Test.FloatStackHelper.read(inS);
                    test(Compare(l2, l));
                }

                {
                    outS = new Ice.OutputStream(communicator);
                    var l = new Stack<Test.SmallStruct>(smallStructArray);
                    Test.SmallStructStackHelper.write(outS, l);
                    byte[] data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var l2 = Test.SmallStructStackHelper.read(inS);
                    test(l2.Count == l.Count);
                    var e = l.GetEnumerator();
                    var e2 = l2.GetEnumerator();
                    while (e.MoveNext() && e2.MoveNext())
                    {
                        test(e.Current.Equals(e2.Current));
                    }
                }

                {
                    var arr = new Ice.ObjectPrx[2];
                    arr[0] = communicator.stringToProxy("zero");
                    arr[1] = communicator.stringToProxy("one");
                    outS = new Ice.OutputStream(communicator);
                    var l = new Stack<Ice.ObjectPrx>(arr);
                    Test.MyClassProxyStackHelper.write(outS, l);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var l2 = Test.MyClassProxyStackHelper.read(inS);
                    test(Compare(l2, l));
                }

                {
                    var arr = new Test.MyInterfacePrx[2];
                    arr[0] = Test.MyInterfacePrxHelper.uncheckedCast(communicator.stringToProxy("zero"));
                    arr[1] = Test.MyInterfacePrxHelper.uncheckedCast(communicator.stringToProxy("one"));
                    outS = new Ice.OutputStream(communicator);
                    var l = new Stack<Test.MyInterfacePrx>(arr);
                    Test.MyInterfaceProxyStackHelper.write(outS, l);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var l2 = Test.MyInterfaceProxyStackHelper.read(inS);
                    test(Compare(l2, l));
                }

                {
                    double[] arr = { 1, 2, 3, 4 };
                    outS = new Ice.OutputStream(communicator);
                    var l = new Queue<double>(arr);
                    Test.DoubleQueueHelper.write(outS, l);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var l2 = Test.DoubleQueueHelper.read(inS);
                    test(Compare(l2, l));
                }

                {
                    string[] arr = { "string1", "string2", "string3", "string4" };
                    outS = new Ice.OutputStream(communicator);
                    var l = new Queue<string>(arr);
                    Test.StringQueueHelper.write(outS, l);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var l2 = Test.StringQueueHelper.read(inS);
                    test(Compare(l2, l));
                }

                {
                    outS = new Ice.OutputStream(communicator);
                    var l = new Queue<Test.SmallStruct>(smallStructArray);
                    Test.SmallStructQueueHelper.write(outS, l);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var l2 = Test.SmallStructQueueHelper.read(inS);
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
                    outS = new Ice.OutputStream(communicator);
                    var l = new List<string[]>(arrS);
                    Test.StringSListHelper.write(outS, l);
                    byte[] data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var l2 = Test.StringSListHelper.read(inS);
                    test(Compare(l2, l));
                }

                {
                    string[] arr = { "string1", "string2", "string3", "string4" };
                    string[][] arrS = { arr, new string[0], arr };
                    outS = new Ice.OutputStream(communicator);
                    var l = new Stack<string[]>(arrS);
                    Test.StringSStackHelper.write(outS, l);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var l2 = Test.StringSStackHelper.read(inS);
                    test(Compare(l2, l));
                }

                {
                    var dict = new SortedDictionary<string, string>();
                    dict.Add("key1", "value1");
                    dict.Add("key2", "value2");
                    outS = new Ice.OutputStream(communicator);
                    Test.SortedStringStringDHelper.write(outS, dict);
                    var data = outS.finished();
                    inS = new Ice.InputStream(communicator, data);
                    var dict2 = Test.SortedStringStringDHelper.read(inS);
                    test(Ice.CollectionComparer.Equals(dict2, dict));
                }

                output.WriteLine("ok");
                return 0;
            }
        }
    }
}
