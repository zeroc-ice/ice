// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestAMDI.h>
#include <TestCommon.h>
#include <functional>
#include <iterator>

using namespace std;

class Thread_opVoid : public IceUtil::Thread
{
public:

    Thread_opVoid(const Test::AMD_MyClass_opVoidPtr& cb) :
        _cb(cb)
    {
    }

    virtual void run()
    {
        _cb->ice_response();
    }

private:

    const Test::AMD_MyClass_opVoidPtr _cb;
};

MyDerivedClassI::MyDerivedClassI() : _opByteSOnewayCallCount(0)
{
}

bool
MyDerivedClassI::ice_isA(const std::string& id, const Ice::Current& current) const
{
    test(current.mode == Ice::Nonmutating);
    return Test::MyDerivedClass::ice_isA(id, current);
}

void
MyDerivedClassI::ice_ping(const Ice::Current& current) const
{
    test(current.mode == Ice::Nonmutating);
    Test::MyDerivedClass::ice_ping(current);
}

std::vector<std::string>
MyDerivedClassI::ice_ids(const Ice::Current& current) const
{
    test(current.mode == Ice::Nonmutating);
    return Test::MyDerivedClass::ice_ids(current);
}

const std::string&
MyDerivedClassI::ice_id(const Ice::Current& current) const
{
    test(current.mode == Ice::Nonmutating);
    return Test::MyDerivedClass::ice_id(current);
}

void
MyDerivedClassI::shutdown_async(const Test::AMD_MyClass_shutdownPtr& cb, const Ice::Current& current)
{
    {
        IceUtil::Mutex::Lock sync(_opVoidMutex);
        if(_opVoidThread)
        {
            _opVoidThread->getThreadControl().join();
            _opVoidThread = 0;
        }
    }

    current.adapter->getCommunicator()->shutdown();
    cb->ice_response();
}

void
MyDerivedClassI::delay_async(const Test::AMD_MyClass_delayPtr& cb, Ice::Int ms, const Ice::Current&)
{
    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(ms));
    cb->ice_response();
}

void
MyDerivedClassI::opVoid_async(const Test::AMD_MyClass_opVoidPtr& cb, const Ice::Current& current)
{
    test(current.mode == Ice::Normal);

    IceUtil::Mutex::Lock sync(_opVoidMutex);
    if(_opVoidThread)
    {
        _opVoidThread->getThreadControl().join();
        _opVoidThread = 0;
    }

    _opVoidThread = new Thread_opVoid(cb);
    _opVoidThread->start();
}

void
MyDerivedClassI::opByte_async(const Test::AMD_MyClass_opBytePtr& cb,
                              Ice::Byte p1,
                              Ice::Byte p2,
                              const Ice::Current&)
{
    cb->ice_response(p1, p1 ^ p2);
}

void
MyDerivedClassI::opBool_async(const Test::AMD_MyClass_opBoolPtr& cb,
                              bool p1,
                              bool p2,
                              const Ice::Current&)
{
    cb->ice_response(p2, p1);
}

void
MyDerivedClassI::opShortIntLong_async(const Test::AMD_MyClass_opShortIntLongPtr& cb,
                                      Ice::Short p1,
                                      Ice::Int p2,
                                      Ice::Long p3,
                                      const Ice::Current&)
{
    cb->ice_response(p3, p1, p2, p3);
}

void
MyDerivedClassI::opFloatDouble_async(const Test::AMD_MyClass_opFloatDoublePtr& cb,
                                     Ice::Float p1,
                                     Ice::Double p2,
                                     const Ice::Current&)
{
    cb->ice_response(p2, p1, p2);
}

void
MyDerivedClassI::opString_async(const Test::AMD_MyClass_opStringPtr& cb,
                                const std::string& p1,
                                const std::string& p2,
                                const Ice::Current&)
{
    cb->ice_response(p1 + " " + p2, p2 + " " + p1);
}

void
MyDerivedClassI::opMyEnum_async(const Test::AMD_MyClass_opMyEnumPtr& cb,
                                Test::MyEnum p1,
                                const Ice::Current&)
{
    cb->ice_response(Test::enum3, p1);
}

