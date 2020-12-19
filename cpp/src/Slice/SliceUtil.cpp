//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Slice/Util.h>
#include <Slice/FileTracker.h>
#include <IceUtil/FileUtil.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/StringConverter.h>
#include <IceUtil/ConsoleUtil.h>
#include <climits>
#include <cstring>

#ifndef _MSC_VER
#  include <unistd.h> // For readlink()
#endif

using namespace std;
using namespace Slice;
using namespace IceUtilInternal;

namespace
{

string
normalizePath(const string& path)
{
    string result = path;

    replace(result.begin(), result.end(), '\\', '/');

    string::size_type startReplace = 0;
#ifdef _WIN32
    //
    // For UNC paths we need to ensure they are in the format that is
    // returned by MCPP. IE. "//MACHINE/PATH"
    //
    if(result.find("//") == 0)
    {
        startReplace = 2;
    }
#endif
    string::size_type pos;
    while((pos = result.find("//", startReplace)) != string::npos)
    {
        result.replace(pos, 2, "/");
    }
    pos = 0;
    while((pos = result.find("/./", pos)) != string::npos)
    {
        result.erase(pos, 2);
    }
    pos = 0;
    while((pos = result.find("/..", pos)) != string::npos)
    {
        string::size_type last = result.find_last_of("/", pos - 1);
        if(last != string::npos && result.substr(last, 4) != "/../")
        {
            result.erase(last, pos - last + 3);
            pos = last;
        }
        else
        {
            ++pos;
        }
    }

    if(result.size() > 1) // Remove trailing "/" or "/."
    {
        if(result[result.size() - 1] == '/')
        {
            result.erase(result.size() - 1);
        }
        else if(result[result.size() - 2] == '/' && result[result.size() - 1] == '.')
        {
            result.erase(result.size() - (result.size() == 2 ? 1 : 2));
        }
    }
    return result;
}

}

string
Slice::fullPath(const string& path)
{
    //
    // This function returns the canonicalized absolute pathname of
    // the given path.
    //
    // It is used for instance used to normalize the paths provided
    // with -I options. Like mcpp, we need to follow the symbolic
    // links to ensure changeIncludes works correctly. For example, it
    // must be possible to specify -I/opt/Ice-3.6 where Ice-3.6 is
    // symbolic link to Ice-3.6.0 (if we don't do the same as mcpp,
    // the generated headers will contain a full path...)
    //

    string result = path;
    if(!IceUtilInternal::isAbsolutePath(result))
    {
        string cwd;
        if(IceUtilInternal::getcwd(cwd) == 0)
        {
            result = string(cwd) + '/' + result;
        }
    }

    result = normalizePath(result);

#ifdef _WIN32
    return result;
#else

    string::size_type beg = 0;
    string::size_type next;
    do
    {
        string subpath;
        next = result.find('/', beg + 1);
        if(next == string::npos)
        {
            subpath = result;
        }
        else
        {
            subpath = result.substr(0, next);
        }

        char buf[PATH_MAX + 1];
        int len = static_cast<int>(readlink(subpath.c_str(), buf, sizeof(buf)));
        if(len > 0)
        {
            buf[len] = '\0';
            string linkpath = buf;
            if(!IceUtilInternal::isAbsolutePath(linkpath)) // Path relative to the location of the link
            {
                string::size_type pos = subpath.rfind('/');
                assert(pos != string::npos);
                linkpath = subpath.substr(0, pos + 1) + linkpath;
            }
            result = normalizePath(linkpath) + (next != string::npos ? result.substr(next) : string());
            beg = 0;
            next = 0;
        }
        else
        {
            beg = next;
        }
    }
    while(next != string::npos);
    return result;
#endif
}

