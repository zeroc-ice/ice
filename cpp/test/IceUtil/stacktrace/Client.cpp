// Copyright (c) ZeroC, Inc.

#include "Ice/StringConverter.h"
#include "Ice/StringUtil.h"
#include "TestHelper.h"

#include <fstream>
#include <sstream>

using namespace Ice;
using namespace std;

namespace
{
    class Thrower
    {
    public:
        Thrower() = default;

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
            throw Ice::IllegalConversionException{__FILE__, __LINE__, "error message"};
        }

    private:
        int _idx{0};
    };
    using ThrowerPtr = shared_ptr<Thrower>;

    vector<string> splitLines(const string& str)
    {
        vector<string> result;
        istringstream is(str);
        string line;
        while (std::getline(is, line))
        {
            result.push_back(line);
        }
        return result;
    }
}

class Client final : public Test::TestHelper
{
public:
    void run(int argc, char* argv[]) final;
};

void
Client::run(int, char*[])
{
    Ice::LocalException::ice_enableStackTraceCollection();

    // We assume this test is executed only on platforms/builds with support for stack trace collection.

    cout << "checking stacktrace... ";

    ThrowerPtr thrower = make_shared<Thrower>();
    try
    {
        thrower->first();
    }
    catch (const Ice::LocalException& ex)
    {
        // cerr << ex << endl;
        test(splitLines(ex.ice_stackTrace()).size() >= 3);
    }
    cout << "ok" << endl;
}

DEFINE_TEST(Client);
