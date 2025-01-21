// Copyright (c) ZeroC, Inc.

#ifndef ICE_REQUEST_FAILED_MESSAGE_H
#define ICE_REQUEST_FAILED_MESSAGE_H

#include "Ice/Identity.h"
#include "Ice/Initialize.h"

#include <sstream>
#include <string>

namespace IceInternal
{
    inline std::string createRequestFailedMessage(
        const char* typeName,
        const Ice::Identity& id,
        const std::string& facet,
        const std::string& operation)
    {
        std::ostringstream os;
        os << "dispatch failed with " << typeName;
        if (!id.name.empty())
        {
            os << "\nidentity: '" << Ice::identityToString(id, Ice::ToStringMode::Unicode) << "'";
            os << "\nfacet: " << facet;
            os << "\noperation: " << operation;
        }
        return os.str();
    }
}

#endif
