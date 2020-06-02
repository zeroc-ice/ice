//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[3.7]]

// 'Test.ice' and 'TestAMD.ice' need to generate code into separate namespaces, but with identical type-ids. So we use
// the 'cs:namespace' metadata here to place the AMD code into a separate AMD namespace, instead of another module;
// Which would result in differing type-ids.
[cs:namespace:ZeroC.Ice.Test.AMD]
module Tagged
{

class OneTagged
{
    tag(1) int? a;
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

[clr:generic:List] sequence<byte> ByteList;
[clr:generic:List] sequence<bool> BoolList;
[clr:generic:List] sequence<short> ShortList;
[clr:generic:List] sequence<int> IntList;
[clr:generic:List] sequence<long> LongList;
[clr:generic:List] sequence<float> FloatList;
[clr:generic:List] sequence<double> DoubleList;
[clr:generic:List] sequence<string> StringList;

sequence<MyEnum> MyEnumSeq;
sequence<SmallStruct> SmallStructSeq;
[clr:generic:List] sequence<SmallStruct> SmallStructList;
sequence<FixedStruct> FixedStructSeq;
[clr:generic:LinkedList] sequence<FixedStruct> FixedStructList;
sequence<VarStruct> VarStructSeq;
sequence<OneTagged> OneTaggedSeq;

[clr:serializable:ZeroC.Ice.Test.Tagged.SerializableClass]
sequence<byte> Serializable;

dictionary<int, int> IntIntDict;
dictionary<string, int> StringIntDict;
dictionary<int, MyEnum> IntEnumDict;
dictionary<int, FixedStruct> IntFixedStructDict;
dictionary<int, VarStruct> IntVarStructDict;
dictionary<int, OneTagged> IntOneTaggedDict;

class MultiTagged
{
    tag(1) byte? a;
    tag(2) bool? b;
    tag(3) short? c;
    tag(4) int? d;
    tag(5) long? e;
    tag(6) float? f;
    tag(7) double? g;
    tag(8) string? h;
    tag(9) MyEnum? i;
    tag(11) MultiTagged? k;
    tag(12) ByteSeq? bs;
    tag(13) StringSeq? ss;
    tag(14) IntIntDict? iid;
    tag(15) StringIntDict? sid;
    tag(16) FixedStruct? fs;
    tag(17) VarStruct? vs;

    tag(18) ShortSeq? shs;
    tag(19) MyEnumSeq? es;
    tag(20) FixedStructSeq? fss;
    tag(21) VarStructSeq? vss;
    tag(22) OneTaggedSeq? oos;

    tag(24) IntEnumDict? ied;
    tag(25) IntFixedStructDict? ifsd;
    tag(26) IntVarStructDict? ivsd;
    tag(27) IntOneTaggedDict? iood;

    tag(29) BoolSeq? bos;