string
Slice::changeInclude(const string& p, const vector<string>& includePaths)
{
    string path = normalizePath(p);
    //
    // Compare each include path against the included file and select
    // the path that produces the shortest relative filename.
    //
    string result = path;
    vector<string> paths;
    paths.push_back(path);
    //
    // if path is not a canonical path we also test with its canonical
    // path
    //
    string canonicalPath = fullPath(path);
    if(canonicalPath != path)
    {
        paths.push_back(canonicalPath);
    }

    for(vector<string>::const_iterator i = paths.begin(); i != paths.end(); ++i)
    {
        for(vector<string>::const_iterator j = includePaths.begin(); j != includePaths.end(); ++j)
        {
            if(i->compare(0, j->length(), *j) == 0)
            {
                string s = i->substr(j->length() + 1); // + 1 for the '/'
                if(s.size() < result.size())
                {
                    result = s;
                }
            }
        }

        //
        // If the path has been already shortened no need to test
        // with canonical path.
        //
        if(result != path)
        {
            break;
        }
    }

    result = normalizePath(result); // Normalize the result.

    string::size_type pos;
    if((pos = result.rfind('.')) != string::npos)
    {
        result.erase(pos);
    }

    return result;
}

void
Slice::emitFilePrefix(const string& file, int line)
{
    if (!file.empty())
    {
        consoleErr << file;
        if(line != -1)
        {
            consoleErr << ':' << line;
        }
        consoleErr << ": ";
    }
}

void
Slice::emitError(const string& file, int line, const string& message)
{
    emitFilePrefix(file, line);
    consoleErr << message << endl;
}

void
Slice::emitWarning(const string& file, int line, const string& message)
{
    emitFilePrefix(file, line);
    consoleErr << "warning: " << message << endl;
}

void
Slice::emitNote(const string& file, int line, const string& message)
{
    emitFilePrefix(file, line);
    consoleErr << "note: " << message << endl;
}

void
Slice::emitRaw(const char* message)
{
    consoleErr << message << flush;
}

vector<string>
Slice::filterMcppWarnings(const string& message)
{
    static const char* messages[] =
    {
        "Converted [CR+LF] to [LF]",
        "no newline, supplemented newline",
        0
    };

    static const string warningPrefix = "warning:";
    static const string fromPrefix = "from";
    static const string separators = "\n\t ";

    vector<string> in;
    string::size_type start = 0;
    string::size_type end;
    while((end = message.find('\n', start)) != string::npos)
    {
        in.push_back(message.substr(start, end - start));
        start = end + 1;
    }
    vector<string> out;
    bool skipped;
    for(vector<string>::const_iterator i = in.begin(); i != in.end(); i++)
    {
        skipped = false;

        if(i->find(warningPrefix) != string::npos)
        {
            for(int j = 0; messages[j] != 0; ++j)
            {
                if(i->find(messages[j]) != string::npos)
                {
                    // This line should be skipped it contains the unwanted mcpp warning
                    // next line should also be skipped it contains the slice line that
                    // produces the skipped warning
                    i++;
                    skipped = true;
                    //
                    // Check if next lines are still the same warning
                    //
                    i++;
                    while(i != in.end())
                    {
                        string token = *i;
                        string::size_type index = token.find_first_not_of(separators);
                        if(index != string::npos)
                        {
                            token = token.substr(index);
                        }
                        if(token.find(fromPrefix) != 0)
                        {
                            //
                            // First line not of this warning
                            //
                            i--;
                            break;
                        }
                        else
                        {
                            i++;
                        }
                    }
                    break;
                }
            }
            if(i == in.end())
            {
                break;
            }
        }
        if(!skipped)
        {
            out.push_back(*i + "\n");
        }
    }
    return out;
}

void
Slice::printGeneratedHeader(IceUtilInternal::Output& out, const string& path, const string& comment)
{
    // Get only the file name part of the given path.
    string file = path;
    size_t pos = file.find_last_of("/\\");
    if(string::npos != pos)
    {
        file = file.substr(pos + 1);
    }

    // TODO: reenable <auto-generated> which disables warnings / code analysis with Visual Studio.
    out << comment << "\n";
    out << comment << " Generated from file `" << file << "'" << "\n";
    out << comment << "\n";
    out << comment << " Warning: do not edit this file." << "\n";
    out << comment << "\n";
}

