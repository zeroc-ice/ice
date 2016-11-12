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

string
condString(bool ok, const string& str)
{
    return ok ? str : "";
}

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
toOptional(const string& s, int typeCtx)
{
    bool cpp11 = (typeCtx & TypeContextCpp11) != 0;
    string result = cpp11 ? "Ice::optional" : "IceUtil::Optional";
    result += '<';
    if(cpp11)
    {
        result += s;
    }
    else
    {
        result += toTemplateArg(s);
    }

    result += '>';
    return result;
}


string
stringTypeToString(const TypePtr& type, const StringList& metaData, int typeCtx)
{
    string strType = findMetaData(metaData, typeCtx);
    if(strType == "wstring" || (typeCtx & TypeContextUseWstring && strType == ""))
    {
        return "::std::wstring";
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
            string s = typeToString(seq->type(), seq->typeMetaData(), typeCtx | (inWstringModule(seq) ? TypeContextUseWstring : 0));
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
        s = toOptional(s, typeCtx);
    }
    out << nl << s << ' ' << fixedName << ';';

    if((typeCtx & TypeContextCpp11) || !(typeCtx & TypeContextInParam) || !endArg)
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
            s = typeToString(seq, md, 0);
        }

        if(!s.empty())
        {
            if(optional)
            {
                s = toOptional(s, typeCtx);
            }
            out << nl << s << ' ' << fixedName << "_tmp_;";
        }
    }
}

void
writeParamEndCode(Output& out, const TypePtr& type, bool optional, const string& fixedName, const StringList& metaData,
                  const string& obj = "")
{
    string objPrefix = obj.empty() ? obj : obj + ".";
    string paramName = objPrefix + fixedName;
    string escapedParamName = objPrefix + fixedName + "_tmp_";

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
                    out << nl << "if(" << escapedParamName << ")";
                    out << sb;
                    out << nl << paramName << " = " << escapedParamName << "->second;";
                    out << eb;
                }
                else
                {
                    out << nl << paramName << " = " << escapedParamName << ".second;";
                }
            }
            else if(!builtin ||
                    builtin->kind() == Builtin::KindString ||
                    builtin->kind() == Builtin::KindObject ||
                    builtin->kind() == Builtin::KindObjectProxy)
            {
                if(optional)
                {
                    out << nl << "if(" << escapedParamName << ")";
                    out << sb;
                    out << nl << paramName << ".__setIsSet();";
                    out << nl << "if(!" << escapedParamName << "->empty())";
                    out << sb;
                    out << nl << paramName << "->first" << " = &(*" << escapedParamName << ")[0];";
                    out << nl << paramName << "->second" << " = " << paramName << "->first + " << escapedParamName << "->size();";
                    out << eb;
                    out << nl << "else";
                    out << sb;
                    out << nl << paramName << "->first" << " = " << paramName << "->second" << " = 0;";
                    out << eb;
                    out << eb;
                }
                else
                {
                    out << nl << "if(!" << escapedParamName << ".empty())";
                    out << sb;
                    out << nl << paramName << ".first" << " = &" << escapedParamName << "[0];";
                    out << nl << paramName << ".second" << " = " << paramName << ".first + " << escapedParamName << ".size();";
                    out << eb;
                    out << nl << "else";
                    out << sb;
                    out << nl << paramName << ".first" << " = " << paramName << ".second" << " = 0;";
                    out << eb;
                }
            }
        }
        else if(seqType.find("%range") == 0)
        {
            if(optional)
            {
                out << nl << "if(" << escapedParamName << ")";
                out << sb;
                out << nl << paramName << ".__setIsSet();";
                out << nl << paramName << "->first = (*" << escapedParamName << ").begin();";
                out << nl << paramName << "->second = (*" << escapedParamName << ").end();";
                out << eb;
            }
            else
            {
                out << nl << paramName << ".first = " << escapedParamName << ".begin();";
                out << nl << paramName << ".second = " << escapedParamName << ".end();";
            }
        }
    }
}

