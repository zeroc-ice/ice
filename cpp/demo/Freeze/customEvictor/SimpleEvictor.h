// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SIMPLE_EVICTOR_H
#define SIMPLE_EVICTOR_H

#include <EvictorBase.h>
#include <CurrentDatabase.h>
#include <IceUtil/IceUtil.h>

class SimpleEvictor : public EvictorBase
{
public:

    SimpleEvictor(CurrentDatabase&, int);

protected:
    
    virtual Ice::ObjectPtr add(const Ice::Current&, Ice::LocalObjectPtr&);
    virtual void evict(const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);

private:

    CurrentDatabase& _currentDb;
};

#endif
