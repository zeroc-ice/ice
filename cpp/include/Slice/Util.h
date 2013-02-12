// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SLICE_UTIL_H
#define SLICE_UTIL_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>

namespace Slice
{

SLICE_API std::string fullPath(const std::string&);
SLICE_API std::string changeInclude(const std::string&, const std::vector<std::string>&);
SLICE_API void setErrorStream(std::ostream&);
SLICE_API std::ostream& getErrorStream();
SLICE_API void emitError(const std::string&, int, const std::string&);
SLICE_API void emitWarning(const std::string&, int, const std::string&);
SLICE_API void emitError(const std::string&, const std::string&, const std::string&);
SLICE_API void emitWarning(const std::string&, const std::string&, const std::string&);
SLICE_API void emitRaw(const char*);
SLICE_API std::vector<std::string> filterMcppWarnings(const std::string&);
SLICE_API void printGeneratedHeader(IceUtilInternal::Output& out, const std::string&, const std::string& commentStyle = "//");

}

#endif
