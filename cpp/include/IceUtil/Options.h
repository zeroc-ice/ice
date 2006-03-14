// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_OPTIONS_H
#define ICE_UTIL_OPTIONS_H

#include <IceUtil/Config.h>
#include <IceUtil/RecMutex.h>
#include <string>
#include <vector>
#include <map>
#include <set>

namespace IceUtil
{

class ICE_UTIL_API Options
{
public:

    struct Error
    {
        Error(const ::std::string& r) : reason(r) {}
        ::std::string reason;
    };

    struct APIError : public Error
    {
        APIError(const ::std::string& r) : Error(r) {}
    };

    struct BadOpt : public Error
    {
        BadOpt(const ::std::string& r) : Error(r) {}
    };

    struct BadQuote : public Error
    {
        BadQuote(const ::std::string& r) : Error(r) {}
    };

    enum LengthType { ShortOpt, LongOpt };
    enum RepeatType { Repeat, NoRepeat };
    enum ArgType { NeedArg, NoArg };

    Options();
    void addOpt(const ::std::string&, const ::std::string& = "",
                ArgType = NoArg, ::std::string = "", RepeatType = NoRepeat);

    static ::std::vector< ::std::string> split(const ::std::string&);
    ::std::vector< ::std::string> parse(const ::std::vector< ::std::string>&);
    ::std::vector< ::std::string> parse(int, const char* const []);
    bool isSet(const ::std::string&) const;
    ::std::string optArg(const ::std::string&) const;
    ::std::vector< ::std::string> argVec(const ::std::string&) const;

private:

    struct OptionDetails
    {
	LengthType length;
	ArgType arg;
        RepeatType repeat;
    };
    typedef ::std::map< ::std::string, OptionDetails> ValidOpts; // Valid options and their details.
    typedef ::std::map< ::std::string, ::std::string> Opts; // Value of non-repeating options.
    typedef ::std::map< ::std::string, ::std::vector< ::std::string> > ROpts; // Value of repeating options.

    void addValidOpt(const ::std::string&, LengthType, ArgType, const ::std::string&, RepeatType);
    ValidOpts::iterator checkOpt(const ::std::string&, LengthType);
    void setOpt(const ::std::string&, const ::std::string&, RepeatType);
    ValidOpts::const_iterator checkOptIsValid(const ::std::string&) const;
    ValidOpts::const_iterator checkOptHasArg(const ::std::string&) const;

    ValidOpts _validOpts;
    Opts _opts;
    ROpts _ropts;

    bool parseCalled;

    RecMutex _m;

    Options(const Options&); // Not allowed.
    void operator=(const Options&); // Not allowed.

    static void checkArgs(const ::std::string&, const ::std::string&, bool, const ::std::string&);
};

}

#endif
