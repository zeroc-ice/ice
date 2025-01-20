// Copyright (c) ZeroC, Inc.

#ifndef ICE_SYSTEMD_JOURNAL_I_H
#define ICE_SYSTEMD_JOURNAL_I_H

#ifdef ICE_USE_SYSTEMD

#    include "Ice/Logger.h"

namespace Ice
{
    class SystemdJournalI final : public Logger
    {
    public:
        SystemdJournalI(std::string);

        void print(const std::string&) final;
        void trace(const std::string&, const std::string&) final;
        void warning(const std::string&) final;
        void error(const std::string&) final;
        std::string getPrefix() final;
        LoggerPtr cloneWithPrefix(std::string) final;

    private:
        void write(int, const std::string&) const;

        const std::string _prefix;
    };
}

#endif

#endif
