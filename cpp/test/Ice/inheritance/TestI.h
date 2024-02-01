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

    virtual std::shared_ptr<Test::MA::IAPrx> iaop(std::shared_ptr<Test::MA::IAPrx>, const Ice::Current&);
};

class IB1I : public virtual Test::MB::IB1,
             public virtual IAI
{
public:

    virtual std::shared_ptr<Test::MB::IB1Prx> ib1op(std::shared_ptr<Test::MB::IB1Prx>, const Ice::Current&);
};

class IB2I : public virtual Test::MB::IB2, public virtual IAI
{
public:

    virtual std::shared_ptr<Test::MB::IB2Prx> ib2op(std::shared_ptr<Test::MB::IB2Prx>, const Ice::Current&);
};

class ICI : public virtual Test::MA::IC, public virtual IB1I, public virtual IB2I
{
public:

    virtual std::shared_ptr<Test::MA::ICPrx> icop(std::shared_ptr<Test::MA::ICPrx>, const Ice::Current&);
};

#endif
