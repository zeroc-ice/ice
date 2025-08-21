// Copyright (c) ZeroC, Inc.

#pragma once

["python:identifier:generated.test.Ice.custom.Test"]
module Test
{
    module NumPy
    {
        ["python:numpy.ndarray"] sequence<bool> BoolSeq1;
        ["python:memoryview:CustomFactory.myNumPyBoolSeq:numpy.ndarray"] sequence<bool> BoolSeq2;

        ["python:numpy.ndarray"] sequence<byte> ByteSeq1;
        ["python:memoryview:CustomFactory.myNumPyByteSeq:numpy.ndarray"] sequence<byte> ByteSeq2;

        ["python:numpy.ndarray"] sequence<short> ShortSeq1;
        ["python:memoryview:CustomFactory.myNumPyShortSeq:numpy.ndarray"] sequence<short> ShortSeq2;

        ["python:numpy.ndarray"] sequence<int> IntSeq1;
        ["python:memoryview:CustomFactory.myNumPyIntSeq:numpy.ndarray"] sequence<int> IntSeq2;

        ["python:numpy.ndarray"] sequence<long> LongSeq1;
        ["python:memoryview:CustomFactory.myNumPyLongSeq:numpy.ndarray"] sequence<long> LongSeq2;

        ["python:numpy.ndarray"] sequence<float> FloatSeq1;
        ["python:memoryview:CustomFactory.myNumPyFloatSeq:numpy.ndarray"] sequence<float> FloatSeq2;

        ["python:numpy.ndarray"] sequence<double> DoubleSeq1;
        ["python:memoryview:CustomFactory.myNumPyDoubleSeq:numpy.ndarray"] sequence<double> DoubleSeq2;

        ["python:memoryview:CustomFactory.myNumPyComplex128Seq:numpy.ndarray"] sequence<byte> Complex128Seq;

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
            BoolSeq1 opBoolSeq(BoolSeq1 v1, out BoolSeq2 v2);
            ByteSeq1 opByteSeq(ByteSeq1 v1, out ByteSeq2 v2);
            ShortSeq1 opShortSeq(ShortSeq1 v1, out ShortSeq2 v2);
            IntSeq1 opIntSeq(IntSeq1 v1, out IntSeq2 v2);
            LongSeq1 opLongSeq(LongSeq1 v1, out LongSeq2 v2);
            FloatSeq1 opFloatSeq(FloatSeq1 v1, out FloatSeq2 v2);
            DoubleSeq1 opDoubleSeq(DoubleSeq1 v1, out DoubleSeq2 v2);
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
