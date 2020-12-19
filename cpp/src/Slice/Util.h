//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef SLICE_UTIL_H
#define SLICE_UTIL_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>

namespace Slice
{

std::string fullPath(const std::string&);
std::string changeInclude(const std::string&, const std::vector<std::string>&);
void emitFilePrefix(const std::string&, int);
void emitError(const std::string&, int, const std::string&);
void emitWarning(const std::string&, int, const std::string&);
void emitNote(const std::string&, int, const std::string&);
void emitRaw(const char*);
std::vector<std::string> filterMcppWarnings(const std::string&);
void printGeneratedHeader(IceUtilInternal::Output& out, const std::string&, const std::string& commentStyle = "//");
#ifdef _WIN32
std::vector<std::string> argvToArgs(int argc, wchar_t* argv[]);
#else
std::vector<std::string> argvToArgs(int argc, char* argv[]);
#endif

enum EscapeMode { UCN, Octal, ShortUCN, Matlab, EC6UCN };

// Parameters:
// const string& value: input string provided by Slice Parser
// const string& nonPrintableEscaped: which of \a, \b, \f, \n, \r, \t, \v, \0 (null), \x20 (\s), \x1b (\e) are
//                                    escaped in the target language
//                                    Warning: don't include \0 if the target language recognizes octal escapes
// const string& printableEscaped: additional printable ASCII characters other than \ and the string delimiter
//                                 that need to be escaped
// EscapeMode escapeMode: whether we generate both UCNs, octal escape sequences, only short UCNs (\unnnn),
//                        Matlab syntax, or ECMAScript 6-style UCNs with \u{...} for astral characters.
// unsigned char cutOff: characters < cutOff other than the nonPrintableEscaped are generated as
//                       octal escape sequences, regardless of escapeMode.
std::string
toStringLiteral(const std::string&, const std::string&, const std::string&, EscapeMode, unsigned char);

void
writeDependencies(const std::string&, const std::string&);

std::vector<std::string>
splitScopedName(const std::string&);

// TODO
std::string
splitMetadata(const std::string& s, const std::string& delimiter, std::vector<std::string>& result);

// Checks an identifier for illegal syntax and reports any that is present.
bool
checkIdentifier(const std::string&);

bool
checkForRedefinition(const ContainerPtr&, const std::string&, const std::string&);

bool
checkForTaggableType(const TypePtr& type, const std::string& name = "");

bool
ciequals(const std::string& a, const std::string& b);

 // return a or an <s>
std::string
prependA(const std::string& s);

TypePtr unwrapIfOptional(const TypePtr& type);

void unalias(TypePtr& type, StringList& metadata);

enum CaseConvention { SliceCase, CamelCase, PascalCase, SnakeCase };

CaseConvention caseConventionFromString(const std::string&);

std::string camelCase(const std::string&);
std::string pascalCase(const std::string&);
std::string snakeCase(const std::string&);

// Sorts a list of members in place as follows:
// - non-tagged members listed first but kept in the same order
// - tagged members listed last and sorted in tag order
void sortMembers(MemberList& members);

// Returns a pair of lists respectively containing the required members, and tagged members of the provided list.
// Untagged members are kept in their original ordering, and tagged members are sorted by tag.
std::pair<MemberList, MemberList> getSortedMembers(const MemberList& members);

// Returns a new list containing all the members of the provided list that use classes, in their original order.
MemberList getClassTypeMembers(const MemberList& members);

// Returns the size of the bit sequence used to encode the optional elements in this member list.
size_t getBitSequenceSize(const MemberList&);

// Returns true if the compress:args metadata is set for the operation
bool opCompressArgs(const OperationPtr& op);

// Returns true if the compress:return metadata is set for the operation
bool opCompressReturn(const OperationPtr& op);

// Checks if a Slice entity is deprecated and returns the deprecation message if there is one (or a default message if
// there isn't any). If the entity isn't deprecated, this returns the empty string.
std::string getDeprecateReason(const ContainedPtr& p, bool checkContainer = false);

// Parses a raw string of metadata, converting it into a directive and argument pair.
std::pair<std::string, std::string> parseMetadata(const std::string& metadata);

// Parses a list of raw metadata strings, converting it into a map of directives (keys) and arguments (values).
std::map<std::string, std::string> parseMetadata(const StringList& metadata);

// Returns true if `metadata` contains the specified metadata directive.
bool hasMetadata(const std::string& directive, const std::map<std::string, std::string>& metadata);

// Returns any arguments passed to the specified metadata directive if it's present.
// Otherwise it returns a null optional to indicate the metadata isn't set.
std::optional<std::string> findMetadata(const std::string& directive,
                                        const std::map<std::string, std::string>& metadata);

// Adds the metadata from m2 into m1. m2 is not altered by this operation, but m1 is.
// If metadata is repeated, the values in m1 take precedence.
void appendMetadataInPlace(StringList& m1, const StringList& m2);

// Combines the metadata in m1 and m2 into a new StringList. If metadata is repeated, the values in m1 take precedence.
StringList appendMetadata(const StringList& m1, const StringList& m2);

}

#endif
