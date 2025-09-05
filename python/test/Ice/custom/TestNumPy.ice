// Copyright (c) ZeroC, Inc.

#pragma once

["python:identifier:generated.test.Ice.custom.Test"]
module Test
{
    module NumPy
    {
        sequence<bool> BoolSeq1;
        ["python:numpy.ndarray"] sequence<bool> BoolSeq2;
        ["python:memoryview:CustomFactory.myNumPyBoolSeq:numpy.ndarray"] sequence<bool> BoolSeq3;

        sequence<byte> ByteSeq1;
        ["python:numpy.ndarray"] sequence<byte> ByteSeq2;
        ["python:memoryview:CustomFactory.myNumPyByteSeq:numpy.ndarray"] sequence<byte> ByteSeq3;

        sequence<short> ShortSeq1;
        ["python:numpy.ndarray"] sequence<short> ShortSeq2;
        ["python:memoryview:CustomFactory.myNumPyShortSeq:numpy.ndarray"] sequence<short> ShortSeq3;

        sequence<int> IntSeq1;
        ["python:numpy.ndarray"] sequence<int> IntSeq2;
        ["python:memoryview:CustomFactory.myNumPyIntSeq:numpy.ndarray"] sequence<int> IntSeq3;

        sequence<long> LongSeq1;
        ["python:numpy.ndarray"] sequence<long> LongSeq2;
        ["python:memoryview:CustomFactory.myNumPyLongSeq:numpy.ndarray"] sequence<long> LongSeq3;

        sequence<float> FloatSeq1;
        ["python:numpy.ndarray"] sequence<float> FloatSeq2;
        ["python:memoryview:CustomFactory.myNumPyFloatSeq:numpy.ndarray"] sequence<float> FloatSeq3;

        sequence<double> DoubleSeq1;
        ["python:numpy.ndarray"] sequence<double> DoubleSeq2;
        ["python:memoryview:CustomFactory.myNumPyDoubleSeq:numpy.ndarray"] sequence<double> DoubleSeq3;

        ["python:memoryview:CustomFactory.myNumPyComplex128Seq:numpy.ndarray"] sequence<byte> Complex128Seq;

        class D
        {
            optional(1) BoolSeq2 boolSeq;
            optional(2) ByteSeq2 byteSeq;
            optional(3) ShortSeq2 shortSeq;
            optional(4) IntSeq2 intSeq;
            optional(5) LongSeq2 longSeq;
            optional(6) FloatSeq2 floatSeq;
            optional(7) DoubleSeq2 doubleSeq;
        }

        class E
        {
            BoolSeq2 boolSeq;
            ByteSeq2 byteSeq;
            ShortSeq2 shortSeq;
            IntSeq2 intSeq;
            LongSeq2 longSeq;
            FloatSeq2 floatSeq;
            DoubleSeq2 doubleSeq;
        }

        interface Custom
        {
            BoolSeq2 opBoolSeq(BoolSeq1 v1, BoolSeq2 v2, out BoolSeq3 v3);
            ByteSeq2 opByteSeq(ByteSeq1 v1, ByteSeq2 v2, out ByteSeq3 v3);
            ShortSeq2 opShortSeq(ShortSeq1 v1, ShortSeq2 v2, out ShortSeq3 v3);
            IntSeq2 opIntSeq(IntSeq1 v1, IntSeq2 v2, out IntSeq3 v3);
            LongSeq2 opLongSeq(LongSeq1 v1, LongSeq2 v2, out LongSeq3 v3);
            FloatSeq2 opFloatSeq(FloatSeq1 v1, FloatSeq2 v2, out FloatSeq3 v3);
            DoubleSeq2 opDoubleSeq(DoubleSeq1 v1, DoubleSeq2 v2, out DoubleSeq3 v3);
            Complex128Seq opComplex128Seq(Complex128Seq v1);

            ["python:memoryview:CustomFactory.myNumPyMatrix3x3:numpy.ndarray"] BoolSeq1 opBoolMatrix();
            ["python:memoryview:CustomFactory.myNumPyMatrix3x3:numpy.ndarray"] ByteSeq1 opByteMatrix();
            ["python:memoryview:CustomFactory.myNumPyMatrix3x3:numpy.ndarray"] ShortSeq1 opShortMatrix();
            ["python:memoryview:CustomFactory.myNumPyMatrix3x3:numpy.ndarray"] IntSeq1 opIntMatrix();
            ["python:memoryview:CustomFactory.myNumPyMatrix3x3:numpy.ndarray"] LongSeq1 opLongMatrix();
            ["python:memoryview:CustomFactory.myNumPyMatrix3x3:numpy.ndarray"] FloatSeq1 opFloatMatrix();
            ["python:memoryview:CustomFactory.myNumPyMatrix3x3:numpy.ndarray"] DoubleSeq1 opDoubleMatrix();

            ["python:memoryview:CustomFactory.myBogusNumpyArrayType:numpy.ndarray"] BoolSeq1 opBogusNumpyArrayType();

            D opD(D d);

            void shutdown();
        }
    }
}
