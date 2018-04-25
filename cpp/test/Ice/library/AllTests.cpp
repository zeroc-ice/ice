// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Test.h>

ICE_DECLSPEC_IMPORT void
consume(const Ice::ObjectPtr&, const Ice::ObjectPrxPtr&);

#if defined(_MSC_VER)
#   pragma comment(lib, ICE_LIBNAME("consumer"))
#   pragma comment(lib, ICE_LIBNAME("gencode"))
#endif

class TestI : public Test::MyInterface
{
public:

    void op(bool, const Ice::Current&);
};

void
TestI::op(bool throwIt, const Ice::Current&)
{
    if(throwIt)
    {
        throw Test::UserError("error message");
    }
}

ICE_DECLSPEC_EXPORT
void allTests(const Ice::ObjectAdapterPtr& oa)
{
    Test::MyInterfacePtr servant = ICE_MAKE_SHARED(TestI);
    Test::MyInterfacePrxPtr proxy = ICE_UNCHECKED_CAST(Test::MyInterfacePrx, oa->addWithUUID(servant));
    consume(servant, proxy);
}
