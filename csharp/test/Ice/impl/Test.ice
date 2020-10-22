//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/Context.ice>

module ZeroC::Ice::Test::Impl
{

enum MyEnum
{
    enum1,
    enum2,
    enum3
}

interface MyClass;

struct AnotherStruct
{
    string s;
}

struct Structure
{
    MyClass* p;
    MyEnum e;
    AnotherStruct s;
}

sequence<byte> ByteS;
sequence<bool> BoolS;
sequence<short> ShortS;
sequence<int> IntS;
sequence<long> LongS;
sequence<float> FloatS;
sequence<double> DoubleS;
sequence<string> StringS;
sequence<MyEnum> MyEnumS;
sequence<MyClass*> MyClassS;

sequence<ByteS> ByteSS;
sequence<BoolS> BoolSS;
sequence<ShortS> ShortSS;
sequence<IntS> IntSS;
sequence<LongS> LongSS;
sequence<FloatS> FloatSS;
sequence<DoubleS> DoubleSS;
sequence<StringS> StringSS;
sequence<MyEnumS> MyEnumSS;
sequence<MyClassS> MyClassSS;

sequence<StringSS> StringSSS;

struct MyStruct
{
    int i;
    int j;
}

dictionary<byte, bool> ByteBoolD;
dictionary<short, int> ShortIntD;
dictionary<long, float> LongFloatD;
dictionary<string, string> StringStringD;
dictionary<string, MyEnum> StringMyEnumD;
dictionary<MyEnum, string> MyEnumStringD;
dictionary<MyStruct, MyEnum> MyStructMyEnumD;

sequence<ByteBoolD> ByteBoolDS;
sequence<ShortIntD> ShortIntDS;
sequence<LongFloatD> LongFloatDS;
sequence<StringStringD> StringStringDS;
sequence<StringMyEnumD> StringMyEnumDS;
sequence<MyEnumStringD> MyEnumStringDS;
sequence<MyStructMyEnumD> MyStructMyEnumDS;

dictionary<byte, ByteS> ByteByteSD;
dictionary<bool, BoolS> BoolBoolSD;
dictionary<short, ShortS> ShortShortSD;
dictionary<int, IntS> IntIntSD;
dictionary<long, LongS> LongLongSD;
dictionary<string, FloatS> StringFloatSD;
dictionary<string, DoubleS> StringDoubleSD;
dictionary<string, StringS> StringStringSD;
dictionary<MyEnum, MyEnumS> MyEnumMyEnumSD;

exception SomeException {}

interface MyClass
{
    void shutdown();

    void opVoid();

    (byte r1, byte r2) opByte(byte p1, byte p2);

    (bool r1, bool r2) opBool(bool p1, bool p2);

    (long r1, short r2, int r3, long r4) opShortIntLong(short p1, int p2, long p3);

    (double r1, float r2, double r3) opFloatDouble(float p1, double p2);

    (string r1, string r2) opString(string p1, string p2);

    (MyEnum r1, MyEnum r2) opMyEnum(MyEnum p1);

    (MyClass* r1, MyClass* r2, MyClass* r3) opMyClass(MyClass* p1);

    (Structure r1, Structure r2) opStruct(Structure p1, Structure p2);

    (ByteS r1, ByteS r2) opByteS(ByteS p1, ByteS p2);

    (BoolS r1, BoolS r2) opBoolS(BoolS p1, BoolS p2);

    (LongS r1, ShortS r2, IntS r3, LongS r4) opShortIntLongS(ShortS p1, IntS p2, LongS p3);

    (DoubleS r1, FloatS r2, DoubleS r3) opFloatDoubleS(FloatS p1, DoubleS p2);

    (StringS r1, StringS r2) opStringS(StringS p1, StringS p2);

    (ByteSS r1, ByteSS r2) opByteSS(ByteSS p1, ByteSS p2);

    (BoolSS r1, BoolSS r2) opBoolSS(BoolSS p1, BoolSS p2);

    (LongSS r1, ShortSS r2, IntSS r3, LongSS r4) opShortIntLongSS(ShortSS p1, IntSS p2, LongSS p3);

    (DoubleSS r1, FloatSS r2, DoubleSS r3) opFloatDoubleSS(FloatSS p1, DoubleSS p2);

    (StringSS r1, StringSS r2) opStringSS(StringSS p1, StringSS p2);

    (StringSSS r1, StringSSS r2) opStringSSS(StringSSS p1, StringSSS p2);

    (ByteBoolD r1, ByteBoolD r2) opByteBoolD(ByteBoolD p1, ByteBoolD p2);

    (ShortIntD r1, ShortIntD r2) opShortIntD(ShortIntD p1, ShortIntD p2);

    (LongFloatD r1, LongFloatD r2) opLongFloatD(LongFloatD p1, LongFloatD p2);

