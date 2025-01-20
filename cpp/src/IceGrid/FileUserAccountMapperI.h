// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_FILEUSERACCOUNTMAPPERI_H
#define ICEGRID_FILEUSERACCOUNTMAPPERI_H

#include "IceGrid/UserAccountMapper.h"

namespace IceGrid
{
    class FileUserAccountMapperI : public UserAccountMapper
    {
    public:
        FileUserAccountMapperI(const std::string&);

        std::string getUserAccount(std::string, const Ice::Current&) override;

    private:
        std::map<std::string, std::string> _accounts;
    };

};

#endif
