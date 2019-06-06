//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Gen.h"
#include <Slice/Util.h>
#include <Slice/CPlusPlusUtil.h>
#include <IceUtil/Functional.h>
#include <IceUtil/Iterator.h>
#include <IceUtil/StringUtil.h>
#include <Slice/Checksum.h>
#include <Slice/FileTracker.h>
#include <IceUtil/FileUtil.h>

#include <limits>
#include <string.h>

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

namespace
{

bool
isConstexprType(const TypePtr& type)
{
    BuiltinPtr bp = BuiltinPtr::dynamicCast(type);
    if(bp)
    {
        switch(bp->kind())
        {
            case Builtin::KindByte:
            case Builtin::KindBool:
            case Builtin::KindShort:
            case Builtin::KindInt:
            case Builtin::KindLong:
            case Builtin::KindFloat:
            case Builtin::KindDouble:
            case Builtin::KindValue:
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            {
                return true;
            }
            default:
            {
                return false;
            }
        }
    }
    else if(EnumPtr::dynamicCast(type) || ProxyPtr::dynamicCast(type) || ClassDeclPtr::dynamicCast(type))
    {
        return true;
    }
    else
    {
        StructPtr s = StructPtr::dynamicCast(type);
        if(s)
        {
            DataMemberList members = s->dataMembers();
            for(DataMemberList::const_iterator i = members.begin(); i != members.end(); ++i)
            {
                if(!isConstexprType((*i)->type()))
                {
                    return false;
                }
            }
            return true;
        }
        return false;
    }
}

string
getDeprecateSymbol(const ContainedPtr& p1, const ContainedPtr& p2)
{
    string deprecateMetadata, deprecateSymbol;
    if(p1->findMetaData("deprecate", deprecateMetadata) ||
       (p2 != 0 && p2->findMetaData("deprecate", deprecateMetadata)))
    {
        string msg = "is deprecated";
        if(deprecateMetadata.find("deprecate:") == 0 && deprecateMetadata.size() > 10)
        {
            msg = deprecateMetadata.substr(10);
        }
        deprecateSymbol = "ICE_DEPRECATED_API(\"" + msg + "\") ";
    }
    return deprecateSymbol;
}

void
writeConstantValue(IceUtilInternal::Output& out, const TypePtr& type, const SyntaxTreeBasePtr& valueType,
                   const string& value, int typeContext, const StringList& metaData, const string& scope)
{
    ConstPtr constant = ConstPtr::dynamicCast(valueType);
    if(constant)
    {
        out << getUnqualified(fixKwd(constant->scoped()), scope);
    }
    else
    {
        bool cpp11 = (typeContext & TypeContextCpp11) == TypeContextCpp11;
        BuiltinPtr bp = BuiltinPtr::dynamicCast(type);
        if(bp && bp->kind() == Builtin::KindString)
        {
            bool wide = (typeContext & TypeContextUseWstring) || findMetaData(metaData) == "wstring";
            if(wide || cpp11)
            {
                out << (wide ? "L\"" : "u8\"");
                out << toStringLiteral(value, "\a\b\f\n\r\t\v", "?", UCN, cpp11 ? 0 : 0x9F + 1);
                out << "\"";
            }
            else // C++98 narrow strings
            {
                out << "\"" << toStringLiteral(value, "\a\b\f\n\r\t\v", "?", Octal, 0) << "\"";
            }
        }
        else if(bp && bp->kind() == Builtin::KindLong)
        {
            if(cpp11)
            {
                out << value << "LL";
            }
            else
            {
                out << "ICE_INT64(" << value << ")";
            }
        }
        else if(bp && bp->kind() == Builtin::KindFloat)
        {
            out << value;
            if(value.find(".") == string::npos)
            {
                out << ".0";
            }
            out << "F";
        }
        else
        {
            EnumPtr ep = EnumPtr::dynamicCast(type);
            if(ep && valueType)
            {
                EnumeratorPtr enumerator = EnumeratorPtr::dynamicCast(valueType);
                assert(enumerator);

                bool unscoped = (cpp11 && findMetaData(ep->getMetaData(), TypeContextCpp11) == "%unscoped") ||
                    (!cpp11 && findMetaData(ep->getMetaData()) != "%scoped");

                if(unscoped)
                {
                    out << getUnqualified(fixKwd(ep->scope() + enumerator->name()), scope);
                }
                else
                {
                    if(cpp11)
                    {
                        out << getUnqualified(fixKwd(enumerator->scoped()), scope);
                    }
                    else
                    {
                        out << getUnqualified(fixKwd(ep->scope() + ep->name() + enumerator->name()), scope);
                    }
                }
            }
            else if(!ep)
            {
                out << value;
            }
        }
    }
}

string
toDllClassExport(const string& dllExport)
{
    if(!dllExport.empty())
    {
        return "ICE_CLASS(" + dllExport.substr(0, dllExport.size() - 1) + ") ";
    }
    else
    {
        return "";
    }
}

string
toDllMemberExport(const string& dllExport)
{
    if(!dllExport.empty())
    {
        return "ICE_MEMBER(" + dllExport.substr(0, dllExport.size() - 1) + ") ";
    }
    else
    {
        return "";
    }
}

void
writeDataMemberInitializers(IceUtilInternal::Output& C, const DataMemberList& members, int typeContext)
{
    bool first = true;
    for(DataMemberList::const_iterator p = members.begin(); p != members.end(); ++p)
    {
        ContainedPtr contained = ContainedPtr::dynamicCast((*p)->container());
        string scope = contained->scope();
        if((*p)->defaultValueType())
        {
            string memberName = fixKwd((*p)->name());

            if(first)
            {
                first = false;
            }
            else
            {
                C << ',';
            }
            C << nl << memberName << '(';
            writeConstantValue(C, (*p)->type(), (*p)->defaultValueType(), (*p)->defaultValue(), typeContext,
                               (*p)->getMetaData(), scope);
            C << ')';
        }
    }
}

void
writeInParamsLambda(IceUtilInternal::Output& C, const OperationPtr& p, const ParamDeclList& inParams,
                    const string& scope)
{
    if(inParams.empty())
    {
        C << "nullptr";
    }
    else
    {
        C << "[&](" << getUnqualified("::Ice::OutputStream*", scope) << " ostr)";
        C << sb;
        writeMarshalCode(C, inParams, 0, true, TypeContextInParam | TypeContextCpp11);
        if(p->sendsClasses(false))
        {
            C << nl << "ostr->writePendingValues();";
        }
        C << eb;
    }
}

void
throwUserExceptionLambda(IceUtilInternal::Output& C, ExceptionList throws, const string& scope)
{
    if(throws.empty())
    {
        C << "nullptr";
    }
    else
    {
        throws.sort();
        throws.unique();

        //
        // Arrange exceptions into most-derived to least-derived order. If we don't
        // do this, a base exception handler can appear before a derived exception
        // handler, causing compiler warnings and resulting in the base exception
        // being marshaled instead of the derived exception.
        //
        throws.sort(Slice::DerivedToBaseCompare());

        C << "[](const " << getUnqualified("::Ice::UserException&", scope) << " ex)";
        C << sb;
        C << nl << "try";
        C << sb;
        C << nl << "ex.ice_throw();";
        C << eb;
        //
        // Generate a catch block for each legal user exception.
        //
        for(ExceptionList::const_iterator i = throws.begin(); i != throws.end(); ++i)
        {
            string scoped = (*i)->scoped();
            C << nl << "catch(const " << getUnqualified(fixKwd((*i)->scoped()), scope) << "&)";
            C << sb;
            C << nl << "throw;";
            C << eb;
        }
        C << nl << "catch(const " << getUnqualified("::Ice::UserException&", scope) << ")";
        C << sb;
        C << eb;
        C << eb;
    }
}

string
resultStructName(const string& name, const string& scope = "", bool marshaledResult = false)
{
    assert(!name.empty());
    string stName = IceUtilInternal::toUpper(name.substr(0, 1)) + name.substr(1);
    stName += marshaledResult ? "MarshaledResult" : "Result";
    if(!scope.empty())
    {
        stName = scope + "::" + stName;
    }
    return stName;
}

string
condMove(bool moveIt, const string& str)
{
    return moveIt ? string("::std::move(") + str + ")" : str;
}

string
escapeParam(const ParamDeclList& params, const string& name)
{
    string r = name;
    for(ParamDeclList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        if(fixKwd((*p)->name()) == name)
        {
            r = name + "_";
            break;
        }
    }
    return r;
}

void
writeDocLines(Output& out, const StringList& lines, bool commentFirst, const string& space = " ")
{
    StringList l = lines;
    if(!commentFirst)
    {
        out << l.front();
        l.pop_front();
    }
    for(StringList::const_iterator i = l.begin(); i != l.end(); ++i)
    {
        out << nl << " *";
        if(!i->empty())
        {
            out << space << *i;
        }
    }
}

void
writeSeeAlso(Output& out, const StringList& lines, const string& space = " ")
{
    for(StringList::const_iterator i = lines.begin(); i != lines.end(); ++i)
    {
        out << nl << " *";
        if(!i->empty())
        {
            out << space << "@see " << *i;
        }
    }
}

string
getDocSentence(const StringList& lines)
{
    //
    // Extract the first sentence.
    //
    ostringstream ostr;
    for(StringList::const_iterator i = lines.begin(); i != lines.end(); ++i)
    {
        const string ws = " \t";

        if(i->empty())
        {
            break;
        }
        if(i != lines.begin() && i->find_first_not_of(ws) == 0)
        {
            ostr << " ";
        }
        string::size_type pos = i->find('.');
        if(pos == string::npos)
        {
            ostr << *i;
        }
        else if(pos == i->size() - 1)
        {
            ostr << *i;
            break;
        }
        else
        {
            //
            // Assume a period followed by whitespace indicates the end of the sentence.
            //
            while(pos != string::npos)
            {
                if(ws.find((*i)[pos + 1]) != string::npos)
                {
                    break;
                }
                pos = i->find('.', pos + 1);
            }
            if(pos != string::npos)
            {
                ostr << i->substr(0, pos + 1);
                break;
            }
            else
            {
                ostr << *i;
            }
        }
    }

    return ostr.str();
}

void
writeDocSummary(Output& out, const ContainedPtr& p)
{
    if(p->comment().empty())
    {
        return;
    }

    CommentPtr doc = p->parseComment(false);

    out << nl << "/**";

    if(!doc->overview().empty())
    {
        writeDocLines(out, doc->overview(), true);
    }

    if(!doc->misc().empty())
    {
        writeDocLines(out, doc->misc(), true);
    }

    if(!doc->seeAlso().empty())
    {
        writeSeeAlso(out, doc->seeAlso());
    }

    if(!doc->deprecated().empty())
    {
        out << nl << " *";
        out << nl << " * @deprecated ";
        writeDocLines(out, doc->deprecated(), false);
    }
    else if(doc->isDeprecated())
    {
        out << nl << " *";
        out << nl << " * @deprecated";
    }

    switch(p->containedType())
    {
        case Contained::ContainedTypeClass:
        case Contained::ContainedTypeStruct:
        case Contained::ContainedTypeException:
        {
            UnitPtr unt = p->container()->unit();
            string file = p->file();
            assert(!file.empty());
            static const string prefix = "cpp:doxygen:include:";
            DefinitionContextPtr dc = unt->findDefinitionContext(file);
            assert(dc);
            string q = dc->findMetaData(prefix);
            if(!q.empty())
            {
                out << nl << " * \\headerfile " << q.substr(prefix.size());
            }
            break;
        }
        default:
            break;
    }

    out << nl << " */";
}

enum OpDocParamType { OpDocInParams, OpDocOutParams, OpDocAllParams };

void
writeOpDocParams(Output& out, const OperationPtr& op, const CommentPtr& doc, OpDocParamType type,
                 const StringList& preParams = StringList(), const StringList& postParams = StringList())
{
    ParamDeclList params;
    switch(type)
    {
        case OpDocInParams:
            params = op->inParameters();
            break;
        case OpDocOutParams:
            params = op->outParameters();
            break;
        case OpDocAllParams:
            params = op->parameters();
            break;
    }

    if(!preParams.empty())
    {
        writeDocLines(out, preParams, true);
    }

    map<string, StringList> paramDoc = doc->parameters();
    for(ParamDeclList::iterator p = params.begin(); p != params.end(); ++p)
    {
        map<string, StringList>::iterator q = paramDoc.find((*p)->name());
        if(q != paramDoc.end())
        {
            out << nl << " * @param " << fixKwd(q->first) << " ";
            writeDocLines(out, q->second, false);
        }
    }

    if(!postParams.empty())
    {
        writeDocLines(out, postParams, true);
    }
}

void
writeOpDocExceptions(Output& out, const OperationPtr& op, const CommentPtr& doc)
{
    map<string, StringList> exDoc = doc->exceptions();
    for(map<string, StringList>::iterator p = exDoc.begin(); p != exDoc.end(); ++p)
    {
        //
        // Try to locate the exception's definition using the name given in the comment.
        //
        string name = p->first;
        ExceptionPtr ex = op->container()->lookupException(name, false);
        if(ex)
        {
            name = ex->scoped().substr(2);
        }
        out << nl << " * @throws " << name << " ";
        writeDocLines(out, p->second, false);
    }
}

void
writeOpDocSummary(Output& out, const OperationPtr& op, const CommentPtr& doc, OpDocParamType type, bool showExceptions,
                  const StringList& preParams = StringList(), const StringList& postParams = StringList(),
                  const StringList& returns = StringList())
{
    out << nl << "/**";

    if(!doc->overview().empty())
    {
        writeDocLines(out, doc->overview(), true);
    }

    writeOpDocParams(out, op, doc, type, preParams, postParams);

    if(!returns.empty())
    {
        out << nl << " * @return ";
        writeDocLines(out, returns, false);
    }

    if(showExceptions)
    {
        writeOpDocExceptions(out, op, doc);
    }

    if(!doc->misc().empty())
    {
        writeDocLines(out, doc->misc(), true);
    }

    if(!doc->seeAlso().empty())
    {
        writeSeeAlso(out, doc->seeAlso());
    }

    if(!doc->deprecated().empty())
    {
        out << nl << " *";
        out << nl << " * @deprecated ";
        writeDocLines(out, doc->deprecated(), false);
    }
    else if(doc->isDeprecated())
    {
        out << nl << " *";
        out << nl << " * @deprecated";
    }

    out << nl << " */";
}

void
emitOpNameResult(IceUtilInternal::Output& H, const OperationPtr& p, int useWstring)
{
    string name = p->name();

    ContainerPtr container = p->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    string clScope = fixKwd(cl->scope());

    TypePtr ret = p->returnType();
    string retS = returnTypeToString(ret, p->returnIsOptional(), clScope, p->getMetaData(),
                                     useWstring | TypeContextCpp11);

    ParamDeclList outParams = p->outParameters();

    if((outParams.size() > 1) || (ret && outParams.size() > 0))
    {
        //
        // Generate OpNameResult struct
        //
        string returnValueS = "returnValue";

        for(ParamDeclList::iterator q = outParams.begin(); q != outParams.end(); ++q)
        {
            if((*q)->name() == "returnValue")
            {
                returnValueS = "_returnValue";
            }
        }

        H << sp;
        H << nl << "/**";
        H << nl << " * Encapsulates the results of a call to " << fixKwd(name) << ".";
        H << nl << " */";
        H << nl << "struct " << resultStructName(name);
        H << sb;
        CommentPtr comment = p->parseComment(false);
        map<string, StringList> paramComments;
        if(comment)
        {
            paramComments = comment->parameters();
        }
        if(ret)
        {
            if(comment && !comment->returns().empty())
            {
                H << nl << "/** " << getDocSentence(comment->returns()) << " */";
            }
            H << nl << retS << " " << returnValueS << ";";
        }
        for(ParamDeclList::iterator q = outParams.begin(); q != outParams.end(); ++q)
        {
            string typeString = typeToString((*q)->type(), (*q)->optional(), clScope, (*q)->getMetaData(),
                                             useWstring | TypeContextCpp11);

            map<string, StringList>::iterator r = paramComments.find((*q)->name());
            if(r != paramComments.end())
            {
                H << nl << "/** " << getDocSentence(r->second) << " */";
            }
            H << nl << typeString << " " << fixKwd((*q)->name()) << ";";
        }
        H << eb << ";";
    }
}

}

Slice::Gen::Gen(const string& base, const string& headerExtension, const string& sourceExtension,
                const vector<string>& extraHeaders, const string& include,
                const vector<string>& includePaths, const string& dllExport, const string& dir,
                bool implCpp98, bool implCpp11, bool checksum, bool ice) :
    _base(base),
    _headerExtension(headerExtension),
    _implHeaderExtension(headerExtension),
    _sourceExtension(sourceExtension),
    _extraHeaders(extraHeaders),
    _include(include),
    _includePaths(includePaths),
    _dllExport(dllExport),
    _dir(dir),
    _implCpp98(implCpp98),
    _implCpp11(implCpp11),
    _checksum(checksum),
    _ice(ice)
{
    for(vector<string>::iterator p = _includePaths.begin(); p != _includePaths.end(); ++p)
    {
        *p = fullPath(*p);
    }

    string::size_type pos = _base.find_last_of("/\\");
    if(pos != string::npos)
    {
        _base.erase(0, pos + 1);
    }
}

Slice::Gen::~Gen()
{
    H << "\n\n#include <IceUtil/PopDisableWarnings.h>";
    H << "\n#endif\n";
    C << '\n';

    if(_implCpp98 || _implCpp11)
    {
        implH << "\n\n#endif\n";
        implC << '\n';
    }
}

void
Slice::Gen::generateChecksumMap(const UnitPtr& p)
{
    if(_checksum)
    {
        ChecksumMap map = createChecksums(p);
        if(!map.empty())
        {
            C << sp << nl << "namespace";
            C << nl << "{";
            C << sp << nl << "const char* iceSliceChecksums[] =";
            C << sb;
            for(ChecksumMap::const_iterator q = map.begin(); q != map.end(); ++q)
            {
                C << nl << "\"" << q->first << "\", \"";
                ostringstream str;
                str.flags(ios_base::hex);
                str.fill('0');
                for(vector<unsigned char>::const_iterator r = q->second.begin(); r != q->second.end(); ++r)
                {
                    str << static_cast<int>(*r);
                }
                C << str.str() << "\",";
            }
            C << nl << "0";
            C << eb << ';';
            C << nl << "const IceInternal::SliceChecksumInit iceSliceChecksumInit(iceSliceChecksums);";
            C << sp << nl << "}";
        }
    }
}

void
Slice::Gen::generate(const UnitPtr& p)
{
    string file = p->topLevelFile();

    //
    // Give precedence to header-ext/source-ext global metadata.
    //
    string headerExtension = getHeaderExt(file, p);
    if(!headerExtension.empty())
    {
        _headerExtension = headerExtension;
    }

    string sourceExtension = getSourceExt(file, p);
    if(!sourceExtension.empty())
    {
        _sourceExtension = sourceExtension;
    }

    //
    // Give precedence to --dll-export command-line option
    //
    if(_dllExport.empty())
    {
        DefinitionContextPtr dc = p->findDefinitionContext(file);
        assert(dc);
        static const string dllExportPrefix = "cpp:dll-export:";
        string meta = dc->findMetaData(dllExportPrefix);
        if(meta.size() > dllExportPrefix.size())
        {
            _dllExport = meta.substr(dllExportPrefix.size());
        }
    }

    if(_implCpp98 || _implCpp11)
    {
        string fileImplH = _base + "I." + _implHeaderExtension;
        string fileImplC = _base + "I." + _sourceExtension;
        if(!_dir.empty())
        {
            fileImplH = _dir + '/' + fileImplH;
            fileImplC = _dir + '/' + fileImplC;
        }

        IceUtilInternal::structstat st;
        if(!IceUtilInternal::stat(fileImplH, &st))
        {
            ostringstream os;
            os << fileImplH << "' already exists - will not overwrite";
            throw FileException(__FILE__, __LINE__, os.str());
        }
        if(!IceUtilInternal::stat(fileImplC, &st))
        {
            ostringstream os;
            os << fileImplC << "' already exists - will not overwrite";
            throw FileException(__FILE__, __LINE__, os.str());
        }

        implH.open(fileImplH.c_str());
        if(!implH)
        {
            ostringstream os;
            os << "cannot open `" << fileImplH << "': " << IceUtilInternal::errorToString(errno);
            throw FileException(__FILE__, __LINE__, os.str());
        }
        FileTracker::instance()->addFile(fileImplH);

        implC.open(fileImplC.c_str());
        if(!implC)
        {
            ostringstream os;
            os << "cannot open `" << fileImplC << "': " << IceUtilInternal::errorToString(errno);
            throw FileException(__FILE__, __LINE__, os.str());
        }
        FileTracker::instance()->addFile(fileImplC);

        string s = _base + "I." + _implHeaderExtension;
        if(_include.size())
        {
            s = _include + '/' + s;
        }
        transform(s.begin(), s.end(), s.begin(), ToIfdef());
        implH << "#ifndef __" << s << "__";
        implH << "\n#define __" << s << "__";
        implH << '\n';
    }

    string fileH = _base + "." + _headerExtension;
    string fileC = _base + "." + _sourceExtension;
    if(!_dir.empty())
    {
        fileH = _dir + '/' + fileH;
        fileC = _dir + '/' + fileC;
    }

    H.open(fileH.c_str());
    if(!H)
    {
        ostringstream os;
        os << "cannot open `" << fileH << "': " << IceUtilInternal::errorToString(errno);
        throw FileException(__FILE__, __LINE__, os.str());
    }
    FileTracker::instance()->addFile(fileH);

    C.open(fileC.c_str());
    if(!C)
    {
        ostringstream os;
        os << "cannot open `" << fileC << "': " << IceUtilInternal::errorToString(errno);
        throw FileException(__FILE__, __LINE__, os.str());
    }
    FileTracker::instance()->addFile(fileC);

    printHeader(H);
    printGeneratedHeader(H, _base + ".ice");
    printHeader(C);
    printGeneratedHeader(C, _base + ".ice");

    string s = _base + "." + _headerExtension;
    if(_include.size())
    {
        s = _include + '/' + s;
    }
    transform(s.begin(), s.end(), s.begin(), ToIfdef());
    H << "\n#ifndef __" << s << "__";
    H << "\n#define __" << s << "__";
    H << '\n';

    validateMetaData(p);

    writeExtraHeaders(C);

    if(_dllExport.size())
    {
        C << "\n#ifndef " << _dllExport << "_EXPORTS";
        C << "\n#   define " << _dllExport << "_EXPORTS";
        C << "\n#endif";
    }

    C << "\n#include <";
    if(_include.size())
    {
        C << _include << '/';
    }
    C << _base << "." << _headerExtension << ">";
    C <<  "\n#include <IceUtil/PushDisableWarnings.h>";

    H << "\n#include <IceUtil/PushDisableWarnings.h>";
    H << "\n#include <Ice/ProxyF.h>";
    H << "\n#include <Ice/ObjectF.h>";
    H << "\n#include <Ice/ValueF.h>";
    H << "\n#include <Ice/Exception.h>";
    H << "\n#include <Ice/LocalObject.h>";
    H << "\n#include <Ice/StreamHelpers.h>";
    H << "\n#include <Ice/Comparable.h>";

    if(p->hasNonLocalClassDefs())
    {
        H << "\n#include <Ice/Proxy.h>";
        H << "\n#include <Ice/Object.h>";
        H << "\n#include <Ice/GCObject.h>";
        H << "\n#include <Ice/Value.h>";
        H << "\n#include <Ice/Incoming.h>";
        if(p->hasContentsWithMetaData("amd"))
        {
            H << "\n#include <Ice/IncomingAsync.h>";
        }
        C << "\n#include <Ice/LocalException.h>";
        C << "\n#include <Ice/ValueFactory.h>";
        C << "\n#include <Ice/OutgoingAsync.h>";
    }
    else if(p->hasLocalClassDefsWithAsync())
    {
        H << "\n#include <Ice/OutgoingAsync.h>";
    }
    else if(p->hasNonLocalClassDecls())
    {
        H << "\n#include <Ice/Proxy.h>";
    }

    if(p->hasNonLocalClassDefs() || p->hasNonLocalExceptions())
    {
        H << "\n#include <Ice/FactoryTableInit.h>";
    }

    H << "\n#include <IceUtil/ScopedArray.h>";
    H << "\n#include <Ice/Optional.h>";

    if(p->hasExceptions())
    {
        H << "\n#include <Ice/ExceptionHelpers.h>";
    }

    if(p->usesNonLocals())
    {
        C << "\n#include <Ice/InputStream.h>";
        C << "\n#include <Ice/OutputStream.h>";
    }

    if(p->hasNonLocalExceptions())
    {
        C << "\n#include <Ice/LocalException.h>";
    }

    if(p->hasContentsWithMetaData("preserve-slice"))
    {
        H << "\n#include <Ice/SlicedDataF.h>";
        C << "\n#include <Ice/SlicedData.h>";
    }

    if(_checksum)
    {
        C << "\n#include <Ice/SliceChecksums.h>";
    }

    C << "\n#include <IceUtil/PopDisableWarnings.h>";

    StringList includes = p->includeFiles();

    for(StringList::const_iterator q = includes.begin(); q != includes.end(); ++q)
    {
        string extension = getHeaderExt((*q), p);
        if(extension.empty())
        {
            extension = _headerExtension;
        }
        H << "\n#include <" << changeInclude(*q, _includePaths) << "." << extension << ">";
    }

    H << "\n#include <IceUtil/UndefSysMacros.h>";

    //
    // Emit #include statements for any cpp:include metadata directives
    // in the top-level Slice file.
    //
    {
        DefinitionContextPtr dc = p->findDefinitionContext(file);
        assert(dc);
        StringList globalMetaData = dc->getMetaData();
        for(StringList::const_iterator q = globalMetaData.begin(); q != globalMetaData.end();)
        {
            string md = *q++;
            static const string includePrefix = "cpp:include:";
            if(md.find(includePrefix) == 0)
            {
                if(md.size() > includePrefix.size())
                {
                    H << nl << "#include <" << md.substr(includePrefix.size()) << ">";
                }
                else
                {
                    ostringstream ostr;
                    ostr << "ignoring invalid global metadata `" << md << "'";
                    dc->warning(InvalidMetaData, file, -1, ostr.str());
                    globalMetaData.remove(md);
                }
            }
        }
        dc->setMetaData(globalMetaData);
    }

    //
    // Disable shadow warnings in .cpp file
    //
    C << sp;
    C.zeroIndent();
    C << nl << "#if defined(_MSC_VER)";
    C << nl << "#   pragma warning(disable:4458) // declaration of ... hides class member";
    C << nl << "#elif defined(__clang__)";
    C << nl << "#   pragma clang diagnostic ignored \"-Wshadow\"";
    C << nl << "#elif defined(__GNUC__)";
    C << nl << "#   pragma GCC diagnostic ignored \"-Wshadow\"";
    C << nl << "#endif";

    printVersionCheck(H);
    printVersionCheck(C);

    printDllExportStuff(H, _dllExport);
    if(_dllExport.size())
    {
        _dllExport += " ";
    }

    H << sp;
    H.zeroIndent();
    H << nl << "#ifdef ICE_CPP11_MAPPING // C++11 mapping";
    H.restoreIndent();

    C << sp;
    C.zeroIndent();
    C << nl << "#ifdef ICE_CPP11_MAPPING // C++11 mapping";
    C.restoreIndent();
    {
        normalizeMetaData(p, true);

        Cpp11DeclVisitor declVisitor(H, C, _dllExport);
        p->visit(&declVisitor, false);

        Cpp11TypesVisitor typesVisitor(H, C, _dllExport);
        p->visit(&typesVisitor, false);

        Cpp11LocalObjectVisitor localObjectVisitor(H, C, _dllExport);
        p->visit(&localObjectVisitor, false);

        Cpp11InterfaceVisitor interfaceVisitor(H, C, _dllExport);
        p->visit(&interfaceVisitor, false);

        Cpp11ValueVisitor valueVisitor(H, C, _dllExport);
        p->visit(&valueVisitor, false);

        Cpp11ProxyVisitor proxyVisitor(H, C, _dllExport);
        p->visit(&proxyVisitor, false);

        Cpp11StreamVisitor streamVisitor(H, C, _dllExport);
        p->visit(&streamVisitor, false);

        if(_implCpp11)
        {
            implH << "\n#include <";
            if(_include.size())
            {
                implH << _include << '/';
            }
            implH << _base << "." << _headerExtension << ">";
            writeExtraHeaders(implC);

            implC << "\n#include <";
            if(_include.size())
            {
                implC << _include << '/';
            }
            implC << _base << "I." << _implHeaderExtension << ">";

            Cpp11ImplVisitor implVisitor(implH, implC, _dllExport);
            p->visit(&implVisitor, false);
        }

        Cpp11CompatibilityVisitor compatibilityVisitor(H, C, _dllExport);
        p->visit(&compatibilityVisitor, false);

        generateChecksumMap(p);
    }
    H << sp;
    H.zeroIndent();
    H << nl << "#else // C++98 mapping";
    H.restoreIndent();

    C << sp;
    C.zeroIndent();
    C << nl << "#else // C++98 mapping";
    C.restoreIndent();
    {
        normalizeMetaData(p, false);

        ProxyDeclVisitor proxyDeclVisitor(H, C, _dllExport);
        p->visit(&proxyDeclVisitor, false);

        ObjectDeclVisitor objectDeclVisitor(H, C, _dllExport);
        p->visit(&objectDeclVisitor, false);

        TypesVisitor typesVisitor(H, C, _dllExport);
        p->visit(&typesVisitor, false);

        AsyncVisitor asyncVisitor(H, C, _dllExport);
        p->visit(&asyncVisitor, false);

        AsyncImplVisitor asyncImplVisitor(H, C, _dllExport);
        p->visit(&asyncImplVisitor, false);

        //
        // The templates are emitted before the proxy definition
        // so the derivation hierarchy is known to the proxy:
        // the proxy relies on knowing the hierarchy to make the begin_
        // methods type-safe.
        //
        AsyncCallbackVisitor asyncCallbackVisitor(H, C, _dllExport);
        p->visit(&asyncCallbackVisitor, false);

        ProxyVisitor proxyVisitor(H, C, _dllExport);
        p->visit(&proxyVisitor, false);

        ObjectVisitor objectVisitor(H, C, _dllExport);
        p->visit(&objectVisitor, false);

        StreamVisitor streamVisitor(H, C, _dllExport);
        p->visit(&streamVisitor, false);

        //
        // We need to delay generating the template after the proxy
        // definition, because completed calls the begin_ method in the
        // proxy.
        //
        AsyncCallbackTemplateVisitor asyncCallbackTemplateVisitor(H, C, _dllExport);
        p->visit(&asyncCallbackTemplateVisitor, false);

        if(_implCpp98)
        {
            implH << "\n#include <";
            if(_include.size())
            {
                implH << _include << '/';
            }
            implH << _base << "." << _headerExtension << ">";

            writeExtraHeaders(implC);

            implC << "\n#include <";
            if(_include.size())
            {
                implC << _include << '/';
            }
            implC << _base << "I." << _implHeaderExtension << ">";

            ImplVisitor implVisitor(implH, implC, _dllExport);
            p->visit(&implVisitor, false);
        }

        generateChecksumMap(p);
    }

    H << sp;
    H.zeroIndent();
    H << nl << "#endif";
    H.restoreIndent();

    C << sp;
    C.zeroIndent();
    C << nl << "#endif";
    C.restoreIndent();
}

void
Slice::Gen::closeOutput()
{
    H.close();
    C.close();
    implH.close();
    implC.close();
}

void
Slice::Gen::writeExtraHeaders(IceUtilInternal::Output& out)
{
    for(vector<string>::const_iterator i = _extraHeaders.begin(); i != _extraHeaders.end(); ++i)
    {
        string hdr = *i;
        string guard;
        string::size_type pos = hdr.rfind(',');
        if(pos != string::npos)
        {
            hdr = i->substr(0, pos);
            guard = i->substr(pos + 1);
        }
        if(!guard.empty())
        {
            out << "\n#ifndef " << guard;
            out << "\n#define " << guard;
        }
        out << "\n#include <";
        out << hdr << '>';
        if(!guard.empty())
        {
            out << "\n#endif";
        }
    }
}

Slice::Gen::TypesVisitor::TypesVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport), _doneStaticSymbol(false), _useWstring(false)
{
}

bool
Slice::Gen::TypesVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasOtherConstructedOrExceptions())
    {
        return false;
    }

    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    H << sp << nl << "namespace " << fixKwd(p->name()) << nl << '{';

    return true;
}

void
Slice::Gen::TypesVisitor::visitModuleEnd(const ModulePtr&)
{
    H << sp << nl << '}';

    _useWstring = resetUseWstring(_useWstringHist);
}

bool
Slice::Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr&)
{
    return false;
}

