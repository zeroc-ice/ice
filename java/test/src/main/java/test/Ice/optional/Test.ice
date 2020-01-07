//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["java:package:test.Ice.optional", "suppress-warning:deprecated"]]
module Test
{

class OneOptional
{
    tag(1) int a;
}

enum MyEnum
{
    MyEnumMember
}

struct SmallStruct
{
    byte m;
}

struct FixedStruct
{
    int m;
}

struct VarStruct
{
    string m;
}

struct ClassVarStruct
{
    int a;
}

sequence<byte> ByteSeq;
sequence<bool> BoolSeq;
sequence<short> ShortSeq;
sequence<int> IntSeq;
sequence<long> LongSeq;
sequence<float> FloatSeq;
sequence<double> DoubleSeq;
sequence<string> StringSeq;
sequence<MyEnum> MyEnumSeq;
sequence<SmallStruct> SmallStructSeq;
["java:type:java.util.ArrayList<SmallStruct>"] sequence<SmallStruct> SmallStructList;
sequence<FixedStruct> FixedStructSeq;
["java:type:java.util.ArrayList<FixedStruct>"] sequence<FixedStruct> FixedStructList;
sequence<VarStruct> VarStructSeq;
sequence<OneOptional> OneOptionalSeq;
sequence<OneOptional*> OneOptionalPrxSeq;

["java:serializable:test.Ice.optional.SerializableClass"] sequence<byte> Serializable;

dictionary<int, int> IntIntDict;
dictionary<string, int> StringIntDict;
dictionary<int, MyEnum> IntEnumDict;
dictionary<int, FixedStruct> IntFixedStructDict;
dictionary<int, VarStruct> IntVarStructDict;
dictionary<int, OneOptional> IntOneOptionalDict;
dictionary<int, OneOptional*> IntOneOptionalPrxDict;

class MultiOptional
{
    tag(1) byte a;
    tag(2) bool b;
    tag(3) short c;
    tag(4) int d;
    tag(5) long e;
    tag(6) float f;
    tag(7) double g;
    tag(8) string h;
    tag(9) MyEnum i;
    tag(10) MultiOptional* j;
    tag(11) MultiOptional k;
    tag(12) ByteSeq bs;
    tag(13) StringSeq ss;
    tag(14) IntIntDict iid;
    tag(15) StringIntDict sid;
    tag(16) FixedStruct fs;
    tag(17) VarStruct vs;

    tag(18) ShortSeq shs;
    tag(19) MyEnumSeq es;
    tag(20) FixedStructSeq fss;
    tag(21) VarStructSeq vss;
    tag(22) OneOptionalSeq oos;
    tag(23) OneOptionalPrxSeq oops;

    tag(24) IntEnumDict ied;
    tag(25) IntFixedStructDict ifsd;
    tag(26) IntVarStructDict ivsd;
    tag(27) IntOneOptionalDict iood;
    tag(28) IntOneOptionalPrxDict ioopd;

    tag(29) BoolSeq bos;

    tag(30) Serializable ser;
}

class A
{
    int requiredA;
    tag(1) int ma;
    tag(50) int mb;
    tag(500) int mc;
}

["preserve-slice"]
class B extends A
{
    int requiredB;
    tag(10) int md;
}

class C extends B
{
    string ss;
    tag(890) string ms;
}

class WD
{
    tag(1) int a = 5;
    tag(2) string s = "test";
}

exception OptionalException
{
    bool req = false;
    tag(1) int a = 5;
    tag(2) string b;
    tag(50) OneOptional o;
}

exception DerivedException extends OptionalException
{
    tag(600) string ss = "test";
    tag(601) OneOptional o2;
}

exception RequiredException extends OptionalException
{
    string ss = "test";
    OneOptional o2;
}

class OptionalWithCustom
{
    tag(1) SmallStructList l;
    ["protected"] tag(2) SmallStructList lp;
    tag(3) ClassVarStruct s;
}

class E
{
    A ae;
}

class F extends E
{
    tag(1) A af;
}

class G1
{
    string a;
}

class G2
{
    long a;
}

class G
{
    tag(1) G1 gg1Opt;
    G2 gg2;
    tag(0) G2 gg2Opt;
    G1 gg1;
}

class Recursive;
sequence<Recursive> RecursiveSeq;

class Recursive {
    tag(0) RecursiveSeq value;
}

interface Initial
{
    void shutdown();

    Object pingPong(Object o);

