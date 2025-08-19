// Copyright (c) ZeroC, Inc.

#pragma once

["python:identifier:generated.test.Ice.custom.Test"]
module Test
{
    sequence<byte> ByteString; /* By default, sequence<byte> is received as a bytes literal. */
    ["python:list"] sequence<byte> ByteList;

    sequence<string> StringList; /* By default, a sequence is received as a list. */
    ["python:tuple"] sequence<string> StringTuple;

    ["python:array.array"] sequence<bool> BoolSeq1;
    ["python:memoryview:Custom.myBoolSeq:array.array"] sequence<bool> BoolSeq2;

    ["python:array.array"] sequence<byte> ByteSeq1;
    ["python:memoryview:Custom.myByteSeq:array.array"] sequence<byte> ByteSeq2;

    ["python:array.array"] sequence<short> ShortSeq1;
    ["python:memoryview:Custom.myShortSeq:array.array"] sequence<short> ShortSeq2;

    ["python:array.array"] sequence<int> IntSeq1;
    ["python:memoryview:Custom.myIntSeq:array.array"] sequence<int> IntSeq2;

    ["python:array.array"] sequence<long> LongSeq1;
    ["python:memoryview:Custom.myLongSeq:array.array"] sequence<long> LongSeq2;

    ["python:array.array"] sequence<float> FloatSeq1;
    ["python:memoryview:Custom.myFloatSeq:array.array"] sequence<float> FloatSeq2;

    ["python:array.array"] sequence<double> DoubleSeq1;
    ["python:memoryview:Custom.myDoubleSeq:array.array"] sequence<double> DoubleSeq2;

    struct S
    {
        ByteString b1;
        ["python:list"] ByteString b2;
        ByteList b4;
        StringList s1;
        ["python:tuple"] StringList s2;
        StringTuple s3;
    }

    class C
    {
        ByteString b1;
        ["python:list"] ByteString b2;
        ByteList b4;
        StringList s1;
        ["python:tuple"] StringList s2;
        StringTuple s3;
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

        ["python:memoryview:Custom.myBogusArrayNotExistsFactory:array.array"] BoolSeq1 opBogusArrayNotExistsFactory();
        ["python:memoryview:Custom.myBogusArrayThrowFactory:array.array"] BoolSeq1 opBogusArrayThrowFactory();
        ["python:memoryview:Custom.myBogusArrayType:array.array"] BoolSeq1 opBogusArrayType();
        ["python:memoryview:Custom.myBogusArrayNoneFactory:array.array"] BoolSeq1 opBogusArrayNoneFactory();
        ["python:memoryview:Custom.myBogusArraySignatureFactory:array.array"] BoolSeq1 opBogusArraySignatureFactory();
        ["python:memoryview:Custom.myNoCallableFactory:array.array"] BoolSeq1 opBogusArrayNoCallableFactory();

        D opD(D d);

        void shutdown();
    }

    struct M
    {
        ["python:memoryview:Custom.myBoolSeq:array.array"] BoolSeq2 boolSeq;
    }
}
