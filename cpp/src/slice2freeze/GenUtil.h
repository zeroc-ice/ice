// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

//
// TODO: This is the same stuff as in ../slice2cpp/GenUtil.cpp. This
// needs to be put into a library!
//

#ifndef GEN_UTIL_H
#define GEN_UTIL_H

#include <Slice/Parser.h>
#include <Slice/OutputUtil.h>

namespace Slice
{

std::string typeToString(const TypePtr&);
std::string returnTypeToString(const TypePtr&);
std::string inputTypeToString(const TypePtr&);

void writeMarshalUnmarshalCode(Output&, const TypePtr&, const std::string&, bool);

}

#endif
