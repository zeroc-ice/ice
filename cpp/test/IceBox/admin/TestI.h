// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TestFacetI : public virtual ::Test::TestFacet,
#ifndef ICE_CPP11_MAPPING
                   public virtual Ice::PropertiesAdminUpdateCallback,
#endif
                   IceUtil::Monitor<IceUtil::Mutex>
{
public:

    TestFacetI();

    virtual Ice::PropertyDict getChanges(const Ice::Current&);

    virtual void updated(const Ice::PropertyDict&);

private:

    Ice::PropertyDict _changes;
};
ICE_DEFINE_PTR(TestFacetIPtr, TestFacetI);

#endif
