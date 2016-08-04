// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_VALUE_FACTORY_MANAGER_I_H
#define ICE_VALUE_FACTORY_MANAGER_I_H

#include <Ice/ValueFactory.h>
#include <IceUtil/Mutex.h>

namespace IceInternal
{

class ValueFactoryManagerI;
ICE_DEFINE_PTR(ValueFactoryManagerIPtr, ValueFactoryManagerI);

class ValueFactoryManagerI : public Ice::ValueFactoryManager,
                             public IceUtil::Mutex
{
public:

    ValueFactoryManagerI();

    virtual void add(ICE_IN(ICE_VALUE_FACTORY), const std::string&);
    virtual ICE_VALUE_FACTORY find(const std::string&) const;

private:

    typedef std::map<std::string, ICE_VALUE_FACTORY> FactoryMap;

    FactoryMap _factoryMap;
    mutable FactoryMap::iterator _factoryMapHint;
};

}

#endif
