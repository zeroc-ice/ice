// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_PACK_ADAPTER_REGISTRY_I_H
#define ICE_PACK_ADAPTER_REGISTRY_I_H

#include <IcePack/Internal.h>
#include <IcePack/StringObjectProxyDict.h>

namespace IcePack
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class AdapterRegistryI : public AdapterRegistry
{
public:

    AdapterRegistryI(const Freeze::DBPtr&, const TraceLevelsPtr&);

    virtual void add(const std::string&, const AdapterPrx&, const ::Ice::Current&);
    virtual void remove(const std::string&, const ::Ice::Current&);

    virtual AdapterPrx findByName(const ::std::string&, const ::Ice::Current&);
    virtual Ice::StringSeq getAll(const ::Ice::Current&) const;

private:

    StringObjectProxyDict _dict;
    TraceLevelsPtr _traceLevels;
};

}

#endif
