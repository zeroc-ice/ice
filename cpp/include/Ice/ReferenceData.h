// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_REFERENCE_DATA_H
#define ICE_REFERENCE_DATA_H

#include <Ice/ReferenceDataF.h>
#include <Ice/EndpointDataF.h>
#include <Ice/InstanceF.h>
#include <Ice/Shared.h>

namespace _Ice
{

class ICE_API ReferenceDataI : virtual public Shared
{
public:

    ReferenceDataI(const Instance&, const std::string&, const EndpointData&);
    virtual ~ReferenceDataI();

    bool operator==(const ReferenceDataI&) const;
    bool operator!=(const ReferenceDataI&) const;

    //
    // All data members are const, because ReferenceDatas are immutable.
    //
    const Instance instance;
    const std::string identity;
    const EndpointData endpointData;
 
private:

    ReferenceDataI(const ReferenceDataI&);
    void operator=(const ReferenceDataI&);
};

}

#endif
