// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/CPlusPlusUtil.h>
#include <Slice/Util.h>
#include <cstring>
#include <functional>

#ifndef _WIN32
#  include <fcntl.h>
#endif

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

namespace
{

string toTemplateArg(const string& arg)
{
    if(arg.empty())
    {
        return arg;
    }
    string fixed = arg;
    if(arg[0] == ':')
    {
        fixed = " " + fixed;
    }
    if(fixed[fixed.length() - 1] == '>')
    {
        fixed = fixed + " ";
    }
    return fixed;
}

string
stringTypeToString(const TypePtr& type, const StringList& metaData, int typeCtx)
{
    string strType = findMetaData(metaData, typeCtx);
    if(strType == "wstring" || (typeCtx & TypeContextUseWstring && strType == ""))
    {
        if(featureProfile == IceE)
        {
            return "::Ice::Wstring";
        }
        else
        {
            return "::std::wstring";
        }
    }
    else if(strType != "" && strType != "string")
    {
        return strType;
    }
    else
    {
        return "::std::string";
    }
}

string
sequenceTypeToString(const SequencePtr& seq, const StringList& metaData, int typeCtx)
{
    string seqType = findMetaData(metaData, typeCtx);
    if(!seqType.empty())
    {
        if(seqType == "%array")
        {
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
            if(typeCtx & TypeContextAMIPrivateEnd)
            {
                if(builtin && builtin->kind() == Builtin::KindByte)
                {
                    string s = typeToString(seq->type());
                    return "::std::pair<const " + s + "*, const " + s + "*>";
                }
                else if(builtin &&
                        builtin->kind() != Builtin::KindString &&
                        builtin->kind() != Builtin::KindObject &&
                        builtin->kind() != Builtin::KindObjectProxy)
                {
                    string s = toTemplateArg(typeToString(builtin));
                    return "::std::pair< ::IceUtil::ScopedArray<" + s + ">, " +
                        "::std::pair<const " + s + "*, const " + s + "*> >";
                }
                else
                {
                    string s = toTemplateArg(typeToString(seq->type(), seq->typeMetaData(),
                                                          inWstringModule(seq) ? TypeContextUseWstring : 0));
                    return "::std::vector<" + s + '>';
                }
            }
            string s = typeToString(seq->type(), seq->typeMetaData(), inWstringModule(seq) ? TypeContextUseWstring : 0);
            return "::std::pair<const " + s + "*, const " + s + "*>";
        }
        else if(seqType.find("%range") == 0)
        {
            string s;
            if(seqType.find("%range:") == 0)
            {
                s = seqType.substr(strlen("%range:"));
            }
            else
            {
                s = fixKwd(seq->scoped());
            }
            if(typeCtx & TypeContextAMIPrivateEnd)
            {
                return s;
            }
            if(s[0] == ':')
            {
                s = " " + s;
            }
            return "::std::pair<" + s + "::const_iterator, " + s + "::const_iterator>";
        }
        else
        {
            return seqType;
        }
    }
    else
    {
        return fixKwd(seq->scoped());
    }
}

string
dictionaryTypeToString(const DictionaryPtr& dict, const StringList& metaData, int typeCtx)
{
    string dictType = findMetaData(metaData, typeCtx);
    if(!dictType.empty())
    {
        return dictType;
    }
    else
    {
        return fixKwd(dict->scoped());
    }
}


void
writeParamAllocateCode(Output& out, const TypePtr& type, bool optional, const string& fixedName,
                       const StringList& metaData, int typeCtx, bool endArg)
{
    string s = typeToString(type, metaData, typeCtx);
    if(optional)
    {
        s = "IceUtil::Optional<" + toTemplateArg(s) + '>';
    }
    out << nl << s << ' ' << fixedName << ';';

    if(!(typeCtx & TypeContextInParam) || !endArg)
    {
        return; // We're done.
    }

    //
    // If using a range or array we need to allocate the range container, or
    // array as well now to ensure they are always in the same scope.
    //
    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        string seqType = findMetaData(metaData, typeCtx);
        if(seqType.empty())
        {
            seqType = findMetaData(seq->getMetaData(), typeCtx);
        }

        string s;
        if(seqType == "%array")
        {
            s = typeToString(seq, metaData, TypeContextAMIPrivateEnd);
        }
        else if(seqType.find("%range") == 0)
        {
            StringList md;
            if(seqType.find("%range:") == 0)
            {
                md.push_back("cpp:type:" + seqType.substr(strlen("%range:")));
            }
            s = typeToString(seq, md);
        }

        if(!s.empty())
        {
            if(optional)
            {
                s = "IceUtil::Optional<" + toTemplateArg(s) + '>';
            }
            out << nl << s << " ___" << fixedName << ";";
        }
    }
}

