// **********************************************************************
//
// Copyright (c) 2003
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

#ifndef TESTAMDI_H
#define TESTAMDI_H

#include <ServerPrivateAMD.h>

class TestI : virtual public Test
{
public:

    TestI(const ::Ice::ObjectAdapterPtr&);
    virtual void baseAsBase_async(const ::AMD_Test_baseAsBasePtr&, const ::Ice::Current&);

    virtual void unknownDerivedAsBase_async(const ::AMD_Test_unknownDerivedAsBasePtr&, const ::Ice::Current&);
    virtual void knownDerivedAsBase_async(const ::AMD_Test_knownDerivedAsBasePtr&, const ::Ice::Current&);
    virtual void knownDerivedAsKnownDerived_async(const ::AMD_Test_knownDerivedAsKnownDerivedPtr&,
	                                          const ::Ice::Current&);

    virtual void unknownIntermediateAsBase_async(const ::AMD_Test_unknownIntermediateAsBasePtr&, const ::Ice::Current&);
    virtual void knownIntermediateAsBase_async(const ::AMD_Test_knownIntermediateAsBasePtr&, const ::Ice::Current&);
    virtual void knownMostDerivedAsBase_async(const ::AMD_Test_knownMostDerivedAsBasePtr&, const ::Ice::Current&);
    virtual void knownIntermediateAsKnownIntermediate_async(const ::AMD_Test_knownIntermediateAsKnownIntermediatePtr&,
	                                                    const ::Ice::Current&);
    virtual void knownMostDerivedAsKnownIntermediate_async(const ::AMD_Test_knownMostDerivedAsKnownIntermediatePtr&,
	                                                   const ::Ice::Current&);
    virtual void knownMostDerivedAsKnownMostDerived_async(const ::AMD_Test_knownMostDerivedAsKnownMostDerivedPtr&,
	                                                  const ::Ice::Current&);

    virtual void unknownMostDerived1AsBase_async(const ::AMD_Test_unknownMostDerived1AsBasePtr&, const ::Ice::Current&);
    virtual void unknownMostDerived1AsKnownIntermediate_async(
					    const ::AMD_Test_unknownMostDerived1AsKnownIntermediatePtr&,
					    const ::Ice::Current&);
    virtual void unknownMostDerived2AsBase_async(const ::AMD_Test_unknownMostDerived2AsBasePtr&, const ::Ice::Current&);

    virtual void shutdown_async(const ::AMD_Test_shutdownPtr&, const ::Ice::Current&);

private:

    const ::Ice::ObjectAdapterPtr& _adapter;
};

#endif