void
writeMarshalUnmarshalParams(Output& out, const ParamDeclList& params, const OperationPtr& op, bool marshal,
                            bool prepend, int typeCtx, const string& customStream = "", const string& retP = "", const string& obj = "")
{
    string prefix = prepend ? paramPrefix : "";
    string returnValueS = retP.empty() ? string("ret") : retP;
    string objPrefix = obj.empty() ? obj : obj + ".";

    string stream = customStream;
    if(stream.empty())
    {
        stream = marshal ? "ostr" : "istr";
    }

    bool cpp11 = (typeCtx & TypeContextCpp11) != 0;

    //
    // Marshal non optional parameters.
    //
    ParamDeclList requiredParams;
    ParamDeclList optionals;
    for(ParamDeclList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        if((*p)->optional())
        {
            optionals.push_back(*p);
        }
        else
        {
            requiredParams.push_back(*p);
        }
    }


    if(!requiredParams.empty() || (op && op->returnType() && !op->returnIsOptional()))
    {
        if(cpp11)
        {
            out << nl;
            if(marshal)
            {
                out << stream << "->writeAll";
            }
            else
            {
                out << stream << "->readAll";
            }
            out << spar;
            for(ParamDeclList::const_iterator p = requiredParams.begin(); p != requiredParams.end(); ++p)
            {
                out << objPrefix + fixKwd(prefix + (*p)->name());
            }
            if(op && op->returnType() && !op->returnIsOptional())
            {
                out << objPrefix + returnValueS;
            }
            out << epar << ";";
        }
        else
        {
            for(ParamDeclList::const_iterator p = requiredParams.begin(); p != requiredParams.end(); ++p)
            {
                writeMarshalUnmarshalCode(out, (*p)->type(), false, 0, fixKwd(prefix + (*p)->name()), marshal, (*p)->getMetaData(),
                                          typeCtx, customStream, true, obj);
            }

            if(op && op->returnType())
            {
                if(!op->returnIsOptional())
                {
                    writeMarshalUnmarshalCode(out, op->returnType(), false, 0, returnValueS, marshal, op->getMetaData(), typeCtx,
                                              customStream, true, obj);
                }
            }
        }
    }

    if(!optionals.empty() || (op && op->returnType() && op->returnIsOptional()))
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

        if(cpp11)
        {
            out << nl;
            if(marshal)
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
                for(ParamDeclList::const_iterator p = optionals.begin(); p != optionals.end(); ++p)
                {
                    if(checkReturnType && op->returnTag() < (*p)->tag())
                    {
                        os << condString(insertComma, ", ") << op->returnTag();
                        checkReturnType = false;
                        insertComma = true;
                    }
                    os << condString(insertComma, ", ") << (*p)->tag();
                    insertComma = true;
                }
                if(checkReturnType)
                {
                    os << condString(insertComma, ", ") << op->returnTag();
                }
                os << '}';
                out << os.str();
            }

            {
                //
                // Parameters
                //
                bool checkReturnType = op && op->returnIsOptional();
                for(ParamDeclList::const_iterator p = optionals.begin(); p != optionals.end(); ++p)
                {
                    if(checkReturnType && op->returnTag() < (*p)->tag())
                    {
                        out << objPrefix + returnValueS;
                        checkReturnType = false;
                    }
                    out << objPrefix + fixKwd(prefix + (*p)->name());
                }
                if(checkReturnType)
                {
                    out << objPrefix + returnValueS;
                }
            }
            out << epar << ";";
        }
        else
        {

            //
            // Marshal optional parameters.
            //

            bool checkReturnType = op && op->returnIsOptional();
            for(ParamDeclList::const_iterator p = optionals.begin(); p != optionals.end(); ++p)
            {
                if(checkReturnType && op->returnTag() < (*p)->tag())
                {
                    writeMarshalUnmarshalCode(out, op->returnType(), true, op->returnTag(), returnValueS, marshal,
                                              op->getMetaData(), typeCtx, customStream, true, obj);

                    checkReturnType = false;
                }
                writeMarshalUnmarshalCode(out, (*p)->type(), true, (*p)->tag(), fixKwd(prefix + (*p)->name()), marshal,
                                          (*p)->getMetaData(), typeCtx, customStream, true, obj);
            }
            if(checkReturnType)
            {
                writeMarshalUnmarshalCode(out, op->returnType(), true, op->returnTag(), returnValueS, marshal, op->getMetaData(),
                                          typeCtx, customStream, true, obj);
            }
        }
    }
}
}

