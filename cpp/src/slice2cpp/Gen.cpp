//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Gen.h"
#include <Slice/Util.h>
#include "CPlusPlusUtil.h"
#include <IceUtil/Iterator.h>
#include <IceUtil/StringUtil.h>
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
isConstexprType(const TypePtr& constType)
{
    TypePtr type = unwrapIfOptional(constType);

    BuiltinPtr bp = BuiltinPtr::dynamicCast(type);
    if(bp)
    {
        switch(bp->kind())
        {
            case Builtin::KindBool:
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindUShort:
            case Builtin::KindInt:
            case Builtin::KindUInt:
            case Builtin::KindVarInt:
            case Builtin::KindVarUInt:
            case Builtin::KindLong:
            case Builtin::KindULong:
            case Builtin::KindVarLong:
            case Builtin::KindVarULong:
            case Builtin::KindFloat:
            case Builtin::KindDouble:
            case Builtin::KindAnyClass:
            case Builtin::KindObject:
            {
                return true;
            }
            default:
            {
                return false;
            }
        }
    }
    else if (EnumPtr::dynamicCast(type) || InterfaceDeclPtr::dynamicCast(type) ||
        ClassDeclPtr::dynamicCast(type))
    {
        return true;
    }
    else
    {
        if (StructPtr s = StructPtr::dynamicCast(type))
        {
            for (const auto& member : s->dataMembers())
            {
                if (!isConstexprType(member->type()))
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
getDeprecateSymbol(const ContainedPtr& p1, bool checkContainer)
{
    string deprecateMessage = getDeprecateReason(p1, checkContainer);
    if(!deprecateMessage.empty())
    {
        return "ICE_DEPRECATED_API(\"" + deprecateMessage + "\") ";
    }
    return "";
}

void
writeConstantValue(IceUtilInternal::Output& out, const TypePtr& constType, const SyntaxTreeBasePtr& valueType,
                   const string& value, int typeContext, const StringList& metadata, const string& scope)
{
    TypePtr type = unwrapIfOptional(constType);
    ConstPtr constant = ConstPtr::dynamicCast(valueType);
    if(constant)
    {
        out << getUnqualified(fixKwd(constant->scoped()), scope);
    }
    else
    {
        BuiltinPtr bp = BuiltinPtr::dynamicCast(type);
        if(bp)
        {
            switch(bp->kind())
            {
                case Builtin::KindString:
                {
                    if ((typeContext & TypeContextUseWstring) || findMetadata(metadata) == "wstring") // wide strings
                    {
                        out << "L\"" << toStringLiteral(value, "\a\b\f\n\r\t\v", "?", UCN, 0) << "\"";
                    }
                    else // narrow strings
                    {
                        out << "\"" << toStringLiteral(value, "\a\b\f\n\r\t\v", "?", Octal, 0) << "\"";
                    }
                    break;
                }

                case Builtin::KindUShort:
                case Builtin::KindUInt:
                case Builtin::KindVarUInt:
                {
                    out << value << "U";
                    break;
                }

                case Builtin::KindLong:
                case Builtin::KindVarLong:
                {
                    out << value << "LL";
                    break;
                }

                case Builtin::KindULong:
                case Builtin::KindVarULong:
                {
                    out << value << "ULL";
                    break;
                }

                case Builtin::KindFloat:
                {
                    out << value;
                    if(value.find(".") == string::npos)
                    {
                        out << ".0";
                    }
                    out << "F";
                    break;
                }

                default:
                {
                    out << value;
                }
            }
        }
        else
        {
            EnumPtr ep = EnumPtr::dynamicCast(type);
            if(ep && valueType)
            {
                EnumeratorPtr enumerator = EnumeratorPtr::dynamicCast(valueType);
                assert(enumerator);

                bool unscoped = (findMetadata(ep->getAllMetadata()) == "%unscoped");

                if(unscoped)
                {
                    out << getUnqualified(fixKwd(ep->scope() + enumerator->name()), scope);
                }
                else
                {
                    out << getUnqualified(fixKwd(enumerator->scoped()), scope);
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
writeInParamsLambda(IceUtilInternal::Output& C, const OperationPtr& p, const MemberList& inParams,
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
        writeMarshalCode(C, inParams, 0, true);
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
escapeParam(const MemberList& params, const string& name)
{
    for (const auto& param : params)
    {
        if (fixKwd(param->name()) == name)
        {
            return name + "_";
        }
    }
    return name;
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

    if (ClassDeclPtr::dynamicCast(p) || ClassDefPtr::dynamicCast(p) ||
        StructPtr::dynamicCast(p) || ExceptionPtr::dynamicCast(p))
    {
        UnitPtr unt = p->container()->unit();
        string file = p->file();
        assert(!file.empty());
        static const string prefix = "cpp:doxygen:include:";
        DefinitionContextPtr dc = unt->findDefinitionContext(file);
        assert(dc);
        string q = dc->findMetadata(prefix);
        if(!q.empty())
        {
            out << nl << " * \\headerfile " << q.substr(prefix.size());
        }
    }

    out << nl << " */";
}

enum OpDocParamType { OpDocInParams, OpDocOutParams, OpDocAllParams };

void
writeOpDocParams(Output& out, const OperationPtr& op, const CommentPtr& doc, OpDocParamType type,
                 const StringList& preParams = StringList(), const StringList& postParams = StringList())
{
    MemberList params;
    switch(type)
    {
        case OpDocInParams:
            params = op->params();
            break;
        case OpDocOutParams:
            params = op->outParameters();
            break;
        case OpDocAllParams:
            params = op->allMembers();
            break;
    }

    if(!preParams.empty())
    {
        writeDocLines(out, preParams, true);
    }

    map<string, StringList> paramDoc = doc->params();
    for (const auto& param : params)
    {
        map<string, StringList>::iterator q = paramDoc.find(param->name());
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
    InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(container);
    string clScope = fixKwd(interface->scope());

    TypePtr ret = p->deprecatedReturnType();
    string retS = returnTypeToString(ret, p->returnIsTagged(), clScope, p->getAllMetadata(), useWstring);

    MemberList outParams = p->outParameters();

    if((outParams.size() > 1) || (ret && outParams.size() > 0))
    {
        //
        // Generate OpNameResult struct
        //
        string returnValueS = "returnValue";

        for (const auto& param : outParams)
        {
            if (param->name() == "returnValue")
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
            paramComments = comment->params();
        }
        if(ret)
        {
            if(comment && !comment->returns().empty())
            {
                H << nl << "/** " << getDocSentence(comment->returns()) << " */";
            }
            H << nl << retS << " " << returnValueS << ";";
        }
        for (const auto& param : outParams)
        {
            string typeString = typeToString(param->type(), param->tagged(), clScope, param->getAllMetadata(),
                                             useWstring);

            map<string, StringList>::iterator r = paramComments.find(param->name());
            if(r != paramComments.end())
            {
                H << nl << "/** " << getDocSentence(r->second) << " */";
            }
            H << nl << typeString << " " << fixKwd(param->name()) << ";";
        }
        H << eb << ";";
    }
}

}

Slice::Gen::Gen(const string& base, const string& headerExtension, const string& sourceExtension,
                const vector<string>& extraHeaders, const string& include,
                const vector<string>& includePaths, const string& dllExport, const string& dir,
                bool impl) :
    _base(base),
    _headerExtension(headerExtension),
    _implHeaderExtension(headerExtension),
    _sourceExtension(sourceExtension),
    _extraHeaders(extraHeaders),
    _include(include),
    _includePaths(includePaths),
    _dllExport(dllExport),
    _dir(dir),
    _impl(impl)
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

    if(_impl)
    {
        implH << "\n\n#endif\n";
        implC << '\n';
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
        string meta = dc->findMetadata(dllExportPrefix);
        if(meta.size() > dllExportPrefix.size())
        {
            _dllExport = meta.substr(dllExportPrefix.size());
        }
    }

    if(_impl)
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

    validateMetadata(p);

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

    if(p->hasClassDefs() || p->hasInterfaceDefs())
    {
        H << "\n#include <Ice/Proxy.h>";
        H << "\n#include <Ice/Object.h>";
        H << "\n#include <Ice/Value.h>";
        H << "\n#include <Ice/Incoming.h>";
        if(p->hasContentsWithMetadata("amd"))
        {
            H << "\n#include <Ice/IncomingAsync.h>";
        }
        C << "\n#include <Ice/LocalException.h>";
        C << "\n#include <Ice/ValueFactory.h>";
        C << "\n#include <Ice/OutgoingAsync.h>";
    }
    else if(p->hasInterfaceDecls())
    {
        H << "\n#include <Ice/Proxy.h>";
    }

    if(p->hasClassDefs() || p->hasExceptions())
    {
        H << "\n#include <Ice/FactoryTableInit.h>";
    }

    H << "\n#include <IceUtil/ScopedArray.h>";
    H << "\n#include <Ice/Optional.h>";

    if(p->hasExceptions())
    {
        H << "\n#include <Ice/ExceptionHelpers.h>";
    }

    C << "\n#include <Ice/InputStream.h>";
    C << "\n#include <Ice/OutputStream.h>";

    if(p->hasExceptions())
    {
        C << "\n#include <Ice/LocalException.h>";
    }

    if(p->hasContentsWithMetadata("preserve-slice"))
    {
        H << "\n#include <Ice/SlicedDataF.h>";
        C << "\n#include <Ice/SlicedData.h>";
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
        StringList globalMetadata = dc->getAllMetadata();
        for(StringList::const_iterator q = globalMetadata.begin(); q != globalMetadata.end();)
        {
            string md = *q++;
            static const string includePrefix = "cpp:include:";
            static const string sourceIncludePrefix = "cpp:source-include:";
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
                    dc->warning(InvalidMetadata, file, -1, ostr.str());
                    globalMetadata.remove(md);
                }
            }
            else if(md.find(sourceIncludePrefix) == 0)
            {
                if(md.size() > sourceIncludePrefix.size())
                {
                    C << nl << "#include <" << md.substr(sourceIncludePrefix.size()) << ">";
                }
                else
                {
                    ostringstream ostr;
                    ostr << "ignoring invalid global metadata `" << md << "'";
                    dc->warning(InvalidMetadata, file, -1, ostr.str());
                    globalMetadata.remove(md);
                }
            }
        }
        dc->setMetadata(globalMetadata);
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
    C << sp;
    {
        DeclVisitor declVisitor(H, C, _dllExport);
        p->visit(&declVisitor, false);

        TypesVisitor typesVisitor(H, C, _dllExport);
        p->visit(&typesVisitor, false);

        InterfaceVisitor interfaceVisitor(H, C, _dllExport);
        p->visit(&interfaceVisitor, false);

        ValueVisitor valueVisitor(H, C, _dllExport);
        p->visit(&valueVisitor, false);

        ProxyVisitor proxyVisitor(H, C, _dllExport);
        p->visit(&proxyVisitor, false);

        StreamVisitor streamVisitor(H, C, _dllExport);
        p->visit(&streamVisitor, false);

        if(_impl)
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

        CompatibilityVisitor compatibilityVisitor(H, C, _dllExport);
        p->visit(&compatibilityVisitor, false);
    }
    H << sp;
    C << sp;
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

void
Slice::Gen::validateMetadata(const UnitPtr& u)
{
    MetadataVisitor visitor;
    u->visit(&visitor, false);
}

bool
Slice::Gen::MetadataVisitor::visitUnitStart(const UnitPtr& p)
{
    static const string prefix = "cpp:";

    //
    // Validate global metadata in the top-level file and all included files.
    //
    StringList files = p->allFiles();

    for(StringList::iterator q = files.begin(); q != files.end(); ++q)
    {
        string file = *q;
        DefinitionContextPtr dc = p->findDefinitionContext(file);
        assert(dc);
        StringList globalMetadata = dc->getAllMetadata();
        int headerExtension = 0;
        int sourceExtension = 0;
        int dllExport = 0;
        for(StringList::const_iterator r = globalMetadata.begin(); r != globalMetadata.end();)
        {
            string s = *r++;
            if(s.find(prefix) == 0)
            {
                static const string cppIncludePrefix = "cpp:include:";
                static const string cppSourceIncludePrefix = "cpp:source-include";
                static const string cppHeaderExtPrefix = "cpp:header-ext:";
                static const string cppSourceExtPrefix = "cpp:source-ext:";
                static const string cppDllExportPrefix = "cpp:dll-export:";
                static const string cppDoxygenIncludePrefix = "cpp:doxygen:include:";

                if(s.find(cppIncludePrefix) == 0 && s.size() > cppIncludePrefix.size())
                {
                    continue;
                }
                else if(s.find(cppSourceIncludePrefix) == 0 && s.size() > cppSourceIncludePrefix.size())
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
                        dc->warning(InvalidMetadata, file, -1, ostr.str());
                        globalMetadata.remove(s);
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
                        dc->warning(InvalidMetadata, file, -1, ostr.str());
                        globalMetadata.remove(s);
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
                        dc->warning(InvalidMetadata, file, -1, ostr.str());

                        globalMetadata.remove(s);
                    }
                    continue;
                }
                else if(s.find(cppDoxygenIncludePrefix) == 0 && s.size() > cppDoxygenIncludePrefix.size())
                {
                    continue;
                }

                ostringstream ostr;
                ostr << "ignoring invalid global metadata `" << s << "'";
                dc->warning(InvalidMetadata, file, -1, ostr.str());
                globalMetadata.remove(s);
            }

        }
        dc->setMetadata(globalMetadata);
    }

    return true;
}

bool
Slice::Gen::MetadataVisitor::visitModuleStart(const ModulePtr& p)
{
    StringList metadata = validate(p, p->getAllMetadata(), p->file(), p->line());
    p->setMetadata(metadata);
    return true;
}

void
Slice::Gen::MetadataVisitor::visitModuleEnd(const ModulePtr&)
{
}

void
Slice::Gen::MetadataVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    StringList metadata = validate(p, p->getAllMetadata(), p->file(), p->line());
    p->setMetadata(metadata);
}

bool
Slice::Gen::MetadataVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    StringList metadata = validate(p, p->getAllMetadata(), p->file(), p->line());
    p->setMetadata(metadata);
    return true;
}

void
Slice::Gen::MetadataVisitor::visitClassDefEnd(const ClassDefPtr&)
{
}

void
Slice::Gen::MetadataVisitor::visitInterfaceDecl(const InterfaceDeclPtr& p)
{
    StringList metadata = validate(p, p->getAllMetadata(), p->file(), p->line());
    p->setMetadata(metadata);
}

bool
Slice::Gen::MetadataVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    StringList metadata = validate(p, p->getAllMetadata(), p->file(), p->line());
    p->setMetadata(metadata);
    return true;
}

void
Slice::Gen::MetadataVisitor::visitInterfaceDefEnd(const InterfaceDefPtr&)
{
}

bool
Slice::Gen::MetadataVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    StringList metadata = validate(p, p->getAllMetadata(), p->file(), p->line());
    p->setMetadata(metadata);
    return true;
}

void
Slice::Gen::MetadataVisitor::visitExceptionEnd(const ExceptionPtr&)
{
}

bool
Slice::Gen::MetadataVisitor::visitStructStart(const StructPtr& p)
{
    StringList metadata = validate(p, p->getAllMetadata(), p->file(), p->line());
    p->setMetadata(metadata);
    return true;
}

void
Slice::Gen::MetadataVisitor::visitStructEnd(const StructPtr&)
{
}

void
Slice::Gen::MetadataVisitor::visitOperation(const OperationPtr& p)
{
    InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(p->container());

    StringList metadata = p->getAllMetadata();

    const UnitPtr ut = p->unit();
    const DefinitionContextPtr dc = ut->findDefinitionContext(p->file());
    assert(dc);

    TypePtr returnType = p->deprecatedReturnType();
    if(!returnType)
    {
        for(StringList::const_iterator q = metadata.begin(); q != metadata.end();)
        {
            string s = *q++;
            if(s.find("cpp:type:") == 0 || s.find("cpp:view-type:") == 0 || s == "cpp:array")
            {
                dc->warning(InvalidMetadata, p->file(), p->line(),
                            "ignoring invalid metadata `" + s + "' for operation with void return type");
                metadata.remove(s);
            }
            else if(s.find("cpp:const") == 0)
            {
                continue;
            }
            else if(s.find("cpp:") == 0)
            {
                dc->warning(InvalidMetadata, p->file(), p->line(), "ignoring invalid metadata `" + s + "'");
            }
        }
    }
    else
    {
        metadata = validate(returnType, metadata, p->file(), p->line(), true);
    }

    p->setMetadata(metadata);

    for (auto& param : p->allMembers())
    {
        metadata = validate(param->type(), param->getAllMetadata(), p->file(), param->line(), true);
        param->setMetadata(metadata);
    }
}

void
Slice::Gen::MetadataVisitor::visitDataMember(const MemberPtr& p)
{
    StringList metadata = validate(p->type(), p->getAllMetadata(), p->file(), p->line());
    p->setMetadata(metadata);
}

void
Slice::Gen::MetadataVisitor::visitSequence(const SequencePtr& p)
{
    StringList metadata = validate(p, p->getAllMetadata(), p->file(), p->line());
    p->setMetadata(metadata);
}

void
Slice::Gen::MetadataVisitor::visitDictionary(const DictionaryPtr& p)
{
    StringList metadata = validate(p, p->getAllMetadata(), p->file(), p->line());
    p->setMetadata(metadata);
}

void
Slice::Gen::MetadataVisitor::visitEnum(const EnumPtr& p)
{
    StringList metadata = validate(p, p->getAllMetadata(), p->file(), p->line());
    p->setMetadata(metadata);
}

void
Slice::Gen::MetadataVisitor::visitConst(const ConstPtr& p)
{
    StringList metadata = validate(p, p->getAllMetadata(), p->file(), p->line());
    p->setMetadata(metadata);
}

StringList
Slice::Gen::MetadataVisitor::validate(const SyntaxTreeBasePtr& cont, const StringList& metadata,
                                      const string& file, int line, bool operation)
{
    if (auto optional = OptionalPtr::dynamicCast(cont))
    {
        return validate(optional->underlying(), metadata, file, line, operation);
    }

    static const string cppPrefix = "cpp:";

    const UnitPtr ut = cont->unit();
    const DefinitionContextPtr dc = ut->findDefinitionContext(file);
    assert(dc);
    StringList newMetadata = metadata;
    for(StringList::const_iterator p = newMetadata.begin(); p != newMetadata.end();)
    {
        string s = *p++;

        if(operation && (s == "cpp:const"))
        {
            continue;
        }

        if(s.find(cppPrefix) == 0)
        {
            string ss = s.substr(cppPrefix.size());
            if(ss == "type:wstring" || ss == "type:string")
            {
                BuiltinPtr builtin = BuiltinPtr::dynamicCast(cont);
                ModulePtr module = ModulePtr::dynamicCast(cont);
                ClassDefPtr clss = ClassDefPtr::dynamicCast(cont);
                InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(cont);
                StructPtr strct = StructPtr::dynamicCast(cont);
                ExceptionPtr exception = ExceptionPtr::dynamicCast(cont);
                if((builtin && builtin->kind() == Builtin::KindString) || module || clss || interface || strct ||
                    exception)
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
                if(ss.find("type:") == 0 || ss.find("view-type:") == 0 || ss == "array")
                {
                    continue;
                }
            }
            if(DictionaryPtr::dynamicCast(cont) && (ss.find("type:") == 0 || ss.find("view-type:") == 0))
            {
                continue;
            }
            if(ExceptionPtr::dynamicCast(cont) && ss == "ice_print")
            {
                continue;
            }
            if(EnumPtr::dynamicCast(cont) && ss == "unscoped")
            {
                continue;
            }

            dc->warning(InvalidMetadata, file, line, "ignoring invalid metadata `" + s + "'");
            newMetadata.remove(s);
            continue;
        }
    }
    return newMetadata;
}

int
Slice::Gen::setUseWstring(ContainedPtr p, list<int>& hist, int use)
{
    hist.push_back(use);
    StringList metadata = p->getAllMetadata();
    if(find(metadata.begin(), metadata.end(), "cpp:type:wstring") != metadata.end())
    {
        use = TypeContextUseWstring;
    }
    else if(find(metadata.begin(), metadata.end(), "cpp:type:string") != metadata.end())
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
    string meta = dc->findMetadata(headerExtPrefix);
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
    string meta = dc->findMetadata(sourceExtPrefix);
    if(meta.size() > sourceExtPrefix.size())
    {
        ext = meta.substr(sourceExtPrefix.size());
    }
    return ext;
}

// Visitors

Slice::Gen::DeclVisitor::DeclVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport)
{
}

bool
Slice::Gen::DeclVisitor::visitUnitStart(const UnitPtr& p)
{
    if(!p->hasClassDecls() && !p->hasInterfaceDecls() && !p->hasExceptions())
    {
        return false;
    }
    C << sp << nl << "namespace" << nl << "{";
    return true;
}

void
Slice::Gen::DeclVisitor::visitUnitEnd(const UnitPtr&)
{
    C << sp << nl << "}";
}

bool
Slice::Gen::DeclVisitor::visitModuleStart(const ModulePtr& p)
{
    if(p->hasClassDecls() || p->hasInterfaceDecls())
    {
        H << sp << nl << "namespace " << fixKwd(p->name()) << nl << '{' << sp;
    }
    return true;
}

void
Slice::Gen::DeclVisitor::visitModuleEnd(const ModulePtr& p)
{
    if(p->hasClassDecls() || p->hasInterfaceDecls())
    {
        H << sp << nl << '}';
    }
}

void
Slice::Gen::DeclVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    H << nl << "class " << fixKwd(p->name()) << ';';
}

bool
Slice::Gen::DeclVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    C << sp;

    C << nl << "const ::IceInternal::DefaultValueFactoryInit<" << fixKwd(p->scoped()) << "> ";
    C << "iceC" + p->flattenedScope() + p->name() + "_init"
      << "(\"" << p->scoped() << "\");";

    if (p->compactId() >= 0)
    {
        string n = "iceC" + p->flattenedScope() + p->name() + "_compactIdInit ";
        C << nl << "const ::IceInternal::CompactIdInit " << n << "(\"" << p->scoped() << "\", " << p->compactId()
          << ");";
    }
    return true;
}

