// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_SCRIPT_EXCEPTION_H
#define FREEZE_SCRIPT_EXCEPTION_H

#include <IceUtil/Exception.h>

namespace FreezeScript
{

class FailureException : public IceUtil::Exception
{
public:
    
    FailureException(const char*, int, const std::string&);
    virtual ~FailureException() throw();
    virtual std::string ice_name() const;
    virtual void ice_print(std::ostream&) const;
    virtual FailureException* ice_clone() const;
    virtual void ice_throw() const;

    std::string reason() const;

private:

    std::string _reason;
    static const char* _name;
};

} // End of namespace FreezeScript

#endif