Slice::FeatureProfile Slice::featureProfile = Slice::Ice;
string Slice::paramPrefix = "iceP_";

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
        out << "\n#   if defined(ICE_STATIC_LIBS)";
        out << "\n#       define " << dllExport << " /**/";
        out << "\n#   elif defined(" << dllExport << "_EXPORTS)";
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
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindString:
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindLocalObject:
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
    return !EnumPtr::dynamicCast(type);
}

string
Slice::typeToString(const TypePtr& type, const StringList& metaData, int typeCtx)
{
    bool cpp11 = (typeCtx & TypeContextCpp11) != 0;

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
        "::Ice::LocalObjectPtr",
        "::Ice::ValuePtr"
    };

    static const char* cpp11BuiltinTable[] =
    {
        "::Ice::Byte",
        "bool",
        "short",
        "int",
        "long long int",
        "float",
        "double",
        "::std::string",
        "::std::shared_ptr<::Ice::Object>",
        "::std::shared_ptr<::Ice::ObjectPrx>",
        "::std::shared_ptr<void>",
        "::std::shared_ptr<::Ice::Value>"
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
            if(cpp11)
            {
                if(builtin->kind() == Builtin::KindObject && !(typeCtx & TypeContextLocal))
                {
                    return "::std::shared_ptr<::Ice::Value>";
                }
                else
                {
                    return cpp11BuiltinTable[builtin->kind()];
                }
            }
            else
            {
                return builtinTable[builtin->kind()];
            }
        }
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        if(cpp11)
        {
            if(cl->definition() && cl->definition()->isDelegate())
            {
                return classDefToDelegateString(cl->definition());
            }
            else if(cl->isInterface() && !cl->isLocal())
            {
                return "std::shared_ptr<::Ice::Value>";
            }
            else
            {
                return "::std::shared_ptr<" + cl->scoped() + ">";
            }
        }
        else
        {
            return cl->scoped() + "Ptr";
        }
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        //
        // C++11 mapping doesn't accept cpp:class metadata
        //
        if(!cpp11 && findMetaData(st->getMetaData()) == "%class")
        {
            return fixKwd(st->scoped() + "Ptr");
        }
        return fixKwd(st->scoped());
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        if(cpp11)
        {
            ClassDefPtr def = proxy->_class()->definition();
            //
            // Non local classes without operations map to the base
            // proxy class shared_ptr<Ice::ObjectPrx>
            //
            if(def && !def->isInterface() && def->allOperations().empty())
            {
                return "::std::shared_ptr<::Ice::ObjectPrx>";
            }
            else
            {
                return "::std::shared_ptr<" + fixKwd(proxy->_class()->scoped() + "Prx") + ">";
            }
        }
        else
        {
            return fixKwd(proxy->_class()->scoped() + "Prx");
        }
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
        return toOptional(typeToString(type, metaData, typeCtx), typeCtx);
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
        return toOptional(typeToString(type, metaData, typeCtx), typeCtx);
    }

    return typeToString(type, metaData, typeCtx);
}