bool
Slice::Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    string name = fixKwd(p->name());
    string scope = fixKwd(p->scope());
    string scoped = fixKwd(p->scoped());
    ExceptionPtr base = p->base();
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList allDataMembers = p->allDataMembers();
    bool hasDefaultValues = p->hasDefaultValues();

    vector<string> allParamDecls;
    vector<string> baseParams;
    map<string, CommentPtr> allComments;

    string fileParam = "file";
    string lineParam = "line";

    for(DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
    {
        string typeName = inputTypeToString((*q)->type(), (*q)->optional(), scope, (*q)->getMetaData(), _useWstring);
        allParamDecls.push_back(typeName + " " + fixKwd((*q)->name()));
        CommentPtr comment = (*q)->parseComment(false);
        if(comment)
        {
            allComments[(*q)->name()] = comment;
        }

        if((*q)->name() == "file")
        {
            fileParam = "file_";
        }
        else if((*q)->name() == "line")
        {
            fileParam = "line_";
        }
    }

    if(base)
    {
        DataMemberList baseDataMembers = base->allDataMembers();
        for(DataMemberList::const_iterator q = baseDataMembers.begin(); q != baseDataMembers.end(); ++q)
        {
            baseParams.push_back(fixKwd((*q)->name()));
        }
    }

    H << sp;
    writeDocSummary(H, p);
    H << nl << "class " << _dllExport << name << " : ";
    H.useCurrentPosAsIndent();
    H << "public ";
    if(base)
    {
        H << getUnqualified(fixKwd(base->scoped()), scope);
    }
    else
    {
        H << getUnqualified(p->isLocal() ? "::Ice::LocalException" : "::Ice::UserException", scope);
    }
    H.restoreIndent();
    H << sb;

    H.dec();
    H << nl << "public:";
    H.inc();

    H << sp;
    if(p->isLocal())
    {
        H << nl << "/**";
        H << nl << " * The file and line number are required for all local exceptions.";
        H << nl << " * @param " << fileParam
          << " The file name in which the exception was raised, typically __FILE__.";
        H << nl << " * @param " << lineParam
          << " The line number at which the exception was raised, typically __LINE__.";
        H << nl << " */";
    }
    else if(hasDefaultValues)
    {
        H << nl << "/** Default constructor that assigns default values to members as specified in the "
          "Slice definition. */";
    }

    H << nl << name << spar;
    if(p->isLocal())
    {
        H << "const char* " + fileParam << "int " + lineParam;
    }
    H << epar;
    if(!p->isLocal() && !hasDefaultValues)
    {
        H << " {}";
    }
    else
    {
        H << ';';
    }
    if(!allParamDecls.empty())
    {
        H << nl << "/**";
        H << nl << " * One-shot constructor to initialize all data members.";
        if(p->isLocal())
        {
            H << nl << " * The file and line number are required for all local exceptions.";
            H << nl << " * @param " << fileParam
              << " The file name in which the exception was raised, typically __FILE__.";
            H << nl << " * @param " << lineParam
              << " The line number at which the exception was raised, typically __LINE__.";
        }
        for(DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
        {
            map<string, CommentPtr>::iterator r = allComments.find((*q)->name());
            if(r != allComments.end())
            {
                H << nl << " * @param " << fixKwd(r->first) << " " << getDocSentence(r->second->overview());
            }
        }
        H << nl << " */";
        H << nl;
        if(!p->isLocal() && allParamDecls.size() == 1)
        {
            H << "explicit ";
        }
        H << name << spar;
        if(p->isLocal())
        {
            H << "const char* " + fileParam << "int " + lineParam;
        }
        H << allParamDecls << epar << ';';
    }
    H << nl << "virtual ~" << name << "() throw();";
    H << sp;

    if(!p->isLocal())
    {
        string initName = "iceC" + p->flattenedScope() + p->name() + "_init";

        C << sp << nl << "namespace";
        C << nl << "{";

        C << sp << nl << "const ::IceInternal::DefaultUserExceptionFactoryInit< " << scoped << "> "
          << initName << "(\"" << p->scoped() << "\");";

        C << sp << nl << "}";
    }

    if(p->isLocal())
    {
        C << sp << nl << scoped.substr(2) << "::" << name << spar << "const char* " + fileParam
          << "int " + lineParam << epar << " :";
        C.inc();
        emitUpcall(base, "(" + fileParam + ", " + lineParam + ")", scope, true);
        if(p->hasDefaultValues())
        {
            C << ",";
            writeDataMemberInitializers(C, dataMembers, _useWstring);
        }
        C.dec();
        C << sb;
        C << eb;
    }
    else if(hasDefaultValues)
    {
        C << sp << nl << scoped.substr(2) << "::" << name << "() :";
        C.inc();
        writeDataMemberInitializers(C, dataMembers, _useWstring);
        C.dec();
        C << sb;
        C << eb;
    }

    if(!allParamDecls.empty())
    {
        C << sp << nl;
        C << scoped.substr(2) << "::" << name << spar;
        if(p->isLocal())
        {
            C << "const char* " + fileParam << "int " + lineParam;
        }
        C << allParamDecls << epar;
        if(p->isLocal() || !baseParams.empty() || !dataMembers.empty())
        {
            C << " :";
            C.inc();
            string upcall;
            if(!allParamDecls.empty())
            {
                upcall = "(";
                if(p->isLocal())
                {
                    upcall += fileParam + ", " + lineParam;
                }
                for(vector<string>::const_iterator pi = baseParams.begin(); pi != baseParams.end(); ++pi)
                {
                    if(p->isLocal() || pi != baseParams.begin())
                    {
                        upcall += ", ";
                    }
                    upcall += *pi;
                }
                upcall += ")";
            }
            if(!dataMembers.empty())
            {
                upcall += ",";
            }
            emitUpcall(base, upcall, scope, p->isLocal());
        }
        for(DataMemberList::const_iterator d = dataMembers.begin();  d != dataMembers.end(); ++d)
        {
            if(d != dataMembers.begin())
            {
                C << ",";
            }
            string memberName = fixKwd((*d)->name());
            C << nl << memberName << "(" << memberName << ")";
        }
        if(p->isLocal() || !baseParams.empty() || !dataMembers.empty())
        {
            C.dec();
        }
        C << sb;
        C << eb;
    }

    C << sp << nl;
    C << scoped.substr(2) << "::~" << name << "() throw()";
    C << sb;
    C << eb;

    H << nl << "/**";
    H << nl << " * Obtains the Slice type ID of this exception.";
    H << nl << " * @return The fully-scoped type ID.";
    H << nl << " */";
    H << nl << "virtual ::std::string ice_id() const;";
    C << sp << nl << "::std::string" << nl << scoped.substr(2) << "::ice_id() const";
    C << sb;
    C << nl << "return \"" << p->scoped() << "\";";
    C << eb;

    StringList metaData = p->getMetaData();
    if(find(metaData.begin(), metaData.end(), "cpp:ice_print") != metaData.end())
    {
        H << nl << "/**";
        H << nl << " * Prints this exception to the given stream.";
        H << nl << " * @param stream The target stream.";
        H << nl << " */";
        H << nl << "virtual void ice_print(::std::ostream& stream) const;";
    }

    H << nl << "/**";
    H << nl << " * Polymporphically clones this exception.";
    H << nl << " * @return A shallow copy of this exception.";
    H << nl << " */";
    H << nl << "virtual " << name << "* ice_clone() const;";
    C << sp << nl << scoped.substr(2) << "*" << nl << scoped.substr(2) << "::ice_clone() const";
    C << sb;
    C << nl << "return new " << name << "(*this);";
    C << eb;

    H << nl << "/**";
    H << nl << " * Throws this exception.";
    H << nl << " */";
    H << nl << "virtual void ice_throw() const;";
    C << sp << nl << "void" << nl << scoped.substr(2) << "::ice_throw() const";
    C << sb;
    C << nl << "throw *this;";
    C << eb;

    if(!p->isLocal() && p->usesClasses(false))
    {
        if(!base || (base && !base->usesClasses(false)))
        {
            H << sp;
            H << nl << "/// \\cond STREAM";
            H << nl << "virtual bool _usesClasses() const;";
            H << nl << "/// \\endcond";

            C << sp << nl << "bool";
            C << nl << scoped.substr(2) << "::_usesClasses() const";
            C << sb;
            C << nl << "return true;";
            C << eb;
        }
    }

    if(!dataMembers.empty())
    {
        H << sp;
    }
    return true;
}

void
Slice::Gen::TypesVisitor::visitExceptionEnd(const ExceptionPtr& p)
{
    string name = fixKwd(p->name());
    string scope = fixKwd(p->scope());
    string scoped = fixKwd(p->scoped());
    string factoryName;

    if(!p->isLocal())
    {
        ExceptionPtr base = p->base();
        bool basePreserved = p->inheritsMetaData("preserve-slice");
        bool preserved = p->hasMetaData("preserve-slice");

        if(preserved && !basePreserved)
        {
            H << sp;
            H << nl << "/**";
            H << nl << " * Obtains the SlicedData object created when an unknown exception type was marshaled";
            H << nl << " * in the sliced format and the Ice run time sliced it to a known type.";
            H << nl << " * @return The SlicedData object, or nil if the exception was not sliced or was not";
            H << nl << " * marshaled in the sliced format.";
            H << nl << " */";
            H << nl << "virtual ::Ice::SlicedDataPtr ice_getSlicedData() const;";

            H << sp;
            H << nl << "/// \\cond STREAM";
            H << nl << "virtual void _write(::Ice::OutputStream*) const;";
            H << nl << "virtual void _read(::Ice::InputStream*);";

            string baseName = base ? fixKwd(base->scoped()) : string("::Ice::UserException");
            H << nl << "using " << baseName << "::_write;";
            H << nl << "using " << baseName << "::_read;";
            H << nl << "/// \\endcond";
        }

        H.dec();
        H << sp << nl << "protected:";
        H.inc();

        H << sp;
        H << nl << "/// \\cond STREAM";
        H << nl << "virtual void _writeImpl(" << getUnqualified("::Ice::OutputStream*", scope) << ") const;";
        H << nl << "virtual void _readImpl(" << getUnqualified("::Ice::InputStream*", scope) << ");";
        H << nl << "/// \\endcond";

        string baseName = getUnqualified(base ? fixKwd(base->scoped()) : "::Ice::UserException", scope);

        if(preserved && !basePreserved)
        {
            H << sp;
            H << nl << "/// \\cond STREAM";
            H << nl << "::Ice::SlicedDataPtr _slicedData;";
            H << nl << "/// \\endcond";

            C << sp;
            C << nl << "::Ice::SlicedDataPtr" << nl << scoped.substr(2) << "::ice_getSlicedData() const";
            C << sb;
            C << nl << "return _slicedData;";
            C << eb;

            C << sp << nl << "void" << nl << scoped.substr(2) << "::_write("
              << getUnqualified("::Ice::OutputStream*", scope) << " ostr) const";
            C << sb;
            C << nl << "ostr->startException(_slicedData);";
            C << nl << "_writeImpl(ostr);";
            C << nl << "ostr->endException();";
            C << eb;

            C << sp << nl << "void" << nl << scoped.substr(2) << "::_read("
              << getUnqualified("::Ice::InputStream*", scope) << " istr)";
            C << sb;
            C << nl << "istr->startException();";
            C << nl << "_readImpl(istr);";
            C << nl << "_slicedData = istr->endException(true);";
            C << eb;
        }

        C << sp;
        C << nl << "/// \\cond STREAM";
        C << nl << "void" << nl << scoped.substr(2) << "::_writeImpl("
          << getUnqualified("::Ice::OutputStream*", scope) << " ostr) const";
        C << sb;
        C << nl << "ostr->startSlice(\"" << p->scoped() << "\", -1, " << (!base ? "true" : "false") << ");";
        C << nl << getUnqualified("::Ice::StreamWriter", scope) << "< " << name << ", "
          << getUnqualified("::Ice::OutputStream", scope) << ">::write(ostr, *this);";
        C << nl << "ostr->endSlice();";
        if(base)
        {
            emitUpcall(base, "::_writeImpl(ostr);", scope);
        }
        C << eb;

        C << sp << nl << "void" << nl << scoped.substr(2) << "::_readImpl("
          << getUnqualified("::Ice::InputStream*", scope) << " istr)";
        C << sb;
        C << nl << "istr->startSlice();";
        C << nl << getUnqualified("::Ice::StreamReader", scope) << "< " << name << ", "
          << getUnqualified("::Ice::InputStream", scope) << ">::read(istr, *this);";
        C << nl << "istr->endSlice();";
        if(base)
        {
            emitUpcall(base, "::_readImpl(istr);", scope);
        }
        C << eb;
        C << nl << "/// \\endcond";
    }
    H << eb << ';';

    if(!p->isLocal())
    {
        //
        // We need an instance here to trigger initialization if the implementation is in a static library.
        // But we do this only once per source file, because a single instance is sufficient to initialize
        // all of the globals in a compilation unit.
        //
        if(!_doneStaticSymbol)
        {
            _doneStaticSymbol = true;
            H << sp;
            H << nl << "/// \\cond INTERNAL";
            H << nl << "static " << name << " _iceS_" << p->name() << "_init;";
            H << nl << "/// \\endcond";
        }
    }

    _useWstring = resetUseWstring(_useWstringHist);
}

bool
Slice::Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    DataMemberList dataMembers = p->dataMembers();
    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);
    string scope = fixKwd(p->scope());
    string name = fixKwd(p->name());

    H << sp;
    writeDocSummary(H, p);

    bool classMetaData = findMetaData(p->getMetaData()) == "%class";
    if(classMetaData)
    {
        H << nl << "class " << name << " : public IceUtil::Shared";
        H << sb;
        H.dec();
        H << nl << "public:";
        H.inc();
        H << nl;
        if(p->hasDefaultValues())
        {
            H << nl << "/** Default constructor that assigns default values to members as specified in the "
              "Slice definition. */";
            H << nl << name << "() :";
            H.inc();
            writeDataMemberInitializers(H, dataMembers, _useWstring);
            H.dec();
            H << sb;
            H << eb;
        }
        else
        {
            H << nl << name << "() {}";
        }
    }
    else
    {
        H << nl << "struct " << name;
        H << sb;
        if(p->hasDefaultValues())
        {
            H << nl << "/** Default constructor that assigns default values to members as specified in the "
              "Slice definition. */";
            H << nl << name << "() :";
            H.inc();
            writeDataMemberInitializers(H, dataMembers, _useWstring);
            H.dec();
            H << sb;
            H << eb << nl;
        }
    }

    //
    // Generate a one-shot constructor if the struct uses the class mapping, or if at least
    // one of its members has a default value.
    //
    if(!dataMembers.empty() && (findMetaData(p->getMetaData()) == "%class" || p->hasDefaultValues()))
    {
        vector<string> paramDecls;
        map<string, CommentPtr> comments;
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            string typeName =
                inputTypeToString((*q)->type(), (*q)->optional(), scope, (*q)->getMetaData(), _useWstring);
            paramDecls.push_back(typeName + " " + fixKwd((*q)->name()));
            CommentPtr comment = (*q)->parseComment(false);
            if(comment && !comment->overview().empty())
            {
                comments[(*q)->name()] = comment;
            }
        }

        if(!comments.empty())
        {
            H << nl << "/**";
            H << nl << " * One-shot constructor to initialize all data members.";
            for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
            {
                map<string, CommentPtr>::iterator r = comments.find((*q)->name());
                if(r != comments.end())
                {
                    H << nl << " * @param " << fixKwd(r->first) << " " << getDocSentence(r->second->overview());
                }
            }
            H << nl << " */";
        }

        H << nl;
        if(paramDecls.size() == 1)
        {
            H << "explicit ";
        }
        H << fixKwd(p->name()) << spar << paramDecls << epar << " :";
        H.inc();

        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            if(q != dataMembers.begin())
            {
                H << ',';
            }
            string memberName = fixKwd((*q)->name());
            H << nl << memberName << '(' << memberName << ')';
        }

        H.dec();
        H << sb;
        H << eb;
        H << nl;
    }

    H << sp;

    return true;
}

void
Slice::Gen::TypesVisitor::visitStructEnd(const StructPtr& p)
{
    string name = fixKwd(p->name());
    string scoped = fixKwd(p->scoped());
    string scope = fixKwd(p->scope());

    DataMemberList dataMembers = p->dataMembers();

    vector<string> params;

    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        params.push_back(fixKwd((*q)->name()));
    }

    bool containsSequence = false;
    if((Dictionary::legalKeyType(p, containsSequence) && !containsSequence) || p->hasMetaData("cpp:comparable"))
    {
        H << sp << nl << "bool operator==(const " << name << "& rhs_) const";
        H << sb;
        H << nl << "if(this == &rhs_)";
        H << sb;
        H << nl << "return true;";
        H << eb;
        for(vector<string>::const_iterator pi = params.begin(); pi != params.end(); ++pi)
        {
            H << nl << "if(" << *pi << " != rhs_." << *pi << ')';
            H << sb;
            H << nl << "return false;";
            H << eb;
        }
        H << nl << "return true;";
        H << eb;
        H << sp << nl << "bool operator<(const " << name << "& rhs_) const";
        H << sb;
        H << nl << "if(this == &rhs_)";
        H << sb;
        H << nl << "return false;";
        H << eb;
        for(vector<string>::const_iterator pi = params.begin(); pi != params.end(); ++pi)
        {
            H << nl << "if(" << *pi << " < rhs_." << *pi << ')';
            H << sb;
            H << nl << "return true;";
            H << eb;
            H << nl << "else if(rhs_." << *pi << " < " << *pi << ')';
            H << sb;
            H << nl << "return false;";
            H << eb;
        }
        H << nl << "return false;";
        H << eb;

        H << sp << nl << "bool operator!=(const " << name << "& rhs_) const";
        H << sb;
        H << nl << "return !operator==(rhs_);";
        H << eb;
        H << nl << "bool operator<=(const " << name << "& rhs_) const";
        H << sb;
        H << nl << "return operator<(rhs_) || operator==(rhs_);";
        H << eb;
        H << nl << "bool operator>(const " << name << "& rhs_) const";
        H << sb;
        H << nl << "return !operator<(rhs_) && !operator==(rhs_);";
        H << eb;
        H << nl << "bool operator>=(const " << name << "& rhs_) const";
        H << sb;
        H << nl << "return !operator<(rhs_);";
        H << eb;
    }
    H << eb << ';';

    if(findMetaData(p->getMetaData()) == "%class")
    {
        H << sp << nl << "typedef ::IceUtil::Handle< " << scoped << "> " << p->name() + "Ptr;";
    }

    _useWstring = resetUseWstring(_useWstringHist);
}

void
Slice::Gen::TypesVisitor::visitDataMember(const DataMemberPtr& p)
{
    ContainerPtr container = p->container();
    string name = fixKwd(p->name());

    string scope = "";
    StructPtr st = StructPtr::dynamicCast(container);
    if(st)
    {
        scope = fixKwd(st->scope());
    }

    ExceptionPtr ex = ExceptionPtr::dynamicCast(container);
    if(ex)
    {
        scope = fixKwd(ex->scope());
    }

    writeDocSummary(H, p);
    H << nl << typeToString(p->type(), p->optional(), scope, p->getMetaData(), _useWstring) << ' ' << name << ';';
}

void
Slice::Gen::TypesVisitor::visitSequence(const SequencePtr& p)
{
    string name = fixKwd(p->name());
    TypePtr type = p->type();
    ContainedPtr cont = ContainedPtr::dynamicCast(p->container());
    string scope = fixKwd(p->scope());
    string s = typeToString(type, scope, p->typeMetaData(), _useWstring);
    StringList metaData = p->getMetaData();

    string seqType = findMetaData(metaData, _useWstring);
    H << sp;

    writeDocSummary(H, p);

    if(!seqType.empty())
    {
        H << nl << "typedef " << seqType << ' ' << name << ';';
    }
    else
    {
        H << nl << "typedef ::std::vector<" << (s[0] == ':' ? " " : "") << s << "> " << name << ';';
    }
}

void
Slice::Gen::TypesVisitor::visitDictionary(const DictionaryPtr& p)
{
    string name = fixKwd(p->name());
    ContainedPtr cont = ContainedPtr::dynamicCast(p->container());
    string scope = fixKwd(p->scope());
    string dictType = findMetaData(p->getMetaData());

    H << sp;
    writeDocSummary(H, p);

    if(dictType.empty())
    {
        //
        // A default std::map dictionary
        //

        TypePtr keyType = p->keyType();
        TypePtr valueType = p->valueType();
        string ks = typeToString(keyType, scope, p->keyMetaData(), _useWstring);
        if(ks[0] == ':')
        {
            ks.insert(0, " ");
        }
        string vs = typeToString(valueType, scope, p->valueMetaData(), _useWstring);

        H << nl << "typedef ::std::map<" << ks << ", " << vs << "> " << name << ';';
    }
    else
    {
        //
        // A custom dictionary
        //
        H << nl << "typedef " << dictType << ' ' << name << ';';
    }
}

void
Slice::Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    string name = fixKwd(p->name());
    EnumeratorList enumerators = p->enumerators();

    string enumeratorPrefix = findMetaData(p->getMetaData()) == "%scoped" ? name : "";

    //
    // Check if any of the enumerators were assigned an explicit value.
    //
    const bool explicitValue = p->explicitValue();

    H << sp;
    writeDocSummary(H, p);

    H << nl << "enum " << name;
    H << sb;

    EnumeratorList::const_iterator en = enumerators.begin();
    while(en != enumerators.end())
    {
        writeDocSummary(H, *en);
        H << nl << fixKwd(enumeratorPrefix + (*en)->name());
        //
        // If any of the enumerators were assigned an explicit value, we emit
        // an explicit value for *all* enumerators.
        //
        if(explicitValue)
        {
            H << " = " << int64ToString((*en)->value());
        }
        if(++en != enumerators.end())
        {
            H << ',';
        }
    }
    H << eb << ';';
}

void
Slice::Gen::TypesVisitor::visitConst(const ConstPtr& p)
{
    string scope = fixKwd(p->scope());
    H << sp;
    writeDocSummary(H, p);
    H << nl << "const " << typeToString(p->type(), scope, p->typeMetaData(), _useWstring) << " " << fixKwd(p->name())
      << " = ";
    writeConstantValue(H, p->type(), p->valueType(), p->value(), _useWstring, p->typeMetaData(), scope);
    H << ';';
}

void
Slice::Gen::TypesVisitor::emitUpcall(const ExceptionPtr& base, const string& call, const string& scope, bool isLocal)
{
    C << nl;
    if(base)
    {
        C << getUnqualified(fixKwd(base->scoped()), scope);
    }
    else
    {
        C << getUnqualified(isLocal ? "::Ice::LocalException" : "::Ice::UserException", scope);
    }
    C << call;
}

Slice::Gen::ProxyDeclVisitor::ProxyDeclVisitor(Output& h, Output&, const string& dllExport) :
    H(h), _dllExport(dllExport)
{
}

bool
Slice::Gen::ProxyDeclVisitor::visitUnitStart(const UnitPtr& p)
{
    if(!p->hasNonLocalClassDecls())
    {
        return false;
    }

    H << sp << nl << "namespace IceProxy" << nl << '{';

    return true;
}

void
Slice::Gen::ProxyDeclVisitor::visitUnitEnd(const UnitPtr&)
{
    H << sp << nl << '}';
}

bool
Slice::Gen::ProxyDeclVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalClassDecls())
    {
        return false;
    }

    string name = fixKwd(p->name());

    H << sp << nl << "namespace " << name << nl << '{';

    return true;
}

void
Slice::Gen::ProxyDeclVisitor::visitModuleEnd(const ModulePtr&)
{
    H << sp << nl << '}';
}

void
Slice::Gen::ProxyDeclVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    if(p->isLocal())
    {
        return;
    }

    string name = fixKwd(p->name());

    H << sp << nl << "class " << name << ';';
    //
    // Underscore prefix because it's a private function that should not clash with
    // an interface named 'readProxy'
    // Note that _readProxy is always in the IceProxy::... namespace
    //
    H << nl << "/// \\cond INTERNAL";
    H << nl << _dllExport << "void _readProxy(::Ice::InputStream*, ::IceInternal::ProxyHandle< " << name << ">&);";
    H << nl << _dllExport << "::IceProxy::Ice::Object* upCast(" << name << "*);";
    H << nl << "/// \\endcond";
}

Slice::Gen::ProxyVisitor::ProxyVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport), _dllClassExport(toDllClassExport(dllExport)),
    _dllMemberExport(toDllMemberExport(dllExport)), _useWstring(false)
{
}

bool
Slice::Gen::ProxyVisitor::visitUnitStart(const UnitPtr& p)
{
    if(!p->hasNonLocalClassDefs())
    {
        return false;
    }

    H << sp << nl << "namespace IceProxy" << nl << '{';

    return true;
}

void
Slice::Gen::ProxyVisitor::visitUnitEnd(const UnitPtr&)
{
    H << sp << nl << '}';
}

bool
Slice::Gen::ProxyVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalClassDefs())
    {
        return false;
    }

    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    string name = fixKwd(p->name());

    H << sp << nl << "namespace " << name << nl << '{';

    return true;
}

void
Slice::Gen::ProxyVisitor::visitModuleEnd(const ModulePtr&)
{
    H << sp << nl << '}';

    _useWstring = resetUseWstring(_useWstringHist);
}

bool
Slice::Gen::ProxyVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal())
    {
        return false;
    }

    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    string name = fixKwd(p->name());
    string scope = fixKwd(p->scope());
    string scoped = fixKwd(p->scoped());
    ClassList bases = p->bases();

    if(bases.size() > 1)
    {
        //
        // Generated helper class to deal with multiple inheritance
        // when using Proxy template.
        //

        string baseName = fixKwd("_" + p->name() + "Base");

        H << sp;
        H << nl << "/// \\cond INTERNAL";
        H << nl << "class " << _dllClassExport << baseName << " : ";
        H.useCurrentPosAsIndent();
        for(ClassList::const_iterator q = bases.begin(); q != bases.end();)
        {
            H << "public virtual ::IceProxy" << fixKwd((*q)->scoped());
            if(++q != bases.end())
            {
                H << ", " << nl;
            }
        }
        H.restoreIndent();
        H << sb;

        H.dec();
        H << nl << "public:";
        H.inc();

        // Out of line dtor to avoid weak vtable
        H << sp << nl << _dllMemberExport << "virtual ~" << baseName << "();";
        C << sp;
        C << nl << "::IceProxy" << scope << baseName << "::~" << baseName << "()";
        C << sb;
        C << eb;

        H.dec();
        H << sp << nl << "protected:";
        H.inc();

        H << sp << nl << "virtual Object* _newInstance() const = 0;";
        H << eb << ';';
        H << nl << "/// \\endcond";
    }

    H << sp << nl << "class " << _dllClassExport << name << " : ";
    H << "public virtual ::Ice::Proxy<" << name << ", ";
    if(bases.empty())
    {
        H << "::IceProxy::Ice::Object";
    }
    else if(bases.size() == 1)
    {
        H << "::IceProxy" << fixKwd(bases.front()->scoped());
    }
    else
    {
        H << fixKwd("_" + p->name() + "Base");
    }
    H << ">";

    H << sb;
    H.dec();
    H << nl << "public:";
    H.inc();

    C << sp;
    C << nl << "/// \\cond INTERNAL";
    C << nl
      << _dllExport
      << "::IceProxy::Ice::Object* ::IceProxy" << scope << "upCast(" << name << "* p) { return p; }";

    C << sp;
    C << nl << "void" << nl << "::IceProxy" << scope << "_readProxy(::Ice::InputStream* istr, "
      << "::IceInternal::ProxyHandle< " << name << ">& v)";
    C << sb;
    C << nl << "::Ice::ObjectPrx proxy;";
    C << nl << "istr->read(proxy);";
    C << nl << "if(!proxy)";
    C << sb;
    C << nl << "v = 0;";
    C << eb;
    C << nl << "else";
    C << sb;
    C << nl << "v = new " << name << ';';
    C << nl << "v->_copyFrom(proxy);";
    C << eb;
    C << eb;
    C << nl << "/// \\endcond";

    return true;
}

void
Slice::Gen::ProxyVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    string name = fixKwd(p->name());
    string scoped = fixKwd(p->scoped());
    string scope = fixKwd(p->scope());

    H << sp;
    H << nl << "/**";
    H << nl << " * Obtains the Slice type ID corresponding to this " << (p->isInterface() ? "interface" : "class")
      << ".";
    H << nl << " * @return A fully-scoped type ID.";
    H << nl << " */";
    H << nl << _dllMemberExport << "static const ::std::string& ice_staticId();";

    H.dec();
    H << sp << nl << "protected:";
    H.inc();
    H << nl << "/// \\cond INTERNAL";
    H << sp << nl << _dllMemberExport << "virtual ::IceProxy::Ice::Object* _newInstance() const;";
    H << nl << "/// \\endcond";
    H << eb << ';';

    C << sp;
    C << nl << "/// \\cond INTERNAL";
    C << nl << "::IceProxy::Ice::Object*";
    C << nl << "IceProxy" << scoped << "::_newInstance() const";
    C << sb;
    C << nl << "return new " << name << ";";
    C << eb;
    C << nl << "/// \\endcond";

    C << sp;
    C << nl << "const ::std::string&" << nl << "IceProxy" << scoped << "::ice_staticId()";
    C << sb;
    C << nl << "return " << scoped << "::ice_staticId();";
    C << eb;

    _useWstring = resetUseWstring(_useWstringHist);
}

namespace
{

bool
usePrivateEnd(const OperationPtr& p)
{
    TypePtr ret = p->returnType();
    bool retIsOpt = p->returnIsOptional();
    string retSEnd = returnTypeToString(ret, retIsOpt, "", p->getMetaData(), TypeContextAMIEnd);
    string retSPrivateEnd = returnTypeToString(ret, retIsOpt, "", p->getMetaData(), TypeContextAMIPrivateEnd);

    ParamDeclList outParams;
    vector<string> outDeclsEnd;
    vector<string> outDeclsPrivateEnd;

    ParamDeclList paramList = p->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if((*q)->isOutParam())
        {
            outDeclsEnd.push_back(outputTypeToString((*q)->type(), (*q)->optional(), "", (*q)->getMetaData(),
                                                     TypeContextAMIEnd));
            outDeclsPrivateEnd.push_back(outputTypeToString((*q)->type(), (*q)->optional(), "", (*q)->getMetaData(),
                                                            TypeContextAMIPrivateEnd));
        }
    }

    return retSEnd != retSPrivateEnd || outDeclsEnd != outDeclsPrivateEnd;
}

}

