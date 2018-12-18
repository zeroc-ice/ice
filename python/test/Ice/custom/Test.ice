// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

module Test
{
    sequence<byte> ByteString; /* By default, sequence<byte> is received as a string. */
    ["python:seq:list"] sequence<byte> ByteList;

    sequence<string> StringList; /* By default, a sequence is received as a list. */
    ["python:seq:tuple"] sequence<string> StringTuple;

    ["python:array.array"] sequence<bool> BoolSeq1;
    ["python:memoryview:Custom.myBoolSeq"] sequence<bool> BoolSeq2;

    ["python:array.array"] sequence<byte> ByteSeq1;
    ["python:memoryview:Custom.myByteSeq"] sequence<byte> ByteSeq2;

    ["python:array.array"] sequence<short> ShortSeq1;
    ["python:memoryview:Custom.myShortSeq"] sequence<short> ShortSeq2;

    ["python:array.array"] sequence<int> IntSeq1;
    ["python:memoryview:Custom.myIntSeq"] sequence<int> IntSeq2;

    ["python:array.array"] sequence<long> LongSeq1;
    ["python:memoryview:Custom.myLongSeq"] sequence<long> LongSeq2;

    ["python:array.array"] sequence<float> FloatSeq1;
    ["python:memoryview:Custom.myFloatSeq"] sequence<float> FloatSeq2;

    ["python:array.array"] sequence<double> DoubleSeq1;
    ["python:memoryview:Custom.myDoubleSeq"] sequence<double> DoubleSeq2;

    struct S
    {
        ByteString b1;
        ["python:seq:list"] ByteString b2;
        ["python:seq:default"] ByteList b3;
        ByteList b4;
        StringList s1;
        ["python:seq:tuple"] StringList s2;
        StringTuple s3;
        ["python:seq:default"] StringTuple s4;
    }

    class C
    {
        ByteString b1;
        ["python:seq:list"] ByteString b2;
        ["python:seq:default"] ByteList b3;
        ByteList b4;
        StringList s1;
        ["python:seq:tuple"] StringList s2;
        StringTuple s3;
        ["python:seq:default"] StringTuple s4;
    }

    interface Custom
    {
        ByteString opByteString1(ByteString b1, out ByteString b2);
        ["python:seq:tuple"] ByteString opByteString2(["python:seq:list"] ByteString b1,
                                                      out ["python:seq:list"] ByteString b2);

        ByteList opByteList1(ByteList b1, out ByteList b2);
        ["python:seq:default"] ByteList opByteList2(["python:seq:tuple"] ByteList b1,
                                                    out ["python:seq:tuple"] ByteList b2);

        StringList opStringList1(StringList s1, out StringList s2);
        ["python:seq:tuple"] StringList opStringList2(["python:seq:tuple"] StringList s1,
                                                      out ["python:seq:tuple"] StringList s2);

        StringTuple opStringTuple1(StringTuple s1, out StringTuple s2);
        ["python:seq:list"] StringTuple opStringTuple2(["python:seq:list"] StringTuple s1,
                                                        out ["python:seq:default"] StringTuple s2);

        void sendS(S val);
        void sendC(C val);

        BoolSeq1 opBoolSeq(BoolSeq1 v1, out BoolSeq2 v2);
        ByteSeq1 opByteSeq(ByteSeq1 v1, out ByteSeq2 v2);
        ShortSeq1 opShortSeq(ShortSeq1 v1, out ShortSeq2 v2);
        IntSeq1 opIntSeq(IntSeq1 v1, out IntSeq2 v2);
        LongSeq1 opLongSeq(LongSeq1 v1, out LongSeq2 v2);
        FloatSeq1 opFloatSeq(FloatSeq1 v1, out FloatSeq2 v2);
        DoubleSeq1 opDoubleSeq(DoubleSeq1 v1, out DoubleSeq2 v2);

        ["python:memoryview:Custom.myBogusArrayNotExistsFactory"] BoolSeq1 opBogusArrayNotExistsFactory();
        ["python:memoryview:Custom.myBogusArrayThrowFactory"]BoolSeq1 opBogusArrayThrowFactory();
        ["python:memoryview:Custom.myBogusArrayType"]BoolSeq1 opBogusArrayType();
        ["python:memoryview:Custom.myBogusArrayNoneFactory"]BoolSeq1 opBogusArrayNoneFactory();
        ["python:memoryview:Custom.myBogusArraySignatureFactory"]BoolSeq1 opBogusArraySignatureFactory();
        ["python:memoryview:Custom.myNoCallableFactory"]BoolSeq1 opBogusArrayNoCallableFactory();

        void shutdown();
    }
}