string
Slice::inputTypeToString(const TypePtr& type, bool optional, const StringList& metaData, int typeCtx)
{
    bool cpp11 = (typeCtx & TypeContextCpp11) != 0;

    static const char* cpp98InputBuiltinTable[] =
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
        "const ::Ice::LocalObjectPtr&",
        "const ::Ice::ValuePtr&"
    };

    static const char* cpp11InputBuiltinTable[] =
    {
        "::Ice::Byte",
        "bool",
        "short",
        "int",
        "long long int",
        "float",
        "double",
        "const ::std::string&",
        "const ::std::shared_ptr<::Ice::Object>&",
        "const ::std::shared_ptr<::Ice::ObjectPrx>&",
        "const ::std::shared_ptr<void>&",
        "const ::std::shared_ptr<::Ice::Value>&"
    };

    typeCtx |= TypeContextInParam;

    if(optional)
    {
        return "const " + toOptional(typeToString(type, metaData, typeCtx), typeCtx) + '&';
    }

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        if(builtin->kind() == Builtin::KindString)
        {
            return string("const ") + stringTypeToString(type, metaData, typeCtx) + '&';
        }
        else
        {
            if(cpp11)
            {
                if(builtin->kind() == Builtin::KindObject && !(typeCtx & TypeContextLocal))
                {
                    return "const ::std::shared_ptr<::Ice::Value>&";
                }
                else
                {
                    return cpp11InputBuiltinTable[builtin->kind()];
                }
            }
            else
            {
                return cpp98InputBuiltinTable[builtin->kind()];
            }
        }
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        if(cpp11)
        {
            if(cl->definition() && cl->definition()->isDelegate())
            {
                return classDefToDelegateString(cl->definition(), typeCtx);
            }
            else if(cl->isInterface() && !cl->isLocal())
            {
                return "const ::std::shared_ptr<::Ice::Value>&";
            }
            else
            {
                return "const ::std::shared_ptr<" + fixKwd(cl->scoped()) + ">&";
            }
        }
        else
        {
            return "const " + fixKwd(cl->scoped() + "Ptr&");
        }
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(cpp11)
        {
            return "const " + fixKwd(st->scoped()) + "&";
        }
        else
        {
            if(findMetaData(st->getMetaData()) == "%class")
            {
                return "const " + fixKwd(st->scoped() + "Ptr&");
            }
            else
            {
                return "const " + fixKwd(st->scoped()) + "&";
            }
        }
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        if(cpp11)
        {
            ClassDefPtr def = proxy->_class()->definition();
            if(def && !def->isInterface() && def->allOperations().empty())
            {
                return "const ::std::shared_ptr<::Ice::ObjectPrx>&";
            }
            else
            {
                return "const ::std::shared_ptr<" + fixKwd(proxy->_class()->scoped() + "Prx") + ">&";
            }
        }
        else
        {
            return "const " + fixKwd(proxy->_class()->scoped() + "Prx&");
        }
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
    bool cpp11 = (typeCtx & TypeContextCpp11) != 0;

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
        "::Ice::ObjectPrxPtr&",
        "::Ice::LocalObjectPtr&",
        "::Ice::ValuePtr&"
    };

    static const char* cpp11OutputBuiltinTable[] =
    {
        "::Ice::Byte&",
        "bool&",
        "short&",
        "int&",
        "long long int&",
        "float&",
        "double&",
        "::std::string&",
        "::std::shared_ptr<::Ice::Object>&",
        "::std::shared_ptr<::Ice::ObjectPrx>&",
        "::std::shared_ptr<void>&",
        "::std::shared_ptr<::Ice::Value>&"
    };

    if(optional)
    {
        return toOptional(typeToString(type, metaData, typeCtx), typeCtx) + '&';
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
            if(cpp11)
            {
                if(builtin->kind() == Builtin::KindObject && !(typeCtx & TypeContextLocal))
                {
                    return "::std::shared_ptr<::Ice::Value>";
                }
                else
                {
                    return cpp11OutputBuiltinTable[builtin->kind()];
                }
            }
            else
            {
                return outputBuiltinTable[builtin->kind()];
            }
        }
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        if(cpp11)
        {
            if(cl->definition() && cl->definition()->isDelegate())
            {
                return classDefToDelegateString(cl->definition(), typeCtx) + "&";
            }
            else if(cl->isInterface() && !cl->isLocal())
            {
                return "::std::shared_ptr<::Ice::Value>&";
            }
            else
            {
                return "::std::shared_ptr<" + fixKwd(cl->scoped()) + ">&";
            }
        }
        else
        {
            return fixKwd(cl->scoped() + "Ptr&");
        }
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(!cpp11 && findMetaData(st->getMetaData()) == "%class")
        {
            return fixKwd(st->scoped() + "Ptr&");
        }
        else
        {
            return fixKwd(st->scoped()) + "&";
        }
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        if(cpp11)
        {
            ClassDefPtr def = proxy->_class()->definition();
            //
            // Non local classes without operations map to the base
            // proxy class shared_ptr<Ice::ObjectPrx>
            //
            if(def && !def->isInterface() && def->allOperations().empty())
            {
                return "::std::shared_ptr<::Ice::ObjectPrx>";
            }
            else
            {
                return "::std::shared_ptr<" + fixKwd(proxy->_class()->scoped() + "Prx") + ">&";
            }
        }
        else
        {
            return fixKwd(proxy->_class()->scoped() + "Prx&");
        }
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
Slice::operationModeToString(Operation::Mode mode, bool cpp11)
{
    switch(mode)
    {
        case Operation::Normal:
        {
            return cpp11 ? "::Ice::OperationMode::Normal" : "::Ice::Normal";
        }

        case Operation::Nonmutating:
        {
            return cpp11 ? "::Ice::OperationMode::Nonmutating" : "::Ice::Nonmutating";
        }

        case Operation::Idempotent:
        {
            return cpp11 ? "::Ice::OperationMode::Idempotent" : "::Ice::Idempotent";
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
    static const string keywordList[] =
    {
        "alignas", "alignof", "and", "and_eq", "asm", "auto", "bit_and", "bit_or", "bool", "break", 
        "case", "catch", "char", "char16_t", "char32_t", "class", "compl", "const", "const_exptr", "const_cast", "continue", 
        "decltype", "default", "delete", "do", "double", "dynamic_cast", 
        "else", "enum", "explicit", "export", "extern", "false", "float", "for", "friend", 
        "goto", "if", "inline", "int", "long", "mutable", "namespace", "new", "noexcept", "not", "not_eq",
        "operator", "or", "or_eq", "private", "protected", "public", "register", "reinterpret_cast", "return", 
        "short", "signed", "sizeof", "static", "static_assert", "static_cast", "struct", "switch", 
        "template", "this", "thread_local", "throw", "true", "try", "typedef", "typeid", "typename", 
        "union", "unsigned", "using", "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq"
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
// their "_cpp_"-prefixed version; otherwise, if the passed name is
// not scoped, but a C++ keyword, return the "_cpp_"-prefixed name;
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
                                 bool marshal, const StringList& metaData, int typeCtx, const string& customStream, bool pointer,
                                 const string& obj)
{
    string objPrefix = obj.empty() ? obj : obj + ".";

    ostringstream os;
    if(customStream.empty())
    {
        os << (marshal ? "ostr" : "istr");
    }
    else
    {
        os << customStream;
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
                    out << nl << func << objPrefix << param << ");";
                    return;
                }

                out << nl << func << objPrefix << param << "_tmp_);";
                writeParamEndCode(out, seq, optional, param, metaData, obj);
                return;
            }
            else if(seqType.find("%range") == 0)
            {
                out << nl << func << objPrefix << param << "_tmp_);";
                writeParamEndCode(out, seq, optional, param, metaData, obj);
                return;
            }
        }
    }

    out << nl << func << objPrefix << param << ");";
}

