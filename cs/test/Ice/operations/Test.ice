// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

#include <Ice/Current.ice>

module Test
{

enum MyEnum
{
    enum1,
    enum2,
    enum3
};

class MyClass;

struct AnotherStruct
{
    string s;
};

struct Structure
{
    MyClass* p;
    MyEnum e;
    AnotherStruct s;
};

["cs:collection"] sequence<byte> ByteS;
["cs:collection"] sequence<bool> BoolS;
["cs:collection"] sequence<short> ShortS;
["cs:collection"] sequence<int> IntS;
["cs:collection"] sequence<long> LongS;
["cs:collection"] sequence<float> FloatS;
["cs:collection"] sequence<double> DoubleS;
["cs:collection"] sequence<string> StringS;
["cs:collection"] sequence<MyEnum> MyEnumS;
["cs:collection"] sequence<MyClass*> MyClassS;

["cs:collection"] sequence<ByteS> ByteSS;
["cs:collection"] sequence<BoolS> BoolSS;
["cs:collection"] sequence<ShortS> ShortSS;
["cs:collection"] sequence<IntS> IntSS;
["cs:collection"] sequence<LongS> LongSS;
["cs:collection"] sequence<FloatS> FloatSS;
["cs:collection"] sequence<DoubleS> DoubleSS;
["cs:collection"] sequence<StringS> StringSS;
["cs:collection"] sequence<MyEnumS> MyEnumSS;
["cs:collection"] sequence<MyClassS> MyClassSS;

sequence<StringSS> StringSSS;

dictionary<byte, bool> ByteBoolD;
dictionary<short, int> ShortIntD;
dictionary<long, float> LongFloatD;
dictionary<string, string> StringStringD;
dictionary<string, MyEnum> StringMyEnumD;

["ami"] class MyClass
{
    void shutdown();

    void opVoid();
    
    byte opByte(byte p1, byte p2,
		out byte p3);

    bool opBool(bool p1, bool p2,
		out bool p3);

    long opShortIntLong(short p1, int p2, long p3,
			out short p4, out int p5, out long p6);

    double opFloatDouble(float p1, double p2,
			 out float p3, out double p4);

    string opString(string p1, string p2,
		    out string p3);

    MyEnum opMyEnum(MyEnum p1, out MyEnum p2);

    MyClass* opMyClass(MyClass* p1, out MyClass* p2, out MyClass* p3);

    Structure opStruct(Structure p1, Structure p2,
		       out Structure p3);

    ByteS opByteS(ByteS p1, ByteS p2,
		  out ByteS p3);

    BoolS opBoolS(BoolS p1, BoolS p2,
		  out BoolS p3);	

    LongS opShortIntLongS(Test::ShortS p1, IntS p2, LongS p3,
			  out ::Test::ShortS p4, out IntS p5, out LongS p6);

    DoubleS opFloatDoubleS(FloatS p1, DoubleS p2,
			   out FloatS p3, out DoubleS p4);
    
    StringS opStringS(StringS p1, StringS p2,
		      out StringS p3);
    
    ByteSS opByteSS(ByteSS p1, ByteSS p2,
		    out ByteSS p3);
    
    BoolSS opBoolSS(BoolSS p1, BoolSS p2,
		    out BoolSS p3);
    
    LongSS opShortIntLongSS(ShortSS p1, IntSS p2, LongSS p3,
			    out ShortSS p4, out IntSS p5, out LongSS p6);
    
    
    DoubleSS opFloatDoubleSS(FloatSS p1, DoubleSS p2,
			     out FloatSS p3, out DoubleSS p4);
    
    StringSS opStringSS(StringSS p1, StringSS p2,
			out StringSS p3);

    StringSSS opStringSSS(StringSSS p1, StringSSS p2,
    			  out StringSSS p3);

    ByteBoolD opByteBoolD(ByteBoolD p1, ByteBoolD p2,
			  out ByteBoolD p3);

    ShortIntD opShortIntD(ShortIntD p1, ShortIntD p2,
			  out ShortIntD p3);

    LongFloatD opLongFloatD(LongFloatD p1, LongFloatD p2,
			    out LongFloatD p3);

    StringStringD opStringStringD(StringStringD p1, StringStringD p2,
				  out StringStringD p3);

    StringMyEnumD opStringMyEnumD(StringMyEnumD p1, StringMyEnumD p2,
				  out StringMyEnumD p3);

    IntS opIntS(IntS s);

    Ice::Context opContext();

};

["ami"] class MyDerivedClass extends MyClass
{
    void opDerived();
};

interface TestCheckedCast
{
    Ice::Context getContext();
};

};

#endif
