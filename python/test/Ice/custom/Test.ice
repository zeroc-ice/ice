// Copyright (c) ZeroC, Inc.

#pragma once

["python:identifier:generated.test.Ice.custom.Test"]
module Test
{
    sequence<byte> ByteString; /* By default, sequence<byte> is received as a bytes literal. */
    ["python:list"] sequence<byte> ByteList;

    sequence<string> StringList; /* By default, a sequence is received as a list. */
    ["python:tuple"] sequence<string> StringTuple;

    sequence<int> IntList; /* By default, sequence<int> is received as a list. */

    ["python:array.array"] sequence<bool> BoolSeq1;
    ["python:memoryview:CustomFactory.myBoolSeq:array.array"] sequence<bool> BoolSeq2;

    ["python:array.array"] sequence<byte> ByteSeq1;
    ["python:memoryview:CustomFactory.myByteSeq:array.array"] sequence<byte> ByteSeq2;

    ["python:array.array"] sequence<short> ShortSeq1;
    ["python:memoryview:CustomFactory.myShortSeq:array.array"] sequence<short> ShortSeq2;

    ["python:array.array"] sequence<int> IntSeq1;
    ["python:memoryview:CustomFactory.myIntSeq:array.array"] sequence<int> IntSeq2;

    ["python:array.array"] sequence<long> LongSeq1;
    ["python:memoryview:CustomFactory.myLongSeq:array.array"] sequence<long> LongSeq2;

    ["python:array.array"] sequence<float> FloatSeq1;
    ["python:memoryview:CustomFactory.myFloatSeq:array.array"] sequence<float> FloatSeq2;

    ["python:array.array"] sequence<double> DoubleSeq1;
    ["python:memoryview:CustomFactory.myDoubleSeq:array.array"] sequence<double> DoubleSeq2;

    struct S
    {
        ByteString b1;
        ["python:list"] ByteString b2;
        ByteList b4;
        StringList s1;
        ["python:tuple"] StringList s2;
        StringTuple s3;

        ["python:array.array"] ByteString a1;
    }

    class C
    {
        ByteString b1;
        ["python:list"] ByteString b2;
        ByteList b4;
        StringList s1;
        ["python:tuple"] StringList s2;
        StringTuple s3;

        ["python:array.array"] ByteString a1;
    }

    class D
    {
        optional(1) BoolSeq1 boolSeq;
        optional(2) ByteSeq1 byteSeq;
        optional(3) ShortSeq1 shortSeq;
        optional(4) IntSeq1 intSeq;
        optional(5) LongSeq1 longSeq;
        optional(6) FloatSeq1 floatSeq;
        optional(7) DoubleSeq1 doubleSeq;
    }

    class E
    {
        BoolSeq1 boolSeq;
        ByteSeq1 byteSeq;
        ShortSeq1 shortSeq;
        IntSeq1 intSeq;
        LongSeq1 longSeq;
        FloatSeq1 floatSeq;
        DoubleSeq1 doubleSeq;
    }

    // Test using memoryview factory function in a Python datastruct
    struct M
    {
        ["python:memoryview:CustomFactory.myBoolSeq:array.array"] BoolSeq1 boolSeq;
    }

    interface Custom
    {
        ByteString opByteString1(ByteString b1, out ByteString b2);
        ["python:tuple"] ByteString opByteString2(["python:list"] ByteString b1,
            ["python:list"] out ByteString b2);

        ByteList opByteList1(ByteList b1, out ByteList b2);
        ["python:list"] ByteList opByteList2(["python:tuple"] ByteList b1,
            ["python:tuple"] out ByteList b2);

        StringList opStringList1(StringList s1, out StringList s2);
        ["python:tuple"] StringList opStringList2(["python:tuple"] StringList s1,
            ["python:tuple"] out StringList s2);

        StringTuple opStringTuple1(StringTuple s1, out StringTuple s2);
        ["python:list"] StringTuple opStringTuple2(["python:list"] StringTuple s1,
            ["python:list"] out StringTuple s2);

        void sendS(S val);
        void sendC(C val);

        BoolSeq1 opBoolSeq(BoolSeq1 v1, out BoolSeq2 v2);
        ByteSeq1 opByteSeq(ByteSeq1 v1, out ByteSeq2 v2);
        ShortSeq1 opShortSeq(ShortSeq1 v1, out ShortSeq2 v2);
        IntSeq1 opIntSeq(IntSeq1 v1, out IntSeq2 v2);
        LongSeq1 opLongSeq(LongSeq1 v1, out LongSeq2 v2);
        FloatSeq1 opFloatSeq(FloatSeq1 v1, out FloatSeq2 v2);
        DoubleSeq1 opDoubleSeq(DoubleSeq1 v1, out DoubleSeq2 v2);

        ["python:memoryview:CustomFactory.myBogusArrayNotExistsFactory:array.array"] BoolSeq1 opBogusArrayNotExistsFactory();
        ["python:memoryview:CustomFactory.myBogusArrayThrowFactory:array.array"] BoolSeq1 opBogusArrayThrowFactory();
        ["python:memoryview:CustomFactory.myBogusArrayType:array.array"] BoolSeq1 opBogusArrayType();
        ["python:memoryview:CustomFactory.myBogusArrayNoneFactory:array.array"] BoolSeq1 opBogusArrayNoneFactory();
        ["python:memoryview:CustomFactory.myBogusArraySignatureFactory:array.array"] BoolSeq1 opBogusArraySignatureFactory();
        ["python:memoryview:CustomFactory.myNoCallableFactory:array.array"] BoolSeq1 opBogusArrayNoCallableFactory();

        D opD(D d);

        M opM(M m);

        void shutdown();
    }

    dictionary<string, IntList> IntListDict;
    dictionary<string, ShortSeq1> ShortSeq1Dict;
    dictionary<string, ByteSeq2> ByteSeq2Dict;
    sequence<IntList> IntListSeq;
    sequence<ShortSeq1> ShortSeq1Seq;
    sequence<IntListDict> IntListDictSeq;

    struct NestedS
    {
        ShortSeq1Dict d;
        ShortSeq1Seq s;
        ByteSeq2Dict m;
        IntListDict p;
    }

    class NestedC
    {
        ShortSeq1Dict d;
        ShortSeq1Seq s;
    }

    // These operations use numeric and custom mapped sequences nested inside dictionaries.
    // This tests that the import visitor recurses through dictionary value types.
    // For this test to be meaningful, numeric and custom mapped sequences must not appear directly in the operation's
    // signature; this would cause the import to be registered even without recursion.
    interface NestedDictCustom
    {
        IntListDict opIntListDict(IntListDict v1, out IntListDict v2);
        ShortSeq1Dict opShortSeq1Dict(ShortSeq1Dict v1, out ShortSeq1Dict v2);
        ByteSeq2Dict opByteSeq2Dict(ByteSeq2Dict v1);

        NestedS opNestedS(NestedS v);
        NestedC opNestedC(NestedC v);
    }

    // These operations use numeric and custom mapped sequences nested inside other sequences.
    // This tests that the import visitor recurses through sequence element types.
    // For this test to be meaningful, numeric and custom mapped sequences must not appear directly in the operation's
    // signature; this would cause the import to be registered even without recursion.
    interface NestedSeqCustom
    {
        IntListSeq opIntListSeq(IntListSeq v1, out IntListSeq v2);
        ShortSeq1Seq opShortSeq1Seq(ShortSeq1Seq v1);
        IntListDictSeq opIntListDictSeq(IntListDictSeq v1);
    }
}