void
Slice::writeMarshalCode(Output& out, const ParamDeclList& params, const OperationPtr& op, bool prepend, int typeCtx,
                        const string& customStream, const string& retP)
{
    writeMarshalUnmarshalParams(out, params, op, true, prepend, typeCtx, customStream, retP);
}

void
Slice::writeUnmarshalCode(Output& out, const ParamDeclList& params, const OperationPtr& op, bool prepend, int typeCtx,
                          const string& customStream, const string& retP, const string& obj)
{
    writeMarshalUnmarshalParams(out, params, op, false, prepend, typeCtx, customStream, retP, obj);
}

void
Slice::writeAllocateCode(Output& out, const ParamDeclList& params, const OperationPtr& op, bool prepend, int typeCtx,
                         const string& customRet)
{
    string prefix = prepend ? paramPrefix : "";
    string returnValueS = customRet;
    if(returnValueS.empty())
    {
        returnValueS = "ret";
    }

    for(ParamDeclList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        writeParamAllocateCode(out, (*p)->type(), (*p)->optional(), fixKwd(prefix + (*p)->name()), (*p)->getMetaData(),
                               typeCtx, getEndArg((*p)->type(), (*p)->getMetaData(), (*p)->name()) != (*p)->name());
    }

    if(op && op->returnType())
    {
        writeParamAllocateCode(out, op->returnType(), op->returnIsOptional(), returnValueS, op->getMetaData(), typeCtx,
                               getEndArg(op->returnType(), op->getMetaData(), returnValueS) != returnValueS);
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
                endArg += "_tmp_";
            }
            else if(!builtin || builtin->kind() != Builtin::KindByte)
            {
                endArg += "_tmp_";
            }
        }
        else if(seqType.find("%range") == 0)
        {
            StringList md;
            if(seqType.find("%range:") == 0)
            {
                md.push_back("cpp:type:" + seqType.substr(strlen("%range:")));
            }
            endArg += "_tmp_";
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
        writeParamEndCode(out, op->returnType(), op->returnIsOptional(), "ret", op->getMetaData());
    }
}

