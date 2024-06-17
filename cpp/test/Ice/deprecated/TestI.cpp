//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "TestI.h"
#include "Ice/Ice.h"

using namespace std;
using namespace Ice;
using namespace Test;

MyServiceI::MyServiceI()
{
}

void
MyServiceI::op1(const ::Ice::Current& current)
{
}

MyStruct
MyServiceI::op2(MyClassPtr ms, const ::Ice::Current& current)
{
    return MyStruct{ 3, 8 };
}

MyEnum
MyServiceI::op3(::std::optional<::std::int32_t> i, const ::Ice::Current& current)
{
    return MyEnum::Foo;
}
