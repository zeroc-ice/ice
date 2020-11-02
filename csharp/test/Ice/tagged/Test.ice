//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[3.7]]
[[suppress-warning(reserved-identifier)]]

module ZeroC::Ice::Test::Tagged
{

class OneTagged
{
    tag(1) int? a;
}

enum MyEnum
{
    M1
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

sequence<ushort> UShortSeq;
sequence<varulong> VarULongSeq;

sequence<IntSeq> IntSeqSeq;

[cs:generic(List)] sequence<byte> ByteList;
[cs:generic(List)] sequence<bool> BoolList;
[cs:generic(List)] sequence<short> ShortList;
[cs:generic(List)] sequence<int> IntList;
[cs:generic(List)] sequence<long> LongList;
[cs:generic(List)] sequence<float> FloatList;
[cs:generic(List)] sequence<double> DoubleList;
[cs:generic(List)] sequence<string> StringList;
[cs:generic(List)] sequence<varint> VarIntList;

sequence<SmallStruct> SmallStructSeq;
[cs:generic(List)] sequence<SmallStruct> SmallStructList;
sequence<FixedStruct> FixedStructSeq;
[cs:generic(LinkedList)] sequence<FixedStruct> FixedStructList;
sequence<VarStruct> VarStructSeq;

dictionary<int, int> IntIntDict;
dictionary<string, int> StringIntDict;
dictionary<int, MyEnum> IntEnumDict;
dictionary<int, FixedStruct> IntFixedStructDict;
dictionary<int, VarStruct> IntVarStructDict;

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

    tag(24) IntEnumDict? ied;
    tag(25) IntFixedStructDict? ifsd;
    tag(26) IntVarStructDict? ivsd;

    tag(29) BoolSeq? bos;

    tag(31) ushort? us;
    tag(32) uint? ui;
    tag(33) ulong? ul;
    tag(34) varint? vi;
    tag(35) varlong? vl;
    tag(36) varuint? vui;
    tag(37) varulong? vul;

    tag(38) UShortSeq? uss;
    tag(39) VarULongSeq? vuls;
    tag(40) VarIntList? vil;
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
    tag(50) VarStruct? vs;
}

exception DerivedException : TaggedException
{
    tag(600) string? ss = "test";
    tag(601) VarStruct? vs2;
}

exception RequiredException : TaggedException
{
    string ss = "test";
    VarStruct vs2;
}

class TaggedWithCustom
{
    tag(1) SmallStructList? l;
    tag(2) SmallStructList? lp;
    tag(3) ClassVarStruct? s;
}

interface Initial
{
    void shutdown();

    Object pingPong(Object o);

    void opTaggedException(tag(1) int? a, tag(2) string? b, optional(3) VarStruct? vs);

    void opDerivedException(tag(1) int? a, tag(2) string? b, optional(3) VarStruct? vs);

    void opRequiredException(tag(1) int? a, tag(2) string? b, optional(3) VarStruct? vs);

    (tag(1) byte? r1, tag(2) byte? r2) opByte(tag(1) byte? p1);

    (tag(1) bool? r1, tag(2) bool? r2) opBool(tag(1) bool? p1);

    (tag(1) short? r1, tag(2) short? r2) opShort(tag(1) short? p1);

    (tag(1) int? r1, tag(2) int? r2) opInt(tag(1) int? p1);

    (tag(1) long? r1, tag(2) long? r2) opLong(tag(1) long? p1);

    (tag(1) float? r1, tag(2) float? r2) opFloat(tag(1) float? p1);

    (tag(1) double? r1, tag(2) double? r2) opDouble(tag(1) double? p1);

    (tag(1) string? r1, tag(2) string? r2) opString(tag(1) string? p1);

    (tag(1) MyEnum? r1, tag(2) MyEnum? r2) opMyEnum(tag(1) MyEnum? p1);

    (tag(1) SmallStruct? r1, tag(2) SmallStruct? r2) opSmallStruct(tag(1) SmallStruct? p1);

    (tag(1) FixedStruct? r1, tag(2) FixedStruct? r2) opFixedStruct(tag(1) FixedStruct? p1);

    (tag(1) VarStruct? r1, tag(2) VarStruct? r2) opVarStruct(tag(1) VarStruct? p1);

