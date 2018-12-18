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
    module NumPy
    {
        ["python:numpy.ndarray"] sequence<bool> BoolSeq1;
        ["python:memoryview:Custom.myNumPyBoolSeq"] sequence<bool> BoolSeq2;

        ["python:numpy.ndarray"] sequence<byte> ByteSeq1;
        ["python:memoryview:Custom.myNumPyByteSeq"] sequence<byte> ByteSeq2;

        ["python:numpy.ndarray"] sequence<short> ShortSeq1;
        ["python:memoryview:Custom.myNumPyShortSeq"] sequence<short> ShortSeq2;

        ["python:numpy.ndarray"] sequence<int> IntSeq1;
        ["python:memoryview:Custom.myNumPyIntSeq"] sequence<int> IntSeq2;

        ["python:numpy.ndarray"] sequence<long> LongSeq1;
        ["python:memoryview:Custom.myNumPyLongSeq"] sequence<long> LongSeq2;

        ["python:numpy.ndarray"] sequence<float> FloatSeq1;
        ["python:memoryview:Custom.myNumPyFloatSeq"] sequence<float> FloatSeq2;

        ["python:numpy.ndarray"] sequence<double> DoubleSeq1;
        ["python:memoryview:Custom.myNumPyDoubleSeq"] sequence<double> DoubleSeq2;

        ["python:memoryview:Custom.myNumPyComplex128Seq"] sequence<byte> Complex128Seq;

        interface Custom
        {
            BoolSeq1 opBoolSeq(BoolSeq1 v1, out BoolSeq2 v2);
            ByteSeq1 opByteSeq(ByteSeq1 v1, out ByteSeq2 v2);
            ShortSeq1 opShortSeq(ShortSeq1 v1, out ShortSeq2 v2);
            IntSeq1 opIntSeq(IntSeq1 v1, out IntSeq2 v2);
            LongSeq1 opLongSeq(LongSeq1 v1, out LongSeq2 v2);
            FloatSeq1 opFloatSeq(FloatSeq1 v1, out FloatSeq2 v2);
            DoubleSeq1 opDoubleSeq(DoubleSeq1 v1, out DoubleSeq2 v2);
            Complex128Seq opComplex128Seq(Complex128Seq v1);

            ["python:memoryview:Custom.myNumPyMatrix3x3"] BoolSeq1 opBoolMatrix();
            ["python:memoryview:Custom.myNumPyMatrix3x3"] ByteSeq1 opByteMatrix();
            ["python:memoryview:Custom.myNumPyMatrix3x3"] ShortSeq1 opShortMatrix();
            ["python:memoryview:Custom.myNumPyMatrix3x3"] IntSeq1 opIntMatrix();
            ["python:memoryview:Custom.myNumPyMatrix3x3"] LongSeq1 opLongMatrix();
            ["python:memoryview:Custom.myNumPyMatrix3x3"] FloatSeq1 opFloatMatrix();
            ["python:memoryview:Custom.myNumPyMatrix3x3"] DoubleSeq1 opDoubleMatrix();

            ["python:memoryview:Custom.myBogusNumpyArrayType"]BoolSeq1 opBogusNumpyArrayType();

            void shutdown();
        }
    }
}