    tag(30) Serializable? ser;
}

class A
{
    int requiredA;
    tag(1) int? ma;
    tag(50) int? mb;
    tag(500) int? mc;
}

[preserve-slice]
class B : A
{
    int requiredB;
    tag(10) int? md;
}

class C : B
{
    string ss;
    tag(890) string? ms;
}

class WD
{
    tag(1) int? a = 5;
    tag(2) string? s = "test";
}

exception TaggedException
{
    bool req = false;
    tag(1) int? a = 5;
    tag(2) string? b;
    tag(50) OneTagged? o;
}

exception DerivedException : TaggedException
{
    tag(600) string? ss = "test";
    tag(601) OneTagged? o2;
}

exception RequiredException : TaggedException
{
    string ss = "test";
    OneTagged? o2;
}

[clr:property]
class TaggedWithCustom
{
    tag(1) SmallStructList? l;
    tag(2) SmallStructList? lp;
    tag(3) ClassVarStruct? s;
}

class E
{
    A ae;
}

class F : E
{
    tag(1) A? af;
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
    tag(1) G1? gg1Opt;
    G2 gg2;
    tag(0) G2? gg2Opt;
    G1 gg1;
}

class Recursive;
sequence<Recursive> RecursiveSeq;

class Recursive
{
    tag(0) RecursiveSeq? value;
}

[amd] interface Initial
{
    void shutdown();

    Object pingPong(Object o);

    void opTaggedException(tag(1) int? a, tag(2) string? b, tag(3) OneTagged? o)
        throws TaggedException;

    void opDerivedException(tag(1) int? a, tag(2) string? b, tag(3) OneTagged? o)
        throws TaggedException;

    void opRequiredException(tag(1) int? a, tag(2) string? b, tag(3) OneTagged? o)
        throws TaggedException;

    tag(1) byte? opByte(tag(2) byte? p1, out tag(3) byte? p3);

    tag(1) bool? opBool(tag(2) bool? p1, out tag(3) bool? p3);

    tag(1) short? opShort(tag(2) short? p1, out tag(3) short? p3);

    tag(1) int? opInt(tag(2) int? p1, out tag(3) int? p3);

    tag(3) long? opLong(tag(1) long? p1, out tag(2) long? p3);

    tag(1) float? opFloat(tag(2) float? p1, out tag(3) float? p3);

    tag(1) double? opDouble(tag(2) double? p1, out tag(3) double? p3);

    tag(1) string? opString(tag(2) string? p1, out tag(3) string? p3);

    tag(1) MyEnum? opMyEnum(tag(2) MyEnum? p1, out tag(3) MyEnum? p3);

    tag(1) SmallStruct? opSmallStruct(tag(2) SmallStruct? p1, out tag(3) SmallStruct? p3);

    tag(1) FixedStruct? opFixedStruct(tag(2) FixedStruct? p1, out tag(3) FixedStruct? p3);

    tag(1) VarStruct? opVarStruct(tag(2) VarStruct? p1, out tag(3) VarStruct? p3);

    tag(1) OneTagged? opOneTagged(tag(2) OneTagged? p1, out tag(3) OneTagged? p3);

    tag(1) ByteSeq? opByteSeq(tag(2) ByteSeq? p1, out tag(3) ByteSeq? p3);
    tag(1) ByteList? opByteList(tag(2) ByteList? p1, out tag(3) ByteList? p3);

    tag(1) BoolSeq? opBoolSeq(tag(2) BoolSeq? p1, out tag(3) BoolSeq? p3);
    tag(1) BoolList? opBoolList(tag(2) BoolList? p1, out tag(3) BoolList? p3);

    tag(1) ShortSeq? opShortSeq(tag(2) ShortSeq? p1, out tag(3) ShortSeq? p3);
    tag(1) ShortList? opShortList(tag(2) ShortList? p1, out tag(3) ShortList? p3);

    tag(1) IntSeq? opIntSeq(tag(2) IntSeq? p1, out tag(3) IntSeq? p3);
    tag(1) IntList? opIntList(tag(2) IntList? p1, out tag(3) IntList? p3);

    tag(1) LongSeq? opLongSeq(tag(2) LongSeq? p1, out tag(3) LongSeq? p3);
    tag(1) LongList? opLongList(tag(2) LongList? p1, out tag(3) LongList? p3);

    tag(1) FloatSeq? opFloatSeq(tag(2) FloatSeq? p1, out tag(3) FloatSeq? p3);
    tag(1) FloatList? opFloatList(tag(2) FloatList? p1, out tag(3) FloatList? p3);

    tag(1) DoubleSeq? opDoubleSeq(tag(2) DoubleSeq? p1, out tag(3) DoubleSeq? p3);
    tag(1) DoubleList? opDoubleList(tag(2) DoubleList? p1, out tag(3) DoubleList? p3);

    tag(1) StringSeq? opStringSeq(tag(2) StringSeq? p1, out tag(3) StringSeq? p3);
    tag(1) StringList? opStringList(tag(2) StringList? p1, out tag(3) StringList? p3);

    tag(1) SmallStructSeq? opSmallStructSeq(tag(2) SmallStructSeq? p1, out tag(3) SmallStructSeq? p3);

    tag(1) SmallStructList? opSmallStructList(tag(2) SmallStructList? p1, out tag(3) SmallStructList? p3);

    tag(1) FixedStructSeq? opFixedStructSeq(tag(2) FixedStructSeq? p1, out tag(3) FixedStructSeq? p3);

    tag(1) FixedStructList? opFixedStructList(tag(2) FixedStructList? p1, out tag(3) FixedStructList? p3);

    tag(1) VarStructSeq? opVarStructSeq(tag(2) VarStructSeq? p1, out tag(3) VarStructSeq? p3);

    tag(1) Serializable? opSerializable(tag(2) Serializable? p1, out tag(3) Serializable? p3);

    tag(1) IntIntDict? opIntIntDict(tag(2) IntIntDict? p1, out tag(3) IntIntDict? p3);

    tag(1) StringIntDict? opStringIntDict(tag(2) StringIntDict? p1, out tag(3) StringIntDict? p3);

    tag(1) IntOneTaggedDict? opIntOneTaggedDict(tag(2) IntOneTaggedDict? p1,
                                                        out tag(3) IntOneTaggedDict? p3);

    void opClassAndUnknownTagged(A p);

    void sendTaggedClass(bool req, tag(1) OneTagged? o);

    void returnTaggedClass(bool req, out tag(1) OneTagged? o);

    G opG(G g);

    void opVoid();

    [marshaled-result] tag(1) SmallStruct? opMStruct1();
    [marshaled-result] tag(1) SmallStruct? opMStruct2(tag(2) SmallStruct? p1, out tag(3) SmallStruct? p2);

    [marshaled-result] tag(1) StringSeq? opMSeq1();
    [marshaled-result] tag(1) StringSeq? opMSeq2(tag(2) StringSeq? p1, out tag(3) StringSeq? p2);

    [marshaled-result] tag(1) StringIntDict? opMDict1();
    [marshaled-result] tag(1) StringIntDict? opMDict2(tag(2) StringIntDict? p1, out tag(3) StringIntDict? p2);

    [marshaled-result] tag(1) G? opMG1();
    [marshaled-result] tag(1) G? opMG2(tag(2) G? p1, out tag(3) G? p2);

    bool supportsRequiredParams();

    bool supportsJavaSerializable();

    bool supportsCsharpSerializable();

    bool supportsCppStringView();

    bool supportsNullTagged();
}

}
