// **********************************************************************
//
// Copyright (c) 2004
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef FREEZE_SCRIPT_EXCEPTION_H
#define FREEZE_SCRIPT_EXCEPTION_H

#include <IceUtil/Exception.h>

namespace FreezeScript
{

class Exception : public IceUtil::Exception
{
public:
    
    Exception(const char*, int, const std::string&);
    virtual const std::string& ice_name() const;
    virtual void ice_print(std::ostream&) const;
    virtual IceUtil::Exception* ice_clone() const;
    virtual void ice_throw() const;

    std::string reason() const;

private:

    std::string _reason;
    static std::string _name;
};

} // End of namespace FreezeScript

#endif
