// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
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

    MyDerivedClassI(const Ice::ObjectAdapterPtr&, const Ice::Identity&);

    virtual void shutdown(const Ice::Current&);

    virtual void opVoid(const Ice::Current&);

    virtual Ice::Byte opByte(Ice::Byte,
			     Ice::Byte,
			     Ice::Byte&,
			     const Ice::Current&);
    
    virtual bool opBool(bool,
			bool,
			bool&,
			const Ice::Current&);
    
    virtual Ice::Long opShortIntLong(Ice::Short,
				     Ice::Int,
				     Ice::Long,
				     Ice::Short&,
				     Ice::Int&,
				     Ice::Long&,
				     const Ice::Current&);
    
    virtual Ice::Double opFloatDouble(Ice::Float,
				      Ice::Double,
				      Ice::Float&,
				      Ice::Double&,
				      const Ice::Current&);
    
    virtual std::string opString(const std::string&,
				 const std::string&,
				 std::string&,
				 const Ice::Current&);
    
    virtual Test::MyEnum opMyEnum(Test::MyEnum,
				  Test::MyEnum&,
				  const Ice::Current&);

    virtual Test::MyClassPrx opMyClass(const Test::MyClassPrx&,
				       Test::MyClassPrx&, Test::MyClassPrx&,
				       const Ice::Current&);

    virtual Test::Structure opStruct(const Test::Structure&, const Test::Structure&,
				  Test::Structure&,
				  const Ice::Current&);

    virtual Test::ByteS opByteS(const Test::ByteS&,
				const Test::ByteS&,
				Test::ByteS&,
				const Ice::Current&);
    
    virtual Test::BoolS opBoolS(const Test::BoolS&,
				const Test::BoolS&,
				Test::BoolS&,
				const Ice::Current&);
    
    virtual Test::LongS opShortIntLongS(const Test::ShortS&,
					const Test::IntS&,
					const Test::LongS&,
					Test::ShortS&,
					Test::IntS&,
					Test::LongS&,
					const Ice::Current&);
    
    virtual Test::DoubleS opFloatDoubleS(const Test::FloatS&,
					 const Test::DoubleS&,
					 Test::FloatS&,
					 Test::DoubleS&,
					 const Ice::Current&);
    
    virtual Test::StringS opStringS(const Test::StringS&,
				    const Test::StringS&,
				    Test::StringS&,
				    const Ice::Current&);
    
    virtual Test::ByteSS opByteSS(const Test::ByteSS&,
				  const Test::ByteSS&,
				  Test::ByteSS&,
				  const Ice::Current&);
    
    virtual Test::BoolSS opBoolSS(const Test::BoolSS&,
				  const Test::BoolSS&,
				  Test::BoolSS&,
				  const Ice::Current&);
    
    virtual Test::LongSS opShortIntLongSS(const Test::ShortSS&,
					  const Test::IntSS&,
					  const Test::LongSS&,
					  Test::ShortSS&,
					  Test::IntSS&,
					  Test::LongSS&,
					  const Ice::Current&);
    
    virtual Test::DoubleSS opFloatDoubleSS(const Test::FloatSS&,
					   const Test::DoubleSS&,
					   Test::FloatSS&,
					   Test::DoubleSS&,
					   const Ice::Current&);
    
    virtual Test::StringSS opStringSS(const Test::StringSS&,
				      const Test::StringSS&,
				      Test::StringSS&,
				      const Ice::Current&);

    virtual Test::ByteBoolD opByteBoolD(const Test::ByteBoolD&, const Test::ByteBoolD&, 
					Test::ByteBoolD&,
					const Ice::Current&);

    virtual Test::ShortIntD opShortIntD(const Test::ShortIntD&, const Test::ShortIntD&,
					Test::ShortIntD&,
					const Ice::Current&);

    virtual Test::LongFloatD opLongFloatD(const Test::LongFloatD&, const Test::LongFloatD&,
					  Test::LongFloatD&,
					  const Ice::Current&);

    virtual Test::DoubleStringD opDoubleStringD(const Test::DoubleStringD&, const Test::DoubleStringD&,
						Test::DoubleStringD&,
						const Ice::Current&);

    virtual Test::StringStringD opStringStringD(const Test::StringStringD&, const Test::StringStringD&,
						Test::StringStringD&,
						const Ice::Current&);

    virtual Test::StringMyEnumD opStringMyEnumD(const Test::StringMyEnumD&, const Test::StringMyEnumD&,
						Test::StringMyEnumD&,
						const Ice::Current&);

    virtual Test::MyClassStringD opMyClassStringD(const Test::MyClassStringD&, const Test::MyClassStringD&,
						  Test::MyClassStringD&,
						  const Ice::Current&);

    virtual void opDerived(const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _adapter;
    Ice::Identity _identity;
};

#endif
