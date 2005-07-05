// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class ThrowerI : public Test::Thrower
{
public:

    ThrowerI(const IceE::ObjectAdapterPtr&);

    virtual void shutdown(const IceE::Current&);
    virtual bool supportsUndeclaredExceptions(const IceE::Current&);
    virtual bool supportsAssertException(const IceE::Current&);

    virtual void throwAasA(IceE::Int, const IceE::Current&);
    virtual void throwAorDasAorD(IceE::Int, const IceE::Current&);
    virtual void throwBasA(IceE::Int, IceE::Int, const IceE::Current&);
    virtual void throwCasA(IceE::Int, IceE::Int, IceE::Int, const IceE::Current&);
    virtual void throwBasB(IceE::Int, IceE::Int, const IceE::Current&);
    virtual void throwCasB(IceE::Int, IceE::Int, IceE::Int, const IceE::Current&);
    virtual void throwCasC(IceE::Int, IceE::Int, IceE::Int, const IceE::Current&);

    virtual void throwModA(IceE::Int, IceE::Int, const IceE::Current&);
    
    virtual void throwUndeclaredA(IceE::Int, const IceE::Current&);
    virtual void throwUndeclaredB(IceE::Int, IceE::Int, const IceE::Current&);
    virtual void throwUndeclaredC(IceE::Int, IceE::Int, IceE::Int, const IceE::Current&);
    virtual void throwLocalException(const IceE::Current&);
    virtual void throwNonIceException(const IceE::Current&);
    virtual void throwAssertException(const IceE::Current&);

private:

    IceE::ObjectAdapterPtr _adapter;
};

#endif
