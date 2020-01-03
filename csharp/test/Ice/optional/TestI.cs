//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace Ice
{
    namespace optional
    {
        public class InitialI : Test.Initial
        {
            public void shutdown(Current current)
            {
                current.Adapter.Communicator.shutdown();
            }

            public Ice.Value pingPong(Ice.Value obj, Current current)
            {
                return obj;
            }

            public void opOptionalException(int? a,
                                            string? b,
                                            Test.OneOptional? o,
                                            Current current)
            {
                throw new Test.OptionalException(false, a, b, o);
            }

            public void opDerivedException(int? a,
                                           string? b,
                                           Test.OneOptional? o,
                                           Current current)
            {
                throw new Test.DerivedException(false, a, b, o, b, o);
            }

            public void opRequiredException(int? a,
                                            string? b,
                                            Test.OneOptional? o,
                                            Current current)
            {
                Test.RequiredException e = new Test.RequiredException();
                e.a = a;
                e.b = b;
                e.o = o;
                if (b != null)
                {
                    e.ss = b;
                }

                if (o != null)
                {
                    e.o2 = o;
                }
                throw e;
            }

            public byte? opByte(byte? p1, out byte? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public bool? opBool(bool? p1, out bool? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public short? opShort(short? p1, out short? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public int? opInt(int? p1, out int? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public long? opLong(long? p1, out long? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public float? opFloat(float? p1, out float? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public double? opDouble(double? p1, out double? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public string? opString(string? p1, out string? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public Test.MyEnum? opMyEnum(Test.MyEnum? p1, out Test.MyEnum? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public Test.SmallStruct? opSmallStruct(Test.SmallStruct? p1, out Test.SmallStruct? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public Test.FixedStruct? opFixedStruct(Test.FixedStruct? p1, out Test.FixedStruct? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public Test.VarStruct? opVarStruct(Test.VarStruct? p1, out Test.VarStruct? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public Test.OneOptional? opOneOptional(Test.OneOptional? p1, out Test.OneOptional? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public IObjectPrx?
            opOneOptionalProxy(IObjectPrx? p1, out IObjectPrx? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public byte[]? opByteSeq(byte[]? p1, out byte[]? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public bool[]? opBoolSeq(bool[]? p1, out bool[]? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public short[]? opShortSeq(short[]? p1, out short[]? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public int[]? opIntSeq(int[]? p1, out int[]? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public long[]? opLongSeq(long[]? p1, out long[]? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public float[]? opFloatSeq(float[]? p1, out float[]? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public double[]? opDoubleSeq(double[]? p1, out double[]? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public string[]? opStringSeq(string[]? p1, out string[]? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public Test.SmallStruct[]?
            opSmallStructSeq(Test.SmallStruct[]? p1, out Test.SmallStruct[]? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public List<Test.SmallStruct>?
            opSmallStructList(List<Test.SmallStruct>? p1, out List<Test.SmallStruct>? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public Test.FixedStruct[]?
            opFixedStructSeq(Test.FixedStruct[]? p1, out Test.FixedStruct[]? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public LinkedList<Test.FixedStruct>?
            opFixedStructList(LinkedList<Test.FixedStruct>? p1, out LinkedList<Test.FixedStruct>? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public Test.VarStruct[]?
            opVarStructSeq(Test.VarStruct[]? p1, out Test.VarStruct[]? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public Test.SerializableClass?
            opSerializable(Test.SerializableClass? p1, out Test.SerializableClass? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public Dictionary<int, int>?
            opIntIntDict(Dictionary<int, int>? p1, out Dictionary<int, int>? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public Dictionary<string, int>?
            opStringIntDict(Dictionary<string, int>? p1, out Dictionary<string, int>? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public Dictionary<int, Test.OneOptional>?
            opIntOneOptionalDict(Dictionary<int, Test.OneOptional>? p1, out Dictionary<int, Test.OneOptional>? p3, Current current)
            {
                p3 = p1;
                return p1;
            }

            public void opClassAndUnknownOptional(Test.A p, Current current)
            {
            }

            public void sendOptionalClass(bool req, Test.OneOptional? o, Current current)
            {
            }

            public void returnOptionalClass(bool req, out Test.OneOptional? o, Current current)
            {
                o = new Test.OneOptional(53);
            }

            public Test.G opG(Test.G g, Current current)
            {
                return g;
            }

            public void opVoid(Current current)
            {
            }

            public Test.Initial.opMStruct1MarshaledResult
            opMStruct1(Current current)
            {
                return new Test.Initial.opMStruct1MarshaledResult(new Test.SmallStruct(), current);
            }

            public Test.Initial.opMStruct2MarshaledResult
            opMStruct2(Test.SmallStruct? p1, Current current)
            {
                return new Test.Initial.opMStruct2MarshaledResult(p1, p1, current);
            }

            public Test.Initial.opMSeq1MarshaledResult
            opMSeq1(Current current)
            {
                return new Test.Initial.opMSeq1MarshaledResult(new string[0], current);
            }

            public Test.Initial.opMSeq2MarshaledResult
            opMSeq2(string[]? p1, Current current)
            {
                return new Test.Initial.opMSeq2MarshaledResult(p1, p1, current);
            }

            public Test.Initial.opMDict1MarshaledResult
            opMDict1(Current current)
            {
                return new Test.Initial.opMDict1MarshaledResult(new Dictionary<string, int>(), current);
            }

            public Test.Initial.opMDict2MarshaledResult
            opMDict2(Dictionary<string, int>? p1, Current current)
            {
                return new Test.Initial.opMDict2MarshaledResult(p1, p1, current);
            }

            public Test.Initial.opMG1MarshaledResult
            opMG1(Current current)
            {
                return new Test.Initial.opMG1MarshaledResult(new Test.G(), current);
            }

            public Test.Initial.opMG2MarshaledResult
            opMG2(Test.G? p1, Current current)
            {
                return new Test.Initial.opMG2MarshaledResult(p1, p1, current);
            }

            public bool supportsRequiredParams(Current current)
            {
                return false;
            }

            public bool supportsJavaSerializable(Current current)
            {
                return false;
            }

            public bool supportsCsharpSerializable(Current current)
            {
                return true;
            }

            public bool supportsCppStringView(Current current)
            {
                return false;
            }

            public bool supportsNullOptional(Current current)
            {
                return true;
            }
        }
    }
}
