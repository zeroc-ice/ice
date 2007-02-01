// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice.h>
#include <Util.h>
#include <Slice/Preprocessor.h>
#include <Slice/RubyUtil.h>
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
            argSeq = IceUtil::Options::split(cmd);
        }
        catch(const IceUtil::BadOptException& ex)
        {
            throw RubyException(rb_eArgError, "error in Slice options: %s", ex.reason.c_str());
        }
        catch(const IceUtil::APIException& ex)
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

        IceUtil::Options opts;
        opts.addOpt("D", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
        opts.addOpt("U", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
        opts.addOpt("I", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
        opts.addOpt("d", "debug");
        opts.addOpt("", "ice");
        opts.addOpt("", "checksum");
        opts.addOpt("", "all");
        opts.addOpt("", "case-sensitive");

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
        catch(const IceUtil::BadOptException& ex)
        {
            throw RubyException(rb_eArgError, "error in Slice options: %s", ex.reason.c_str());
        }
        catch(const IceUtil::APIException& ex)
        {
            throw RubyException(rb_eArgError, "error in Slice options: %s", ex.reason.c_str());
        }

        string cppArgs;
        vector<string> includePaths;
        bool debug = false;
        bool ice = true; // This must be true so that we can create Ice::Identity when necessary.
        bool caseSensitive = false;
        bool all = false;
        bool checksum = false;
        if(opts.isSet("D"))
        {
            vector<string> optargs = opts.argVec("D");
            for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
            {
                cppArgs += " -D" + *i;
            }
        }
        if(opts.isSet("U"))
        {
            vector<string> optargs = opts.argVec("U");
            for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
            {
                cppArgs += " -U" + *i;
            }
        }
        if(opts.isSet("I"))
        {
            includePaths = opts.argVec("I");
            for(vector<string>::const_iterator i = includePaths.begin(); i != includePaths.end(); ++i)
            {
                cppArgs += " -I" + *i;
            }
        }
        debug = opts.isSet("d") || opts.isSet("debug");
        caseSensitive = opts.isSet("case-sensitive");
        all = opts.isSet("all");
        checksum = opts.isSet("checksum");

        bool ignoreRedefs = false;

        for(vector<string>::const_iterator p = files.begin(); p != files.end(); ++p)
        {
            string file = *p;
            Slice::Preprocessor icecpp("icecpp", file, cppArgs);
            FILE* cppHandle = icecpp.preprocess(false);

            if(cppHandle == 0)
            {
                throw RubyException(rb_eArgError, "Slice preprocessing failed for `%s'", cmd.c_str());
            }

            UnitPtr u = Slice::Unit::createUnit(ignoreRedefs, all, ice, caseSensitive);
            int parseStatus = u->parse(cppHandle, debug);

            if(!icecpp.close() || parseStatus == EXIT_FAILURE)
            {
                u->destroy();
                throw RubyException(rb_eArgError, "Slice parsing failed for `%s'", cmd.c_str());
            }

            //
            // Generate the Ruby code into a string stream.
            //
            ostringstream codeStream;
            IceUtil::Output out(codeStream);
            out.setUseTab(false);
            generate(u, all, checksum, includePaths, out);
            u->destroy();

            string code = codeStream.str();
            callRuby(rb_eval_string, code.c_str());
        }
    }
    ICE_RUBY_CATCH

    return Qnil;
}

void
IceRuby::initSlice(VALUE iceModule)
{
    rb_define_module_function(iceModule, "loadSlice", CAST_METHOD(IceRuby_loadSlice), -1);
}
