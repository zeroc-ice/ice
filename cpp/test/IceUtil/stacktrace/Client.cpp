//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/StringUtil.h>
#include <TestHelper.h>

#include <fstream>

using namespace IceUtil;
using namespace std;

namespace IceUtilInternal
{
extern bool ICE_API printStackTraces;
}

namespace
{
class Thrower : public IceUtil::Shared
{
public:

    Thrower() : _idx(0)
    {
    }

    void first()
    {
        _idx++;
        second();
    }

    void second()
    {
        _idx++;
        third();
    }

    void third()
    {
        _idx++;
        forth();
    }

    void forth()
    {
        _idx++;
        fifth();
    }

    void fifth()
    {
        _idx++;
        throw IceUtil::NullHandleException(__FILE__, __LINE__);
    }

private:

    int _idx;
};
typedef IceUtil::Handle<Thrower> ThrowerPtr;

vector<string>
splitLines(const string& str)
{
    vector<string> result;
    istringstream is(str);
    string line;
    while(std::getline(is, line))
    {
        result.push_back(line);
    }
    return result;
}

}

class Client : public Test::TestHelper
{
public:

    virtual void run(int argc, char* argv[]);
};

void
Client::run(int, char*[])
{
    if(IceUtilInternal::stackTraceImpl() == IceUtilInternal::STNone)
    {
        cout << "This Ice build cannot capture stack traces" << endl;
        return;
    }
    IceUtilInternal::printStackTraces = true;

    cout << "checking stacktrace... ";

    ThrowerPtr thrower = new Thrower();
    try
    {
        thrower->first();
    }
    catch(const IceUtil::Exception& ex)
    {
        test(splitLines(ex.ice_stackTrace()).size() >= 3);
    }
    cout << "ok" << endl;
}

DEFINE_TEST(Client);
