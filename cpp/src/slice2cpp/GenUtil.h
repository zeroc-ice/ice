// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef GEN_UTIL_H
#define GEN_UTIL_H

#include <Parser.h>
#include <OutputUtil.h>

namespace Slice
{

std::string typeToString(const TypePtr&);
std::string returnTypeToString(const TypePtr&);
std::string inputTypeToString(const TypePtr&);
std::string outputTypeToString(const TypePtr&);
std::string exceptionTypeToString(const TypePtr&);

void writeMarshalUnmarshalCode(Output&, const TypePtr&, const std::string&, bool);
void writeMarshalCode(Output&, const std::list<std::pair<TypePtr, std::string> >&, const TypePtr&);
void writeUnmarshalCode(Output&, const std::list<std::pair<TypePtr, std::string> >&, const TypePtr&);
void writeAllocateCode(Output&,	const std::list<std::pair<TypePtr, std::string> >&, const TypePtr&);

}

#endif
