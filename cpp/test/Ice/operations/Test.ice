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

vector<byte> ByteV;
vector<bool> BoolV;
vector<short> ShortV;
vector<int> IntV;
vector<long> LongV;
vector<float> FloatV;
vector<double> DoubleV;
vector<string> StringV;
vector<wstring> WStringV;
vector<MyEnum> MyEnumV;
vector<MyClass*> MyClassV;

vector<ByteV> ByteVV;
vector<BoolV> BoolVV;
vector<ShortV> ShortVV;
vector<IntV> IntVV;
vector<LongV> LongVV;
vector<FloatV> FloatVV;
vector<DoubleV> DoubleVV;
vector<StringV> StringVV;
vector<WStringV> WStringVV;
vector<MyEnumV> MyEnumVV;
vector<MyClassV> MyClassVV;

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

    ByteV opByteV(ByteV p1, ByteV p2;
		  ByteV p3);

    BoolV opBoolV(BoolV p1, BoolV p2;
		  BoolV p3);	

    LongV opShortIntLongV(Test::ShortV p1, IntV p2, LongV p3;
			  ::Test::ShortV p4, IntV p5, LongV p6);

    DoubleV opFloatDoubleV(FloatV p1, DoubleV p2;
			   FloatV p3, DoubleV p4);
    
    StringV opStringV(StringV p1, StringV p2;
		      StringV p3);
    
    WStringV opWStringV(WStringV p1, WStringV p2;
			WStringV p3);
    
    ByteVV opByteVV(ByteVV p1, ByteVV p2;
		    ByteVV p3);
    
    BoolVV opBoolVV(BoolVV p1, BoolVV p2;
		    BoolVV p3);
    
    LongVV opShortIntLongVV(ShortVV p1, IntVV p2, LongVV p3;
			    ShortVV p4, IntVV p5, LongVV p6);
    
    
    DoubleVV opFloatDoubleVV(FloatVV p1, DoubleVV p2;
			     FloatVV p3, DoubleVV p4);
    
    StringVV opStringVV(StringVV p1, StringVV p2;
			StringVV p3);

    WStringVV opWStringVV(WStringVV p1, WStringVV p2;
			  WStringVV p3);

    void opEx(int p) throws byte, bool, short, int, long, float, double,
	string, wstring, ByteV, BoolV, ShortV, IntV, LongV, FloatV, DoubleV,
	StringV, WStringV, ByteVV, BoolVV, ShortVV, IntVV, LongVV, FloatVV,
	DoubleVV, StringVV, WStringVV, MyClass*;
    
/*
    bool bool_;
    short short_;
    int int_;
    long long_;
    float float_;
    double double_;
    string string_;
    wstring wstring_;

    ByteV ByteV_;
    BoolV BoolV_;
    ShortV ShortV_;
    IntV IntV_;
    LongV LongV_;
    FloatV FloatV_;
    DoubleV DoubleV_;
    StringV StringV_;
    WStringV WStringV_;

    ByteVV ByteVV_;
    BoolVV BoolVV_;
    ShortVV ShortVV_;
    IntVV IntVV_;
    LongVV LongVV_;
    FloatVV FloatVV_;
    DoubleVV DoubleVV_;
    StringVV StringVV_;
    WStringVV WStringVV_;
*/
};

class MyDerivedClass extends MyClass
{
    void opDerived();
};

};

#endif
