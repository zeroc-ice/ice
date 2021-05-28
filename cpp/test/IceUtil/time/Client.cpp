//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/Time.h>
#include <TestHelper.h>

using namespace IceUtil;
using namespace std;

class Client : public Test::TestHelper
{
public:

    virtual void run(int argc, char* argv[]);
};

void
Client::run(int argc, char*[])
{
    if(argc > 1)
    {
        throw std::invalid_argument("too many arguments");
    }

    cerr << "testing Time::toString... " << flush;
    Time t = Time::now();
    test(!t.toDateTime().empty());
    // Add forty years to current time to ensure toDateTime works for large values
    // see https://github.com/zeroc-ice/ice/issues/1283
    t = Time::seconds(t.toSeconds() + (40 * 365 * 24 * 3600));
    test(!t.toDateTime().empty());
    cerr << "ok" << endl;
}

DEFINE_TEST(Client);
