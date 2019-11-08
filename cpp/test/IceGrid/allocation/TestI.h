//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TestI final : public Test::TestIntf
{
public:

    TestI(std::shared_ptr<Ice::Properties>);

    void shutdown(const Ice::Current&) override;
    std::string getProperty(std::string, const ::Ice::Current&) override;

private:

    std::shared_ptr<Ice::Properties> _properties;
};

#endif