void
Slice::Gen::ProxyVisitor::visitOperation(const OperationPtr& p)
{
    string name = p->name();
    string flatName = "iceC" + p->flattenedScope() + p->name() + "_name";
    string scoped = fixKwd(p->scoped());
    string scope = fixKwd(p->scope());

    TypePtr ret = p->returnType();

    bool retIsOpt = p->returnIsOptional();
    string retS = returnTypeToString(ret, retIsOpt, "", p->getMetaData(), _useWstring | TypeContextAMIEnd);
    string retSEndAMI =
        returnTypeToString(ret, retIsOpt, "", p->getMetaData(), _useWstring | TypeContextAMIPrivateEnd);
    string retInS = retS != "void" ? inputTypeToString(ret, retIsOpt, "", p->getMetaData(), _useWstring) : "";

    ContainerPtr container = p->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    string clName = cl->name();
    string clScope = fixKwd(cl->scope());
    string delName = "Callback_" + clName + "_" + name;
    string delNameScoped = clScope + delName;

    vector<string> paramsDecl;
    vector<string> args;

    vector<string> paramsAMI;
    vector<string> paramsDeclAMI;
    vector<string> paramsDeclAMIBeginI;
    vector<string> argsAMI;
    vector<string> outParamsAMI;
    vector<string> outParamNamesAMI;
    vector<string> outParamsDeclAMI;
    vector<string> outParamsDeclImplAMI;
    vector<string> outParamsDeclEndAMI;
    vector<string> outDecls;

    ParamDeclList paramList = p->parameters();
    ParamDeclList inParams = p->inParameters();
    ParamDeclList outParams = p->outParameters();

    const string contextParam = escapeParam(paramList, "context");
    const string cbParam = escapeParam(inParams, "cb");
    const string cookieParam = escapeParam(paramList, "cookie");
    const string resultParam = escapeParam(outParams, "result");

    vector<string> outEndArgs;

    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        string paramName = fixKwd((*q)->name());

        StringList metaData = (*q)->getMetaData();
        string typeString;
        string typeStringEndAMI;
        if((*q)->isOutParam())
        {
            typeString =
                outputTypeToString((*q)->type(), (*q)->optional(), "", metaData, _useWstring | TypeContextAMIEnd);
            typeStringEndAMI = outputTypeToString((*q)->type(), (*q)->optional(), "", metaData,
                                                  _useWstring | TypeContextAMIPrivateEnd);
        }
        else
        {
            typeString = inputTypeToString((*q)->type(), (*q)->optional(), "", metaData, _useWstring);
        }

        paramsDecl.push_back(typeString + ' ' + paramName);
        args.push_back(paramName);

        if(!(*q)->isOutParam())
        {
            paramsAMI.push_back(typeString);
            paramsDeclAMI.push_back(typeString + ' ' + paramName);
            paramsDeclAMIBeginI.push_back(typeString + ' ' + paramPrefix + (*q)->name());
            argsAMI.push_back(paramName);
        }
        else
        {
            outParamsAMI.push_back(typeString);
            outParamNamesAMI.push_back(paramName);
            outParamsDeclAMI.push_back(typeString + ' ' + paramName);
            outParamsDeclImplAMI.push_back(typeString + ' ' + paramPrefix + (*q)->name());
            outParamsDeclEndAMI.push_back(typeStringEndAMI + ' ' + paramPrefix + (*q)->name());
            outDecls.push_back(
                inputTypeToString((*q)->type(), (*q)->optional(), "", (*q)->getMetaData(), _useWstring));
            outEndArgs.push_back(getEndArg((*q)->type(), (*q)->getMetaData(), outParamNamesAMI.back()));
        }
    }

    //
    // Check if we need to generate a private _iceI_end_ method. This is the case if the
    // when using certain mapping features such as cpp:array. While
    // the regular end_ method can't return pair<const TYPE*, const TYPE*> because the
    // pointers would be invalid once end_ returns, we still want to allow using this
    // alternate mapping with AMI response callbacks (to allow zero-copy for instance).
    // For this purpose, we generate a special _iceI_end method which is used by the
    // completed implementation of the generated Callback_Inft_opName operation
    // delegate.
    //
    bool generatePrivateEnd = retS != retSEndAMI || outParamsDeclAMI != outParamsDeclEndAMI;
    if(ret && generatePrivateEnd)
    {
        string typeStringEndAMI = outputTypeToString(ret, p->returnIsOptional(), "", p->getMetaData(),
                                                     _useWstring | TypeContextAMIPrivateEnd);
        outParamsDeclEndAMI.push_back(typeStringEndAMI + ' ' + "ret");
    }

    string thisPointer = fixKwd(scope.substr(0, scope.size() - 2)) + "*";

    CommentPtr comment = p->parseComment(false);
    const string contextDoc = "@param " + contextParam + " The Context map to send with the invocation.";
    const string contextDecl = "const ::Ice::Context& " + contextParam + " = ::Ice::noExplicitContext";
    const string resultDoc = "The asynchronous result object for the invocation.";
    const string cbDoc = "@param " + cbParam + " Asynchronous callback object.";
    const string cookieDoc = "@param " + cookieParam + " User-defined data to associate with the invocation.";
    const string cookieDecl = "const ::Ice::LocalObjectPtr& " + cookieParam + " = 0";

    const string deprecateSymbol = getDeprecateSymbol(p, cl);
    H << sp;
    if(comment)
    {
        StringList postParams;
        postParams.push_back(contextDoc);
        writeOpDocSummary(H, p, comment, OpDocAllParams, true, StringList(), postParams, comment->returns());
    }
    H << nl << deprecateSymbol << _dllMemberExport << retS << ' ' << fixKwd(name) << spar << paramsDecl
      << contextDecl << epar;
    H << sb << nl;
    if(ret)
    {
        H << "return ";
    }
    H << "end_" << name << spar << outParamNamesAMI << "_iceI_begin_" + name << spar << argsAMI;
    H << contextParam << "::IceInternal::dummyCallback" << "0" << "true" << epar << epar << ';';
    H << eb;

    H << sp;
    if(comment)
    {
        StringList postParams, returns;
        postParams.push_back(contextDoc);
        returns.push_back(resultDoc);
        writeOpDocSummary(H, p, comment, OpDocInParams, false, StringList(), postParams, returns);
    }
    H << nl << "::Ice::AsyncResultPtr begin_" << name << spar << paramsDeclAMI << contextDecl << epar;
    H << sb;
    H << nl << "return _iceI_begin_" << name << spar << argsAMI << contextParam << "::IceInternal::dummyCallback"
      << "0"
      << epar << ';';
    H << eb;

    H << sp;
    if(comment)
    {
        StringList postParams, returns;
        postParams.push_back(cbDoc);
        postParams.push_back(cookieDoc);
        returns.push_back(resultDoc);
        writeOpDocSummary(H, p, comment, OpDocInParams, false, StringList(), postParams, returns);
    }
    H << nl << "::Ice::AsyncResultPtr begin_" << name << spar << paramsDeclAMI
      << "const ::Ice::CallbackPtr& " + cbParam
      << cookieDecl << epar;
    H << sb;
    H << nl << "return _iceI_begin_" << name << spar << argsAMI << "::Ice::noExplicitContext" << cbParam << cookieParam
      << epar << ';';
    H << eb;

    H << sp;
    if(comment)
    {
        StringList postParams, returns;
        postParams.push_back(contextDoc);
        postParams.push_back(cbDoc);
        postParams.push_back(cookieDoc);
        returns.push_back(resultDoc);
        writeOpDocSummary(H, p, comment, OpDocInParams, false, StringList(), postParams, returns);
    }
    H << nl << "::Ice::AsyncResultPtr begin_" << name << spar << paramsDeclAMI
      << "const ::Ice::Context& " + contextParam
      << "const ::Ice::CallbackPtr& " + cbParam
      << cookieDecl << epar;
    H << sb;
    H << nl << "return _iceI_begin_" << name << spar << argsAMI << contextParam << cbParam << cookieParam << epar
      << ';';
    H << eb;

    H << sp;
    if(comment)
    {
        StringList postParams, returns;
        postParams.push_back(cbDoc);
        postParams.push_back(cookieDoc);
        returns.push_back(resultDoc);
        writeOpDocSummary(H, p, comment, OpDocInParams, false, StringList(), postParams, returns);
    }
    H << nl << "::Ice::AsyncResultPtr begin_" << name << spar << paramsDeclAMI
      << "const " + delNameScoped + "Ptr& " + cbParam
      << cookieDecl << epar;
    H << sb;
    H << nl << "return _iceI_begin_" << name << spar << argsAMI << "::Ice::noExplicitContext" << cbParam << cookieParam
      << epar << ';';
    H << eb;

    H << sp;
    if(comment)
    {
        StringList postParams, returns;
        postParams.push_back(contextDoc);
        postParams.push_back(cbDoc);
        postParams.push_back(cookieDoc);
        returns.push_back(resultDoc);
        writeOpDocSummary(H, p, comment, OpDocInParams, false, StringList(), postParams, returns);
    }
    H << nl << "::Ice::AsyncResultPtr begin_" << name << spar << paramsDeclAMI
      << "const ::Ice::Context& " + contextParam
      << "const " + delNameScoped + "Ptr& " + cbParam
      << cookieDecl << epar;
    H << sb;
    H << nl << "return _iceI_begin_" << name << spar << argsAMI << contextParam << cbParam << cookieParam << epar
      << ';';
    H << eb;

    H << sp;
    if(comment)
    {
        H << nl << "/**";
        H << nl << " * Completes an invocation of begin_" << name << ".";
        StringList postParams;
        postParams.push_back("@param " + resultParam + " " + resultDoc);
        writeOpDocParams(H, p, comment, OpDocOutParams, StringList(), postParams);
        if(!comment->returns().empty())
        {
            H << nl << " * @return ";
            writeDocLines(H, comment->returns(), false);
        }
        if(!comment->exceptions().empty())
        {
            writeOpDocExceptions(H, p, comment);
        }
        H << nl << " */";
    }
    H << nl << _dllMemberExport << retS << " end_" << name << spar << outParamsDeclAMI
      << "const ::Ice::AsyncResultPtr& " + resultParam << epar << ';';
    if(generatePrivateEnd)
    {
        H << nl << "/// \\cond INTERNAL";
        H << sp << nl << _dllMemberExport << "void _iceI_end_" << name << spar << outParamsDeclEndAMI;
        H << "const ::Ice::AsyncResultPtr&" << epar << ';';
        H << nl << "/// \\endcond";
    }

    H.dec();
    H << nl;
    H << nl << "private:";
    H.inc();
    H << sp << nl << _dllMemberExport << "::Ice::AsyncResultPtr _iceI_begin_" << name << spar
      << paramsAMI << "const ::Ice::Context&"
      << "const ::IceInternal::CallbackBasePtr&"
      << "const ::Ice::LocalObjectPtr& cookie = 0"
      << "bool sync = false" << epar << ';';
    H.dec();
    H << nl;
    H << nl << "public:";
    H.inc();

    C << sp << nl << "::Ice::AsyncResultPtr" << nl << "IceProxy" << scope << "_iceI_begin_" << name << spar
      << paramsDeclAMIBeginI
      << "const ::Ice::Context& context" << "const ::IceInternal::CallbackBasePtr& del"
      << "const ::Ice::LocalObjectPtr& cookie" << "bool sync" << epar;
    C << sb;
    if(p->returnsData())
    {
        C << nl << "_checkTwowayOnly(" << flatName <<  ", sync);";
    }
    C << nl << "::IceInternal::OutgoingAsyncPtr result = new ::IceInternal::CallbackOutgoing(this, " << flatName
        << ", del, cookie, sync);";
    C << nl << "try";
    C << sb;
    C << nl << "result->prepare(" << flatName << ", " << operationModeToString(p->sendMode()) << ", context);";
    if(inParams.empty())
    {
        C << nl << "result->writeEmptyParams();";
    }
    else
    {
        C << nl << "::Ice::OutputStream* ostr = result->startWriteParams(" << opFormatTypeToString(p, false) <<");";
        writeMarshalCode(C, inParams, 0, true, TypeContextInParam);
        if(p->sendsClasses(false))
        {
            C << nl << "ostr->writePendingValues();";
        }
        C << nl << "result->endWriteParams();";
    }
    C << nl << "result->invoke(" << flatName << ");";
    C << eb;
    C << nl << "catch(const ::Ice::Exception& ex)";
    C << sb;
    C << nl << "result->abort(ex);";
    C << eb;
    C << nl << "return result;";
    C << eb;

    C << sp << nl << retS << nl << "IceProxy" << scope << "end_" << name << spar << outParamsDeclImplAMI
      << "const ::Ice::AsyncResultPtr& result" << epar;
    C << sb;
    if(p->returnsData())
    {
        C << nl << "::Ice::AsyncResult::_check(result, this, " << flatName << ");";

        //
        // COMPILERFIX: It's necessary to generate the allocate code here before
        // this if(!result->wait()). If generated after this if block, we get
        // access violations errors with the test/Ice/slicing/objects test on VC9
        // and Windows 64 bits when compiled with optimization (see bug 4400).
        //
        writeAllocateCode(C, ParamDeclList(), p, true, "", _useWstring | TypeContextAMIEnd);
        C << nl << "if(!result->_waitForResponse())";
        C << sb;
        C << nl << "try";
        C << sb;
        C << nl << "result->_throwUserException();";
        C << eb;
        //
        // Generate a catch block for each legal user exception.
        //
        ExceptionList throws = p->throws();
        throws.sort();
        throws.unique();
#if defined(__SUNPRO_CC)
        throws.sort(derivedToBaseCompare);
#else
        throws.sort(Slice::DerivedToBaseCompare());
#endif
        for(ExceptionList::const_iterator i = throws.begin(); i != throws.end(); ++i)
        {
            C << nl << "catch(const " << fixKwd((*i)->scoped()) << "&)";
            C << sb;
            C << nl << "throw;";
            C << eb;
        }
        C << nl << "catch(const ::Ice::UserException& ex)";
        C << sb;
        C << nl << "throw ::Ice::UnknownUserException(__FILE__, __LINE__, ex.ice_id());";
        C << eb;
        C << eb;
        if(ret || !outParams.empty())
        {
            C << nl << "::Ice::InputStream* istr = result->_startReadParams();";
            writeUnmarshalCode(C, outParams, p, true, _useWstring | TypeContextAMIEnd);
            if(p->returnsClasses(false))
            {
                C << nl << "istr->readPendingValues();";
            }
            C << nl << "result->_endReadParams();";
        }
        else
        {
            C << nl << "result->_readEmptyParams();";
        }
        if(ret)
        {
            C << nl << "return ret;";
        }
    }
    else
    {
        C << nl << "_end(result, " << flatName << ");";
    }
    C << eb;

    if(generatePrivateEnd)
    {
        assert(p->returnsData());

        C << sp << nl << "void IceProxy" << scope << "_iceI_end_" << name << spar << outParamsDeclEndAMI
          << "const ::Ice::AsyncResultPtr& result" << epar;
        C << sb;
        C << nl << "::Ice::AsyncResult::_check(result, this, " << flatName << ");";
        C << nl << "if(!result->_waitForResponse())";
        C << sb;
        C << nl << "try";
        C << sb;
        C << nl << "result->_throwUserException();";
        C << eb;
        //
        // Generate a catch block for each legal user exception.
        //
        ExceptionList throws = p->throws();
        throws.sort();
        throws.unique();
#if defined(__SUNPRO_CC)
        throws.sort(derivedToBaseCompare);
#else
        throws.sort(Slice::DerivedToBaseCompare());
#endif
        for(ExceptionList::const_iterator i = throws.begin(); i != throws.end(); ++i)
        {
            C << nl << "catch(const " << fixKwd((*i)->scoped()) << "&)";
            C << sb;
            C << nl << "throw;";
            C << eb;
        }
        C << nl << "catch(const ::Ice::UserException& ex)";
        C << sb;
        C << nl << "throw ::Ice::UnknownUserException(__FILE__, __LINE__, ex.ice_id());";
        C << eb;
        C << eb;

        if(ret || !outParams.empty())
        {
            C << nl << "::Ice::InputStream* istr = result->_startReadParams();";
            writeUnmarshalCode(C, outParams, p, true, _useWstring | TypeContextAMIPrivateEnd);
            if(p->returnsClasses(false))
            {
                C << nl << "istr->readPendingValues();";
            }
            C << nl << "result->_endReadParams();";
        }
        else
        {
            C << nl << "result->_readEmptyParams();";
        }
        C << eb;
    }
}

Slice::Gen::ObjectDeclVisitor::ObjectDeclVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport)
{
}

bool
Slice::Gen::ObjectDeclVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasClassDecls())
    {
        return false;
    }

    string name = fixKwd(p->name());

    H << sp << nl << "namespace " << name << nl << '{';
    C << sp << nl << "namespace" << nl << "{";
    return true;
}

void
Slice::Gen::ObjectDeclVisitor::visitModuleEnd(const ModulePtr&)
{
    H << sp << nl << '}';
    C << sp << nl << "}";
}

void
Slice::Gen::ObjectDeclVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    string name = fixKwd(p->name());
    string scope = fixKwd(p->scope());
    string scoped = fixKwd(p->scoped());

    H << sp << nl << "class " << name << ';';
    if(!p->isLocal())
    {
        //
        // upCast is not _upCast nor _iceUpCast for historical reasons. IceInternal::Handle
        // depends on this name
        //
        H << nl << "/// \\cond INTERNAL";
        H << nl << _dllExport << getUnqualified("::Ice::Object*", scope) << " upCast(" << name << "*);";
        H << nl << "/// \\endcond";
        H << nl << "typedef ::IceInternal::Handle< " << name << "> " << p->name() << "Ptr;";
        H << nl << "typedef ::IceInternal::ProxyHandle< ::IceProxy" << scoped << "> " << p->name() << "Prx;";
        H << nl << "typedef " << p->name() << "Prx " << p->name() << "PrxPtr;";

        //
        // _ice prefix because this function is in the Slice module namespace, where the user
        // is allowed to define classes, functions etc. that start with _.
        //
        H << nl << "/// \\cond INTERNAL";
        H << nl << _dllExport << "void _icePatchObjectPtr(" << p->name() << "Ptr&, const "
          << getUnqualified("::Ice::ObjectPtr&", scope) << ");";
        H << nl << "/// \\endcond";
    }
    else
    {
        H << nl << "/// \\cond INTERNAL";
        H << nl << _dllExport << getUnqualified("::Ice::LocalObject*", scope) << " upCast("
          << getUnqualified(scoped, scope) << "*);";
        H << nl << "/// \\endcond";
        H << nl << "typedef ::IceInternal::Handle< " << name << "> " << p->name() << "Ptr;";
    }
}

void
Slice::Gen::ObjectDeclVisitor::visitOperation(const OperationPtr& p)
{
    ClassDefPtr cl = ClassDefPtr::dynamicCast(p->container());
    if(cl && !cl->isLocal())
    {
        string flatName = "iceC" + p->flattenedScope() + p->name() + "_name";
        C << sp << nl << "const ::std::string " << flatName << " = \"" << p->name() << "\";";
    }
}

Slice::Gen::ObjectVisitor::ObjectVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport), _doneStaticSymbol(false), _useWstring(false)
{
}

bool
Slice::Gen::ObjectVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasClassDefs())
    {
        return false;
    }

    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    string name = fixKwd(p->name());

    H << sp << nl << "namespace " << name << nl << '{';

    return true;
}

void
Slice::Gen::ObjectVisitor::visitModuleEnd(const ModulePtr&)
{
    H << sp;
    H << nl << '}';

    _useWstring = resetUseWstring(_useWstringHist);
}

bool
Slice::Gen::ObjectVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    string name = fixKwd(p->name());
    string scope = fixKwd(p->scope());
    string scoped = fixKwd(p->scoped());
    ClassList bases = p->bases();
    ClassDefPtr base;
    if(!bases.empty() && !bases.front()->isInterface())
    {
        base = bases.front();
    }
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList allDataMembers = p->allDataMembers();
    bool basePreserved = p->inheritsMetaData("preserve-slice");
    bool preserved = basePreserved || p->hasMetaData("preserve-slice");

    H << sp;
    writeDocSummary(H, p);
    H << nl << "class " << _dllExport << name << " : ";
    H.useCurrentPosAsIndent();
    if(bases.empty())
    {
        H << "public virtual " << getUnqualified(p->isLocal() ? "::Ice::LocalObject" : "::Ice::Object", scope);
    }
    else
    {
        ClassList::const_iterator q = bases.begin();
        bool virtualInheritance = p->hasMetaData("cpp:virtual") || p->isInterface();
        while(q != bases.end())
        {
            if(virtualInheritance || (*q)->isInterface())
            {
                H << "virtual ";
            }

            H << "public " << getUnqualified(fixKwd((*q)->scoped()), scope);
            if(++q != bases.end())
            {
                H << ',' << nl;
            }
        }
    }

    bool hasBaseClass = !bases.empty() && !bases.front()->isInterface();
    bool override = !p->isLocal() && p->canBeCyclic() && (!hasBaseClass || !bases.front()->canBeCyclic());
    bool hasGCObjectBaseClass = basePreserved || override || preserved;
    if(!basePreserved && (override || preserved))
    {
        H << ", public ::IceInternal::GCObject";
    }

    H.restoreIndent();
    H << sb;
    H.dec();
    H << nl << "public:" << sp;
    H.inc();

    //
    // In C++, a nested type cannot have the same name as the enclosing type
    //
    if(!p->isLocal() && p->name() != "ProxyType")
    {
        H << nl << "typedef " << p->name() << "Prx ProxyType;";
    }

    if(p->name() != "PointerType")
    {
        H << nl << "typedef " << p->name() << "Ptr PointerType;";
    }

    H << sp << nl << "virtual ~" << name << "();";
    C << sp;
    C << nl << scoped.substr(2) << "::~" << name << "()";
    C << sb;
    C << eb;

    vector<string> params;
    vector<string> allParamDecls;

    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        params.push_back(fixKwd((*q)->name()));
    }

    for(DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
    {
        string typeName = inputTypeToString((*q)->type(), (*q)->optional(), scope, (*q)->getMetaData(), _useWstring);
        allParamDecls.push_back(typeName + " " + fixKwd((*q)->name()));
    }

    if(!p->isInterface())
    {
        if(p->hasDefaultValues())
        {
            H << sp;
            H << nl << "/** Default constructor that assigns default values to members as specified in the "
              "Slice definition. */";
            H << nl << name << "() :";
            H.inc();
            writeDataMemberInitializers(H, dataMembers, _useWstring);
            H.dec();
            H << sb;
            H << eb;
        }
        else
        {
            H << sp << nl << name << "()";
            H << sb << eb;
        }

        emitOneShotConstructor(p);
    }

    if(!p->isLocal())
    {
        C << sp;
        C << nl << "/// \\cond INTERNAL";
        C << nl
          << _dllExport
          << "::Ice::Object* " << scope.substr(2) << "upCast(" << name << "* p) { return p; }"
          << nl;
        C << nl << "/// \\endcond";

        //
        // It would make sense to provide a covariant ice_clone(); unfortunately many compilers
        // (including VS2010) generate bad code for covariant types that use virtual inheritance
        //

        if(!p->isInterface())
        {
            H << sp;
            H << nl << "/**";
            H << nl << " * Polymporphically clones this object.";
            H << nl << " * @return A shallow copy of this object.";
            H << nl << " */";
            H << nl << "virtual " << getUnqualified("::Ice::ObjectPtr", scope) << " ice_clone() const;";

            if(hasGCObjectBaseClass)
            {
                C.zeroIndent();
                C << sp;
                C << nl << "#if defined(_MSC_VER) && (_MSC_VER >= 1900)";
                C << nl << "#   pragma warning(push)";
                C << nl << "#   pragma warning(disable:4589)";
                C << nl << "#endif";
                C.restoreIndent();
            }
            C << nl << "::Ice::ObjectPtr";
            C << nl << scoped.substr(2) << "::ice_clone() const";
            C << sb;
            if(!p->isAbstract())
            {
                C << nl << getUnqualified("::Ice::Object*", scope) << " p = new " << name << "(*this);";
                C << nl << "return p;";
            }
            else
            {
                //
                // We need this ice_clone for abstract classes derived from concrete classes
                //
                C << nl << "throw " << getUnqualified("::Ice::CloneNotImplementedException", scope)
                  << "(__FILE__, __LINE__);";
            }
            C << eb;
            if(hasGCObjectBaseClass)
            {
                C.zeroIndent();
                C << nl << "#if defined(_MSC_VER) && (_MSC_VER >= 1900)";
                C << nl << "#   pragma warning(pop)";
                C << nl << "#endif";
                C.restoreIndent();
            }
        }

        ClassList allBases = p->allBases();
        StringList ids;
        transform(allBases.begin(), allBases.end(), back_inserter(ids), ::IceUtil::constMemFun(&Contained::scoped));
        StringList other;
        other.push_back(p->scoped());
        other.push_back("::Ice::Object");
        other.sort();
        ids.merge(other);
        ids.unique();
        StringList::const_iterator firstIter = ids.begin();
        StringList::const_iterator scopedIter = find(ids.begin(), ids.end(), p->scoped());
        assert(scopedIter != ids.end());
        StringList::difference_type scopedPos = IceUtilInternal::distance(firstIter, scopedIter);

        H << sp;
        H << nl << "/**";
        H << nl << " * Determines whether this object supports an interface with the given Slice type ID.";
        H << nl << " * @param id The fully-scoped Slice type ID.";
        H << nl << " * @param current The Current object for the invocation.";
        H << nl << " * @return True if this object supports the interface, false, otherwise.";
        H << nl << " */";
        H << nl << "virtual bool ice_isA(const ::std::string& id, const " << getUnqualified("::Ice::Current&", scope)
          << " current = " << getUnqualified("::Ice::emptyCurrent", scope) << ") const;";
        H << sp;
        H << nl << "/**";
        H << nl << " * Obtains a list of the Slice type IDs representing the interfaces supported by this object.";
        H << nl << " * @param current The Current object for the invocation.";
        H << nl << " * @return A list of fully-scoped type IDs.";
        H << nl << " */";
        H << nl << "virtual ::std::vector< ::std::string> ice_ids(const " << getUnqualified("::Ice::Current&", scope)
          << " current = " << getUnqualified("::Ice::emptyCurrent", scope) << ") const;";
        H << sp;
        H << nl << "/**";
        H << nl << " * Obtains a Slice type ID representing the most-derived interface supported by this object.";
        H << nl << " * @param current The Current object for the invocation.";
        H << nl << " * @return A fully-scoped type ID.";
        H << nl << " */";
        H << nl << "virtual const ::std::string& ice_id(const " << getUnqualified("::Ice::Current&", scope)
          << " current = " << getUnqualified("::Ice::emptyCurrent", scope) << ") const;";
        H << sp;
        H << nl << "/**";
        H << nl << " * Obtains the Slice type ID corresponding to this class.";
        H << nl << " * @return A fully-scoped type ID.";
        H << nl << " */";
        H << nl << "static const ::std::string& ice_staticId();";

        string flatName = "iceC" + p->flattenedScope() + p->name() + "_ids";

        C << sp << nl << "namespace";
        C << nl << "{";
        C << nl << "const ::std::string " << flatName << '[' << ids.size() << "] =";
        C << sb;

        for(StringList::const_iterator r = ids.begin(); r != ids.end();)
        {
            C << nl << '"' << *r << '"';
            if(++r != ids.end())
            {
                C << ',';
            }
        }
        C << eb << ';';
        C << sp << nl << "}";

        C << sp;
        C << nl << "bool" << nl << scoped.substr(2)
          << "::ice_isA(const ::std::string& s, const " << getUnqualified("::Ice::Current&", scope) << ") const";
        C << sb;
        C << nl << "return ::std::binary_search(" << flatName << ", " << flatName << " + " << ids.size() << ", s);";
        C << eb;

        C << sp;
        C << nl << "::std::vector< ::std::string>" << nl << scoped.substr(2)
          << "::ice_ids(const " << getUnqualified("::Ice::Current&", scope) << ") const";
        C << sb;
        C << nl << "return ::std::vector< ::std::string>(&" << flatName << "[0], &" << flatName
          << '[' << ids.size() << "]);";
        C << eb;

        C << sp;
        C << nl << "const ::std::string&" << nl << scoped.substr(2)
          << "::ice_id(const " << getUnqualified("::Ice::Current&", scope) << ") const";
        C << sb;
        C << nl << "return ice_staticId();";
        C << eb;

        C << sp;
        C << nl << "const ::std::string&" << nl << scoped.substr(2) << "::ice_staticId()";
        C << sb;
        C.zeroIndent();
        C << nl << "#ifdef ICE_HAS_THREAD_SAFE_LOCAL_STATIC";
        C.restoreIndent();
        C << nl << "static const ::std::string typeId = \"" << *scopedIter << "\";";
        C << nl << "return typeId;";
        C.zeroIndent();
        C << nl << "#else";
        C.restoreIndent();
        C << nl << "return " << flatName << '[' << scopedPos << "];";
        C.zeroIndent();
        C << nl << "#endif";
        C.restoreIndent();
        C << eb;

        emitGCFunctions(p);
    }
    else
    {
        C << sp;
        C << nl << "/// \\cond INTERNAL";
        C << nl
          << _dllExport
          << "::Ice::LocalObject* " << scope.substr(2) << "upCast(" << getUnqualified(scoped, scope)
          << "* p) { return p; }";
        C << nl << "/// \\endcond";
    }

    return true;
}

void
Slice::Gen::ObjectVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    string name = fixKwd(p->name());
    string scoped = fixKwd(p->scoped());
    string scope = fixKwd(p->scope());
    ClassList bases = p->bases();
    ClassDefPtr base;
    if(!bases.empty() && !bases.front()->isInterface())
    {
        base = bases.front();
    }
    bool basePreserved = p->inheritsMetaData("preserve-slice");
    bool preserved = p->hasMetaData("preserve-slice");

    bool inProtected = false;

    if(!p->isLocal())
    {
        OperationList allOps = p->allOperations();
        if(!allOps.empty())
        {
            StringList allOpNames;
            transform(allOps.begin(), allOps.end(), back_inserter(allOpNames),
                      ::IceUtil::constMemFun(&Contained::name));

            allOpNames.push_back("ice_id");
            allOpNames.push_back("ice_ids");
            allOpNames.push_back("ice_isA");
            allOpNames.push_back("ice_ping");
            allOpNames.sort();
            allOpNames.unique();

            H << sp;
            H << nl << "/// \\cond INTERNAL";
            H << nl << "virtual bool _iceDispatch(::IceInternal::Incoming&, const "
              << getUnqualified("::Ice::Current&", scope) << ");";
            H << nl << "/// \\endcond";

            string flatName = "iceC" + p->flattenedScope() + p->name() + "_all";
            C << sp << nl << "namespace";
            C << nl << "{";
            C << nl << "const ::std::string " << flatName << "[] =";
            C << sb;

            for(StringList::const_iterator q = allOpNames.begin(); q != allOpNames.end();)
            {
                C << nl << '"' << *q << '"';
                if(++q != allOpNames.end())
                {
                    C << ',';
                }
            }
            C << eb << ';';
            C << sp << nl << "}";
            C << sp;
            C << nl << "/// \\cond INTERNAL";
            C << nl << "bool";
            C << nl << scoped.substr(2) << "::_iceDispatch(::IceInternal::Incoming& in, const "
              << getUnqualified("::Ice::Current&", scope) << " current)";
            C << sb;

            C << nl << "::std::pair<const ::std::string*, const ::std::string*> r = "
              << "::std::equal_range(" << flatName << ", " << flatName << " + " << allOpNames.size()
              << ", current.operation);";
            C << nl << "if(r.first == r.second)";
            C << sb;
            C << nl << "throw " << getUnqualified("::Ice::OperationNotExistException", scope)
              << "(__FILE__, __LINE__, current.id, " << "current.facet, current.operation);";
            C << eb;
            C << sp;
            C << nl << "switch(r.first - " << flatName << ')';
            C << sb;
            int i = 0;
            for(StringList::const_iterator q = allOpNames.begin(); q != allOpNames.end(); ++q)
            {
                C << nl << "case " << i++ << ':';
                C << sb;
                C << nl << "return _iceD_" << *q << "(in, current);";
                C << eb;
            }
            C << nl << "default:";
            C << sb;
            C << nl << "assert(false);";
            C << nl << "throw " << getUnqualified("::Ice::OperationNotExistException", scope)
              << "(__FILE__, __LINE__, current.id, " << "current.facet, current.operation);";
            C << eb;
            C << eb;
            C << eb;
            C << nl << "/// \\endcond";

            //
            // Check if we need to generate ice_operationAttributes()
            //
            map<string, int> attributesMap;
            for(OperationList::iterator r = allOps.begin(); r != allOps.end(); ++r)
            {
                int attributes = (*r)->attributes();
                if(attributes != 0)
                {
                    attributesMap.insert(map<string, int>::value_type((*r)->name(), attributes));
                }
            }

            if(!attributesMap.empty())
            {
                H << sp;
                H << nl << "/// \\cond INTERNAL";
                H << nl << "virtual " << getUnqualified("::Ice::Int", scope)
                  << " ice_operationAttributes(const ::std::string&) const;";
                H << nl << "/// \\endcond";

                string opAttrFlatName = "iceC" + p->flattenedScope() + p->name() + "_operationAttributes";

                C << sp << nl << "namespace";
                C << nl << "{";
                C << nl << "const int " << opAttrFlatName << "[] = ";
                C << sb;

                for(StringList::const_iterator q = allOpNames.begin(); q != allOpNames.end();)
                {
                    int attributes = 0;
                    string opName = *q;
                    map<string, int>::iterator it = attributesMap.find(opName);
                    if(it != attributesMap.end())
                    {
                        attributes = it->second;
                    }
                    C << nl << attributes;

                    if(++q != allOpNames.end())
                    {
                        C << ',';
                    }
                    C << " // " << opName;
                }

                C << eb << ';';
                C << sp << nl << "}";

                C << sp;

                C << nl << "::Ice::Int" << nl << scoped.substr(2)
                  << "::ice_operationAttributes(const ::std::string& opName) const";
                C << sb;

                C << nl << "::std::pair<const ::std::string*, const ::std::string*> r = "
                  << "::std::equal_range(" << flatName << ", " << flatName << " + " << allOpNames.size()
                  << ", opName);";
                C << nl << "if(r.first == r.second)";
                C << sb;
                C << nl << "return -1;";
                C << eb;

                C << nl << "return " << opAttrFlatName << "[r.first - " << flatName << "];";
                C << eb;
            }
        }

        if(!p->isAbstract())
        {
            H << sp;
            H << nl << "/**";
            H << nl << " * Obtains a value factory that instantiates this class.";
            H << nl << " * @return The value factory.";
            H << nl << " */";
            H << nl << "static " << getUnqualified("::Ice::ValueFactoryPtr", scope) << " ice_factory();";
        }

        if(preserved && !basePreserved)
        {
            H << sp;
            H << nl << "/**";
            H << nl << " * Obtains the SlicedData object created when an unknown class type was marshaled";
            H << nl << " * in the sliced format and the Ice run time sliced it to a known type.";
            H << nl << " * @return The SlicedData object, or nil if the class was not sliced or was not";
            H << nl << " * marshaled in the sliced format.";
            H << nl << " */";
            H << nl << "virtual " << getUnqualified("::Ice::SlicedDataPtr", scope) << " ice_getSlicedData() const;";

            H << sp;
            H << nl << "/// \\cond STREAM";
            H << nl << "virtual void _iceWrite(" << getUnqualified("::Ice::OutputStream*", scope) << ") const;";
            H << nl << "virtual void _iceRead(" << getUnqualified("::Ice::InputStream*", scope) << ");";
            H << nl << "/// \\endcond";
        }

        H.dec();
        H << sp << nl << "protected:";
        inProtected = true;
        H.inc();

        H << sp;
        H << nl << "/// \\cond STREAM";
        H << nl << "virtual void _iceWriteImpl(" << getUnqualified("::Ice::OutputStream*", scope) << ") const;";
        H << nl << "virtual void _iceReadImpl(" << getUnqualified("::Ice::InputStream*", scope) << ");";
        H << nl << "/// \\endcond";

        if(preserved && !basePreserved)
        {
            C << sp;
            C << nl << "::Ice::SlicedDataPtr" << nl << scoped.substr(2) << "::ice_getSlicedData() const";
            C << sb;
            C << nl << "return _iceSlicedData;";
            C << eb;

            C << sp;
            C << nl << "void" << nl << scoped.substr(2) << "::_iceWrite(" << getUnqualified("::Ice::OutputStream*", scope)
              << "ostr) const";
            C << sb;
            C << nl << "ostr->startValue(_iceSlicedData);";
            C << nl << "_iceWriteImpl(ostr);";
            C << nl << "ostr->endValue();";
            C << eb;

            C << sp;
            C << nl << "void" << nl << scoped.substr(2) << "::_iceRead(" << getUnqualified("::Ice::InputStream*", scope)
              << " istr)";
            C << sb;
            C << nl << "istr->startValue();";
            C << nl << "_iceReadImpl(istr);";
            C << nl << "_iceSlicedData = istr->endValue(true);";
            C << eb;
        }

        C << sp;
        C << nl << "/// \\cond STREAM";
        C << nl << "void" << nl << scoped.substr(2) << "::_iceWriteImpl(" << getUnqualified("::Ice::OutputStream*", scope)
          << " ostr) const";
        C << sb;
        C << nl << "ostr->startSlice(ice_staticId(), " << p->compactId() << (!base ? ", true" : ", false") << ");";
        C << nl << getUnqualified("::Ice::StreamWriter", scope) << "< " << name << ", "
          << getUnqualified("::Ice::OutputStream", scope) << ">::write(ostr, *this);";
        C << nl << "ostr->endSlice();";
        if(base)
        {
            emitUpcall(base, "::_iceWriteImpl(ostr);", scope);
        }
        C << eb;

        C << sp;
        C << nl << "void" << nl << scoped.substr(2) << "::_iceReadImpl(" << getUnqualified("::Ice::InputStream*", scope)
          << " istr)";
        C << sb;
        C << nl << "istr->startSlice();";
        C << nl << getUnqualified("::Ice::StreamReader", scope) << "< " << name << ", "
          << getUnqualified("::Ice::InputStream", scope) << ">::read(istr, *this);";
        C << nl << "istr->endSlice();";
        if(base)
        {
            emitUpcall(base, "::_iceReadImpl(istr);", scope);
        }
        C << eb;
        C << nl << "/// \\endcond";

        if(!p->isAbstract() || p->compactId() >= 0)
        {
            C << sp << nl << "namespace";
            C << nl << "{";

            if(!p->isAbstract())
            {
                string initName = "iceC" + p->flattenedScope() + p->name() + "_init";
                C << nl << "const ::IceInternal::DefaultValueFactoryInit< " << scoped << "> "
                  << initName << "(\"" << p->scoped() << "\");";
            }
            if(p->compactId() >= 0)
            {
                string initName = "iceC" + p->flattenedScope() + p->name() + "_compactIdInit";
                C << nl << "const ::IceInternal::CompactIdInit "
                  << initName << "(\"" << p->scoped() << "\", " << p->compactId() << ");";
            }
            C << nl << "}";

            if(!p->isAbstract())
            {
                C << sp << nl << "::Ice::ValueFactoryPtr" << nl << scoped.substr(2) << "::ice_factory()";
                C << sb;
                C << nl << "return ::IceInternal::factoryTable->getValueFactory(" << scoped << "::ice_staticId());";
                C << eb;
            }
        }
    }

    //
    // Emit data members. Access visibility may be specified by metadata.
    //
    bool generateFriend = false;
    DataMemberList dataMembers = p->dataMembers();
    bool prot = p->hasMetaData("protected");
    bool needSp = true;
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        if(prot || (*q)->hasMetaData("protected"))
        {
            if(!inProtected)
            {
                H.dec();
                H << sp << nl << "protected:" << sp;
                H.inc();
                inProtected = true;
                needSp = false;
            }
            generateFriend = true;
        }
        else
        {
            if(inProtected)
            {
                H.dec();
                H << sp << nl << "public:" << sp;
                H.inc();
                inProtected = false;
                needSp = false;
            }
        }

        if(needSp)
        {
            H << sp;
            needSp = false;
        }

        emitDataMember(*q);
    }

    if(!p->isLocal() && preserved && !basePreserved)
    {
        if(!inProtected)
        {
            H.dec();
            H << sp << nl << "protected:";
            H.inc();
            inProtected = true;
        }
        H << sp;
        H << nl << "/// \\cond STREAM";
        H << nl << "::Ice::SlicedDataPtr _iceSlicedData;";
        H << nl << "/// \\endcond";
    }

    if(generateFriend)
    {
        if(!inProtected)
        {
            H.dec();
            H << sp << nl << "protected:";
            H.inc();
            inProtected = true;
        }

        H << sp;
        H << nl << "template<typename T, typename S>";
        H << nl << "friend struct Ice::StreamWriter;";
        H << nl << "template<typename T, typename S>";
        H << nl << "friend struct Ice::StreamReader;";
    }

    H << eb << ';';

    if(!p->isAbstract() && !p->isLocal() && !_doneStaticSymbol)
    {
        //
        // We need an instance here to trigger initialization if the implementation is in a static library.
        // But we do this only once per source file, because a single instance is sufficient to initialize
        // all of the globals in a compilation unit.
        //
        H << nl << "/// \\cond INTERNAL";
        H << nl << "static ::Ice::ValueFactoryPtr _iceS_" << p->name() << "_init = " << fixKwd(p->scoped())
          << "::ice_factory();";
        H << nl << "/// \\endcond";
    }

    if(p->isLocal())
    {
        H << sp;
        H << nl << "/// \\cond INTERNAL";
        H << nl << "inline bool operator==(const " << fixKwd(p->name()) << "& lhs, const " << fixKwd(p->name())
          << "& rhs)";
        H << sb;
        H << nl << "return static_cast<const " << getUnqualified("::Ice::LocalObject&", scope)
          << ">(lhs) == static_cast<const " << getUnqualified("::Ice::LocalObject&", scope) << ">(rhs);";
        H << eb;
        H << sp;
        H << nl << "inline bool operator<(const " << fixKwd(p->name()) << "& lhs, const " << fixKwd(p->name())
          << "& rhs)";
        H << sb;
        H << nl << "return static_cast<const " << getUnqualified("::Ice::LocalObject&", scope)
          << ">(lhs) < static_cast<const " << getUnqualified("::Ice::LocalObject&", scope) << ">(rhs);";
        H << eb;
        H << nl << "/// \\endcond";
    }
    else
    {
        C << sp;
        C << nl << "/// \\cond INTERNAL";
        C << nl << "void";
        C << nl << scope.substr(2) << "_icePatchObjectPtr(" << p->name() << "Ptr& handle, const "
          << getUnqualified("::Ice::ObjectPtr&", scope) << " v)";
        C << sb;
        C << nl << "handle = " << p->name() << "Ptr::dynamicCast(v);";
        C << nl << "if(v && !handle)";
        C << sb;
        C << nl << "IceInternal::Ex::throwUOE(" << name << "::ice_staticId(), v);";
        C << eb;
        C << eb;
        C << nl << "/// \\endcond";

        H << sp;
        H << nl << "/// \\cond INTERNAL";
        H << nl << "inline bool operator==(const " << fixKwd(p->name()) << "& lhs, const " << fixKwd(p->name())
          << "& rhs)";
        H << sb;
        H << nl << "return static_cast<const " << getUnqualified("::Ice::Object&", scope)
          << ">(lhs) == static_cast<const " << getUnqualified("::Ice::Object&", scope) << ">(rhs);";
        H << eb;
        H << sp;
        H << nl << "inline bool operator<(const " << fixKwd(p->name()) << "& lhs, const " << fixKwd(p->name())
          << "& rhs)";
        H << sb;
        H << nl << "return static_cast<const " << getUnqualified("::Ice::Object&", scope)
          << ">(lhs) < static_cast<const " << getUnqualified("::Ice::Object&", scope) << ">(rhs);";
        H << eb;
        H << nl << "/// \\endcond";
    }

    _useWstring = resetUseWstring(_useWstringHist);
}

