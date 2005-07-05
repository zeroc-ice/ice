// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class MyDerivedClassI : public Test::MyDerivedClass
{
public:

    MyDerivedClassI(const IceE::ObjectAdapterPtr&, const IceE::Identity&);

    virtual void shutdown(const IceE::Current&);

    virtual void opVoid(const IceE::Current&);

    virtual IceE::Byte opByte(IceE::Byte,
			     IceE::Byte,
			     IceE::Byte&,
			     const IceE::Current&);
    
    virtual bool opBool(bool,
			bool,
			bool&,
			const IceE::Current&);
    
    virtual IceE::Long opShortIntLong(IceE::Short,
				     IceE::Int,
				     IceE::Long,
				     IceE::Short&,
				     IceE::Int&,
				     IceE::Long&,
				     const IceE::Current&);
    
    virtual IceE::Double opFloatDouble(IceE::Float,
				      IceE::Double,
				      IceE::Float&,
				      IceE::Double&,
				      const IceE::Current&);
    
    virtual std::string opString(const std::string&,
				 const std::string&,
				 std::string&,
				 const IceE::Current&);
    
    virtual Test::MyEnum opMyEnum(Test::MyEnum,
				  Test::MyEnum&,
				  const IceE::Current&);

    virtual Test::MyClassPrx opMyClass(const Test::MyClassPrx&,
				       Test::MyClassPrx&, Test::MyClassPrx&,
				       const IceE::Current&);

    virtual Test::Structure opStruct(const Test::Structure&, const Test::Structure&,
				  Test::Structure&,
				  const IceE::Current&);

    virtual Test::ByteS opByteS(const Test::ByteS&,
				const Test::ByteS&,
				Test::ByteS&,
				const IceE::Current&);
    
    virtual Test::BoolS opBoolS(const Test::BoolS&,
				const Test::BoolS&,
				Test::BoolS&,
				const IceE::Current&);
    
    virtual Test::LongS opShortIntLongS(const Test::ShortS&,
					const Test::IntS&,
					const Test::LongS&,
					Test::ShortS&,
					Test::IntS&,
					Test::LongS&,
					const IceE::Current&);
    
    virtual Test::DoubleS opFloatDoubleS(const Test::FloatS&,
					 const Test::DoubleS&,
					 Test::FloatS&,
					 Test::DoubleS&,
					 const IceE::Current&);
    
    virtual Test::StringS opStringS(const Test::StringS&,
				    const Test::StringS&,
				    Test::StringS&,
				    const IceE::Current&);
    
    virtual Test::ByteSS opByteSS(const Test::ByteSS&,
				  const Test::ByteSS&,
				  Test::ByteSS&,
				  const IceE::Current&);
    
    virtual Test::BoolSS opBoolSS(const Test::BoolSS&,
				  const Test::BoolSS&,
				  Test::BoolSS&,
				  const IceE::Current&);
    
    virtual Test::LongSS opShortIntLongSS(const Test::ShortSS&,
					  const Test::IntSS&,
					  const Test::LongSS&,
					  Test::ShortSS&,
					  Test::IntSS&,
					  Test::LongSS&,
					  const IceE::Current&);
    
    virtual Test::DoubleSS opFloatDoubleSS(const Test::FloatSS&,
					   const Test::DoubleSS&,
					   Test::FloatSS&,
					   Test::DoubleSS&,
					   const IceE::Current&);
    
    virtual Test::StringSS opStringSS(const Test::StringSS&,
				      const Test::StringSS&,
				      Test::StringSS&,
				      const IceE::Current&);

    virtual Test::StringSSS opStringSSS(const Test::StringSSS&,
	                                const Test::StringSSS&,
					Test::StringSSS&,
					const ::IceE::Current&);

    virtual Test::ByteBoolD opByteBoolD(const Test::ByteBoolD&, const Test::ByteBoolD&, 
					Test::ByteBoolD&,
					const IceE::Current&);

    virtual Test::ShortIntD opShortIntD(const Test::ShortIntD&, const Test::ShortIntD&,
					Test::ShortIntD&,
					const IceE::Current&);

    virtual Test::LongFloatD opLongFloatD(const Test::LongFloatD&, const Test::LongFloatD&,
					  Test::LongFloatD&,
					  const IceE::Current&);

    virtual Test::StringStringD opStringStringD(const Test::StringStringD&, const Test::StringStringD&,
						Test::StringStringD&,
						const IceE::Current&);

    virtual Test::StringMyEnumD opStringMyEnumD(const Test::StringMyEnumD&, const Test::StringMyEnumD&,
						Test::StringMyEnumD&,
						const IceE::Current&);

    virtual Test::IntS opIntS(const Test::IntS&, const IceE::Current&);

    virtual void opByteSOneway(const Test::ByteS&, const IceE::Current&);

    virtual IceE::Context opContext(const IceE::Current&);

    virtual void opDerived(const IceE::Current&);

private:

    IceE::ObjectAdapterPtr _adapter;
    IceE::Identity _identity;
};

class TestCheckedCastI : public Test::TestCheckedCast
{
public:

    virtual IceE::Context getContext(const IceE::Current&);
    void setContext(const IceE::Context& c);

private:

    IceE::Context _ctx;
};

typedef IceE::Handle<TestCheckedCastI> TestCheckedCastIPtr;

#endif