void
writeParamEndCode(Output& out, const TypePtr& type, bool optional, const string& fixedName, const StringList& metaData)
{
    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        string seqType = findMetaData(metaData, TypeContextInParam);
        if(seqType.empty())
        {
            seqType = findMetaData(seq->getMetaData(), TypeContextInParam);
        }

        if(seqType == "%array")
        {
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
            if(builtin &&
               builtin->kind() != Builtin::KindByte &&
               builtin->kind() != Builtin::KindString &&
               builtin->kind() != Builtin::KindObject &&
               builtin->kind() != Builtin::KindObjectProxy)
            {
                if(optional)
                {
                    out << nl << "if(___" << fixedName << ")";
                    out << sb;
                    out << nl << fixedName << " = ___" << fixedName << "->second;";
                        out << eb;
                }
                else
                {
                    out << nl << fixedName << " = ___" << fixedName << ".second;";
                }
            }
            else if(!builtin ||
                    builtin->kind() == Builtin::KindString ||
                    builtin->kind() == Builtin::KindObject ||
                    builtin->kind() == Builtin::KindObjectProxy)
            {
                if(optional)
                {
                    out << nl << "if(___" << fixedName << ")";
                    out << sb;
                    out << nl << fixedName << ".__setIsSet();";
                    out << nl << "if(!___" << fixedName << "->empty())";
                    out << sb;
                    out << nl << fixedName << "->first" << " = &(*___" << fixedName << ")[0];";
                    out << nl << fixedName << "->second" << " = " << fixedName << "->first + " << "___"
                        << fixedName << "->size();";
                    out << eb;
                    out << nl << "else";
                    out << sb;
                    out << nl << fixedName << "->first" << " = " << fixedName << "->second" << " = 0;";
                    out << eb;
                    out << eb;
                }
                else
                {
                    out << nl << "if(!___" << fixedName << ".empty())";
                    out << sb;
                    out << nl << fixedName << ".first" << " = &___" << fixedName << "[0];";
                    out << nl << fixedName << ".second" << " = " << fixedName << ".first + " << "___"
                        << fixedName << ".size();";
                    out << eb;
                    out << nl << "else";
                    out << sb;
                    out << nl << fixedName << ".first" << " = " << fixedName << ".second" << " = 0;";
                    out << eb;
                }
            }
        }
        else if(seqType.find("%range") == 0)
        {
            if(optional)
            {
                out << nl << "if(___" << fixedName << ")";
                out << sb;
                out << nl << fixedName << ".__setIsSet();";
                out << nl << fixedName << "->first = (*___" << fixedName << ").begin();";
                out << nl << fixedName << "->second = (*___" << fixedName << ").end();";
                out << eb;
            }
            else
            {
                out << nl << fixedName << ".first = ___" << fixedName << ".begin();";
                out << nl << fixedName << ".second = ___" << fixedName << ".end();";
            }
        }
    }
}

