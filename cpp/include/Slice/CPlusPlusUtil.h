// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef C_PLUS_PLUS_UTIL_H
#define C_PLUS_PLUS_UTIL_H

#include <Slice/Parser.h>
#include <Slice/OutputUtil.h>

namespace Slice
{

struct ToIfdef
{
    char operator()(char);
};

std::string changeInclude(const std::string&, const std::vector<std::string>&);
void printHeader(Output&);
void printVersionCheck(Output&);
void printDllExportStuff(Output&, const std::string&);

std::string typeToString(const TypePtr&);
std::string returnTypeToString(const TypePtr&);
std::string inputTypeToString(const TypePtr&);
std::string outputTypeToString(const TypePtr&);

void writeMarshalUnmarshalCode(Output&, const TypePtr&, const std::string&, bool, const std::string& = "",
			       bool = true);
void writeMarshalCode(Output&, const std::list<std::pair<TypePtr, std::string> >&, const TypePtr&);
void writeUnmarshalCode(Output&, const std::list<std::pair<TypePtr, std::string> >&, const TypePtr&);
void writeAllocateCode(Output&,	const std::list<std::pair<TypePtr, std::string> >&, const TypePtr&);

}

#endif
