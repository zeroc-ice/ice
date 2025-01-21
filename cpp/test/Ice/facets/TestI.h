// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class AI : public virtual Test::A
{
public:
    std::string callA(const Ice::Current&) override;
};

class BI : public virtual Test::B, public virtual AI
{
public:
    std::string callB(const Ice::Current&) override;
};

class CI : public virtual Test::C, public virtual AI
{
public:
    std::string callC(const Ice::Current&) override;
};

class DI : public virtual Test::D, public virtual BI, public virtual CI
{
public:
    std::string callD(const Ice::Current&) override;
};

class EI : public virtual Test::E
{
public:
    std::string callE(const Ice::Current&) override;
};

class FI : public virtual Test::F, public virtual EI
{
public:
    std::string callF(const Ice::Current&) override;
};

class GI : public virtual Test::G
{
public:
    void shutdown(const Ice::Current&) override;
    std::string callG(const Ice::Current&) override;
};

class HI : public virtual Test::H, public virtual GI
{
public:
    std::string callH(const Ice::Current&) override;
};

#endif