void
writeMarshalUnmarshalParams(Output& out, const ParamDeclList& params, const OperationPtr& op, bool marshal, bool prepend, int typeCtx)
{
    string prefix = prepend ? paramPrefix : "";

    //
    // Marshal non optional parameters.
    //
    ParamDeclList optionals;
    for(ParamDeclList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        if((*p)->optional())
        {
            optionals.push_back(*p);
        }
        else
        {
            writeMarshalUnmarshalCode(out, (*p)->type(), false, 0, fixKwd(prefix + (*p)->name()), marshal, (*p)->getMetaData(),
                                      typeCtx);
        }
    }
    if(op && op->returnType())
    {
        if(!op->returnIsOptional())
        {
            writeMarshalUnmarshalCode(out, op->returnType(), false, 0, "__ret", marshal, op->getMetaData(), typeCtx);
        }
    }

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

    //
    // Marshal optional parameters.
    //
    bool checkReturnType = op && op->returnIsOptional();
    for(ParamDeclList::const_iterator p = optionals.begin(); p != optionals.end(); ++p)
    {
        if(checkReturnType && op->returnTag() < (*p)->tag())
        {
            writeMarshalUnmarshalCode(out, op->returnType(), true, op->returnTag(), "__ret", marshal,
                                      op->getMetaData(), typeCtx);
            checkReturnType = false;
        }
        writeMarshalUnmarshalCode(out, (*p)->type(), true, (*p)->tag(), fixKwd(prefix + (*p)->name()), marshal,
                                  (*p)->getMetaData(), typeCtx);
    }
    if(checkReturnType)
    {
        writeMarshalUnmarshalCode(out, op->returnType(), true, op->returnTag(), "__ret", marshal, op->getMetaData(),
                                  typeCtx);
    }
}

}

Slice::FeatureProfile Slice::featureProfile = Slice::Ice;
string Slice::paramPrefix = "__p_";

char
Slice::ToIfdef::operator()(char c)
{
    if(!isalnum(static_cast<unsigned char>(c)))
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
    static const char* header =
"// **********************************************************************\n"
"//\n"
"// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.\n"
"//\n"
"// This copy of Ice is licensed to you under the terms described in the\n"
"// ICE_LICENSE file included in this distribution.\n"
"//\n"
"// **********************************************************************\n"
        ;

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
    if(iceVersion % 100 > 50)
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
        out << "\n#   if ICE_INT_VERSION % 100 > 50";
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
    if(dllExport.size())
    {
        out << sp;
        out << "\n#ifndef " << dllExport;
        out << "\n#   ifdef " << dllExport << "_EXPORTS";
        out << "\n#       define " << dllExport << " ICE_DECLSPEC_EXPORT";
        out << "\n#   elif defined(ICE_STATIC_LIBS)";
        out << "\n#       define " << dllExport << " /**/";
        out << "\n#   else";
        out << "\n#       define " << dllExport << " ICE_DECLSPEC_IMPORT";
        out << "\n#   endif";
        out << "\n#endif";
    }
}