void
Slice::writeDependencies(const string& dependencies, const string& dependFile)
{
    if(dependFile.empty())
    {
        consoleOut << dependencies << flush;
    }
    else
    {
        ofstream of(IceUtilInternal::streamFilename(dependFile).c_str()); // dependFile is a UTF-8 string
        if(!of)
        {
            ostringstream os;
            os << "cannot open file `" << dependFile << "': " << IceUtilInternal::errorToString(errno);
            throw Slice::FileException(__FILE__, __LINE__, os.str());
        }
        of << dependencies;
        of.close();
    }
}

#ifdef _WIN32
vector<string>
Slice::argvToArgs(int argc, wchar_t* argv[])
{
    vector<string> args;
    for(int i = 0; i < argc; i++)
    {
        args.push_back(IceUtil::wstringToString(argv[i]));
    }
    return args;
}
#else
vector<string>
Slice::argvToArgs(int argc, char* argv[])
{
    vector<string> args;
    for(int i = 0; i < argc; i++)
    {
        args.push_back(argv[i]);
    }
    return args;
}
#endif

//
// Split a scoped name into its components and return the components as a list of (unscoped) identifiers.
//
vector<string>
Slice::splitScopedName(const string& scoped)
{
    assert(scoped[0] == ':');
    vector<string> ids;
    string::size_type next = 0;
    string::size_type pos;
    while((pos = scoped.find("::", next)) != string::npos)
    {
        pos += 2;
        if(pos != scoped.size())
        {
            string::size_type endpos = scoped.find("::", pos);
            if(endpos != string::npos)
            {
                ids.push_back(scoped.substr(pos, endpos - pos));
            }
        }
        next = pos;
    }
    if(next != scoped.size())
    {
        ids.push_back(scoped.substr(next));
    }
    else
    {
        ids.push_back("");
    }

    return ids;
}

string Slice::splitMetadata(const string& s, const string& delimiter, vector<string>& result)
{
    size_t startPos = 0;
    size_t endPos;
    size_t delimPos;
    size_t errorPos;

    while(startPos != string::npos)
    {
        // Skip any leading whitespace.
        startPos = s.find_first_not_of(" \t", startPos);

        // If the string has nothing but whitespace left.
        if (startPos == string::npos)
        {
            result.push_back("");
            break;
        }

        // Check if the string is quoted.
        if (s[startPos] == '"')
        {
            // Skip the opening quotation mark.
            endPos = startPos++;

            // Find the closing quotation mark, not counting escaped quotation marks.
            do
            {
                endPos = s.find('"', endPos + 1);
                // If no closing quotation mark was found.
                if (endPos == string::npos)
                {
                    return "unbalanced quotation marks";
                }
            } while(s[endPos - 1] == '\\');

            // Get the position of the next delimiter.
            delimPos = s.find(delimiter, endPos + 1);
            // Ensure there is only whitespace between the closing quotation mark and the next delimiter.
            errorPos = s.find_first_not_of(" \t", endPos + 1);
            if (errorPos != delimPos)
            {
                return "illegal syntax: `" + s.substr(errorPos, (delimPos - errorPos)) +
                    "', only whitespace can appear between quotation marks and delimiters";
            }
        }
        else
        {
            // Get the position of the next delimiter.
            delimPos = s.find(delimiter, startPos);
            // Skip any trailing whitespace.
            endPos = s.find_last_not_of(" \t", delimPos - 1) + 1;

            // Check for errorneous quotation marks in the string.
            errorPos = s.find('"', startPos);
            if (errorPos < endPos)
            {
                return "illegal syntax: `" + s.substr(errorPos, (delimPos - errorPos)) +
                    "', only whitespace can appear between quotation marks and delimiters";
            }
        }

        result.push_back(s.substr(startPos, (endPos - startPos)));
        // If a delimiter was found, skip past it. If a delimiter wasn't found, this sets startPos to npos.
        startPos = delimPos + (delimPos == string::npos ? 0 : delimiter.length());
    }
    return "";
}