void
Slice::writeMarshalUnmarshalDataMemberInHolder(IceUtilInternal::Output& C,
                                               const string& holder,
                                               const DataMemberPtr& p,
                                               bool marshal)
{
    writeMarshalUnmarshalCode(C, p->type(), p->optional(), p->tag(), holder + fixKwd(p->name()), marshal,
                              p->getMetaData());
}

void
Slice::writeMarshalUnmarshalAllInHolder(IceUtilInternal::Output& out,
                                          const string& holder,
                                          const DataMemberList& dataMembers,
                                          bool optional,
                                          bool marshal)
{
    if(dataMembers.empty())
    {
        return;
    }

    string stream = marshal ? "ostr" : "istr";
    string streamOp = marshal ? "writeAll" : "readAll";

    out << nl << stream << "->" << streamOp;
    out << spar;

    if(optional)
    {
        ostringstream os;
        os << "{";
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            if(q != dataMembers.begin())
            {
                os << ", ";
            }
            os << (*q)->tag();
        }
        os << "}";
        out << os.str();
    }

    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        out << holder + fixKwd((*q)->name());
    }

    out << epar << ";";

}

void
Slice::writeStreamHelpers(Output& out,
                          const ContainedPtr& c,
                          DataMemberList dataMembers,
                          bool hasBaseDataMembers,
                          bool checkClassMetaData,
                          bool cpp11)
{
    // If c is a C++11 class/exception whose base class contains data members (recursively), then we need to generate
    // an StreamWriter even if its implementation is empty. This is becuase our default marsaling uses ice_tuple() which
    // contains all of our class/exception's data members as well the base data members, which breaks marshaling. This
    // is not an issue for structs.
    if(dataMembers.empty() && !(cpp11 && hasBaseDataMembers))
    {
        return;
    }

    DataMemberList requiredMembers;
    DataMemberList optionalMembers;

    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        if((*q)->optional())
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
        static bool compare(const DataMemberPtr& lhs, const DataMemberPtr& rhs)
        {
            return lhs->tag() < rhs->tag();
        }
    };
    optionalMembers.sort(SortFn::compare);

    string scoped = c->scoped();
    bool classMetaData = checkClassMetaData ? (findMetaData(c->getMetaData(), false) == "%class") : false;
    string fullName = classMetaData ? fixKwd(scoped + "Ptr") : fixKwd(scoped);
    string holder = classMetaData ? "v->" : "v.";

    //
    // Generate StreamWriter
    //
    // Only generate StreamWriter specializations if we are generating for C++98 or
    // we are generating for C++11 with optional data members and no base class data members
    //
    if(!cpp11 || !optionalMembers.empty() || hasBaseDataMembers)
    {
        out << nl << "template<typename S>";
        out << nl << "struct StreamWriter" << (cpp11 ? "<" : "< ") << fullName << ", S>";
        out << sb;
        if(requiredMembers.empty() && optionalMembers.empty())
        {
            out << nl << "static void write(S*, const " << fullName << "&)";
        }
        else
        {
            out << nl << "static void write(S* ostr, const " << fullName << "& v)";
        }

        out << sb;

        if(cpp11)
        {
            writeMarshalUnmarshalAllInHolder(out, holder, requiredMembers, false, true);
            writeMarshalUnmarshalAllInHolder(out, holder, optionalMembers, true, true);
        }
        else
        {
            for(DataMemberList::const_iterator q = requiredMembers.begin(); q != requiredMembers.end(); ++q)
            {
                writeMarshalUnmarshalDataMemberInHolder(out, holder, *q, true);
            }

            for(DataMemberList::const_iterator q = optionalMembers.begin(); q != optionalMembers.end(); ++q)
            {
                writeMarshalUnmarshalDataMemberInHolder(out, holder, *q, true);
            }
        }

        out << eb;
        out << eb << ";" << nl;
    }

    //
    // Generate StreamWriter
    //
    out << nl << "template<typename S>";
    out << nl << "struct StreamReader" << (cpp11 ? "<" : "< ") << fullName << ", S>";
    out << sb;
    if (requiredMembers.empty() && optionalMembers.empty())
    {
        out << nl << "static void read(S*, " << fullName << "&)";
    }
    else
    {
        out << nl << "static void read(S* istr, " << fullName << "& v)";
    }

    out << sb;

    if(cpp11)
    {
        writeMarshalUnmarshalAllInHolder(out, holder, requiredMembers, false, false);
        writeMarshalUnmarshalAllInHolder(out, holder, optionalMembers, true, false);
    }
    else
    {
        for(DataMemberList::const_iterator q = requiredMembers.begin(); q != requiredMembers.end(); ++q)
        {
            writeMarshalUnmarshalDataMemberInHolder(out, holder, *q, false);
        }

        for(DataMemberList::const_iterator q = optionalMembers.begin(); q != optionalMembers.end(); ++q)
        {
            writeMarshalUnmarshalDataMemberInHolder(out, holder, *q, false);
        }
    }

    out << eb;
    out << eb << ";" << nl;
}