void
MyDerivedClassI::opMyClass_async(const Test::AMD_MyClass_opMyClassPtr& cb,
                                 const Test::MyClassPrx& p1,
                                 const Ice::Current& current)
{
    Test::MyClassPrx p2 = p1;
    Test::MyClassPrx p3 = Test::MyClassPrx::uncheckedCast(current.adapter->createProxy(
                                        current.adapter->getCommunicator()->stringToIdentity("noSuchIdentity")));
    cb->ice_response(Test::MyClassPrx::uncheckedCast(current.adapter->createProxy(current.id)), p2, p3);
}

void
MyDerivedClassI::opStruct_async(const Test::AMD_MyClass_opStructPtr& cb,
                                const Test::Structure& p1,
                                const Test::Structure& p2,
                                const Ice::Current&)
{
    Test::Structure p3 = p1;
    p3.s.s = "a new string";
    cb->ice_response(p2, p3);
}

void
MyDerivedClassI::opByteS_async(const Test::AMD_MyClass_opByteSPtr& cb,
                               const Test::ByteS& p1,
                               const Test::ByteS& p2,
                               const Ice::Current&)
{
    Test::ByteS p3;
    p3.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), p3.begin());
    Test::ByteS r = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(r));
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opBoolS_async(const Test::AMD_MyClass_opBoolSPtr& cb,
                               const Test::BoolS& p1,
                               const Test::BoolS& p2,
                               const Ice::Current&)
{
    Test::BoolS p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    Test::BoolS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opShortIntLongS_async(const Test::AMD_MyClass_opShortIntLongSPtr& cb,
                                       const Test::ShortS& p1,
                                       const Test::IntS& p2,
                                       const Test::LongS& p3,
                                       const Ice::Current&)
{
    Test::ShortS p4 = p1;
    Test::IntS p5;
    p5.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), p5.begin());
    Test::LongS p6 = p3;
    std::copy(p3.begin(), p3.end(), std::back_inserter(p6));
    cb->ice_response(p3, p4, p5, p6);
}

void
MyDerivedClassI::opFloatDoubleS_async(const Test::AMD_MyClass_opFloatDoubleSPtr& cb,
                                      const Test::FloatS& p1,
                                      const Test::DoubleS& p2,
                                      const Ice::Current&)
{
    Test::FloatS p3 = p1;
    Test::DoubleS p4;
    p4.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), p4.begin());
    Test::DoubleS r = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(r));
    cb->ice_response(r, p3, p4);
}

void
MyDerivedClassI::opStringS_async(const Test::AMD_MyClass_opStringSPtr& cb,
                                 const Test::StringS& p1,
                                 const Test::StringS& p2,
                                 const Ice::Current&)
{
    Test::StringS p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    Test::StringS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opByteSS_async(const Test::AMD_MyClass_opByteSSPtr& cb,
                                const Test::ByteSS& p1,
                                const Test::ByteSS& p2,
                                const Ice::Current&)
{
    Test::ByteSS p3;
    p3.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), p3.begin());
    Test::ByteSS r = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(r));
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opBoolSS_async(const Test::AMD_MyClass_opBoolSSPtr& cb,
                                const Test::BoolSS& p1,
                                const Test::BoolSS& p2,
                                const Ice::Current&)
{
    Test::BoolSS p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    Test::BoolSS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opShortIntLongSS_async(const Test::AMD_MyClass_opShortIntLongSSPtr& cb,
                                        const Test::ShortSS& p1,
                                        const Test::IntSS& p2,
                                        const Test::LongSS& p3,
                                        const Ice::Current&)
{
    Test::ShortSS p4 = p1;
    Test::IntSS p5;
    p5.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), p5.begin());
    Test::LongSS p6 = p3;
    std::copy(p3.begin(), p3.end(), std::back_inserter(p6));
    cb->ice_response(p3, p4, p5, p6);
}

void
MyDerivedClassI::opFloatDoubleSS_async(const Test::AMD_MyClass_opFloatDoubleSSPtr& cb,
                                       const Test::FloatSS& p1,
                                       const Test::DoubleSS& p2,
                                       const Ice::Current&)
{
    Test::FloatSS p3 = p1;
    Test::DoubleSS p4;
    p4.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), p4.begin());
    Test::DoubleSS r = p2;
    std::copy(p2.begin(), p2.end(), std::back_inserter(r));
    cb->ice_response(r, p3, p4);
}

