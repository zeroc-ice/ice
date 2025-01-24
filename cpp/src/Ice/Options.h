// Copyright (c) ZeroC, Inc.

#ifndef ICE_OPTIONS_H
#define ICE_OPTIONS_H

#include "Ice/Config.h"
#include "Ice/LocalException.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace IceInternal
{
    class ICE_API APIException final : public Ice::LocalException
    {
    public:
        using Ice::LocalException::LocalException;

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    class ICE_API BadOptException final : public Ice::LocalException
    {
    public:
        using Ice::LocalException::LocalException;

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    class ICE_API Options
    {
    public:
        enum LengthType
        {
            ShortOpt,
            LongOpt
        };
        enum RepeatType
        {
            Repeat,
            NoRepeat
        };
        enum ArgType
        {
            NeedArg,
            NoArg
        };

        Options();

        Options(const Options&) = delete;
        void operator=(const Options&) = delete;

        void addOpt(
            const std::string&,
            const std::string& = "",
            ArgType = NoArg,
            const std::string& = "",
            RepeatType = NoRepeat);

        using StringVector = std::vector<std::string>;

        static StringVector split(const std::string&);
        StringVector parse(const StringVector&);
        StringVector parse(int, const char* const[]);
        [[nodiscard]] bool isSet(const std::string&) const;
        [[nodiscard]] std::string optArg(const std::string&) const;
        [[nodiscard]] StringVector argVec(const std::string&) const;

    private:
        struct OptionDetails
        {
            LengthType length;
            ArgType arg;
            RepeatType repeat;
            bool hasDefault;
        };
        using ODPtr = std::shared_ptr<OptionDetails>;

        struct OptionValue
        {
            std::string val;
        };
        using OValPtr = std::shared_ptr<OptionValue>;

        struct OptionValueVector
        {
            std::vector<std::string> vals;
        };
        using OVecPtr = std::shared_ptr<OptionValueVector>;

        using ValidOpts = std::map<std::string, ODPtr>;      // Valid options and their details.
        using Opts = std::map<std::string, OValPtr>;         // Value of non-repeating options.
        using ROpts = std::map<std::string, OVecPtr>;        // Value of repeating options.
        using Synonyms = std::map<std::string, std::string>; // Map from short to long option and vice versa.

        void addValidOpt(const std::string&, const std::string&, ArgType, const std::string&, RepeatType);
        ValidOpts::iterator checkOpt(const std::string&, LengthType);
        void setOpt(const std::string&, const std::string&, const std::string&, RepeatType);
        void setNonRepeatingOpt(const std::string&, const std::string&);
        void setRepeatingOpt(const std::string&, const std::string&);
        [[nodiscard]] ValidOpts::const_iterator checkOptIsValid(const std::string&) const;
        [[nodiscard]] ValidOpts::const_iterator checkOptHasArg(const std::string&) const;
        void updateSynonyms(const std::string&, const std::string&);
        [[nodiscard]] std::string getSynonym(const std::string&) const;

        ValidOpts _validOpts;
        Opts _opts;
        ROpts _ropts;
        Synonyms _synonyms;

        bool parseCalled{false};

        static void checkArgs(const std::string&, const std::string&, bool, const std::string&);
    };
}

#endif
