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

    cout << "testing string to command line arguments... " << flush;
    {
        vector<string> args;

        test(Options::split("").empty());

        args = Options::split("\"\"");
        test(args.size() == 1 && args[0].empty());
        args = Options::split("''");
        test(args.size() == 1 && args[0].empty());
        args = Options::split("$''");
        test(args.size() == 1 && args[0].empty());

        args = Options::split("-a -b -c");
        test(args.size() == 3 && args[0] == "-a" && args[1] == "-b" && args[2] == "-c");
        args = Options::split("\"-a\" '-b' $'-c'");
        test(args.size() == 3 && args[0] == "-a" && args[1] == "-b" && args[2] == "-c");
        args = Options::split("  '-b' \"-a\" $'-c' ");
        test(args.size() == 3 && args[0] == "-b" && args[1] == "-a" && args[2] == "-c");
        args = Options::split(" $'-c' '-b' \"-a\"  ");
        test(args.size() == 3 && args[0] == "-c" && args[1] == "-b" && args[2] == "-a");

        // Single quote
        args = Options::split(R"(-Dir='C:\\test\\file')");
        test(args.size() == 1 && args[0] == R"(-Dir=C:\\test\\file)");
        args = Options::split(R"(-Dir='C:\test\file')");
        test(args.size() == 1 && args[0] == R"(-Dir=C:\test\file)");
        args = Options::split(R"(-Dir='C:\test\filewith"quote')");
        test(args.size() == 1 && args[0] == R"(-Dir=C:\test\filewith"quote)");

        // Double quote
        args = Options::split(R"(-Dir="C:\\test\\file")");
        test(args.size() == 1 && args[0] == R"(-Dir=C:\test\file)");
        args = Options::split(R"(-Dir="C:\test\file")");
        test(args.size() == 1 && args[0] == R"(-Dir=C:\test\file)");
        args = Options::split(R"(-Dir="C:\test\filewith\"quote")");
        test(args.size() == 1 && args[0] == R"(-Dir=C:\test\filewith"quote)");

        // ANSI quote
        args = Options::split(R"(-Dir=$'C:\\test\\file')");
        test(args.size() == 1 && args[0] == R"(-Dir=C:\test\file)");
        args = Options::split(R"(-Dir=$'C:\oest\oile')");
        test(args.size() == 1 && args[0] == R"(-Dir=C:\oest\oile)");
        args = Options::split(R"(-Dir=$'C:\oest\oilewith"quote')");
        test(args.size() == 1 && args[0] == R"(-Dir=C:\oest\oilewith"quote)");
        args = Options::split(R"(-Dir=$'\103\072\134\164\145\163\164\134\146\151\154\145')");
        test(args.size() == 1 && args[0] == R"(-Dir=C:\test\file)");
        args = Options::split(R"(-Dir=$'\x43\x3A\x5C\x74\x65\x73\x74\x5C\x66\x69\x6C\x65')");
        test(args.size() == 1 && args[0] == R"(-Dir=C:\test\file)");
        args = Options::split(R"(-Dir=$'\cM\c_')"); // Control characters
        test(args.size() == 1 && args[0] == "-Dir=\015\037");
        args = Options::split(R"(-Dir=$'C:\\\146\x66\cMi')");
        test(args.size() == 1 && args[0] == "-Dir=C:\\ff\015i");
        args = Options::split(R"(-Dir=$'C:\\\cM\x66\146i')");
        test(args.size() == 1 && args[0] == "-Dir=C:\\\015ffi");

        vector<string> badQuoteCommands{
            "\"",
            "'",
            "\\$'",
            "-Dir=\"test",
            "-Dir='test",
            "-Dir=$'test",
            "-Dir=$'test\\c", // trailing \c inside an unterminated $'...' quote
        };
        for (const auto& badQuoteCommand : badQuoteCommands)
        {
            try
            {
                Options::split(badQuoteCommand);
                test(false);
            }
            catch (const BadOptException&)
            {
            }
        }
    }
    cout << "ok" << endl;
}

DEFINE_TEST(Client)
