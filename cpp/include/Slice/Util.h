// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SLICE_UTIL_H
#define SLICE_UTIL_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>

#include <ostream>
#include <fstream>

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
#ifdef _WIN32
SLICE_API std::vector<std::string> argvToArgs(int argc, wchar_t* argv[]);
#else
SLICE_API std::vector<std::string> argvToArgs(int argc, char* argv[]);
#endif

class SLICE_API DependOutputUtil : IceUtil::noncopyable
{
public:

    DependOutputUtil(std::string& file);
    ~DependOutputUtil();
    void cleanup();
    std::ostream& os();

private:

    const std::string _file;
    std::ofstream _os;
};

}

#endif
