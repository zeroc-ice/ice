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

namespace IceLang
{

std::string typeToString(const Type_ptr&);
std::string returnTypeToString(const Type_ptr&);
std::string inputTypeToString(const Type_ptr&);
std::string outputTypeToString(const Type_ptr&);

void writeMarshalUnmarshalCode(Output&, const Type_ptr&, const std::string&, bool);

void writeMarshalCode(Output&,
		      const std::list<std::pair<Type_ptr, std::string> >&,
		      const Type_ptr&);

void writeUnmarshalCode(Output&,
			const std::list<std::pair<Type_ptr, std::string> >&,
			const Type_ptr&);

void writeAllocateCode(Output&,
			const std::list<std::pair<Type_ptr, std::string> >&,
			const Type_ptr&);

}

#endif