bool
Slice::checkIdentifier(const string& id)
{
    // check whether the identifier is scoped
    size_t scopeIndex = id.rfind("::");
    bool isScoped = scopeIndex != string::npos;
    string name;
    if(isScoped)
    {
        name = id.substr(scopeIndex + 2); // Only check the unscoped identifier for syntax
    }
    else
    {
        name = id;
    }

    // Validity is only determined by invalid syntax that triggers errors, not warnings.
    bool isValid = true;

    // check the identifier for reserved suffixes
    static const string suffixBlacklist[] = { "Helper", "Prx", "Ptr" };
    for(size_t i = 0; i < sizeof(suffixBlacklist) / sizeof(*suffixBlacklist); ++i)
    {
        if(name.find(suffixBlacklist[i], name.size() - suffixBlacklist[i].size()) != string::npos)
        {
            unit->warning(ReservedIdentifier, "identifiers with a `" + suffixBlacklist[i] +
                                              "' suffix are reserved by Ice: using `" + name +
                                              "' as an identifier may result in name collisions in the generated code");
        }
    }

    // Check the identifier for illegal ice prefixes
    if(ciequals(name.substr(0, 3), "ice"))
    {
        unit->warning(ReservedIdentifier, "identifiers with an `" + name.substr(0, 3) +
                                          "' prefix are reserved by Ice: using `" + name +
                                          "' as an identifier may result in name collisions in the generated code");
    }

    // check the identifier for illegal underscores
    size_t index = name.find('_');
    if(index == 0)
    {
        unit->error("illegal leading underscore in identifier `" + name + "'");
        isValid = false;
    }
    else if(name.rfind('_') == (name.size() - 1))
    {
        unit->error("illegal trailing underscore in identifier `" + name + "'");
        isValid = false;
    }
    else if(name.find("__") != string::npos)
    {
        unit->error("illegal double underscore in identifier `" + name + "'");
        isValid = false;
    }
    else if(index != string::npos)
    {
        unit->warning(ReservedIdentifier, "identifiers with underscores are reserved by Ice: using `" + name +
                                          "' as an identifier may result in name collisions in the generated code");
    }
    return isValid;
}

bool
Slice::checkForRedefinition(const ContainerPtr& container, const string& name, const string& type)
{
    ContainedList matches = unit->findContents(container->thisScope() + name);
    if (!matches.empty())
    {
        ContainedPtr match = matches.front();
        if (match->name() != name)
        {
            unit->error(type + " `" + name + "' differs only in capitalization from the " + match->kindOf() + " named `"
                        + match->name() + "'");
            unit->note(match, match->kindOf() + " `" + match->name() + "' is defined here");
            return false;
        }
        else if (match->kindOf() != type)
        {
            unit->error(type + " `" + name + "' was previously defined as " + prependA(match->kindOf()));
            unit->note(match, match->kindOf() + " `" + name + "' was originally defined here");
            return false;
        }
        else if (type != "module") // Modules can be re-opened if they have the same name.
        {
            unit->error("redefinition of " + type + " `" + name + "'");
            unit->note(match, type + " `" + name + "' was originally defined here");
            return false;
        }
    }
    return true;
}

