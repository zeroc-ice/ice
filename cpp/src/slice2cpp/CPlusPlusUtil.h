//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef C_PLUS_PLUS_UTIL_H
#define C_PLUS_PLUS_UTIL_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>

namespace Slice
{

extern std::string paramPrefix;

struct ToIfdef
{
    char operator()(char);
};

void printHeader(::IceUtilInternal::Output&);
void printVersionCheck(::IceUtilInternal::Output&);
void printDllExportStuff(::IceUtilInternal::Output&, const std::string&);

const int TypeContextAcceptArrayParam = 1;
const int TypeContextAcceptViewParam = 2;
const int TypeContextUseWstring = 4;

bool isMovable(const TypePtr&);

std::string getUnqualified(const std::string&, const std::string&);

// Gets the C++ type for a Slice parameter or field.
std::string typeToString(const TypePtr&, bool, const std::string& = "", const StringList& = StringList(), int = 0);

// TODO: find a better name.
// Gets the C++ type for a Slice parameter to be marshaled.
std::string inputTypeToString(const TypePtr&, bool, const std::string& = "", const StringList& = StringList(), int = 0);

// TODO: find a better name.
// Gets the C++ type for a Slice out parameter when mapped to a C++ out parameter.
std::string outputTypeToString(const TypePtr&, bool, const std::string& = "", const StringList& = StringList(), int = 0);

std::string operationModeToString(Operation::Mode);
std::string opFormatTypeToString(const OperationPtr&);

std::string fixKwd(const std::string&);

void writeMarshalCode(::IceUtilInternal::Output&, const ParamDeclList&, const OperationPtr&);
void writeUnmarshalCode(::IceUtilInternal::Output&, const ParamDeclList&, const OperationPtr&);
void writeAllocateCode(::IceUtilInternal::Output&, const ParamDeclList&, const OperationPtr&, const std::string&, int);

void writeMarshalUnmarshalAllInHolder(IceUtilInternal::Output&, const std::string&, const DataMemberList&, bool, bool);
void writeStreamHelpers(::IceUtilInternal::Output&, const ContainedPtr&, DataMemberList, bool);
void writeIceTuple(::IceUtilInternal::Output&, DataMemberList, int);

bool findMetaData(const std::string&, const ClassDeclPtr&, std::string&);
bool findMetaData(const std::string&, const StringList&, std::string&);
std::string findMetaData(const StringList&, int = 0);
bool inWstringModule(const SequencePtr&);

}

#endif
