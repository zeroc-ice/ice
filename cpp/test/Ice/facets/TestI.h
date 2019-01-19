//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class AI : public virtual Test::A
{
public:

    virtual std::string callA(const Ice::Current&);
};

class BI : public virtual Test::B, public virtual AI
{
public:

    virtual std::string callB(const Ice::Current&);
};

class CI : public virtual Test::C, public virtual AI
{
public:

    virtual std::string callC(const Ice::Current&);
};

class DI : public virtual Test::D, public virtual BI, public virtual CI
{
public:

    virtual std::string callD(const Ice::Current&);
};

class EI : public virtual Test::E
{
public:

    virtual std::string callE(const Ice::Current&);
};

class FI : public virtual Test::F, public virtual EI
{
public:

    virtual std::string callF(const Ice::Current&);
};

class GI : public virtual Test::G
{
public:

    virtual void shutdown(const Ice::Current&);
    virtual std::string callG(const Ice::Current&);
};

class HI : public virtual Test::H, public virtual GI
{
public:

    virtual std::string callH(const Ice::Current&);
};

#endif