bool
Slice::Gen::ObjectVisitor::visitExceptionStart(const ExceptionPtr&)
{
    return false;
}

bool
Slice::Gen::ObjectVisitor::visitStructStart(const StructPtr&)
{
    return false;
}

void
Slice::Gen::ObjectVisitor::visitOperation(const OperationPtr& p)
{
    string name = p->name();
    string scoped = fixKwd(p->scoped());
    string scope = fixKwd(p->scope());

    ContainerPtr container = p->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    string classNameAMD = "AMD_" + cl->name();
    string classScope = fixKwd(cl->scope());
    string classScopedAMD = classScope + classNameAMD;

    TypePtr ret = p->returnType();
    string retS = returnTypeToString(ret, p->returnIsOptional(), classScope, p->getMetaData(), _useWstring);

    ParamDeclList inParams = p->inParameters();
    ParamDeclList outParams = p->outParameters();
    ParamDeclList paramList = p->parameters();

    const string cbParam = escapeParam(paramList, "cb");
    const string cookieParam = escapeParam(paramList, "cookie");
    const string resultParam = escapeParam(outParams, "result");
    const string currentParam = escapeParam(paramList, "current");

    string params = "(";
    string paramsDecl = "(";
    string args = "(";

    string paramsAMD = "(const " + classScopedAMD + '_' + name + "Ptr& " + cbParam + ", ";
    string argsAMD = "(new IceAsync" + classScopedAMD + '_' + name + "(inS), ";

    vector< string> outDecls;
    for(ParamDeclList::iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        string paramName = fixKwd((*q)->name());
        TypePtr type = (*q)->type();
        bool isOutParam = (*q)->isOutParam();
        string typeString;
        if(isOutParam)
        {
            typeString = outputTypeToString(type, (*q)->optional(), classScope, (*q)->getMetaData(), _useWstring);
        }
        else
        {
            typeString =
                inputTypeToString((*q)->type(), (*q)->optional(), classScope, (*q)->getMetaData(), _useWstring);
        }

        if(q != paramList.begin())
        {
            params += ", ";
            paramsDecl += ", ";
            args += ", ";
        }

        params += typeString;
        params += ' ';
        params += paramName;
        paramsDecl += typeString;
        paramsDecl += ' ';
        paramsDecl += paramName;
        args += paramPrefix + (*q)->name();

        if(!isOutParam)
        {
            paramsAMD += typeString;
            paramsAMD += " ";
            paramsAMD += paramName;
            paramsAMD += ", ";
            argsAMD += paramPrefix + (*q)->name();
            argsAMD += ", ";
        }
        else
        {
            outDecls.push_back(inputTypeToString((*q)->type(), (*q)->optional(), classScope, (*q)->getMetaData(),
                                                 _useWstring));
        }
    }

    if(!cl->isLocal())
    {
        if(!paramList.empty())
        {
            params += ", ";
            paramsDecl += ", ";
            args += ", ";
        }

        params += "const " + getUnqualified("::Ice::Current&", classScope) + " " + currentParam + " = " +
            getUnqualified("::Ice::emptyCurrent", classScope) + ")";
        paramsDecl += "const " + getUnqualified("::Ice::Current&", classScope) + " " + currentParam + ")";
        args += "current)";
    }
    else
    {
        params += ')';
        paramsDecl += ')';
        args += ')';
    }

    paramsAMD += "const " + getUnqualified("::Ice::Current&", classScope) + " " + currentParam + " = " +
        getUnqualified("::Ice::emptyCurrent", classScope) + ")";
    argsAMD += "current)";

    string isConst = ((p->mode() == Operation::Nonmutating) || p->hasMetaData("cpp:const")) ? " const" : "";
    string noExcept = (cl->isLocal() && p->hasMetaData("cpp:noexcept")) ? " ICE_NOEXCEPT" : "";
    bool amd = !cl->isLocal() && (cl->hasMetaData("amd") || p->hasMetaData("amd"));

    string deprecateSymbol = getDeprecateSymbol(p, cl);

    CommentPtr comment = p->parseComment(false);
    const string cbDoc = "@param " + cbParam + " The AMD callback object for the invocation.";
    const string currentDoc = "@param " + currentParam + " The Current object for the invocation.";
    const string cookieDoc = "@param " + cookieParam + " Extra data to associate with the invocation.";
    const string returnDoc = "The asynchronous result object for the invocation.";

    H << sp;
    if(!amd)
    {
        if(comment)
        {
            StringList postParams;
            if(!cl->isLocal())
            {
                postParams.push_back(currentDoc);
            }
            writeOpDocSummary(H, p, comment, OpDocAllParams, true, StringList(), postParams, comment->returns());
        }

        H << nl << deprecateSymbol
          << "virtual " << retS << ' ' << fixKwd(name) << params << isConst << noExcept << " = 0;";
    }
    else
    {
        if(comment)
        {
            StringList preParams, postParams;
            preParams.push_back(cbDoc);
            postParams.push_back(currentDoc);
            StringList noReturns; // Leave empty - the AMD method has a void return type.
            writeOpDocSummary(H, p, comment, OpDocInParams, true, preParams, postParams, noReturns);
        }

        H << nl << deprecateSymbol
          << "virtual void " << name << "_async" << paramsAMD << isConst << noExcept << " = 0;";
    }

    if(!cl->isLocal())
    {
        H << nl << "/// \\cond INTERNAL";
        H << nl << "bool _iceD_" << name << "(::IceInternal::Incoming&, const " << getUnqualified("::Ice::Current&", scope)
          << ")" << isConst << ';';
        H << nl << "/// \\endcond";

        C << sp;
        //
        // inS, ret, current etc. may shadow class-with-operations data members in C++98
        //
        C << nl << "/// \\cond INTERNAL";
        C << nl << "bool" << nl << scope.substr(2) << "_iceD_" << name << "(::IceInternal::Incoming& inS"
          << ", const " << getUnqualified("::Ice::Current&", classScope) << " current)" << isConst;
        C << sb;
        C << nl << "_iceCheckMode(" << operationModeToString(p->mode()) << ", current.mode);";

        if(!inParams.empty())
        {
            C << nl << getUnqualified("::Ice::InputStream*", classScope) << " istr = inS.startReadParams();";
            writeAllocateCode(C, inParams, 0, true, classScope, _useWstring | TypeContextInParam);
            writeUnmarshalCode(C, inParams, 0, true, TypeContextInParam);
            if(p->sendsClasses(false))
            {
                C << nl << "istr->readPendingValues();";
            }
            C << nl << "inS.endReadParams();";
        }
        else
        {
            C << nl << "inS.readEmptyParams();";
        }
        if(p->format() != DefaultFormat)
        {
            C << nl << "inS.setFormat(" << opFormatTypeToString(p, false) << ");";
        }

        if(!amd)
        {
            writeAllocateCode(C, outParams, 0, true, classScope, _useWstring, "ret");
            C << nl;
            if(ret)
            {
                C << retS << " ret = ";
            }
            C << "this->" << fixKwd(name) << args << ';';
            if(ret || !outParams.empty())
            {
                C << nl << getUnqualified("::Ice::OutputStream*", classScope) << " ostr = inS.startWriteParams();";
                writeMarshalCode(C, outParams, p, true, 0);
                if(p->returnsClasses(false))
                {
                    C << nl << "ostr->writePendingValues();";
                }
                C << nl << "inS.endWriteParams();";
            }
            else
            {
                C << nl << "inS.writeEmptyParams();";
            }
            C << nl << "return true;";
        }
        else
        {
            C << nl << "this->" << name << "_async" << argsAMD << ';';
            C << nl << "return false;";
        }
        C << eb;
        C << nl << "/// \\endcond";
    }

    if(cl->isLocal() && (cl->hasMetaData("async-oneway") || p->hasMetaData("async-oneway")))
    {
        vector<string> paramsDeclAMI;
        vector<string> outParamsDeclAMI;

        for(ParamDeclList::const_iterator r = paramList.begin(); r != paramList.end(); ++r)
        {
            string paramName = fixKwd((*r)->name());

            StringList metaData = (*r)->getMetaData();
            string typeString;
            if((*r)->isOutParam())
            {
                typeString = outputTypeToString((*r)->type(), (*r)->optional(), classScope, metaData,
                                                _useWstring | TypeContextAMIEnd);
            }
            else
            {
                typeString = inputTypeToString((*r)->type(), (*r)->optional(), classScope, metaData, _useWstring);
            }

            if(!(*r)->isOutParam())
            {
                paramsDeclAMI.push_back(typeString + ' ' + paramName);
            }
            else
            {
                outParamsDeclAMI.push_back(typeString + ' ' + paramName);
            }
        }

        H << sp;
        if(comment)
        {
            StringList returns;
            returns.push_back(returnDoc);
            writeOpDocSummary(H, p, comment, OpDocInParams, false, StringList(), StringList(), returns);
        }
        H << nl << "virtual " << getUnqualified("::Ice::AsyncResultPtr", classScope) << " begin_" << name << spar
          << paramsDeclAMI << epar << " = 0;";

        H << sp;
        if(comment)
        {
            StringList postParams, returns;
            postParams.push_back("@param " + cbParam + " Callback to be invoked when the invocation completes");
            postParams.push_back(cookieDoc);
            returns.push_back(returnDoc);
            writeOpDocSummary(H, p, comment, OpDocInParams, false, StringList(), postParams, returns);
        }
        H << nl << "virtual " << getUnqualified("::Ice::AsyncResultPtr", classScope) << " begin_" << name << spar
          << paramsDeclAMI
          << ("const " + getUnqualified("::Ice::CallbackPtr&", classScope) + " " + cbParam)
          << ("const " + getUnqualified("::Ice::LocalObjectPtr&", classScope) + " " + cookieParam + " = 0")
          << epar << " = 0;";

        string delName = "Callback_" + cl->name() + "_" + name;

        H << sp;
        if(comment)
        {
            StringList postParams, returns;
            postParams.push_back("@param " + cbParam + " Callback to be invoked when the invocation completes");
            postParams.push_back(cookieDoc);
            returns.push_back(returnDoc);
            writeOpDocSummary(H, p, comment, OpDocInParams, false, StringList(), postParams, returns);
        }
        H << nl << "virtual " << getUnqualified("::Ice::AsyncResultPtr", classScope) << " begin_" << name << spar
          << paramsDeclAMI
          << ("const " + delName + "Ptr& " + cbParam)
          << ("const " + getUnqualified("::Ice::LocalObjectPtr&", classScope) + " " + cookieParam + " = 0") << epar
          << " = 0;";

        H << sp;
        if(comment)
        {
            StringList postParams, returns;
            postParams.push_back("@param " + resultParam +
                                 " The asynchronous result object returned by the begin_ method.");
            writeOpDocSummary(H, p, comment, OpDocOutParams, true, StringList(), postParams, comment->returns());
        }
        H << nl << "virtual " << retS << " end_" << name << spar << outParamsDeclAMI
          << ("const " + getUnqualified("::Ice::AsyncResultPtr&", classScope) + " " + resultParam) << epar << " = 0;";
    }
}

void
Slice::Gen::ObjectVisitor::emitDataMember(const DataMemberPtr& p)
{
    string name = fixKwd(p->name());
    ContainerPtr container = p->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    int typeContext = cl->isLocal() ? TypeContextLocal | _useWstring : _useWstring;
    writeDocSummary(H, p);
    H << nl << typeToString(p->type(), p->optional(), fixKwd(cl->scope()), p->getMetaData(), typeContext) << ' '
      << name << ';';
}

void
Slice::Gen::ObjectVisitor::emitGCFunctions(const ClassDefPtr& p)
{
    string scoped = fixKwd(p->scoped());
    string scope = fixKwd(p->scope());
    ClassList bases = p->bases();
    ClassDefPtr base;
    if(!bases.empty() && !bases.front()->isInterface())
    {
        base = bases.front();
    }
    DataMemberList dataMembers = p->dataMembers();

    //
    // A class can potentially be part of a cycle if it (recursively) contains class
    // members.
    //
    bool canBeCyclic = p->canBeCyclic();
    bool basePreserved = p->inheritsMetaData("preserve-slice");
    bool preserved = basePreserved || p->hasMetaData("preserve-slice");

    //
    // _iceGcVisit() is overridden by the basemost class that can be
    // cyclic, plus all classes derived from that class.
    //
    // We also override these methods for the initial preserved class in a
    // hierarchy, regardless of whether the class itself is cyclic.
    //
    if(canBeCyclic || (preserved && !basePreserved))
    {
        H << nl << "/// \\cond INTERNAL";
        H << nl << "virtual void _iceGcVisitMembers(::IceInternal::GCVisitor&);";
        H << nl << "/// \\endcond";

        C << sp << nl << "void" << nl << scoped.substr(2) << "::_iceGcVisitMembers(::IceInternal::GCVisitor& v_)";
        C << sb;

        bool hasCyclicBase = base && base->canBeCyclic();
        if(hasCyclicBase || basePreserved)
        {
            emitUpcall(bases.front(), "::_iceGcVisitMembers(v_);", scope);
        }

        if(preserved && !basePreserved)
        {
            C << nl << "if(_iceSlicedData)";
            C << sb;
            C << nl << "_iceSlicedData->_iceGcVisitMembers(v_);";
            C << eb;
        }

        for(DataMemberList::const_iterator i = dataMembers.begin(); i != dataMembers.end(); ++i)
        {
            if((*i)->type()->usesClasses())
            {
                if((*i)->optional())
                {
                    C << nl << "if(" << fixKwd((*i)->name()) << ')';
                    C << sb;
                    emitGCVisitCode((*i)->type(), getDataMemberRef(*i), "", 0);
                    C << eb;
                }
                else
                {
                    emitGCVisitCode((*i)->type(), getDataMemberRef(*i), "", 0);
                }
            }
        }
        C << eb;
    }
}

void
Slice::Gen::ObjectVisitor::emitGCVisitCode(const TypePtr& p, const string& prefix, const string& name, int level)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(p);
    if((builtin &&
       (BuiltinPtr::dynamicCast(p)->kind() == Builtin::KindObject || BuiltinPtr::dynamicCast(p)->kind() == Builtin::KindValue)) ||
       ClassDeclPtr::dynamicCast(p))
    {
        C << nl << "if(" << prefix << name << ')';
        C << sb;
        ClassDeclPtr decl = ClassDeclPtr::dynamicCast(p);
        if(decl)
        {
            string scope = fixKwd(decl->scope());
            C << nl << "if((" << scope << "upCast(" << prefix << name << ".get())->_iceGcVisit(v_)))";
        }
        else
        {
            C << nl << "if((" << prefix << name << ".get())->_iceGcVisit(v_))";
        }
        C << sb;
        C << nl << prefix << name << " = 0;";
        C << eb;
        C << eb;
    }
    else if(StructPtr::dynamicCast(p))
    {
        StructPtr s = StructPtr::dynamicCast(p);
        DataMemberList dml = s->dataMembers();
        for(DataMemberList::const_iterator i = dml.begin(); i != dml.end(); ++i)
        {
            if((*i)->type()->usesClasses())
            {
                emitGCVisitCode((*i)->type(), prefix + name + ".", fixKwd((*i)->name()), ++level);
            }
        }
    }
    else if(DictionaryPtr::dynamicCast(p))
    {
        DictionaryPtr d = DictionaryPtr::dynamicCast(p);
        string scoped = fixKwd(d->scoped());
        ostringstream tmp;
        tmp << "_i" << level;
        string iterName = tmp.str();
        C << sb;
        C << nl << "for(" << scoped << "::iterator " << iterName << " = " << prefix + name
          << ".begin(); " << iterName << " != " << prefix + name << ".end(); ++" << iterName << ")";
        C << sb;
        emitGCVisitCode(d->valueType(), "", string("(*") + iterName + ").second", ++level);
        C << eb;
        C << eb;
    }
    else if(SequencePtr::dynamicCast(p))
    {
        SequencePtr s = SequencePtr::dynamicCast(p);
        string scoped = fixKwd(s->scoped());
        ostringstream tmp;
        tmp << "_i" << level;
        string iterName = tmp.str();
        C << sb;
        C << nl << "for(" << scoped << "::iterator " << iterName << " = " << prefix + name
          << ".begin(); " << iterName << " != " << prefix + name << ".end(); ++" << iterName << ")";
        C << sb;
        emitGCVisitCode(s->type(), string("(*") + iterName + ")", "", ++level);
        C << eb;
        C << eb;
    }
}

bool
Slice::Gen::ObjectVisitor::emitVirtualBaseInitializers(const ClassDefPtr& p, bool virtualInheritance, bool direct)
{
    DataMemberList allDataMembers = p->allDataMembers();
    if(allDataMembers.empty())
    {
        return false;
    }

    ClassList bases = p->bases();
    if(!bases.empty() && !bases.front()->isInterface())
    {
        if(emitVirtualBaseInitializers(bases.front(), p->hasMetaData("cpp:virtual"), false))
        {
            H << ',';
        }
    }

    //
    // Do not call non direct base classes constructor if not using virtual inheritance.
    //
    if(!direct && !virtualInheritance)
    {
        return false;
    }

    string upcall = "(";
    for(DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
    {
        if(q != allDataMembers.begin())
        {
            upcall += ", ";
        }
        upcall += fixKwd((*q)->name());
    }
    upcall += ")";

    H << nl << fixKwd(p->scoped()) << upcall;

    return true;
}

void
Slice::Gen::ObjectVisitor::emitOneShotConstructor(const ClassDefPtr& p)
{
    DataMemberList allDataMembers = p->allDataMembers();
    string scope = fixKwd(p->scope());

    if(!allDataMembers.empty())
    {
        vector<string> allParamDecls;
        map<string, CommentPtr> allComments;

        bool virtualInheritance = p->hasMetaData("cpp:virtual");
        bool callBaseConstructors = !(p->isAbstract() && virtualInheritance);
        DataMemberList dataMembers = p->dataMembers();

        int typeContext = p->isLocal() ? (_useWstring | TypeContextLocal) : _useWstring;

        for(DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
        {
            string typeName =
                inputTypeToString((*q)->type(), (*q)->optional(), scope, (*q)->getMetaData(), typeContext);
            bool dataMember = std::find(dataMembers.begin(), dataMembers.end(), (*q)) != dataMembers.end();
            allParamDecls.push_back(typeName + ((dataMember || callBaseConstructors) ?
                                                    (" " + fixKwd((*q)->name())) :
                                                    (" /*" + fixKwd((*q)->name()) + "*/")));
            CommentPtr comment = (*q)->parseComment(false);
            if(comment)
            {
                allComments[(*q)->name()] = comment;
            }
        }

        H << sp;
        H << nl << "/**";
        H << nl << " * One-shot constructor to initialize all data members.";
        for(DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
        {
            map<string, CommentPtr>::iterator r = allComments.find((*q)->name());
            if(r != allComments.end())
            {
                H << nl << " * @param " << fixKwd(r->first) << " " << getDocSentence(r->second->overview());
            }
        }
        H << nl << " */";
        H << nl;
        if(allParamDecls.size() == 1)
        {
            H << "explicit ";
        }
        H << fixKwd(p->name()) << spar << allParamDecls << epar;
        if(callBaseConstructors || !dataMembers.empty())
        {
            H << " :";
        }
        H.inc();

        ClassList bases = p->bases();
        ClassDefPtr base;

        if(!bases.empty() && !bases.front()->isInterface() && callBaseConstructors)
        {
            if(emitVirtualBaseInitializers(bases.front(), virtualInheritance, true))
            {
                if(!dataMembers.empty())
                {
                    H << ',';
                }
            }
        }

        if(!dataMembers.empty())
        {
            H << nl;
        }
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            if(q != dataMembers.begin())
            {
                H << ',' << nl;
            }
            string memberName = fixKwd((*q)->name());
            H << memberName << '(' << memberName << ')';
        }

        H.dec();
        H << sb;
        H << eb;
    }
}

void
Slice::Gen::ObjectVisitor::emitUpcall(const ClassDefPtr& base, const string& call, const string& scope)
{
    C << nl;
    if(base)
    {
        C << getUnqualified(fixKwd(base->scoped()), scope);
    }
    else
    {
        C << getUnqualified("::Ice::Object", scope);
    }
    C << call;
}

Slice::Gen::AsyncCallbackVisitor::AsyncCallbackVisitor(Output& h, Output&, const string& dllExport) :
    H(h), _dllExport(dllExport), _useWstring(false)
{
}

bool
Slice::Gen::AsyncCallbackVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalClassDefs() && !p->hasContentsWithMetaData("async-oneway"))
    {
        return false;
    }

    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    H << sp << nl << "namespace " << fixKwd(p->name())  << nl << '{';

    return true;
}

void
Slice::Gen::AsyncCallbackVisitor::visitModuleEnd(const ModulePtr&)
{
    _useWstring = resetUseWstring(_useWstringHist);

    H << sp << nl << '}';
}

bool
Slice::Gen::AsyncCallbackVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);
    return true;
}

void
Slice::Gen::AsyncCallbackVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _useWstring = resetUseWstring(_useWstringHist);
}

void
Slice::Gen::AsyncCallbackVisitor::visitOperation(const OperationPtr& p)
{
    ClassDefPtr cl = ClassDefPtr::dynamicCast(p->container());

    if(cl->isLocal() && !(cl->hasMetaData("async-oneway") || p->hasMetaData("async-oneway")))
    {
        return;
    }

    //
    // Write the callback base class and callback smart pointer.
    //
    string delName = "Callback_" + cl->name() + "_" + p->name();
    H << sp;
    H << nl << "/**";
    H << nl << " * Base class for asynchronous callback wrapper classes used for calls to";
    H << nl << " * IceProxy" << fixKwd(cl->scoped()) << "::begin_" << p->name() << ".";
    H << nl << " * Create a wrapper instance by calling " << fixKwd(cl->scope()) << "new" << delName << ".";
    H << nl << " */";
    H << nl << "class " << delName << "_Base : public virtual ::IceInternal::CallbackBase { };";
    H << nl << "typedef ::IceUtil::Handle< " << delName << "_Base> " << delName << "Ptr;";
}

Slice::Gen::AsyncCallbackTemplateVisitor::AsyncCallbackTemplateVisitor(Output& h, Output&, const string& dllExport)
    : H(h), _dllExport(dllExport), _useWstring(false)
{
}

bool
Slice::Gen::AsyncCallbackTemplateVisitor::visitUnitStart(const UnitPtr& p)
{
    return p->hasNonLocalClassDefs();
}

void
Slice::Gen::AsyncCallbackTemplateVisitor::visitUnitEnd(const UnitPtr&)
{
}

bool
Slice::Gen::AsyncCallbackTemplateVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalClassDefs())
    {
        return false;
    }

    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    H << sp << nl << "namespace " << fixKwd(p->name())  << nl << '{';
    return true;
}

void
Slice::Gen::AsyncCallbackTemplateVisitor::visitModuleEnd(const ModulePtr&)
{
    _useWstring = resetUseWstring(_useWstringHist);
    H << sp << nl << '}';
}

bool
Slice::Gen::AsyncCallbackTemplateVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);
    return true;
}

void
Slice::Gen::AsyncCallbackTemplateVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _useWstring = resetUseWstring(_useWstringHist);
}

void
Slice::Gen::AsyncCallbackTemplateVisitor::visitOperation(const OperationPtr& p)
{
    generateOperation(p, false);
    generateOperation(p, true);
}

void
Slice::Gen::AsyncCallbackTemplateVisitor::generateOperation(const OperationPtr& p, bool withCookie)
{
    ClassDefPtr cl = ClassDefPtr::dynamicCast(p->container());
    if(cl->isLocal() || cl->operations().empty())
    {
        return;
    }

    string clName = cl->name();
    string clScope = fixKwd(cl->scope());
    string delName = "Callback_" + clName + "_" + p->name();
    string delTmplName = (withCookie ? "Callback_" : "CallbackNC_") + clName + "_" + p->name();

    TypePtr ret = p->returnType();
    string retS = inputTypeToString(ret, p->returnIsOptional(), clScope, p->getMetaData(), _useWstring);
    string retEndArg = getEndArg(ret, p->getMetaData(), "ret");

    ParamDeclList outParams;
    vector<string> outArgs;
    vector<string> outDecls;
    vector<string> outDeclsEnd;
    vector<string> outEndArgs;

    ParamDeclList paramList = p->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if((*q)->isOutParam())
        {
            outParams.push_back(*q);
            outArgs.push_back("iceP_" + (*q)->name());
            outEndArgs.push_back(getEndArg((*q)->type(), (*q)->getMetaData(), outArgs.back()));
            outDecls.push_back(
                inputTypeToString((*q)->type(), (*q)->optional(), clScope, (*q)->getMetaData(), _useWstring));
        }
    }

    H << sp;
    string baseD;
    string inheritD;
    if(withCookie)
    {
        baseD = "::IceInternal::Callback<T, CT>";
        H << nl << "/**";
        H << nl << " * Type-safe asynchronous callback wrapper class with cookie support used for calls to";
        H << nl << " * IceProxy" << fixKwd(cl->scoped()) << "::begin_" << p->name() << ".";
        H << nl << " * Create a wrapper instance by calling " << fixKwd(cl->scope()) << "new" << delName << ".";
        H << nl << " */";
        H << nl << "template<class T, typename CT>";
        inheritD = p->returnsData() ? "::IceInternal::TwowayCallback<T, CT>" : "::IceInternal::OnewayCallback<T, CT>";
    }
    else
    {
        baseD = "::IceInternal::CallbackNC<T>";
        H << nl << "/**";
        H << nl << " * Type-safe asynchronous callback wrapper class used for calls to";
        H << nl << " * IceProxy" << fixKwd(cl->scoped()) << "::begin_" << p->name() << ".";
        H << nl << " * Create a wrapper instance by calling " << fixKwd(cl->scope()) << "new" << delName << ".";
        H << nl << " */";
        H << nl << "template<class T>";
        inheritD = p->returnsData() ? "::IceInternal::TwowayCallbackNC<T>" : "::IceInternal::OnewayCallbackNC<T>";
    }

    H << nl << "class " << delTmplName << " : public " << delName << "_Base, public " << inheritD;
    H << sb;
    H.dec();
    H << nl << "public:";
    H.inc();

    H << sp << nl << "typedef IceUtil::Handle<T> TPtr;";

    string cookieT;
    string comCookieT;
    if(withCookie)
    {
        cookieT = "const CT&";
        comCookieT = " , const CT&";
    }

    H << sp << nl << "typedef void (T::*Exception)(const ::Ice::Exception&" << comCookieT << ");";
    H << nl << "typedef void (T::*Sent)(bool" << comCookieT << ");";
    if(p->returnsData())
    {
        //
        // typedefs for callbacks.
        //
        H << nl << "typedef void (T::*Response)" << spar;
        if(ret)
        {
            H << retS;
        }
        H << outDecls;
        if(withCookie)
        {
            H << cookieT;
        }
        H << epar << ';';
    }
    else
    {
        H << nl << "typedef void (T::*Response)(" << cookieT << ");";
    }

    //
    // constructor.
    //
    H << sp;
    H << nl << delTmplName << spar << "const TPtr& obj";
    H << "Response cb";
    H << "Exception excb";
    H << "Sent sentcb" << epar;
    H.inc();
    if(p->returnsData())
    {
        H << nl << ": " << inheritD + "(obj, cb != 0, excb, sentcb), _response(cb)";
    }
    else
    {
        H << nl << ": " << inheritD + "(obj, cb, excb, sentcb)";
    }
    H.dec();
    H << sb;
    H << eb;

    if(p->returnsData())
    {
        //
        // completed.
        //
        H << sp;
        H << nl << "/// \\cond INTERNAL";
        H << nl << "virtual void completed(const " << getUnqualified("::Ice::AsyncResultPtr&", clScope)
          << " result) const";
        H << sb;
        H << nl << clName << "Prx proxy = " << clName << "Prx::uncheckedCast(result->getProxy());";
        writeAllocateCode(H, outParams, p, true, clScope,
                          _useWstring | TypeContextInParam | TypeContextAMICallPrivateEnd);
        H << nl << "try";
        H << sb;
        H << nl;
        if(!usePrivateEnd(p))
        {
            if(ret)
            {
                H << retEndArg << " = ";
            }
            H << "proxy->end_" << p->name() << spar << outEndArgs << "result" << epar << ';';
        }
        else
        {
            H << "proxy->_iceI_end_" << p->name() << spar << outEndArgs;
            if(ret)
            {
                H << retEndArg;
            }
            H << "result" << epar << ';';
        }
        writeEndCode(H, outParams, p, true);
        H << eb;
        H << nl << "catch(const ::Ice::Exception& ex)";
        H << sb;

        H << nl << "" << baseD << "::exception(result, ex);";
        H << nl << "return;";
        H << eb;
        H << nl << "if(_response)";
        H << sb;
        H << nl << "(" << baseD << "::_callback.get()->*_response)" << spar;
        if(ret)
        {
            H << "ret";
        }
        H << outArgs;
        if(withCookie)
        {
            H << "CT::dynamicCast(result->getCookie())";
        }
        H << epar << ';';
        H << eb;
        H << eb;
        H << nl << "/// \\endcond";
        H.dec();
        H << sp << nl << "private:";
        H.inc();
        H << sp << nl << "Response _response;";
    }
    H << eb << ';';

    // Factory method
    for(int i = 0; i < 2; i++)
    {
        string callbackT = i == 0 ? "const IceUtil::Handle<T>&" : "T*";

        H << sp;
        H << nl << "/**";
        H << nl << " * Creates a callback wrapper instance that delegates to your object.";
        if(withCookie)
        {
            H << nl << " * Use this overload when your callback methods receive a cookie value.";
        }
        H << nl << " * @param instance The callback object.";
        H << nl << " * @param cb The success method of the callback object.";
        H << nl << " * @param excb The exception method of the callback object.";
        H << nl << " * @param sentcb The sent method of the callback object.";
        H << nl << " * @return An object that can be passed to an asynchronous invocation of IceProxy"
          << clScope << clName << "::begin_" << p->name() << ".";
        H << nl << " */";
        if(withCookie)
        {
            cookieT = "const CT&";
            comCookieT = ", const CT&";
            H << nl << "template<class T, typename CT> " << delName << "Ptr";
        }
        else
        {
            H << nl << "template<class T> " << delName << "Ptr";
        }

        H << nl << "new" << delName << "(" << callbackT << " instance, ";
        if(p->returnsData())
        {
            H  << "void (T::*cb)" << spar;
            if(ret)
            {
                H << retS;
            }
            H << outDecls;
            if(withCookie)
            {
                H << cookieT;
            }
            H << epar << ", ";
        }
        else
        {
            H  << "void (T::*cb)(" << cookieT << "), ";
        }
        H << "void (T::*excb)(" << "const ::Ice::Exception&" << comCookieT << "), ";
        H << "void (T::*sentcb)(bool" << comCookieT << ") = 0)";
        H << sb;
        if(withCookie)
        {
            H << nl << "return new " << delTmplName << "<T, CT>(instance, cb, excb, sentcb);";
        }
        else
        {
            H << nl << "return new " << delTmplName << "<T>(instance, cb, excb, sentcb);";
        }
        H << eb;

        if(!ret && outParams.empty())
        {
            H << sp;
            H << nl << "/**";
            H << nl << " * Creates a callback wrapper instance that delegates to your object.";
            if(withCookie)
            {
                H << nl << " * Use this overload when your callback methods receive a cookie value.";
            }
            H << nl << " * @param instance The callback object.";
            H << nl << " * @param excb The exception method of the callback object.";
            H << nl << " * @param sentcb The sent method of the callback object.";
            H << nl << " * @return An object that can be passed to an asynchronous invocation of IceProxy"
              << clScope << clName << "::begin_" << p->name() << ".";
            H << nl << " */";
            if(withCookie)
            {
                H << nl << "template<class T, typename CT> " << delName << "Ptr";
            }
            else
            {
                H << nl << "template<class T> " << delName << "Ptr";
            }
            H << nl << "new" << delName << "(" << callbackT << " instance, ";
            H << "void (T::*excb)(" << "const ::Ice::Exception&" << comCookieT << "), ";
            H << "void (T::*sentcb)(bool" << comCookieT << ") = 0)";
            H << sb;
            if(withCookie)
            {
                H << nl << "return new " << delTmplName << "<T, CT>(instance, 0, excb, sentcb);";
            }
            else
            {
                H << nl << "return new " << delTmplName << "<T>(instance, 0, excb, sentcb);";
            }
            H << eb;
        }
    }
}

