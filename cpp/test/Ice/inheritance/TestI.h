//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TESTI_H
#define TESTI_H

#include <Test.h>

class InitialI : public Test::Initial
{
public:
    InitialI(const Ice::ObjectAdapterPtr&);

    virtual void shutdown(const Ice::Current&);

    virtual Test::MA::IAPrxPtr iaop(const Ice::Current&);
    virtual Test::MB::IB1PrxPtr ib1op(const Ice::Current&);
    virtual Test::MB::IB2PrxPtr ib2op(const Ice::Current&);
    virtual Test::MA::ICPrxPtr icop(const Ice::Current&);

private:
    Test::MA::IAPrxPtr _ia;
    Test::MB::IB1PrxPtr _ib1;
    Test::MB::IB2PrxPtr _ib2;
    Test::MA::ICPrxPtr _ic;
};

class IAI : public virtual Test::MA::IA
{
public:
    virtual Test::MA::IAPrxPtr iaop(Test::MA::IAPrxPtr, const Ice::Current&);
};

class IB1I : public virtual Test::MB::IB1, public virtual IAI
{
public:
    virtual Test::MB::IB1PrxPtr ib1op(Test::MB::IB1PrxPtr, const Ice::Current&);
};

class IB2I : public virtual Test::MB::IB2, public virtual IAI
{
public:
    virtual Test::MB::IB2PrxPtr ib2op(Test::MB::IB2PrxPtr, const Ice::Current&);
};

class ICI : public virtual Test::MA::IC, public virtual IB1I, public virtual IB2I
{
public:
    virtual Test::MA::ICPrxPtr icop(Test::MA::ICPrxPtr, const Ice::Current&);
};

#endif