    (StringStringD r1, StringStringD r2) opStringStringD(StringStringD p1, StringStringD p2);

    (StringMyEnumD r1, StringMyEnumD r2) opStringMyEnumD(StringMyEnumD p1, StringMyEnumD p2);

    (MyEnumStringD r1, MyEnumStringD r2) opMyEnumStringD(MyEnumStringD p1, MyEnumStringD p2);

    (MyStructMyEnumD r1, MyStructMyEnumD r2) opMyStructMyEnumD(MyStructMyEnumD p1, MyStructMyEnumD p2);

    (ByteBoolDS r1, ByteBoolDS r2) opByteBoolDS(ByteBoolDS p1, ByteBoolDS p2);

    (ShortIntDS r1, ShortIntDS r2) opShortIntDS(ShortIntDS p1, ShortIntDS p2);

    (LongFloatDS r1, LongFloatDS r2) opLongFloatDS(LongFloatDS p1, LongFloatDS p2);

    (StringStringDS r1, StringStringDS r2) opStringStringDS(StringStringDS p1, StringStringDS p2);

    (StringMyEnumDS r1, StringMyEnumDS r2) opStringMyEnumDS(StringMyEnumDS p1, StringMyEnumDS p2);

    (MyEnumStringDS r1, MyEnumStringDS r2) opMyEnumStringDS(MyEnumStringDS p1, MyEnumStringDS p2);

    (MyStructMyEnumDS r1, MyStructMyEnumDS r2) opMyStructMyEnumDS(MyStructMyEnumDS p1, MyStructMyEnumDS p2);

    (ByteByteSD r1, ByteByteSD r2) opByteByteSD(ByteByteSD p1, ByteByteSD p2);

    (BoolBoolSD r1, BoolBoolSD r2) opBoolBoolSD(BoolBoolSD p1, BoolBoolSD p2);

    (ShortShortSD r1, ShortShortSD r2) opShortShortSD(ShortShortSD p1, ShortShortSD p2);

    (IntIntSD r1, IntIntSD r2) opIntIntSD(IntIntSD p1, IntIntSD p2);

    (LongLongSD r1, LongLongSD r2) opLongLongSD(LongLongSD p1, LongLongSD p2);

    (StringFloatSD r1, StringFloatSD r2) opStringFloatSD(StringFloatSD p1, StringFloatSD p2);

    (StringDoubleSD r1, StringDoubleSD r2) opStringDoubleSD(StringDoubleSD p1, StringDoubleSD p2);

    (StringStringSD r1, StringStringSD r2) opStringStringSD(StringStringSD p1, StringStringSD p2);

    (MyEnumMyEnumSD r1, MyEnumMyEnumSD r2) opMyEnumMyEnumSD(MyEnumMyEnumSD p1, MyEnumMyEnumSD p2);

    IntS opIntS(IntS s);

    void opByteSOneway(ByteS s);

    int opByteSOnewayCallCount();

    Ice::Context opContext();

    void opDoubleMarshaling(double p1, DoubleS p2);

    idempotent void opIdempotent();

    [nonmutating] idempotent void opNonmutating();

    byte opByte1(byte opByte1);
    short opShort1(short opShort1);
    int opInt1(int opInt1);
    long opLong1(long opLong1);
    float opFloat1(float opFloat1);
    double opDouble1(double opDouble1);
    string opString1(string opString1);
    StringS opStringS1(StringS opStringS1);
    ByteBoolD opByteBoolD1(ByteBoolD opByteBoolD1);
    StringS opStringS2(StringS stringS);
    ByteBoolD opByteBoolD2(ByteBoolD byteBoolD);

    StringS opStringLiterals();
    StringS opWStringLiterals();

    [marshaled-result] Structure opMStruct1();
    [marshaled-result] (Structure r1, Structure r2) opMStruct2(Structure p1);

    [marshaled-result] StringS opMSeq1();
    [marshaled-result] (StringS r1, StringS r2) opMSeq2(StringS p1);

