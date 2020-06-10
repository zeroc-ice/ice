//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>
#include <Forward.h>

class BI : public Test::B
{
public:

    virtual void ice_preMarshal();
    virtual void ice_postUnmarshal();
};

class CI : public Test::C
{
public:

    virtual void ice_preMarshal();
    virtual void ice_postUnmarshal();
};

class DI : public Test::D
{
public:

    virtual void ice_preMarshal();
    virtual void ice_postUnmarshal();
};

class EI : public Test::E
{
public:

    EI();

    bool checkValues();
};
ICE_DEFINE_PTR(EIPtr, EI);

class FI : public Test::F
{
public:

    FI();
    FI(const Test::EPtr&);

    bool checkValues();
};
ICE_DEFINE_PTR(FIPtr, FI);

class II : public ::Ice::InterfaceByValue<Test::I>
{
};

class JI : public ::Ice::InterfaceByValue<Test::J>
{
};

class InitialI : public Test::Initial
{
public:

    InitialI(const Ice::ObjectAdapterPtr&);
    virtual ~InitialI();

    virtual void shutdown(const Ice::Current&);
    virtual Test::BPtr getB1(const Ice::Current&);
    virtual Test::BPtr getB2(const Ice::Current&);
    virtual Test::CPtr getC(const Ice::Current&);
    virtual Test::DPtr getD(const Ice::Current&);
    virtual Test::EPtr getE(const Ice::Current&);
    virtual Test::FPtr getF(const Ice::Current&);

    virtual void setRecursive(Test::RecursivePtr, const Ice::Current&);
    virtual bool supportsClassGraphDepthMax(const Ice::Current&);

    virtual GetMBMarshaledResult getMB(const Ice::Current&);
    virtual void getAMDMBAsync(std::function<void(const GetAMDMBMarshaledResult&)>,
                               std::function<void(std::exception_ptr)>,
                               const Ice::Current&);

    virtual void getAll(Test::BPtr&, Test::BPtr&, Test::CPtr&, Test::DPtr&, const Ice::Current&);

    virtual Test::KPtr getK(const Ice::Current&);

    virtual Ice::ValuePtr opClass(Ice::ValuePtr, Ice::ValuePtr&, const Ice::Current&);
    virtual Test::ClassSeq opClassSeq(Test::ClassSeq, Test::ClassSeq&, const Ice::Current&);
    virtual Test::ClassMap opClassMap(Test::ClassMap, Test::ClassMap&, const Ice::Current&);

    virtual Test::D1Ptr getD1(Test::D1Ptr, const Ice::Current&);
    virtual void throwEDerived(const Ice::Current&);

    virtual void setG(Test::GPtr, const Ice::Current&);

    virtual Test::BaseSeq opBaseSeq(Test::BaseSeq, Test::BaseSeq&, const Ice::Current&);

    virtual Test::CompactPtr getCompact(const Ice::Current&);

    virtual Test::Inner::APtr getInnerA(const Ice::Current&);
    virtual Test::Inner::Sub::APtr getInnerSubA(const Ice::Current&);

    virtual void throwInnerEx(const Ice::Current&);
    virtual void throwInnerSubEx(const Ice::Current&);

    virtual Test::MPtr opM(Test::MPtr, Test::MPtr&, const Ice::Current&);

    virtual Test::F1Ptr opF1(Test::F1Ptr, Test::F1Ptr&, const Ice::Current&);
    virtual Test::F2PrxPtr opF2(Test::F2PrxPtr, Test::F2PrxPtr&, const Ice::Current&);
    virtual Test::F3Ptr opF3(Test::F3Ptr, Test::F3Ptr&, const Ice::Current&);
    virtual bool hasF3(const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _adapter;
    Test::BPtr _b1;
    Test::BPtr _b2;
    Test::CPtr _c;
    Test::DPtr _d;
    Test::EPtr _e;
    Test::FPtr _f;
};

class UnexpectedObjectExceptionTestI : public Ice::Blobject
{
public:

    virtual bool ice_invoke(std::vector<Ice::Byte>, std::vector<Ice::Byte>&, const Ice::Current&);
};
ICE_DEFINE_PTR(UnexpectedObjectExceptionTestIPtr, UnexpectedObjectExceptionTestI);

class TestIntfI : public Test::TestIntf
{
public:

    virtual Test::BasePtr opDerived(const Ice::Current&);
    virtual void throwDerived(const Ice::Current&);
};

class F2I : public Test::F2
{
public:

    virtual void op(const Ice::Current&)
    {
    }
};

#endif