Slice::Gen::ImplVisitor::ImplVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport), _useWstring(false)
{
}

string
Slice::Gen::ImplVisitor::defaultValue(const TypePtr& type, const string& scope, const StringList& metaData) const
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindBool:
            {
                return "false";
            }
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindInt:
            case Builtin::KindLong:
            {
                return "0";
            }
            case Builtin::KindFloat:
            case Builtin::KindDouble:
            {
                return "0.0";
            }
            case Builtin::KindString:
            {
                return "::std::string()";
            }
            case Builtin::KindValue:
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindLocalObject:
            {
                return "0";
            }
        }
    }
    else
    {
        ProxyPtr prx = ProxyPtr::dynamicCast(type);

        if(ProxyPtr::dynamicCast(type) || ClassDeclPtr::dynamicCast(type))
        {
            return "0";
        }

        StructPtr st = StructPtr::dynamicCast(type);
        if(st)
        {
            return getUnqualified(fixKwd(st->scoped()), scope) + "()";
        }

        EnumPtr en = EnumPtr::dynamicCast(type);
        if(en)
        {
            EnumeratorList enumerators = en->enumerators();
            return getUnqualified(fixKwd(en->scope() + enumerators.front()->name()), scope);
        }

        SequencePtr seq = SequencePtr::dynamicCast(type);
        if(seq)
        {
            return typeToString(seq, scope, metaData, _useWstring | TypeContextCpp11) + "()";
        }

        DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
        if(dict)
        {
            return getUnqualified(fixKwd(dict->scoped()), scope) + "()";
        }
    }

    assert(false);
    return "???";
}

bool
Slice::Gen::ImplVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasClassDefs())
    {
        return false;
    }
    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);
    H << sp << nl << "namespace " << fixKwd(p->name()) << nl << '{';
    return true;
}

void
Slice::Gen::ImplVisitor::visitModuleEnd(const ModulePtr&)
{
    H << sp;
    H << nl << '}';

    _useWstring = resetUseWstring(_useWstringHist);
}

bool
Slice::Gen::ImplVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(!p->isAbstract())
    {
        return false;
    }

    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    string name = p->name();
    string scope = fixKwd(p->scope());
    string cls = scope.substr(2) + name + "I";
    ClassList bases = p->bases();

    H << sp;
    H << nl << "class " << name << "I : public virtual " << fixKwd(name);

    H << sb;
    H.dec();
    H << nl << "public:";
    H.inc();

    OperationList ops = p->allOperations();

    for(OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = (*r);
        string opName = op->name();
        string isConst = ((op->mode() == Operation::Nonmutating) || op->hasMetaData("cpp:const")) ? " const" : "";

        string classScopedAMD = "AMD_" + ClassDefPtr::dynamicCast(op->container())->name();

        TypePtr ret = op->returnType();
        string retS = returnTypeToString(ret, op->returnIsOptional(), "", op->getMetaData(), _useWstring);

        if(!p->isLocal() && (p->hasMetaData("amd") || op->hasMetaData("amd")))
        {
            H << sp << nl << "virtual void " << opName << "_async(";
            H.useCurrentPosAsIndent();
            H << "const " << classScopedAMD << '_' << opName << "Ptr&";
            ParamDeclList paramList = op->parameters();

            for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
            {
                if(!(*q)->isOutParam())
                {
                    H << ',' << nl << inputTypeToString((*q)->type(), (*q)->optional(), scope, (*q)->getMetaData(),
                                                        _useWstring);
                }
            }
            H << ',' << nl << "const Ice::Current&";
            H.restoreIndent();
            H << ")" << isConst << ';';

            C << sp << nl << "void" << nl << scope << name << "I::" << opName << "_async(";
            C.useCurrentPosAsIndent();
            C << "const " << classScopedAMD << '_' << opName << "Ptr& " << opName << "CB";
            for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
            {
                if(!(*q)->isOutParam())
                {
                    C << ',' << nl << inputTypeToString((*q)->type(), (*q)->optional(), scope, (*q)->getMetaData(),
                                                        _useWstring) << ' ' << fixKwd((*q)->name());
                }
            }
            C << ',' << nl << "const Ice::Current& current";
            C.restoreIndent();
            C << ")" << isConst;
            C << sb;

            string result = "r";
            for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
            {
                if((*q)->name() == result)
                {
                    result = "_" + result;
                    break;
                }
            }

            C << nl << opName << "CB->ice_response(";
            if(ret)
            {
                C << defaultValue(ret, scope, op->getMetaData());
            }
            for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
            {
                if((*q)->isOutParam())
                {
                    if(ret || q != paramList.begin())
                    {
                        C << ", ";
                    }
                    C << defaultValue((*q)->type(), scope, op->getMetaData());
                }
            }
            C << ");";

            C << eb;
        }
        else
        {
            H << sp << nl << "virtual " << getUnqualified(retS, scope) << ' ' << fixKwd(opName) << '(';
            H.useCurrentPosAsIndent();
            ParamDeclList paramList = op->parameters();
            for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
            {
                if(q != paramList.begin())
                {
                    H << ',' << nl;
                }
                if((*q)->isOutParam())
                {
                    H << outputTypeToString((*q)->type(), (*q)->optional(), scope, (*q)->getMetaData(), _useWstring);
                }
                else
                {
                    H << inputTypeToString((*q)->type(), (*q)->optional(), scope, (*q)->getMetaData(), _useWstring);
                }
            }
            if(!p->isLocal())
            {
                if(!paramList.empty())
                {
                    H << ',' << nl;
                }
                H << "const Ice::Current&";
            }
            H.restoreIndent();

            H << ")" << isConst << ';';

            C << sp << nl << retS << nl;
            C << scope.substr(2) << name << "I::" << fixKwd(opName) << '(';
            C.useCurrentPosAsIndent();
            for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
            {
                if(q != paramList.begin())
                {
                    C << ',' << nl;
                }
                if((*q)->isOutParam())
                {
                    C << outputTypeToString((*q)->type(), (*q)->optional(), scope, (*q)->getMetaData(), _useWstring) << " "
                      << fixKwd((*q)->name());
                }
                else
                {
                    C << inputTypeToString((*q)->type(), (*q)->optional(), scope, (*q)->getMetaData(), _useWstring) << " /*"
                      << fixKwd((*q)->name()) << "*/";
                }
            }
            if(!p->isLocal())
            {
                if(!paramList.empty())
                {
                    C << ',' << nl;
                }
                C << "const Ice::Current& current";
            }
            C.restoreIndent();
            C << ')';
            C << isConst;
            C << sb;

            if(ret)
            {
                C << nl << "return " << defaultValue(ret, scope, op->getMetaData()) << ";";
            }

            C << eb;
        }
    }

    H << eb << ';';

    _useWstring = resetUseWstring(_useWstringHist);

    return true;
}

Slice::Gen::AsyncVisitor::AsyncVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport), _useWstring(false)
{
}

bool
Slice::Gen::AsyncVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalClassDecls() || !p->hasContentsWithMetaData("amd"))
    {
        return false;
    }

    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    string name = fixKwd(p->name());

    H << sp << nl << "namespace " << name << nl << '{';

    return true;
}

void
Slice::Gen::AsyncVisitor::visitModuleEnd(const ModulePtr&)
{
    H << sp << nl << '}';

    _useWstring = resetUseWstring(_useWstringHist);
}

bool
Slice::Gen::AsyncVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);
    return true;
}

void
Slice::Gen::AsyncVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _useWstring = resetUseWstring(_useWstringHist);
}

void
Slice::Gen::AsyncVisitor::visitOperation(const OperationPtr& p)
{
    ContainerPtr container = p->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);

    if(cl->isLocal() || (!cl->hasMetaData("amd") && !p->hasMetaData("amd")))
    {
        return;
    }

    string name = p->name();

    string className = cl->name();
    string classNameAMD = "AMD_" + className;
    string classScope = fixKwd(cl->scope());
    string classScopedAMD = classScope + classNameAMD;

    TypePtr ret = p->returnType();
    string retS = inputTypeToString(ret, p->returnIsOptional(), classScope, p->getMetaData(), _useWstring);

    string resultParam = "result";
    ParamDeclList paramList = p->outParameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if((*q)->name() == "result")
        {
            resultParam = "result_";
            break;
        }
    }

    vector<string> paramsAMD;

    if(ret)
    {
        paramsAMD.push_back(inputTypeToString(ret, p->returnIsOptional(), classScope, p->getMetaData(), _useWstring) +
                            " " + resultParam);
    }

    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        string paramName = fixKwd((*q)->name());
        TypePtr type = (*q)->type();
        string typeString = inputTypeToString(type, (*q)->optional(), classScope, (*q)->getMetaData(), _useWstring);
        paramsAMD.push_back(typeString + " " + paramName);
    }

    string cbName = classNameAMD + '_' + name;

    CommentPtr comment = p->parseComment(false);

    H << sp;
    H << nl << "/**";
    H << nl << " * AMD callback class for " << fixKwd(p->scoped()).substr(2) << "_async.";
    H << nl << " * Call the ice_response method for a successful completion, or the ice_exception";
    H << nl << " * method in the case of an error.";
    H << nl << " */";
    H << nl << "class " << _dllExport << cbName << " : public virtual "
      << getUnqualified("::Ice::AMDCallback", classScope);
    H << sb;
    H.dec();
    H << nl << "public:";
    H.inc();

    // Out of line dtor to avoid weak vtable
    H << sp << nl << "virtual ~" << cbName << "();";
    C << sp;
    C << nl << classScope.substr(2) << cbName << "::~" << cbName << "()";
    C << sb;
    C << eb;

    H << sp;
    H << nl << "/**";
    H << nl << " * Call ice_response for a successful completion.";
    if(comment)
    {
        StringList preParams;
        StringList returns = comment->returns();
        if(ret && !returns.empty())
        {
            preParams = returns;
            preParams.pop_front();
            preParams.push_front("@param " + resultParam + " " + returns.front());
        }
        writeOpDocParams(H, p, comment, OpDocOutParams, preParams);
    }
    H << nl << " */";
    H << nl << "virtual void ice_response" << spar << paramsAMD << epar << " = 0;";
    H << eb << ';';
    H << sp << nl << "typedef ::IceUtil::Handle< " << classScopedAMD << '_' << name << "> "
      << classNameAMD << '_' << name  << "Ptr;";
}

Slice::Gen::AsyncImplVisitor::AsyncImplVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport), _useWstring(false)
{
}

bool
Slice::Gen::AsyncImplVisitor::visitUnitStart(const UnitPtr& p)
{
    if(!p->hasNonLocalClassDecls() || !p->hasContentsWithMetaData("amd"))
    {
        return false;
    }

    H << sp;
    H << nl << "/// \\cond INTERNAL";
    H << nl << "namespace IceAsync" << nl << '{';

    return true;
}

void
Slice::Gen::AsyncImplVisitor::visitUnitEnd(const UnitPtr&)
{
    H << sp << nl << '}';
    H << nl << "/// \\endcond";
}

bool
Slice::Gen::AsyncImplVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalClassDecls() || !p->hasContentsWithMetaData("amd"))
    {
        return false;
    }

    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    string name = fixKwd(p->name());

    H << sp << nl << "namespace " << name << nl << '{';

    return true;
}

void
Slice::Gen::AsyncImplVisitor::visitModuleEnd(const ModulePtr&)
{
    H << sp << nl << '}';

    _useWstring = resetUseWstring(_useWstringHist);
}

bool
Slice::Gen::AsyncImplVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);
    return true;
}

void
Slice::Gen::AsyncImplVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _useWstring = resetUseWstring(_useWstringHist);
}

void
Slice::Gen::AsyncImplVisitor::visitOperation(const OperationPtr& p)
{
    ContainerPtr container = p->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);

    if(cl->isLocal() || (!cl->hasMetaData("amd") && !p->hasMetaData("amd")))
    {
        return;
    }

    string name = p->name();

    string classNameAMD = "AMD_" + cl->name();
    string classScope = fixKwd(cl->scope());
    string classScopedAMD = classScope + classNameAMD;

    string params;
    string paramsDecl;
    string args;

    TypePtr ret = p->returnType();
    string retS = inputTypeToString(ret, p->returnIsOptional(), "", p->getMetaData(), _useWstring);

    if(ret)
    {
        params += retS;
        paramsDecl += retS;
        paramsDecl += ' ';
        paramsDecl += "ret";
        args += "ret";
    }

    ParamDeclList outParams;
    ParamDeclList paramList = p->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if((*q)->isOutParam())
        {
            string paramName = fixKwd((*q)->name());
            TypePtr type = (*q)->type();
            string typeString = inputTypeToString(type, (*q)->optional(), "", (*q)->getMetaData(), _useWstring);

            if(ret || !outParams.empty())
            {
                params += ", ";
                paramsDecl += ", ";
                args += ", ";
            }

            params += typeString;
            paramsDecl += typeString;
            paramsDecl += ' ';
            paramsDecl += paramName;
            args += paramName;

            outParams.push_back(*q);
        }
    }

    H.zeroIndent();
    H << sp;
    H << nl << "#if defined(_MSC_VER) && (_MSC_VER >= 1900)";
    H << nl << "#   pragma warning(push)";
    H << nl << "#   pragma warning(disable:4239)";
    H << nl << "#endif";
    H.restoreIndent();

    H << sp << nl << "class " << _dllExport << classNameAMD << '_' << name
      << " : public " << classScopedAMD  << '_' << name << ", public ::IceInternal::IncomingAsync";
    H << sb;
    H.dec();
    H << nl << "public:";
    H.inc();

    H << sp;
    H << nl << classNameAMD << '_' << name << "(::IceInternal::Incoming&);";

    H << sp;
    H << nl << "virtual void ice_response(" << params << ");";
    H << eb << ';';

    H.zeroIndent();
    H << sp;
    H << nl << "#if defined(_MSC_VER) && (_MSC_VER >= 1900)";
    H << nl << "#   pragma warning(pop)";
    H << nl << "#endif";
    H.restoreIndent();

    C << sp;
    C << nl << "/// \\cond INTERNAL";
    C << nl << "IceAsync" << classScopedAMD << '_' << name << "::" << classNameAMD << '_' << name
      << "(::IceInternal::Incoming& in) :";
    C.inc();
    C << nl << "::IceInternal::IncomingAsync(in)";
    C.dec();
    C << sb;
    C << eb;

    C << sp << nl << "void";
    C << nl << "IceAsync" << classScopedAMD << '_' << name << "::ice_response(" << paramsDecl << ')';
    C << sb;
    if(ret || !outParams.empty())
    {
        C << nl << "::Ice::OutputStream* ostr = startWriteParams();";
        writeMarshalCode(C, outParams, p, false, TypeContextInParam);
        if(p->returnsClasses(false))
        {
            C << nl << "ostr->writePendingValues();";
        }
        C << nl << "endWriteParams();";
    }
    else
    {
        C << nl << "writeEmptyParams();";
    }
    C << nl << "completed();";
    C << eb;
    C << nl << "/// \\endcond";
}

Slice::Gen::StreamVisitor::StreamVisitor(Output& h, Output& c, const string& dllExport) :
    H(h),
    C(c),
    _dllExport(dllExport)
{
}

bool
Slice::Gen::StreamVisitor::visitModuleStart(const ModulePtr& m)
{
    if(!m->hasNonLocalContained(Contained::ContainedTypeStruct) &&
       !m->hasNonLocalContained(Contained::ContainedTypeEnum) &&
       !m->hasNonLocalContained(Contained::ContainedTypeClass) &&
       !m->hasNonLocalContained(Contained::ContainedTypeException))
    {
        return false;
    }

    if(UnitPtr::dynamicCast(m->container()))
    {
        //
        // Only emit this for the top-level module.
        //
        H << sp;
        H << nl << "/// \\cond STREAM";
        H << nl << "namespace Ice" << nl << '{' << sp;

        C << sp;
        C << nl << "namespace Ice" << nl << '{';
    }

    return true;
}

void
Slice::Gen::StreamVisitor::visitModuleEnd(const ModulePtr& m)
{
    if(UnitPtr::dynamicCast(m->container()))
    {
        //
        // Only emit this for the top-level module.
        //
        H << nl << '}';
        H << nl << "/// \\endcond";
        C << nl << '}';
    }
}

bool
Slice::Gen::StreamVisitor::visitClassDefStart(const ClassDefPtr& c)
{
    if(!c->isLocal())
    {
        writeStreamHelpers(H, c, c->dataMembers(), c->hasBaseDataMembers(), true, false);
    }
    return false;
}

bool
Slice::Gen::StreamVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    if(!p->isLocal())
    {
        string scoped = p->scoped();
        H << nl << "template<>";
        H << nl << "struct StreamableTraits< " << fixKwd(scoped) << ">";
        H << sb;
        H << nl << "static const StreamHelperCategory helper = StreamHelperCategoryUserException;";
        H << eb << ";" << nl;

        writeStreamHelpers(H, p, p->dataMembers(), p->hasBaseDataMembers(), true, false);
    }
    return false;
}

bool
Slice::Gen::StreamVisitor::visitStructStart(const StructPtr& p)
{
    if(!p->isLocal())
    {
        bool classMetaData = findMetaData(p->getMetaData(), false) == "%class";
        string scoped = p->scoped();

        string fullStructName = classMetaData ? fixKwd(scoped + "Ptr") : fixKwd(scoped);

        H << nl << "template<>";

        H << nl << "struct StreamableTraits< " << fullStructName << ">";

        H << sb;
        if(classMetaData)
        {
            H << nl << "static const StreamHelperCategory helper = StreamHelperCategoryStructClass;";
        }
        else
        {
            H << nl << "static const StreamHelperCategory helper = StreamHelperCategoryStruct;";
        }
        H << nl << "static const int minWireSize = " << p->minWireSize() << ";";
        if(p->isVariableLength())
        {
            H << nl << "static const bool fixedLength = false;";
        }
        else
        {
            H << nl << "static const bool fixedLength = true;";
        }
        H << eb << ";" << nl;

        writeStreamHelpers(H, p, p->dataMembers(), false, true, false);
    }
    return false;
}

void
Slice::Gen::StreamVisitor::visitEnum(const EnumPtr& p)
{
    if(!p->isLocal())
    {
        string scoped = fixKwd(p->scoped());
        H << nl << "template<>";
        H << nl << "struct StreamableTraits< " << scoped << ">";
        H << sb;
        H << nl << "static const StreamHelperCategory helper = StreamHelperCategoryEnum;";
        H << nl << "static const int minValue = " << p->minValue() << ";";
        H << nl << "static const int maxValue = " << p->maxValue() << ";";
        H << nl << "static const int minWireSize = " << p->minWireSize() << ";";
        H << nl << "static const bool fixedLength = false;";
        H << eb << ";" << nl;
    }
}

void
Slice::Gen::validateMetaData(const UnitPtr& u)
{
    MetaDataVisitor visitor;
    u->visit(&visitor, false);
}

bool
Slice::Gen::MetaDataVisitor::visitUnitStart(const UnitPtr& p)
{
    static const string prefix = "cpp:";

    //
    // Validate global metadata in the top-level file and all included files.
    // Note that these metadata can only be cpp:, never cpp98: or cpp11:
    //
    StringList files = p->allFiles();

    for(StringList::iterator q = files.begin(); q != files.end(); ++q)
    {
        string file = *q;
        DefinitionContextPtr dc = p->findDefinitionContext(file);
        assert(dc);
        StringList globalMetaData = dc->getMetaData();
        int headerExtension = 0;
        int sourceExtension = 0;
        int dllExport = 0;
        for(StringList::const_iterator r = globalMetaData.begin(); r != globalMetaData.end();)
        {
            string s = *r++;
            if(s.find(prefix) == 0)
            {
                static const string cppIncludePrefix = "cpp:include:";
                static const string cppHeaderExtPrefix = "cpp:header-ext:";
                static const string cppSourceExtPrefix = "cpp:source-ext:";
                static const string cppDllExportPrefix = "cpp:dll-export:";
                static const string cppDoxygenIncludePrefix = "cpp:doxygen:include:";

                if(s.find(cppIncludePrefix) == 0 && s.size() > cppIncludePrefix.size())
                {
                    continue;
                }
                else if(s.find(cppHeaderExtPrefix) == 0 && s.size() > cppHeaderExtPrefix.size())
                {
                    headerExtension++;
                    if(headerExtension > 1)
                    {
                        ostringstream ostr;
                        ostr << "ignoring invalid global metadata `" << s
                             << "': directive can appear only once per file";
                        dc->warning(InvalidMetaData, file, -1, ostr.str());
                        globalMetaData.remove(s);
                    }
                    continue;
                }
                else if(s.find(cppSourceExtPrefix) == 0 && s.size() > cppSourceExtPrefix.size())
                {
                    sourceExtension++;
                    if(sourceExtension > 1)
                    {
                        ostringstream ostr;
                        ostr << "ignoring invalid global metadata `" << s
                             << "': directive can appear only once per file";
                        dc->warning(InvalidMetaData, file, -1, ostr.str());
                        globalMetaData.remove(s);
                    }
                    continue;
                }
                else if(s.find(cppDllExportPrefix) == 0 && s.size() > cppDllExportPrefix.size())
                {
                    dllExport++;
                    if(dllExport > 1)
                    {
                        ostringstream ostr;
                        ostr << "ignoring invalid global metadata `" << s
                             << "': directive can appear only once per file";
                        dc->warning(InvalidMetaData, file, -1, ostr.str());

                        globalMetaData.remove(s);
                    }
                    continue;
                }
                else if(s.find(cppDoxygenIncludePrefix) == 0 && s.size() > cppDoxygenIncludePrefix.size())
                {
                    continue;
                }

                ostringstream ostr;
                ostr << "ignoring invalid global metadata `" << s << "'";
                dc->warning(InvalidMetaData, file, -1, ostr.str());
                globalMetaData.remove(s);
            }

        }
        dc->setMetaData(globalMetaData);
    }

    return true;
}

bool
Slice::Gen::MetaDataVisitor::visitModuleStart(const ModulePtr& p)
{
    StringList metaData = validate(p, p->getMetaData(), p->file(), p->line());
    p->setMetaData(metaData);
    return true;
}

void
Slice::Gen::MetaDataVisitor::visitModuleEnd(const ModulePtr&)
{
}

void
Slice::Gen::MetaDataVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    StringList metaData = validate(p, p->getMetaData(), p->file(), p->line());
    p->setMetaData(metaData);
}

bool
Slice::Gen::MetaDataVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    StringList metaData = validate(p, p->getMetaData(), p->file(), p->line());
    p->setMetaData(metaData);
    return true;
}

void
Slice::Gen::MetaDataVisitor::visitClassDefEnd(const ClassDefPtr&)
{
}

bool
Slice::Gen::MetaDataVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    StringList metaData = validate(p, p->getMetaData(), p->file(), p->line());
    p->setMetaData(metaData);
    return true;
}

void
Slice::Gen::MetaDataVisitor::visitExceptionEnd(const ExceptionPtr&)
{
}

bool
Slice::Gen::MetaDataVisitor::visitStructStart(const StructPtr& p)
{
    StringList metaData = validate(p, p->getMetaData(), p->file(), p->line());
    p->setMetaData(metaData);
    return true;
}

void
Slice::Gen::MetaDataVisitor::visitStructEnd(const StructPtr&)
{
}

void
Slice::Gen::MetaDataVisitor::visitOperation(const OperationPtr& p)
{
    ClassDefPtr cl = ClassDefPtr::dynamicCast(p->container());

    StringList metaData = p->getMetaData();

    const UnitPtr ut = p->unit();
    const DefinitionContextPtr dc = ut->findDefinitionContext(p->file());
    assert(dc);
    if(!cl->isLocal() && p->hasMetaData("cpp:noexcept"))
    {
        dc->warning(InvalidMetaData, p->file(), p->line(), "ignoring metadata `cpp:noexcept' for non local interface");
        metaData.remove("cpp:noexcept");
    }

    TypePtr returnType = p->returnType();
    if(!returnType)
    {
        for(StringList::const_iterator q = metaData.begin(); q != metaData.end();)
        {
            string s = *q++;
            if(s.find("cpp:type:") == 0 || s.find("cpp:view-type:") == 0 ||
               s.find("cpp:range") == 0 || s == "cpp:array")
            {
                dc->warning(InvalidMetaData, p->file(), p->line(),
                            "ignoring invalid metadata `" + s + "' for operation with void return type");
                metaData.remove(s);
            }
        }
    }
    else
    {
        metaData = validate(returnType, metaData, p->file(), p->line(), true);
    }

    p->setMetaData(metaData);

    ParamDeclList params = p->parameters();
    for(ParamDeclList::iterator q = params.begin(); q != params.end(); ++q)
    {
        metaData = validate((*q)->type(), (*q)->getMetaData(), p->file(), (*q)->line(), true);
        (*q)->setMetaData(metaData);
    }
}

void
Slice::Gen::MetaDataVisitor::visitDataMember(const DataMemberPtr& p)
{
    StringList metaData = validate(p->type(), p->getMetaData(), p->file(), p->line());
    p->setMetaData(metaData);
}

void
Slice::Gen::MetaDataVisitor::visitSequence(const SequencePtr& p)
{
    StringList metaData = validate(p, p->getMetaData(), p->file(), p->line());
    p->setMetaData(metaData);
}

void
Slice::Gen::MetaDataVisitor::visitDictionary(const DictionaryPtr& p)
{
    StringList metaData = validate(p, p->getMetaData(), p->file(), p->line());
    p->setMetaData(metaData);
}

void
Slice::Gen::MetaDataVisitor::visitEnum(const EnumPtr& p)
{
    StringList metaData = validate(p, p->getMetaData(), p->file(), p->line());
    p->setMetaData(metaData);
}

void
Slice::Gen::MetaDataVisitor::visitConst(const ConstPtr& p)
{
    StringList metaData = validate(p, p->getMetaData(), p->file(), p->line());
    p->setMetaData(metaData);
}

StringList
Slice::Gen::MetaDataVisitor::validate(const SyntaxTreeBasePtr& cont, const StringList& metaData,
                                      const string& file, const string& line, bool operation)
{
    static const string cppPrefix = "cpp:";
    static const string cpp11Prefix = "cpp11:";
    static const string cpp98Prefix  = "cpp98:";

    const UnitPtr ut = cont->unit();
    const DefinitionContextPtr dc = ut->findDefinitionContext(file);
    assert(dc);
    StringList newMetaData = metaData;
    for(StringList::const_iterator p = newMetaData.begin(); p != newMetaData.end();)
    {
        string s = *p++;

        string prefix;
        bool cpp98 = false;
        bool cpp11 = false;

        if(s.find(cppPrefix) == 0)
        {
            prefix = cppPrefix;
        }
        else if(s.find(cpp98Prefix) == 0)
        {
            prefix = cpp98Prefix;
            cpp98 = true;
        }
        else if(s.find(cpp11Prefix) == 0)
        {
            prefix = cpp11Prefix;
            cpp11 = true;
        }

        if(operation && (s == "cpp:const" || s == "cpp:noexcept"))
        {
            continue;
        }

        if(!prefix.empty())
        {
            string ss = s.substr(prefix.size());
            if(ss == "type:wstring" || ss == "type:string")
            {
                BuiltinPtr builtin = BuiltinPtr::dynamicCast(cont);
                ModulePtr module = ModulePtr::dynamicCast(cont);
                ClassDefPtr clss = ClassDefPtr::dynamicCast(cont);
                StructPtr strct = StructPtr::dynamicCast(cont);
                ExceptionPtr exception = ExceptionPtr::dynamicCast(cont);
                if((builtin && builtin->kind() == Builtin::KindString) || module || clss || strct || exception)
                {
                    continue;
                }
            }
            if(BuiltinPtr::dynamicCast(cont) && (ss.find("type:") == 0 || ss.find("view-type:") == 0))
            {
                if(BuiltinPtr::dynamicCast(cont)->kind() == Builtin::KindString)
                {
                    continue;
                }
            }
            if(SequencePtr::dynamicCast(cont))
            {
                if(ss.find("type:") == 0 || ss.find("view-type:") == 0 || ss == "array" || ss.find("range") == 0)
                {
                    continue;
                }
            }
            if(DictionaryPtr::dynamicCast(cont) && (ss.find("type:") == 0 || ss.find("view-type:") == 0))
            {
                continue;
            }
            if(!cpp11 && StructPtr::dynamicCast(cont) && (ss == "class" || ss == "comparable"))
            {
                continue;
            }

            {
                ClassDefPtr cl = ClassDefPtr::dynamicCast(cont);
                if(cl && ((!cpp11 && ss == "virtual") ||
                          (cl->isLocal() && ss.find("type:") == 0) ||
                          (!cpp11 && cl->isLocal() && ss == "comparable")))
                {
                    continue;
                }
            }
            if(ExceptionPtr::dynamicCast(cont) && ss == "ice_print")
            {
                continue;
            }
            if(!cpp11 && EnumPtr::dynamicCast(cont) && ss == "scoped")
            {
                continue;
            }
            if(!cpp98 && EnumPtr::dynamicCast(cont) && ss == "unscoped")
            {
                continue;
            }

            {
                ClassDeclPtr cl = ClassDeclPtr::dynamicCast(cont);
                if(cl && cl->isLocal() && ss.find("type:") == 0)
                {
                    continue;
                }
            }

            dc->warning(InvalidMetaData, file, line, "ignoring invalid metadata `" + s + "'");
            newMetaData.remove(s);
            continue;
        }

        if(s.find("delegate") == 0)
        {
            ClassDefPtr cl = ClassDefPtr::dynamicCast(cont);
            if(cl && cl->isDelegate())
            {
                continue;
            }

            dc->warning(InvalidMetaData, file, line, "ignoring invalid metadata `" + s + "'");
            newMetaData.remove(s);
            continue;
        }
    }
    return newMetaData;
}

void
Slice::Gen::normalizeMetaData(const UnitPtr& u, bool cpp11)
{
    NormalizeMetaDataVisitor visitor(cpp11);
    u->visit(&visitor, false);
}

Slice::Gen::NormalizeMetaDataVisitor::NormalizeMetaDataVisitor(bool cpp11) :
    _cpp11(cpp11)
{
}

bool
Slice::Gen::NormalizeMetaDataVisitor::visitUnitStart(const UnitPtr&)
{
    return true;
}

bool
Slice::Gen::NormalizeMetaDataVisitor::visitModuleStart(const ModulePtr& p)
{
    p->setMetaData(normalize(p->getMetaData()));
    return true;
}

void
Slice::Gen::NormalizeMetaDataVisitor::visitModuleEnd(const ModulePtr&)
{
}

void
Slice::Gen::NormalizeMetaDataVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    p->setMetaData(normalize(p->getMetaData()));
}

bool
Slice::Gen::NormalizeMetaDataVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    p->setMetaData(normalize(p->getMetaData()));
    return true;
}

void
Slice::Gen::NormalizeMetaDataVisitor::visitClassDefEnd(const ClassDefPtr&)
{
}

bool
Slice::Gen::NormalizeMetaDataVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    p->setMetaData(normalize(p->getMetaData()));
    return true;
}

void
Slice::Gen::NormalizeMetaDataVisitor::visitExceptionEnd(const ExceptionPtr&)
{
}

bool
Slice::Gen::NormalizeMetaDataVisitor::visitStructStart(const StructPtr& p)
{
    p->setMetaData(normalize(p->getMetaData()));
    return true;
}

void
Slice::Gen::NormalizeMetaDataVisitor::visitStructEnd(const StructPtr&)
{
}

void
Slice::Gen::NormalizeMetaDataVisitor::visitOperation(const OperationPtr& p)
{
    p->setMetaData(normalize(p->getMetaData()));

    ParamDeclList params = p->parameters();
    for(ParamDeclList::iterator q = params.begin(); q != params.end(); ++q)
    {
        (*q)->setMetaData(normalize((*q)->getMetaData()));
    }
}

void
Slice::Gen::NormalizeMetaDataVisitor::visitDataMember(const DataMemberPtr& p)
{
    p->setMetaData(normalize(p->getMetaData()));
}

void
Slice::Gen::NormalizeMetaDataVisitor::visitSequence(const SequencePtr& p)
{
    p->setMetaData(normalize(p->getMetaData()));
}

void
Slice::Gen::NormalizeMetaDataVisitor::visitDictionary(const DictionaryPtr& p)
{
    p->setMetaData(normalize(p->getMetaData()));
}

void
Slice::Gen::NormalizeMetaDataVisitor::visitEnum(const EnumPtr& p)
{
    p->setMetaData(normalize(p->getMetaData()));
}

void
Slice::Gen::NormalizeMetaDataVisitor::visitConst(const ConstPtr& p)
{
    p->setMetaData(normalize(p->getMetaData()));
}

