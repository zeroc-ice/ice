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

namespace Slice
{

std::string fullPath(const std::string&);
std::string changeInclude(const std::string&, const std::vector<std::string>&);
void emitError(const std::string&, int, const std::string&);
void emitWarning(const std::string&, int, const std::string&);
void emitError(const std::string&, const std::string&, const std::string&);
void emitWarning(const std::string&, const std::string&, const std::string&);
void emitRaw(const char*);
std::vector<std::string> filterMcppWarnings(const std::string&);
void printGeneratedHeader(IceUtilInternal::Output& out, const std::string&, const std::string& commentStyle = "//");
#ifdef _WIN32
std::vector<std::string> argvToArgs(int argc, wchar_t* argv[]);
#else
std::vector<std::string> argvToArgs(int argc, char* argv[]);
#endif


enum EscapeMode { UCN, Octal, ShortUCN, EC6UCN };

// Parameters:
// const string& value: input string provided by Slice Parser
// const string& nonPrintableEscaped: which of \a, \b, \f, \n, \r, \t, \v, \0 (null), \x20 (\s), \x1b (\e) are
//                                    escaped in the target language
//                                    Warning: don't include \0 if the target language recognizes octal escapes
// const string& printableEscaped: additional printable ASCII characters other than \ and " that need to be escaped
// EscapeMode escapeMode: whether we generate both UCNs, octal escape sequences, only short UCNs (\unnnn),
//                        or ECMAScript 6-style UCNs with \u{...} for astral characters
// unsigned char cutOff: characters < cutOff other than the nonPrintableEscaped are generated as
//                       octal escape sequences, regardless of escapeMode.
std::string
toStringLiteral(const std::string&, const std::string&, const std::string&, EscapeMode, unsigned char);


void
writeDependencies(const std::string&, const std::string&);

}

#endif
