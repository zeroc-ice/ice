//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "CPlusPlusUtil.h"
#include "../Slice/Util.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <functional>

#ifndef _WIN32
#    include <fcntl.h>
#endif

using namespace std;
using namespace Slice;
using namespace IceInternal;

namespace
{
    string stringTypeToString(const TypePtr&, const StringList& metaData, TypeContext typeCtx)
    {
        string strType = findMetaData(metaData, typeCtx);

        if (strType == "")
        {
            strType = (typeCtx & TypeContext::UseWstring) != TypeContext::None ? "::std::wstring" : "::std::string";
        }
        else
        {
            assert(strType == "string" || strType == "wstring");
            strType = "::std::" + strType;
        }

        if ((typeCtx & TypeContext::MarshalParam) != TypeContext::None)
        {
            strType += "_view";
        }
        return strType;
    }

    string
    sequenceTypeToString(const SequencePtr& seq, const string& scope, const StringList& metaData, TypeContext typeCtx)
    {
        string seqType = findMetaData(metaData, typeCtx);
        if (!seqType.empty())
        {
            if (seqType == "%array")
            {
                BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(seq->type());
                string s;
                if (builtin && builtin->kind() == Builtin::KindByte)
                {
                    s = "std::byte";
                }
                else
                {
                    s = typeToString(
                        seq->type(),
                        false,
                        scope,
                        seq->typeMetaData(),
                        typeCtx | (inWstringModule(seq) ? TypeContext::UseWstring : TypeContext::None));
                }
                return "::std::pair<const " + s + "*, const " + s + "*>";
            }
            else
            {
                return seqType;
            }
        }
        else
        {
            return getUnqualified(fixKwd(seq->scoped()), scope);
        }
    }

    string dictionaryTypeToString(
        const DictionaryPtr& dict,
        const string& scope,
        const StringList& metaData,
        TypeContext typeCtx)
    {
        const string dictType = findMetaData(metaData, typeCtx);
        if (dictType.empty())
        {
            return getUnqualified(fixKwd(dict->scoped()), scope);
        }
        else
        {
            return dictType;
        }
    }

    // Do we pass this type by value when it's an input parameter?
    bool inputParamByValue(const TypePtr& type, const StringList& metaData)
    {
        BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
        if ((builtin && (!builtin->isVariableLength() || builtin->kind() == Builtin::KindString)))
        {
            return true;
        }
        if (dynamic_pointer_cast<Enum>(type))
        {
            return true;
        }
        if (dynamic_pointer_cast<Sequence>(type) || dynamic_pointer_cast<Dictionary>(type))
        {
            static const string prefix = "cpp:";

            // Return true for view-type (sequence and dictionary) and array (sequence only)
            for (const auto& str : metaData)
            {
                if (str.find(prefix) == 0)
                {
                    string::size_type pos = str.find(':', prefix.size());
                    if (pos != string::npos)
                    {
                        string ss = str.substr(prefix.size());
                        if (ss.find("view-type:") == 0)
                        {
                            return true;
                        }
                        // else check remaining meta data
                    }
                    else
                    {
                        if (str.substr(prefix.size()) == "array")
                        {
                            return true;
                        }
                        // else check remaining meta data
                    }
                }
            }
        }
        return false;
    }

    void writeParamAllocateCode(
        Output& out,
        const TypePtr& type,
        bool optional,
        const string& scope,
        const string& fixedName,
        const StringList& metaData,
        TypeContext typeCtx)
    {
        string s = typeToString(type, optional, scope, metaData, typeCtx);
        out << nl << s << ' ' << fixedName << ';';
    }

