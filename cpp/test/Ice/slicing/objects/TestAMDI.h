// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#ifndef TESTI_H
#define TESTI_H

#include <ServerPrivateAMD.h>
#include <Forward.h>

class TestI : virtual public Test
{
public:

    TestI(const ::Ice::ObjectAdapterPtr&);

    virtual void SBaseAsObject_async(const ::AMD_Test_SBaseAsObjectPtr&, const ::Ice::Current&);
    virtual void SBaseAsSBase_async(const ::AMD_Test_SBaseAsSBasePtr&, const ::Ice::Current&);
    virtual void SBSKnownDerivedAsSBase_async(const ::AMD_Test_SBSKnownDerivedAsSBasePtr&, const ::Ice::Current&);
    virtual void SBSKnownDerivedAsSBSKnownDerived_async(const ::AMD_Test_SBSKnownDerivedAsSBSKnownDerivedPtr&,
	                                                const ::Ice::Current&);

    virtual void SBSUnknownDerivedAsSBase_async(const ::AMD_Test_SBSUnknownDerivedAsSBasePtr&,
	                                        const ::Ice::Current&);

    virtual void SUnknownAsObject_async(const ::AMD_Test_SUnknownAsObjectPtr&, const ::Ice::Current&);

    virtual void oneElementCycle_async(const ::AMD_Test_oneElementCyclePtr&, const ::Ice::Current&);
    virtual void twoElementCycle_async(const ::AMD_Test_twoElementCyclePtr&, const ::Ice::Current&);

    virtual void D1AsB_async(const ::AMD_Test_D1AsBPtr&, const ::Ice::Current&);
    virtual void D1AsD1_async(const ::AMD_Test_D1AsD1Ptr&, const ::Ice::Current&);
    virtual void D2AsB_async(const ::AMD_Test_D2AsBPtr&, const ::Ice::Current&);

    virtual void paramTest1_async(const ::AMD_Test_paramTest1Ptr&, const ::Ice::Current&);
    virtual void paramTest2_async(const ::AMD_Test_paramTest2Ptr&, const ::Ice::Current&);
    virtual void paramTest3_async(const ::AMD_Test_paramTest3Ptr&, const ::Ice::Current&);
    virtual void paramTest4_async(const ::AMD_Test_paramTest4Ptr&, const ::Ice::Current&);

    virtual void returnTest1_async(const ::AMD_Test_returnTest1Ptr&, const ::Ice::Current&);
    virtual void returnTest2_async(const ::AMD_Test_returnTest2Ptr&, const ::Ice::Current&);
    virtual void returnTest3_async(const ::AMD_Test_returnTest3Ptr&, const BPtr&, const BPtr&, const ::Ice::Current&);

    virtual void sequenceTest_async(const ::AMD_Test_sequenceTestPtr&,
	                            const SS1Ptr&, const SS2Ptr&, const ::Ice::Current&);

    virtual void dictionaryTest_async(const ::AMD_Test_dictionaryTestPtr&,
	                              const ::BDict&, const ::Ice::Current&);

    virtual void throwBaseAsBase_async(const ::AMD_Test_throwBaseAsBasePtr&, const ::Ice::Current&);
    virtual void throwDerivedAsBase_async(const ::AMD_Test_throwDerivedAsBasePtr&, const ::Ice::Current&);
    virtual void throwDerivedAsDerived_async(const ::AMD_Test_throwDerivedAsDerivedPtr&, const ::Ice::Current&);
    virtual void throwUnknownDerivedAsBase_async(const ::AMD_Test_throwUnknownDerivedAsBasePtr&, const ::Ice::Current&);

    virtual void useForward_async(const ::AMD_Test_useForwardPtr&, const ::Ice::Current&);

    virtual void shutdown_async(const ::AMD_Test_shutdownPtr&, const ::Ice::Current&);

private:

    const ::Ice::ObjectAdapterPtr& _adapter;
};

#endif
