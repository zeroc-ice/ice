// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

    virtual Test::StringSSS opStringSSS(const Test::StringSSS&,
                                        const Test::StringSSS&,
                                        Test::StringSSS&,
                                        const ::Ice::Current&);

    virtual Test::ByteBoolD opByteBoolD(const Test::ByteBoolD&, const Test::ByteBoolD&, 
                                        Test::ByteBoolD&,
                                        const Ice::Current&);

    virtual Test::ShortIntD opShortIntD(const Test::ShortIntD&, const Test::ShortIntD&,
                                        Test::ShortIntD&,
                                        const Ice::Current&);

    virtual Test::LongFloatD opLongFloatD(const Test::LongFloatD&, const Test::LongFloatD&,
                                          Test::LongFloatD&,
                                          const Ice::Current&);

    virtual Test::StringStringD opStringStringD(const Test::StringStringD&, const Test::StringStringD&,
                                                Test::StringStringD&,
                                                const Ice::Current&);

    virtual Test::StringMyEnumD opStringMyEnumD(const Test::StringMyEnumD&, const Test::StringMyEnumD&,
                                                Test::StringMyEnumD&,
                                                const Ice::Current&);

    virtual Test::IntS opIntS(const Test::IntS&, const Ice::Current&);

    virtual void opByteSOneway(const Test::ByteS&, const Ice::Current&);

    virtual Ice::Context opContext(const Ice::Current&);

    virtual void opDoubleMarshaling(Ice::Double, const Test::DoubleS&, const Ice::Current&);

    virtual void opDerived(const Ice::Current&);
};

#endif