    void writeMarshalUnmarshalParams(Output& out, const ParamDeclList& params, const OperationPtr& op, bool marshal)
    {
        const string returnValueS = "ret";
        const string stream = marshal ? "ostr" : "istr";

        // True when unmarshaling a tuple response.
        bool tuple = !marshal && op && (params.size() + (op->returnType() ? 1 : 0)) > 1;

        //
        // Marshal non optional parameters.
        //
        ParamDeclList requiredParams;
        ParamDeclList optionals;
        for (ParamDeclList::const_iterator p = params.begin(); p != params.end(); ++p)
        {
            if ((*p)->optional())
            {
                optionals.push_back(*p);
            }
            else
            {
                requiredParams.push_back(*p);
            }
        }

        int retOffset = op && op->returnType() ? 1 : 0;

        if (!requiredParams.empty() || (op && op->returnType() && !op->returnIsOptional()))
        {
            out << nl;
            if (marshal)
            {
                out << stream << "->writeAll";
            }
            else
            {
                out << stream << "->readAll";
            }
            out << spar;
            for (ParamDeclList::const_iterator p = requiredParams.begin(); p != requiredParams.end(); ++p)
            {
                if (tuple)
                {
                    auto index = std::distance(params.begin(), std::find(params.begin(), params.end(), *p)) + retOffset;
                    out << "::std::get<" + std::to_string(index) + ">(v)";
                }
                else
                {
                    out << fixKwd(paramPrefix + (*p)->name());
                }
            }
            if (op && op->returnType() && !op->returnIsOptional())
            {
                if (tuple)
                {
                    out << "::std::get<0>(v)";
                }
                else
                {
                    out << returnValueS;
                }
            }
            out << epar << ";";
        }

        if (!optionals.empty() || (op && op->returnType() && op->returnIsOptional()))
        {
            //
            // Sort optional parameters by tag.
            //
            class SortFn
            {
            public:
                static bool compare(const ParamDeclPtr& lhs, const ParamDeclPtr& rhs)
                {
                    return lhs->tag() < rhs->tag();
                }
            };
            optionals.sort(SortFn::compare);

            out << nl;
            if (marshal)
            {
                out << stream << "->writeAll";
            }
            else
            {
                out << stream << "->readAll";
            }
            out << spar;

            {
                //
                // Tags
                //
                ostringstream os;
                os << '{';
                bool checkReturnType = op && op->returnIsOptional();
                bool insertComma = false;
                for (ParamDeclList::const_iterator p = optionals.begin(); p != optionals.end(); ++p)
                {
                    if (checkReturnType && op->returnTag() < (*p)->tag())
                    {
                        os << (insertComma ? ", " : "") << op->returnTag();
                        checkReturnType = false;
                        insertComma = true;
                    }
                    os << (insertComma ? ", " : "") << (*p)->tag();
                    insertComma = true;
                }
                if (checkReturnType)
                {
                    os << (insertComma ? ", " : "") << op->returnTag();
                }
                os << '}';
                out << os.str();
            }

            {
                //
                // Parameters
                //
                bool checkReturnType = op && op->returnIsOptional();
                for (ParamDeclList::const_iterator p = optionals.begin(); p != optionals.end(); ++p)
                {
                    if (checkReturnType && op->returnTag() < (*p)->tag())
                    {
                        if (tuple)
                        {
                            out << "::std::get<0>(v)";
                        }
                        else
                        {
                            out << returnValueS;
                        }
                        checkReturnType = false;
                    }

                    if (tuple)
                    {
                        auto index =
                            std::distance(params.begin(), std::find(params.begin(), params.end(), *p)) + retOffset;
                        out << "::std::get<" + std::to_string(index) + ">(v)";
                    }
                    else
                    {
                        out << fixKwd(paramPrefix + (*p)->name());
                    }
                }
                if (checkReturnType)
                {
                    if (tuple)
                    {
                        out << "::std::get<0>(v)";
                    }
                    else
                    {
                        out << returnValueS;
                    }
                }
            }
            out << epar << ";";
        }
    }
}

string Slice::paramPrefix = "iceP_";

char
Slice::ToIfdef::operator()(char c)
{
    if (!isalnum(static_cast<unsigned char>(c)))
    {
        return '_';
    }
    else
    {
        return c;
    }
}

void
Slice::printHeader(Output& out)
{
    static const char* header = "//\n"
                                "// Copyright (c) ZeroC, Inc. All rights reserved.\n"
                                "//\n";

    out << header;
    out << "//\n";
    out << "// Ice version " << ICE_STRING_VERSION << "\n";
    out << "//\n";
}

