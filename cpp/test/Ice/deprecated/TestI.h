//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TESTI_H
#define TESTI_H

#include "Test.h"

class MyServiceI final : public Test::MyService
{
public:
    MyServiceI();

    void op1(const Ice::Current&) final;
    Test::MyStruct op2(Test::MyClassPtr, const Ice::Current&) final;
    Test::MyEnum op3(std::optional<::std::int32_t>, const Ice::Current&) final;

};

#endif
