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

const int TypeContextInParam = 1;
const int TypeContextAMIEnd = 2;
const int TypeContextAMIPrivateEnd = 4;
const int TypeContextAMICallPrivateEnd = 8;
const int TypeContextUseWstring = 16;
const int TypeContextLocal = 32;

bool isMovable(const TypePtr&);

std::string getUnqualified(const std::string&, const std::string&);
std::string typeToString(const TypePtr&, const std::string& = "", const StringList& = StringList(), int = 0);
std::string typeToString(const TypePtr&, bool, const std::string& = "", const StringList& = StringList(), int = 0);
std::string returnTypeToString(const TypePtr&, bool, const std::string& = "", const StringList& = StringList(), int = 0);
std::string inputTypeToString(const TypePtr&, bool, const std::string& = "", const StringList& = StringList(), int = 0);
std::string outputTypeToString(const TypePtr&, bool, const std::string& = "", const StringList& = StringList(), int = 0);
std::string operationModeToString(Operation::Mode);
std::string opFormatTypeToString(const OperationPtr&);

std::string fixKwd(const std::string&);

void writeMarshalCode(::IceUtilInternal::Output&, const MemberList&, const OperationPtr&, bool,
                      const std::string& = "", const std::string& = "");
void writeUnmarshalCode(::IceUtilInternal::Output&, const MemberList&, const OperationPtr&, bool,
                        const std::string& = "", const std::string& = "", const std::string& = "");
void writeAllocateCode(::IceUtilInternal::Output&, const MemberList&, const OperationPtr&, bool, const std::string&,
                       int = 0, const std::string& = "");

void writeMarshalUnmarshalAllInHolder(IceUtilInternal::Output&, const std::string&, const MemberList&, bool, bool);
void writeStreamHelpers(::IceUtilInternal::Output&, const DataMemberContainerPtr&);
void writeIceTuple(::IceUtilInternal::Output&, MemberList, int);

bool findMetadata(const std::string&, const ClassDeclPtr&, std::string&);
bool findMetadata(const std::string&, const StringList&, std::string&);
std::string findMetadata(const StringList&, int = 0);
bool inWstringModule(const SequencePtr&);

std::string getDataMemberRef(const MemberPtr&);
}

#endif