void
Slice::printVersionCheck(Output& out)
{
    out << "\n";
    out << "\n#ifndef ICE_IGNORE_VERSION";
    int iceVersion = ICE_INT_VERSION; // Use this to prevent warning with C++Builder
    if (iceVersion % 100 >= 50)
    {
        //
        // Beta version: exact match required
        //
        out << "\n#   if ICE_INT_VERSION  != " << ICE_INT_VERSION;
        out << "\n#       error Ice version mismatch: an exact match is required for beta generated code";
        out << "\n#   endif";
    }
    else
    {
        out << "\n#   if ICE_INT_VERSION / 100 != " << ICE_INT_VERSION / 100;
        out << "\n#       error Ice version mismatch!";
        out << "\n#   endif";

        //
        // Generated code is release; reject beta header
        //
        out << "\n#   if ICE_INT_VERSION % 100 >= 50";
        out << "\n#       error Beta header file detected";
        out << "\n#   endif";

        out << "\n#   if ICE_INT_VERSION % 100 < " << ICE_INT_VERSION % 100;
        out << "\n#       error Ice patch level mismatch!";
        out << "\n#   endif";
    }
    out << "\n#endif";
}

void
Slice::printDllExportStuff(Output& out, const string& dllExport)
{
    if (dllExport.size() && dllExport != "ICE_API")
    {
        out << sp;
        out << "\n#ifndef " << dllExport;
        out << "\n#   if defined(" << dllExport << "_EXPORTS)";
        out << "\n#       define " << dllExport << " ICE_DECLSPEC_EXPORT";
        out << "\n#   else";
        out << "\n#       define " << dllExport << " ICE_DECLSPEC_IMPORT";
        out << "\n#   endif";
        out << "\n#endif";
    }
}

bool
Slice::isMovable(const TypePtr& type)
{
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindString:
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindValue:
            {
                return true;
            }
            default:
            {
                return false;
            }
        }
    }
    return !dynamic_pointer_cast<Enum>(type);
}

string
Slice::getUnqualified(const std::string& type, const std::string& scope)
{
    if (type.find("::") != string::npos)
    {
        string prefix;
        if (type.find("const ") == 0)
        {
            prefix += "const ";
        }

        if (type.find(scope, prefix.size()) == prefix.size())
        {
            string t = type.substr(prefix.size() + scope.size());
            if (t.find("::") == string::npos)
            {
                return prefix + t;
            }
        }
    }
    return type;
}

string
Slice::typeToString(
    const TypePtr& type,
    bool optional,
    const string& scope,
    const StringList& metaData,
    TypeContext typeCtx)
{
    assert(type);

    if (optional)
    {
        if (isProxyType(type))
        {
            // We map optional proxies like regular proxies, as optional<XxxPrx>.
            return typeToString(type, false, scope, metaData, typeCtx);
        }
        else
        {
            return "::std::optional<" + typeToString(type, false, scope, metaData, typeCtx) + '>';
        }
    }

    static constexpr string_view builtinTable[] = {
        "::std::uint8_t",
        "bool",
        "::std::int16_t",
        "::std::int32_t",
        "::std::int64_t",
        "float",
        "double",
        "****", // string or wstring, see below
        "::Ice::ValuePtr",
        "::std::optional<::Ice::ObjectPrx>",
        "::Ice::ValuePtr"};

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        if (builtin->kind() == Builtin::KindString)
        {
            return stringTypeToString(type, metaData, typeCtx);
        }
        else
        {
            if (builtin->kind() == Builtin::KindObject)
            {
                return getUnqualified(string{builtinTable[Builtin::KindValue]}, scope);
            }
            else
            {
                return getUnqualified(string{builtinTable[builtin->kind()]}, scope);
            }
        }
    }

    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
    if (cl)
    {
        return getUnqualified(cl->scoped(), scope) + "Ptr";
    }

    StructPtr st = dynamic_pointer_cast<Struct>(type);
    if (st)
    {
        return getUnqualified(fixKwd(st->scoped()), scope);
    }

    InterfaceDeclPtr proxy = dynamic_pointer_cast<InterfaceDecl>(type);
    if (proxy)
    {
        return "::std::optional<" + getUnqualified(fixKwd(proxy->scoped() + "Prx"), scope) + ">";
    }

    EnumPtr en = dynamic_pointer_cast<Enum>(type);
    if (en)
    {
        return getUnqualified(fixKwd(en->scoped()), scope);
    }

    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    if (seq)
    {
        return sequenceTypeToString(seq, scope, metaData, typeCtx);
    }

    DictionaryPtr dict = dynamic_pointer_cast<Dictionary>(type);
    if (dict)
    {
        return dictionaryTypeToString(dict, scope, metaData, typeCtx);
    }

    return "???";
}

