// **********************************************************************
//
// Copyright (c) 2003
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

#ifndef TRANSFORM_EXCEPTION_H
#define TRANSFORM_EXCEPTION_H

#include <IceUtil/Exception.h>

#ifndef TRANSFORM_API
#   ifdef TRANSFORM_API_EXPORTS
#       define TRANSFORM_API ICE_DECLSPEC_EXPORT
#    else
#       define TRANSFORM_API ICE_DECLSPEC_IMPORT
#    endif
#endif

namespace Transform
{

class TRANSFORM_API TransformException : public IceUtil::Exception
{
public:
    
    TransformException(const char*, int, const std::string&);
    virtual std::string ice_name() const;
    virtual void ice_print(std::ostream&) const;
    virtual IceUtil::Exception* ice_clone() const;
    virtual void ice_throw() const;

    std::string reason() const;

private:

    std::string _reason;
};

} // End of namespace Transform

#endif