string
Slice::typeToString(const TypePtr& type, const StringList& metaData, int typeCtx)
{
    static const char* builtinTable[] =
    {
        "::Ice::Byte",
        "bool",
        "::Ice::Short",
        "::Ice::Int",
        "::Ice::Long",
        "::Ice::Float",
        "::Ice::Double",
        "::std::string",
        "::Ice::ObjectPtr",
        "::Ice::ObjectPrx",
        "::Ice::LocalObjectPtr"
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        if(builtin->kind() == Builtin::KindString)
        {
            return stringTypeToString(type, metaData, typeCtx);
        }
        else
        {
            return builtinTable[builtin->kind()];
        }
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        return fixKwd(cl->scoped() + "Ptr");
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(findMetaData(st->getMetaData()) == "%class")
        {
            return fixKwd(st->scoped() + "Ptr");
        }
        return fixKwd(st->scoped());
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        return fixKwd(proxy->_class()->scoped() + "Prx");
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        return sequenceTypeToString(seq, metaData, typeCtx);
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if(dict)
    {
        return dictionaryTypeToString(dict, metaData, typeCtx);
    }

    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
    {
        return fixKwd(contained->scoped());
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        return fixKwd(en->scoped());
    }

    return "???";
}

string
Slice::typeToString(const TypePtr& type, bool optional, const StringList& metaData, int typeCtx)
{
    if(optional)
    {
        return "IceUtil::Optional<" + toTemplateArg(typeToString(type, metaData, typeCtx)) + ">";
    }
    else
    {
        return typeToString(type, metaData, typeCtx);
    }
}

string
Slice::returnTypeToString(const TypePtr& type, bool optional, const StringList& metaData, int typeCtx)
{
    if(!type)
    {
        return "void";
    }

    if(optional)
    {
        return "IceUtil::Optional<" + toTemplateArg(typeToString(type, metaData, typeCtx)) + ">";
    }

    return typeToString(type, metaData, typeCtx);
}

string
Slice::inputTypeToString(const TypePtr& type, bool optional, const StringList& metaData, int typeCtx)
{
    static const char* inputBuiltinTable[] =
    {
        "::Ice::Byte",
        "bool",
        "::Ice::Short",
        "::Ice::Int",
        "::Ice::Long",
        "::Ice::Float",
        "::Ice::Double",
        "const ::std::string&",
        "const ::Ice::ObjectPtr&",
        "const ::Ice::ObjectPrx&",
        "const ::Ice::LocalObjectPtr&"
    };

    typeCtx |= TypeContextInParam;

    if(optional)
    {
        return "const IceUtil::Optional<" + toTemplateArg(typeToString(type, metaData, typeCtx)) +">&";
    }

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        if(builtin->kind() == Builtin::KindString)
        {
            return string("const ") + stringTypeToString(type, metaData, typeCtx) + "&";
        }
        else
        {
            return inputBuiltinTable[builtin->kind()];
        }
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        return "const " + fixKwd(cl->scoped() + "Ptr&");
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(findMetaData(st->getMetaData()) == "%class")
        {
            return "const " + fixKwd(st->scoped() + "Ptr&");
        }
        return "const " + fixKwd(st->scoped()) + "&";
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        return "const " + fixKwd(proxy->_class()->scoped() + "Prx&");
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        return fixKwd(en->scoped());
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        return "const " + sequenceTypeToString(seq, metaData, typeCtx) + "&";
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if(dict)
    {
        return "const " + dictionaryTypeToString(dict, metaData, typeCtx) + "&";
    }

    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
    {
        return "const " + fixKwd(contained->scoped()) + "&";
    }

    return "???";
}

string
Slice::outputTypeToString(const TypePtr& type, bool optional, const StringList& metaData, int typeCtx)
{
    static const char* outputBuiltinTable[] =
    {
        "::Ice::Byte&",
        "bool&",
        "::Ice::Short&",
        "::Ice::Int&",
        "::Ice::Long&",
        "::Ice::Float&",
        "::Ice::Double&",
        "::std::string&",
        "::Ice::ObjectPtr&",
        "::Ice::ObjectPrx&",
        "::Ice::LocalObjectPtr&"
    };

    if(optional)
    {
        return "IceUtil::Optional<" + toTemplateArg(typeToString(type, metaData, typeCtx)) +">&";
    }

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        if(builtin->kind() == Builtin::KindString)
        {
            return stringTypeToString(type, metaData, typeCtx) + "&";
        }
        else
        {
            return outputBuiltinTable[builtin->kind()];
        }
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        return fixKwd(cl->scoped() + "Ptr&");
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(findMetaData(st->getMetaData()) == "%class")
        {
            return fixKwd(st->scoped() + "Ptr&");
        }
        return fixKwd(st->scoped()) + "&";
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        return fixKwd(proxy->_class()->scoped() + "Prx&");
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        return sequenceTypeToString(seq, metaData, typeCtx) + "&";
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if(dict)
    {
        return dictionaryTypeToString(dict, metaData, typeCtx) + "&";
    }

    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
    {
        return fixKwd(contained->scoped()) + "&";
    }

    return "???";
}

string
Slice::operationModeToString(Operation::Mode mode)
{
    switch(mode)
    {
        case Operation::Normal:
        {
            return "::Ice::Normal";
        }

        case Operation::Nonmutating:
        {
            return "::Ice::Nonmutating";
        }

        case Operation::Idempotent:
        {
            return "::Ice::Idempotent";
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
    switch(op->format())
    {
    case DefaultFormat:
        return "::Ice::DefaultFormat";
    case CompactFormat:
        return "::Ice::CompactFormat";
    case SlicedFormat:
        return "::Ice::SlicedFormat";

    default:
        assert(false);
    }

    return "???";
}

//
// If the passed name is a keyword, return the name with a "_cxx_" prefix;
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
    static const string keywordList[] =
    {
        "and", "and_eq", "asm", "auto", "bit_and", "bit_or", "bool", "break", "case", "catch", "char",
        "class", "compl", "const", "const_cast", "continue", "default", "delete", "do", "double",
        "dynamic_cast", "else", "enum", "explicit", "export", "extern", "false", "float", "for",
        "friend", "goto", "if", "inline", "int", "long", "mutable", "namespace", "new", "not", "not_eq",
        "operator", "or", "or_eq", "private", "protected", "public", "register", "reinterpret_cast",
        "return", "short", "signed", "sizeof", "static", "static_cast", "struct", "switch", "template",
        "this", "throw", "true", "try", "typedef", "typeid", "typename", "union", "unsigned", "using",
        "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq"
    };
    bool found =  binary_search(&keywordList[0],
                                &keywordList[sizeof(keywordList) / sizeof(*keywordList)],
                                name);
    return found ? "_cpp_" + name : name;
}

//
// Split a scoped name into its components and return the components as a list of (unscoped) identifiers.
//
static StringList
splitScopedName(const string& scoped)
{
    assert(scoped[0] == ':');
    StringList ids;
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

//
// If the passed name is a scoped name, return the identical scoped name,
// but with all components that are C++ keywords replaced by
// their "_cxx_"-prefixed version; otherwise, if the passed name is
// not scoped, but a C++ keyword, return the "_cxx_"-prefixed name;
// otherwise, return the name unchanged.
//
string
Slice::fixKwd(const string& name)
{
    if(name[0] != ':')
    {
        return lookupKwd(name);
    }
    StringList ids = splitScopedName(name);
    transform(ids.begin(), ids.end(), ids.begin(), ptr_fun(lookupKwd));
    stringstream result;
    for(StringList::const_iterator i = ids.begin(); i != ids.end(); ++i)
    {
        result << "::" + *i;
    }
    return result.str();
}

void
Slice::writeMarshalUnmarshalCode(Output& out, const TypePtr& type, bool optional, int tag, const string& param,
                                 bool marshal, const StringList& metaData, int typeCtx, const string& str, bool pointer)
{
    ostringstream os;
    if(str.empty())
    {
        os << (marshal ? "__os" : "__is");
    }
    else
    {
        os << str;
    }

    string deref;
    if(pointer)
    {
        os << "->";
    }
    else
    {
        os << '.';
    }

    if(marshal)
    {
        os << "write(";
    }
    else
    {
        os << "read(";
    }

    if(optional)
    {
        os << tag << ", ";
    }

    string func = os.str();
    if(!marshal)
    {
        SequencePtr seq = SequencePtr::dynamicCast(type);
        if(seq && !(typeCtx & TypeContextAMIPrivateEnd))
        {
            string seqType = findMetaData(metaData, typeCtx);
            if(seqType == "%array")
            {
                BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
                if(builtin && builtin->kind() == Builtin::KindByte)
                {
                    out << nl << func << param << ");";
                    return;
                }

                out << nl << func << "___" << param << ");";
                writeParamEndCode(out, seq, optional, param, metaData);
                return;
            }
            else if(seqType.find("%range") == 0)
            {
                out << nl << func << "___" << param << ");";
                writeParamEndCode(out, seq, optional, param, metaData);
                return;
            }
        }
    }

    out << nl << func << param << ");";
}

void
Slice::writeMarshalCode(Output& out, const ParamDeclList& params, const OperationPtr& op, bool prepend, int typeCtx)
{
    writeMarshalUnmarshalParams(out, params, op, true, prepend, typeCtx);
}

void
Slice::writeUnmarshalCode(Output& out, const ParamDeclList& params, const OperationPtr& op, bool prepend, int typeCtx)
{
    writeMarshalUnmarshalParams(out, params, op, false, prepend, typeCtx);
}

void
Slice::writeAllocateCode(Output& out, const ParamDeclList& params, const OperationPtr& op, bool prepend, int typeCtx)
{
    string prefix = prepend ? paramPrefix : "";
    for(ParamDeclList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        writeParamAllocateCode(out, (*p)->type(), (*p)->optional(), fixKwd(prefix + (*p)->name()), (*p)->getMetaData(),
                               typeCtx, getEndArg((*p)->type(),(*p)->getMetaData(), (*p)->name()) != (*p)->name());
    }
    if(op && op->returnType())
    {
        writeParamAllocateCode(out, op->returnType(), op->returnIsOptional(), "__ret", op->getMetaData(), typeCtx,
                               getEndArg(op->returnType(), op->getMetaData(), "__ret") != "__ret");
    }
}

string
Slice::getEndArg(const TypePtr& type, const StringList& metaData, const string& arg)
{
    string endArg = arg;
    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        string seqType = findMetaData(metaData, TypeContextInParam);
        if(seqType.empty())
        {
            seqType = findMetaData(seq->getMetaData(), TypeContextInParam);
        }

        if(seqType == "%array")
        {
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
            if(builtin &&
               builtin->kind() != Builtin::KindByte &&
               builtin->kind() != Builtin::KindString &&
               builtin->kind() != Builtin::KindObject &&
               builtin->kind() != Builtin::KindObjectProxy)
            {
                endArg = "___" + endArg;
            }
            else if(!builtin || builtin->kind() != Builtin::KindByte)
            {
                endArg = "___" + endArg;
            }
        }
        else if(seqType.find("%range") == 0)
        {
            StringList md;
            if(seqType.find("%range:") == 0)
            {
                md.push_back("cpp:type:" + seqType.substr(strlen("%range:")));
            }
            endArg = "___" + endArg;
        }
    }
    return endArg;
}

void
Slice::writeEndCode(Output& out, const ParamDeclList& params, const OperationPtr& op, bool prepend)
{
    string prefix = prepend ? paramPrefix : "";
    for(ParamDeclList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        writeParamEndCode(out, (*p)->type(), (*p)->optional(), fixKwd(prefix + (*p)->name()), (*p)->getMetaData());
    }
    if(op && op->returnType())
    {
        writeParamEndCode(out, op->returnType(), op->returnIsOptional(), "__ret", op->getMetaData());
    }
}

string
Slice::findMetaData(const StringList& metaData, int typeCtx)
{
    static const string prefix = "cpp:";

    for(StringList::const_iterator q = metaData.begin(); q != metaData.end(); ++q)
    {
        string str = *q;
        if(str.find(prefix) == 0)
        {
            string::size_type pos = str.find(':', prefix.size());

            //
            // If the form is cpp:type:<...> the data after cpp:type:
            // is returned.
            // If the form is cpp:view-type:<...> the data after the
            // cpp:view-type: is returned
            // If the form is cpp:range[:<...>], cpp:array or cpp:class,
            // the return value is % followed by the string after cpp:.
            //
            // The priority of the metadata is as follows:
            // 1: protobuf
            // 2: array, range, view-type for "view" parameters
            // 3: class

            if(pos != string::npos)
            {
                string ss = str.substr(prefix.size());

                if(ss.find("protobuf:") == 0)
                {
                    return str.substr(pos + 1);
                }

                if(typeCtx & (TypeContextInParam | TypeContextAMIPrivateEnd))
                {
                    if(ss.find("range:") == 0)
                    {
                        return string("%") + str.substr(prefix.size());
                    }
                    else if(ss.find("view-type:") == 0)
                    {
                        return str.substr(pos + 1);
                    }
                }

                if(ss.find("type:") == 0)
                {
                    return str.substr(pos + 1);
                }
            }
            else if(typeCtx & (TypeContextInParam | TypeContextAMIPrivateEnd))
            {
                string ss = str.substr(prefix.size());
                if(ss == "array")
                {
                    return "%array";
                }
                else if(ss == "range")
                {
                    return "%range";
                }
            }
            //
            // Otherwise if the data is "class" it is returned.
            //
            else
            {
                string ss = str.substr(prefix.size());
                if(ss == "class")
                {
                    return "%class";
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
    while(cont)
    {
        ModulePtr mod = ModulePtr::dynamicCast(cont);
        if(!mod)
        {
            break;
        }
        StringList metaData = mod->getMetaData();
        if(find(metaData.begin(), metaData.end(), "cpp:type:wstring") != metaData.end())
        {
            return true;
        }
        else if(find(metaData.begin(), metaData.end(), "cpp:type:string") != metaData.end())
        {
            return false;
        }
        cont = mod->container();
    }
    return false;
}


string
Slice::getDataMemberRef(const DataMemberPtr& p)
{
    string name = fixKwd(p->name());
    if(!p->optional())
    {
        return name;
    }

    if(BuiltinPtr::dynamicCast(p->type()))
    {
        return "*" + name;
    }
    else
    {
        return "(*" + name + ")";
    }
}
