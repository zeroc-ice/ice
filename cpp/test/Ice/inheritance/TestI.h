// Copyright (c) ZeroC, Inc.

#ifndef TESTI_H
#define TESTI_H

#include "Test.h"

class InitialI final : public Test::Initial
{
public:
    InitialI(const Ice::ObjectAdapterPtr&);

    void shutdown(const Ice::Current&) final;

    std::optional<Test::MA::IAPrx> iaop(const Ice::Current&) final;
    std::optional<Test::MB::IB1Prx> ib1op(const Ice::Current&) final;
    std::optional<Test::MB::IB2Prx> ib2op(const Ice::Current&) final;
    std::optional<Test::MA::ICPrx> icop(const Ice::Current&) final;

private:
    std::optional<Test::MA::IAPrx> _ia;
    std::optional<Test::MB::IB1Prx> _ib1;
    std::optional<Test::MB::IB2Prx> _ib2;
    std::optional<Test::MA::ICPrx> _ic;
};

class IAI : public virtual Test::MA::IA
{
public:
    std::optional<Test::MA::IAPrx> iaop(std::optional<Test::MA::IAPrx>, const Ice::Current&) override;
};

class IB1I : public virtual Test::MB::IB1, public virtual IAI
{
public:
    std::optional<Test::MB::IB1Prx> ib1op(std::optional<Test::MB::IB1Prx>, const Ice::Current&) override;
};

class IB2I : public virtual Test::MB::IB2, public virtual IAI
{
public:
    std::optional<Test::MB::IB2Prx> ib2op(std::optional<Test::MB::IB2Prx>, const Ice::Current&) override;
};

class ICI final : public virtual Test::MA::IC, public virtual IB1I, public virtual IB2I
{
public:
    std::optional<Test::MA::ICPrx> icop(std::optional<Test::MA::ICPrx>, const Ice::Current&) final;
};

#endif