StringList
Slice::Gen::NormalizeMetaDataVisitor::normalize(const StringList& metaData)
{
    //
    // if _cpp11: transform "cpp:" into "cpp-all:" and "cpp"
    //            + transform "cpp11:" into "cpp:" in front
    //
    // if !_cpp11: remove "cpp:", transform "cpp-all:" into "cpp"
    //             + transform "cpp98:" into "cpp:" in front

    //
    // Note: global metadata like header-ext exists only in cpp:
    // form and are not processed at all
    //

    StringList result;

    static const string cppPrefixTable[] =
    {
        "array",
        "class",
        "comparable",
        "const",
        "ice_print",
        "range",
        "scoped",
        "type:",
        "unscoped",
        "view-type:",
        "virtual",
        ""
    };

    static const string cppPrefix = "cpp:";
    static const string cppAllPrefix = "cpp-all:";

    //
    // First look for the higher priority cpp98/cpp11, that go to the
    // front of result
    //

    static const string cpp11Prefix = "cpp11:";
    static const string cpp98Prefix = "cpp98:";

    const string altCppPrefix = _cpp11 ? cpp11Prefix : cpp98Prefix;

    for(StringList::const_iterator p = metaData.begin(); p != metaData.end(); ++p)
    {
        string s = *p;

        unsigned int i = 0;
        bool found = false;
        while(!found)
        {
            string m = cppPrefixTable[i++];
            if(m.empty())
            {
                break;
            }
            if(s.find(altCppPrefix + m) == 0)
            {
                found = true;
            }
        }

        if(found)
        {
            s.replace(0, altCppPrefix.length(), cppPrefix);
            result.push_back(s);
        }
    }

    //
    // Then look for the lower-priority "cpp:" / "cpp-all:", pushed back later
    //

    const string prefix = _cpp11 ? cppPrefix : cppAllPrefix;

    for(StringList::const_iterator p = metaData.begin(); p != metaData.end(); ++p)
    {
        string s = *p;

        unsigned int i = 0;
        bool foundPrefix = false;
        bool foundOld = false;
        while(!foundPrefix && !foundOld)
        {
            string m = cppPrefixTable[i++];
            if(m.empty())
            {
                break; // while
            }
            if(s.find(prefix + m) == 0)
            {
                foundPrefix = true;
            }
            else if(!_cpp11 && s.find(cppPrefix + m) == 0)
            {
                //
                // We want to filter-out "cpp:" when !_cpp11
                //
                foundOld = true;
            }
        }

        if(foundPrefix)
        {
            if(_cpp11)
            {
                result.push_back(s);
                s.replace(0, prefix.length(), cppAllPrefix);
                result.push_back(s);
            }
            else
            {
                s.replace(0, prefix.length(), cppPrefix);
                result.push_back(s);
            }
        }
        else if(_cpp11 || !foundOld)
        {
            result.push_back(s);
        }
    }

    return result;
}

int
Slice::Gen::setUseWstring(ContainedPtr p, list<int>& hist, int use)
{
    hist.push_back(use);
    StringList metaData = p->getMetaData();
    if(find(metaData.begin(), metaData.end(), "cpp:type:wstring") != metaData.end())
    {
        use = TypeContextUseWstring;
    }
    else if(find(metaData.begin(), metaData.end(), "cpp:type:string") != metaData.end())
    {
        use = 0;
    }
    return use;
}

int
Slice::Gen::resetUseWstring(list<int>& hist)
{
    int use = hist.back();
    hist.pop_back();
    return use;
}

string
Slice::Gen::getHeaderExt(const string& file, const UnitPtr& ut)
{
    string ext;
    static const string headerExtPrefix = "cpp:header-ext:";
    DefinitionContextPtr dc = ut->findDefinitionContext(file);
    assert(dc);
    string meta = dc->findMetaData(headerExtPrefix);
    if(meta.size() > headerExtPrefix.size())
    {
        ext = meta.substr(headerExtPrefix.size());
    }
    return ext;
}

string
Slice::Gen::getSourceExt(const string& file, const UnitPtr& ut)
{
    string ext;
    static const string sourceExtPrefix = "cpp:source-ext:";
    DefinitionContextPtr dc = ut->findDefinitionContext(file);
    assert(dc);
    string meta = dc->findMetaData(sourceExtPrefix);
    if(meta.size() > sourceExtPrefix.size())
    {
        ext = meta.substr(sourceExtPrefix.size());
    }
    return ext;
}

// C++11 visitors
Slice::Gen::Cpp11DeclVisitor::Cpp11DeclVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport)
{
}

bool
Slice::Gen::Cpp11DeclVisitor::visitUnitStart(const UnitPtr& p)
{
    if(!p->hasClassDecls() && !p->hasNonLocalExceptions())
    {
        return false;
    }
    C << sp << nl << "namespace" << nl << "{";
    return true;
}

void
Slice::Gen::Cpp11DeclVisitor::visitUnitEnd(const UnitPtr&)
{
    C << sp << nl << "}";
}

bool
Slice::Gen::Cpp11DeclVisitor::visitModuleStart(const ModulePtr& p)
{
    if(p->hasClassDecls())
    {
        H << sp << nl << "namespace " << fixKwd(p->name()) << nl << '{' << sp;
    }
    return true;
}

void
Slice::Gen::Cpp11DeclVisitor::visitModuleEnd(const ModulePtr& p)
{
    if(p->hasClassDecls())
    {
        H << sp << nl << '}';
    }
}

void
Slice::Gen::Cpp11DeclVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    ClassDefPtr def = p->definition();
    if(def && def->isDelegate())
    {
        return;
    }

    H << nl << "class " << fixKwd(p->name()) << ';';
    if(!p->isLocal() && (p->isInterface() || (def && !def->allOperations().empty())))
    {
        H << nl << "class " << p->name() << "Prx;";
    }
}

bool
Slice::Gen::Cpp11DeclVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal())
    {
        return false;
    }

    if(!p->isInterface())
    {
        C << sp;

        C << nl << "const ::IceInternal::DefaultValueFactoryInit<" << fixKwd(p->scoped()) << "> ";
        C << "iceC" + p->flattenedScope() + p->name() + "_init" << "(\"" << p->scoped() << "\");";

        if(p->compactId() >= 0)
        {
            string n = "iceC" + p->flattenedScope() + p->name() + "_compactIdInit ";
            C << nl << "const ::IceInternal::CompactIdInit " << n << "(\"" << p->scoped() << "\", " << p->compactId()
              << ");";
        }
    }

    OperationList allOps = p->allOperations();
    if(p->isInterface() || !allOps.empty())
    {
        C << sp;

        ClassList allBases = p->allBases();
        StringList ids;
        transform(allBases.begin(), allBases.end(), back_inserter(ids), ::IceUtil::constMemFun(&Contained::scoped));
        StringList other;
        other.push_back(p->scoped());
        other.push_back("::Ice::Object");
        other.sort();
        ids.merge(other);
        ids.unique();

        C << nl << "const ::std::string iceC" << p->flattenedScope() << p->name() << "_ids[" << ids.size() << "] =";
        C << sb;
        for(StringList::const_iterator r = ids.begin(); r != ids.end();)
        {
            C << nl << '"' << *r << '"';
            if(++r != ids.end())
            {
                C << ',';
            }
        }
        C << eb << ';';

        StringList allOpNames;
        transform(allOps.begin(), allOps.end(), back_inserter(allOpNames), ::IceUtil::constMemFun(&Contained::name));
        allOpNames.push_back("ice_id");
        allOpNames.push_back("ice_ids");
        allOpNames.push_back("ice_isA");
        allOpNames.push_back("ice_ping");
        allOpNames.sort();
        allOpNames.unique();

        C << nl << "const ::std::string iceC" << p->flattenedScope() << p->name() << "_ops[] =";
        C << sb;
        for(StringList::const_iterator q = allOpNames.begin(); q != allOpNames.end();)
        {
            C << nl << '"' << *q << '"';
            if(++q != allOpNames.end())
            {
                C << ',';
            }
        }
        C << eb << ';';
    }

    return true;
}

bool
Slice::Gen::Cpp11DeclVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    if(p->isLocal())
    {
        return false;
    }

    C << sp;
    C << nl << "const ::IceInternal::DefaultUserExceptionFactoryInit<" << fixKwd(p->scoped()) << "> ";
    C << "iceC" + p->flattenedScope() + p->name() + "_init" << "(\"" << p->scoped() << "\");";
    return false;
}

void
Slice::Gen::Cpp11DeclVisitor::visitOperation(const OperationPtr& p)
{
    ClassDefPtr cl = ClassDefPtr::dynamicCast(p->container());
    if(cl && !cl->isLocal())
    {
        string flatName = "iceC" + p->flattenedScope() + p->name() + "_name";
        C << nl << "const ::std::string " << flatName << " = \"" << p->name() << "\";";
    }
}

Slice::Gen::Cpp11TypesVisitor::Cpp11TypesVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport), _dllClassExport(toDllClassExport(dllExport)),
    _dllMemberExport(toDllMemberExport(dllExport)), _doneStaticSymbol(false), _useWstring(false)
{
}

bool
Slice::Gen::Cpp11TypesVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasOtherConstructedOrExceptions())
    {
        return false;
    }

    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);
    H << sp << nl << "namespace " << fixKwd(p->name()) << nl << '{';
    return true;
}

void
Slice::Gen::Cpp11TypesVisitor::visitModuleEnd(const ModulePtr& p)
{
    if(p->hasStructs())
    {
        H << sp << nl << "using Ice::operator<;";
        H << nl << "using Ice::operator<=;";
        H << nl << "using Ice::operator>;";
        H << nl << "using Ice::operator>=;";
        H << nl << "using Ice::operator==;";
        H << nl << "using Ice::operator!=;";
    }
    H << sp << nl << '}';
    _useWstring = resetUseWstring(_useWstringHist);
}

bool
Slice::Gen::Cpp11TypesVisitor::visitClassDefStart(const ClassDefPtr&)
{
    return false;
}

bool
Slice::Gen::Cpp11TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    string name = fixKwd(p->name());
    string scope = fixKwd(p->scope());
    string scoped = fixKwd(p->scoped());
    ExceptionPtr base = p->base();
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList allDataMembers = p->allDataMembers();
    DataMemberList baseDataMembers;

    vector<string> params;
    vector<string> allParamDecls;
    vector<string> baseParams;
    map<string, CommentPtr> allComments;

    string fileParam = "file";
    string lineParam = "line";

    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        params.push_back(fixKwd((*q)->name()));
    }

    for(DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
    {
        string typeName = inputTypeToString((*q)->type(), (*q)->optional(), scope, (*q)->getMetaData(),
                                            _useWstring | TypeContextCpp11);
        allParamDecls.push_back(typeName + " " + fixKwd((*q)->name()));

        CommentPtr comment = (*q)->parseComment(false);
        if(comment)
        {
            allComments[(*q)->name()] = comment;
        }

        if((*q)->name() == "file")
        {
            fileParam = "file_";
        }
        else if((*q)->name() == "line")
        {
            fileParam = "line_";
        }
    }

    if(base)
    {
        baseDataMembers = base->allDataMembers();
        for(DataMemberList::const_iterator q = baseDataMembers.begin(); q != baseDataMembers.end(); ++q)
        {
            baseParams.push_back(fixKwd((*q)->name()));
        }
    }

    string helperClass =
        getUnqualified(p->isLocal() ? "::Ice::LocalExceptionHelper" : "::Ice::UserExceptionHelper", scope);
    string baseClass = base ?
        getUnqualified(fixKwd(base->scoped()), scope) :
        getUnqualified(p->isLocal() ? "::Ice::LocalException" : "::Ice::UserException", scope);
    string templateParameters = name + ", " + baseClass;

    H << sp;
    writeDocSummary(H, p);
    H << nl << "class " << _dllClassExport << name << " : public " << helperClass << "<" << templateParameters << ">";
    H << sb;

    H.dec();
    H << nl << "public:";
    H.inc();

    // Out of line dtor to avoid weak vtable
    H << sp << nl << _dllMemberExport << "virtual ~" << name << "();";

    // Default copy ctor
    H << sp << nl << name << "(const " << name << "&) = default;";

    C << sp;
    C << nl << scoped.substr(2) << "::~" << name << "()";
    C << sb;
    C << eb;

    if(p->isLocal())
    {
        H << sp;
        H << nl << "/**";
        H << nl << " * The file and line number are required for all local exceptions.";
        H << nl << " * @param " << fileParam
          << " The file name in which the exception was raised, typically __FILE__.";
        H << nl << " * @param " << lineParam
          << " The line number at which the exception was raised, typically __LINE__.";
        H << nl << " */";
        H << nl << name << "(const char* " << fileParam << ", int " << lineParam << ") : ";
        H << getUnqualified("::Ice::LocalExceptionHelper", scope) << "<" << templateParameters << ">";
        H << "(" << fileParam << ", " << lineParam << ")";
        H << sb;
        H << eb;
    }
    else
    {
        H << sp << nl << name << "() = default;";
    }

    if(!allDataMembers.empty())
    {
        H << sp;
        H << nl << "/**";
        H << nl << " * One-shot constructor to initialize all data members.";
        if(p->isLocal())
        {
            H << nl << " * The file and line number are required for all local exceptions.";
            H << nl << " * @param " << fileParam
              << " The file name in which the exception was raised, typically __FILE__.";
            H << nl << " * @param " << lineParam
              << " The line number at which the exception was raised, typically __LINE__.";
        }
        for(DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
        {
            map<string, CommentPtr>::iterator r = allComments.find((*q)->name());
            if(r != allComments.end())
            {
                H << nl << " * @param " << fixKwd(r->first) << " " << getDocSentence(r->second->overview());
            }
        }
        H << nl << " */";
        H << nl << name << "(";
        if(p->isLocal())
        {
            H << "const char* " << fileParam << ", int " << lineParam;
            if(!allParamDecls.empty())
            {
                H << ", ";
            }
        }

        for(vector<string>::const_iterator q = allParamDecls.begin(); q != allParamDecls.end(); ++q)
        {
            if(q != allParamDecls.begin())
            {
                H << ", ";
            }
            H << (*q);
        }
        H << ") :";
        H.inc();
        if(base && (p->isLocal() || !baseDataMembers.empty()))
        {
            H << nl << helperClass << "<" << templateParameters << ">" << "(";
            if(p->isLocal())
            {
                H << fileParam << ", " << lineParam;
                if(!baseDataMembers.empty())
                {
                    H << ", ";
                }
            }

            for(DataMemberList::const_iterator q = baseDataMembers.begin(); q != baseDataMembers.end(); ++q)
            {
                if(q != baseDataMembers.begin())
                {
                    H << ", ";
                }
                if(isMovable((*q)->type()))
                {
                    H << "::std::move(" << fixKwd((*q)->name()) << ")";
                }
                else
                {
                    H << fixKwd((*q)->name());
                }
            }

            H << ")";
            if(!dataMembers.empty())
            {
                H << ",";
            }
        }
        else if(p->isLocal())
        {
            H << " " << getUnqualified("::Ice::LocalExceptionHelper", scope) << "<" << templateParameters << ">";
            H << "(" << fileParam << ", " << lineParam << ")";
            if(!dataMembers.empty())
            {
                H << ",";
            }
        }

        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            string memberName = fixKwd((*q)->name());
            if(q != dataMembers.begin())
            {
                H << ",";
            }
            if(isMovable((*q)->type()))
            {
                H << nl << memberName << "(::std::move(" << memberName << "))";
            }
            else
            {
                H << nl << memberName << "(" << memberName << ")";
            }
        }

        H.dec();
        H << sb;
        H << eb;
    }

    H << sp;
    H << nl << "/**";
    H << nl << " * Obtains a tuple containing all of the exception's data members.";
    H << nl << " * @return The data members in a tuple.";
    H << nl << " */";
    writeIceTuple(H, scope, p->allDataMembers(), _useWstring);

    H << sp;
    H << nl << "/**";
    H << nl << " * Obtains the Slice type ID of this exception.";
    H << nl << " * @return The fully-scoped type ID.";
    H << nl << " */";
    H << nl << _dllMemberExport << "static const ::std::string& ice_staticId();";

    C << sp << nl << "const ::std::string&" << nl << scoped.substr(2) << "::ice_staticId()";
    C << sb;
    //
    // Use local static so that ice_staticId() is usable during static construction.
    //
    C << nl << "static const ::std::string typeId = \"" << p->scoped() << "\";";
    C << nl << "return typeId;";
    C << eb;

    StringList metaData = p->getMetaData();
    if(find(metaData.begin(), metaData.end(), "cpp:ice_print") != metaData.end())
    {
        H << nl << "/**";
        H << nl << " * Prints this exception to the given stream.";
        H << nl << " * @param stream The target stream.";
        H << nl << " */";
        H << nl << _dllMemberExport << "virtual void ice_print(::std::ostream& stream) const override;";
    }

    if(!p->isLocal() && p->usesClasses(false))
    {
        if(!base || (base && !base->usesClasses(false)))
        {
            H << sp;
            H << nl << "/// \\cond STREAM";
            H << nl << _dllMemberExport << "virtual bool _usesClasses() const override;";
            H << nl << "/// \\endcond";

            C << sp;
            C << nl << "/// \\cond STREAM";
            C << nl << "bool";
            C << nl << scoped.substr(2) << "::_usesClasses() const";
            C << sb;
            C << nl << "return true;";
            C << eb;
            C << nl << "/// \\endcond";
        }
    }

    if(!dataMembers.empty())
    {
        H << sp;
    }
    return true;
}

void
Slice::Gen::Cpp11TypesVisitor::visitExceptionEnd(const ExceptionPtr& p)
{
    string name = fixKwd(p->name());
    string scope = fixKwd(p->scope());
    string scoped = fixKwd(p->scoped());
    string factoryName;

    if(!p->isLocal())
    {
        ExceptionPtr base = p->base();
        bool basePreserved = p->inheritsMetaData("preserve-slice");
        bool preserved = p->hasMetaData("preserve-slice");

        if(preserved && !basePreserved)
        {
            H << sp;
            H << nl << "/**";
            H << nl << " * Obtains the SlicedData object created when an unknown exception type was marshaled";
            H << nl << " * in the sliced format and the Ice run time sliced it to a known type.";
            H << nl << " * @return The SlicedData object, or nil if the exception was not sliced or was not";
            H << nl << " * marshaled in the sliced format.";
            H << nl << " */";
            H << nl << _dllMemberExport << "virtual ::std::shared_ptr<" << getUnqualified("::Ice::SlicedData", scope)
              << "> ice_getSlicedData() const override;";
            H << sp;
            H << nl << "/// \\cond STREAM";
            H << nl << _dllMemberExport << "virtual void _write(" << getUnqualified("::Ice::OutputStream*", scope)
              << ") const override;";
            H << nl << _dllMemberExport << "virtual void _read(" << getUnqualified("::Ice::InputStream*", scope)
              << ") override;";

            H << sp << nl << "::std::shared_ptr<" << getUnqualified("::Ice::SlicedData", scope) << "> _slicedData;";
            H << nl << "/// \\endcond";

            C << sp;
            C << nl << "::std::shared_ptr<::Ice::SlicedData>" << nl << scoped.substr(2)
              << "::ice_getSlicedData() const";
            C << sb;
            C << nl << "return _slicedData;";
            C << eb;

            C << sp;
            C << nl << "/// \\cond STREAM";
            C << nl << "void" << nl << scoped.substr(2) << "::_write("
              << getUnqualified("::Ice::OutputStream*", scope) << " ostr) const";
            C << sb;
            C << nl << "ostr->startException(_slicedData);";
            C << nl << "_writeImpl(ostr);";
            C << nl << "ostr->endException();";
            C << eb;

            C << sp << nl << "void" << nl << scoped.substr(2) << "::_read("
              << getUnqualified("::Ice::InputStream*", scope) << " istr)";
            C << sb;
            C << nl << "istr->startException();";
            C << nl << "_readImpl(istr);";
            C << nl << "_slicedData = istr->endException(true);";
            C << eb;
            C << nl << "/// \\endcond";
        }
    }
    H << eb << ';';

    if(!p->isLocal())
    {
        //
        // We need an instance here to trigger initialization if the implementation is in a shared libarry.
        // But we do this only once per source file, because a single instance is sufficient to initialize
        // all of the globals in a shared library.
        //
        if(!_doneStaticSymbol)
        {
            _doneStaticSymbol = true;
            H << sp;
            H << nl << "/// \\cond INTERNAL";
            H << nl << "static " << name << " _iceS_" << p->name() << "_init;";
            H << nl << "/// \\endcond";
        }
    }

    _useWstring = resetUseWstring(_useWstringHist);
}

bool
Slice::Gen::Cpp11TypesVisitor::visitStructStart(const StructPtr& p)
{
    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    H << sp;
    writeDocSummary(H, p);
    H << nl << "struct " << fixKwd(p->name());
    H << sb;

    return true;
}

void
Slice::Gen::Cpp11TypesVisitor::visitStructEnd(const StructPtr& p)
{
    H << sp;
    H << nl << "/**";
    H << nl << " * Obtains a tuple containing all of the exception's data members.";
    H << nl << " * @return The data members in a tuple.";
    H << nl << " */";
    writeIceTuple(H, fixKwd(p->scope()), p->dataMembers(), _useWstring);
    H << eb << ';';
    _useWstring = resetUseWstring(_useWstringHist);
}

void
Slice::Gen::Cpp11TypesVisitor::visitDataMember(const DataMemberPtr& p)
{
    string scope = fixKwd(ContainedPtr::dynamicCast(p->container())->scope());
    string name = fixKwd(p->name());
    writeDocSummary(H, p);
    H << nl << typeToString(p->type(), p->optional(), scope, p->getMetaData(), _useWstring | TypeContextCpp11)
      << ' ' << name;

    string defaultValue = p->defaultValue();
    if(!defaultValue.empty())
    {
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(p->type());
        if(p->optional() && builtin->kind() == Builtin::KindString)
        {
            //
            // = "<string literal>" doesn't work for optional<std::string>
            //
            H << '{';
            writeConstantValue(H, p->type(), p->defaultValueType(), defaultValue, _useWstring | TypeContextCpp11,
                               p->getMetaData(), scope);
            H << '}';
        }
        else
        {
            H << " = ";
            writeConstantValue(H, p->type(), p->defaultValueType(), defaultValue, _useWstring | TypeContextCpp11,
                               p->getMetaData(), scope);
        }
    }

    H << ';';
}

void
Slice::Gen::Cpp11TypesVisitor::visitSequence(const SequencePtr& p)
{
    string name = fixKwd(p->name());
    string scope = fixKwd(p->scope());
    TypePtr type = p->type();
    int typeCtx = p->isLocal() ? (_useWstring | TypeContextLocal) : _useWstring;
    string s = typeToString(type, scope, p->typeMetaData(), typeCtx | TypeContextCpp11);
    StringList metaData = p->getMetaData();

    string seqType = findMetaData(metaData, _useWstring);
    H << sp;
    writeDocSummary(H, p);

    if(!seqType.empty())
    {
        H << nl << "using " << name << " = " << seqType << ';';
    }
    else
    {
        H << nl << "using " << name << " = ::std::vector<" << s << ">;";
    }
}

void
Slice::Gen::Cpp11TypesVisitor::visitDictionary(const DictionaryPtr& p)
{
    string name = fixKwd(p->name());
    string scope = fixKwd(p->scope());
    string dictType = findMetaData(p->getMetaData());
    int typeCtx = p->isLocal() ? (_useWstring | TypeContextLocal) : _useWstring;

    H << sp;
    writeDocSummary(H, p);

    if(dictType.empty())
    {
        //
        // A default std::map dictionary
        //
        TypePtr keyType = p->keyType();
        TypePtr valueType = p->valueType();
        string ks = typeToString(keyType, scope, p->keyMetaData(), typeCtx | TypeContextCpp11);
        string vs = typeToString(valueType, scope, p->valueMetaData(), typeCtx | TypeContextCpp11);

        H << nl << "using " << name << " = ::std::map<" << ks << ", " << vs << ">;";
    }
    else
    {
        //
        // A custom dictionary
        //
        H << nl << "using " << name << " = " << dictType << ';';
    }
}

Slice::Gen::Cpp11ProxyVisitor::Cpp11ProxyVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllClassExport(toDllClassExport(dllExport)),
    _dllMemberExport(toDllMemberExport(dllExport)),
    _useWstring(false)
{
}

bool
Slice::Gen::Cpp11ProxyVisitor::visitUnitStart(const UnitPtr&)
{
    return true;
}

void
Slice::Gen::Cpp11ProxyVisitor::visitUnitEnd(const UnitPtr&)
{
}

bool
Slice::Gen::Cpp11ProxyVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalClassDefs())
    {
        return false;
    }

    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    H << sp << nl << "namespace " << fixKwd(p->name()) << nl << '{';
    return true;
}

void
Slice::Gen::Cpp11ProxyVisitor::visitModuleEnd(const ModulePtr&)
{
    H << sp << nl << '}';

    _useWstring = resetUseWstring(_useWstringHist);
}

bool
Slice::Gen::Cpp11ProxyVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal() || (!p->isInterface() && p->allOperations().empty()))
    {
        return false;
    }

    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    string name = fixKwd(p->name());
    string scope = fixKwd(p->scope());
    string scoped = fixKwd(p->scoped());
    ClassList bases = p->bases();

    ClassDefPtr base;
    if(!bases.empty() && !bases.front()->isInterface())
    {
        base = bases.front();
    }

    H << sp;
    writeDocSummary(H, p);
    H << nl << "class " << _dllClassExport << p->name() << "Prx : public virtual "
      << getUnqualified("::Ice::Proxy", scope) << "<" << fixKwd(p->name() + "Prx") << ", ";
    if(bases.empty() || (bases.size() == 1 && base && base->allOperations().empty()))
    {
        H << getUnqualified("::Ice::ObjectPrx", scope);
    }
    else
    {
        if(base && base->allOperations().empty())
        {
            bases.pop_front();
        }

        ClassList::const_iterator q = bases.begin();
        while(q != bases.end())
        {
            H << getUnqualified(fixKwd((*q)->scoped() + "Prx"), scope);
            if(++q != bases.end())
            {
                H << ", ";
            }
        }
    }
    H << ">";

    H << sb;
    H.dec();
    H << nl << "public:";
    H.inc();
    return true;
}

void
Slice::Gen::Cpp11ProxyVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    string prx = fixKwd(p->name() + "Prx");
    const string suffix = p->isInterface() ? "" : "Disp";
    const string scoped = fixKwd(p->scoped() + "Prx");
    const string scope = fixKwd(p->scope());

    H << sp;
    H << nl << "/**";
    H << nl << " * Obtains the Slice type ID of this " << (p->isInterface() ? "interface" : "class") << ".";
    H << nl << " * @return The fully-scoped type ID.";
    H << nl << " */";
    H << nl << _dllMemberExport << "static const ::std::string& ice_staticId();";

    H.dec();
    H << sp << nl << "protected:";
    H.inc();
    H << sp;
    H << nl << "/// \\cond INTERNAL";
    H << nl << getUnqualified(prx, scope) << "() = default;";
    H << nl << "friend ::std::shared_ptr<" << getUnqualified(prx, scope) << "> IceInternal::createProxy<"
      << getUnqualified(prx, scope) << ">();";
    H << sp;
    H << nl << _dllMemberExport << "virtual ::std::shared_ptr<" << getUnqualified("::Ice::ObjectPrx", scope)
      << "> _newInstance() const override;";
    H << nl << "/// \\endcond";
    H << eb << ';';

    C << sp;
    C << nl << "/// \\cond INTERNAL";
    C << nl << "::std::shared_ptr<::Ice::ObjectPrx>";
    C << nl << scoped.substr(2) << "::_newInstance() const";
    C << sb;
    C << nl << "return ::IceInternal::createProxy<" << getUnqualified(prx, scope) << ">();";
    C << eb;
    C << nl << "/// \\endcond";
    C << sp;
    C << nl << "const ::std::string&" << nl << scoped.substr(2) << "::ice_staticId()";
    C << sb;
    C << nl << "return "<< fixKwd(p->name() + suffix) << "::ice_staticId();";
    C << eb;

    _useWstring = resetUseWstring(_useWstringHist);
}

