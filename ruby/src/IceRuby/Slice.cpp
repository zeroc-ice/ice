// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice.h>
#include <Util.h>
#include <Slice/Preprocessor.h>
#include <Slice/RubyUtil.h>
#include <Slice/Util.h>
#include <IceUtil/Options.h>

using namespace std;
using namespace IceRuby;
using namespace Slice;
using namespace Slice::Ruby;

extern "C"
VALUE
IceRuby_loadSlice(int argc, VALUE* argv, VALUE self)
{
    ICE_RUBY_TRY
    {
        if(argc < 1 || argc > 2)
        {
            throw RubyException(rb_eArgError, "wrong number of arguments");
        }

        string cmd = getString(argv[0]);
        vector<string> argSeq;
        try
        {
            argSeq = IceUtilInternal::Options::split(cmd);
        }
        catch(const IceUtilInternal::BadOptException& ex)
        {
            throw RubyException(rb_eArgError, "error in Slice options: %s", ex.reason.c_str());
        }
        catch(const IceUtilInternal::APIException& ex)
        {
            throw RubyException(rb_eArgError, "error in Slice options: %s", ex.reason.c_str());
        }

        if(argc > 1)
        {
            if(!arrayToStringSeq(argv[1], argSeq))
            {
                throw RubyException(rb_eTypeError, "argument 2 is not an array");
            }
        }

        IceUtilInternal::Options opts;
        opts.addOpt("D", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
        opts.addOpt("U", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
        opts.addOpt("I", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
        opts.addOpt("d", "debug");
        opts.addOpt("", "ice");
        opts.addOpt("", "underscore");
        opts.addOpt("", "checksum");
        opts.addOpt("", "all");

        vector<string> files;
        try
        {
            argSeq.insert(argSeq.begin(), ""); // dummy argv[0]
            files = opts.parse(argSeq);
            if(files.empty())
            {
                throw RubyException(rb_eArgError, "no Slice files specified in `%s'", cmd.c_str());
            }
        }
        catch(const IceUtilInternal::BadOptException& ex)
        {
            throw RubyException(rb_eArgError, "error in Slice options: %s", ex.reason.c_str());
        }
        catch(const IceUtilInternal::APIException& ex)
        {
            throw RubyException(rb_eArgError, "error in Slice options: %s", ex.reason.c_str());
        }

        vector<string> cppArgs;
        vector<string> includePaths;
        bool debug = false;
        bool ice = true; // This must be true so that we can create Ice::Identity when necessary.
        bool underscore = opts.isSet("underscore");
        bool all = false;
        bool checksum = false;
        if(opts.isSet("D"))
        {
            vector<string> optargs = opts.argVec("D");
            for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
            {
                cppArgs.push_back("-D" + *i);
            }
        }
        if(opts.isSet("U"))
        {
            vector<string> optargs = opts.argVec("U");
            for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
            {
                cppArgs.push_back("-U" + *i);
            }
        }
        if(opts.isSet("I"))
        {
            includePaths = opts.argVec("I");
            for(vector<string>::const_iterator i = includePaths.begin(); i != includePaths.end(); ++i)
            {
                cppArgs.push_back("-I" + *i);
            }
        }
        debug = opts.isSet("d") || opts.isSet("debug");
        all = opts.isSet("all");
        checksum = opts.isSet("checksum");

        bool ignoreRedefs = false;

        for(vector<string>::const_iterator p = files.begin(); p != files.end(); ++p)
        {
            string file = *p;
            Slice::PreprocessorPtr icecpp = Slice::Preprocessor::create("icecpp", file, cppArgs);
            FILE* cppHandle = icecpp->preprocess(false, "-D__SLICE2RB__");

            if(cppHandle == 0)
            {
                throw RubyException(rb_eArgError, "Slice preprocessing failed for `%s'", cmd.c_str());
            }

            UnitPtr u = Slice::Unit::createUnit(ignoreRedefs, all, ice, underscore);
            int parseStatus = u->parse(file, cppHandle, debug);

            if(!icecpp->close() || parseStatus == EXIT_FAILURE)
            {
                u->destroy();
                throw RubyException(rb_eArgError, "Slice parsing failed for `%s'", cmd.c_str());
            }

            //
            // Generate the Ruby code into a string stream.
            //
            ostringstream codeStream;
            IceUtilInternal::Output out(codeStream);
            out.setUseTab(false);
            //
            // Ruby magic comment to set the file encoding, it must be first or second line
            //
            out << "# encoding: utf-8\n";
            generate(u, all, checksum, includePaths, out);
            u->destroy();

            string code = codeStream.str();
            callRuby(rb_eval_string, code.c_str());
        }
    }
    ICE_RUBY_CATCH

    return Qnil;
}

extern "C"
VALUE
IceRuby_compile(int argc, VALUE* argv, VALUE self)
{
    ICE_RUBY_TRY
    {
        if(argc != 1)
        {
            throw RubyException(rb_eArgError, "wrong number of arguments");
        }

        vector<string> argSeq;
        if(!arrayToStringSeq(argv[0], argSeq))
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
        catch(const std::exception& ex)
        {
            getErrorStream() << argSeq[0] << ": error:" << ex.what() << endl;
            rc = EXIT_FAILURE;
        }
        catch(const std::string& msg)
        {
            getErrorStream() << argSeq[0] << ": error:" << msg << endl;
            rc = EXIT_FAILURE;
        }
        catch(const char* msg)
        {
            getErrorStream() << argSeq[0] << ": error:" << msg << endl;
            rc = EXIT_FAILURE;
        }
        catch(...)
        {
            getErrorStream() << argSeq[0] << ": error:" << "unknown exception" << endl;
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