void
Slice::Gen::DeclVisitor::visitInterfaceDecl(const InterfaceDeclPtr& p)
{
    H << nl << "class " << fixKwd(p->name()) << ';';
    H << nl << "class " << p->name() << "Prx;";
}

bool
Slice::Gen::DeclVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    OperationList allOps = p->allOperations();

    C << sp;

    StringList ids = p->ids();
    C << nl << "const ::std::string iceC" << p->flattenedScope() << p->name() << "_ids[" << ids.size() << "] =";
    C << sb;
    for (StringList::const_iterator r = ids.begin(); r != ids.end();)
    {
        C << nl << '"' << *r << '"';
        if (++r != ids.end())
        {
            C << ',';
        }
    }
    C << eb << ';';

    StringList allOpNames;
    transform(allOps.begin(), allOps.end(), back_inserter(allOpNames), [](const auto &c) { return c->name(); });
    allOpNames.push_back("ice_id");
    allOpNames.push_back("ice_ids");
    allOpNames.push_back("ice_isA");
    allOpNames.push_back("ice_ping");
    allOpNames.sort();
    allOpNames.unique();

    C << nl << "const ::std::string iceC" << p->flattenedScope() << p->name() << "_ops[] =";
    C << sb;
    for (StringList::const_iterator q = allOpNames.begin(); q != allOpNames.end();)
    {
        C << nl << '"' << *q << '"';
        if (++q != allOpNames.end())
        {
            C << ',';
        }
    }
    C << eb << ';';

    return true;
}

