//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Slice.h"
#include "Ice/Options.h"
#include "Slice/Preprocessor.h"
#include "Slice/Util.h"
#include "Util.h"
#include "slice2rb/RubyUtil.h"

#include <iostream>

using namespace std;
using namespace IceRuby;
using namespace Slice;
using namespace Slice::Ruby;

extern "C" VALUE
IceRuby_loadSlice(int argc, VALUE* argv, VALUE /*self*/)
{
    ICE_RUBY_TRY
    {
        if (argc < 1 || argc > 2)
        {
            throw RubyException(rb_eArgError, "wrong number of arguments");
        }

        string cmd = getString(argv[0]);
        vector<string> argSeq;
        try
        {
            argSeq = IceInternal::Options::split(cmd);
        }
        catch (const IceInternal::BadOptException& ex)
        {
            throw RubyException(rb_eArgError, "error in Slice options: %s", ex.what());
        }
        catch (const IceInternal::APIException& ex)
        {
            throw RubyException(rb_eArgError, "error in Slice options: %s", ex.what());
        }

        if (argc > 1)
        {
            if (!arrayToStringSeq(argv[1], argSeq))
            {
                throw RubyException(rb_eTypeError, "argument 2 is not an array");
            }
        }

        IceInternal::Options opts;
        opts.addOpt("D", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
        opts.addOpt("U", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
        opts.addOpt("I", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
        opts.addOpt("d", "debug");
        opts.addOpt("", "all");

        vector<string> files;
        try
        {
            argSeq.insert(argSeq.begin(), ""); // dummy argv[0]
            files = opts.parse(argSeq);
            if (files.empty())
            {
                throw RubyException(rb_eArgError, "no Slice files specified in `%s'", cmd.c_str());
            }
        }
        catch (const IceInternal::BadOptException& ex)
        {
            throw RubyException(rb_eArgError, "error in Slice options: %s", ex.what());
        }
        catch (const IceInternal::APIException& ex)
        {
            throw RubyException(rb_eArgError, "error in Slice options: %s", ex.what());
        }

        vector<string> cppArgs;
        vector<string> includePaths;
        bool debug = false;
        bool all = false;
        if (opts.isSet("D"))
        {
            vector<string> optargs = opts.argVec("D");
            for (vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
            {
                cppArgs.push_back("-D" + *i);
            }
        }
        if (opts.isSet("U"))
        {
            vector<string> optargs = opts.argVec("U");
            for (vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
            {
                cppArgs.push_back("-U" + *i);
            }
        }
        if (opts.isSet("I"))
        {
            includePaths = opts.argVec("I");
            for (vector<string>::const_iterator i = includePaths.begin(); i != includePaths.end(); ++i)
            {
                cppArgs.push_back("-I" + *i);
            }
        }
        debug = opts.isSet("d") || opts.isSet("debug");
        all = opts.isSet("all");

        for (vector<string>::const_iterator p = files.begin(); p != files.end(); ++p)
        {
            string file = *p;
            Slice::PreprocessorPtr icecpp = Slice::Preprocessor::create("icecpp", file, cppArgs);
            FILE* cppHandle = icecpp->preprocess(false, "-D__SLICE2RB__");

            if (cppHandle == 0)
            {
                throw RubyException(rb_eArgError, "Slice preprocessing failed for `%s'", cmd.c_str());
            }

            UnitPtr u = Slice::Unit::createUnit("ruby", all);
            int parseStatus = u->parse(file, cppHandle, debug);

            if (!icecpp->close() || parseStatus == EXIT_FAILURE)
            {
                u->destroy();
                throw RubyException(rb_eArgError, "Slice parsing failed for `%s'", cmd.c_str());
            }

            //
            // Generate the Ruby code into a string stream.
            //
            ostringstream codeStream;
            IceInternal::Output out(codeStream);
            out.setUseTab(false);
            //
            // Ruby magic comment to set the file encoding, it must be first or second line
            //
            out << "# encoding: utf-8\n";
            generate(u, all, includePaths, out);
            u->destroy();

            string code = codeStream.str();
            callRuby(rb_eval_string, code.c_str());
        }
    }
    ICE_RUBY_CATCH

    return Qnil;
}

extern "C" VALUE
IceRuby_compile(int argc, VALUE* argv, VALUE /*self*/)
{
    ICE_RUBY_TRY
    {
        if (argc != 1)
        {
            throw RubyException(rb_eArgError, "wrong number of arguments");
        }

        vector<string> argSeq;
        if (!arrayToStringSeq(argv[0], argSeq))
        {
            throw RubyException(rb_eTypeError, "argument is not an array");
        }
        // Manufacture a fake argv[0].
        argSeq.insert(argSeq.begin(), "slice2rb");

        int rc;
        try
        {
            rc = Slice::Ruby::compile(argSeq);
        }
        catch (const std::exception& ex)
        {
            cerr << argSeq[0] << ": error:" << ex.what() << endl;
            rc = EXIT_FAILURE;
        }
        catch (...)
        {
            cerr << argSeq[0] << ": error:"
                 << "unknown exception" << endl;
            rc = EXIT_FAILURE;
        }
        return INT2FIX(rc);
    }
    ICE_RUBY_CATCH

    return Qnil;
}

void
IceRuby::initSlice(VALUE iceModule)
{
    rb_define_module_function(iceModule, "loadSlice", CAST_METHOD(IceRuby_loadSlice), -1);
    rb_define_module_function(iceModule, "compile", CAST_METHOD(IceRuby_compile), -1);
}
