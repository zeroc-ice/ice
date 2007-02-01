// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TESTI_H
#define TESTI_H

#include <ServerPrivateAMD.h>
#include <Forward.h>

class TestI : virtual public ::Test::TestIntf
{
public:

    TestI(const ::Ice::ObjectAdapterPtr&);

    virtual void SBaseAsObject_async(const ::Test::AMD_TestIntf_SBaseAsObjectPtr&, const ::Ice::Current&);
    virtual void SBaseAsSBase_async(const ::Test::AMD_TestIntf_SBaseAsSBasePtr&, const ::Ice::Current&);
    virtual void SBSKnownDerivedAsSBase_async(const ::Test::AMD_TestIntf_SBSKnownDerivedAsSBasePtr&,
                                              const ::Ice::Current&);
    virtual void SBSKnownDerivedAsSBSKnownDerived_async(const ::Test::AMD_TestIntf_SBSKnownDerivedAsSBSKnownDerivedPtr&,
                                                        const ::Ice::Current&);

    virtual void SBSUnknownDerivedAsSBase_async(const ::Test::AMD_TestIntf_SBSUnknownDerivedAsSBasePtr&,
                                                const ::Ice::Current&);

    virtual void SUnknownAsObject_async(const ::Test::AMD_TestIntf_SUnknownAsObjectPtr&, const ::Ice::Current&);

    virtual void oneElementCycle_async(const ::Test::AMD_TestIntf_oneElementCyclePtr&, const ::Ice::Current&);
    virtual void twoElementCycle_async(const ::Test::AMD_TestIntf_twoElementCyclePtr&, const ::Ice::Current&);

    virtual void D1AsB_async(const ::Test::AMD_TestIntf_D1AsBPtr&, const ::Ice::Current&);
    virtual void D1AsD1_async(const ::Test::AMD_TestIntf_D1AsD1Ptr&, const ::Ice::Current&);
    virtual void D2AsB_async(const ::Test::AMD_TestIntf_D2AsBPtr&, const ::Ice::Current&);

    virtual void paramTest1_async(const ::Test::AMD_TestIntf_paramTest1Ptr&, const ::Ice::Current&);
    virtual void paramTest2_async(const ::Test::AMD_TestIntf_paramTest2Ptr&, const ::Ice::Current&);
    virtual void paramTest3_async(const ::Test::AMD_TestIntf_paramTest3Ptr&, const ::Ice::Current&);
    virtual void paramTest4_async(const ::Test::AMD_TestIntf_paramTest4Ptr&, const ::Ice::Current&);

    virtual void returnTest1_async(const ::Test::AMD_TestIntf_returnTest1Ptr&, const ::Ice::Current&);
    virtual void returnTest2_async(const ::Test::AMD_TestIntf_returnTest2Ptr&, const ::Ice::Current&);
    virtual void returnTest3_async(const ::Test::AMD_TestIntf_returnTest3Ptr&, const ::Test::BPtr&, const ::Test::BPtr&,
                                   const ::Ice::Current&);

    virtual void sequenceTest_async(const ::Test::AMD_TestIntf_sequenceTestPtr&,
                                    const ::Test::SS1Ptr&, const ::Test::SS2Ptr&, const ::Ice::Current&);

    virtual void dictionaryTest_async(const ::Test::AMD_TestIntf_dictionaryTestPtr&,
                                      const ::Test::BDict&, const ::Ice::Current&);

    virtual void throwBaseAsBase_async(const ::Test::AMD_TestIntf_throwBaseAsBasePtr&, const ::Ice::Current&);
    virtual void throwDerivedAsBase_async(const ::Test::AMD_TestIntf_throwDerivedAsBasePtr&, const ::Ice::Current&);
    virtual void throwDerivedAsDerived_async(const ::Test::AMD_TestIntf_throwDerivedAsDerivedPtr&,
                                             const ::Ice::Current&);
    virtual void throwUnknownDerivedAsBase_async(const ::Test::AMD_TestIntf_throwUnknownDerivedAsBasePtr&,
                                                 const ::Ice::Current&);

    virtual void useForward_async(const ::Test::AMD_TestIntf_useForwardPtr&, const ::Ice::Current&);

    virtual void shutdown_async(const ::Test::AMD_TestIntf_shutdownPtr&, const ::Ice::Current&);

private:

    const ::Ice::ObjectAdapterPtr& _adapter;
};

#endif