string
Slice::inputTypeToString(
    const TypePtr& type,
    bool optional,
    const string& scope,
    const StringList& metaData,
    TypeContext typeCtx)
{
    assert(type);
    assert(typeCtx == TypeContext::None || typeCtx == TypeContext::UseWstring);
    typeCtx = (typeCtx | TypeContext::MarshalParam);

    if (inputParamByValue(type, metaData))
    {
        // Pass by value, even if it's optional.
        return typeToString(type, optional, scope, metaData, typeCtx);
    }
    else
    {
        // For all other types, pass by const reference.
        return "const " + typeToString(type, optional, scope, metaData, typeCtx) + '&';
    }
}

string
Slice::outputTypeToString(
    const TypePtr& type,
    bool optional,
    const string& scope,
    const StringList& metaData,
    TypeContext typeCtx)
{
    assert(type);
    assert(typeCtx == TypeContext::None || typeCtx == TypeContext::UseWstring);

    return typeToString(type, optional, scope, metaData, typeCtx) + '&';
}

string
Slice::operationModeToString(Operation::Mode mode)
{
    switch (mode)
    {
        case Operation::Normal:
        {
            return "::Ice::OperationMode::Normal";
        }

        case Operation::Idempotent:
        {
            return "::Ice::OperationMode::Idempotent";
        }
        default:
        {
            assert(false);
        }
    }

    return "???";
}

string
Slice::opFormatTypeToString(const OperationPtr& op)
{
    optional<FormatType> opFormat = op->format();
    if (opFormat)
    {
        switch (*opFormat)
        {
            case CompactFormat:
                return "::Ice::FormatType::CompactFormat";
            case SlicedFormat:
                return "::Ice::FormatType::SlicedFormat";

            default:
                assert(false);
                return "???";
        }
    }
    else
    {
        return "::std::nullopt";
    }
}

//
// If the passed name is a keyword, return the name with a "_cpp_" prefix;
// otherwise, return the name unchanged.
//

static string
lookupKwd(const string& name)
{
    //
    // Keyword list. *Must* be kept in alphabetical order.
    //
    // Note that this keyword list unnecessarily contains C++ keywords
    // that are illegal Slice identifiers -- namely identifiers that
    // are Slice keywords (class, int, etc.). They have not been removed
    // so that the keyword list is kept complete.
    //
    static const string keywordList[] = {
        "alignas",     "alignof",  "and",      "and_eq",    "asm",        "auto",         "bitand",
        "bitor",       "bool",     "break",    "case",      "catch",      "char",         "char16_t",
        "char32_t",    "class",    "compl",    "const",     "const_cast", "constexpr",    "continue",
        "decltype",    "default",  "delete",   "do",        "double",     "dynamic_cast", "else",
        "enum",        "explicit", "export",   "extern",    "false",      "float",        "for",
        "friend",      "goto",     "if",       "inline",    "int",        "long",         "mutable",
        "namespace",   "new",      "noexcept", "not",       "not_eq",     "nullptr",      "operator",
        "or",          "or_eq",    "private",  "protected", "public",     "register",     "reinterpret_cast",
        "requires",    "return",   "short",    "signed",    "sizeof",     "static",       "static_assert",
        "static_cast", "struct",   "switch",   "template",  "this",       "thread_local", "throw",
        "true",        "try",      "typedef",  "typeid",    "typename",   "union",        "unsigned",
        "using",       "virtual",  "void",     "volatile",  "wchar_t",    "while",        "xor",
        "xor_eq"};
    bool found = binary_search(&keywordList[0], &keywordList[sizeof(keywordList) / sizeof(*keywordList)], name);
    return found ? "_cpp_" + name : name;
}

