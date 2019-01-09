// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_SYSTEMD_JOURNAL_I_H
#define ICE_SYSTEMD_JOURNAL_I_H

#ifdef ICE_USE_SYSTEMD

#include <Ice/Logger.h>

namespace Ice
{

class SystemdJournalI : public Logger
{
public:

    SystemdJournalI(const std::string&);

    virtual void print(const std::string&);
    virtual void trace(const std::string&, const std::string&);
    virtual void warning(const std::string&);
    virtual void error(const std::string&);
    virtual std::string getPrefix();
    virtual LoggerPtr cloneWithPrefix(const std::string&);

private:

    void write(int, const std::string&) const;

    const std::string _prefix;
};

}

#endif

#endif
