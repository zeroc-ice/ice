// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class BI : public Test::B
{
public:

    BI();

    virtual bool postUnmarshalInvoked(const Ice::Current&);

    virtual void ice_preMarshal();
    virtual void ice_postUnmarshal();

private:

    bool _postUnmarshalInvoked;
};

class CI : public Test::C
{
public:

    CI();

    virtual bool postUnmarshalInvoked(const Ice::Current&);

    virtual void ice_preMarshal();
    virtual void ice_postUnmarshal();

private:

    bool _postUnmarshalInvoked;
};

class DI : public Test::D
{
public:

    DI();

    virtual bool postUnmarshalInvoked(const Ice::Current&);

    virtual void ice_preMarshal();
    virtual void ice_postUnmarshal();

private:

    bool _postUnmarshalInvoked;
};

class EI : public Test::E
{
public:

    EI();

    virtual bool checkValues(const Ice::Current&);
};

class FI : public Test::F
{
public:

    FI();
    FI(const Test::EPtr&);

    virtual bool checkValues(const Ice::Current&);
};

class II : public Test::I
{
};

class JI : public Test::J
{
};

class HI : public Test::H
{
};

class InitialI : public Test::Initial
{
public:

    InitialI(const Ice::ObjectAdapterPtr&);

    virtual void shutdown(const Ice::Current&);
    virtual Test::BPtr getB1(const Ice::Current&);
    virtual Test::BPtr getB2(const Ice::Current&);
    virtual Test::CPtr getC(const Ice::Current&);
    virtual Test::DPtr getD(const Ice::Current&);
    virtual Test::EPtr getE(const Ice::Current&);
    virtual Test::FPtr getF(const Ice::Current&);

    virtual void setRecursive(const Test::RecursivePtr&, const Ice::Current&);
    virtual bool supportsClassGraphDepthMax(const Ice::Current&);

    virtual void getAll(Test::BPtr&, Test::BPtr&, Test::CPtr&, Test::DPtr&, const Ice::Current&);
    virtual Test::IPtr getI(const Ice::Current&);
    virtual Test::IPtr getJ(const Ice::Current&);
    virtual Test::IPtr getH(const Ice::Current&);

    virtual Test::D1Ptr getD1(const Test::D1Ptr&, const Ice::Current&);
    virtual void throwEDerived(const Ice::Current&);

    virtual void setI(const Test::IPtr&, const Ice::Current&);

    virtual Test::BaseSeq opBaseSeq(const Test::BaseSeq&, Test::BaseSeq&, const Ice::Current&);

    virtual Test::CompactPtr getCompact(const Ice::Current&);

    virtual Test::Inner::APtr getInnerA(const Ice::Current&);
    virtual Test::Inner::Sub::APtr getInnerSubA(const Ice::Current&);

    virtual void throwInnerEx(const Ice::Current&);
    virtual void throwInnerSubEx(const Ice::Current&);

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

    virtual bool ice_invoke(const std::vector<Ice::Byte>&, std::vector<Ice::Byte>&, const Ice::Current&);
};
typedef IceUtil::Handle<UnexpectedObjectExceptionTestI> UnexpectedObjectExceptionTestIPtr;

class TestIntfI : public Test::TestIntf
{
public:

    virtual Test::BasePtr opDerived(const Ice::Current&);
    virtual void throwDerived(const Ice::Current&);
};

#endif