void
MyDerivedClassI::opStringSS_async(const Test::AMD_MyClass_opStringSSPtr& cb,
                                  const Test::StringSS& p1,
                                  const Test::StringSS& p2,
                                  const Ice::Current&)
{
    Test::StringSS p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    Test::StringSS r;
    r.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), r.begin());
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opStringSSS_async(const Test::AMD_MyClass_opStringSSSPtr& cb,
                                  const Test::StringSSS& p1,
                                  const Test::StringSSS& p2,
                                  const Ice::Current&)
{
    Test::StringSSS p3 = p1;
    std::copy(p2.begin(), p2.end(), std::back_inserter(p3));
    Test::StringSSS r;
    r.resize(p2.size());
    std::reverse_copy(p2.begin(), p2.end(), r.begin());
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opByteBoolD_async(const Test::AMD_MyClass_opByteBoolDPtr& cb,
                                   const Test::ByteBoolD& p1,
                                   const Test::ByteBoolD& p2,
                                   const Ice::Current&)
{
    Test::ByteBoolD p3 = p1;
    Test::ByteBoolD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opShortIntD_async(const Test::AMD_MyClass_opShortIntDPtr& cb,
                                   const Test::ShortIntD& p1,
                                   const Test::ShortIntD& p2,
                                   const Ice::Current&)
{
    Test::ShortIntD p3 = p1;
    Test::ShortIntD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opLongFloatD_async(const Test::AMD_MyClass_opLongFloatDPtr& cb,
                                    const Test::LongFloatD& p1,
                                    const Test::LongFloatD& p2,
                                    const Ice::Current&)
{
    Test::LongFloatD p3 = p1;
    Test::LongFloatD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opStringStringD_async(const Test::AMD_MyClass_opStringStringDPtr& cb,
                                       const Test::StringStringD& p1,
                                       const Test::StringStringD& p2,
                                       const Ice::Current&)
{
    Test::StringStringD p3 = p1;
    Test::StringStringD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opStringMyEnumD_async(const Test::AMD_MyClass_opStringMyEnumDPtr& cb,
                                       const Test::StringMyEnumD& p1,
                                       const Test::StringMyEnumD& p2,
                                       const Ice::Current&)
{
    Test::StringMyEnumD p3 = p1;
    Test::StringMyEnumD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opMyEnumStringD_async(const Test::AMD_MyClass_opMyEnumStringDPtr& cb,
                                       const Test::MyEnumStringD& p1,
                                       const Test::MyEnumStringD& p2,
                                       const Ice::Current&)
{
    Test::MyEnumStringD p3 = p1;
    Test::MyEnumStringD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opMyStructMyEnumD_async(const Test::AMD_MyClass_opMyStructMyEnumDPtr& cb,
                                       const Test::MyStructMyEnumD& p1,
                                       const Test::MyStructMyEnumD& p2,
                                       const Ice::Current&)
{
    Test::MyStructMyEnumD p3 = p1;
    Test::MyStructMyEnumD r = p1;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opByteBoolDS_async(const Test::AMD_MyClass_opByteBoolDSPtr& cb,
                                    const Test::ByteBoolDS& p1,
                                    const Test::ByteBoolDS& p2,
                                    const Ice::Current&)
{
    Test::ByteBoolDS p3 = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(p3));
    Test::ByteBoolDS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opShortIntDS_async(const Test::AMD_MyClass_opShortIntDSPtr& cb,
                                       const Test::ShortIntDS& p1,
                                       const Test::ShortIntDS& p2,
                                       const Ice::Current&)
{
    Test::ShortIntDS p3 = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(p3));
    Test::ShortIntDS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opLongFloatDS_async(const Test::AMD_MyClass_opLongFloatDSPtr& cb,
                                       const Test::LongFloatDS& p1,
                                       const Test::LongFloatDS& p2,
                                       const Ice::Current&)
{
    Test::LongFloatDS p3 = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(p3));
    Test::LongFloatDS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opStringStringDS_async(const Test::AMD_MyClass_opStringStringDSPtr& cb,
                                       const Test::StringStringDS& p1,
                                       const Test::StringStringDS& p2,
                                       const Ice::Current&)
{
    Test::StringStringDS p3 = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(p3));
    Test::StringStringDS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opStringMyEnumDS_async(const Test::AMD_MyClass_opStringMyEnumDSPtr& cb,
                                       const Test::StringMyEnumDS& p1,
                                       const Test::StringMyEnumDS& p2,
                                       const Ice::Current&)
{
    Test::StringMyEnumDS p3 = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(p3));
    Test::StringMyEnumDS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opMyEnumStringDS_async(const Test::AMD_MyClass_opMyEnumStringDSPtr& cb,
                                       const Test::MyEnumStringDS& p1,
                                       const Test::MyEnumStringDS& p2,
                                       const Ice::Current&)
{
    Test::MyEnumStringDS p3 = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(p3));
    Test::MyEnumStringDS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opMyStructMyEnumDS_async(const Test::AMD_MyClass_opMyStructMyEnumDSPtr& cb,
                                       const Test::MyStructMyEnumDS& p1,
                                       const Test::MyStructMyEnumDS& p2,
                                       const Ice::Current&)
{
    Test::MyStructMyEnumDS p3 = p2;
    std::copy(p1.begin(), p1.end(), std::back_inserter(p3));
    Test::MyStructMyEnumDS r;
    r.resize(p1.size());
    std::reverse_copy(p1.begin(), p1.end(), r.begin());
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opByteByteSD_async(const Test::AMD_MyClass_opByteByteSDPtr& cb,
                                       const Test::ByteByteSD& p1,
                                       const Test::ByteByteSD& p2,
                                       const Ice::Current&)
{
    Test::ByteByteSD p3 = p2;
    Test::ByteByteSD r;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opBoolBoolSD_async(const Test::AMD_MyClass_opBoolBoolSDPtr& cb,
                                       const Test::BoolBoolSD& p1,
                                       const Test::BoolBoolSD& p2,
                                       const Ice::Current&)
{
    Test::BoolBoolSD p3 = p2;
    Test::BoolBoolSD r;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opShortShortSD_async(const Test::AMD_MyClass_opShortShortSDPtr& cb,
                                       const Test::ShortShortSD& p1,
                                       const Test::ShortShortSD& p2,
                                       const Ice::Current&)
{
    Test::ShortShortSD p3 = p2;
    Test::ShortShortSD r;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opIntIntSD_async(const Test::AMD_MyClass_opIntIntSDPtr& cb,
                                       const Test::IntIntSD& p1,
                                       const Test::IntIntSD& p2,
                                       const Ice::Current&)
{
    Test::IntIntSD p3 = p2;
    Test::IntIntSD r;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opLongLongSD_async(const Test::AMD_MyClass_opLongLongSDPtr& cb,
                                       const Test::LongLongSD& p1,
                                       const Test::LongLongSD& p2,
                                       const Ice::Current&)
{
    Test::LongLongSD p3 = p2;
    Test::LongLongSD r;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opStringFloatSD_async(const Test::AMD_MyClass_opStringFloatSDPtr& cb,
                                       const Test::StringFloatSD& p1,
                                       const Test::StringFloatSD& p2,
                                       const Ice::Current&)
{
    Test::StringFloatSD p3 = p2;
    Test::StringFloatSD r;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opStringDoubleSD_async(const Test::AMD_MyClass_opStringDoubleSDPtr& cb,
                                       const Test::StringDoubleSD& p1,
                                       const Test::StringDoubleSD& p2,
                                       const Ice::Current&)
{
    Test::StringDoubleSD p3 = p2;
    Test::StringDoubleSD r;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opStringStringSD_async(const Test::AMD_MyClass_opStringStringSDPtr& cb,
                                       const Test::StringStringSD& p1,
                                       const Test::StringStringSD& p2,
                                       const Ice::Current&)
{
    Test::StringStringSD p3 = p2;
    Test::StringStringSD r;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opMyEnumMyEnumSD_async(const Test::AMD_MyClass_opMyEnumMyEnumSDPtr& cb,
                                       const Test::MyEnumMyEnumSD& p1,
                                       const Test::MyEnumMyEnumSD& p2,
                                       const Ice::Current&)
{
    Test::MyEnumMyEnumSD p3 = p2;
    Test::MyEnumMyEnumSD r;
    std::set_union(p1.begin(), p1.end(), p2.begin(), p2.end(), std::inserter(r, r.end()));
    cb->ice_response(r, p3);
}

void
MyDerivedClassI::opIntS_async(const Test::AMD_MyClass_opIntSPtr& cb, const Test::IntS& s, const Ice::Current&)
{
    Test::IntS r;
    std::transform(s.begin(), s.end(), std::back_inserter(r), std::negate<int>());
    cb->ice_response(r);
}

void
MyDerivedClassI::opByteSOneway_async(const Test::AMD_MyClass_opByteSOnewayPtr& cb, const Test::ByteS&,
                                     const Ice::Current&)
{
    IceUtil::Mutex::Lock sync(_mutex);
    ++_opByteSOnewayCallCount;
    cb->ice_response();
}

void
MyDerivedClassI::opByteSOnewayCallCount_async(const Test::AMD_MyClass_opByteSOnewayCallCountPtr& cb, 
                                              const Ice::Current&)
{
    IceUtil::Mutex::Lock sync(_mutex);
    cb->ice_response(_opByteSOnewayCallCount);
    _opByteSOnewayCallCount = 0;
}

void
MyDerivedClassI::opContext_async(const Test::AMD_MyClass_opContextPtr& cb, const Ice::Current& c)
{
    Test::StringStringD r = c.ctx;
    cb->ice_response(r);
}

void
MyDerivedClassI::opDoubleMarshaling_async(const Test::AMD_MyClass_opDoubleMarshalingPtr& cb,
                                          Ice::Double p1, const Test::DoubleS& p2, const Ice::Current&)
{
    Ice::Double d = 1278312346.0 / 13.0;
    test(p1 == d);
    for(unsigned int i = 0; i < p2.size(); ++i)
    {
        test(p2[i] == d);
    }
    cb->ice_response();
}

void
MyDerivedClassI::opIdempotent_async(const Test::AMD_MyClass_opIdempotentPtr& cb, const Ice::Current& current)
{
    test(current.mode == Ice::Idempotent);
    cb->ice_response();
}

void
MyDerivedClassI::opNonmutating_async(const Test::AMD_MyClass_opNonmutatingPtr& cb, const Ice::Current& current)
{
    test(current.mode == Ice::Nonmutating);
    cb->ice_response();
}

void
MyDerivedClassI::opDerived_async(const Test::AMD_MyDerivedClass_opDerivedPtr& cb, const Ice::Current&)
{
    cb->ice_response();
}

void
MyDerivedClassI::opByte1_async(const Test::AMD_MyClass_opByte1Ptr& cb, Ice::Byte b, const Ice::Current&)
{
    cb->ice_response(b);
}

void
MyDerivedClassI::opShort1_async(const Test::AMD_MyClass_opShort1Ptr& cb, Ice::Short s, const Ice::Current&)
{
    cb->ice_response(s);
}

void
MyDerivedClassI::opInt1_async(const Test::AMD_MyClass_opInt1Ptr& cb, Ice::Int i, const Ice::Current&)
{
    cb->ice_response(i);
}

void
MyDerivedClassI::opLong1_async(const Test::AMD_MyClass_opLong1Ptr& cb, Ice::Long l, const Ice::Current&)
{
    cb->ice_response(l);
}

void
MyDerivedClassI::opFloat1_async(const Test::AMD_MyClass_opFloat1Ptr& cb, Ice::Float f, const Ice::Current&)
{
    cb->ice_response(f);
}

void
MyDerivedClassI::opDouble1_async(const Test::AMD_MyClass_opDouble1Ptr& cb, Ice::Double d, const Ice::Current&)
{
    cb->ice_response(d);
}

void
MyDerivedClassI::opString1_async(const Test::AMD_MyClass_opString1Ptr& cb, const std::string& s,
                                 const Ice::Current&)
{
    cb->ice_response(s);
}

void
MyDerivedClassI::opStringS1_async(const Test::AMD_MyClass_opStringS1Ptr& cb, const Test::StringS& seq,
                                  const Ice::Current&)
{
    cb->ice_response(seq);
}

void
MyDerivedClassI::opByteBoolD1_async(const Test::AMD_MyClass_opByteBoolD1Ptr& cb, const Test::ByteBoolD& dict,
                                    const Ice::Current&)
{
    cb->ice_response(dict);
}

void
MyDerivedClassI::opStringS2_async(const Test::AMD_MyClass_opStringS2Ptr& cb, const Test::StringS& seq,
                                  const Ice::Current&)
{
    cb->ice_response(seq);
}

void
MyDerivedClassI::opByteBoolD2_async(const Test::AMD_MyClass_opByteBoolD2Ptr& cb, const Test::ByteBoolD& dict,
                                    const Ice::Current&)
{
    cb->ice_response(dict);
}

void
MyDerivedClassI::opMyStruct1_async(const Test::AMD_MyDerivedClass_opMyStruct1Ptr& cb, const Test::MyStruct1& s, const Ice::Current&)
{
    cb->ice_response(s);
}

void
MyDerivedClassI::opMyClass1_async(const Test::AMD_MyDerivedClass_opMyClass1Ptr& cb, const Test::MyClass1Ptr& c, const Ice::Current&)
{
    cb->ice_response(c);
}

void
MyDerivedClassI::opStringLiterals_async(const Test::AMD_MyClass_opStringLiteralsPtr& cb,
                                        const Ice::Current&)
{
    Test::StringS data;
    data.push_back(Test::s0);
    data.push_back(Test::s1);
    data.push_back(Test::s2);
    data.push_back(Test::s3);
    data.push_back(Test::s4);
    data.push_back(Test::s5);
    data.push_back(Test::s6);
    data.push_back(Test::s7);
    data.push_back(Test::s8);
    data.push_back(Test::s9);
    data.push_back(Test::s10);

    data.push_back(Test::sw0);
    data.push_back(Test::sw1);
    data.push_back(Test::sw2);
    data.push_back(Test::sw3);
    data.push_back(Test::sw4);
    data.push_back(Test::sw5);
    data.push_back(Test::sw6);
    data.push_back(Test::sw7);
    data.push_back(Test::sw8);
    data.push_back(Test::sw9);
    data.push_back(Test::sw10);

    data.push_back(Test::ss0);
    data.push_back(Test::ss1);
    data.push_back(Test::ss2);
    data.push_back(Test::ss3);
    data.push_back(Test::ss4);
    data.push_back(Test::ss5);

    data.push_back(Test::su0);
    data.push_back(Test::su1);
    data.push_back(Test::su2);
    cb->ice_response(data);
}

void
MyDerivedClassI::opWStringLiterals_async(const Test::AMD_MyClass_opWStringLiteralsPtr& cb,
                                         const Ice::Current&)
{
    Test::WStringS data;
    data.push_back(Test::ws0);
    data.push_back(Test::ws1);
    data.push_back(Test::ws2);
    data.push_back(Test::ws3);
    data.push_back(Test::ws4);
    data.push_back(Test::ws5);
    data.push_back(Test::ws6);
    data.push_back(Test::ws7);
    data.push_back(Test::ws8);
    data.push_back(Test::ws9);
    data.push_back(Test::ws10);

    data.push_back(Test::wsw0);
    data.push_back(Test::wsw1);
    data.push_back(Test::wsw2);
    data.push_back(Test::wsw3);
    data.push_back(Test::wsw4);
    data.push_back(Test::wsw5);
    data.push_back(Test::wsw6);
    data.push_back(Test::wsw7);
    data.push_back(Test::wsw8);
    data.push_back(Test::wsw9);
    data.push_back(Test::wsw10);

    data.push_back(Test::wss0);
    data.push_back(Test::wss1);
    data.push_back(Test::wss2);
    data.push_back(Test::wss3);
    data.push_back(Test::wss4);
    data.push_back(Test::wss5);

    data.push_back(Test::wsu0);
    data.push_back(Test::wsu1);
    data.push_back(Test::wsu2);
    cb->ice_response(data);
}