bool
Slice::Gen::DeclVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    C << sp;
    C << nl << "const ::IceInternal::DefaultUserExceptionFactoryInit<" << fixKwd(p->scoped()) << "> ";
    C << "iceC" + p->flattenedScope() + p->name() + "_init" << "(\"" << p->scoped() << "\");";
    return false;
}

void
Slice::Gen::DeclVisitor::visitOperation(const OperationPtr& p)
{
    InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(p->container());
    assert(interface);
    string flatName = "iceC" + p->flattenedScope() + p->name() + "_name";
    C << nl << "const ::std::string " << flatName << " = \"" << p->name() << "\";";
}

Slice::Gen::TypesVisitor::TypesVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllExport(dllExport), _dllClassExport(toDllClassExport(dllExport)),
    _dllMemberExport(toDllMemberExport(dllExport)), _doneStaticSymbol(false), _useWstring(false)
{
}

bool
Slice::Gen::TypesVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonClassTypes())
    {
        return false;
    }

    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);
    H << sp << nl << "namespace " << fixKwd(p->name()) << nl << '{';
    return true;
}

void
Slice::Gen::TypesVisitor::visitModuleEnd(const ModulePtr& p)
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
Slice::Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    string name = fixKwd(p->name());
    string scope = fixKwd(p->scope());
    string scoped = fixKwd(p->scoped());
    const ExceptionPtr base = p->base();
    const MemberList dataMembers = p->dataMembers();
    const MemberList allDataMembers = p->allDataMembers();
    const MemberList baseDataMembers = base ? base->allDataMembers() : MemberList();

    vector<string> allParameters;
    map<string, CommentPtr> allComments;

    for (const auto& member : allDataMembers)
    {
        string typeName = inputTypeToString(member->type(), member->tagged(), scope, member->getAllMetadata(),
                                            _useWstring);
        allParameters.push_back(typeName + " " + fixKwd(member->name()));

        if (CommentPtr comment = member->parseComment(false))
        {
            allComments[member->name()] = comment;
        }
    }

    string helperClass = getUnqualified("::Ice::UserExceptionHelper", scope);
    string baseClass = base ?
        getUnqualified(fixKwd(base->scoped()), scope) :
        getUnqualified("::Ice::UserException", scope);
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
    H << sp << nl << name << "() = default;";

    if(!allDataMembers.empty())
    {
        H << sp;
        H << nl << "/**";
        H << nl << " * One-shot constructor to initialize all data members.";
        for (const auto& member : allDataMembers)
        {
            map<string, CommentPtr>::iterator r = allComments.find(member->name());
            if(r != allComments.end())
            {
                H << nl << " * @param " << fixKwd(r->first) << " " << getDocSentence(r->second->overview());
            }
        }
        H << nl << " */";
        H << nl << name << "(";

        for(vector<string>::const_iterator q = allParameters.begin(); q != allParameters.end(); ++q)
        {
            if(q != allParameters.begin())
            {
                H << ", ";
            }
            H << (*q);
        }
        H << ") :";
        H.inc();
        if(base && !baseDataMembers.empty())
        {
            H << nl << helperClass << "<" << templateParameters << ">" << "(";
            for (auto q = baseDataMembers.begin(); q != baseDataMembers.end(); ++q)
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

        for (auto q = dataMembers.begin(); q != dataMembers.end(); ++q)
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
    writeIceTuple(H, allDataMembers, _useWstring);

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

    StringList metadata = p->getAllMetadata();
    if(find(metadata.begin(), metadata.end(), "cpp:ice_print") != metadata.end())
    {
        H << nl << "/**";
        H << nl << " * Prints this exception to the given stream.";
        H << nl << " * @param stream The target stream.";
        H << nl << " */";
        H << nl << _dllMemberExport << "virtual void ice_print(::std::ostream& stream) const override;";
    }

    if(p->usesClasses(false))
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
Slice::Gen::TypesVisitor::visitExceptionEnd(const ExceptionPtr& p)
{
    string name = fixKwd(p->name());
    string scope = fixKwd(p->scope());
    string scoped = fixKwd(p->scoped());
    string factoryName;

    ExceptionPtr base = p->base();
    bool basePreserved = p->inheritsMetadata("preserve-slice");
    bool preserved = p->hasMetadata("preserve-slice");

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
    H << eb << ';';

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

    _useWstring = resetUseWstring(_useWstringHist);
}

bool
Slice::Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    H << sp;
    writeDocSummary(H, p);
    H << nl << "struct " << fixKwd(p->name());
    H << sb;

    return true;
}

void
Slice::Gen::TypesVisitor::visitStructEnd(const StructPtr& p)
{
    H << sp;
    H << nl << "/**";
    H << nl << " * Obtains a tuple containing all of the struct's data members.";
    H << nl << " * @return The data members in a tuple.";
    H << nl << " */";
    writeIceTuple(H, p->dataMembers(), _useWstring);
    H << eb << ';';
    _useWstring = resetUseWstring(_useWstringHist);
}

void
Slice::Gen::TypesVisitor::visitDataMember(const MemberPtr& p)
{
    //
    // Use an empty scope to get full qualified names from calls to typeToString.
    //
    const string scope = "";
    string name = fixKwd(p->name());
    writeDocSummary(H, p);
    H << nl << typeToString(p->type(), p->tagged(), scope, p->getAllMetadata(), _useWstring)
      << ' ' << name;

    string defaultValue = p->defaultValue();
    if(!defaultValue.empty())
    {
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(unwrapIfOptional(p->type()));
        if(p->tagged() && builtin->kind() == Builtin::KindString)
        {
            //
            // = "<string literal>" doesn't work for optional<std::string>
            //
            H << '{';
            writeConstantValue(H, p->type(), p->defaultValueType(), defaultValue, _useWstring,
                               p->getAllMetadata(), scope);
            H << '}';
        }
        else
        {
            H << " = ";
            writeConstantValue(H, p->type(), p->defaultValueType(), defaultValue, _useWstring,
                               p->getAllMetadata(), scope);
        }
    }

    H << ';';
}

void
Slice::Gen::TypesVisitor::visitSequence(const SequencePtr& p)
{
    string name = fixKwd(p->name());
    string scope = fixKwd(p->scope());
    TypePtr type = p->type();
    int typeCtx = _useWstring;
    string s = typeToString(type, scope, p->typeMetadata(), typeCtx);
    StringList metadata = p->getAllMetadata();

    string seqType = findMetadata(metadata, _useWstring);
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
Slice::Gen::TypesVisitor::visitDictionary(const DictionaryPtr& p)
{
    string name = fixKwd(p->name());
    string scope = fixKwd(p->scope());
    string dictType = findMetadata(p->getAllMetadata());
    int typeCtx = _useWstring;

    H << sp;
    writeDocSummary(H, p);

    if(dictType.empty())
    {
        //
        // A default std::map dictionary
        //
        TypePtr keyType = p->keyType();
        TypePtr valueType = p->valueType();
        string ks = typeToString(keyType, scope, p->keyMetadata(), typeCtx);
        string vs = typeToString(valueType, scope, p->valueMetadata(), typeCtx);

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

Slice::Gen::ProxyVisitor::ProxyVisitor(Output& h, Output& c, const string& dllExport) :
    H(h), C(c), _dllClassExport(toDllClassExport(dllExport)),
    _dllMemberExport(toDllMemberExport(dllExport)),
    _useWstring(false)
{
}

bool
Slice::Gen::ProxyVisitor::visitUnitStart(const UnitPtr&)
{
    return true;
}

void
Slice::Gen::ProxyVisitor::visitUnitEnd(const UnitPtr&)
{
}

bool
Slice::Gen::ProxyVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasInterfaceDefs())
    {
        return false;
    }

    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    H << sp << nl << "namespace " << fixKwd(p->name()) << nl << '{';
    return true;
}

void
Slice::Gen::ProxyVisitor::visitModuleEnd(const ModulePtr&)
{
    H << sp << nl << '}';

    _useWstring = resetUseWstring(_useWstringHist);
}

bool
Slice::Gen::ProxyVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    string name = fixKwd(p->name());
    string scope = fixKwd(p->scope());
    string scoped = fixKwd(p->scoped());
    InterfaceList bases = p->bases();

    H << sp;
    writeDocSummary(H, p);
    H << nl << "class " << _dllClassExport << p->name() << "Prx : public virtual "
      << getUnqualified("::Ice::Proxy", scope) << "<" << fixKwd(p->name() + "Prx") << ", ";
    if(bases.empty())
    {
        H << getUnqualified("::Ice::ObjectPrx", scope);
    }
    else
    {
        InterfaceList::const_iterator q = bases.begin();
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
Slice::Gen::ProxyVisitor::visitInterfaceDefEnd(const InterfaceDefPtr& p)
{
    string prx = fixKwd(p->name() + "Prx");
    const string scoped = fixKwd(p->scoped() + "Prx");
    const string scope = fixKwd(p->scope());

    H << sp;
    H << nl << "/**";
    H << nl << " * Obtains the Slice type ID of this interface.";
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
    C << nl << "return "<< fixKwd(p->name()) << "::ice_staticId();";
    C << eb;

    _useWstring = resetUseWstring(_useWstringHist);
}

void
Slice::Gen::ProxyVisitor::visitOperation(const OperationPtr& p)
{
    string name = p->name();
    string flatName = "iceC" + p->flattenedScope() + p->name() + "_name";

    ContainerPtr container = p->container();
    InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(container);
    string clScope = fixKwd(interface->scope());

    TypePtr ret = p->deprecatedReturnType();

    bool retIsTagged = p->returnIsTagged();
    string retS = returnTypeToString(ret, retIsTagged, clScope, p->getAllMetadata(), _useWstring);

    vector<string> params;
    vector<string> paramsDecl;

    vector<string> inParamsS;
    vector<string> inParamsDecl;
    vector<string> inParamsImplDecl;

    vector<string> futureOutParams;
    vector<string> lambdaOutParams;

    MemberList paramList = p->allMembers();
    MemberList inParams = p->params();
    MemberList outParams = p->outParameters();

    string returnValueS = "returnValue";
    bool hasTaggedOutParams = false;

    if(ret)
    {
        //
        // Use empty scope to get full qualified names in types used with future declarations.
        //
        futureOutParams.push_back(typeToString(ret, retIsTagged, "", p->getAllMetadata(), _useWstring));

        lambdaOutParams.push_back(typeToString(ret, retIsTagged, "", p->getAllMetadata(), _useWstring |
                                               TypeContextInParam));

        hasTaggedOutParams |= p->returnIsTagged();
    }

    for (const auto& inParam : inParams)
    {
        string paramName = fixKwd(inParam->name());
        string typeString = inputTypeToString(inParam->type(), inParam->tagged(), clScope, inParam->getAllMetadata(),
                                              _useWstring);

        params.push_back(typeString);
        paramsDecl.push_back(typeString + ' ' + paramName);

        inParamsS.push_back(typeString);
        inParamsDecl.push_back(typeString + ' ' + paramName);
        inParamsImplDecl.push_back(typeString + ' ' + paramPrefix + inParam->name());
    }

    for (const auto& outParam : outParams)
    {
        StringList metadata = outParam->getAllMetadata();

        // Use empty scope to get full qualified names in types used with future declarations.
        futureOutParams.push_back(typeToString(outParam->type(), outParam->tagged(), "", metadata, _useWstring));
        lambdaOutParams.push_back(typeToString(outParam->type(), outParam->tagged(), "", metadata,
                                               _useWstring | TypeContextInParam));
        string outputTypeString = outputTypeToString(outParam->type(), outParam->tagged(), clScope, metadata,
                                                     _useWstring);

        params.push_back(outputTypeString);
        paramsDecl.push_back(outputTypeString + ' ' + fixKwd(outParam->name()));

        hasTaggedOutParams |= outParam->tagged();

        if (outParam->name() == "returnValue")
        {
            returnValueS = "_returnValue";
        }
    }

    string scoped = fixKwd(interface->scope() + interface->name() + "Prx" + "::").substr(2);

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
        futureT = resultStructName(name, fixKwd(interface->name()));
    }

    const string deprecateSymbol = getDeprecateSymbol(p, true);

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

    H << "_makePromiseOutgoing<" << futureT << ">";

    H << spar << "true, this" << "&" + interface->name() + "Prx::_iceI_" + name;
    for (const auto& param : inParams)
    {
        H << fixKwd(param->name());
    }
    H << contextParam << epar << ".get();";
    if(futureOutParams.size() > 1)
    {
        for (const auto& param : outParams)
        {
            H << nl << fixKwd(param->name()) << " = ";
            H << condMove(isMovable(param->type()), "_result." + fixKwd(param->name())) + ";";
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

    H << "false, this" << string("&" + interface->name() + "Prx::_iceI_" + name);
    for (const auto& param : inParams)
    {
        H << fixKwd(param->name());
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
        for(auto q = inParamsDecl.begin(); q != inParamsDecl.end(); ++q)
        {
            if(q != inParamsDecl.begin())
            {
                H << " ";
            }

            H << *q << ",";
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
        writeAllocateCode(C, outParams, p, true, clScope, _useWstring | TypeContextInParam);
        writeUnmarshalCode(C, outParams, p, true);

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
        for (const auto& param : outParams)
        {
            C << fixKwd(paramPrefix + param->name());
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
        C << operationModeToString(p->sendMode()) << ", " << opFormatTypeToString(p) << ", context,";
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
            for (const auto& param : outParams)
            {
                H << condMove(isMovable(param->type()), "_result." + fixKwd(param->name()));
            }
            H << epar << ";" << eb << ";";
        }

        H << nl << "return _makeLambdaOutgoing<" << futureT << ">" << spar;

        H << "std::move(" + (futureOutParams.size() > 1 ? "_responseCb" : responseParam) + ")"
          << "std::move(" + exParam + ")"
          << "std::move(" + sentParam + ")"
          << "this";
        H << string("&" + getUnqualified(scoped, clScope.substr(2)) + "_iceI_" + name);
        for (const auto& param : inParams)
        {
            H << fixKwd(param->name());
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
    C << getUnqualified(operationModeToString(p->sendMode()), clScope) << ", "
      << getUnqualified(opFormatTypeToString(p), clScope) << ", context,";
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
        // except if the unique ret/out is tagged or is an array/range.
        //
        C << "," << nl << "[](" << getUnqualified("::Ice::InputStream*", clScope) << " istr)";
        C << sb;
        C << nl << futureT << " v;";
        writeUnmarshalCode(C, outParams, p, false, "", returnValueS, "v");

        if(p->returnsClasses(false))
        {
            C << nl << "istr->readPendingValues();";
        }
        C << nl << "return v;";
        C << eb;
    }
    else if(hasTaggedOutParams || p->returnsClasses(false))
    {
        //
        // If there's only one tagged ret/out parameter, we still need to generate
        // a read method, we can't rely on the default read method which wouldn't
        // known which tag to use.
        //
        C << "," << nl << "[](" << getUnqualified("::Ice::InputStream*", clScope) << " istr)";
        C << sb;

        writeAllocateCode(C, outParams, p, true, clScope, _useWstring);
        writeUnmarshalCode(C, outParams, p, true);

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
Slice::Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    bool unscoped = findMetadata(p->getAllMetadata()) == "%unscoped";
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
Slice::Gen::TypesVisitor::visitConst(const ConstPtr& p)
{
    const string scope = fixKwd(p->scope());
    H << sp;
    writeDocSummary(H, p);
    H << nl << (isConstexprType(p->type()) ? "constexpr " : "const ")
      << typeToString(p->type(), scope, p->typeMetadata(), _useWstring) << " " << fixKwd(p->name())
      << " = ";
    writeConstantValue(H, p->type(), p->valueType(), p->value(), _useWstring, p->typeMetadata(),
                       scope);
    H << ';';
}

void
Slice::Gen::TypesVisitor::emitUpcall(const ExceptionPtr& base, const string& call, const string& scope)
{
    C << nl;
    if(base)
    {
        C << getUnqualified(fixKwd(base->scoped()), scope);
    }
    else
    {
        getUnqualified("::Ice::UserException", scope);
    }
    C << call;
}

Slice::Gen::ObjectVisitor::ObjectVisitor(::IceUtilInternal::Output& h,
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
Slice::Gen::ObjectVisitor::emitDataMember(const MemberPtr& p)
{
    string name = fixKwd(p->name());
    int typeContext = _useWstring;
    ContainerPtr container = p->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    //
    // Use empty scope to get full qualified names in types used with future declarations.
    //
    string scope = "";

    writeDocSummary(H, p);
    H << nl << typeToString(p->type(), p->tagged(), scope, p->getAllMetadata(), typeContext) << ' ' << name;

    string defaultValue = p->defaultValue();
    if(!defaultValue.empty())
    {
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(unwrapIfOptional(p->type()));
        if(p->tagged() && builtin && builtin->kind() == Builtin::KindString)
        {
            //
            // = "<string literal>" doesn't work for optional<std::string>
            //
            H << '{';
            writeConstantValue(H, p->type(), p->defaultValueType(), defaultValue, _useWstring,
                               p->getAllMetadata(), scope);
            H << '}';
        }
        else
        {
            H << " = ";
            writeConstantValue(H, p->type(), p->defaultValueType(), defaultValue, _useWstring,
                               p->getAllMetadata(), scope);
        }
    }
    H << ";";
}

void
Slice::Gen::InterfaceVisitor::emitUpcall(const InterfaceDefPtr& base, const string& call, const string& scope)
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
Slice::Gen::ValueVisitor::emitUpcall(const ClassDefPtr& base, const string& call, const string& scope)
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

Slice::Gen::InterfaceVisitor::InterfaceVisitor(::IceUtilInternal::Output& h,
                                                         ::IceUtilInternal::Output& c,
                                                         const std::string& dllExport) :
    ObjectVisitor(h, c, dllExport)
{
}

bool
Slice::Gen::InterfaceVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasInterfaceDefs())
    {
        return false;
    }

    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);
    string name = fixKwd(p->name());
    H << sp << nl << "namespace " << name << nl << '{';
    return true;
}

void
Slice::Gen::InterfaceVisitor::visitModuleEnd(const ModulePtr&)
{
    H << sp;
    H << nl << '}';

    _useWstring = resetUseWstring(_useWstringHist);
}

bool
Slice::Gen::InterfaceVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);
    string name = fixKwd(p->name());
    string scope = fixKwd(p->scope());
    string scoped = fixKwd(p->scope() + p->name());
    InterfaceList bases = p->bases();

    H << sp;
    writeDocSummary(H, p);
    H << nl << "class " << _dllExport << name << " : ";
    H.useCurrentPosAsIndent();
    if(bases.empty())
    {
        H << "public virtual " << getUnqualified("::Ice::Object", scope);
    }
    else
    {
        InterfaceList::const_iterator q = bases.begin();
        while(q != bases.end())
        {
            string baseScoped = fixKwd((*q)->scope() + (*q)->name());

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

    StringList ids = p->ids();
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
Slice::Gen::InterfaceVisitor::visitInterfaceDefEnd(const InterfaceDefPtr& p)
{
    string scoped = fixKwd(p->scope() + p->name());

    string scope = fixKwd(p->scope());
    string name = fixKwd(p->name());
    InterfaceList bases = p->bases();

    OperationList allOps = p->allOperations();
    if(!allOps.empty())
    {
        StringList allOpNames;
        transform(allOps.begin(), allOps.end(), back_inserter(allOpNames), [](const auto& c) { return c->name(); });
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

void
Slice::Gen::InterfaceVisitor::visitOperation(const OperationPtr& p)
{
    string name = p->name();

    TypePtr ret = p->deprecatedReturnType();

    vector<string> params;
    vector<string> args;

    vector<string> responseParams;
    vector<string> responseParamsDecl;
    vector<string> responseParamsImplDecl;

    ContainerPtr container = p->container();
    InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(container);
    string interfaceScope = fixKwd(interface->scope());

    string scope = fixKwd(interface->scope() + interface->name() + "::");
    string scoped = fixKwd(interface->scope() + interface->name() + "::" + p->name());

    MemberList inParams = p->params();
    MemberList outParams = p->outParameters();
    MemberList paramList = p->allMembers();

    const bool amd = (interface->hasMetadata("amd") || p->hasMetadata("amd"));

    const string returnValueParam = escapeParam(outParams, "returnValue");
    const string responsecbParam = escapeParam(inParams, "response");
    const string excbParam = escapeParam(inParams, "exception");
    const string currentParam = escapeParam(amd ? inParams : paramList, "current");
    const string currentTypeDecl = "const " + getUnqualified("::Ice::Current&", interfaceScope);
    const string currentDecl = currentTypeDecl + " " + currentParam;

    CommentPtr comment = p->parseComment(false);

    if(ret)
    {
        string typeS = inputTypeToString(ret, p->returnIsTagged(), interfaceScope, p->getAllMetadata(), _useWstring);
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
        retS = returnTypeToString(ret, p->returnIsTagged(), interfaceScope, p->getAllMetadata(), _useWstring);
    }

    for (const auto& inParam : inParams)
    {
        TypePtr type = inParam->type();
        string paramName = fixKwd(inParam->name());
        params.push_back(typeToString(type, inParam->tagged(), interfaceScope, inParam->getAllMetadata(),
                                      _useWstring | TypeContextInParam) + " " + paramName);
        args.push_back(condMove(isMovable(type), paramPrefix + inParam->name()));
    }

    for (const auto& outParam : outParams)
    {
        TypePtr type = outParam->type();
        string paramName = fixKwd(outParam->name());

        if(!p->hasMarshaledResult() && !amd)
        {
            params.push_back(outputTypeToString(type, outParam->tagged(), interfaceScope, outParam->getAllMetadata(),
                                                _useWstring) + " " + paramName);
            args.push_back(paramPrefix + outParam->name());
        }

        string responseTypeS = inputTypeToString(outParam->type(), outParam->tagged(), interfaceScope,
                                                    outParam->getAllMetadata(), _useWstring);
        responseParams.push_back(responseTypeS + " " + paramName);
        responseParamsDecl.push_back(responseTypeS + " " + paramPrefix + outParam->name());
        responseParamsImplDecl.push_back(responseTypeS + " " + paramPrefix + outParam->name());
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

    emitOpNameResult(H, p, _useWstring);

    if(p->hasMarshaledResult())
    {
        string resultName = resultStructName(name, "", true);
        H << sp;
        H << nl << "/**";
        H << nl << " * Marshaled result structure for operation " << (amd ? name + "Async" : fixKwd(name)) << ".";
        H << nl << " */";
        H << nl << "class " << resultName << " : public " << getUnqualified("::Ice::MarshaledResult", interfaceScope);
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
            paramComments = comment->params();
        }
        const string mrcurrent = escapeParam(outParams, "current");
        for (const auto& param : outParams)
        {
            map<string, StringList>::iterator r = paramComments.find(param->name());
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
        C << nl << "ostr->startEncapsulation(current.encoding, " << opFormatTypeToString(p) << ");";
        writeMarshalCode(C, outParams, p, true, "ostr");
        if(p->returnsClasses(false))
        {
            C << nl << "ostr->writePendingValues();";
        }
        C << nl << "ostr->endEncapsulation();";
        C << eb;
    }

    string isConst = ((p->mode() == Operation::Nonmutating) || p->hasMetadata("cpp:const")) ? " const" : "";

    string opName = amd ? (name + "Async") : fixKwd(name);
    string deprecateSymbol = getDeprecateSymbol(p, true);

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
      << getUnqualified("::Ice::Current&", interfaceScope) << ")" << isConst << ';';
    H << nl << "/// \\endcond";

    C << sp;
    C << nl << "/// \\cond INTERNAL";
    C << nl << "bool";
    C << nl << scope.substr(2);
    C << "_iceD_" << name << "(::IceInternal::Incoming& inS, const "
      << getUnqualified("::Ice::Current&", interfaceScope) << " current)" << isConst;
    C << sb;
    C << nl << "_iceCheckMode(" << getUnqualified(operationModeToString(p->mode()), interfaceScope)
      << ", current.mode);";

    if(!inParams.empty())
    {
        C << nl << "auto istr = inS.startReadParams();";
        writeAllocateCode(C, inParams, 0, true, interfaceScope, _useWstring | TypeContextInParam);
        writeUnmarshalCode(C, inParams, 0, true);
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
        C << nl << "inS.setFormat(" << opFormatTypeToString(p) << ");";
    }

    if(!amd)
    {
        if(p->hasMarshaledResult())
        {
            C << nl << "inS.setMarshaledResult(";
        }
        else
        {
            writeAllocateCode(C, outParams, 0, true, interfaceScope, _useWstring);
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
                writeMarshalCode(C, outParams, p, true);
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
            writeMarshalCode(C, outParams, p, true);
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

Slice::Gen::ValueVisitor::ValueVisitor(::IceUtilInternal::Output& h,
                                                 ::IceUtilInternal::Output& c,
                                                 const std::string& dllExport) :
    ObjectVisitor(h, c, dllExport)
{
}

bool
Slice::Gen::ValueVisitor::visitModuleStart(const ModulePtr& p)
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
Slice::Gen::ValueVisitor::visitModuleEnd(const ModulePtr&)
{
    H << sp;
    H << nl << '}';

    _useWstring = resetUseWstring(_useWstringHist);
}

bool
Slice::Gen::ValueVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    string name = fixKwd(p->name());
    string scope = fixKwd(p->scope());
    string scoped = fixKwd(p->scoped());
    ClassDefPtr base = p->base();

    H << sp;
    writeDocSummary(H, p);
    H << nl << "class " << _dllClassExport << name << " : public " << getUnqualified("::Ice::ValueHelper", scope)
      << "<" << name << ", ";

    if(!base)
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
    writeIceTuple(H, p->allDataMembers(), _useWstring);

    H << sp;
    H << nl << "/**";
    H << nl << " * Obtains the Slice type ID of this value.";
    H << nl << " * @return The fully-scoped type ID.";
    H << nl << " */";
    H << nl << _dllMemberExport << "static const ::std::string& ice_staticId();";
    return true;
}

void
Slice::Gen::ValueVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    string scoped = fixKwd(p->scoped());
    string scope = fixKwd(p->scope());
    string name = fixKwd(p->name());
    ClassDefPtr base = p->base();
    bool basePreserved = p->inheritsMetadata("preserve-slice");
    bool preserved = p->hasMetadata("preserve-slice");

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
    MemberList dataMembers = p->dataMembers();
    bool prot = p->hasMetadata("protected");
    bool needSp = true;

    for (const auto& member : dataMembers)
    {
        if(prot || member->hasMetadata("protected"))
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

        emitDataMember(member);
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
Slice::Gen::ObjectVisitor::emitVirtualBaseInitializers(const ClassDefPtr& derived, const ClassDefPtr& base)
{
    const string scope = fixKwd(derived->scope());
    MemberList allBaseDataMembers = base->allDataMembers();
    if(allBaseDataMembers.empty())
    {
        return false;
    }

    string upcall = "(";
    for (auto q = allBaseDataMembers.begin(); q != allBaseDataMembers.end(); ++q)
    {
        if(q != allBaseDataMembers.begin())
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
    H << nl << "Ice::ValueHelper<" << getUnqualified(fixKwd(derived->scoped()), scope)
      << ", " << getUnqualified(fixKwd(base->scoped()), scope) << ">";
    H << upcall;
    return true;
}

void
Slice::Gen::ObjectVisitor::emitOneShotConstructor(const ClassDefPtr& p)
{
    MemberList allDataMembers = p->allDataMembers();
    // Use empty scope to get full qualified names in types used with future declarations.
    string scope = "";
    if(!allDataMembers.empty())
    {
        vector<string> allParamDecls;
        map<string, CommentPtr> allComments;
        MemberList dataMembers = p->dataMembers();

        int typeContext = _useWstring;

        for (const auto& member : allDataMembers)
        {
            string typeName = inputTypeToString(member->type(), member->tagged(), scope, member->getAllMetadata(),
                                                typeContext);
            allParamDecls.push_back(typeName + " " + fixKwd(member->name()));
            CommentPtr comment = member->parseComment(false);
            if(comment)
            {
                allComments[member->name()] = comment;
            }
        }

        CommentPtr comment = p->parseComment(false);

        H << sp;
        H << nl << "/**";
        H << nl << " * One-shot constructor to initialize all data members.";
        for (const auto& member : allDataMembers)
        {
            map<string, CommentPtr>::iterator r = allComments.find(member->name());
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

        if (p->base())
        {
            if(emitVirtualBaseInitializers(p, p->base()))
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

        for (auto q = dataMembers.begin(); q != dataMembers.end(); ++q)
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

Slice::Gen::StreamVisitor::StreamVisitor(Output& h, Output& c, const string& dllExport) :
    H(h),
    C(c),
    _dllExport(dllExport)
{
}

bool
Slice::Gen::StreamVisitor::visitModuleStart(const ModulePtr& m)
{
    if (!m->hasStructs() && !m->hasEnums() && !m->hasExceptions() && !m->hasClassDecls() && !m->hasClassDefs())
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

        if (m->hasStructs())
        {
            C << sp;
            C << nl << "namespace Ice" << nl << '{';
        }
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
        if (m->hasStructs())
        {
            C << nl << '}';
        }
    }
}

bool
Slice::Gen::StreamVisitor::visitStructStart(const StructPtr& p)
{
    string scoped = fixKwd(p->scoped());

    H << nl << "template<>";
    H << nl << "struct StreamableTraits<" << scoped << ">";
    H << sb;
    H << nl << "static const StreamHelperCategory helper = StreamHelperCategoryStruct;";
    H << nl << "static const int minWireSize = " << p->minWireSize() << ";";
    H << nl << "static const bool fixedLength = " << (p->isVariableLength() ? "false" : "true") << ";";
    H << eb << ";" << nl;

    writeStreamHelpers(H, p);

    return false;
}

bool
Slice::Gen::StreamVisitor::visitClassDefStart(const ClassDefPtr& c)
{
    writeStreamHelpers(H, c);
    return false;
}

void
Slice::Gen::StreamVisitor::visitExceptionEnd(const ExceptionPtr& p)
{
    writeStreamHelpers(H, p);
}

void
Slice::Gen::StreamVisitor::visitEnum(const EnumPtr& p)
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

Slice::Gen::CompatibilityVisitor::CompatibilityVisitor(Output& h, Output&, const string& dllExport) :
    H(h),
    _dllExport(dllExport)
{
}

bool
Slice::Gen::CompatibilityVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasClassDecls() && !p->hasInterfaceDecls())
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
Slice::Gen::CompatibilityVisitor::visitModuleEnd(const ModulePtr&)
{
    H << sp;
    H << nl << '}';
    H << nl << "/// \\endcond";
}

void
Slice::Gen::CompatibilityVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    string name = fixKwd(p->name());
    string scoped = fixKwd(p->scoped());

    H << sp << nl << "using " << p->name() << "Ptr = ::std::shared_ptr<" << name << ">;";
}

void
Slice::Gen::CompatibilityVisitor::visitInterfaceDecl(const InterfaceDeclPtr& p)
{
    string name = fixKwd(p->name());
    string scoped = fixKwd(p->scoped());

    H << sp << nl << "using " << p->name() << "Ptr = ::std::shared_ptr<" << name << ">;";
    H << nl << "using " << p->name() << "PrxPtr = ::std::shared_ptr<" << p->name() << "Prx>;";
}

Slice::Gen::ImplVisitor::ImplVisitor(Output& h, Output& c, const string& dllExport) :
    H(h),
    C(c),
    _dllExport(dllExport),
    _useWstring(false)
{
}

string
Slice::Gen::ImplVisitor::defaultValue(const TypePtr& type, const string& scope, const StringList& metadata) const
{
    if (OptionalPtr::dynamicCast(type))
    {
        return "nullptr";
    }

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
            case Builtin::KindUShort:
            case Builtin::KindInt:
            case Builtin::KindUInt:
            case Builtin::KindVarInt:
            case Builtin::KindVarUInt:
            case Builtin::KindLong:
            case Builtin::KindULong:
            case Builtin::KindVarLong:
            case Builtin::KindVarULong:
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
            case Builtin::KindAnyClass:
            case Builtin::KindObject:
            {
                return "nullptr";
            }
        }
    }
    else
    {
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
            return typeToString(seq, scope, metadata, _useWstring) + "()";
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
    if(!p->hasInterfaceDefs())
    {
        return false;
    }

    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    string name = fixKwd(p->name());

    H << sp << nl << "namespace " << name << nl << '{';

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
Slice::Gen::ImplVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    string name = p->name();
    string scoped = fixKwd(p->scoped());
    string scope = fixKwd(p->scope());
    string cls = scope.substr(2) + name + "I";
    InterfaceList bases = p->bases();

    H << sp;
    H << nl << "class " << name << "I : ";
    H.useCurrentPosAsIndent();
    H << "public virtual " << fixKwd(name);

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

        TypePtr ret = op->deprecatedReturnType();
        string retS = op->hasMarshaledResult() ?
            scoped + "::" + resultStructName(opName, "", true) :
            returnTypeToString(ret, op->returnIsTagged(), "", op->getAllMetadata(), _useWstring);

        MemberList inParams = op->params();
        MemberList outParams = op->outParameters();

        if(p->hasMetadata("amd") || op->hasMetadata("amd"))
        {
            string responseParams;

            H << sp << nl << "virtual void " << opName << "Async(";
            H.useCurrentPosAsIndent();
            for (const auto& param : inParams)
            {
                H << typeToString(param->type(), param->tagged(), scope,
                                  param->getAllMetadata(), _useWstring | TypeContextInParam)
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
                    responseParams = inputTypeToString(ret, op->returnIsTagged(), scope, op->getAllMetadata(),
                                                       _useWstring);
                    if(!outParams.empty())
                    {
                        responseParams += ", ";
                    }
                }

                for(auto q = outParams.begin(); q != outParams.end(); ++q)
                {
                    if(q != outParams.begin())
                    {
                        responseParams += ", ";
                    }
                    responseParams += inputTypeToString((*q)->type(), (*q)->tagged(), scope, (*q)->getAllMetadata(),
                                                        _useWstring);
                }
            }

            string isConst = ((op->mode() == Operation::Nonmutating) || op->hasMetadata("cpp:const")) ? " const" : "";

            H << "std::function<void(" << responseParams << ")>,";
            H << nl << "std::function<void(std::exception_ptr)>,";
            H << nl << "const Ice::Current&)" << isConst << " override;";
            H.restoreIndent();

            C << sp << nl << "void" << nl << scope << name << "I::" << opName << "Async(";
            C.useCurrentPosAsIndent();
            for (const auto& param : inParams)
            {
                C << typeToString(param->type(), param->tagged(), scope, param->getAllMetadata(),
                                  _useWstring | TypeContextInParam);
                C << ' ' << fixKwd(param->name()) << "," << nl;
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
                C << defaultValue(ret, scope, op->getAllMetadata());
            }
            for (const auto& param : outParams)
            {
                C << defaultValue(param->type(), scope, op->getAllMetadata());
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

            for (const auto& param : inParams)
            {
                H << typeToString(param->type(), param->tagged(), scope, param->getAllMetadata(),
                                  _useWstring | TypeContextInParam)
                << ',' << nl;
            }
            if (!op->hasMarshaledResult())
            {
                for (const auto& param : outParams)
                {
                    H << outputTypeToString(param->type(), param->tagged(), scope, param->getAllMetadata(), _useWstring)
                    << ',' << nl;
                }
            }

            H << "const Ice::Current&";
            H.restoreIndent();

            string isConst = ((op->mode() == Operation::Nonmutating) || op->hasMetadata("cpp:const")) ? " const" : "";

            H << ")" << isConst << " override;";

            C << sp << nl << retS << nl;
            C << scope.substr(2) << name << "I::" << fixKwd(opName) << '(';
            C.useCurrentPosAsIndent();

            for (const auto& param : inParams)
            {
                C << typeToString(param->type(), param->tagged(), scope, param->getAllMetadata(),
                                  _useWstring | TypeContextInParam)
                  << " /*" << fixKwd(param->name()) << "*/" << ',' << nl;
            }
            if (!op->hasMarshaledResult())
            {
                for (const auto& param : outParams)
                {
                    C << outputTypeToString(param->type(), param->tagged(), scope, param->getAllMetadata(), _useWstring)
                      << " " << fixKwd(param->name()) << ',' << nl;
                }
            }

            C << "const Ice::Current& current";
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
                        C << defaultValue(ret, scope, op->getAllMetadata());
                        if(!outParams.empty())
                        {
                            C << ", ";
                        }
                    }

                    for(auto q = outParams.begin(); q != outParams.end();)
                    {
                        C << defaultValue((*q)->type(), scope, op->getAllMetadata());
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
                for (const auto& param : outParams)
                {
                    C << nl << fixKwd(param->name()) << " = " << defaultValue(param->type(), scope,
                                                                              op->getAllMetadata())
                      << ";";
                }

                if(ret)
                {
                    C << nl << "return " << defaultValue(ret, scope, op->getAllMetadata()) << ";";
                }
            }
            C << eb;
        }
    }

    H << eb << ';';

    _useWstring = resetUseWstring(_useWstringHist);

    return true;
}