bool
Slice::checkForTaggableType(const TypePtr& type, const string& name)
{
    std::string message;
    if (type->isClassType())
    {
        message = " cannot be tagged, since its type '" + type->typeId() + "' is a class";
    }
    else if (type->usesClasses())
    {
        message = " cannot be tagged, since its type '" + type->typeId() + "' contains classes";
    }
    else
    {
        return true;
    }

    if (!name.empty())
    {
        message = " '" + name + "'" + message;
    }

    ContainerPtr container = unit->currentContainer();
    if (OperationPtr::dynamicCast(container))
    {
        unit->error("parameter" + message);
    }
    else if (InterfaceDefPtr::dynamicCast(container))
    {
        unit->error("return type" + message);
    }
    else
    {
        unit->error("data member" + message);
    }
    return false;
}

bool
Slice::ciequals(const string& lhs, const string& rhs)
{
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), [](char a, char b)
                                                                      {
                                                                          return tolower(a) == tolower(b);
                                                                      });
}

string
Slice::prependA(const string& s)
{
    if (string("aeiou").find_first_of(s[0]) != string::npos)
    {
        return "an " + s;
    }
    return "a " + s;
}

TypePtr
Slice::unwrapIfOptional(const TypePtr& type)
{
    if (auto optional = OptionalPtr::dynamicCast(type))
    {
        return optional->underlying();
    }
    return type;
}

void
Slice::unalias(TypePtr& type, StringList& metadata)
{
    if (auto alias = TypeAliasPtr::dynamicCast(unwrapIfOptional(type)))
    {
        appendMetadataInPlace(metadata, alias->typeMetadata());
        if (OptionalPtr::dynamicCast(type))
        {
            type = new Optional(alias->underlying());
        }
        else
        {
            type = alias->underlying();
        }
    }
}

CaseConvention
Slice::caseConventionFromString(const std::string& conventionName)
{
    if(conventionName == "snake_case")
    {
        return SnakeCase;
    }
    else if(conventionName == "PascalCase")
    {
        return PascalCase;
    }
    else if(conventionName == "camelCase")
    {
        return CamelCase;
    }
    else if(conventionName == "SliceCase")
    {
        return SliceCase;
    }
    else
    {
        throw invalid_argument("usupported case convention `" + conventionName + "'");
    }
}

std::string
Slice::camelCase(const std::string& name)
{
    ostringstream os;
    bool nextIsUpper = false;
    for(string::size_type i = 0; i < name.size(); ++i)
    {
        if(i == 0 && isupper(name[i]))
        {
            os << static_cast<char>(tolower(name[i]));
        }
        else if(name[i] == '_')
        {
            nextIsUpper = true;
        }
        else if(nextIsUpper)
        {
            nextIsUpper = false;
            os << static_cast<char>(toupper(name[i]));
        }
        else
        {
            os << name[i];
        }
    }
    return os.str();
}

std::string
Slice::pascalCase(const std::string& name)
{
    ostringstream os;
    bool nextIsUpper = false;
    for(string::size_type i = 0; i < name.size(); ++i)
    {
        if(i == 0 && islower(name[i]))
        {
            os << static_cast<char>(toupper(name[i]));
        }
        else if(name[i] == '_')
        {
            nextIsUpper = true;
        }
        else if(nextIsUpper)
        {
            nextIsUpper = false;
            os << static_cast<char>(toupper(name[i]));
        }
        else
        {
            os << name[i];
        }
    }
    return os.str();
}

std::string
Slice::snakeCase(const std::string& name)
{
    ostringstream os;
    for(string::size_type i = 0; i < name.size(); ++i)
    {
        if(isupper(name[i]))
        {
            if(i > 0)
            {
                os << '_';
            }
            os << static_cast<char>(tolower(name[i]));
        }
        else
        {
            os << name[i];
        }
    }
    return os.str();
}

void
Slice::sortMembers(MemberList& members)
{
    members.sort([](const MemberPtr& lhs, const MemberPtr& rhs) -> bool
    {
        if (lhs->tagged() && rhs->tagged())
        {
            return lhs->tag() < rhs->tag();
        }
        else if (!lhs->tagged() && !rhs->tagged())
        {
            // sort keeps the order of unordered elements
            return false;
        }
        else
        {
            // If only one is tagged, lhs < rhs when lhs is not tagged.
            return !lhs->tagged();
        }
    });
}