void
Slice::writeIceTuple(::IceUtilInternal::Output& out, DataMemberList dataMembers, int typeCtx)
{
    out << sp << nl << "std::tuple<";
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        if(q != dataMembers.begin())
        {
            out << ", ";
        }
        out << "const ";
        out << typeToString((*q)->type(), (*q)->optional(), (*q)->getMetaData(), typeCtx | TypeContextCpp11) << "&";
    }
    out << "> ice_tuple() const";

    out << sb;
    out << nl << "return std::tie(";
    for(DataMemberList::const_iterator pi = dataMembers.begin(); pi != dataMembers.end(); ++pi)
    {
        if(pi != dataMembers.begin())
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
    if(findMetaData(prefix, cl->getMetaData(), value))
    {
        return true;
    }

    ClassDefPtr def = cl->definition();
    return def ? findMetaData(prefix, def->getMetaData(), value) : false;
}

bool
Slice::findMetaData(const string& prefix, const StringList& metaData, string& value)
{
    for(StringList::const_iterator i = metaData.begin(); i != metaData.end(); i++)
    {
        string s = *i;
        if(s.find(prefix) == 0)
        {
            value = s.substr(prefix.size());
            return true;
        }
    }
    return false;
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
            // 1: array, range (C++98 only), view-type for "view" parameters
            // 2: class (C++98 only), unscoped (C++11 only)
            //

            if(pos != string::npos)
            {
                string ss = str.substr(prefix.size());

                if(typeCtx & (TypeContextInParam | TypeContextAMIPrivateEnd))
                {
                    if(ss.find("view-type:") == 0)
                    {
                        return str.substr(pos + 1);
                    }
                    else if(ss.find("range:") == 0 && !(typeCtx & TypeContextCpp11))
                    {
                        return string("%") + str.substr(prefix.size());
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
                else if(ss == "range" && !(typeCtx & TypeContextCpp11))
                {
                    return "%range";
                }
            }
            //
            // Otherwise if the data is "class", "unscoped" it is returned.
            //
            else
            {
                string ss = str.substr(prefix.size());
                if(ss == "class" && !(typeCtx & TypeContextCpp11))
                {
                    return "%class";
                }
                else if(ss == "unscoped" && (typeCtx & TypeContextCpp11))
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

string
Slice::classDefToDelegateString(const ClassDefPtr& cl, int typeCtx)
{
    assert(cl->isDelegate());

    // A delegate only has one operation
    OperationPtr op = cl->allOperations().front();

    TypePtr ret = op->returnType();
    string retS = returnTypeToString(ret, op->returnIsOptional(), op->getMetaData(), typeCtx);

    string t = "::std::function<" + retS + "(";

    ParamDeclList paramList = cl->allOperations().front()->parameters();
    for(ParamDeclList::iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if((*q)->isOutParam())
        {
            t += outputTypeToString((*q)->type(), (*q)->optional(), (*q)->getMetaData(), typeCtx);
        }
        else
        {
            t += inputTypeToString((*q)->type(), (*q)->optional(), (*q)->getMetaData(), typeCtx);
        }

        t += distance(q, paramList.end()) == 1  ? "" : ", ";
    }

    t += ")>";
    return t;
}
