// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#ifndef ICE_STRING_UTIL_H
#define ICE_STRING_UTIL_H

#include <IceUtil/Config.h>

namespace IceUtil
{

//
// Add escape sequences (like "\n", or "\0xxx") to make a string
// readable in ASCII.
//
ICE_UTIL_API std::string escapeString(const std::string&, const std::string&);

//
// Remove escape sequences added by escapeString.
//
ICE_UTIL_API bool unescapeString(const std::string&, std::string::size_type, std::string::size_type, std::string&);

//
// If a single or double quotation mark is found at the start
// position, then the position of the matching closing quote is
// returned. If no quotation mark is found at the start position, then
// 0 is returned. If no matching closing quote is found, then
// std::string::npos is returned.
//
ICE_UTIL_API std::string::size_type checkQuote(const std::string&, std::string::size_type = 0);

}

#endif