    (tag(1) ByteSeq? r1, tag(2) ByteSeq? r2) opByteSeq(tag(1) ByteSeq? p1);
    (tag(1) ByteList? r1, tag(2) ByteList? r2) opByteList(tag(1) ByteList? p1);

    (tag(1) BoolSeq? r1, tag(2) BoolSeq? r2) opBoolSeq(tag(1) BoolSeq? p1);
    (tag(1) BoolList? r1, tag(2) BoolList? r2) opBoolList(tag(1) BoolList? p1);

    (tag(1) ShortSeq? r1, tag(2) ShortSeq? r2) opShortSeq(tag(1) ShortSeq? p1);
    (tag(1) ShortList? r1, tag(2) ShortList? r2) opShortList(tag(1) ShortList? p1);

    (tag(1) IntSeq? r1, tag(2) IntSeq? r2) opIntSeq(tag(1) IntSeq? p1);
    (tag(1) IntList? r1, tag(2) IntList? r2) opIntList(tag(1) IntList? p1);

    (tag(1) LongSeq? r1, tag(2) LongSeq? r2) opLongSeq(tag(1) LongSeq? p1);
    (tag(1) LongList? r1, tag(2) LongList? r2) opLongList(tag(1) LongList? p1);

    (tag(1) FloatSeq? r1, tag(2) FloatSeq? r2) opFloatSeq(tag(1) FloatSeq? p1);
    (tag(1) FloatList? r1, tag(2) FloatList? r2) opFloatList(tag(1) FloatList? p1);

    (tag(1) DoubleSeq? r1, tag(2) DoubleSeq? r2) opDoubleSeq(tag(1) DoubleSeq? p1);
    (tag(1) DoubleList? r1, tag(2) DoubleList? r2) opDoubleList(tag(1) DoubleList? p1);

    (tag(1) StringSeq? r1, tag(2) StringSeq? r2) opStringSeq(tag(1) StringSeq? p1);
    (tag(1) StringList? r1, tag(2) StringList? r2) opStringList(tag(1) StringList? p1);

    (tag(1) SmallStructSeq? r1, tag(2) SmallStructSeq? r2) opSmallStructSeq(tag(1) SmallStructSeq? p1);

    (tag(1) SmallStructList? r1, tag(2) SmallStructList? r2) opSmallStructList(tag(1) SmallStructList? p1);

    (tag(1) FixedStructSeq? r1, tag(2) FixedStructSeq? r2) opFixedStructSeq(tag(1) FixedStructSeq? p1);

    (tag(1) FixedStructList? r1, tag(2) FixedStructList? r2) opFixedStructList(tag(1) FixedStructList? p1);

    (tag(1) VarStructSeq? r1, tag(2) VarStructSeq? r2) opVarStructSeq(tag(1) VarStructSeq? p1);

    (tag(1) IntSeqSeq? r1, tag(2) IntSeqSeq? r2) opIntSeqSeq(tag(1) IntSeqSeq? p1);

    (tag(1) IntIntDict? r1, tag(2) IntIntDict? r2) opIntIntDict(tag(1) IntIntDict? p1);

    (tag(1) StringIntDict? r1, tag(2) StringIntDict? r2) opStringIntDict(tag(1) StringIntDict? p1);

    void opClassAndUnknownTagged(A p);

    void opVoid();

    [marshaled-result] tag(1) SmallStruct? opMStruct1();
    [marshaled-result] (tag(1) SmallStruct? r1, tag(2) SmallStruct? r2) opMStruct2(tag(1) SmallStruct? p1);

    [marshaled-result] tag(1) StringSeq? opMSeq1();
    [marshaled-result] (tag(1) StringSeq? r1, tag(2) StringSeq? r2) opMSeq2(tag(1) StringSeq? p1);

    [marshaled-result] tag(1) StringIntDict? opMDict1();
    [marshaled-result] (tag(1) StringIntDict? r1, tag(2) StringIntDict? r2) opMDict2(tag(1) StringIntDict? p1);

    bool supportsRequiredParams();

    bool supportsJavaSerializable();

    bool supportsCppStringView();

    bool supportsNullTagged();
}

}
