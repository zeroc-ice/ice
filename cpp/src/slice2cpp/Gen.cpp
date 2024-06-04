//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Gen.h"
#include "../Slice/FileTracker.h"
#include "../Slice/Util.h"
#include "CPlusPlusUtil.h"
#include "IceUtil/FileUtil.h"
#include "IceUtil/StringUtil.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <string.h>

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

namespace
{
    bool isConstexprType(const TypePtr& type)
    {
        BuiltinPtr bp = dynamic_pointer_cast<Builtin>(type);
        if (bp)
        {
            switch (bp->kind())
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
        else if (
            dynamic_pointer_cast<Enum>(type) || dynamic_pointer_cast<InterfaceDecl>(type) ||
            dynamic_pointer_cast<ClassDecl>(type))
        {
            return true;
        }
        else
        {
            StructPtr s = dynamic_pointer_cast<Struct>(type);
            if (s)
            {
                DataMemberList members = s->dataMembers();
                for (DataMemberList::const_iterator i = members.begin(); i != members.end(); ++i)
                {
                    if (!isConstexprType((*i)->type()))
                    {
                        return false;
                    }
                }
                return true;
            }
            return false;
        }
    }

    string getDeprecateSymbol(const ContainedPtr& p1)
    {
        string deprecatedSymbol;
        if (p1->isDeprecated(true))
        {
            if (auto reason = p1->getDeprecationReason(true))
            {
                deprecatedSymbol = "[[deprecated(\"" + *reason + "\")]] ";
            }
            else
            {
                deprecatedSymbol = "[[deprecated]] ";
            }
        }
        return deprecatedSymbol;
    }

    void writeConstantValue(
        IceUtilInternal::Output& out,
        const TypePtr& type,
        const SyntaxTreeBasePtr& valueType,
        const string& value,
        TypeContext typeContext,
        const StringList& metaData,
        const string& scope)
    {
        ConstPtr constant = dynamic_pointer_cast<Const>(valueType);
        if (constant)
        {
            out << getUnqualified(fixKwd(constant->scoped()), scope);
        }
        else
        {
            BuiltinPtr bp = dynamic_pointer_cast<Builtin>(type);
            if (bp && bp->kind() == Builtin::KindString)
            {
                if ((typeContext & TypeContext::UseWstring) != TypeContext::None ||
                    findMetaData(metaData) == "wstring") // wide strings
                {
                    out << "L\"";
                    out << toStringLiteral(value, "\a\b\f\n\r\t\v", "?", UCN, 0);
                    out << "\"";
                }
                else // narrow strings
                {
                    out << "\"" << toStringLiteral(value, "\a\b\f\n\r\t\v", "?", Octal, 0) << "\"";
                }
            }
            else if (bp && bp->kind() == Builtin::KindLong)
            {
                out << "INT64_C(" << value << ")";
            }
            else if (bp && bp->kind() == Builtin::KindFloat)
            {
                out << value;
                if (value.find(".") == string::npos)
                {
                    out << ".0";
                }
                out << "F";
            }
            else
            {
                EnumPtr ep = dynamic_pointer_cast<Enum>(type);
                if (ep && valueType)
                {
                    EnumeratorPtr enumerator = dynamic_pointer_cast<Enumerator>(valueType);
                    assert(enumerator);

                    bool unscoped = findMetaData(ep->getMetaData()) == "%unscoped";

                    if (unscoped)
                    {
                        out << getUnqualified(fixKwd(ep->scope() + enumerator->name()), scope);
                    }
                    else
                    {
                        out << getUnqualified(fixKwd(enumerator->scoped()), scope);
                    }
                }
                else if (!ep)
                {
                    out << value;
                }
            }
        }
    }

    string toDllClassExport(const string& dllExport)
    {
        if (!dllExport.empty())
        {
            return "ICE_CLASS(" + dllExport.substr(0, dllExport.size() - 1) + ") ";
        }
        else
        {
            return "";
        }
    }

    string toDllMemberExport(const string& dllExport)
    {
        if (!dllExport.empty())
        {
            return "ICE_MEMBER(" + dllExport.substr(0, dllExport.size() - 1) + ") ";
        }
        else
        {
            return "";
        }
    }

    // Marshals the parameters of an outgoing request.
    void writeInParamsLambda(
        IceUtilInternal::Output& C,
        const OperationPtr& p,
        const ParamDeclList& inParams,
        const string& scope)
    {
        if (inParams.empty())
        {
            C << "nullptr";
        }
        else
        {
            C << "[&](" << getUnqualified("::Ice::OutputStream*", scope) << " ostr)";
            C << sb;
            writeMarshalCode(C, inParams, nullptr);
            if (p->sendsClasses(false))
            {
                C << nl << "ostr->writePendingValues();";
            }
            C << eb;
        }
    }

    void throwUserExceptionLambda(IceUtilInternal::Output& C, ExceptionList throws, const string& scope)
    {
        if (throws.empty())
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
            for (const auto& ex : throws)
            {
                C << nl << "catch(const " << getUnqualified(fixKwd(ex->scoped()), scope) << "&)";
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

    string marshaledResultStructName(const string& name)
    {
        assert(!name.empty());
        string stName = IceUtilInternal::toUpper(name.substr(0, 1)) + name.substr(1);
        stName += "MarshaledResult";
        return stName;
    }

    string condMove(bool moveIt, const string& str) { return moveIt ? string("::std::move(") + str + ")" : str; }

    string escapeParam(const ParamDeclList& params, const string& name)
    {
        string r = name;
        for (const auto& param : params)
        {
            if (fixKwd(param->name()) == name)
            {
                r = name + "_";
                break;
            }
        }
        return r;
    }

    void writeDocLines(Output& out, const StringList& lines, bool commentFirst, const string& space = " ")
    {
        auto l = lines.cbegin();
        if (!commentFirst)
        {
            assert(l != lines.cend());
            out << *l;
            l++;
        }
        for (; l != lines.cend(); ++l)
        {
            out << nl << " *";
            if (!l->empty())
            {
                out << space << *l;
            }
        }
    }

    void writeSeeAlso(Output& out, const StringList& lines, const string& space = " ")
    {
        for (const string& line : lines)
        {
            out << nl << " *";
            if (!line.empty())
            {
                out << space << "@see " << line;
            }
        }
    }

    string getDocSentence(const StringList& lines)
    {
        //
        // Extract the first sentence.
        //
        ostringstream ostr;
        for (StringList::const_iterator i = lines.begin(); i != lines.end(); ++i)
        {
            const string ws = " \t";

            if (i->empty())
            {
                break;
            }
            if (i != lines.begin() && i->find_first_not_of(ws) == 0)
            {
                ostr << " ";
            }
            string::size_type pos = i->find('.');
            if (pos == string::npos)
            {
                ostr << *i;
            }
            else if (pos == i->size() - 1)
            {
                ostr << *i;
                break;
            }
            else
            {
                //
                // Assume a period followed by whitespace indicates the end of the sentence.
                //
                while (pos != string::npos)
                {
                    if (ws.find((*i)[pos + 1]) != string::npos)
                    {
                        break;
                    }
                    pos = i->find('.', pos + 1);
                }
                if (pos != string::npos)
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

    void writeDocSummary(Output& out, const ContainedPtr& p)
    {
        if (p->comment().empty())
        {
            return;
        }

        CommentPtr doc = p->parseComment(false);

        out << nl << "/**";

        if (!doc->overview().empty())
        {
            writeDocLines(out, doc->overview(), true);
        }

        if (!doc->misc().empty())
        {
            writeDocLines(out, doc->misc(), true);
        }

        if (!doc->seeAlso().empty())
        {
            writeSeeAlso(out, doc->seeAlso());
        }

        if (!doc->deprecated().empty())
        {
            out << nl << " *";
            out << nl << " * @deprecated ";
            writeDocLines(out, doc->deprecated(), false);
        }
        else if (doc->isDeprecated())
        {
            out << nl << " *";
            out << nl << " * @deprecated";
        }

        switch (p->containedType())
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
                if (!q.empty())
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

    enum OpDocParamType
    {
        OpDocInParams,
        OpDocOutParams,
        OpDocAllParams
    };

    void writeOpDocParams(
        Output& out,
        const OperationPtr& op,
        const CommentPtr& doc,
        OpDocParamType type,
        const StringList& preParams = StringList(),
        const StringList& postParams = StringList())
    {
        ParamDeclList params;
        switch (type)
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

        if (!preParams.empty())
        {
            writeDocLines(out, preParams, true);
        }

        map<string, StringList> paramDoc = doc->parameters();
        for (const auto& param : params)
        {
            map<string, StringList>::iterator q = paramDoc.find(param->name());
            if (q != paramDoc.end())
            {
                out << nl << " * @param " << fixKwd(q->first) << " ";
                writeDocLines(out, q->second, false);
            }
        }

        if (!postParams.empty())
        {
            writeDocLines(out, postParams, true);
        }
    }

    void writeOpDocExceptions(Output& out, const OperationPtr& op, const CommentPtr& doc)
    {
        for (const auto& [name, lines] : doc->exceptions())
        {
            string scopedName = name;
            // Try to locate the exception's definition using the name given in the comment.
            ExceptionPtr ex = op->container()->lookupException(name, false);
            if (ex)
            {
                scopedName = ex->scoped().substr(2);
            }
            out << nl << " * @throws " << scopedName << " ";
            writeDocLines(out, lines, false);
        }
    }

    void writeOpDocSummary(
        Output& out,
        const OperationPtr& op,
        const CommentPtr& doc,
        OpDocParamType type,
        bool showExceptions,
        const StringList& preParams = StringList(),
        const StringList& postParams = StringList(),
        const StringList& returns = StringList())
    {
        out << nl << "/**";

        const auto& overview = doc->overview();
        if (!overview.empty())
        {
            writeDocLines(out, overview, true);
        }

        writeOpDocParams(out, op, doc, type, preParams, postParams);

        if (!returns.empty())
        {
            out << nl << " * @return ";
            writeDocLines(out, returns, false);
        }

        if (showExceptions)
        {
            writeOpDocExceptions(out, op, doc);
        }

        const auto& misc = doc->misc();
        if (!misc.empty())
        {
            writeDocLines(out, misc, true);
        }

        const auto& seeAlso = doc->seeAlso();
        if (!seeAlso.empty())
        {
            writeSeeAlso(out, seeAlso);
        }

        const auto& deprecated = doc->deprecated();
        if (!deprecated.empty())
        {
            out << nl << " *";
            out << nl << " * @deprecated ";
            writeDocLines(out, deprecated, false);
        }
        else if (doc->isDeprecated())
        {
            out << nl << " *";
            out << nl << " * @deprecated";
        }

        out << nl << " */";
    }

    // Returns the client-side result type for an operation - can be void, a single type, or a tuple.
    string createOutgoingAsyncTypeParam(const vector<string>& elements)
    {
        switch (elements.size())
        {
            case 0:
                return "void";
            case 1:
                return elements.front();
            default:
            {
                ostringstream os;
                Output out(os);
                out << "::std::tuple" << sabrk;
                for (const auto& element : elements)
                {
                    out << element;
                }
                out << eabrk;
                return os.str();
            }
        }
    }

    // Returns the C++ types that make up the client-side result type of an operation (first return type then out
    // parameter types, as per the future API).
    vector<string> createOutgoingAsyncParams(const OperationPtr& p, const string& scope, TypeContext typeContext)
    {
        vector<string> elements;
        TypePtr ret = p->returnType();
        if (ret)
        {
            elements.push_back(typeToString(ret, p->returnIsOptional(), scope, p->getMetaData(), typeContext));
        }
        for (const auto& param : p->outParameters())
        {
            elements.push_back(
                typeToString(param->type(), param->optional(), scope, param->getMetaData(), typeContext));
        }
        return elements;
    }

    string createLambdaResponse(const OperationPtr& p, TypeContext typeContext)
    {
        ostringstream os;
        Output out(os);
        out << "::std::function<void" << spar << createOutgoingAsyncParams(p, "", typeContext) << epar << ">";
        return os.str();
    }
}

Slice::Gen::Gen(
    const string& base,
    const string& headerExtension,
    const string& sourceExtension,
    const vector<string>& extraHeaders,
    const string& include,
    const vector<string>& includePaths,
    const string& dllExport,
    const string& dir)
    : _base(base),
      _headerExtension(headerExtension),
      _sourceExtension(sourceExtension),
      _extraHeaders(extraHeaders),
      _include(include),
      _includePaths(includePaths),
      _dllExport(dllExport),
      _dir(dir)
{
    for (vector<string>::iterator p = _includePaths.begin(); p != _includePaths.end(); ++p)
    {
        *p = fullPath(*p);
    }

    string::size_type pos = _base.find_last_of("/\\");
    if (pos != string::npos)
    {
        _base.erase(0, pos + 1);
    }
}

Slice::Gen::~Gen()
{
    H << "\n\n#include <IceUtil/PopDisableWarnings.h>";
    H << "\n#endif\n";
    C << '\n';

    H.close();
    C.close();
}

void
Slice::Gen::generate(const UnitPtr& p)
{
    string file = p->topLevelFile();
    DefinitionContextPtr dc = p->findDefinitionContext(file);
    assert(dc);

    //
    // Give precedence to header-ext/source-ext file metadata.
    //
    string headerExtension = getHeaderExt(file, p);
    if (!headerExtension.empty())
    {
        _headerExtension = headerExtension;
    }

    string sourceExtension = getSourceExt(file, p);
    if (!sourceExtension.empty())
    {
        _sourceExtension = sourceExtension;
    }

    //
    // Give precedence to --dll-export command-line option
    //
    if (_dllExport.empty())
    {
        static const string dllExportPrefix = "cpp:dll-export:";
        string meta = dc->findMetaData(dllExportPrefix);
        if (meta.size() > dllExportPrefix.size())
        {
            _dllExport = meta.substr(dllExportPrefix.size());
        }
    }

    string fileH = _base + "." + _headerExtension;
    string fileC = _base + "." + _sourceExtension;
    if (!_dir.empty())
    {
        fileH = _dir + '/' + fileH;
        fileC = _dir + '/' + fileC;
    }

    H.open(fileH.c_str());
    if (!H)
    {
        ostringstream os;
        os << "cannot open `" << fileH << "': " << IceUtilInternal::errorToString(errno);
        throw FileException(__FILE__, __LINE__, os.str());
    }
    FileTracker::instance()->addFile(fileH);

    C.open(fileC.c_str());
    if (!C)
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
    if (_include.size())
    {
        s = _include + '/' + s;
    }
    transform(s.begin(), s.end(), s.begin(), ToIfdef());
    H << "\n#ifndef __" << s << "__";
    H << "\n#define __" << s << "__";
    H << '\n';

    validateMetaData(p);

    writeExtraHeaders(C);

    if (_dllExport.size())
    {
        C << "\n#ifndef " << _dllExport << "_EXPORTS";
        C << "\n#   define " << _dllExport << "_EXPORTS";
        C << "\n#endif";
    }

    C << "\n#define ICE_BUILDING_GENERATED_CODE";
    C << "\n#include \"";
    if (_include.size())
    {
        C << _include << '/';
    }
    C << _base << "." << _headerExtension << "\"";

    H << "\n#include <IceUtil/PushDisableWarnings.h>";

    if (!dc->hasMetaDataDirective("cpp:no-default-include"))
    {
        H << "\n#include <Ice/Ice.h>";
    }

    for (const string& includeFile : p->includeFiles())
    {
        string extension = getHeaderExt(includeFile, p);
        if (extension.empty())
        {
            extension = _headerExtension;
        }
        if (isAbsolutePath(includeFile))
        {
            // This means mcpp found the .ice file in its -I paths. So we generate an angled include for the C++ header.
            H << "\n#include <" << changeInclude(includeFile, _includePaths) << "." << extension << ">";
        }
        else
        {
            H << "\n#include \"" << removeExtension(includeFile) << "." << extension << "\"";
        }
    }

    // Emit #include statements for any cpp:include metadata directives in the top-level Slice file.
    {
        StringList globalMetaData = dc->getMetaData();
        for (StringList::const_iterator q = globalMetaData.begin(); q != globalMetaData.end();)
        {
            string metaData = *q++;
            static const string includePrefix = "cpp:include:";
            static const string sourceIncludePrefix = "cpp:source-include:";
            if (metaData.find(includePrefix) == 0)
            {
                if (metaData.size() > includePrefix.size())
                {
                    H << nl << "#include <" << metaData.substr(includePrefix.size()) << ">";
                }
                else
                {
                    ostringstream ostr;
                    ostr << "ignoring invalid file metadata `" << metaData << "'";
                    dc->warning(InvalidMetaData, file, -1, ostr.str());
                    globalMetaData.remove(metaData);
                }
            }
            else if (metaData.find(sourceIncludePrefix) == 0)
            {
                if (metaData.size() > sourceIncludePrefix.size())
                {
                    C << nl << "#include <" << metaData.substr(sourceIncludePrefix.size()) << ">";
                }
                else
                {
                    ostringstream ostr;
                    ostr << "ignoring invalid file metadata `" << metaData << "'";
                    dc->warning(InvalidMetaData, file, -1, ostr.str());
                    globalMetaData.remove(metaData);
                }
            }
        }
        dc->setMetaData(globalMetaData);
    }

    if (!dc->hasMetaDataDirective("cpp:no-default-include"))
    {
        // For simplicity, we include these extra headers all the time.

        C << "\n#include <Ice/AsyncResponseHandler.h>"; // for async dispatches
        C << "\n#include <Ice/FactoryTable.h>";         // for class and exception factories
        C << "\n#include <Ice/OutgoingAsync.h>";        // for proxies
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
    if (_dllExport.size())
    {
        _dllExport += " ";
    }

    {
        ForwardDeclVisitor forwardDeclVisitor(H);
        p->visit(&forwardDeclVisitor, false);

        DefaultFactoryVisitor defaultFactoryVisitor(C);
        p->visit(&defaultFactoryVisitor, false);

        ProxyVisitor proxyVisitor(H, C, _dllExport);
        p->visit(&proxyVisitor, false);

        DataDefVisitor dataDefVisitor(H, C, _dllExport);
        p->visit(&dataDefVisitor, false);

        InterfaceVisitor interfaceVisitor(H, C, _dllExport);
        p->visit(&interfaceVisitor, false);

        if (!dc->hasMetaDataDirective("cpp:no-stream"))
        {
            StreamVisitor streamVisitor(H);
            p->visit(&streamVisitor, false);
        }
    }
}

void
Slice::Gen::writeExtraHeaders(IceUtilInternal::Output& out)
{
    for (string header : _extraHeaders)
    {
        string guard;
        string::size_type pos = header.rfind(',');
        if (pos != string::npos)
        {
            header = header.substr(0, pos);
            guard = header.substr(pos + 1);
        }
        if (!guard.empty())
        {
            out << "\n#ifndef " << guard;
            out << "\n#define " << guard;
        }
        out << "\n#include <" << header << '>';
        if (!guard.empty())
        {
            out << "\n#endif";
        }
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
    // Validate file metadata in the top-level file and all included files.
    //
    for (const string& file : p->allFiles())
    {
        DefinitionContextPtr dc = p->findDefinitionContext(file);
        StringList globalMetaData = dc->getMetaData();
        assert(dc);
        int headerExtension = 0;
        int sourceExtension = 0;
        int dllExport = 0;
        for (StringList::const_iterator r = globalMetaData.begin(); r != globalMetaData.end();)
        {
            string s = *r++;
            if (s.find(prefix) == 0)
            {
                static const string cppIncludePrefix = "cpp:include:";
                static const string cppNoDefaultInclude = "cpp:no-default-include";
                static const string cppNoStream = "cpp:no-stream";
                static const string cppSourceIncludePrefix = "cpp:source-include";
                static const string cppHeaderExtPrefix = "cpp:header-ext:";
                static const string cppSourceExtPrefix = "cpp:source-ext:";
                static const string cppDllExportPrefix = "cpp:dll-export:";
                static const string cppDoxygenIncludePrefix = "cpp:doxygen:include:";

                if (s == cppNoDefaultInclude || s == cppNoStream)
                {
                    continue;
                }
                else if (s.find(cppIncludePrefix) == 0 && s.size() > cppIncludePrefix.size())
                {
                    continue;
                }
                else if (s.find(cppSourceIncludePrefix) == 0 && s.size() > cppSourceIncludePrefix.size())
                {
                    continue;
                }
                else if (s.find(cppHeaderExtPrefix) == 0 && s.size() > cppHeaderExtPrefix.size())
                {
                    headerExtension++;
                    if (headerExtension > 1)
                    {
                        ostringstream ostr;
                        ostr << "ignoring invalid file metadata `" << s << "': directive can appear only once per file";
                        dc->warning(InvalidMetaData, file, -1, ostr.str());
                        globalMetaData.remove(s);
                    }
                    continue;
                }
                else if (s.find(cppSourceExtPrefix) == 0 && s.size() > cppSourceExtPrefix.size())
                {
                    sourceExtension++;
                    if (sourceExtension > 1)
                    {
                        ostringstream ostr;
                        ostr << "ignoring invalid file metadata `" << s << "': directive can appear only once per file";
                        dc->warning(InvalidMetaData, file, -1, ostr.str());
                        globalMetaData.remove(s);
                    }
                    continue;
                }
                else if (s.find(cppDllExportPrefix) == 0 && s.size() > cppDllExportPrefix.size())
                {
                    dllExport++;
                    if (dllExport > 1)
                    {
                        ostringstream ostr;
                        ostr << "ignoring invalid file metadata `" << s << "': directive can appear only once per file";
                        dc->warning(InvalidMetaData, file, -1, ostr.str());

                        globalMetaData.remove(s);
                    }
                    continue;
                }
                else if (s.find(cppDoxygenIncludePrefix) == 0 && s.size() > cppDoxygenIncludePrefix.size())
                {
                    continue;
                }

                ostringstream ostr;
                ostr << "ignoring invalid file metadata `" << s << "'";
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

bool
Slice::Gen::MetaDataVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    StringList metaData = validate(p, p->getMetaData(), p->file(), p->line());
    p->setMetaData(metaData);
    return true;
}

bool
Slice::Gen::MetaDataVisitor::visitStructStart(const StructPtr& p)
{
    StringList metaData = validate(p, p->getMetaData(), p->file(), p->line());
    p->setMetaData(metaData);
    return true;
}

void
Slice::Gen::MetaDataVisitor::visitOperation(const OperationPtr& p)
{
    TypePtr returnType = p->returnType();
    if (!returnType)
    {
        const DefinitionContextPtr dc = p->unit()->findDefinitionContext(p->file());
        assert(dc);
        StringList metaData = p->getMetaData();
        for (StringList::const_iterator q = metaData.begin(); q != metaData.end();)
        {
            string s = *q++;
            if (s.find("cpp:type:") == 0 || s.find("cpp:view-type:") == 0 || s == "cpp:array")
            {
                dc->warning(
                    InvalidMetaData,
                    p->file(),
                    p->line(),
                    "ignoring invalid metadata `" + s + "' for operation with void return type");
                metaData.remove(s);
            }
        }
        p->setMetaData(metaData);
    }
    else
    {
        p->setMetaData(validate(returnType, p->getMetaData(), p->file(), p->line(), true));
    }

    for (const auto& param : p->parameters())
    {
        param->setMetaData(validate(param->type(), param->getMetaData(), p->file(), param->line(), true));
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
Slice::Gen::MetaDataVisitor::validate(
    const SyntaxTreeBasePtr& cont,
    const StringList& metaData,
    const string& file,
    const string& line,
    bool operation)
{
    static const string cppPrefix = "cpp:";

    const UnitPtr ut = cont->unit();
    const DefinitionContextPtr dc = ut->findDefinitionContext(file);
    assert(dc);
    StringList newMetaData = metaData;
    for (const string& s : metaData)
    {
        // Issue friendly warning for cpp11 and cpp98 metadata what were removed as Slice does not issue warnings
        // for unknown "top-level" metadata.
        if (s.find("cpp11:") == 0 || s.find("cpp98:") == 0)
        {
            dc->warning(InvalidMetaData, file, line, "ignoring invalid metadata `" + s + "'");
            newMetaData.remove(s);
            continue;
        }

        if (s.find(cppPrefix) != 0)
        {
            continue;
        }

        if (operation && s == "cpp:const")
        {
            continue;
        }

        string ss = s.substr(cppPrefix.size());
        if (ss == "type:wstring" || ss == "type:string")
        {
            BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(cont);
            ModulePtr module = dynamic_pointer_cast<Module>(cont);
            ClassDefPtr clss = dynamic_pointer_cast<ClassDef>(cont);
            InterfaceDefPtr interface = dynamic_pointer_cast<InterfaceDef>(cont);
            StructPtr strct = dynamic_pointer_cast<Struct>(cont);
            ExceptionPtr exception = dynamic_pointer_cast<Exception>(cont);
            if ((builtin && builtin->kind() == Builtin::KindString) || module || clss || strct || interface ||
                exception)
            {
                continue;
            }
        }
        if (dynamic_pointer_cast<Sequence>(cont))
        {
            if (ss.find("type:") == 0 || ss.find("view-type:") == 0 || ss == "array")
            {
                continue;
            }
        }
        if (dynamic_pointer_cast<Dictionary>(cont) && (ss.find("type:") == 0 || ss.find("view-type:") == 0))
        {
            continue;
        }
        if (dynamic_pointer_cast<Exception>(cont) && ss == "ice_print")
        {
            continue;
        }
        if (dynamic_pointer_cast<Enum>(cont) && ss == "unscoped")
        {
            continue;
        }

        {
            ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(cont);
            if (cl && ss.find("type:") == 0)
            {
                continue;
            }
        }

        dc->warning(InvalidMetaData, file, line, "ignoring invalid metadata `" + s + "'");
        newMetaData.remove(s);
    }
    return newMetaData;
}

TypeContext
Slice::Gen::setUseWstring(ContainedPtr p, list<TypeContext>& hist, TypeContext typeCtx)
{
    hist.push_back(typeCtx);
    StringList metaData = p->getMetaData();
    if (find(metaData.begin(), metaData.end(), "cpp:type:wstring") != metaData.end())
    {
        typeCtx = TypeContext::UseWstring;
    }
    else if (find(metaData.begin(), metaData.end(), "cpp:type:string") != metaData.end())
    {
        typeCtx = TypeContext::None;
    }
    return typeCtx;
}

TypeContext
Slice::Gen::resetUseWstring(list<TypeContext>& hist)
{
    TypeContext use = hist.back();
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
    if (meta.size() > headerExtPrefix.size())
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
    if (meta.size() > sourceExtPrefix.size())
    {
        ext = meta.substr(sourceExtPrefix.size());
    }
    return ext;
}

Slice::Gen::ForwardDeclVisitor::ForwardDeclVisitor(Output& h) : H(h), _useWstring(TypeContext::None) {}

bool
Slice::Gen::ForwardDeclVisitor::visitModuleStart(const ModulePtr& p)
{
    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);
    H << sp << nl << "namespace " << fixKwd(p->name()) << nl << '{';
    H.inc();
    return true;
}

void
Slice::Gen::ForwardDeclVisitor::visitModuleEnd(const ModulePtr&)
{
    H.dec();
    H << nl << '}';
    _useWstring = resetUseWstring(_useWstringHist);
}

void
Slice::Gen::ForwardDeclVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    ClassDefPtr def = p->definition();
    string name = fixKwd(p->name());

    H << nl << "class " << name << ';';
    H << nl << "using " << p->name() << "Ptr = ::std::shared_ptr<" << name << ">;" << sp;
}

bool
Slice::Gen::ForwardDeclVisitor::visitStructStart(const StructPtr& p)
{
    H << nl << "struct " << fixKwd(p->name()) << ';' << sp;
    return false;
}

void
Slice::Gen::ForwardDeclVisitor::visitInterfaceDecl(const InterfaceDeclPtr& p)
{
    H << nl << "class " << p->name() << "Prx;" << sp;
}

void
Slice::Gen::ForwardDeclVisitor::visitEnum(const EnumPtr& p)
{
    bool unscoped = findMetaData(p->getMetaData()) == "%unscoped";
    writeDocSummary(H, p);
    H << nl << "enum ";
    if (!unscoped)
    {
        H << "class ";
    }
    H << fixKwd(p->name());
    if (!unscoped && p->maxValue() <= 0xFF)
    {
        H << " : ::std::uint8_t";
    }
    H << sb;

    EnumeratorList enumerators = p->enumerators();
    //
    // Check if any of the enumerators were assigned an explicit value.
    //
    const bool explicitValue = p->explicitValue();
    for (EnumeratorList::const_iterator en = enumerators.begin(); en != enumerators.end();)
    {
        writeDocSummary(H, *en);
        H << nl << fixKwd((*en)->name());
        //
        // If any of the enumerators were assigned an explicit value, we emit
        // an explicit value for *all* enumerators.
        //
        if (explicitValue)
        {
            H << " = " << std::to_string((*en)->value());
        }
        if (++en != enumerators.end())
        {
            H << ',';
        }
    }
    H << eb << ';' << sp;
}

void
Slice::Gen::ForwardDeclVisitor::visitSequence(const SequencePtr& p)
{
    string name = fixKwd(p->name());
    string scope = fixKwd(p->scope());
    TypePtr type = p->type();
    TypeContext typeCtx = _useWstring;
    StringList metaData = p->getMetaData();

    string seqType = findMetaData(metaData, _useWstring);
    writeDocSummary(H, p);

    if (!seqType.empty())
    {
        H << nl << "using " << name << " = " << seqType << ';' << sp;
    }
    else
    {
        auto builtin = dynamic_pointer_cast<Builtin>(type);
        if (builtin && builtin->kind() == Builtin::KindByte)
        {
            H << nl << "using " << name << " = ::std::vector<std::byte>;" << sp;
        }
        else
        {
            string s = typeToString(type, false, scope, p->typeMetaData(), typeCtx);
            H << nl << "using " << name << " = ::std::vector<" << s << ">;" << sp;
        }
    }
}

void
Slice::Gen::ForwardDeclVisitor::visitDictionary(const DictionaryPtr& p)
{
    string name = fixKwd(p->name());
    string scope = fixKwd(p->scope());
    string dictType = findMetaData(p->getMetaData());
    TypeContext typeCtx = _useWstring;

    writeDocSummary(H, p);

    if (dictType.empty())
    {
        //
        // A default std::map dictionary
        //
        TypePtr keyType = p->keyType();
        TypePtr valueType = p->valueType();
        string ks = typeToString(keyType, false, scope, p->keyMetaData(), typeCtx);
        string vs = typeToString(valueType, false, scope, p->valueMetaData(), typeCtx);

        H << nl << "using " << name << " = ::std::map<" << ks << ", " << vs << ">;" << sp;
    }
    else
    {
        //
        // A custom dictionary
        //
        H << nl << "using " << name << " = " << dictType << ';' << sp;
    }
}

void
Slice::Gen::ForwardDeclVisitor::visitConst(const ConstPtr& p)
{
    const string scope = fixKwd(p->scope());
    writeDocSummary(H, p);
    H << nl << (isConstexprType(p->type()) ? "constexpr " : "const ")
      << typeToString(p->type(), false, scope, p->typeMetaData(), _useWstring) << " " << fixKwd(p->name()) << " = ";
    writeConstantValue(H, p->type(), p->valueType(), p->value(), _useWstring, p->typeMetaData(), scope);
    H << ';' << sp;
}

Slice::Gen::DefaultFactoryVisitor::DefaultFactoryVisitor(Output& c) : C(c), _factoryTableInitDone(false) {}

bool
Slice::Gen::DefaultFactoryVisitor::visitUnitStart(const UnitPtr& p)
{
    if (p->hasClassDefs() || p->hasExceptions())
    {
        C << sp << nl << "namespace" << nl << "{";
        C.inc();
        return true;
    }
    else
    {
        return false;
    }
}

void
Slice::Gen::DefaultFactoryVisitor::visitUnitEnd(const UnitPtr&)
{
    C.dec();
    C << nl << "}";
}

bool
Slice::Gen::DefaultFactoryVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if (!_factoryTableInitDone)
    {
        // Make sure the global factory table is initialized before we use it.
        C << nl << "const ::IceInternal::FactoryTableInit iceC_factoryTableInit;";
        _factoryTableInitDone = true;
    }

    C << nl << "const ::IceInternal::DefaultValueFactoryInit<" << fixKwd(p->scoped()) << "> ";
    C << "iceC" + p->flattenedScope() + p->name() + "_init"
      << "(\"" << p->scoped() << "\");";

    if (p->compactId() >= 0)
    {
        string n = "iceC" + p->flattenedScope() + p->name() + "_compactIdInit ";
        C << nl << "const ::IceInternal::CompactIdInit " << n << "(\"" << p->scoped() << "\", " << p->compactId()
          << ");";
    }
    return false;
}

bool
Slice::Gen::DefaultFactoryVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    if (!_factoryTableInitDone)
    {
        // Make sure the global factory table is initialized before we use it.
        C << nl << "const ::IceInternal::FactoryTableInit iceC_factoryTableInit;";
        _factoryTableInitDone = true;
    }
    C << nl << "const ::IceInternal::DefaultUserExceptionFactoryInit<" << fixKwd(p->scoped()) << "> ";
    C << "iceC" + p->flattenedScope() + p->name() + "_init"
      << "(\"" << p->scoped() << "\");";
    return false;
}

Slice::Gen::ProxyVisitor::ProxyVisitor(Output& h, Output& c, const string& dllExport)
    : H(h),
      C(c),
      _dllExport(dllExport),
      _useWstring(TypeContext::None)
{
}

bool
Slice::Gen::ProxyVisitor::visitModuleStart(const ModulePtr& p)
{
    if (!p->hasInterfaceDefs())
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

    const string scope = fixKwd(p->scope());
    InterfaceList bases = p->bases();

    H << sp;
    writeDocSummary(H, p);
    H << nl << "class " << _dllExport << p->name() << "Prx : public " << getUnqualified("::Ice::Proxy", scope) << "<"
      << fixKwd(p->name() + "Prx") << ", ";
    if (bases.empty())
    {
        H << getUnqualified("::Ice::ObjectPrx", scope);
    }
    else
    {
        InterfaceList::const_iterator q = bases.begin();
        while (q != bases.end())
        {
            H << getUnqualified(fixKwd((*q)->scoped() + "Prx"), scope);
            if (++q != bases.end())
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
    const string prx = fixKwd(p->name() + "Prx");
    const string scoped = fixKwd(p->scoped() + "Prx");
    InterfaceList bases = p->allBases();

    H << sp;
    H << nl << "/**";
    H << nl << " * Obtains the Slice type ID of this interface.";
    H << nl << " * @return The fully-scoped type ID.";
    H << nl << " */";
    H << nl << "static ::std::string_view ice_staticId() noexcept;";

    if (!bases.empty())
    {
        // -Wextra wants to initialize all the virtual base classes _in the right order_, which is not practical, and
        // is useless here.
        H << sp;
        H.zeroIndent();
        H << nl << "#if defined(__GNUC__)";
        H << nl << "#   pragma GCC diagnostic push";
        H << nl << "#   pragma GCC diagnostic ignored \"-Wextra\" // initialize all virtual bases in correct order";
        H << nl << "#endif";
        H.restoreIndent();
    }

    // TODO: generate doc-comments.
    H << sp;
    H << nl << "explicit " << prx << "(const ::Ice::ObjectPrx& other) : ::Ice::ObjectPrx(other)";
    H << sb << eb;
    H << sp;

    // We can't use "= default" for the copy/move ctor/assignment operator as it's not correct with virtual inheritance.

    H << nl << prx << "(const " << prx << "& other) noexcept : ::Ice::ObjectPrx(other)";
    H << sb << eb;
    H << sp;
    H << nl << prx << "(" << prx << "&& other) noexcept : ::Ice::ObjectPrx(::std::move(other))";
    H << sb << eb;
    H << sp;
    H << nl << prx << "(const ::Ice::CommunicatorPtr& communicator, std::string_view proxyString) :";
    H.inc();
    H << nl << "::Ice::ObjectPrx(communicator, proxyString)";
    H.dec();
    H << sb << eb;
    H << sp;
    H << nl << prx << "& operator=(const " << prx << "& rhs) noexcept";
    H << sb;
    H << nl << "::Ice::ObjectPrx::operator=(rhs);";
    H << nl << "return *this;";
    H << eb;
    H << sp;
    H << nl << prx << "& operator=(" << prx << "&& rhs) noexcept";
    H << sb;
    H << nl << "::Ice::ObjectPrx::operator=(::std::move(rhs));";
    H << nl << "return *this;";
    H << eb;
    H << sp;
    H << nl << "/// \\cond INTERNAL";
    H << nl << "static " << prx << " _fromReference(::IceInternal::ReferencePtr ref) { return " << prx
      << "(::std::move(ref)); }";
    H.dec();
    H << sp << nl << "protected:";
    H.inc();
    H << sp;
    H << nl << prx << "() = default;";
    H << sp;
    H << nl << "explicit " << prx << "(::IceInternal::ReferencePtr&& ref) : ::Ice::ObjectPrx(::std::move(ref))";
    H << sb << eb;
    H << nl << "/// \\endcond";

    if (!bases.empty())
    {
        // -Wextra wants to initialize all the virtual base classes _in the right order_, which is not practical, and
        // is useless here.
        H << sp;
        H.zeroIndent();
        H << nl << "#if defined(__GNUC__)";
        H << nl << "#   pragma GCC diagnostic pop";
        H << nl << "#endif";
        H.restoreIndent();
    }

    H << eb << ';';

    C << sp;
    C << nl << "::std::string_view" << nl << scoped.substr(2) << "::ice_staticId() noexcept";
    C << sb;
    C << nl << "static constexpr ::std::string_view typeId = \"" << p->scoped() << "\";";
    C << nl << "return typeId;";
    C << eb;

    _useWstring = resetUseWstring(_useWstringHist);
}

void
Slice::Gen::ProxyVisitor::visitOperation(const OperationPtr& p)
{
    string name = p->name();
    InterfaceDefPtr interface = p->interface();
    string interfaceScope = fixKwd(interface->scope());

    TypePtr ret = p->returnType();

    bool retIsOpt = p->returnIsOptional();
    string retS = ret ? typeToString(ret, retIsOpt, interfaceScope, p->getMetaData(), _useWstring) : "void";
    string retSImpl = ret ? typeToString(ret, retIsOpt, "", p->getMetaData(), _useWstring) : "void";

    // All parameters
    vector<string> paramsDecl;
    vector<string> paramsImplDecl;

    // Only the parameters marshaled into the request.
    vector<string> inParamsDecl;
    vector<string> inParamsImplDecl;
    vector<string> inParamsImpl;

    vector<string> futureOutParams = createOutgoingAsyncParams(p, interfaceScope, _useWstring);
    vector<string> lambdaOutParams =
        createOutgoingAsyncParams(p, interfaceScope, _useWstring | TypeContext::UnmarshalParamZeroCopy);

    const string futureImplPrefix = "_iceI_";
    const string lambdaImplPrefix = futureOutParams == lambdaOutParams ? "_iceI_" : "_iceIL_";

    ParamDeclList paramList = p->parameters();
    ParamDeclList inParams = p->inParameters();
    ParamDeclList outParams = p->outParameters();

    for (const auto& q : paramList)
    {
        string paramName = fixKwd(q->name());
        StringList metaData = q->getMetaData();

        if (q->isOutParam())
        {
            string outputTypeString =
                outputTypeToString(q->type(), q->optional(), interfaceScope, metaData, _useWstring);

            paramsDecl.push_back(outputTypeString + ' ' + paramName);
            paramsImplDecl.push_back(outputTypeString + ' ' + paramPrefix + q->name());
        }
        else
        {
            string typeString = inputTypeToString(q->type(), q->optional(), interfaceScope, metaData, _useWstring);

            paramsDecl.push_back(typeString + ' ' + paramName);
            paramsImplDecl.push_back(typeString + ' ' + paramPrefix + q->name());

            inParamsDecl.push_back(typeString + ' ' + paramName);
            inParamsImplDecl.push_back(typeString + ' ' + paramPrefix + q->name());
            inParamsImpl.push_back(paramPrefix + q->name());
        }
    }

    string scoped = fixKwd(interface->scope() + interface->name() + "Prx" + "::").substr(2);

    const string contextParam = escapeParam(paramList, "context");
    const string contextDef = "const " + getUnqualified("::Ice::Context&", interfaceScope) + " " + contextParam;
    const string contextDecl = contextDef + " = " + getUnqualified("::Ice::noExplicitContext", interfaceScope);

    string futureT = createOutgoingAsyncTypeParam(futureOutParams);
    string futureTAbsolute = createOutgoingAsyncTypeParam(createOutgoingAsyncParams(p, "", _useWstring));
    string lambdaT = createOutgoingAsyncTypeParam(lambdaOutParams);

    const string deprecateSymbol = getDeprecateSymbol(p);

    CommentPtr comment = p->parseComment(false);
    const string contextDoc = "@param " + contextParam + " The Context map to send with the invocation.";
    const string futureDoc = "The future object for the invocation.";

    //
    // Synchronous operation
    //
    H << sp;
    if (comment)
    {
        StringList postParams;
        postParams.push_back(contextDoc);
        writeOpDocSummary(H, p, comment, OpDocAllParams, true, StringList(), postParams, comment->returns());
    }
    H << nl << deprecateSymbol << retS << ' ' << fixKwd(name) << spar << paramsDecl << contextDecl << epar << " const;";

    C << sp;
    C << nl << retSImpl << nl << scoped << fixKwd(name) << spar << paramsImplDecl << "const ::Ice::Context& context"
      << epar << " const";
    C << sb;
    C << nl;
    if (futureOutParams.size() == 1)
    {
        if (ret)
        {
            C << "return ";
        }
        else
        {
            C << paramPrefix << (*outParams.begin())->name() << " = ";
        }
    }
    else if (futureOutParams.size() > 1)
    {
        C << "auto result = ";
    }

    // We call makePromiseOutgoing with the "sync" parameter set to true; the Promise/future implementation later on
    // calls makePromiseOutgoing with this parameter set to false. This parameter is useful for collocated calls.
    C << "::IceInternal::makePromiseOutgoing<" << futureT << ">";
    C << spar << "true, this"
      << "&" + interface->name() + "Prx::_iceI_" + name;
    C << inParamsImpl;
    C << "context" << epar << ".get();";
    if (futureOutParams.size() > 1)
    {
        int index = ret ? 1 : 0;
        for (const auto& q : outParams)
        {
            C << nl << paramPrefix << q->name() << " = ";
            C << condMove(isMovable(q->type()), "::std::get<" + std::to_string(index++) + ">(result)") << ";";
        }
        if (ret)
        {
            C << nl << "return " + condMove(isMovable(ret), "::std::get<0>(result)") + ";";
        }
    }
    C << eb;

    //
    // Promise based asynchronous operation
    //
    H << sp;
    if (comment)
    {
        StringList postParams, returns;
        postParams.push_back(contextDoc);
        returns.push_back(futureDoc);
        writeOpDocSummary(H, p, comment, OpDocInParams, false, StringList(), postParams, returns);
    }

    H << nl << deprecateSymbol << "::std::future<" << futureT << "> " << name << "Async" << spar << inParamsDecl
      << contextDecl << epar << " const;";

    C << sp;
    C << nl << "::std::future<" << futureTAbsolute << ">";
    C << nl;
    C << scoped << name << "Async" << spar << inParamsImplDecl << "const ::Ice::Context& context" << epar << " const";

    C << sb;
    C << nl << "return ::IceInternal::makePromiseOutgoing<" << futureT << ">" << spar;
    C << "false, this" << string("&" + interface->name() + "Prx::_iceI_" + name);
    C << inParamsImpl;
    C << "context" << epar << ";";
    C << eb;

    //
    // Lambda based asynchronous operation
    //

    const string responseParam = escapeParam(inParams, "response");
    const string exParam = escapeParam(inParams, "ex");
    const string sentParam = escapeParam(inParams, "sent");
    const string lambdaResponse = createLambdaResponse(p, _useWstring | TypeContext::UnmarshalParamZeroCopy);

    H << sp;
    if (comment)
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
    H << deprecateSymbol;
    H << "::std::function<void()>";

    // TODO: need "nl" version of spar/epar
    H << nl << name << "Async" << spar;
    H.useCurrentPosAsIndent();
    H << inParamsDecl;

    H << lambdaResponse + " " + responseParam;
    H << "::std::function<void(::std::exception_ptr)> " + exParam + " = nullptr";
    H << "::std::function<void(bool)> " + sentParam + " = nullptr";
    H << contextDecl << epar << " const;";
    H.restoreIndent();

    C << sp;
    C << nl << "::std::function<void()>";
    C << nl << scoped << name << "Async" << spar;
    C.useCurrentPosAsIndent();
    C << inParamsImplDecl;
    C << lambdaResponse + " response";
    C << "::std::function<void(::std::exception_ptr)> ex";
    C << "::std::function<void(bool)> sent";
    C << "const ::Ice::Context& context" << epar << " const";
    C.restoreIndent();

    C << sb;
    if (lambdaOutParams.size() > 1)
    {
        C << nl << "auto responseCb = [response = ::std::move(response)](" << lambdaT << "&& result)";
        C << sb;
        C << nl << "::std::apply(::std::move(response), ::std::move(result));";
        C << eb << ";";
    }

    C << nl << "return ::IceInternal::makeLambdaOutgoing<" << lambdaT << ">" << spar;

    C << "::std::move(" + (lambdaOutParams.size() > 1 ? string("responseCb") : "response") + ")"
      << "::std::move(ex)"
      << "::std::move(sent)"
      << "this";
    C << string("&" + getUnqualified(scoped, interfaceScope.substr(2)) + lambdaImplPrefix + name);
    C << inParamsImpl;
    C << "context" << epar << ";";
    C << eb;

    // Implementation

    emitOperationImpl(p, futureImplPrefix, futureOutParams);

    if (lambdaImplPrefix != futureImplPrefix)
    {
        emitOperationImpl(p, lambdaImplPrefix, lambdaOutParams);
    }
}

void
Slice::Gen::ProxyVisitor::emitOperationImpl(
    const OperationPtr& p,
    const string& prefix,
    const std::vector<std::string>& outgoingAsyncParams)
{
    string name = p->name();
    InterfaceDefPtr interface = p->interface();
    string interfaceScope = fixKwd(interface->scope());

    TypePtr ret = p->returnType();

    bool retIsOpt = p->returnIsOptional();
    string retS = ret ? typeToString(ret, retIsOpt, interfaceScope, p->getMetaData(), _useWstring) : "void";
    string retSImpl = ret ? typeToString(ret, retIsOpt, "", p->getMetaData(), _useWstring) : "void";

    vector<string> inParamsS;
    vector<string> inParamsImplDecl;

    ParamDeclList inParams = p->inParameters();
    ParamDeclList outParams = p->outParameters();

    bool outParamsHasOpt = false;

    if (ret)
    {
        outParamsHasOpt |= p->returnIsOptional();
    }
    outParamsHasOpt |=
        std::find_if(outParams.begin(), outParams.end(), [](const ParamDeclPtr& q) { return q->optional(); }) !=
        outParams.end();

    for (const auto& q : inParams)
    {
        string typeString = inputTypeToString(q->type(), q->optional(), interfaceScope, q->getMetaData(), _useWstring);

        inParamsS.push_back(typeString);
        inParamsImplDecl.push_back(typeString + ' ' + paramPrefix + q->name());
    }

    string scoped = fixKwd(interface->scope() + interface->name() + "Prx" + "::").substr(2);

    string returnT = createOutgoingAsyncTypeParam(outgoingAsyncParams);

    string implName = prefix + name;

    H << sp;
    H << nl << "/// \\cond INTERNAL";
    H << nl << "void " << implName << spar;
    H << "const ::std::shared_ptr<::IceInternal::OutgoingAsyncT<" + returnT + ">>&";
    H << inParamsS;
    H << ("const " + getUnqualified("::Ice::Context&", interfaceScope));
    H << epar << " const;";
    H << nl << "/// \\endcond";

    C << sp;
    C << nl << "void" << nl << scoped << implName << spar;
    C << "const ::std::shared_ptr<::IceInternal::OutgoingAsyncT<" + returnT + ">>& outAsync";
    C << inParamsImplDecl << ("const " + getUnqualified("::Ice::Context&", interfaceScope) + " context");
    C << epar << " const";
    C << sb;
    C << nl << "static constexpr ::std::string_view operationName = \"" << name << "\";";
    C << sp;
    if (p->returnsData())
    {
        C << nl << "_checkTwowayOnly(operationName);";
    }
    C << nl << "outAsync->invoke(operationName, ";
    C << getUnqualified(operationModeToString(p->mode()), interfaceScope) << ", "
      << getUnqualified(opFormatTypeToString(p), interfaceScope) << ", context,";
    C.inc();
    C << nl;

    writeInParamsLambda(C, p, inParams, interfaceScope);
    C << "," << nl;
    throwUserExceptionLambda(C, p->throws(), interfaceScope);

    if (outgoingAsyncParams.size() > 1)
    {
        //
        // Generate a read method if there are more than one ret/out parameter. If there's
        // only one, we rely on the default read method from LambdaOutgoing
        // except if the unique ret/out is optional or is an array.
        //
        C << "," << nl << "[](" << getUnqualified("::Ice::InputStream*", interfaceScope) << " istr)";
        C << sb;
        C << nl << returnT << " v;";

        writeUnmarshalCode(C, outParams, p);

        if (p->returnsClasses(false))
        {
            C << nl << "istr->readPendingValues();";
        }
        C << nl << "return v;";
        C << eb;
    }
    else if (outParamsHasOpt || p->returnsClasses(false))
    {
        //
        // If there's only one optional ret/out parameter, we still need to generate
        // a read method, we can't rely on the default read method which wouldn't
        // known which tag to use.
        //
        C << "," << nl << "[](" << getUnqualified("::Ice::InputStream*", interfaceScope) << " istr)";
        C << sb;

        writeAllocateCode(C, outParams, p, interfaceScope, _useWstring);
        writeUnmarshalCode(C, outParams, p);

        if (p->returnsClasses(false))
        {
            C << nl << "istr->readPendingValues();";
        }

        if (ret)
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
}

Slice::Gen::DataDefVisitor::DataDefVisitor(
    IceUtilInternal::Output& h,
    IceUtilInternal::Output& c,
    const string& dllExport)
    : H(h),
      C(c),
      _dllExport(dllExport),
      _dllClassExport(toDllClassExport(dllExport)),
      _dllMemberExport(toDllMemberExport(dllExport)),
      _doneStaticSymbol(false),
      _useWstring(TypeContext::None)
{
}

bool
Slice::Gen::DataDefVisitor::visitModuleStart(const ModulePtr& p)
{
    // TODO: this most likely includes definitions in included files, which is not what we want here.
    if (!p->hasStructs() && !p->hasValueDefs() && !p->hasExceptions())
    {
        return false;
    }

    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);
    H << sp << nl << "namespace " << fixKwd(p->name()) << nl << '{';
    return true;
}

void
Slice::Gen::DataDefVisitor::visitModuleEnd(const ModulePtr& p)
{
    if (p->hasStructs())
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
Slice::Gen::DataDefVisitor::visitStructStart(const StructPtr& p)
{
    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    H << sp;
    writeDocSummary(H, p);
    H << nl << "struct " << fixKwd(p->name());
    H << sb;

    return true;
}

void
Slice::Gen::DataDefVisitor::visitStructEnd(const StructPtr& p)
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
Slice::Gen::DataDefVisitor::visitDataMember(const DataMemberPtr& p)
{
    auto container = p->container();
    if (dynamic_pointer_cast<Struct>(container) || dynamic_pointer_cast<Exception>(container))
    {
        emitDataMember(p);
    }
    // else don't do anything - visitClassXxx already calls emitDataMember explicitly.
}

bool
Slice::Gen::DataDefVisitor::visitExceptionStart(const ExceptionPtr& p)
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
    map<string, CommentPtr> allComments;

    for (const auto& dataMember : dataMembers)
    {
        params.push_back(fixKwd(dataMember->name()));
    }

    for (const auto& dataMember : allDataMembers)
    {
        string typeName =
            typeToString(dataMember->type(), dataMember->optional(), scope, dataMember->getMetaData(), _useWstring);
        allParamDecls.push_back(typeName + " " + fixKwd(dataMember->name()));

        CommentPtr comment = dataMember->parseComment(false);
        if (comment)
        {
            allComments[dataMember->name()] = comment;
        }
    }

    if (base)
    {
        baseDataMembers = base->allDataMembers();
    }

    string baseClass =
        base ? getUnqualified(fixKwd(base->scoped()), scope) : getUnqualified("::Ice::UserException", scope);
    string baseName = base ? fixKwd(base->name()) : "UserException";

    H << sp;
    writeDocSummary(H, p);
    H << nl << "class " << _dllClassExport << name << " : public " << baseClass;
    H << sb;

    H.dec();
    H << nl << "public:";
    H.inc();

    H << nl << "using " << baseClass << "::" << baseName << ";";

    if (!allDataMembers.empty())
    {
        H << sp;
        H << nl << "/**";
        H << nl << " * One-shot constructor to initialize all data members.";
        for (const auto& dataMember : allDataMembers)
        {
            map<string, CommentPtr>::iterator r = allComments.find(dataMember->name());
            if (r != allComments.end())
            {
                H << nl << " * @param " << fixKwd(r->first) << " " << getDocSentence(r->second->overview());
            }
        }
        H << nl << " */";
        H << nl << name << "(";

        for (vector<string>::const_iterator q = allParamDecls.begin(); q != allParamDecls.end(); ++q)
        {
            if (q != allParamDecls.begin())
            {
                H << ", ";
            }
            H << (*q);
        }
        H << ") noexcept :";
        H.inc();
        if (base || !baseDataMembers.empty())
        {
            H << nl << baseClass << "(";

            for (DataMemberList::const_iterator q = baseDataMembers.begin(); q != baseDataMembers.end(); ++q)
            {
                if (q != baseDataMembers.begin())
                {
                    H << ", ";
                }
                string memberName = fixKwd((*q)->name());
                TypePtr memberType = (*q)->type();
                H << condMove(isMovable(memberType), memberName);
            }

            H << ")";
            if (!dataMembers.empty())
            {
                H << ",";
            }
        }

        for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            string memberName = fixKwd((*q)->name());
            TypePtr memberType = (*q)->type();

            if (q != dataMembers.begin())
            {
                H << ",";
            }
            H << nl << memberName << "(" << condMove(isMovable(memberType), memberName) << ")";
        }

        H.dec();
        H << sb;
        H << eb;
    }

    if (!dataMembers.empty())
    {
        H << sp;
        H << nl << "/**";
        H << nl << " * Obtains a tuple containing all of the exception's data members.";
        H << nl << " * @return The data members in a tuple.";
        H << nl << " */";
        writeIceTuple(H, p->allDataMembers(), _useWstring);
    }

    H << sp;
    H << nl << "/**";
    H << nl << " * Obtains the Slice type ID of this exception.";
    H << nl << " * @return The fully-scoped type ID.";
    H << nl << " */";
    H << nl << _dllMemberExport << "static ::std::string_view ice_staticId() noexcept;";

    C << sp << nl << "::std::string_view" << nl << scoped.substr(2) << "::ice_staticId() noexcept";
    C << sb;
    C << nl << "static constexpr ::std::string_view typeId = \"" << p->scoped() << "\";";
    C << nl << "return typeId;";
    C << eb;

    StringList metaData = p->getMetaData();
    if (find(metaData.begin(), metaData.end(), "cpp:ice_print") != metaData.end())
    {
        H << nl << "/**";
        H << nl << " * Prints this exception to the given stream.";
        H << nl << " * @param stream The target stream.";
        H << nl << " */";
        H << nl << _dllMemberExport << "void ice_print(::std::ostream& stream) const override;";
    }

    H << sp << nl << _dllMemberExport << "::std::string ice_id() const override;";
    C << sp << nl << "::std::string" << nl << scoped.substr(2) << "::ice_id() const";
    C << sb;
    C << nl << "return ::std::string{ice_staticId()};";
    C << eb;

    H << sp << nl << _dllMemberExport << "void ice_throw() const override;";
    C << sp << nl << "void" << nl << scoped.substr(2) << "::ice_throw() const";
    C << sb;
    C << nl << "throw *this;";
    C << eb;

    if (p->usesClasses(false))
    {
        if (!base || !base->usesClasses(false))
        {
            H << sp;
            H << nl << "/// \\cond STREAM";
            H << nl << _dllMemberExport << "bool _usesClasses() const override;";
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

    if (!dataMembers.empty())
    {
        H << sp;
    }
    return true;
}

void
Slice::Gen::DataDefVisitor::visitExceptionEnd(const ExceptionPtr& p)
{
    string name = fixKwd(p->name());
    string scope = fixKwd(p->scope());
    string scoped = fixKwd(p->scoped());
    DataMemberList dataMembers = p->dataMembers();

    ExceptionPtr base = p->base();
    string baseClass =
        base ? getUnqualified(fixKwd(base->scoped()), scope) : getUnqualified("::Ice::UserException", scope);

    H.dec();
    H << sp << nl << "protected:";
    H.inc();

    H << nl << _dllMemberExport << "void _writeImpl(::Ice::OutputStream*) const override;";
    C << sp << nl << "void" << nl << scoped.substr(2) << "::_writeImpl(::Ice::OutputStream* ostr) const";
    C << sb;
    // lastSlice is true or false.
    C << nl << "ostr->startSlice(ice_staticId(), -1, " << (base ? "false" : "true") << ");";
    if (!dataMembers.empty())
    {
        C << nl << "::Ice::StreamWriter<" << name << ">::write(ostr, *this);";
    }
    C << nl << "ostr->endSlice();";
    if (base)
    {
        C << nl << baseClass << "::_writeImpl(ostr);";
    }
    C << eb;

    H << sp << nl << _dllMemberExport << "void _readImpl(::Ice::InputStream*) override;";
    C << sp << nl << "void" << nl << scoped.substr(2) << "::_readImpl(::Ice::InputStream* istr)";
    C << sb;
    C << nl << "istr->startSlice();";
    if (!dataMembers.empty())
    {
        C << nl << "::Ice::StreamReader<" << name << ">::read(istr, *this);";
    }
    C << nl << "istr->endSlice();";
    if (base)
    {
        C << nl << baseClass << "::_readImpl(istr);";
    }
    C << eb;

    H << eb << ';';

    //
    // We need an instance here to trigger initialization if the implementation is in a shared library.
    // But we do this only once per source file, because a single instance is sufficient to initialize
    // all of the globals in a shared library.
    //
    if (!_doneStaticSymbol)
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
Slice::Gen::DataDefVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    _useWstring = setUseWstring(p, _useWstringHist, _useWstring);

    string name = fixKwd(p->name());
    string scope = fixKwd(p->scope());
    string scoped = fixKwd(p->scoped());
    ClassDefPtr base = p->base();
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList allDataMembers = p->allDataMembers();

    H << sp;
    writeDocSummary(H, p);
    H << nl << "class " << _dllClassExport << name << " : public ";

    if (!base)
    {
        H << getUnqualified("::Ice::Value", scope);
    }
    else
    {
        H << getUnqualified(fixKwd(base->scoped()), scope);
    }
    H << sb;
    H.dec();
    H << nl << "public:" << sp;
    H.inc();

    vector<string> params;

    for (const auto& dataMember : dataMembers)
    {
        params.push_back(fixKwd(dataMember->name()));
    }

    H << nl << name << "() = default;";

    emitOneShotConstructor(p);

    H << sp;
    H << nl << "/**";
    H << nl << " * Obtains the Slice type ID of this value.";
    H << nl << " * @return The fully-scoped type ID.";
    H << nl << " */";
    H << nl << _dllMemberExport << "static ::std::string_view ice_staticId() noexcept;";

    H << sp << nl << _dllMemberExport << "::std::string ice_id() const override;";
    C << sp << nl << "::std::string" << nl << scoped.substr(2) << "::ice_id() const";
    C << sb;
    C << nl << "return ::std::string{ice_staticId()};";
    C << eb;

    if (!dataMembers.empty())
    {
        H << sp;
        H << nl << "/**";
        H << nl << " * Obtains a tuple containing all of the value's data members.";
        H << nl << " * @return The data members in a tuple.";
        H << nl << " */";
        writeIceTuple(H, p->allDataMembers(), _useWstring);
    }

    H << sp;
    H << nl << "/**";
    H << nl << " * Creates a shallow polymorphic copy of this instance.";
    H << nl << " * @return The cloned value.";
    H << nl << " */";
    H << nl << p->name() << "Ptr ice_clone() const { return ::std::static_pointer_cast <" << name
      << ">(_iceCloneImpl()); }";

    return true;
}

void
Slice::Gen::DataDefVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    string name = fixKwd(p->name());
    string scoped = fixKwd(p->scoped());
    string scope = fixKwd(p->scope());
    ClassDefPtr base = p->base();

    C << sp;
    C << nl << "::std::string_view" << nl << scoped.substr(2) << "::ice_staticId() noexcept";
    C << sb;
    C << nl << "static constexpr ::std::string_view typeId = \"" << p->scoped() << "\";";
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

    for (const auto& dataMember : dataMembers)
    {
        if (prot || dataMember->hasMetaData("protected"))
        {
            if (!inProtected)
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
            if (inProtected)
            {
                H.dec();
                H << sp << nl << "public:" << sp;
                H.inc();
                inProtected = false;
                needSp = false;
            }
        }

        if (needSp)
        {
            H << sp;
            needSp = false;
        }

        emitDataMember(dataMember);
    }

    if (!inProtected)
    {
        H.dec();
        H << sp << nl << "protected:";
        H.inc();
        inProtected = true;
    }

    if (generateFriend)
    {
        H << sp;
        H << nl << "template<typename T>";
        H << nl << "friend struct Ice::StreamWriter;";
        H << nl << "template<typename T>";
        H << nl << "friend struct Ice::StreamReader;";
    }

    H << sp << nl << name << "(const " << name << "&) = default;";
    H << sp << nl << _dllMemberExport << "::Ice::ValuePtr _iceCloneImpl() const override;";
    C << sp;
    C << nl << "::Ice::ValuePtr" << nl << scoped.substr(2) << "::_iceCloneImpl() const";
    C << sb;
    C << nl << "return CloneEnabler<" << name << ">::clone(*this);";
    C << eb;

    string baseClass = base ? getUnqualified(fixKwd(base->scoped()), scope) : getUnqualified("::Ice::Value", scope);

    H << nl << _dllMemberExport << "void _iceWriteImpl(::Ice::OutputStream*) const override;";
    C << sp << nl << "void" << nl << scoped.substr(2) << "::_iceWriteImpl(::Ice::OutputStream* ostr) const";
    C << sb;
    // lastSlice is true or false.
    C << nl << "ostr->startSlice(ice_staticId(), -1, " << (base ? "false" : "true") << ");";
    if (!dataMembers.empty())
    {
        C << nl << "::Ice::StreamWriter<" << name << ">::write(ostr, *this);";
    }
    C << nl << "ostr->endSlice();";
    if (base)
    {
        C << nl << baseClass << "::_iceWriteImpl(ostr);";
    }
    C << eb;

    H << sp << nl << _dllMemberExport << "void _iceReadImpl(::Ice::InputStream*) override;";
    C << sp << nl << "void" << nl << scoped.substr(2) << "::_iceReadImpl(::Ice::InputStream* istr)";
    C << sb;
    C << nl << "istr->startSlice();";
    if (!dataMembers.empty())
    {
        C << nl << "::Ice::StreamReader<" << name << ">::read(istr, *this);";
    }
    C << nl << "istr->endSlice();";
    if (base)
    {
        C << nl << baseClass << "::_iceReadImpl(istr);";
    }
    C << eb;

    H << eb << ';';

    if (!_doneStaticSymbol)
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
Slice::Gen::DataDefVisitor::emitBaseInitializers(const ClassDefPtr& p)
{
    ClassDefPtr base = p->base();
    if (!base)
    {
        return false;
    }

    DataMemberList allBaseDataMembers = base->allDataMembers();
    if (allBaseDataMembers.empty())
    {
        return false;
    }

    const string scope = fixKwd(p->scope());

    string upcall = "(";
    for (DataMemberList::const_iterator q = allBaseDataMembers.begin(); q != allBaseDataMembers.end(); ++q)
    {
        string memberName = fixKwd((*q)->name());
        TypePtr memberType = (*q)->type();
        if (q != allBaseDataMembers.begin())
        {
            upcall += ", ";
        }
        upcall += condMove(isMovable(memberType), memberName);
    }
    upcall += ")";

    H << nl << getUnqualified(fixKwd(base->scoped()), scope) << upcall;
    return true;
}

void
Slice::Gen::DataDefVisitor::emitOneShotConstructor(const ClassDefPtr& p)
{
    DataMemberList allDataMembers = p->allDataMembers();
    //
    // Use empty scope to get full qualified names in types used with future declarations.
    //
    string scope = "";
    if (!allDataMembers.empty())
    {
        vector<string> allParamDecls;
        map<string, CommentPtr> allComments;
        DataMemberList dataMembers = p->dataMembers();

        for (const auto& dataMember : allDataMembers)
        {
            string typeName =
                typeToString(dataMember->type(), dataMember->optional(), scope, dataMember->getMetaData(), _useWstring);
            allParamDecls.push_back(typeName + " " + fixKwd(dataMember->name()));
            CommentPtr comment = dataMember->parseComment(false);
            if (comment)
            {
                allComments[dataMember->name()] = comment;
            }
        }

        CommentPtr comment = p->parseComment(false);

        H << sp;
        H << nl << "/**";
        H << nl << " * One-shot constructor to initialize all data members.";
        for (const auto& dataMember : allDataMembers)
        {
            map<string, CommentPtr>::iterator r = allComments.find(dataMember->name());
            if (r != allComments.end())
            {
                H << nl << " * @param " << fixKwd(r->first) << " " << getDocSentence(r->second->overview());
            }
        }
        H << nl << " */";
        H << nl;
        if (allParamDecls.size() == 1)
        {
            H << "explicit ";
        }
        H << fixKwd(p->name()) << spar << allParamDecls << epar << " :";
        H.inc();

        if (emitBaseInitializers(p))
        {
            if (!dataMembers.empty())
            {
                H << ',';
            }
        }

        if (!dataMembers.empty())
        {
            H << nl;
        }

        for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            if (q != dataMembers.begin())
            {
                H << ',' << nl;
            }
            string memberName = fixKwd((*q)->name());
            TypePtr memberType = (*q)->type();
            H << memberName << "(" << condMove(isMovable(memberType), memberName) << ')';
        }

        H.dec();
        H << sb;
        H << eb;
    }
}

void
Slice::Gen::DataDefVisitor::emitDataMember(const DataMemberPtr& p)
{
    string name = fixKwd(p->name());
    ContainerPtr container = p->container();
    ClassDefPtr cl = dynamic_pointer_cast<ClassDef>(container);
    //
    // Use empty scope to get full qualified names in types used with future declarations.
    //
    string scope = "";

    writeDocSummary(H, p);
    H << nl << typeToString(p->type(), p->optional(), scope, p->getMetaData(), _useWstring) << ' ' << name;

    string defaultValue = p->defaultValue();
    if (!defaultValue.empty())
    {
        BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(p->type());
        if (p->optional() && builtin && builtin->kind() == Builtin::KindString)
        {
            //
            // = "<string literal>" doesn't work for optional<std::string>
            //
            H << '{';
            writeConstantValue(H, p->type(), p->defaultValueType(), defaultValue, _useWstring, p->getMetaData(), scope);
            H << '}';
        }
        else
        {
            H << " = ";
            writeConstantValue(H, p->type(), p->defaultValueType(), defaultValue, _useWstring, p->getMetaData(), scope);
        }
    }
    H << ";";
}

Slice::Gen::InterfaceVisitor::InterfaceVisitor(
    ::IceUtilInternal::Output& h,
    ::IceUtilInternal::Output& c,
    const string& dllExport)
    : H(h),
      C(c),
      _dllExport(dllExport),
      _useWstring(TypeContext::None)
{
}

bool
Slice::Gen::InterfaceVisitor::visitModuleStart(const ModulePtr& p)
{
    if (!p->hasInterfaceDefs())
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
    if (bases.empty())
    {
        H << "public virtual " << getUnqualified("::Ice::Object", scope);
    }
    else
    {
        InterfaceList::const_iterator q = bases.begin();
        while (q != bases.end())
        {
            string baseScoped = fixKwd((*q)->scope() + (*q)->name());

            H << "public virtual " << getUnqualified(baseScoped, scope);
            if (++q != bases.end())
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
    if (name != "ProxyType")
    {
        H << nl << "using ProxyType = " << p->name() << "Prx;";
    }

    StringList ids = p->ids();

    H << sp;
    H << nl << "/**";
    H << nl << " * Obtains a list of the Slice type IDs representing the interfaces supported by this object.";
    H << nl << " * @param current The Current object for the invocation.";
    H << nl << " * @return A list of fully-scoped type IDs.";
    H << nl << " */";
    H << nl << "::std::vector<::std::string> ice_ids(const " << getUnqualified("::Ice::Current&", scope)
      << " current) const override;";
    H << sp;
    H << nl << "/**";
    H << nl << " * Obtains a Slice type ID representing the most-derived interface supported by this object.";
    H << nl << " * @param current The Current object for the invocation.";
    H << nl << " * @return A fully-scoped type ID.";
    H << nl << " */";
    H << nl << "::std::string ice_id(const " << getUnqualified("::Ice::Current&", scope) << " current) const override;";
    H << sp;
    H << nl << "/**";
    H << nl << " * Obtains the Slice type ID corresponding to this interface.";
    H << nl << " * @return A fully-scoped type ID.";
    H << nl << " */";
    H << nl << "static ::std::string_view ice_staticId() noexcept;";

    C << sp;
    C << nl << "::std::vector<::std::string>" << nl << scoped.substr(2) << "::ice_ids(const "
      << getUnqualified("::Ice::Current&", scope) << ") const";
    C << sb;

    // These type IDs are sorted alphabetically.
    C << nl << "static const ::std::vector<::std::string> allTypeIds = ";
    C.spar('{');
    for (const auto& typeId : p->ids())
    {
        C << '"' + typeId + '"';
    }
    C.epar('}');
    C << ";";

    C << nl << "return allTypeIds;";
    C << eb;

    C << sp;
    C << nl << "::std::string" << nl << scoped.substr(2) << "::ice_id(const "
      << getUnqualified("::Ice::Current&", scope) << ") const";
    C << sb;
    C << nl << "return ::std::string{ice_staticId()};";
    C << eb;

    C << sp;
    C << nl << "::std::string_view" << nl << scoped.substr(2) << "::ice_staticId() noexcept";
    C << sb;
    C << nl << "static constexpr ::std::string_view typeId = \"" << p->scoped() << "\";";
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
    if (!allOps.empty())
    {
        StringList allOpNames;
        transform(
            allOps.begin(),
            allOps.end(),
            back_inserter(allOpNames),
            [](const ContainedPtr& it) { return it->name(); });
        allOpNames.push_back("ice_id");
        allOpNames.push_back("ice_ids");
        allOpNames.push_back("ice_isA");
        allOpNames.push_back("ice_ping");
        allOpNames.sort();
        allOpNames.unique();

        H << sp;
        H << nl << "/// \\cond INTERNAL";
        H << nl
          << "void dispatch(::Ice::IncomingRequest&, ::std::function<void(::Ice::OutgoingResponse)>) "
             "override;";
        H << nl << "/// \\endcond";

        C << sp;
        C << nl << "/// \\cond INTERNAL";
        C << nl << "void";
        C << nl << scoped.substr(2)
          << "::dispatch(::Ice::IncomingRequest& request, ::std::function<void(::Ice::OutgoingResponse)> "
             "sendResponse)";
        C << sb;

        C << sp;
        C << nl << "static constexpr ::std::string_view allOperations[] = ";
        C.spar('{');
        for (const auto& opName : allOpNames)
        {
            C << '"' + opName + '"';
        }
        C.epar('}');
        C << ";";

        C << sp;
        C << nl << "const ::Ice::Current& current = request.current();";
        C << nl << "::std::pair<const ::std::string_view*, const ::std::string_view*> r = "
          << "::std::equal_range(allOperations, allOperations"
          << " + " << allOpNames.size() << ", current.operation);";
        C << nl << "if(r.first == r.second)";
        C << sb;
        C << nl
          << "sendResponse(::Ice::makeOutgoingResponse(::std::make_exception_ptr(::Ice::OperationNotExistException(__"
             "FILE__, __LINE__)), current));";
        C << nl << "return;";
        C << eb;
        C << sp;
        C << nl << "switch(r.first - allOperations)";
        C << sb;
        int i = 0;
        for (StringList::const_iterator q = allOpNames.begin(); q != allOpNames.end(); ++q)
        {
            C << nl << "case " << i++ << ':';
            C << sb;
            C << nl << "_iceD_" << *q << "(request, ::std::move(sendResponse));";
            C << nl << "break;";
            C << eb;
        }
        C << nl << "default:";
        C << sb;
        C << nl << "assert(false);";
        C << nl
          << "sendResponse(::Ice::makeOutgoingResponse(::std::make_exception_ptr(::Ice::OperationNotExistException(__"
             "FILE__, __LINE__)), current));";
        C << eb;
        C << eb;
        C << eb;
        C << nl << "/// \\endcond";
    }

    H << eb << ';';

    H << sp << nl << "using " << p->name() << "Ptr = ::std::shared_ptr<" << name << ">;";

    _useWstring = resetUseWstring(_useWstringHist);
}

void
Slice::Gen::InterfaceVisitor::visitOperation(const OperationPtr& p)
{
    string name = p->name();

    TypePtr ret = p->returnType();

    vector<string> params;
    vector<string> args;

    vector<string> responseParams;
    vector<string> responseParamsDecl;
    vector<string> responseParamsImplDecl;

    InterfaceDefPtr interface = p->interface();
    string interfaceScope = fixKwd(interface->scope());

    string scope = fixKwd(interface->scope() + interface->name() + "::");

    ParamDeclList inParams = p->inParameters();
    ParamDeclList outParams = p->outParameters();
    ParamDeclList paramList = p->parameters();

    const bool amd = (interface->hasMetaData("amd") || p->hasMetaData("amd"));

    const string returnValueParam = escapeParam(outParams, "returnValue");
    const string responsecbParam = escapeParam(inParams, "response");
    const string excbParam = escapeParam(inParams, "exception");
    const string currentParam = escapeParam(amd ? inParams : paramList, "current");
    const string currentTypeDecl = "const " + getUnqualified("::Ice::Current&", interfaceScope);
    const string currentDecl = currentTypeDecl + " " + currentParam;

    CommentPtr comment = p->parseComment(false);

    if (ret)
    {
        string typeS = inputTypeToString(ret, p->returnIsOptional(), interfaceScope, p->getMetaData(), _useWstring);
        responseParams.push_back(typeS + " " + returnValueParam);
        responseParamsDecl.push_back(typeS + " ret");
        responseParamsImplDecl.push_back(typeS + " ret");
    }

    string retS;
    if (amd || !ret)
    {
        retS = "void";
    }
    else if (p->hasMarshaledResult())
    {
        retS = marshaledResultStructName(name);
    }
    else
    {
        retS = typeToString(ret, p->returnIsOptional(), interfaceScope, p->getMetaData(), _useWstring);
    }

    for (const auto& param : paramList)
    {
        TypePtr type = param->type();
        string paramName = fixKwd(param->name());
        bool isOutParam = param->isOutParam();

        if (!isOutParam)
        {
            params.push_back(
                typeToString(
                    type,
                    param->optional(),
                    interfaceScope,
                    param->getMetaData(),
                    _useWstring | TypeContext::UnmarshalParamZeroCopy) +
                " " + paramName);
            args.push_back(condMove(isMovable(type), paramPrefix + param->name()));
        }
        else
        {
            if (!p->hasMarshaledResult() && !amd)
            {
                params.push_back(
                    outputTypeToString(type, param->optional(), interfaceScope, param->getMetaData(), _useWstring) +
                    " " + paramName);
                args.push_back(condMove(isMovable(type) && !isOutParam, paramPrefix + param->name()));
            }

            string responseTypeS =
                inputTypeToString(param->type(), param->optional(), interfaceScope, param->getMetaData(), _useWstring);
            responseParams.push_back(responseTypeS + " " + paramName);
            responseParamsDecl.push_back(responseTypeS + " " + paramPrefix + param->name());
            responseParamsImplDecl.push_back(responseTypeS + " " + paramPrefix + param->name());
        }
    }
    if (amd)
    {
        if (p->hasMarshaledResult())
        {
            string resultName = marshaledResultStructName(name);
            params.push_back("::std::function<void(" + resultName + ")> " + responsecbParam);
            args.push_back(
                "[responseHandler](" + resultName +
                " marshaledResult) { responseHandler->sendResponse(::std::move(marshaledResult)); }");
        }
        else
        {
            params.push_back("::std::function<void(" + joinString(responseParams, ", ") + ")> " + responsecbParam);
            args.push_back(
                ret || !outParams.empty() ? "::std::move(responseCb)"
                                          : "[responseHandler] { responseHandler->sendEmptyResponse(); }");
        }
        params.push_back("::std::function<void(::std::exception_ptr)> " + excbParam);
        args.push_back("[responseHandler](std::exception_ptr ex) { "
                       "responseHandler->sendException(ex); }");
        params.push_back(currentDecl);
        args.push_back("responseHandler->current()");
    }
    else
    {
        params.push_back(currentDecl);
        args.push_back("request.current()");
    }

    if (p->hasMarshaledResult())
    {
        string resultName = marshaledResultStructName(name);
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
        if (ret && comment && !comment->returns().empty())
        {
            H << nl << " * @param " << returnValueParam << " " << getDocSentence(comment->returns());
        }
        map<string, StringList> paramComments;
        if (comment)
        {
            paramComments = comment->parameters();
        }
        const string mrcurrent = escapeParam(outParams, "current");
        for (const auto& param : outParams)
        {
            map<string, StringList>::iterator r = paramComments.find(param->name());
            if (r != paramComments.end())
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
        // Switch to ostr pointer expected by WriteMarshalCode.
        C << nl << "::Ice::OutputStream* ostr = &_ostr;";
        C << nl << "ostr->startEncapsulation(current.encoding, " << opFormatTypeToString(p) << ");";
        writeMarshalCode(C, outParams, p);
        if (p->returnsClasses(false))
        {
            C << nl << "ostr->writePendingValues();";
        }
        C << nl << "ostr->endEncapsulation();";
        C << eb;
    }

    string isConst = p->hasMetaData("cpp:const") ? " const" : "";

    string opName = amd ? (name + "Async") : fixKwd(name);
    string deprecateSymbol = getDeprecateSymbol(p);

    H << sp;
    if (comment)
    {
        OpDocParamType pt = (amd || p->hasMarshaledResult()) ? OpDocInParams : OpDocAllParams;
        StringList postParams, returns;
        if (amd)
        {
            postParams.push_back("@param " + responsecbParam + " The response callback.");
            postParams.push_back("@param " + excbParam + " The exception callback.");
        }
        else if (p->hasMarshaledResult())
        {
            returns.push_back("The marshaled result structure.");
        }
        else
        {
            returns = comment->returns();
        }
        postParams.push_back("@param " + currentParam + " The Current object for the invocation.");
        writeOpDocSummary(H, p, comment, pt, true, StringList(), postParams, returns);
    }
    H << nl << deprecateSymbol << "virtual " << retS << ' ' << opName << spar << params << epar << isConst << " = 0;";
    H << nl << "/// \\cond INTERNAL";
    H << nl << "void _iceD_" << name << "(::Ice::IncomingRequest&, ::std::function<void(::Ice::OutgoingResponse)>)"
      << isConst << ';';
    H << nl << "/// \\endcond";

    C << sp;
    C << nl << "/// \\cond INTERNAL";
    C << nl << "void";
    C << nl << scope.substr(2);
    C << "_iceD_" << name
      << "(::Ice::IncomingRequest& request, ::std::function<void(::Ice::OutgoingResponse)> sendResponse)" << isConst;

    C << sb;
    C << nl << "_iceCheckMode(" << getUnqualified(operationModeToString(p->mode()), interfaceScope)
      << ", request.current().mode);";

    if (!inParams.empty())
    {
        C << nl << "auto istr = &request.inputStream();";
        C << nl << "istr->startEncapsulation();";
        writeAllocateCode(C, inParams, nullptr, interfaceScope, _useWstring | TypeContext::UnmarshalParamZeroCopy);
        writeUnmarshalCode(C, inParams, nullptr);
        if (p->sendsClasses(false))
        {
            C << nl << "istr->readPendingValues();";
        }
        C << nl << "istr->endEncapsulation();";
    }
    else
    {
        C << nl << "request.inputStream().skipEmptyEncapsulation();";
    }

    if (!amd)
    {
        if (p->hasMarshaledResult())
        {
            C << nl << "sendResponse(::Ice::OutgoingResponse{";
        }
        else
        {
            writeAllocateCode(C, outParams, nullptr, interfaceScope, _useWstring);
            if (ret)
            {
                C << nl << retS << " ret = ";
            }
            else
            {
                C << nl;
            }
        }

        C << "this->" << opName << spar << args << epar;
        if (p->hasMarshaledResult())
        {
            C << ".outputStream(), request.current()});";
        }
        else
        {
            C << ";";
            if (ret || !outParams.empty())
            {
                C << nl << "sendResponse(::Ice::makeOutgoingResponse([&](::Ice::OutputStream* ostr)";
                C.inc();
                C << sb;
                writeMarshalCode(C, outParams, p);
                if (p->returnsClasses(false))
                {
                    C << nl << "ostr->writePendingValues();";
                }
                C << eb << ",";
                C << nl << "request.current()";
                if (p->format() != DefaultFormat)
                {
                    C << ",";
                    C << nl << opFormatTypeToString(p);
                }
                C << "));";
                C.dec();
            }
            else
            {
                C << nl << "sendResponse(::Ice::makeEmptyOutgoingResponse(request.current()));";
            }
        }
    }
    else
    {
        C << nl
          << "auto responseHandler = "
             "::std::make_shared<::IceInternal::AsyncResponseHandler>(::std::move(sendResponse), request.current());";
        if (!p->hasMarshaledResult() && (ret || !outParams.empty()))
        {
            C << nl << "auto responseCb = [responseHandler]" << spar << responseParamsDecl << epar;
            C << sb;
            C << nl << "responseHandler->sendResponse(";
            C.inc();
            C << nl << "[&](::Ice::OutputStream* ostr)";
            C << sb;
            writeMarshalCode(C, outParams, p);
            if (p->returnsClasses(false))
            {
                C << nl << "ostr->writePendingValues();";
            }
            C << eb;
            if (p->format() != DefaultFormat)
            {
                C << ",";
                C << nl << opFormatTypeToString(p);
            }
            C << ");";
            C.dec();
            C << eb << ';';
        }
        C << nl << "try";
        C << sb;
        C << nl << "this->" << opName << spar << args << epar << ';';
        C << eb;
        C << nl << "catch (...)";
        C << sb;
        C << nl << "responseHandler->sendException(::std::current_exception());";
        C << eb;
    }
    C << eb;
    C << nl << "/// \\endcond";
}

Slice::Gen::StreamVisitor::StreamVisitor(Output& h) : H(h) {}

bool
Slice::Gen::StreamVisitor::visitModuleStart(const ModulePtr& m)
{
    if (!m->hasContained(Contained::ContainedTypeStruct) && !m->hasContained(Contained::ContainedTypeEnum) &&
        !m->hasContained(Contained::ContainedTypeException) && !m->hasContained(Contained::ContainedTypeClass))
    {
        return false;
    }

    if (dynamic_pointer_cast<Unit>(m->container()))
    {
        //
        // Only emit this for the top-level module.
        //
        H << sp;
        H << nl << "/// \\cond STREAM";
        H << nl << "namespace Ice" << nl << '{' << sp;
    }
    return true;
}

void
Slice::Gen::StreamVisitor::visitModuleEnd(const ModulePtr& m)
{
    if (dynamic_pointer_cast<Unit>(m->container()))
    {
        //
        // Only emit this for the top-level module.
        //
        H << nl << '}';
        H << nl << "/// \\endcond";
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

    writeStreamHelpers(H, p, p->dataMembers(), false);

    return false;
}

bool
Slice::Gen::StreamVisitor::visitClassDefStart(const ClassDefPtr& c)
{
    writeStreamHelpers(H, c, c->dataMembers(), c->hasBaseDataMembers());
    return false;
}

bool
Slice::Gen::StreamVisitor::visitExceptionStart(const ExceptionPtr&)
{
    return true;
}

void
Slice::Gen::StreamVisitor::visitExceptionEnd(const ExceptionPtr& p)
{
    writeStreamHelpers(H, p, p->dataMembers(), p->hasBaseDataMembers());
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