pair<MemberList, MemberList>
Slice::getSortedMembers(const MemberList& members)
{
    MemberList required, tagged;
    for (const auto& member : members)
    {
        (member->tagged() ? tagged : required).push_back(member);
    }
    sortMembers(tagged);
    return make_pair(move(required), move(tagged));
}

MemberList
Slice::getClassTypeMembers(const MemberList& members)
{
    MemberList result;
    for (const auto& member : members)
    {
        if (member->type()->isClassType())
        {
            result.push_back(member);
        }
    }
    return result;
}

size_t
Slice::getBitSequenceSize(const MemberList& members)
{
    size_t result = 0;
    for (const auto& member : members)
    {
        if (auto optional = OptionalPtr::dynamicCast(member->type());
            optional && !member->tagged() && optional->encodedUsingBitSequence())
        {
            result++;
        }
    }
    return result;
}

namespace
{

bool opCompress(const OperationPtr& op, bool args)
{
    string direction = args ? "args" : "return";
    string prefix = "compress:";
    string compress = op->findMetadataWithPrefix(prefix);

    if (compress.empty())
    {
        return false;
    }
    vector<string> directions;
    splitMetadata(compress, ",", directions);
    return find(directions.begin(), directions.end(), direction) != directions.end();
}

}

bool Slice::opCompressArgs(const OperationPtr& op)
{
    return opCompress(op, true);
}

bool Slice::opCompressReturn(const OperationPtr& op)
{
    return opCompress(op, false);
}

string Slice::getDeprecateReason(const ContainedPtr& p, bool checkContainer)
{
    // Check if there was `deprecate` metadata directly on the entity.
    auto reason = p->findMetadata("deprecate");
    // If the entity wasn't directly deprecated, check if it's container is.
    if (!reason && checkContainer)
    {
        if (ContainedPtr p2 = ContainedPtr::dynamicCast(p->container()))
        {
            reason = p2->findMetadata("deprecate");
        }
    }

    if (reason)
    {
        if (reason->empty())
        {
            return "This " + p->kindOf() + " has been deprecated";
        }
        return *reason;
    }
    return "";
}

pair<string, string> Slice::parseMetadata(const string& metadata)
{
    auto start = metadata.find("(");
    if (start == string::npos)
    {
        return make_pair(metadata, "");
    }

    // We subtract 1 under the assumption that it ends with a ')' which shouldn't be included.
    auto length = metadata.length() - start - 1;
    // Check the raw string ends with a closing parentheses.
    if (metadata.at(length + start) != ')')
    {
        unit->error("missing closing parentheses for metadata arguments");
        length += 1;
    }
    return make_pair(metadata.substr(0, start), metadata.substr(start + 1, length));
}

map<string, string> Slice::parseMetadata(const StringList& metadata)
{
    map<string, string> parsedMetadata;
    for (const auto& m : metadata)
    {
        parsedMetadata.insert(parseMetadata(m));
    }
    return parsedMetadata;
}

bool Slice::hasMetadata(const string& directive, const map<string, string>& metadata)
{
    return metadata.find(directive) != metadata.end();
}

optional<string> Slice::findMetadata(const string& directive, const map<string, string>& metadata)
{
    auto match = metadata.find(directive);
    return (match != metadata.end() ? make_optional(match->second) : nullopt);
}

void Slice::appendMetadataInPlace(StringList& m1, const StringList& m2)
{
    m1.insert(m1.end(), m2.begin(), m2.end());
}

StringList Slice::appendMetadata(const StringList& m1, const StringList& m2)
{
    StringList result = m1;
    appendMetadataInPlace(result, m2);
    return result;
}
