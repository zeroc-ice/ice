// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"

ICE_DECLSPEC_IMPORT void consume(const Ice::ObjectPtr&, const Ice::ObjectPrx&);

#if defined(_MSC_VER) && !defined(ICE_DISABLE_PRAGMA_COMMENT)
#    pragma comment(lib, ICE_LIBNAME("consumer"))
#    pragma comment(lib, ICE_LIBNAME("gencode"))
#endif

class TestI : public Test::MyInterface
{
public:
    void op(bool, const Ice::Current&) override;
};

void
TestI::op(bool throwIt, const Ice::Current&)
{
    if (throwIt)
    {
        throw Test::UserError("error message");
    }
}

ICE_DECLSPEC_EXPORT
void
allTests(const Ice::ObjectAdapterPtr& oa)
{
    Test::MyInterfacePtr servant = std::make_shared<TestI>();
    auto proxy = oa->addWithUUID<Test::MyInterfacePrx>(servant);
    consume(servant, proxy);
}