void
Slice::Gen::Cpp11ProxyVisitor::visitOperation(const OperationPtr& p)
{
    string name = p->name();
    string flatName = "iceC" + p->flattenedScope() + p->name() + "_name";

    ContainerPtr container = p->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    string clScope = fixKwd(cl->scope());

    TypePtr ret = p->returnType();

    bool retIsOpt = p->returnIsOptional();
    string retS = returnTypeToString(ret, retIsOpt, clScope, p->getMetaData(), _useWstring | TypeContextCpp11);

    vector<string> params;
    vector<string> paramsDecl;

    vector<string> inParamsS;
    vector<string> inParamsDecl;
    vector<string> inParamsImplDecl;

    vector<string> futureOutParams;
    vector<string> lambdaOutParams;

    ParamDeclList paramList = p->parameters();
    ParamDeclList inParams = p->inParameters();
    ParamDeclList outParams = p->outParameters();

    string returnValueS = "returnValue";
    bool outParamsHasOpt = false;

    if(ret)
    {
        futureOutParams.push_back(typeToString(ret, retIsOpt, clScope, p->getMetaData(), _useWstring |
                                               TypeContextCpp11));

        lambdaOutParams.push_back(typeToString(ret, retIsOpt, clScope, p->getMetaData(), _useWstring |
                                               TypeContextInParam | TypeContextCpp11));

        outParamsHasOpt |= p->returnIsOptional();
    }

    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        string paramName = fixKwd((*q)->name());
        StringList metaData = (*q)->getMetaData();

        if((*q)->isOutParam())
        {
            futureOutParams.push_back(typeToString((*q)->type(), (*q)->optional(), clScope, metaData,
                                                   _useWstring | TypeContextCpp11));
            lambdaOutParams.push_back(typeToString((*q)->type(), (*q)->optional(), clScope, metaData,
                                                   _useWstring | TypeContextInParam | TypeContextCpp11));

            string outputTypeString = outputTypeToString((*q)->type(), (*q)->optional(), clScope, metaData,
                                                         _useWstring | TypeContextCpp11);

            params.push_back(outputTypeString);
            paramsDecl.push_back(outputTypeString + ' ' + paramName);

            outParamsHasOpt |= (*q)->optional();

            if((*q)->name() == "returnValue")
            {
                returnValueS = "_returnValue";
            }
        }
        else
        {
            string typeString = inputTypeToString((*q)->type(), (*q)->optional(), clScope, metaData,
                                                  _useWstring | TypeContextCpp11);

            params.push_back(typeString);
            paramsDecl.push_back(typeString + ' ' + paramName);

            inParamsS.push_back(typeString);
            inParamsDecl.push_back(typeString + ' ' + paramName);
            inParamsImplDecl.push_back(typeString + ' ' + paramPrefix + (*q)->name());
        }
    }

    string scoped = fixKwd(cl->scope() + cl->name() + "Prx" + "::").substr(2);

    const string contextParam = escapeParam(paramList, "context");
    const string contextDecl = "const " + getUnqualified("::Ice::Context&", clScope) + " " + contextParam + " = " +
        getUnqualified("::Ice::noExplicitContext", clScope);

    string futureT;
    if(futureOutParams.empty())
    {
        futureT = "void";
    }
    else if(futureOutParams.size() == 1)
    {
        futureT = futureOutParams[0];
    }
    else
    {
        futureT = resultStructName(name, fixKwd(cl->name()));
    }

    const string deprecateSymbol = getDeprecateSymbol(p, cl);

    CommentPtr comment = p->parseComment(false);
    const string contextDoc = "@param " + contextParam + " The Context map to send with the invocation.";
    const string futureDoc = "The future object for the invocation.";

    //
    // Synchronous operation
    //
    H << sp;
    if(comment)
    {
        StringList postParams;
        postParams.push_back(contextDoc);
        writeOpDocSummary(H, p, comment, OpDocAllParams, true, StringList(), postParams, comment->returns());
    }
    H << nl << deprecateSymbol << retS << ' ' << fixKwd(name) << spar << paramsDecl << contextDecl << epar;
    H << sb;
    H << nl;
    if(futureOutParams.size() == 1)
    {
        if(ret)
        {
            H << "return ";
        }
        else
        {
            H << fixKwd((*outParams.begin())->name()) << " = ";
        }
    }
    else if(futureOutParams.size() > 1)
    {
        H << "auto _result = ";
    }

    H << "_makePromiseOutgoing<" << getUnqualified(futureT, cl->scoped()) << ">";

    H << spar << "true, this" << "&" + cl->name() + "Prx::_iceI_" + name;
    for(ParamDeclList::const_iterator q = inParams.begin(); q != inParams.end(); ++q)
    {
        H << fixKwd((*q)->name());
    }
    H << contextParam << epar << ".get();";
    if(futureOutParams.size() > 1)
    {
        for(ParamDeclList::const_iterator q = outParams.begin(); q != outParams.end(); ++q)
        {
            H << nl << fixKwd((*q)->name()) << " = ";
            H << condMove(isMovable((*q)->type()), "_result." + fixKwd((*q)->name())) + ";";
        }
        if(ret)
        {
            H << nl << "return " + condMove(isMovable(ret), "_result." + returnValueS) + ";";
        }
    }
    H << eb;

    //
    // Promise based asynchronous operation
    //
    H << sp;
    if(comment)
    {
        StringList postParams, returns;
        postParams.push_back(contextDoc);
        returns.push_back(futureDoc);
        writeOpDocSummary(H, p, comment, OpDocInParams, false, StringList(), postParams, returns);
    }
    H << nl << "template<template<typename> class P = ::std::promise>";
    H << nl << deprecateSymbol << "auto " << name << "Async" << spar << inParamsDecl << contextDecl << epar;
    H.inc();
    H << nl << "-> decltype(::std::declval<P<" << futureT << ">>().get_future())";
    H.dec();
    H << sb;

    H << nl << "return _makePromiseOutgoing<" << futureT << ", P>" << spar;

    H << "false, this" << string("&" + cl->name() + "Prx::_iceI_" + name);
    for(ParamDeclList::const_iterator q = inParams.begin(); q != inParams.end(); ++q)
    {
        H << fixKwd((*q)->name());
    }
    H << contextParam << epar << ";";
    H << eb;

    //
    // Lambda based asynchronous operation
    //
    bool lambdaCustomOut = (lambdaOutParams != futureOutParams);

    const string responseParam = escapeParam(inParams, "response");
    const string exParam = escapeParam(inParams, "ex");
    const string sentParam = escapeParam(inParams, "sent");

    H << sp;
    if(comment)
    {
        StringList postParams, returns;
        postParams.push_back("@param " + responseParam + " The response callback.");
        postParams.push_back("@param " + exParam + " The exception callback.");
        postParams.push_back("@param " + sentParam + " The sent callback.");
        postParams.push_back(contextDoc);
        returns.push_back("A function that can be called to cancel the invocation locally.");
        writeOpDocSummary(H, p, comment, OpDocInParams, false, StringList(), postParams, returns);
    }
    H << nl;
    if(lambdaCustomOut)
    {
        H << _dllMemberExport;
    }
    H << "::std::function<void()>";
    H << nl << name << "Async(";
    H.useCurrentPosAsIndent();
    if(!inParamsDecl.empty())
    {
        if(lambdaCustomOut)
        {
            for(vector<string>::const_iterator q = inParamsS.begin(); q != inParamsS.end(); ++q)
            {
                if(q != inParamsS.begin())
                {
                    H << " ";
                }

                H << *q << ",";
            }
        }
        else
        {
            for(vector<string>::const_iterator q = inParamsDecl.begin(); q != inParamsDecl.end(); ++q)
            {
                if(q != inParamsDecl.begin())
                {
                    H << " ";
                }

                H << *q << ",";
            }
        }
        H << nl;
    }

    H << "::std::function<void" << spar << lambdaOutParams << epar << "> " << responseParam << ",";
    H << nl << "::std::function<void(::std::exception_ptr)> " << exParam << " = nullptr,";
    H << nl << "::std::function<void(bool)> " << sentParam << " = nullptr,";
    H << nl << contextDecl << ")" << string(lambdaCustomOut ? ";" : "");

    H.restoreIndent();
    if(lambdaCustomOut)
    {
        //
        // "Custom" implementation in .cpp file
        //

        C << sp;
        C << nl << "::std::function<void()>";
        C << nl << scoped << name << "Async(";
        C.useCurrentPosAsIndent();
        if(!inParamsImplDecl.empty())
        {
            for(vector<string>::const_iterator q = inParamsImplDecl.begin(); q != inParamsImplDecl.end(); ++q)
            {
                if(q != inParamsImplDecl.begin())
                {
                    C << " ";
                }
                C << *q << ",";
            }
            C << nl;
        }
        C << "::std::function<void " << spar << lambdaOutParams << epar << "> response,";
        C << nl << "::std::function<void(::std::exception_ptr)> ex,";
        C << nl << "::std::function<void(bool)> sent,";
        C << nl << "const ::Ice::Context& context)";
        C.restoreIndent();
        C << sb;
        if(p->returnsData())
        {
            C << nl << "_checkTwowayOnly(" << flatName << ");";
        }

        C << nl << "::std::function<void(::Ice::InputStream*)> read;";
        C << nl << "if(response)";
        C << sb;
        C << nl << "read = [response](::Ice::InputStream* istr)";
        C << sb;
        C << nl << "istr->startEncapsulation();";
        writeAllocateCode(C, outParams, p, true, clScope, _useWstring | TypeContextInParam | TypeContextCpp11);
        writeUnmarshalCode(C, outParams, p, true, _useWstring | TypeContextInParam | TypeContextCpp11);

        if(p->returnsClasses(false))
        {
            C << nl << "istr->readPendingValues();";
        }
        C << nl << "istr->endEncapsulation();";
        C << nl << "try" << sb;
        C << nl << "response" << spar;
        if(ret)
        {
            C << "ret";
        }
        for(ParamDeclList::const_iterator q = outParams.begin(); q != outParams.end(); ++q)
        {
            C << fixKwd(paramPrefix + (*q)->name());
        }
        C << epar << ";";
        C << eb;
        C << nl << "catch(...)";
        C << sb;
        C << nl << "throw ::std::current_exception();";
        C << eb;
        C << eb << ";";
        C << eb;
        C << nl << "auto outAsync = ::std::make_shared<::IceInternal::CustomLambdaOutgoing>(";
        C << "shared_from_this(), read, ex, sent);";
        C << sp;

        C << nl << "outAsync->invoke(" << flatName << ", ";
        C << operationModeToString(p->sendMode(), true) << ", " << opFormatTypeToString(p, true) << ", context,";
        C.inc();
        C << nl;

        writeInParamsLambda(C, p, inParams, clScope);
        C << "," << nl;
        throwUserExceptionLambda(C, p->throws(), clScope);

        C.dec();
        C << ");";
        C << nl << "return [outAsync]() { outAsync->cancel(); };";
        C << eb;
    }
    else
    {
        //
        // Simple implementation directly in header file
        //

        H << sb;
        if(futureOutParams.size() > 1)
        {
            H << nl << "auto _responseCb = [response](" << futureT << "&& _result)";
            H << sb;
            H << nl << responseParam << spar;

            if(ret)
            {
                H << condMove(isMovable(ret), string("_result.") + returnValueS);
            }
            for(ParamDeclList::const_iterator q = outParams.begin(); q != outParams.end(); ++q)
            {
                H << condMove(isMovable((*q)->type()), "_result." + fixKwd((*q)->name()));
            }
            H << epar << ";" << eb << ";";
        }

        H << nl << "return _makeLamdaOutgoing<" << futureT << ">" << spar;

        H << (futureOutParams.size() > 1 ? "_responseCb" : responseParam) << exParam << sentParam << "this";
        H << string("&" + getUnqualified(scoped, clScope.substr(2)) + "_iceI_" + name);
        for(ParamDeclList::const_iterator q = inParams.begin(); q != inParams.end(); ++q)
        {
            H << fixKwd((*q)->name());
        }
        H << contextParam << epar << ";";
        H << eb;
    }

    //
    // Private implementation
    //

    H << sp;
    H << nl << "/// \\cond INTERNAL";
    H << nl << _dllMemberExport << "void _iceI_" << name << spar;
    H << "const ::std::shared_ptr<::IceInternal::OutgoingAsyncT<" + futureT + ">>&";
    H << inParamsS;
    H << ("const " + getUnqualified("::Ice::Context&", clScope));
    H << epar << ";";
    H << nl << "/// \\endcond";

    C << sp;
    C << nl << "/// \\cond INTERNAL";
    C << nl << "void" << nl << scoped << "_iceI_" << name << spar;
    C << "const ::std::shared_ptr<::IceInternal::OutgoingAsyncT<" + futureT + ">>& outAsync";
    C << inParamsImplDecl << ("const " + getUnqualified("::Ice::Context&", clScope) + " context");
    C << epar;
    C << sb;
    if(p->returnsData())
    {
        C << nl << "_checkTwowayOnly(" << flatName << ");";
    }
    C << nl << "outAsync->invoke(" << flatName << ", ";
    C << getUnqualified(operationModeToString(p->sendMode(), true), clScope) << ", "
      << getUnqualified(opFormatTypeToString(p, true), clScope) << ", context,";
    C.inc();
    C << nl;

    writeInParamsLambda(C, p, inParams, clScope);
    C << "," << nl;
    throwUserExceptionLambda(C, p->throws(), clScope);

    if(futureOutParams.size() > 1)
    {
        //
        // Generate a read method if there are more than one ret/out parameter. If there's
        // only one, we rely on the default read method from LambdaOutgoing
        // except if the unique ret/out is optional or is an array/range.
        //
        C << "," << nl << "[](" << getUnqualified("::Ice::InputStream*", clScope) << " istr)";
        C << sb;
        C << nl << futureT << " v;";
        writeUnmarshalCode(C, outParams, p, false, _useWstring | TypeContextCpp11, "", returnValueS, "v");

        if(p->returnsClasses(false))
        {
            C << nl << "istr->readPendingValues();";
        }
        C << nl << "return v;";
        C << eb;
    }
    else if(outParamsHasOpt || p->returnsClasses(false))
    {
        //
        // If there's only one optional ret/out parameter, we still need to generate
        // a read method, we can't rely on the default read method which wouldn't
        // known which tag to use.
        //
        C << "," << nl << "[](" << getUnqualified("::Ice::InputStream*", clScope) << " istr)";
        C << sb;

        writeAllocateCode(C, outParams, p, true, clScope, _useWstring | TypeContextCpp11);
        writeUnmarshalCode(C, outParams, p, true, _useWstring | TypeContextCpp11);

        if(p->returnsClasses(false))
        {
            C << nl << "istr->readPendingValues();";
        }

        if(ret)
        {
            C << nl << "return ret;";
        }
        else
        {
            C << nl << "return " << fixKwd(paramPrefix + outParams.front()->name()) << ";";
        }
        C << eb;
    }

    C.dec();
    C << ");" << eb;
    C << nl << "/// \\endcond";
}

void
Slice::Gen::Cpp11TypesVisitor::visitEnum(const EnumPtr& p)
{
    bool unscoped = findMetaData(p->getMetaData(), TypeContextCpp11) == "%unscoped";
    H << sp;
    writeDocSummary(H, p);
    H << nl << "enum ";
    if(!unscoped)
    {
        H << "class ";
    }
    H << fixKwd(p->name());
    if(!unscoped && p->maxValue() <= 0xFF)
    {
        H << " : unsigned char";
    }
    H << sb;

    EnumeratorList enumerators = p->enumerators();
    //
    // Check if any of the enumerators were assigned an explicit value.
    //
    const bool explicitValue = p->explicitValue();
    for(EnumeratorList::const_iterator en = enumerators.begin(); en != enumerators.end();)
    {
        writeDocSummary(H, *en);
        H << nl << fixKwd((*en)->name());
        //
        // If any of the enumerators were assigned an explicit value, we emit
        // an explicit value for *all* enumerators.
        //
        if(explicitValue)
        {
            H << " = " << int64ToString((*en)->value());
        }
        if(++en != enumerators.end())
        {
            H << ',';
        }
    }
    H << eb << ';';
}

void
Slice::Gen::Cpp11TypesVisitor::visitConst(const ConstPtr& p)
{
    const string scope = fixKwd(p->scope());
    H << sp;
    writeDocSummary(H, p);
    H << nl << (isConstexprType(p->type()) ? "constexpr " : "const ")
      << typeToString(p->type(), scope, p->typeMetaData(), _useWstring | TypeContextCpp11) << " " << fixKwd(p->name())
      << " = ";
    writeConstantValue(H, p->type(), p->valueType(), p->value(), _useWstring | TypeContextCpp11, p->typeMetaData(),
                       scope);
    H << ';';
}

void
Slice::Gen::Cpp11TypesVisitor::emitUpcall(const ExceptionPtr& base, const string& call, const string& scope,
                                          bool isLocal)
{
    C << nl;
    if(base)
    {
        C << getUnqualified(fixKwd(base->scoped()), scope);
    }
    else
    {
        getUnqualified(isLocal ? "::Ice::LocalException" : "::Ice::UserException", scope);
    }
    C << call;
}

Slice::Gen::Cpp11ObjectVisitor::Cpp11ObjectVisitor(::IceUtilInternal::Output& h,
                                                   ::IceUtilInternal::Output& c,
                                                   const std::string& dllExport) :
    H(h),
    C(c),
    _dllExport(dllExport),
    _dllClassExport(toDllClassExport(dllExport)), _dllMemberExport(toDllMemberExport(dllExport)),
    _doneStaticSymbol(false),
    _useWstring(false)
{
}

void
Slice::Gen::Cpp11ObjectVisitor::emitDataMember(const DataMemberPtr& p)
{
    string name = fixKwd(p->name());
    int typeContext = _useWstring | TypeContextCpp11;
    ContainerPtr container = p->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    string scope = fixKwd(cl->scope());
    if(cl->isLocal())
    {
        typeContext |= TypeContextLocal;
    }

    writeDocSummary(H, p);
    H << nl << typeToString(p->type(), p->optional(), scope, p->getMetaData(), typeContext) << ' ' << name;

    string defaultValue = p->defaultValue();
    if(!defaultValue.empty())
    {
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(p->type());
        if(p->optional() && builtin && builtin->kind() == Builtin::KindString)
        {
            //
            // = "<string literal>" doesn't work for optional<std::string>
            //
            H << '{';
            writeConstantValue(H, p->type(), p->defaultValueType(), defaultValue, _useWstring | TypeContextCpp11,
                               p->getMetaData(), scope);
            H << '}';
        }
        else
        {
            H << " = ";
            writeConstantValue(H, p->type(), p->defaultValueType(), defaultValue, _useWstring | TypeContextCpp11,
                               p->getMetaData(), scope);
        }
    }
    H << ";";
}

void
Slice::Gen::Cpp11InterfaceVisitor::emitUpcall(const ClassDefPtr& base, const string& call, const string& scope)
{
    C << nl;
    if(base)
    {
        C << getUnqualified(fixKwd(base->scoped()), scope);
    }
    else
    {
        C << getUnqualified("::Ice::Object", scope);
    }
    C << call;
}

void
Slice::Gen::Cpp11ValueVisitor::emitUpcall(const ClassDefPtr& base, const string& call, const string& scope)
{
    C << nl;
    if(base)
    {
        C << getUnqualified(fixKwd(base->scoped()), scope);
    }
    else
    {
        C << getUnqualified("::Ice::Value", scope);
    }
    C << call;
}

Slice::Gen::Cpp11LocalObjectVisitor::Cpp11LocalObjectVisitor(::IceUtilInternal::Output& h,
                                                             ::IceUtilInternal::Output& c,
                                                             const std::string& dllExport) :
    Cpp11ObjectVisitor(h, c, dllExport)
{
}

bool
Slice::Gen::Cpp11LocalObjectVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasLocalClassDefs())
    {
        return false;
    }

    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);
    string name = fixKwd(p->name());
    H << sp << nl << "namespace " << name << nl << '{';
    return true;
}

void
Slice::Gen::Cpp11LocalObjectVisitor::visitModuleEnd(const ModulePtr&)
{
    H << sp;
    H << nl << '}';
    _useWstring = resetUseWstring(_useWstringHist);
}

bool
Slice::Gen::Cpp11LocalObjectVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(!p->isLocal())
    {
        return false;
    }

    string name = fixKwd(p->name());
    string scope = fixKwd(p->scope());
    if(p->isDelegate())
    {
        int typeCtx = _useWstring | TypeContextLocal | TypeContextCpp11;

        // A delegate only has one operation
        OperationPtr op = p->allOperations().front();

        // Generate alias
        H << sp;
        CommentPtr comment = op->parseComment(false);
        if(comment)
        {
            writeOpDocSummary(H, op, comment, OpDocAllParams, true, StringList(), StringList(), comment->returns());
        }
        H << nl << "using " << name << " = ";

        TypePtr ret = op->returnType();
        string retS = returnTypeToString(ret, op->returnIsOptional(), scope, op->getMetaData(), typeCtx);

        H << "::std::function<" << retS << "(";

        ParamDeclList paramList = op->parameters();
        for(ParamDeclList::iterator q = paramList.begin(); q != paramList.end(); ++q)
        {
            if((*q)->isOutParam())
            {
                H << outputTypeToString((*q)->type(), (*q)->optional(), scope, (*q)->getMetaData(), typeCtx);
            }
            else
            {
                H << inputTypeToString((*q)->type(), (*q)->optional(), scope, (*q)->getMetaData(), typeCtx);
            }
            H << " " << fixKwd((*q)->name());
            H << (IceUtilInternal::distance(q, paramList.end()) == 1  ? "" : ", ");
        }
        H << ")>;";

        return false;
    }

    string scoped = fixKwd(p->scoped());
    ClassList bases = p->bases();
    ClassDefPtr base;
    if(!bases.empty() && !bases.front()->isInterface())
    {
        base = bases.front();
    }
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList allDataMembers = p->allDataMembers();

    H << sp;
    writeDocSummary(H, p);
    H << nl << "class " << _dllClassExport << name;
    H.useCurrentPosAsIndent();
    if(!bases.empty())
    {
        H << " : ";
        ClassList::const_iterator q = bases.begin();
        bool virtualInheritance = p->isInterface();
        while(q != bases.end())
        {
            H << "public ";
            if(virtualInheritance || (*q)->isInterface())
            {
                H << "virtual ";
            }

            H << fixKwd((*q)->scoped());
            if(++q != bases.end())
            {
                H << ',' << nl;
            }
        }
    }

    H.restoreIndent();
    H << sb;
    H.dec();
    H << nl << "public:" << sp;
    H.inc();

    //
    // Out of line virtual dtor to avoid weak vtable
    //
    H << nl << _dllMemberExport << "virtual ~" << name  << "();";
    C << sp << nl << scoped.substr(2) << "::~" << name << "()";
    C << sb;
    C << eb;

    if(!p->isInterface())
    {
        if(p->hasDefaultValues())
        {
            H << sp << nl << name << "() :";
            H.inc();
            writeDataMemberInitializers(H, dataMembers, _useWstring | TypeContextCpp11 | TypeContextLocal);
            H.dec();
            H << sb;
            H << eb;
        }
        else
        {
            H << sp << nl << name << "() = default;";
        }

        H << sp << nl << name << "(const " << name << "&) = default;";
        H << nl << name << "(" << name << "&&) = default;";
        H << nl << name << "& operator=(const " << name << "&) = default;";
        H << nl << name << "& operator=(" << name << "&&) = default;";

        emitOneShotConstructor(p);
    }

    if(p->hasMetaData("cpp:comparable"))
    {
        H << sp;
        H << nl << "virtual bool operator==(const " << p->name() << "&) const = 0;";
        H << nl << "virtual bool operator<(const " << p->name() << "&) const = 0;";
    }
    return true;
}

void
Slice::Gen::Cpp11LocalObjectVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    string scoped = fixKwd(p->scoped());
    string scope = fixKwd(p->scope());
    ClassList bases = p->bases();
    ClassDefPtr base;
    if(!bases.empty() && !bases.front()->isInterface())
    {
        base = bases.front();
    }

    //
    // Emit data members. Access visibility may be specified by metadata.
    //
    bool inProtected = false;
    DataMemberList dataMembers = p->dataMembers();
    bool prot = p->hasMetaData("protected");
    bool needSp = true;
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        if(prot || (*q)->hasMetaData("protected"))
        {
            if(!inProtected)
            {
                H.dec();
                H << sp << nl << "protected:" << sp;
                H.inc();
                inProtected = true;
                needSp = false;
            }
        }
        else
        {
            if(inProtected)
            {
                H.dec();
                H << sp << nl << "public:" << sp;
                H.inc();
                inProtected = false;
                needSp = false;
            }
        }

        if(needSp)
        {
            H << sp;
            needSp = false;
        }

        emitDataMember(*q);
    }

    H << eb << ';';
}

bool
Slice::Gen::Cpp11LocalObjectVisitor::visitExceptionStart(const ExceptionPtr&)
{
    return false;
}

bool
Slice::Gen::Cpp11LocalObjectVisitor::visitStructStart(const StructPtr&)
{
    return false;
}

void
Slice::Gen::Cpp11LocalObjectVisitor::visitOperation(const OperationPtr& p)
{
    string name = p->name();
    string scoped = fixKwd(p->scoped());
    string scope = fixKwd(p->scope());

    int typeCtx = _useWstring | TypeContextLocal | TypeContextCpp11;
    TypePtr ret = p->returnType();
    string retS = returnTypeToString(ret, p->returnIsOptional(), scope, p->getMetaData(), typeCtx);

    string params = "(";

    ContainerPtr container = p->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    string classScope = fixKwd(cl->scope());

    string isConst = ((p->mode() == Operation::Nonmutating) || p->hasMetaData("cpp:const")) ? " const" : "";
    string noExcept = p->hasMetaData("cpp:noexcept") ? " noexcept" : "";

    string deprecateSymbol = getDeprecateSymbol(p, cl);

    CommentPtr comment = p->parseComment(false);

    if(cl->hasMetaData("async-oneway") || p->hasMetaData("async-oneway"))
    {
        vector<string> paramsDeclAMI;
        vector<string> outParamsDeclAMI;
        vector<string> paramsArgAMI;
        ParamDeclList paramList = p->inParameters();
        for(ParamDeclList::const_iterator r = paramList.begin(); r != paramList.end(); ++r)
        {
            string paramName = fixKwd((*r)->name());

            StringList metaData = (*r)->getMetaData();
            string typeString = inputTypeToString((*r)->type(), (*r)->optional(), classScope, metaData, typeCtx);
            paramsDeclAMI.push_back(typeString + ' ' + paramName);
            paramsArgAMI.push_back(paramName);
        }

        H << sp;
        if(comment)
        {
            writeOpDocSummary(H, p, comment, OpDocAllParams, true, StringList(), StringList(), comment->returns());
        }
        H << nl << deprecateSymbol << "virtual " << retS << ' ' << fixKwd(name) << spar << paramsDeclAMI << epar
                << isConst << noExcept;
        H << sb;
        H << nl << name << "Async" << spar << paramsArgAMI << epar << ".get();";
        H << eb;

        H << sp;
        if(comment)
        {
            string exParam = escapeParam(paramList, "exception");
            string sentParam = escapeParam(paramList, "sent");
            StringList postParams, returns;
            postParams.push_back("@param " + exParam + " The exception callback.");
            postParams.push_back("@param " + sentParam + " The sent callback.");
            returns.push_back("A function that can be called to cancel the invocation locally.");
            writeOpDocSummary(H, p, comment, OpDocInParams, false, StringList(), postParams, returns);
        }
        H << nl << "virtual ::std::function<void()>";
        H << nl << name << "Async(";
        H.useCurrentPosAsIndent();
        for(vector<string>::const_iterator i = paramsDeclAMI.begin(); i != paramsDeclAMI.end(); ++i)
        {
            if(i != paramsDeclAMI.begin())
            {
                H << nl;
            }
            H << *i << ",";
        }
        if(!paramsDeclAMI.empty())
        {
            H << nl;
        }
        H << "::std::function<void(::std::exception_ptr)> exception,";
        H << nl << "::std::function<void(bool)> sent = nullptr) = 0;";
        H.restoreIndent();

        H << sp;
        if(comment)
        {
            StringList returns;
            returns.push_back("The future object for the invocation.");
            writeOpDocSummary(H, p, comment, OpDocInParams, false, StringList(), StringList(), returns);
        }
        H << nl << "template<template<typename> class P = ::std::promise>";
        H << nl << deprecateSymbol << "auto " << name << "Async" << spar << paramsDeclAMI << epar;
        H.inc();
        H << nl << "-> decltype(::std::declval<P<void>>().get_future())";
        H.dec();
        H << sb;
        H << nl << "using Promise = P<void>;";
        H << nl << "auto promise = ::std::make_shared<Promise>();";

        H << nl << name << "Async(";
        H.useCurrentPosAsIndent();
        for(vector<string>::const_iterator i = paramsArgAMI.begin(); i != paramsArgAMI.end(); ++i)
        {
            if(i != paramsArgAMI.begin())
            {
                H << " ";
            }
            H << *i << ",";
        }
        if(!paramsArgAMI.empty())
        {
            H << nl;
        }
        H << "[promise](::std::exception_ptr ex)";
        H << sb;
        H << nl << "promise->set_exception(::std::move(ex));";
        H << eb << ",";
        H << nl << "[promise](bool)";
        H << sb;
        H << nl << "promise->set_value();";
        H << eb << ");";
        H.restoreIndent();

        H << nl << "return promise->get_future();";
        H << eb;
    }
    else
    {
        ParamDeclList paramList = p->parameters();
        for(ParamDeclList::iterator q = paramList.begin(); q != paramList.end(); ++q)
        {
            string paramName = fixKwd((*q)->name());
            TypePtr type = (*q)->type();
            string typeString;
            if((*q)->isOutParam())
            {
                typeString = outputTypeToString(type, (*q)->optional(), classScope, (*q)->getMetaData(), typeCtx);
            }
            else
            {
                typeString = inputTypeToString(type, (*q)->optional(), classScope, (*q)->getMetaData(), typeCtx);
            }

            if(q != paramList.begin())
            {
                params += ", ";
            }

            params += typeString;
            params += ' ';
            params += paramName;
        }

        params += ')';

        H << sp;
        if(comment)
        {
            writeOpDocSummary(H, p, comment, OpDocAllParams, true, StringList(), StringList(), comment->returns());
        }
        H << nl << deprecateSymbol << "virtual " << retS << ' ' << fixKwd(name) << params << isConst << noExcept
          << " = 0;";
    }
}

Slice::Gen::Cpp11InterfaceVisitor::Cpp11InterfaceVisitor(::IceUtilInternal::Output& h,
                                                         ::IceUtilInternal::Output& c,
                                                         const std::string& dllExport) :
    Cpp11ObjectVisitor(h, c, dllExport)
{
}

bool
Slice::Gen::Cpp11InterfaceVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalInterfaceDefs())
    {
        return false;
    }

    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);
    string name = fixKwd(p->name());
    H << sp << nl << "namespace " << name << nl << '{';
    return true;
}

void
Slice::Gen::Cpp11InterfaceVisitor::visitModuleEnd(const ModulePtr&)
{
    H << sp;
    H << nl << '}';

    _useWstring = resetUseWstring(_useWstringHist);
}

bool
Slice::Gen::Cpp11InterfaceVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal() || (!p->isInterface() && p->allOperations().empty()))
    {
        return false;
    }

    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    string suffix = p->isInterface() ? "" : "Disp";

    string name = fixKwd(p->name() + suffix);
    string scope = fixKwd(p->scope());
    string scoped = fixKwd(p->scope() + p->name() + suffix);
    ClassList bases = p->bases();
    ClassDefPtr base;
    if(!bases.empty() && !bases.front()->isInterface())
    {
        base = bases.front();
    }

    H << sp;
    writeDocSummary(H, p);
    H << nl << "class " << _dllExport << name << " : ";
    H.useCurrentPosAsIndent();
    if(bases.empty() || (base && bases.size() == 1 && base->allOperations().empty()))
    {
        H << "public virtual " << getUnqualified("::Ice::Object", scope);
    }
    else
    {
        if(base && base->allOperations().empty())
        {
            bases.pop_front();
        }

        ClassList::const_iterator q = bases.begin();
        while(q != bases.end())
        {
            string baseSuffix = (*q)->isInterface() ? "" : "Disp";
            string baseScoped = fixKwd((*q)->scope() + (*q)->name() + baseSuffix);

            H << "public virtual " << getUnqualified(baseScoped, scope);
            if(++q != bases.end())
            {
                H << ',' << nl;
            }
        }
    }

    H.restoreIndent();
    H << sb;
    H.dec();
    H << nl << "public:" << sp;
    H.inc();

    //
    // In C++, a nested type cannot have the same name as the enclosing type
    //
    if(name != "ProxyType")
    {
        H << nl << "using ProxyType = " << p->name() << "Prx;";
    }

    vector<string> params;
    vector<string> allTypes;
    vector<string> allParamDecls;

    ClassList allBases = p->allBases();
    StringList ids;
    transform(allBases.begin(), allBases.end(), back_inserter(ids), ::IceUtil::constMemFun(&Contained::scoped));
    StringList other;
    other.push_back(p->scoped());
    other.push_back("::Ice::Object");
    other.sort();
    ids.merge(other);
    ids.unique();
    StringList::const_iterator scopedIter = find(ids.begin(), ids.end(), p->scoped());
    assert(scopedIter != ids.end());

    H << sp;
    H << nl << "/**";
    H << nl << " * Determines whether this object supports an interface with the given Slice type ID.";
    H << nl << " * @param id The fully-scoped Slice type ID.";
    H << nl << " * @param current The Current object for the invocation.";
    H << nl << " * @return True if this object supports the interface, false, otherwise.";
    H << nl << " */";
    H << nl << "virtual bool ice_isA(::std::string id, const " << getUnqualified("::Ice::Current&", scope)
      << " current) const override;";
        H << sp;
    H << nl << "/**";
    H << nl << " * Obtains a list of the Slice type IDs representing the interfaces supported by this object.";
    H << nl << " * @param current The Current object for the invocation.";
    H << nl << " * @return A list of fully-scoped type IDs.";
    H << nl << " */";
    H << nl
      << "virtual ::std::vector<::std::string> ice_ids(const " << getUnqualified("::Ice::Current&", scope)
      << " current) const override;";
        H << sp;
    H << nl << "/**";
    H << nl << " * Obtains a Slice type ID representing the most-derived interface supported by this object.";
    H << nl << " * @param current The Current object for the invocation.";
    H << nl << " * @return A fully-scoped type ID.";
    H << nl << " */";
    H << nl << "virtual ::std::string ice_id(const " << getUnqualified("::Ice::Current&", scope)
      << " current) const override;";
        H << sp;
    H << nl << "/**";
    H << nl << " * Obtains the Slice type ID corresponding to this class.";
    H << nl << " * @return A fully-scoped type ID.";
    H << nl << " */";
    H << nl << "static const ::std::string& ice_staticId();";

    string flatName = "iceC" + p->flattenedScope() + p->name() + "_ids";

    C << sp;
    C << nl << "bool" << nl << scoped.substr(2) << "::ice_isA(::std::string s, const "
      << getUnqualified("::Ice::Current&", scope) << ") const";
    C << sb;
    C << nl << "return ::std::binary_search(" << flatName << ", " << flatName << " + " << ids.size() << ", s);";
    C << eb;

    C << sp;
    C << nl << "::std::vector<::std::string>" << nl << scoped.substr(2) << "::ice_ids(const "
      << getUnqualified("::Ice::Current&", scope) << ") const";
    C << sb;
    C << nl << "return ::std::vector<::std::string>(&" << flatName << "[0], &" << flatName << '[' << ids.size()
      << "]);";
    C << eb;

    C << sp;
    C << nl << "::std::string" << nl << scoped.substr(2) << "::ice_id(const " << getUnqualified("::Ice::Current&", scope)
      << ") const";
    C << sb;
    C << nl << "return ice_staticId();";
    C << eb;

    C << sp;
    C << nl << "const ::std::string&" << nl << scoped.substr(2) << "::ice_staticId()";
    C << sb;
    //
    // Use local static so that ice_staticId() is usable during static construction.
    //
    C << nl << "static const ::std::string typeId = \"" << *scopedIter << "\";";
    C << nl << "return typeId;";
    C << eb;
    return true;
}

void
Slice::Gen::Cpp11InterfaceVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    string suffix = p->isInterface() ? "" : "Disp";
    string scoped = fixKwd(p->scope() + p->name() + suffix);

    string scope = fixKwd(p->scope());
    string name = fixKwd(p->name() + suffix);
    ClassList bases = p->bases();
    ClassDefPtr base;
    if(!bases.empty() && !bases.front()->isInterface())
    {
        base = bases.front();
    }

    OperationList allOps = p->allOperations();
    if(!allOps.empty())
    {
        StringList allOpNames;
        transform(allOps.begin(), allOps.end(), back_inserter(allOpNames), ::IceUtil::constMemFun(&Contained::name));
        allOpNames.push_back("ice_id");
        allOpNames.push_back("ice_ids");
        allOpNames.push_back("ice_isA");
        allOpNames.push_back("ice_ping");
        allOpNames.sort();
        allOpNames.unique();

        string flatName = "iceC" + p->flattenedScope() + p->name() + "_ops";

        H << sp;
        H << nl << "/// \\cond INTERNAL";
        H << nl << "virtual bool _iceDispatch(::IceInternal::Incoming&, const "
          << getUnqualified("::Ice::Current&", scope) << ") override;";
        H << nl << "/// \\endcond";

        C << sp;
        C << nl << "/// \\cond INTERNAL";
        C << nl << "bool";
        C << nl << scoped.substr(2) << "::_iceDispatch(::IceInternal::Incoming& in, const "
          << getUnqualified("::Ice::Current&", scope) << " current)";
        C << sb;

        C << nl << "::std::pair<const ::std::string*, const ::std::string*> r = "
          << "::std::equal_range(" << flatName << ", " << flatName << " + " << allOpNames.size()
          << ", current.operation);";
        C << nl << "if(r.first == r.second)";
        C << sb;
        C << nl << "throw " << getUnqualified("::Ice::OperationNotExistException", scope)
          << "(__FILE__, __LINE__, current.id, current.facet, current.operation);";
        C << eb;
        C << sp;
        C << nl << "switch(r.first - " << flatName << ')';
        C << sb;
        int i = 0;
        for(StringList::const_iterator q = allOpNames.begin(); q != allOpNames.end(); ++q)
        {
            C << nl << "case " << i++ << ':';
            C << sb;
            C << nl << "return _iceD_" << *q << "(in, current);";
            C << eb;
        }
        C << nl << "default:";
        C << sb;
        C << nl << "assert(false);";
        C << nl << "throw " << getUnqualified("::Ice::OperationNotExistException", scope)
          << "(__FILE__, __LINE__, current.id, current.facet, current.operation);";
        C << eb;
        C << eb;
        C << eb;
        C << nl << "/// \\endcond";
    }

    H << eb << ';';

    _useWstring = resetUseWstring(_useWstringHist);
}

bool
Slice::Gen::Cpp11InterfaceVisitor::visitExceptionStart(const ExceptionPtr&)
{
    return false;
}

bool
Slice::Gen::Cpp11InterfaceVisitor::visitStructStart(const StructPtr&)
{
    return false;
}