//
// If the passed name is a scoped name, return the identical scoped name,
// but with all components that are C++ keywords replaced by
// their "_cpp_"-prefixed version; otherwise, if the passed name is
// not scoped, but a C++ keyword, return the "_cpp_"-prefixed name;
// otherwise, return the name unchanged.
//
string
Slice::fixKwd(const string& name)
{
    if (name[0] != ':')
    {
        return lookupKwd(name);
    }
    vector<string> ids = splitScopedName(name);
    transform(ids.begin(), ids.end(), ids.begin(), [](const string& id) -> string { return lookupKwd(id); });
    stringstream result;
    for (vector<string>::const_iterator i = ids.begin(); i != ids.end(); ++i)
    {
        result << "::" + *i;
    }
    return result.str();
}

void
Slice::writeMarshalCode(Output& out, const ParamDeclList& params, const OperationPtr& op)
{
    writeMarshalUnmarshalParams(out, params, op, true);
}

void
Slice::writeUnmarshalCode(Output& out, const ParamDeclList& params, const OperationPtr& op)
{
    writeMarshalUnmarshalParams(out, params, op, false);
}

void
Slice::writeAllocateCode(
    Output& out,
    const ParamDeclList& params,
    const OperationPtr& op,
    const string& clScope,
    TypeContext typeCtx)
{
    for (ParamDeclList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        writeParamAllocateCode(
            out,
            (*p)->type(),
            (*p)->optional(),
            clScope,
            fixKwd(paramPrefix + (*p)->name()),
            (*p)->getMetaData(),
            typeCtx);
    }

    if (op && op->returnType())
    {
        writeParamAllocateCode(
            out,
            op->returnType(),
            op->returnIsOptional(),
            clScope,
            "ret",
            op->getMetaData(),
            typeCtx);
    }
}

void
Slice::writeMarshalUnmarshalAllInHolder(
    IceInternal::Output& out,
    const string& holder,
    const DataMemberList& dataMembers,
    bool optional,
    bool marshal)
{
    if (dataMembers.empty())
    {
        return;
    }

    string stream = marshal ? "ostr" : "istr";
    string streamOp = marshal ? "writeAll" : "readAll";

    out << nl << stream << "->" << streamOp;
    out << spar;

    if (optional)
    {
        ostringstream os;
        os << "{";
        for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            if (q != dataMembers.begin())
            {
                os << ", ";
            }
            os << (*q)->tag();
        }
        os << "}";
        out << os.str();
    }

    for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        out << holder + fixKwd((*q)->name());
    }

    out << epar << ";";
}

void
Slice::writeStreamHelpers(Output& out, const ContainedPtr& c, DataMemberList dataMembers, bool hasBaseDataMembers)
{
    // If c is a class/exception whose base class contains data members (recursively), then we need to generate
    // a StreamWriter even if its implementation is empty. This is because our default marshaling uses ice_tuple() which
    // contains all of our class/exception's data members as well the base data members, which breaks marshaling. This
    // is not an issue for structs.
    if (dataMembers.empty() && !hasBaseDataMembers)
    {
        return;
    }

    DataMemberList requiredMembers;
    DataMemberList optionalMembers;

    for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        if ((*q)->optional())
        {
            optionalMembers.push_back(*q);
        }
        else
        {
            requiredMembers.push_back(*q);
        }
    }

    // Sort optional data members
    class SortFn
    {
    public:
        static bool compare(const DataMemberPtr& lhs, const DataMemberPtr& rhs) { return lhs->tag() < rhs->tag(); }
    };
    optionalMembers.sort(SortFn::compare);

    string scoped = c->scoped();
    string fullName = fixKwd(scoped);
    string holder = "v.";

    //
    // Generate StreamWriter
    //
    // Only generate StreamWriter specializations if we are generating optional data members and no
    // base class data members
    //
    if (!optionalMembers.empty() || hasBaseDataMembers)
    {
        out << nl << "template<>";
        out << nl << "struct StreamWriter<" << fullName << ">";
        out << sb;
        if (requiredMembers.empty() && optionalMembers.empty())
        {
            out << nl << "static void write(OutputStream*, const " << fullName << "&)";
        }
        else
        {
            out << nl << "static void write(OutputStream* ostr, const " << fullName << "& v)";
        }

        out << sb;

        writeMarshalUnmarshalAllInHolder(out, holder, requiredMembers, false, true);
        writeMarshalUnmarshalAllInHolder(out, holder, optionalMembers, true, true);

        out << eb;
        out << eb << ";" << nl;
    }

    //
    // Generate StreamReader
    //
    out << nl << "template<>";
    out << nl << "struct StreamReader<" << fullName << ">";
    out << sb;
    if (requiredMembers.empty() && optionalMembers.empty())
    {
        out << nl << "static void read(InputStream*, " << fullName << "&)";
    }
    else
    {
        out << nl << "static void read(InputStream* istr, " << fullName << "& v)";
    }

    out << sb;

    writeMarshalUnmarshalAllInHolder(out, holder, requiredMembers, false, false);
    writeMarshalUnmarshalAllInHolder(out, holder, optionalMembers, true, false);

    out << eb;
    out << eb << ";" << nl;
}

