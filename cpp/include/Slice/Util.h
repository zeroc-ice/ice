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

std::string fullPath(const std::string&);
std::string changeInclude(const std::string&, const std::vector<std::string>&);
void setErrorStream(std::ostream&);
std::ostream& getErrorStream();
void emitError(const std::string&, int, const std::string&);
void emitWarning(const std::string&, int, const std::string&);
void emitError(const std::string&, const std::string&, const std::string&);
void emitWarning(const std::string&, const std::string&, const std::string&);
void emitRaw(const char*);
std::vector<std::string> filterMcppWarnings(const std::string&);
void printGeneratedHeader(IceUtilInternal::Output& out, const std::string&, const std::string& commentStyle = "//");

class DependOutputUtil : IceUtil::noncopyable
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