void
Slice::Gen::Cpp11InterfaceVisitor::visitOperation(const OperationPtr& p)
{
    string name = p->name();

    TypePtr ret = p->returnType();

    vector<string> params;
    vector<string> args;

    vector<string> responseParams;
    vector<string> responseParamsDecl;
    vector<string> responseParamsImplDecl;

    ContainerPtr container = p->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    string classScope = fixKwd(cl->scope());

    string suffix = cl->isInterface() ? "" : "Disp";
    string scope = fixKwd(cl->scope() + cl->name() + suffix + "::");
    string scoped = fixKwd(cl->scope() + cl->name() + suffix + "::" + p->name());

    ParamDeclList inParams = p->inParameters();
    ParamDeclList outParams = p->outParameters();
    ParamDeclList paramList = p->parameters();

    const bool amd = (cl->hasMetaData("amd") || p->hasMetaData("amd"));

    const string returnValueParam = escapeParam(outParams, "returnValue");
    const string responsecbParam = escapeParam(inParams, "response");
    const string excbParam = escapeParam(inParams, "exception");
    const string currentParam = escapeParam(amd ? inParams : paramList, "current");
    const string currentTypeDecl = "const " + getUnqualified("::Ice::Current&", classScope);
    const string currentDecl = currentTypeDecl + " " + currentParam;

    CommentPtr comment = p->parseComment(false);

    if(ret)
    {
        string typeS = inputTypeToString(ret, p->returnIsOptional(), classScope, p->getMetaData(),
                                         _useWstring | TypeContextCpp11);
        responseParams.push_back(typeS + " " + returnValueParam);
        responseParamsDecl.push_back(typeS + " ret");
        responseParamsImplDecl.push_back(typeS + " ret");
    }

    string retS;
    if(amd)
    {
        retS = "void";
    }
    else if(p->hasMarshaledResult())
    {
        retS = resultStructName(name, "", true);
    }
    else
    {
        retS = returnTypeToString(ret, p->returnIsOptional(), classScope, p->getMetaData(),
                                  _useWstring | TypeContextCpp11);
    }

    for(ParamDeclList::iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        TypePtr type = (*q)->type();
        string paramName = fixKwd((*q)->name());
        bool isOutParam = (*q)->isOutParam();
        string typeString;
        int typeCtx = _useWstring | TypeContextCpp11;

        if(!isOutParam)
        {
            params.push_back(typeToString(type, (*q)->optional(), classScope, (*q)->getMetaData(),
                                          typeCtx | TypeContextInParam) + " " + paramName);
            args.push_back(condMove(isMovable(type) && !isOutParam, paramPrefix + (*q)->name()));
        }
        else
        {
            if(!p->hasMarshaledResult() && !amd)
            {
                params.push_back(
                    outputTypeToString(type, (*q)->optional(), classScope, (*q)->getMetaData(), typeCtx) + " " +
                    paramName);
                args.push_back(condMove(isMovable(type) && !isOutParam, paramPrefix + (*q)->name()));
            }

            string responseTypeS = inputTypeToString((*q)->type(), (*q)->optional(), classScope, (*q)->getMetaData(),
                                                     typeCtx);
            responseParams.push_back(responseTypeS + " " + paramName);
            responseParamsDecl.push_back(responseTypeS + " " + paramPrefix + (*q)->name());
            responseParamsImplDecl.push_back(responseTypeS + " " + paramPrefix + (*q)->name());
        }
    }
    if(amd)
    {
        if(p->hasMarshaledResult())
        {
            string resultName = resultStructName(name, "", true);
            params.push_back("::std::function<void(const " + resultName + "&)> " + responsecbParam);
            args.push_back("inA->response<" + resultName + ">()");
        }
        else
        {
            params.push_back("::std::function<void(" + joinString(responseParams, ", ") + ")> " + responsecbParam);
            args.push_back(ret || !outParams.empty() ? "responseCB" : "inA->response()");
        }
        params.push_back("::std::function<void(::std::exception_ptr)> " + excbParam);
        args.push_back("inA->exception()");
    }
    params.push_back(currentDecl);
    args.push_back("current");

    if(cl->isInterface())
    {
        emitOpNameResult(H, p, _useWstring);
    }

    if(p->hasMarshaledResult())
    {
        string resultName = resultStructName(name, "", true);
        H << sp;
        H << nl << "/**";
        H << nl << " * Marshaled result structure for operation " << (amd ? name + "Async" : fixKwd(name)) << ".";
        H << nl << " */";
        H << nl << "class " << resultName << " : public " << getUnqualified("::Ice::MarshaledResult", classScope);
        H << sb;
        H.dec();
        H << nl << "public:";
        H.inc();
        H << nl << "/**";
        H << nl << " * Marshals the results immediately.";
        if(ret && comment && !comment->returns().empty())
        {
            H << nl << " * @param " << returnValueParam << " " << getDocSentence(comment->returns());
        }
        map<string, StringList> paramComments;
        if(comment)
        {
            paramComments = comment->parameters();
        }
        const string mrcurrent = escapeParam(outParams, "current");
        for(ParamDeclList::iterator q = outParams.begin(); q != outParams.end(); ++q)
        {
            map<string, StringList>::iterator r = paramComments.find((*q)->name());
            if(r != paramComments.end())
            {
                H << nl << " * @param " << fixKwd(r->first) << " " << getDocSentence(r->second);
            }
        }
        H << nl << " * @param " << mrcurrent << " The Current object for the invocation.";
        H << nl << " */";
        H << nl << resultName << spar << responseParams << currentTypeDecl + " " + mrcurrent << epar << ";";
        H << eb << ';';

        C << sp << nl << scope.substr(2) << resultName << "::" << resultName;
        C << spar << responseParamsImplDecl << currentTypeDecl + " current" << epar << ":";
        C.inc();
        C << nl << "MarshaledResult(current)";
        C.dec();
        C << sb;
        C << nl << "ostr->startEncapsulation(current.encoding, " << opFormatTypeToString(p, true) << ");";
        writeMarshalCode(C, outParams, p, true, TypeContextCpp11, "ostr");
        if(p->returnsClasses(false))
        {
            C << nl << "ostr->writePendingValues();";
        }
        C << nl << "ostr->endEncapsulation();";
        C << eb;
    }

    string isConst = ((p->mode() == Operation::Nonmutating) || p->hasMetaData("cpp:const")) ? " const" : "";

    string opName = amd ? (name + "Async") : fixKwd(name);
    string deprecateSymbol = getDeprecateSymbol(p, cl);

    H << sp;
    if(comment)
    {
        OpDocParamType pt = (amd || p->hasMarshaledResult()) ? OpDocInParams : OpDocAllParams;
        StringList postParams, returns;
        if(amd)
        {
            postParams.push_back("@param " + responsecbParam + " The response callback.");
            postParams.push_back("@param " + excbParam + " The exception callback.");
        }
        else if(p->hasMarshaledResult())
        {
            returns.push_back("The marshaled result structure.");
        }
        else if(!amd)
        {
            returns = comment->returns();
        }
        postParams.push_back("@param " + currentParam + " The Current object for the invocation.");
        writeOpDocSummary(H, p, comment, pt, true, StringList(), postParams, returns);
    }
    H << nl << deprecateSymbol << "virtual " << retS << ' ' << opName << spar << params << epar << isConst << " = 0;";
    H << nl << "/// \\cond INTERNAL";
    H << nl << "bool _iceD_" << name << "(::IceInternal::Incoming&, const "
      << getUnqualified("::Ice::Current&", classScope) << ")" << isConst << ';';
    H << nl << "/// \\endcond";

    C << sp;
    C << nl << "/// \\cond INTERNAL";
    C << nl << "bool";
    C << nl << scope.substr(2);
    C << "_iceD_" << name << "(::IceInternal::Incoming& inS, const "
      << getUnqualified("::Ice::Current&", classScope) << " current)" << isConst;
    C << sb;
    C << nl << "_iceCheckMode(" << getUnqualified(operationModeToString(p->mode(), true), classScope)
      << ", current.mode);";

    if(!inParams.empty())
    {
        C << nl << "auto istr = inS.startReadParams();";
        writeAllocateCode(C, inParams, 0, true, classScope, _useWstring | TypeContextInParam | TypeContextCpp11);
        writeUnmarshalCode(C, inParams, 0, true, _useWstring | TypeContextInParam | TypeContextCpp11);
        if(p->sendsClasses(false))
        {
            C << nl << "istr->readPendingValues();";
        }
        C << nl << "inS.endReadParams();";
    }
    else
    {
        C << nl << "inS.readEmptyParams();";
    }
    if(p->format() != DefaultFormat)
    {
        C << nl << "inS.setFormat(" << opFormatTypeToString(p, true) << ");";
    }

    if(!amd)
    {
        if(p->hasMarshaledResult())
        {
            C << nl << "inS.setMarshaledResult(";
        }
        else
        {
            writeAllocateCode(C, outParams, 0, true, classScope, _useWstring | TypeContextCpp11);
            if(ret)
            {
                C << nl << retS << " ret = ";
            }
            else
            {
                C << nl;
            }
        }

        C << "this->" << opName << spar << args << epar;
        if(p->hasMarshaledResult())
        {
            C << ");";
        }
        else
        {
            C << ";";
            if(ret || !outParams.empty())
            {
                C << nl << "auto ostr = inS.startWriteParams();";
                writeMarshalCode(C, outParams, p, true, TypeContextCpp11);
                if(p->returnsClasses(false))
                {
                    C << nl << "ostr->writePendingValues();";
                }
                C << nl << "inS.endWriteParams();";
            }
            else
            {
                C << nl << "inS.writeEmptyParams();";
            }
        }
        C << nl << "return true;";
    }
    else
    {
        C << nl << "auto inA = ::IceInternal::IncomingAsync::create(inS);";
        if(!p->hasMarshaledResult() && (ret || !outParams.empty()))
        {
            C << nl << "auto responseCB = [inA]" << spar << responseParamsDecl << epar;
            C << sb;
            C << nl << "auto ostr = inA->startWriteParams();";
            writeMarshalCode(C, outParams, p, true, TypeContextCpp11);
            if(p->returnsClasses(false))
            {
                C << nl << "ostr->writePendingValues();";
            }
            C << nl << "inA->endWriteParams();";
            C << nl << "inA->completed();";
            C << eb << ';';
        }
        C << nl << "this->" << opName << spar << args << epar << ';';
        C << nl << "return false;";
    }
    C << eb;
    C << nl << "/// \\endcond";
}

Slice::Gen::Cpp11ValueVisitor::Cpp11ValueVisitor(::IceUtilInternal::Output& h,
                                                 ::IceUtilInternal::Output& c,
                                                 const std::string& dllExport) :
    Cpp11ObjectVisitor(h, c, dllExport)
{
}

bool
Slice::Gen::Cpp11ValueVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasValueDefs())
    {
        return false;
    }

    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);
    string name = fixKwd(p->name());
    H << sp << nl << "namespace " << name << nl << '{';
    return true;
}

void
Slice::Gen::Cpp11ValueVisitor::visitModuleEnd(const ModulePtr&)
{
    H << sp;
    H << nl << '}';

    _useWstring = resetUseWstring(_useWstringHist);
}

bool
Slice::Gen::Cpp11ValueVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal() || p->isInterface())
    {
        return false;
    }
    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    string name = fixKwd(p->name());
    string scope = fixKwd(p->scope());
    string scoped = fixKwd(p->scoped());
    ClassList bases = p->bases();
    ClassDefPtr base;
    if(!bases.empty())
    {
        base = bases.front();
    }
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList allDataMembers = p->allDataMembers();

    H << sp;
    writeDocSummary(H, p);
    H << nl << "class " << _dllClassExport << name << " : public " << getUnqualified("::Ice::ValueHelper", scope)
      << "<" << name << ", ";

    if(!base || (base && base->isInterface()))
    {
        H << getUnqualified("::Ice::Value", scope);
    }
    else
    {
        H << getUnqualified(fixKwd(base->scoped()), scope);
    }
    H << ">";
    H << sb;
    H.dec();
    H << nl << "public:" << sp;
    H.inc();

    // Out of line dtor to avoid weak vtable
    H << nl << _dllMemberExport << "virtual ~" << name << "();";
    C << sp;
    C << nl << scoped.substr(2) << "::~" << name << "()";
    C << sb;
    C << eb;

    vector<string> params;

    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        params.push_back(fixKwd((*q)->name()));
    }

    H << sp << nl << name << "() = default;";

    H << sp << nl << name << "(const " << name << "&) = default;";
    H << nl << name << "(" << name << "&&) = default;";
    H << nl << name << "& operator=(const " << name << "&) = default;";
    H << nl << name << "& operator=(" << name << "&&) = default;";

    emitOneShotConstructor(p);

    H << sp;
    H << nl << "/**";
    H << nl << " * Obtains a tuple containing all of the value's data members.";
    H << nl << " * @return The data members in a tuple.";
    H << nl << " */";
    writeIceTuple(H, fixKwd(p->scope()), p->allDataMembers(), _useWstring);

    H << sp;
    H << nl << "/**";
    H << nl << " * Obtains the Slice type ID of this value.";
    H << nl << " * @return The fully-scoped type ID.";
    H << nl << " */";
    H << nl << _dllMemberExport << "static const ::std::string& ice_staticId();";
    return true;
}

void
Slice::Gen::Cpp11ValueVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    string scoped = fixKwd(p->scoped());
    string scope = fixKwd(p->scope());
    string name = fixKwd(p->name());
    ClassList bases = p->bases();
    ClassDefPtr base;
    if(!bases.empty() && !bases.front()->isInterface())
    {
        base = bases.front();
    }
    bool basePreserved = p->inheritsMetaData("preserve-slice");
    bool preserved = p->hasMetaData("preserve-slice");

    if(preserved && !basePreserved)
    {
        H << sp;
        H << nl << "/**";
        H << nl << " * Obtains the SlicedData object created when an unknown value type was marshaled";
        H << nl << " * in the sliced format and the Ice run time sliced it to a known type.";
        H << nl << " * @return The SlicedData object, or nil if the value was not sliced or was not";
        H << nl << " * marshaled in the sliced format.";
        H << nl << " */";
        H << nl << "virtual ::std::shared_ptr<" << getUnqualified("::Ice::SlicedData", scope)
          << "> ice_getSlicedData() const override;";

        C << sp;
        C << nl << "::std::shared_ptr<::Ice::SlicedData>" << nl << scoped.substr(2) << "::ice_getSlicedData() const";
        C << sb;
        C << nl << "return _iceSlicedData;";
        C << eb;

        H << sp;
        H << nl << "/// \\cond STREAM";
        H << nl << "virtual void _iceWrite(" << getUnqualified("::Ice::OutputStream*", scope) << ") const override;";
        H << nl << "virtual void _iceRead(" << getUnqualified("::Ice::InputStream*", scope) << ") override;";
        H << nl << "/// \\endcond";

        C << sp;
        C << nl << "/// \\cond STREAM";
        C << nl << "void" << nl << scoped.substr(2) << "::_iceWrite(" << getUnqualified("::Ice::OutputStream*", scope)
          << " ostr) const";
        C << sb;
        C << nl << "ostr->startValue(_iceSlicedData);";
        C << nl << "_iceWriteImpl(ostr);";
        C << nl << "ostr->endValue();";
        C << eb;

        C << sp;
        C << nl << "void" << nl << scoped.substr(2) << "::_iceRead(" << getUnqualified("::Ice::InputStream*", scope)
          << " istr)";
        C << sb;
        C << nl << "istr->startValue();";
        C << nl << "_iceReadImpl(istr);";
        C << nl << "_iceSlicedData = istr->endValue(true);";
        C << eb;
        C << nl << "/// \\endcond";
    }

    C << sp;
    C << nl << "const ::std::string&" << nl << scoped.substr(2) << "::ice_staticId()";
    C << sb;
    //
    // Use local static so that ice_staticId() is usable during static construction.
    //
    C << nl << "static const ::std::string typeId = \"" << p->scoped() << "\";";
    C << nl << "return typeId;";
    C << eb;

    //
    // Emit data members. Access visibility may be specified by metadata.
    //
    bool inProtected = false;
    bool generateFriend = false;
    DataMemberList dataMembers = p->dataMembers();
    bool prot = p->hasMetaData("protected");
    bool needSp = true;

    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        if(prot || (*q)->hasMetaData("protected"))
        {
            if(!inProtected)
            {
                H.dec();
                H << sp << nl << "protected:" << sp;
                H.inc();
                inProtected = true;
                needSp = false;
            }
            generateFriend = true;
        }
        else
        {
            if(inProtected)
            {
                H.dec();
                H << sp << nl << "public:" << sp;
                H.inc();
                inProtected = false;
                needSp = false;
            }
        }

        if(needSp)
        {
            H << sp;
            needSp = false;
        }

        emitDataMember(*q);
    }

    if(preserved && !basePreserved)
    {
        if(!inProtected)
        {
            H.dec();
            H << sp << nl << "protected:";
            H.inc();
            inProtected = true;
        }
        H << sp;
        H << nl << "/// \\cond STREAM";
        H << nl << "::std::shared_ptr<" << getUnqualified("::Ice::SlicedData", scope) << "> _iceSlicedData;";
        H << nl << "/// \\endcond";
    }

    if(generateFriend)
    {
        if(!inProtected)
        {
            H.dec();
            H << sp << nl << "protected:";
            H.inc();
            inProtected = true;
        }

        H << sp;
        H << nl << "template<typename T, typename S>";
        H << nl << "friend struct Ice::StreamWriter;";
        H << nl << "template<typename T, typename S>";
        H << nl << "friend struct Ice::StreamReader;";
    }

    H << eb << ';';

    if(!_doneStaticSymbol)
    {
        //
        // We need an instance here to trigger initialization if the implementation is in a static library.
        // But we do this only once per source file, because a single instance is sufficient to initialize
        // all of the globals in a compilation unit.
        //
        _doneStaticSymbol = true;
        H << sp;
        H << nl << "/// \\cond INTERNAL";
        H << nl << "static " << fixKwd(p->name()) << " _iceS_" << p->name() << "_init;";
        H << nl << "/// \\endcond";
    }

    _useWstring = resetUseWstring(_useWstringHist);
}

bool
Slice::Gen::Cpp11ValueVisitor::visitExceptionStart(const ExceptionPtr&)
{
    return false;
}

bool
Slice::Gen::Cpp11ValueVisitor::visitStructStart(const StructPtr&)
{
    return false;
}

void
Slice::Gen::Cpp11ValueVisitor::visitOperation(const OperationPtr& p)
{
    emitOpNameResult(H, p, _useWstring);
}

bool
Slice::Gen::Cpp11ObjectVisitor::emitVirtualBaseInitializers(const ClassDefPtr& derived, const ClassDefPtr& base)
{
    const string scope = fixKwd(derived->scope());
    DataMemberList allDataMembers = base->allDataMembers();
    if(allDataMembers.empty())
    {
        return false;
    }

    string upcall = "(";
    for(DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
    {
        if(q != allDataMembers.begin())
        {
            upcall += ", ";
        }
        if(isMovable((*q)->type()))
        {
            upcall += "::std::move(" + fixKwd((*q)->name()) + ")";
        }
        else
        {
            upcall += "" + fixKwd((*q)->name());
        }
    }
    upcall += ")";

    if(base->isLocal())
    {
        H << nl << getUnqualified(fixKwd(base->scoped()), scope);
    }
    else
    {
        H << nl << "Ice::ValueHelper<" << getUnqualified(fixKwd(derived->scoped()), scope)
          << ", " << getUnqualified(fixKwd(base->scoped()), scope) << ">";
    }
    H << upcall;
    return true;
}

void
Slice::Gen::Cpp11ObjectVisitor::emitOneShotConstructor(const ClassDefPtr& p)
{
    DataMemberList allDataMembers = p->allDataMembers();
    string scope = fixKwd(p->scope());
    if(!allDataMembers.empty())
    {
        vector<string> allParamDecls;
        map<string, CommentPtr> allComments;
        DataMemberList dataMembers = p->dataMembers();

        int typeContext = _useWstring | TypeContextCpp11;
        if(p->isLocal())
        {
            typeContext |= TypeContextLocal;
        }

        for(DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
        {
            string typeName =
                inputTypeToString((*q)->type(), (*q)->optional(), scope, (*q)->getMetaData(), typeContext);
            allParamDecls.push_back(typeName + " " + fixKwd((*q)->name()));
            CommentPtr comment = (*q)->parseComment(false);
            if(comment)
            {
                allComments[(*q)->name()] = comment;
            }
        }

        CommentPtr comment = p->parseComment(false);

        H << sp;
        H << nl << "/**";
        H << nl << " * One-shot constructor to initialize all data members.";
        for(DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
        {
            map<string, CommentPtr>::iterator r = allComments.find((*q)->name());
            if(r != allComments.end())
            {
                H << nl << " * @param " << fixKwd(r->first) << " " << getDocSentence(r->second->overview());
            }
        }
        H << nl << " */";
        H << nl;
        if(allParamDecls.size() == 1)
        {
            H << "explicit ";
        }
        H << fixKwd(p->name()) << spar << allParamDecls << epar << " :";
        H.inc();

        ClassList bases = p->bases();

        if(!bases.empty() && !bases.front()->isInterface())
        {
            if(emitVirtualBaseInitializers(p, bases.front()))
            {
                if(!dataMembers.empty())
                {
                    H << ',';
                }
            }
        }

        if(!dataMembers.empty())
        {
            H << nl;
        }

        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            if(q != dataMembers.begin())
            {
                H << ',' << nl;
            }
            string memberName = fixKwd((*q)->name());
            if(isMovable((*q)->type()))
            {
                H << memberName << "(::std::move(" << memberName << "))";
            }
            else
            {
                H << memberName << "(" << memberName << ')';
            }
        }

        H.dec();
        H << sb;
        H << eb;
    }
}

Slice::Gen::Cpp11StreamVisitor::Cpp11StreamVisitor(Output& h, Output& c, const string& dllExport) :
    H(h),
    C(c),
    _dllExport(dllExport)
{
}

bool
Slice::Gen::Cpp11StreamVisitor::visitModuleStart(const ModulePtr& m)
{
    if(!m->hasNonLocalContained(Contained::ContainedTypeStruct) &&
       !m->hasNonLocalContained(Contained::ContainedTypeEnum) &&
       !m->hasNonLocalContained(Contained::ContainedTypeException) &&
       !m->hasNonLocalContained(Contained::ContainedTypeClass))
    {
        return false;
    }

    if(UnitPtr::dynamicCast(m->container()))
    {
        //
        // Only emit this for the top-level module.
        //
        H << sp;
        H << nl << "/// \\cond STREAM";
        H << nl << "namespace Ice" << nl << '{' << sp;

        if(m->hasNonLocalContained(Contained::ContainedTypeStruct))
        {
            C << sp;
            C << nl << "namespace Ice" << nl << '{';
        }
    }
    return true;
}

void
Slice::Gen::Cpp11StreamVisitor::visitModuleEnd(const ModulePtr& m)
{
    if(UnitPtr::dynamicCast(m->container()))
    {
        //
        // Only emit this for the top-level module.
        //
        H << nl << '}';
        H << nl << "/// \\endcond";
        if(m->hasNonLocalContained(Contained::ContainedTypeStruct))
        {
            C << nl << '}';
        }
    }
}

bool
Slice::Gen::Cpp11StreamVisitor::visitStructStart(const StructPtr& p)
{
    if(p->isLocal())
    {
        return false;
    }

    string scoped = fixKwd(p->scoped());

    H << nl << "template<>";
    H << nl << "struct StreamableTraits<" << scoped << ">";
    H << sb;
    H << nl << "static const StreamHelperCategory helper = StreamHelperCategoryStruct;";
    H << nl << "static const int minWireSize = " << p->minWireSize() << ";";
    H << nl << "static const bool fixedLength = " << (p->isVariableLength() ? "false" : "true") << ";";
    H << eb << ";" << nl;

    writeStreamHelpers(H, p, p->dataMembers(), false, false, true);

    return false;
}

bool
Slice::Gen::Cpp11StreamVisitor::visitClassDefStart(const ClassDefPtr& c)
{
    if(!c->isLocal() && !c->isInterface())
    {
        writeStreamHelpers(H,c, c->dataMembers(), c->hasBaseDataMembers(), true, true);
    }
    return false;
}

void
Slice::Gen::Cpp11StreamVisitor::visitExceptionEnd(const ExceptionPtr& p)
{
    if(!p->isLocal())
    {
        writeStreamHelpers(H,p, p->dataMembers(), p->hasBaseDataMembers(), true, true);
    }
}

void
Slice::Gen::Cpp11StreamVisitor::visitEnum(const EnumPtr& p)
{
    if(!p->isLocal())
    {
        string scoped = fixKwd(p->scoped());
        H << nl << "template<>";
        H << nl << "struct StreamableTraits< " << scoped << ">";
        H << sb;
        H << nl << "static const StreamHelperCategory helper = StreamHelperCategoryEnum;";
        H << nl << "static const int minValue = " << p->minValue() << ";";
        H << nl << "static const int maxValue = " << p->maxValue() << ";";
        H << nl << "static const int minWireSize = " << p->minWireSize() << ";";
        H << nl << "static const bool fixedLength = false;";
        H << eb << ";" << nl;
    }
}

Slice::Gen::Cpp11CompatibilityVisitor::Cpp11CompatibilityVisitor(Output& h, Output&, const string& dllExport) :
    H(h),
    _dllExport(dllExport)
{
}

bool
Slice::Gen::Cpp11CompatibilityVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasClassDecls())
    {
        return false;
    }

    string name = fixKwd(p->name());

    H << sp;
    H << nl << "/// \\cond INTERNAL";
    H << nl << "namespace " << name << nl << '{';
    return true;
}

void
Slice::Gen::Cpp11CompatibilityVisitor::visitModuleEnd(const ModulePtr&)
{
    H << sp;
    H << nl << '}';
    H << nl << "/// \\endcond";
}

void
Slice::Gen::Cpp11CompatibilityVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    if(p->definition() && p->definition()->isDelegate())
    {
        return;
    }

    string name = fixKwd(p->name());
    string scoped = fixKwd(p->scoped());

    H << sp << nl << "using " << p->name() << "Ptr = ::std::shared_ptr<" << name << ">;";

    if(!p->isLocal())
    {
        ClassDefPtr def = p->definition();
        if(p->isInterface() || (def && !def->allOperations().empty()))
        {
            H << nl << "using " << p->name() << "PrxPtr = ::std::shared_ptr<" << p->name() << "Prx>;";
        }
    }
}

Slice::Gen::Cpp11ImplVisitor::Cpp11ImplVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport), _useWstring(false)
{
}

string
Slice::Gen::Cpp11ImplVisitor::defaultValue(const TypePtr& type, const string& scope, const StringList& metaData) const
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindBool:
            {
                return "false";
            }
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindInt:
            case Builtin::KindLong:
            {
                return "0";
            }
            case Builtin::KindFloat:
            case Builtin::KindDouble:
            {
                return "0.0";
            }
            case Builtin::KindString:
            {
                return "::std::string()";
            }
            case Builtin::KindValue:
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindLocalObject:
            {
                return "nullptr";
            }
        }
    }
    else
    {
        ProxyPtr prx = ProxyPtr::dynamicCast(type);

        if(ProxyPtr::dynamicCast(type) || ClassDeclPtr::dynamicCast(type))
        {
            return "nullptr";
        }

        StructPtr st = StructPtr::dynamicCast(type);
        if(st)
        {
            return getUnqualified(fixKwd(st->scoped()), scope) + "()";
        }

        EnumPtr en = EnumPtr::dynamicCast(type);
        if(en)
        {
            EnumeratorList enumerators = en->enumerators();
            return getUnqualified(fixKwd(en->scoped() + "::" + enumerators.front()->name()), scope);
        }

        SequencePtr seq = SequencePtr::dynamicCast(type);
        if(seq)
        {
            return typeToString(seq, scope, metaData, _useWstring | TypeContextCpp11) + "()";
        }

        DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
        if(dict)
        {
            return getUnqualified(fixKwd(dict->scoped()), scope) + "()";
        }
    }

    assert(false);
    return "???";
}

bool
Slice::Gen::Cpp11ImplVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasClassDefs())
    {
        return false;
    }

    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    string name = fixKwd(p->name());

    H << sp << nl << "namespace " << name << nl << '{';

    return true;
}

void
Slice::Gen::Cpp11ImplVisitor::visitModuleEnd(const ModulePtr&)
{
    H << sp;
    H << nl << '}';

    _useWstring = resetUseWstring(_useWstringHist);
}

bool
Slice::Gen::Cpp11ImplVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(!p->isAbstract())
    {
        return false;
    }

    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    string name = p->name();
    string scoped = fixKwd(p->scoped());
    string scope = fixKwd(p->scope());
    string cls = scope.substr(2) + name + "I";
    ClassList bases = p->bases();

    H << sp;
    H << nl << "class " << name << "I : ";
    H.useCurrentPosAsIndent();
    H << "public virtual ";

    if(p->isInterface() || p->isLocal())
    {
        H << fixKwd(name);
    }
    else
    {
        H << fixKwd(name + "Disp");
    }
    H.restoreIndent();

    H << sb;
    H.dec();
    H << nl << "public:";
    H.inc();

    OperationList ops = p->allOperations();

    for(OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = (*r);
        string opName = op->name();

        TypePtr ret = op->returnType();
        string retS = op->hasMarshaledResult() ?
            scoped + "::" + resultStructName(opName, "", true) :
            returnTypeToString(ret, op->returnIsOptional(), "", op->getMetaData(), _useWstring | TypeContextCpp11);

        ParamDeclList params = op->parameters();
        ParamDeclList outParams;
        ParamDeclList inParams;
        for(ParamDeclList::const_iterator q = params.begin(); q != params.end(); ++q)
        {
            if((*q)->isOutParam())
            {
                outParams.push_back(*q);
            }
            else
            {
                inParams.push_back(*q);
            }
        }

        if(!p->isLocal() && (p->hasMetaData("amd") || op->hasMetaData("amd")))
        {
            string responseParams;

            H << sp << nl << "virtual void " << opName << "Async(";
            H.useCurrentPosAsIndent();
            for(ParamDeclList::const_iterator q = inParams.begin(); q != inParams.end(); ++q)
            {
                H << typeToString((*q)->type(), (*q)->optional(), scope,
                                  (*q)->getMetaData(), _useWstring | TypeContextInParam | TypeContextCpp11)
                  << "," << nl;
            }

            if(op->hasMarshaledResult())
            {
                responseParams = "const " + scoped + "::" + resultStructName(opName, "", true) + "&";
            }
            else
            {
                if(ret)
                {
                    responseParams = inputTypeToString(ret, op->returnIsOptional(), scope, op->getMetaData(),
                                                       _useWstring | TypeContextCpp11);
                    if(!outParams.empty())
                    {
                        responseParams += ", ";
                    }
                }

                for(ParamDeclList::iterator q = outParams.begin(); q != outParams.end(); ++q)
                {
                    if(q != outParams.begin())
                    {
                        responseParams += ", ";
                    }
                    responseParams += inputTypeToString((*q)->type(), (*q)->optional(), scope, (*q)->getMetaData(),
                                                        _useWstring | TypeContextCpp11);
                }
            }

            string isConst = ((op->mode() == Operation::Nonmutating) || op->hasMetaData("cpp:const")) ? " const" : "";

            H << "std::function<void(" << responseParams << ")>,";
            H << nl << "std::function<void(std::exception_ptr)>,";
            H << nl << "const Ice::Current&)" << isConst << " override;";
            H.restoreIndent();

            C << sp << nl << "void" << nl << scope << name << "I::" << opName << "Async(";
            C.useCurrentPosAsIndent();
            for(ParamDeclList::const_iterator q = inParams.begin(); q != inParams.end(); ++q)
            {
                C << typeToString((*q)->type(), (*q)->optional(), scope, (*q)->getMetaData(),
                                  _useWstring | TypeContextInParam | TypeContextCpp11);
                C << ' ' << fixKwd((*q)->name()) << "," << nl;
            }

            C << "std::function<void(" << responseParams << ")> " << opName << "_response,";
            C << nl << "std::function<void(std::exception_ptr)>,";
            C << nl << "const Ice::Current& current)" << isConst;
            C.restoreIndent();
            C << sb;

             C << nl << opName << "_response";
            if(op->hasMarshaledResult())
            {
                C << "(" << scoped + "::" + resultStructName(opName, "", true);
            }
            C << spar;
            if(ret)
            {
                C << defaultValue(ret, scope, op->getMetaData());
            }
            for(ParamDeclList::const_iterator q = outParams.begin(); q != outParams.end(); ++q)
            {
                C << defaultValue((*q)->type(), scope, op->getMetaData());
            }

            if(op->hasMarshaledResult())
            {
                C << "current" << epar << ");";
            }
            else
            {
                C << epar << ';';
            }

            C << eb;
        }
        else
        {
            H << sp << nl << "virtual " << getUnqualified(retS, scope) << ' ' << fixKwd(opName) << '(';
            H.useCurrentPosAsIndent();
            ParamDeclList paramList = op->hasMarshaledResult() ? inParams : op->parameters();
            for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
            {
                if(q != paramList.begin())
                {
                    H << ',' << nl;
                }
                StringList metaData = (*q)->getMetaData();
                string typeString;
                if((*q)->isOutParam())
                {
                    typeString = outputTypeToString((*q)->type(), (*q)->optional(), scope, metaData,
                                                    _useWstring | TypeContextCpp11);
                }
                else
                {
                    typeString = typeToString((*q)->type(), (*q)->optional(), scope, metaData,
                                              _useWstring | TypeContextInParam | TypeContextCpp11);
                }
                H << typeString;
            }
            if(!p->isLocal())
            {
                if(!paramList.empty())
                {
                    H << ',' << nl;
                }
                H << "const Ice::Current&";
            }
            H.restoreIndent();

            string isConst = ((op->mode() == Operation::Nonmutating) || op->hasMetaData("cpp:const")) ? " const" : "";

            H << ")" << isConst << " override;";

            C << sp << nl << retS << nl;
            C << scope.substr(2) << name << "I::" << fixKwd(opName) << '(';
            C.useCurrentPosAsIndent();
            for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
            {
                if(q != paramList.begin())
                {
                    C << ',' << nl;
                }
                StringList metaData = (*q)->getMetaData();
                string typeString;
                if((*q)->isOutParam())
                {
                    C << outputTypeToString((*q)->type(), (*q)->optional(), scope, metaData, _useWstring | TypeContextCpp11)
                      << " "
                      << fixKwd((*q)->name());
                }
                else
                {
                    C << typeToString((*q)->type(), (*q)->optional(), scope, metaData,
                                      _useWstring | TypeContextInParam | TypeContextCpp11)
                      << " /*" << fixKwd((*q)->name()) << "*/";
                }
            }
            if(!p->isLocal())
            {
                if(!paramList.empty())
                {
                    C << ',' << nl;
                }
                C << "const Ice::Current& current";
            }
            C.restoreIndent();
            C << ')';
            C << isConst;
            C << sb;

            if(op->hasMarshaledResult())
            {
                if(ret || !outParams.empty())
                {
                    C << nl << "return " << scoped << "::" << resultStructName(opName, "", true) << "(";
                    if(ret)
                    {
                        C << defaultValue(ret, scope, op->getMetaData());
                        if(!outParams.empty())
                        {
                            C << ", ";
                        }
                    }

                    for(ParamDeclList::const_iterator q = outParams.begin(); q != outParams.end();)
                    {
                        C << defaultValue((*q)->type(), scope, op->getMetaData());
                        if(++q != outParams.end())
                        {
                            C << ", ";
                        }
                    }
                    C << ", current);";
                }
            }
            else
            {
                for(ParamDeclList::const_iterator q = outParams.begin(); q != outParams.end(); ++q)
                {
                    C << nl << fixKwd((*q)->name()) << " = " << defaultValue((*q)->type(), scope, op->getMetaData()) << ";";
                }

                if(ret)
                {
                    C << nl << "return " << defaultValue(ret, scope, op->getMetaData()) << ";";
                }
            }
            C << eb;
        }
    }

    H << eb << ';';

    _useWstring = resetUseWstring(_useWstringHist);

    return true;
}
