// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
class CAI_ : public virtual Test::MA::CADisp
{
public:

    virtual std::shared_ptr<Test::MA::CAPrx> caop(std::shared_ptr<Test::MA::CAPrx>, const Ice::Current&);
};

class CBI : public virtual Test::MB::CBDisp, public virtual CAI_
{
public:

    virtual std::shared_ptr<Test::MB::CBPrx> cbop(std::shared_ptr<Test::MB::CBPrx>, const Ice::Current&);
};

class CCI : public virtual Test::MA::CCDisp, public virtual CBI
{
public:

    virtual std::shared_ptr<Test::MA::CCPrx> ccop(std::shared_ptr<Test::MA::CCPrx>, const Ice::Current&);
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

class CDI : public virtual Test::MA::CDDisp, public virtual CCI, public virtual IB1I, public virtual IB2I
{
public:

    virtual std::shared_ptr<Test::MA::CDPrx> cdop(std::shared_ptr<Test::MA::CDPrx>, const Ice::Current&);
};
#else
class CAI_ : public virtual Test::MA::CA
{
public:

    virtual Test::MA::CAPrx caop(const Test::MA::CAPrx&, const Ice::Current&);
};

class CBI : public virtual Test::MB::CB, public virtual CAI_
{
public:

    virtual Test::MB::CBPrx cbop(const Test::MB::CBPrx&, const Ice::Current&);
};

class CCI : public virtual Test::MA::CC, public virtual CBI
{
public:

    virtual Test::MA::CCPrx ccop(const Test::MA::CCPrx&, const Ice::Current&);
};

class IAI : public virtual Test::MA::IA
{
public:

    virtual Test::MA::IAPrx iaop(const Test::MA::IAPrx&, const Ice::Current&);
};

class IB1I : public virtual Test::MB::IB1, public virtual IAI
{
public:

    virtual Test::MB::IB1Prx ib1op(const Test::MB::IB1Prx&, const Ice::Current&);
};

class IB2I : public virtual Test::MB::IB2, public virtual IAI
{
public:

    virtual Test::MB::IB2Prx ib2op(const Test::MB::IB2Prx&, const Ice::Current&);
};

class ICI : public virtual Test::MA::IC, public virtual IB1I, public virtual IB2I
{
public:

    virtual Test::MA::ICPrx icop(const Test::MA::ICPrx&, const Ice::Current&);
};

class CDI : public virtual Test::MA::CD, public virtual CCI, public virtual IB1I, public virtual IB2I
{
public:

    virtual Test::MA::CDPrx cdop(const Test::MA::CDPrx&, const Ice::Current&);
};
#endif

#endif