void
Slice::writeIceTuple(::IceInternal::Output& out, DataMemberList dataMembers, TypeContext typeCtx)
{
    //
    // Use an empty scope to get full qualified names from calls to typeToString.
    //
    const string scope = "";
    out << nl << "std::tuple<";
    for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        if (q != dataMembers.begin())
        {
            out << ", ";
        }
        out << "const ";
        out << typeToString((*q)->type(), (*q)->optional(), scope, (*q)->getMetaData(), typeCtx) << "&";
    }
    out << "> ice_tuple() const";

    out << sb;
    out << nl << "return std::tie(";
    for (DataMemberList::const_iterator pi = dataMembers.begin(); pi != dataMembers.end(); ++pi)
    {
        if (pi != dataMembers.begin())
        {
            out << ", ";
        }
        out << fixKwd((*pi)->name());
    }
    out << ");" << eb;
}

bool
Slice::findMetaData(const string& prefix, const ClassDeclPtr& cl, string& value)
{
    if (findMetaData(prefix, cl->getMetaData(), value))
    {
        return true;
    }

    ClassDefPtr def = cl->definition();
    return def ? findMetaData(prefix, def->getMetaData(), value) : false;
}

bool
Slice::findMetaData(const string& prefix, const StringList& metaData, string& value)
{
    for (const auto& s : metaData)
    {
        if (s.find(prefix) == 0)
        {
            value = s.substr(prefix.size());
            return true;
        }
    }
    return false;
}

string
Slice::findMetaData(const StringList& metaData, TypeContext typeCtx)
{
    static const string prefix = "cpp:";

    for (StringList::const_iterator q = metaData.begin(); q != metaData.end(); ++q)
    {
        string str = *q;
        if (str.find(prefix) == 0)
        {
            string::size_type pos = str.find(':', prefix.size());

            // If a marshal param, we first check view-type then type. Otherwise, we check type.
            // Then, if a marshal param or an unmarshal param where the underlying InputStream buffer remains valid for
            // a while, we check for "array".

            if (pos != string::npos)
            {
                string ss = str.substr(prefix.size());

                if ((typeCtx & TypeContext::MarshalParam) != TypeContext::None)
                {
                    if (ss.find("view-type:") == 0)
                    {
                        return str.substr(pos + 1);
                    }
                }

                if (ss.find("type:") == 0)
                {
                    return str.substr(pos + 1);
                }
            }
            else if ((typeCtx & (TypeContext::MarshalParam | TypeContext::UnmarshalParamZeroCopy)) != TypeContext::None)
            {
                string ss = str.substr(prefix.size());
                if (ss == "array")
                {
                    return "%array";
                }
            }
            //
            // Otherwise if the data is "unscoped" it is returned.
            //
            else
            {
                string ss = str.substr(prefix.size());
                if (ss == "unscoped")
                {
                    return "%unscoped";
                }
            }
        }
    }

    return "";
}

bool
Slice::inWstringModule(const SequencePtr& seq)
{
    ContainerPtr cont = seq->container();
    while (cont)
    {
        ModulePtr mod = dynamic_pointer_cast<Module>(cont);
        if (!mod)
        {
            break;
        }
        StringList metaData = mod->getMetaData();
        if (find(metaData.begin(), metaData.end(), "cpp:type:wstring") != metaData.end())
        {
            return true;
        }
        else if (find(metaData.begin(), metaData.end(), "cpp:type:string") != metaData.end())
        {
            return false;
        }
        cont = mod->container();
    }
    return false;
}