    void opOptionalException(tag(1) int a, tag(2) string b, tag(3) OneOptional o)
        throws OptionalException;

    void opDerivedException(tag(1) int a, tag(2) string b, tag(3) OneOptional o)
        throws OptionalException;

    void opRequiredException(tag(1) int a, tag(2) string b, tag(3) OneOptional o)
        throws OptionalException;

    tag(1) byte opByte(tag(2) byte p1, out tag(3) byte p3);
    tag(1) byte opByteReq(tag(2) byte p1, out tag(3) byte p3);

    tag(1) bool opBool(tag(2) bool p1, out tag(3) bool p3);
    tag(1) bool opBoolReq(tag(2) bool p1, out tag(3) bool p3);

    tag(1) short opShort(tag(2) short p1, out tag(3) short p3);
    tag(1) short opShortReq(tag(2) short p1, out tag(3) short p3);

    tag(1) int opInt(tag(2) int p1, out tag(3) int p3);
    tag(1) int opIntReq(tag(2) int p1, out tag(3) int p3);

    tag(3) long opLong(tag(1) long p1, out tag(2) long p3);
    tag(3) long opLongReq(tag(1) long p1, out tag(2) long p3);

    tag(1) float opFloat(tag(2) float p1, out tag(3) float p3);
    tag(1) float opFloatReq(tag(2) float p1, out tag(3) float p3);

    tag(1) double opDouble(tag(2) double p1, out tag(3) double p3);
    tag(1) double opDoubleReq(tag(2) double p1, out tag(3) double p3);

    tag(1) string opString(tag(2) string p1, out tag(3) string p3);
    tag(1) string opStringReq(tag(2) string p1, out tag(3) string p3);

    tag(1) MyEnum opMyEnum(tag(2) MyEnum p1, out tag(3) MyEnum p3);
    tag(1) MyEnum opMyEnumReq(tag(2) MyEnum p1, out tag(3) MyEnum p3);

    tag(1) SmallStruct opSmallStruct(tag(2) SmallStruct p1, out tag(3) SmallStruct p3);
    tag(1) SmallStruct opSmallStructReq(tag(2) SmallStruct p1, out tag(3) SmallStruct p3);

    tag(1) FixedStruct opFixedStruct(tag(2) FixedStruct p1, out tag(3) FixedStruct p3);
    tag(1) FixedStruct opFixedStructReq(tag(2) FixedStruct p1, out tag(3) FixedStruct p3);

    tag(1) VarStruct opVarStruct(tag(2) VarStruct p1, out tag(3) VarStruct p3);
    tag(1) VarStruct opVarStructReq(tag(2) VarStruct p1, out tag(3) VarStruct p3);

    tag(1) OneOptional opOneOptional(tag(2) OneOptional p1, out tag(3) OneOptional p3);
    tag(1) OneOptional opOneOptionalReq(tag(2) OneOptional p1, out tag(3) OneOptional p3);

    tag(1) OneOptional* opOneOptionalProxy(tag(2) OneOptional* p1, out tag(3) OneOptional* p3);
    tag(1) OneOptional* opOneOptionalProxyReq(tag(2) OneOptional* p1, out tag(3) OneOptional* p3);

    tag(1) ByteSeq opByteSeq(tag(2) ByteSeq p1, out tag(3) ByteSeq p3);
    tag(1) ByteSeq opByteSeqReq(tag(2) ByteSeq p1, out tag(3) ByteSeq p3);

    tag(1) BoolSeq opBoolSeq(tag(2) BoolSeq p1, out tag(3) BoolSeq p3);
    tag(1) BoolSeq opBoolSeqReq(tag(2) BoolSeq p1, out tag(3) BoolSeq p3);

    tag(1) ShortSeq opShortSeq(tag(2) ShortSeq p1, out tag(3) ShortSeq p3);
    tag(1) ShortSeq opShortSeqReq(tag(2) ShortSeq p1, out tag(3) ShortSeq p3);

    tag(1) IntSeq opIntSeq(tag(2) IntSeq p1, out tag(3) IntSeq p3);
    tag(1) IntSeq opIntSeqReq(tag(2) IntSeq p1, out tag(3) IntSeq p3);

    tag(1) LongSeq opLongSeq(tag(2) LongSeq p1, out tag(3) LongSeq p3);
    tag(1) LongSeq opLongSeqReq(tag(2) LongSeq p1, out tag(3) LongSeq p3);

