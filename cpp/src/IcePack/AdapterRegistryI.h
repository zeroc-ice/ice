// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
