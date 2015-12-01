// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TESTI_H
#define TESTI_H

#include <Test.h>

class InitialI : public Test::Initial
{
public:

    InitialI(const Ice::ObjectAdapterPtr&);

    virtual void shutdown(const Ice::Current&);

    virtual Test::MA::CAPrxPtr caop(const Ice::Current&);
    virtual Test::MB::CBPrxPtr cbop(const Ice::Current&);
    virtual Test::MA::CCPrxPtr ccop(const Ice::Current&);
    virtual Test::MA::CDPrxPtr cdop(const Ice::Current&);
    virtual Test::MA::IAPrxPtr iaop(const Ice::Current&);
    virtual Test::MB::IB1PrxPtr ib1op(const Ice::Current&);
    virtual Test::MB::IB2PrxPtr ib2op(const Ice::Current&);
    virtual Test::MA::ICPrxPtr icop(const Ice::Current&);

private:

    Test::MA::CAPrxPtr _ca;
    Test::MB::CBPrxPtr _cb;
    Test::MA::CCPrxPtr _cc;
    Test::MA::CDPrxPtr _cd;
    Test::MA::IAPrxPtr _ia;
    Test::MB::IB1PrxPtr _ib1;
    Test::MB::IB2PrxPtr _ib2;
    Test::MA::ICPrxPtr _ic;
};

#ifdef ICE_CPP11_MAPPING
class CAI_ : virtual public Test::MA::CADisp
{
public:

    virtual std::shared_ptr<Test::MA::CAPrx> caop(const std::shared_ptr<Test::MA::CAPrx>&, const Ice::Current&);
};

class CBI : virtual public Test::MB::CBDisp, virtual public CAI_
{
public:

    virtual std::shared_ptr<Test::MB::CBPrx> cbop(const std::shared_ptr<Test::MB::CBPrx>&, const Ice::Current&);
};

class CCI : virtual public Test::MA::CCDisp, virtual public CBI
{
public:

    virtual std::shared_ptr<Test::MA::CCPrx> ccop(const std::shared_ptr<Test::MA::CCPrx>&, const Ice::Current&);
};

class IAI : virtual public Test::MA::IA
{
public:

    virtual std::shared_ptr<Test::MA::IAPrx> iaop(const std::shared_ptr<Test::MA::IAPrx>&, const Ice::Current&);
};

class IB1I : virtual public Test::MB::IB1,
             virtual public IAI
{
public:

    virtual std::shared_ptr<Test::MB::IB1Prx> ib1op(const std::shared_ptr<Test::MB::IB1Prx>&, const Ice::Current&);
};

class IB2I : virtual public Test::MB::IB2, virtual public IAI
{
public:

    virtual std::shared_ptr<Test::MB::IB2Prx> ib2op(const std::shared_ptr<Test::MB::IB2Prx>&, const Ice::Current&);
};

class ICI : virtual public Test::MA::IC, virtual public IB1I, virtual public IB2I
{
public:

    virtual std::shared_ptr<Test::MA::ICPrx> icop(const std::shared_ptr<Test::MA::ICPrx>&, const Ice::Current&);
};

class CDI : virtual public Test::MA::CDDisp, virtual public CCI, virtual public IB1I, virtual public IB2I
{
public:

    virtual std::shared_ptr<Test::MA::CDPrx> cdop(const std::shared_ptr<Test::MA::CDPrx>&, const Ice::Current&);
};
#else
class CAI_ : virtual public Test::MA::CA
{
public:

    virtual Test::MA::CAPrx caop(const Test::MA::CAPrx&, const Ice::Current&);
};

class CBI : virtual public Test::MB::CB, virtual public CAI_
{
public:

    virtual Test::MB::CBPrx cbop(const Test::MB::CBPrx&, const Ice::Current&);
};

class CCI : virtual public Test::MA::CC, virtual public CBI
{
public:

    virtual Test::MA::CCPrx ccop(const Test::MA::CCPrx&, const Ice::Current&);
};

class IAI : virtual public Test::MA::IA
{
public:

    virtual Test::MA::IAPrx iaop(const Test::MA::IAPrx&, const Ice::Current&);
};

class IB1I : virtual public Test::MB::IB1, virtual public IAI
{
public:

    virtual Test::MB::IB1Prx ib1op(const Test::MB::IB1Prx&, const Ice::Current&);                  
};

class IB2I : virtual public Test::MB::IB2, virtual public IAI
{
public:

    virtual Test::MB::IB2Prx ib2op(const Test::MB::IB2Prx&, const Ice::Current&);
};

class ICI : virtual public Test::MA::IC, virtual public IB1I, virtual public IB2I
{
public:

    virtual Test::MA::ICPrx icop(const Test::MA::ICPrx&, const Ice::Current&);
};

class CDI : virtual public Test::MA::CD, virtual public CCI, virtual public IB1I, virtual public IB2I
{
public:

    virtual Test::MA::CDPrx cdop(const Test::MA::CDPrx&, const Ice::Current&);
};
#endif

#endif
