// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_STUB_MARSHAL_H
#define ICE_STUB_MARSHAL_H

#include <Ice/Stub.h>

namespace Ice
{

class StubMarshal;
void _incRef(StubMarshal*);
void _decRef(StubMarshal*);
typedef Handle<StubMarshal> StubMarshalHandle;

// ----------------------------------------------------------------------
// StubMarshal
// ----------------------------------------------------------------------

class ICE_API StubMarshal : virtual public Stub
{
public:

protected:

    StubMarshal();
    virtual ~StubMarshal();

private:

    StubMarshal(const StubMarshal&);
    void operator=(const StubMarshal&);
};

}

#endif
