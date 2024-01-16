//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TestFacetI : public virtual ::Test::TestFacet,
                   private IceUtil::Monitor<IceUtil::Mutex>
{
public:

    TestFacetI();

    virtual Ice::PropertyDict getChanges(const Ice::Current&);

    virtual void updated(const Ice::PropertyDict&);

private:

    Ice::PropertyDict _changes;
};
ICE_DEFINE_SHARED_PTR(TestFacetIPtr, TestFacetI);

#endif