    tag(1) FloatSeq opFloatSeq(tag(2) FloatSeq p1, out tag(3) FloatSeq p3);
    tag(1) FloatSeq opFloatSeqReq(tag(2) FloatSeq p1, out tag(3) FloatSeq p3);

    tag(1) DoubleSeq opDoubleSeq(tag(2) DoubleSeq p1, out tag(3) DoubleSeq p3);
    tag(1) DoubleSeq opDoubleSeqReq(tag(2) DoubleSeq p1, out tag(3) DoubleSeq p3);

    tag(1) StringSeq opStringSeq(tag(2) StringSeq p1, out tag(3) StringSeq p3);
    tag(1) StringSeq opStringSeqReq(tag(2) StringSeq p1, out tag(3) StringSeq p3);

    tag(1) SmallStructSeq opSmallStructSeq(tag(2) SmallStructSeq p1, out tag(3) SmallStructSeq p3);
    tag(1) SmallStructSeq opSmallStructSeqReq(tag(2) SmallStructSeq p1, out tag(3) SmallStructSeq p3);

    tag(1) SmallStructList opSmallStructList(tag(2) SmallStructList p1, out tag(3) SmallStructList p3);
    tag(1) SmallStructList opSmallStructListReq(tag(2) SmallStructList p1,
                                                     out tag(3) SmallStructList p3);

    tag(1) FixedStructSeq opFixedStructSeq(tag(2) FixedStructSeq p1, out tag(3) FixedStructSeq p3);
    tag(1) FixedStructSeq opFixedStructSeqReq(tag(2) FixedStructSeq p1, out tag(3) FixedStructSeq p3);

    tag(1) FixedStructList opFixedStructList(tag(2) FixedStructList p1, out tag(3) FixedStructList p3);
    tag(1) FixedStructList opFixedStructListReq(tag(2) FixedStructList p1,
                                                     out tag(3) FixedStructList p3);

    tag(1) VarStructSeq opVarStructSeq(tag(2) VarStructSeq p1, out tag(3) VarStructSeq p3);
    tag(1) VarStructSeq opVarStructSeqReq(tag(2) VarStructSeq p1, out tag(3) VarStructSeq p3);

    tag(1) Serializable opSerializable(tag(2) Serializable p1, out tag(3) Serializable p3);
    tag(1) Serializable opSerializableReq(tag(2) Serializable p1, out tag(3) Serializable p3);

    tag(1) IntIntDict opIntIntDict(tag(2) IntIntDict p1, out tag(3) IntIntDict p3);
    tag(1) IntIntDict opIntIntDictReq(tag(2) IntIntDict p1, out tag(3) IntIntDict p3);

    tag(1) StringIntDict opStringIntDict(tag(2) StringIntDict p1, out tag(3) StringIntDict p3);
    tag(1) StringIntDict opStringIntDictReq(tag(2) StringIntDict p1, out tag(3) StringIntDict p3);

    tag(1) IntOneOptionalDict opIntOneOptionalDict(tag(2) IntOneOptionalDict p1,
                                                        out tag(3) IntOneOptionalDict p3);
    tag(1) IntOneOptionalDict opIntOneOptionalDictReq(tag(2) IntOneOptionalDict p1,
                                                           out tag(3) IntOneOptionalDict p3);

    void opClassAndUnknownOptional(A p);

    void sendOptionalClass(bool req, tag(1) OneOptional o);

    void returnOptionalClass(bool req, out tag(1) OneOptional o);

    G opG(G g);

    void opVoid();

    ["marshaled-result"] tag(1) SmallStruct opMStruct1();
    ["marshaled-result"] tag(1) SmallStruct opMStruct2(tag(2) SmallStruct p1,
                                                            out tag(3)SmallStruct p2);

    ["marshaled-result"] tag(1) StringSeq opMSeq1();
    ["marshaled-result"] tag(1) StringSeq opMSeq2(tag(2) StringSeq p1,
                                                       out tag(3) StringSeq p2);

    ["marshaled-result"] tag(1) StringIntDict opMDict1();
    ["marshaled-result"] tag(1) StringIntDict opMDict2(tag(2) StringIntDict p1,
                                                            out tag(3) StringIntDict p2);

    ["marshaled-result"] tag(1) G opMG1();
    ["marshaled-result"] tag(1) G opMG2(tag(2) G p1, out tag(3) G p2);

    bool supportsRequiredParams();

    bool supportsJavaSerializable();

    bool supportsCsharpSerializable();

    bool supportsCppStringView();

    bool supportsNullOptional();
}

}
