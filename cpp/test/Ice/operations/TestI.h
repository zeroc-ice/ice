// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class MyDerivedClassI : public Test::MyDerivedClass
{
public:

    MyDerivedClassI(const Ice::ObjectAdapterPtr&);

    virtual void shutdown();

    virtual void opVoid();

    virtual Ice::Byte opByte(Ice::Byte,
			     Ice::Byte,
			     Ice::Byte&);
    
    virtual bool opBool(bool,
			bool,
			bool&);
    
    virtual Ice::Long opShortIntLong(Ice::Short,
				     Ice::Int,
				     Ice::Long,
				     Ice::Short&,
				     Ice::Int&,
				     Ice::Long&);
    
    virtual Ice::Double opFloatDouble(Ice::Float,
				      Ice::Double,
				      Ice::Float&,
				      Ice::Double&);
    
    virtual std::string opString(const std::string&,
				 const std::string&,
				 std::string&);
    
    virtual std::wstring opWString(const std::wstring&,
				   const std::wstring&,
				   std::wstring&);
    
    virtual Test::MyEnum opMyEnum(Test::MyEnum,
				  Test::MyEnum&);

    virtual Test::MyClassPrx opMyClass(const Test::MyClassPrx&,
				       Test::MyClassPrx&);

    virtual Test::ByteS opByteS(const Test::ByteS&,
				const Test::ByteS&,
				Test::ByteS&);
    
    virtual Test::BoolS opBoolS(const Test::BoolS&,
				const Test::BoolS&,
				Test::BoolS&);
    
    virtual Test::LongS opShortIntLongS(const Test::ShortS&,
					const Test::IntS&,
					const Test::LongS&,
					Test::ShortS&,
					Test::IntS&,
					Test::LongS&);
    
    virtual Test::DoubleS opFloatDoubleS(const Test::FloatS&,
					 const Test::DoubleS&,
					 Test::FloatS&,
					 Test::DoubleS&);
    
    virtual Test::StringS opStringS(const Test::StringS&,
				    const Test::StringS&,
				    Test::StringS&);
    
    virtual Test::WStringS opWStringS(const Test::WStringS&,
				      const Test::WStringS&,
				      Test::WStringS&);
    
    virtual Test::ByteSS opByteSS(const Test::ByteSS&,
				  const Test::ByteSS&,
				  Test::ByteSS&);
    
    virtual Test::BoolSS opBoolSS(const Test::BoolSS&,
				  const Test::BoolSS&,
				  Test::BoolSS&);
    
    virtual Test::LongSS opShortIntLongSS(const Test::ShortSS&,
					  const Test::IntSS&,
					  const Test::LongSS&,
					  Test::ShortSS&,
					  Test::IntSS&,
					  Test::LongSS&);
    
    virtual Test::DoubleSS opFloatDoubleSS(const Test::FloatSS&,
					   const Test::DoubleSS&,
					   Test::FloatSS&,
					   Test::DoubleSS&);
    
    virtual Test::StringSS opStringSS(const Test::StringSS&,
				      const Test::StringSS&,
				      Test::StringSS&);

    virtual Test::WStringSS opWStringSS(const Test::WStringSS&,
					const Test::WStringSS&,
					Test::WStringSS&);
    
    virtual void opEx(Ice::Int);

    virtual void opDerived();

private:

    Ice::ObjectAdapterPtr _adapter;
};

#endif
