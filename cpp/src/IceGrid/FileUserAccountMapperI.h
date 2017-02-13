// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_FILEUSERACCOUNTMAPPERI_H
#define ICE_GRID_FILEUSERACCOUNTMAPPERI_H

#include <IceGrid/UserAccountMapper.h>

namespace IceGrid
{

class FileUserAccountMapperI : public UserAccountMapper
{
public:

    FileUserAccountMapperI(const std::string&);

    virtual std::string getUserAccount(const std::string&, const Ice::Current&);
    
private:

    std::map<std::string, std::string> _accounts;
};

};

#endif