    [marshaled-result] StringStringD opMDict1();
    [marshaled-result] (StringStringD r1, StringStringD r2) opMDict2(StringStringD p1);
}

struct MyStruct1
{
    string tesT; // Same name as the enclosing module
    MyClass* myClass; // Same name as an already defined class
}

class MyClass1
{
    string tesT; // Same name as the enclosing module
    MyClass* myClass; // Same name as an already defined class
}

interface MyDerivedClass : MyClass
{
    void opDerived();
    MyClass1 opMyClass1(MyClass1 opMyClass1);
    MyStruct1 opMyStruct1(MyStruct1 opMyStruct1);
}

//
// String literals
//

const string s0 = "\u005c";                           // backslash
const string s1 = "\u0041";                           // A
const string s2 = "\u0049\u0063\u0065";               // Ice
const string s3 = "\u004121";                         // A21
const string s4 = "\\u0041 \\U00000041";              // \\u0041 \\U00000041
const string s5 = "\u00FF";                           // ÿ
const string s6 = "\u03FF";                           // GREEK CAPITAL REVERSED DOTTED LUNATE SIGMA SYMBOL (U+03FF)
const string s7 = "\u05F0";                           // HEBREW LIGATURE YIDDISH DOUBLE VAV (U+05F0)
const string s8 = "\U00010000";                       // LINEAR B SYLLABLE B008 A (U+10000)
const string s9 = "\U0001F34C";                       // BANANA (U+1F34C)
const string s10 = "\u0DA7";                          // Sinhala Letter Alpapraana Ttayanna

const string sw0 = "\U0000005c";                      // backslash
const string sw1 = "\U00000041";                      // A
const string sw2 = "\U00000049\U00000063\U00000065";  // Ice
const string sw3 = "\U0000004121";                    // A21
const string sw4 = "\\u0041 \\U00000041";             // \\u0041 \\U00000041
const string sw5 = "\U000000FF";                      // ÿ
const string sw6 = "\U000003FF";                      // GREEK CAPITAL REVERSED DOTTED LUNATE SIGMA SYMBOL (U+03FF)
const string sw7 = "\U000005F0";                      // HEBREW LIGATURE YIDDISH DOUBLE VAV (U+05F0)
const string sw8 = "\U00010000";                      // LINEAR B SYLLABLE B008 A (U+10000)
const string sw9 = "\U0001F34C";                      // BANANA (U+1F34C)
const string sw10 = "\U00000DA7";                     // Sinhala Letter Alpapraana Ttayanna

/**
\'      single quote    byte 0x27 in ASCII encoding
\"      double quote    byte 0x22 in ASCII encoding
\?      question mark   byte 0x3f in ASCII encoding
\\      backslash       byte 0x5c in ASCII encoding
\a      audible bell    byte 0x07 in ASCII encoding
\b      backspace       byte 0x08 in ASCII encoding
\f      form feed - new page    byte 0x0c in ASCII encoding
\n      line feed - new line    byte 0x0a in ASCII encoding
\r      carriage return byte 0x0d in ASCII encoding
\t      horizontal tab  byte 0x09 in ASCII encoding
\v      vertical tab    byte 0x0b in ASCII encoding
**/

const string ss0 = "\'\"\?\\\a\b\f\n\r\t\v\6";
const string ss1 = "\u0027\u0022\u003f\u005c\u0007\u0008\u000c\u000a\u000d\u0009\u000b\u0006";
const string ss2 = "\U00000027\U00000022\U0000003f\U0000005c\U00000007\U00000008\U0000000c\U0000000a\U0000000d\U00000009\U0000000b\U00000006";

const string ss3 = "\\\\U\\u\\"; /* \\U\u\  */
const string ss4 = "\\\u0041\\"; /* \A\     */
const string ss5 = "\\u0041\\";  /* \u0041\ */

//
// Ĩ - Unicode Character 'LATIN CAPITAL LETTER I WITH TILDE' (U+0128)
// Ÿ - Unicode Character 'LATIN CAPITAL LETTER Y WITH DIAERESIS' (U+0178)
// ÿ - Unicode Character 'LATIN SMALL LETTER Y WITH DIAERESIS' (U+00FF)
// Ā - Unicode Character 'LATIN CAPITAL LETTER A WITH MACRON' (U+0100)
// ἀ - Unicode Character 'GREEK SMALL LETTER ALPHA WITH PSILI' (U+1F00)
// 𐆔 - Unicode Character 'ROMAN DIMIDIA SEXTULA SIGN' (U+10194)
// 𐅪 - Unicode Character 'GREEK ACROPHONIC THESPIAN ONE HUNDRED' (U+1016A)
// 𐆘 - Unicode Character 'ROMAN SESTERTIUS SIGN' (U+10198)
// 🍀 - Unicode Character 'FOUR LEAF CLOVER' (U+1F340)
// 🍁 - Unicode Character 'MAPLE LEAF' (U+1F341)
// 🍂 - Unicode Character 'FALLEN LEAF' (U+1F342)
// 🍃 - Unicode Character 'LEAF FLUTTERING IN WIND' (U+1F343)
//
const string su0 = "ĨŸÿĀἀ𐆔𐅪𐆘🍀🍁🍂🍃";
const string su1 = "\u0128\u0178\u00FF\u0100\u1F00\U00010194\U0001016A\U00010198\U0001F340\U0001F341\U0001F342\U0001F343";
const string su2 = "\U00000128\U00000178\U000000FF\U00000100\U00001F00\U00010194\U0001016A\U00010198\U0001F340\U0001F341\U0001F342\U0001F343";

}
