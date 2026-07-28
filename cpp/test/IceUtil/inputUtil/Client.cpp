// Copyright (c) ZeroC, Inc.

#include "../../src/Ice/Options.h"
#include "TestHelper.h"

using namespace std;
using namespace IceInternal;

class Client : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Client::run(int, char**)
{
    cout << "testing command line parsing... " << flush;
    {
        // The argument of a required-argument option can be attached to the option or provided as the next
        // argument.
        Options opts;
        opts.addOpt("d");
        opts.addOpt("I", "", Options::NeedArg, "", Options::Repeat);

        Options::StringVector remaining = opts.parse(Options::StringVector{"prog", "-I", "dir1", "-Idir2", "a.ice"});
        test(!opts.isSet("d"));
        test((opts.argVec("I") == Options::StringVector{"dir1", "dir2"}));
        test((remaining == Options::StringVector{"a.ice"}));
    }
    {
        // A required-argument option at the end of a short option cluster takes the next argument as its
        // argument.
        Options opts;
        opts.addOpt("d");
        opts.addOpt("v");
        opts.addOpt("I", "", Options::NeedArg, "", Options::Repeat);

        Options::StringVector remaining = opts.parse(Options::StringVector{"prog", "-dvI", "dir1", "a.ice"});
        test(opts.isSet("d"));
        test(opts.isSet("v"));
        test((opts.argVec("I") == Options::StringVector{"dir1"}));
        test((remaining == Options::StringVector{"a.ice"}));
    }
    {
        // A required-argument option in the middle of a short option cluster takes the rest of the cluster as
        // its argument.
        Options opts;
        opts.addOpt("d");
        opts.addOpt("I", "", Options::NeedArg, "", Options::Repeat);

        Options::StringVector remaining = opts.parse(Options::StringVector{"prog", "-dIdir1", "a.ice"});
        test(opts.isSet("d"));
        test((opts.argVec("I") == Options::StringVector{"dir1"}));
        test((remaining == Options::StringVector{"a.ice"}));
    }
    {
        // A required-argument option with no argument is an error, including at the end of a short option
        // cluster.
        Options opts;
        opts.addOpt("d");
        opts.addOpt("I", "", Options::NeedArg, "", Options::Repeat);

        try
        {
            opts.parse(Options::StringVector{"prog", "-dI"});
            test(false);
        }
        catch (const BadOptException&)
        {
        }
    }
    cout << "ok" << endl;
}

DEFINE_TEST(Client)
