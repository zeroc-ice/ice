// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

module Test
{

enum MyEnum
{
    enum1,
    enum2,
    enum3
};

class MyClass;

sequence<byte> ByteS;
sequence<bool> BoolS;
sequence<short> ShortS;
sequence<int> IntS;
sequence<long> LongS;
sequence<float> FloatS;
sequence<double> DoubleS;
sequence<string> StringS;
sequence<wstring> WStringS;
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
sequence<WStringS> WStringSS;
sequence<MyEnumS> MyEnumSS;
sequence<MyClassS> MyClassSS;

dictionary<byte, bool> ByteBoolD;
dictionary<short, int> ShortIntD;
dictionary<long, float> LongFloatD;
dictionary<double, string> DoubleStringD;
dictionary<wstring, MyEnum> WStringMyEnumD;
dictionary<MyClass*, string> MyClassStringD;

class MyClass
{
    void shutdown();
 
    void opVoid();
    
    byte opByte(byte p1, byte p2;
		byte p3);

    bool opBool(bool p1, bool p2;
		bool p3);

    long opShortIntLong(short p1, int p2, long p3;
			short p4, int p5, long p6);

    double opFloatDouble(float p1, double p2;
			 float p3, double p4);

    string opString(string p1, string p2;
		    string p3);

    wstring opWString(wstring p1, wstring p2;
		      wstring p3);

    MyEnum opMyEnum(MyEnum p1; MyEnum p2);

    MyClass* opMyClass(MyClass* p1; MyClass* p2);

    ByteS opByteS(ByteS p1, ByteS p2;
		  ByteS p3);

    BoolS opBoolS(BoolS p1, BoolS p2;
		  BoolS p3);	

    LongS opShortIntLongS(Test::ShortS p1, IntS p2, LongS p3;
			  ::Test::ShortS p4, IntS p5, LongS p6);

    DoubleS opFloatDoubleS(FloatS p1, DoubleS p2;
			   FloatS p3, DoubleS p4);
    
    StringS opStringS(StringS p1, StringS p2;
		      StringS p3);
    
    WStringS opWStringS(WStringS p1, WStringS p2;
			WStringS p3);
    
    ByteSS opByteSS(ByteSS p1, ByteSS p2;
		    ByteSS p3);
    
    BoolSS opBoolSS(BoolSS p1, BoolSS p2;
		    BoolSS p3);
    
    LongSS opShortIntLongSS(ShortSS p1, IntSS p2, LongSS p3;
			    ShortSS p4, IntSS p5, LongSS p6);
    
    
    DoubleSS opFloatDoubleSS(FloatSS p1, DoubleSS p2;
			     FloatSS p3, DoubleSS p4);
    
    StringSS opStringSS(StringSS p1, StringSS p2;
			StringSS p3);

    WStringSS opWStringSS(WStringSS p1, WStringSS p2;
			  WStringSS p3);

    ByteBoolD opByteBoolD(ByteBoolD p1, ByteBoolD p2;
			  ByteBoolD p3);

    ShortIntD opShortIntD(ShortIntD p1, ShortIntD p2;
			  ShortIntD p3);

    LongFloatD opLongFloatD(LongFloatD p1, LongFloatD p2;
			    LongFloatD p3);

    DoubleStringD opDoubleStringD(DoubleStringD p1, DoubleStringD p2;
				  DoubleStringD p3);

    WStringMyEnumD opWStringMyEnumD(WStringMyEnumD p1, WStringMyEnumD p2;
				    WStringMyEnumD p3);

    MyClassStringD opMyClassStringD(MyClassStringD p1, MyClassStringD p2;
				    MyClassStringD p3);

    void opEx(int p) throws
        byte,
	bool,
	short,
	int,
	long,
	float,
	double,
	string,
	wstring,
	ByteS,
	BoolS,
	ShortS,
	IntS,
	LongS,
	FloatS,
	DoubleS,
	StringS,
	WStringS,
	ByteSS,
	BoolSS,
	ShortSS,
	IntSS,
	LongSS,
	FloatSS,
	DoubleSS,
	StringSS,
	WStringSS,
	ByteBoolD,
	ShortIntD,
	LongFloatD,
	DoubleStringD,
	WStringMyEnumD,
	MyClassStringD,
	MyClass*;
};

class MyDerivedClass extends MyClass
{
    void opDerived();
};

};

#endif
