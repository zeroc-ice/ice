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

    virtual Test::ByteV opByteV(const Test::ByteV&,
				const Test::ByteV&,
				Test::ByteV&);
    
    virtual Test::BoolV opBoolV(const Test::BoolV&,
				const Test::BoolV&,
				Test::BoolV&);
    
    virtual Test::LongV opShortIntLongV(const Test::ShortV&,
					const Test::IntV&,
					const Test::LongV&,
					Test::ShortV&,
					Test::IntV&,
					Test::LongV&);
    
    virtual Test::DoubleV opFloatDoubleV(const Test::FloatV&,
					 const Test::DoubleV&,
					 Test::FloatV&,
					 Test::DoubleV&);
    
    virtual Test::StringV opStringV(const Test::StringV&,
				    const Test::StringV&,
				    Test::StringV&);
    
    virtual Test::WStringV opWStringV(const Test::WStringV&,
				      const Test::WStringV&,
				      Test::WStringV&);
    
    virtual Test::ByteVV opByteVV(const Test::ByteVV&,
				  const Test::ByteVV&,
				  Test::ByteVV&);
    
    virtual Test::BoolVV opBoolVV(const Test::BoolVV&,
				  const Test::BoolVV&,
				  Test::BoolVV&);
    
    virtual Test::LongVV opShortIntLongVV(const Test::ShortVV&,
					  const Test::IntVV&,
					  const Test::LongVV&,
					  Test::ShortVV&,
					  Test::IntVV&,
					  Test::LongVV&);
    
    virtual Test::DoubleVV opFloatDoubleVV(const Test::FloatVV&,
					   const Test::DoubleVV&,
					   Test::FloatVV&,
					   Test::DoubleVV&);
    
    virtual Test::StringVV opStringVV(const Test::StringVV&,
				      const Test::StringVV&,
				      Test::StringVV&);

    virtual Test::WStringVV opWStringVV(const Test::WStringVV&,
					const Test::WStringVV&,
					Test::WStringVV&);
    
    virtual void opEx(Ice::Int);

    virtual void opDerived();

private:

    Ice::ObjectAdapterPtr _adapter;
};

#endif
