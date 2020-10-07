//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/Functional.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/InputUtil.h>
#include <Gen.h>
#include <IceUtil/Iterator.h>
#include <IceUtil/UUID.h>
#include <Slice/FileTracker.h>
#include <Slice/Util.h>

// TODO: fix this warning!
#if defined(_MSC_VER)
#   pragma warning(disable:4456) // shadow
#   pragma warning(disable:4457) // shadow
#   pragma warning(disable:4459) // shadow
#elif defined(__clang__)
#   pragma clang diagnostic ignored "-Wshadow"
#elif defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wshadow"
#endif

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

namespace
{

string
sliceModeToIceMode(Operation::Mode opMode)
{
    switch(opMode)
    {
    case Operation::Normal:
        return "0";
    case Operation::Nonmutating:
        return "1";
    case Operation::Idempotent:
        return "2";
    default:
        assert(false);
    }

    return "???";
}

string
opFormatTypeToString(const OperationPtr& op)
{
    switch(op->format())
    {
    case DefaultFormat:
        return "0";
    case CompactFormat:
        return "1";
    case SlicedFormat:
        return "2";
    default:
        assert(false);
    }

    return "???";
}

void
printHeader(IceUtilInternal::Output& out)
{
    static const char* header =
        "//\n"
        "// Copyright (c) ZeroC, Inc. All rights reserved.\n"
        "//\n"
        ;

    out << header;
    out << "//\n";
    out << "// Ice version " << ICE_STRING_VERSION << "\n";
    out << "//\n";
}

string
escapeParam(const MemberList& params, const string& name)
{
    for (const auto& param : params)
    {
        if(Slice::JsGenerator::fixId(param->name()) == name)
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

    out << nl << " */";
}

enum OpDocParamType { OpDocInParams, OpDocOutParams, OpDocAllParams };

void
writeOpDocParams(Output& out, const OperationPtr& op, const CommentPtr& doc, OpDocParamType type,
                 const StringList& preParams = StringList(), const StringList& postParams = StringList())
{
    MemberList params;
    switch (type)
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

    if (!preParams.empty())
    {
        writeDocLines(out, preParams, true);
    }

    map<string, StringList> paramDoc = doc->params();
    for (auto p = params.begin(); p != params.end(); ++p)
    {
        map<string, StringList>::iterator q = paramDoc.find((*p)->name());
        if(q != paramDoc.end())
        {
            out << nl << " * @param " << Slice::JsGenerator::fixId(q->first) << " ";
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
    for (map<string, StringList>::iterator p = exDoc.begin(); p != exDoc.end(); ++p)
    {
        //
        // Try to locate the exception's definition using the name given in the comment.
        //
        string name = p->first;
        ExceptionPtr ex = op->container()->lookupException(name, false);
        if (ex)
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

    if (!doc->overview().empty())
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

}

Slice::JsVisitor::JsVisitor(Output& out, const vector<pair<string, string> >& imports) :
    _out(out),
    _imports(imports)
{
}

Slice::JsVisitor::~JsVisitor()
{
}

vector<pair<string, string> >
Slice::JsVisitor::imports() const
{
    return _imports;
}

void
Slice::JsVisitor::writeMarshalDataMembers(const MemberList& dataMembers)
{
    const auto [required, tagged] = getSortedMembers(dataMembers);
    for (const auto& member : required)
    {
        writeMarshalUnmarshalCode(_out, member->type(), "this." + fixId(member->name()), true);
    }
    for (const auto& member : tagged)
    {
        writeTaggedMarshalUnmarshalCode(_out, member->type(), "this." + fixId(member->name()), member->tag(), true);
    }
}

void
Slice::JsVisitor::writeUnmarshalDataMembers(const MemberList& dataMembers)
{
    const auto [required, tagged] = getSortedMembers(dataMembers);
    for (const auto& member : required)
    {
        writeMarshalUnmarshalCode(_out, member->type(), "this." + fixId(member->name()), false);
    }
    for (const auto& member : tagged)
    {
        writeTaggedMarshalUnmarshalCode(_out, member->type(), "this." + fixId(member->name()), member->tag(), false);
    }
}

void
Slice::JsVisitor::writeInitDataMembers(const MemberList& dataMembers)
{
    for (const auto& member : dataMembers)
    {
        const string m = fixId(member->name());
        _out << nl << "this." << m << " = " << m << ';';
    }
}

string
Slice::JsVisitor::getValue(const string& /*scope*/, const TypePtr& type)
{
    assert(type);

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindBool:
            {
                return "false";
            }
            // TODO change these whole-number types to use BigInt
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindUShort:
            case Builtin::KindInt:
            case Builtin::KindUInt:
            case Builtin::KindVarInt:
            case Builtin::KindVarUInt:
            {
                return "0";
            }
            case Builtin::KindLong:
            case Builtin::KindULong:
            case Builtin::KindVarLong:
            case Builtin::KindVarULong:
            {
                return "new Ice.Long(0, 0)";
            }
            case Builtin::KindFloat:
            case Builtin::KindDouble:
            {
                return "0.0";
            }
            case Builtin::KindString:
            {
                return "\"\"";
            }
            default:
            {
                return "null";
            }
        }
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        return fixId(en->scoped()) + '.' + fixId((*en->enumerators().begin())->name());
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        return "new " + typeToString(type) + "()";
    }

    return "null";
}

string
Slice::JsVisitor::writeConstantValue(const string& /*scope*/, const TypePtr& constType, const SyntaxTreeBasePtr& valueType,
                                     const string& value)
{
    TypePtr type = unwrapIfOptional(constType);

    ostringstream os;
    ConstPtr constant = ConstPtr::dynamicCast(valueType);
    if(constant)
    {
        os << fixId(constant->scoped());
    }
    else
    {
        BuiltinPtr bp = BuiltinPtr::dynamicCast(type);
        EnumPtr ep;
        if(bp && bp->kind() == Builtin::KindString)
        {
            //
            // For now, we generate strings in ECMAScript 5 format, with two \unnnn for astral characters
            //
            os << "\"" << toStringLiteral(value, "\b\f\n\r\t\v", "", ShortUCN, 0) << "\"";
        }
        // TODO eliminate this special Ice.Long logic in a future PR
        else if(bp && (bp->kind() == Builtin::KindLong || bp->kind() == Builtin::KindULong ||
                bp->kind() == Builtin::KindVarLong || bp->kind() == Builtin::KindVarULong))
        {
            IceUtil::Int64 l = IceUtilInternal::strToInt64(value.c_str(), 0, 0);

            //
            // JavaScript doesn't support 64 bit integer so long types are written as
            // two 32 bit words hi, low wrapped in the Ice.Long class.
            //
            // If slice2js runs in a big endian machine we need to swap the words, we do not
            // need to swap the word bytes as we just write each word as a number to the
            // output file.
            //
#ifdef ICE_BIG_ENDIAN
            os << "new Ice.Long(" << (l & 0xFFFFFFFF) << ", " << ((l >> 32) & 0xFFFFFFFF)  << ")";
#else
            os << "new Ice.Long(" << ((l >> 32) & 0xFFFFFFFF) << ", " << (l & 0xFFFFFFFF)  << ")";
#endif
        }
        else if((ep = EnumPtr::dynamicCast(type)))
        {
            EnumeratorPtr lte = EnumeratorPtr::dynamicCast(valueType);
            assert(lte);
            os << fixId(ep->scoped()) << '.' << fixId(lte->name());
        }
        else
        {
            os << value;
        }
    }
    return os.str();
}

StringList
Slice::JsVisitor::splitComment(const ContainedPtr& p)
{
    StringList result;

    string comment = p->comment();
    string::size_type pos = 0;
    string::size_type nextPos;
    while((nextPos = comment.find_first_of('\n', pos)) != string::npos)
    {
        result.push_back(string(comment, pos, nextPos - pos));
        pos = nextPos + 1;
    }
    string lastLine = string(comment, pos);
    if(lastLine.find_first_not_of(" \t\n\r") != string::npos)
    {
        result.push_back(lastLine);
    }

    return result;
}

void
Slice::JsVisitor::writeDocComment(const ContainedPtr& p, const string& deprecateReason, const string& extraParam)
{
    StringList lines = splitComment(p);
    if(lines.empty())
    {
        if(!deprecateReason.empty())
        {
            _out << nl << "/**";
            _out << nl << " * @deprecated " << deprecateReason;
            _out << nl << " **/";
        }
        return;
    }

    _out << nl << "/**";

    bool doneExtraParam = false;
    for(StringList::const_iterator i = lines.begin(); i != lines.end(); ++i)
    {
        //
        // @param must precede @return, so emit any extra parameter
        // when @return is seen.
        //
        if(i->find("@return") != string::npos && !extraParam.empty())
        {
            _out << nl << " * " << extraParam;
            doneExtraParam = true;
        }
        if((*i).empty())
        {
            _out << nl << " *";
        }
        else
        {
            _out << nl << " * " << *i;
        }
    }

    if(!doneExtraParam && !extraParam.empty())
    {
        //
        // Above code doesn't emit the comment for the extra parameter
        // if the operation returns a void or doesn't have an @return.
        //
        _out << nl << " * " << extraParam;
    }

    if(!deprecateReason.empty())
    {
        _out << nl << " * @deprecated " << deprecateReason;
    }

    _out << nl << " **/";
}

Slice::Gen::Gen(const string& base, const vector<string>& includePaths, const string& dir, bool typeScript) :
    _includePaths(includePaths),
    _useStdout(false),
    _typeScript(typeScript)
{
    _fileBase = base;

    string::size_type pos = base.find_last_of("/\\");
    if(pos != string::npos)
    {
        _fileBase = base.substr(pos + 1);
    }

    string file = _fileBase + ".js";

    if(!dir.empty())
    {
        file = dir + '/' + file;
    }

    _jsout.open(file.c_str());
    if(!_jsout)
    {
        ostringstream os;
        os << "cannot open `" << file << "': " << IceUtilInternal::errorToString(errno);
        throw FileException(__FILE__, __LINE__, os.str());
    }
    FileTracker::instance()->addFile(file);

    if(typeScript)
    {
        file = _fileBase + ".d.ts";

        if(!dir.empty())
        {
            file = dir + '/' + file;
        }

        _tsout.open(file.c_str());
        if(!_tsout)
        {
            ostringstream os;
            os << "cannot open `" << file << "': " << IceUtilInternal::errorToString(errno);
            throw FileException(__FILE__, __LINE__, os.str());
        }
        FileTracker::instance()->addFile(file);
    }
}

Slice::Gen::Gen(const string& base, const vector<string>& includePaths, const string& /*dir*/, bool typeScript,
                ostream& out) :
    _jsout(out),
    _tsout(out),
    _includePaths(includePaths),
    _useStdout(true),
    _typeScript(typeScript)
{
    _fileBase = base;
    string::size_type pos = base.find_last_of("/\\");
    if(pos != string::npos)
    {
        _fileBase = base.substr(pos + 1);
    }
}

Slice::Gen::~Gen()
{
    if(_jsout.isOpen() || _useStdout)
    {
        _jsout << '\n';
    }

    if(_tsout.isOpen() || _useStdout)
    {
        _tsout << '\n';
    }
}

void
Slice::Gen::generate(const UnitPtr& p)
{
    string module;
    DefinitionContextPtr dc = p->findDefinitionContext(p->topLevelFile());
    assert(dc);

    {
        const string prefix = "js:module:";
        const string m = dc->findMetadata(prefix);
        if(!m.empty())
        {
            module = m.substr(prefix.size());
        }
    }

    if(_useStdout)
    {
        _jsout << "\n";
        _jsout << "/** slice2js: " << _fileBase << ".js generated begin module:\"" << module << "\" **/";
        _jsout << "\n";
    }
    printHeader(_jsout);
    printGeneratedHeader(_jsout, _fileBase + ".ice");

    //
    // Check for global "js:module:ice" metadata. If this is set then we are building Ice.
    //
    bool icejs = module == "ice";

    //
    // Check for global "js:es6-module" metadata. If this is set we are using es6 module mapping
    //
    bool es6module = dc->findMetadata("js:es6-module") == "js:es6-module";

    _jsout << nl << "/* eslint-disable */";
    _jsout << nl << "/* jshint ignore: start */";
    _jsout << nl;

    if(!es6module)
    {
        if(icejs)
        {
            _jsout.zeroIndent();
            _jsout << nl << "/* slice2js browser-bundle-skip */";
            _jsout.restoreIndent();
        }
        _jsout << nl << "(function(module, require, exports)";
        _jsout << sb;
        if(icejs)
        {
            _jsout.zeroIndent();
            _jsout << nl << "/* slice2js browser-bundle-skip-end */";
            _jsout.restoreIndent();
        }
    }
    RequireVisitor requireVisitor(_jsout, _includePaths, icejs, es6module);
    p->visit(&requireVisitor, false);
    vector<string> seenModules = requireVisitor.writeRequires(p);

    TypesVisitor typesVisitor(_jsout, seenModules, icejs);
    p->visit(&typesVisitor, false);

    //
    // Export the top-level modules.
    //
    ExportVisitor exportVisitor(_jsout, icejs, es6module);
    p->visit(&exportVisitor, false);

    if(!es6module)
    {
        if(icejs)
        {
            _jsout.zeroIndent();
            _jsout << nl << "/* slice2js browser-bundle-skip */";
            _jsout.restoreIndent();
        }

        _jsout << eb;
        _jsout << nl << "(typeof(global) !== \"undefined\" && typeof(global.process) !== \"undefined\" ? module : undefined,"
               << nl << " typeof(global) !== \"undefined\" && typeof(global.process) !== \"undefined\" ? require :"
               << nl << " (typeof WorkerGlobalScope !== \"undefined\" && self instanceof WorkerGlobalScope) ? self.Ice._require : window.Ice._require,"
               << nl << " typeof(global) !== \"undefined\" && typeof(global.process) !== \"undefined\" ? exports :"
               << nl << " (typeof WorkerGlobalScope !== \"undefined\" && self instanceof WorkerGlobalScope) ? self : window));";

        if(icejs)
        {
            _jsout.zeroIndent();
            _jsout << nl << "/* slice2js browser-bundle-skip-end */";
            _jsout.restoreIndent();
        }
    }

    if(_useStdout)
    {
        _jsout << "\n";
        _jsout << "/** slice2js: generated end **/";
        _jsout << "\n";
    }

    if(_typeScript)
    {
        if(_useStdout)
        {
            _tsout << "\n";
            _tsout << "/** slice2js: " << _fileBase << ".d.ts generated begin module:\"" << module << "\" **/";
            _tsout << "\n";
        }
        printHeader(_tsout);
        printGeneratedHeader(_tsout, _fileBase + ".ice");

        TypeScriptRequireVisitor requireVisitor(_tsout, icejs);
        p->visit(&requireVisitor, false);

        //
        // If at some point TypeScript adds an operator to refer to a type in the global scope
        // we can get rid of the TypeScriptAliasVisitor and use this. For now we need to generate
        // a type alias when there is an abiguity.
        // see: https://github.com/Microsoft/TypeScript/issues/983
        //
        TypeScriptAliasVisitor aliasVisitor(_tsout);
        p->visit(&aliasVisitor, false);
        aliasVisitor.writeAlias(p);

        TypeScriptVisitor typeScriptVisitor(_tsout, requireVisitor.imports());
        p->visit(&typeScriptVisitor, false);

        if(_useStdout)
        {
            _tsout << "\n";
            _tsout << "/** slice2js: generated end **/";
            _tsout << "\n";
        }
    }
}

void
Slice::Gen::closeOutput()
{
    if(_jsout.isOpen())
    {
        _jsout.close();
    }

    if(_tsout.isOpen())
    {
        _tsout.close();
    }
}

Slice::Gen::RequireVisitor::RequireVisitor(IceUtilInternal::Output& out, vector<string> includePaths,
                                           bool icejs, bool es6modules) :
    JsVisitor(out),
    _icejs(icejs),
    _es6modules(es6modules),
    _seenClass(false),
    _seenCompactId(false),
    _seenInterface(false),
    _seenOperation(false),
    _seenStruct(false),
    _seenUserException(false),
    _seenEnum(false),
    _seenObjectSeq(false),
    _seenValueSeq(false),
    _seenObjectDict(false),
    _seenValueDict(false),
    _includePaths(includePaths)
{
    for(vector<string>::iterator p = _includePaths.begin(); p != _includePaths.end(); ++p)
    {
        *p = fullPath(*p);
    }
}

bool
Slice::Gen::RequireVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    _seenClass = true;
    if(p->compactId() >= 0)
    {
        _seenCompactId = true;
    }
    return true;
}

bool
Slice::Gen::RequireVisitor::visitInterfaceDefStart(const InterfaceDefPtr&)
{
    _seenInterface = true;
    return true;
}

bool
Slice::Gen::RequireVisitor::visitStructStart(const StructPtr&)
{
    _seenStruct = true;
    return false;
}

void
Slice::Gen::RequireVisitor::visitOperation(const OperationPtr&)
{
    _seenOperation = true;
}

bool
Slice::Gen::RequireVisitor::visitExceptionStart(const ExceptionPtr&)
{
    _seenUserException = true;
    return false;
}

void
Slice::Gen::RequireVisitor::visitSequence(const SequencePtr& seq)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(unwrapIfOptional(seq->type()));
    if(builtin)
    {
        switch(builtin->kind())
        {
        case Builtin::KindObject:
            _seenObjectSeq = true;
            break;
        case Builtin::KindAnyClass:
            _seenValueSeq = true;
            break;
        default:
            break;
        }
    }
}

void
Slice::Gen::RequireVisitor::visitDictionary(const DictionaryPtr& dict)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(unwrapIfOptional(dict->valueType()));
    if(builtin)
    {
        switch(builtin->kind())
        {
        case Builtin::KindObject:
            _seenObjectDict = true;
            break;
        case Builtin::KindAnyClass:
            _seenValueDict = true;
            break;
        default:
            break;
        }
    }
}

void
Slice::Gen::RequireVisitor::visitEnum(const EnumPtr&)
{
    _seenEnum = true;
}

namespace
{

bool iceBuiltinModule(const string& name)
{
    return name == "Glacier2" || name == "Ice" || name == "IceGrid" || name == "IceMX" || name == "IceStorm";
}

}

vector<string>
Slice::Gen::RequireVisitor::writeRequires(const UnitPtr& p)
{
    vector<string> seenModules;
    map<string, list<string> > requires;
    if(_icejs)
    {
        requires["Ice"] = list<string>();

        //
        // Generate require() statements for all of the run-time code needed by the generated code.
        //
        if(_seenClass || _seenValueSeq || _seenValueDict)
        {
            requires["Ice"].push_back("Ice/Value");
        }
        if(_seenInterface)
        {
            requires["Ice"].push_back("Ice/Object");
            requires["Ice"].push_back("Ice/ObjectPrx");
        }
        if(_seenOperation)
        {
            requires["Ice"].push_back("Ice/Operation");
        }
        if(_seenStruct)
        {
            requires["Ice"].push_back("Ice/Struct");
        }

        if(_seenUserException)
        {
            requires["Ice"].push_back("Ice/Exception");
        }

        if(_seenEnum)
        {
            requires["Ice"].push_back("Ice/EnumBase");
        }

        if(_seenCompactId)
        {
            requires["Ice"].push_back("Ice/CompactIdRegistry");
        }

        requires["Ice"].push_back("Ice/Long");
        requires["Ice"].push_back("Ice/HashMap");
        requires["Ice"].push_back("Ice/HashUtil");
        requires["Ice"].push_back("Ice/ArrayUtil");
        requires["Ice"].push_back("Ice/StreamHelpers");
    }
    else
    {
        requires["Ice"] = list<string>();
        requires["Ice"].push_back("ice");
    }

    StringList includes = p->includeFiles();
    if(_es6modules)
    {
        const string prefix = "js:module:";
        DefinitionContextPtr dc = p->findDefinitionContext(p->topLevelFile());
        string m1 = dc->findMetadata(prefix);
        if(!m1.empty())
        {
            m1 = m1.substr(prefix.size());
        }

        seenModules.push_back("Ice");

        map<string, set<string> > imports;
        set<string> mImports;
        {
            mImports.insert("Ice");
            imports["ice"] = mImports;
        }

        for(StringList::const_iterator i = includes.begin(); i != includes.end(); ++i)
        {
            set<string> modules = p->getTopLevelModules(*i);

            dc = p->findDefinitionContext(*i);

            string m2 = dc->findMetadata(prefix);
            if(!m2.empty())
            {
                m2 = m2.substr(prefix.size());
            }

            if(m1 != m2 && !m2.empty())
            {
                for(set<string>::const_iterator j = modules.begin(); j != modules.end(); ++j)
                {
                    if(imports.find(m2) == imports.end())
                    {
                        set<string> mImports;
                        mImports.insert(*j);
                        imports[m2] = mImports;
                    }
                    else
                    {
                        imports[m2].insert(*j);
                    }
                }
            }
            else
            {
                set<string> newModules;
                for(set<string>::const_iterator j = modules.begin(); j != modules.end(); ++j)
                {
                    if(find(seenModules.begin(), seenModules.end(), *j) == seenModules.end())
                    {
                        seenModules.push_back(*j);
                        newModules.insert(*j);
                    }
                }

                string f = relativePath(*i, p->topLevelFile());
                string::size_type pos;
                if((pos = f.rfind('.')) != string::npos)
                {
                    f.erase(pos);
                }

                imports[f] = newModules;
            }
        }

        //
        // We first import the Ice runtime
        //
        _out << nl << "import { ";
        mImports = imports["ice"];
        for(set<string>::const_iterator i = mImports.begin(); i != mImports.end();)
        {
            _out << (*i);
            if(++i != mImports.end())
            {
                _out << ", ";
            }
        }
        _out << " } from \"ice\";";

        _out << nl << "const _ModuleRegistry = Ice._ModuleRegistry;";

        for(map<string, set<string> >::const_iterator i = imports.begin(); i != imports.end(); ++i)
        {
            if(i->first != "ice")
            {
                mImports = i->second;
                _out << nl << "import ";
                if(!mImports.empty())
                {
                    _out << "{ ";
                    for(set<string>::const_iterator i = mImports.begin(); i != mImports.end();)
                    {
                        _out << (*i);
                        if(++i != mImports.end())
                        {
                            _out << ", ";
                        }
                    }
                    _out << " } from ";
                }
                _out << "\"" << i->first << "\";";
            }
        }
        _out << nl << "const Slice = Ice.Slice;";
    }
    else
    {
        for(StringList::const_iterator i = includes.begin(); i != includes.end(); ++i)
        {
            set<string> modules = p->getTopLevelModules(*i);
            for(set<string>::const_iterator j = modules.begin(); j != modules.end(); ++j)
            {
                if(!_icejs && iceBuiltinModule(*j))
                {
                    if(requires.find(*j) == requires.end())
                    {
                        requires[*j] = list<string>();
                        requires[*j].push_back("ice");
                    }
                }
                else
                {
                    if(requires.find(*j) == requires.end())
                    {
                        requires[*j] = list<string>();
                    }
                    requires[*j].push_back(changeInclude(*i, _includePaths));
                }
            }
        }

        if(_icejs)
        {
            _out.zeroIndent();
            _out << nl << "/* slice2js browser-bundle-skip */";
            _out.restoreIndent();
        }

        if(!_icejs)
        {
            _out << nl << "const Ice = require(\"ice\").Ice;";
            _out << nl << "const _ModuleRegistry = Ice._ModuleRegistry;";
        }
        else
        {
            _out << nl << "const _ModuleRegistry = require(\"../Ice/ModuleRegistry\").Ice._ModuleRegistry;";
        }

        for(map<string, list<string> >::const_iterator i = requires.begin(); i != requires.end(); ++i)
        {
            if(!_icejs && i->first == "Ice")
            {
                continue;
            }

            if(i->second.size() == 1)
            {
                _out << nl << "const " << i->first << " = require(\"";
                if(_icejs && iceBuiltinModule(i->first))
                {
                    _out << "../";
                }
                _out << i->second.front() << "\")." << i->first << ";";
            }
            else
            {
                _out << nl << "const " << i->first << " = _ModuleRegistry.require(module,";
                _out << nl << "[";
                _out.inc();
                for(list<string>::const_iterator j = i->second.begin(); j != i->second.end();)
                {
                    _out << nl << '"';
                    if(_icejs && iceBuiltinModule(i->first))
                    {
                        _out << "../";
                    }
                    _out << *j << '"';
                    if(++j != i->second.end())
                    {
                        _out << ",";
                    }
                }
                _out.dec();
                _out << nl << "])." << i->first << ";";
                _out << sp;
            }
            seenModules.push_back(i->first);
        }

        _out << nl << "const Slice = Ice.Slice;";

        if(_icejs)
        {
            _out.zeroIndent();
            _out << nl << "/* slice2js browser-bundle-skip-end */";
            _out.restoreIndent();
        }
    }
    return seenModules;
}

Slice::Gen::TypesVisitor::TypesVisitor(IceUtilInternal::Output& out, vector<string> seenModules, bool icejs) :
    JsVisitor(out),
    _seenModules(seenModules),
    _icejs(icejs)
{
}

bool
Slice::Gen::TypesVisitor::visitModuleStart(const ModulePtr& p)
{
    //
    // For a top-level module we write the following:
    //
    // let Foo = _ModuleRegistry.module("Foo");
    //
    // For a nested module we write
    //
    // Foo.Bar = _ModuleRegistry.module("Foo.Bar");
    //
    const string scoped = getLocalScope(p->scoped());
    vector<string>::const_iterator i = find(_seenModules.begin(), _seenModules.end(), scoped);
    if(i == _seenModules.end())
    {
        if(_icejs)
        {
            _out.zeroIndent();
            _out << nl << "/* slice2js browser-bundle-skip */";
            _out.restoreIndent();
        }

        _seenModules.push_back(scoped);
        const bool topLevel = UnitPtr::dynamicCast(p->container());
        _out << sp;
        _out << nl;
        if(topLevel)
        {
            _out << "let ";
        }
        _out << scoped << " = _ModuleRegistry.module(\"" << scoped << "\");";

        if(_icejs)
        {
            _out.zeroIndent();
            _out << nl << "/* slice2js browser-bundle-skip-end */";
            _out.restoreIndent();
        }
    }
    return true;
}

void
Slice::Gen::TypesVisitor::visitModuleEnd(const ModulePtr&)
{
}

bool
Slice::Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    const string scope = p->scope();
    const string scoped = p->scoped();
    const string localScope = getLocalScope(scope);
    const string name = fixId(p->name());

    ClassDefPtr base = p->base();
    string baseRef = base ? fixId(base->scoped()) : "Ice.Value";

    const MemberList allDataMembers = p->allDataMembers();
    const MemberList dataMembers = p->dataMembers();

    vector<string> allParamNames;
    for (const auto& member : allDataMembers)
    {
        allParamNames.push_back(fixId(member->name()));
    }

    vector<string> baseParamNames;
    MemberList baseDataMembers;
    if(base)
    {
        baseDataMembers = base->allDataMembers();
        for (const auto& member : baseDataMembers)
        {
            baseParamNames.push_back(fixId(member->name()));
        }
    }

    StringList ids = p->ids();
    StringList::const_iterator firstIter = ids.begin();
    StringList::const_iterator scopedIter = find(ids.begin(), ids.end(), scoped);
    assert(scopedIter != ids.end());
    StringList::difference_type scopedPos = IceUtilInternal::distance(firstIter, scopedIter);

    _out << sp;
    _out << nl << "const iceC_" << getLocalScope(scoped, "_") << "_ids = [";
    _out.inc();

    for(StringList::const_iterator q = ids.begin(); q != ids.end(); ++q)
    {
        if(q != ids.begin())
        {
            _out << ',';
        }
        _out << nl << '"' << *q << '"';
    }

    _out.dec();
    _out << nl << "];";

    _out << sp;
    writeDocComment(p, getDeprecateReason(p));
    _out << nl << localScope << '.' << name << " = class";
    _out << " extends " << baseRef;

    _out << sb;
    if (!allParamNames.empty())
    {
        _out << nl << "constructor" << spar;
        for (const auto& member : baseDataMembers)
        {
            _out << fixId(member->name());
        }

        for (const auto& member : dataMembers)
        {
            string value;
            if (member->defaultValueType())
            {
                value = writeConstantValue(scope, member->type(), member->defaultValueType(), member->defaultValue());
            }
            else if (member->tagged())
            {
                value = "undefined";
            }
            else
            {
                value = getValue(scope, member->type());
            }
            _out << (fixId(member->name()) + (value.empty() ? value : (" = " + value)));
        }

        _out << epar << sb;
        _out << nl << "super" << spar << baseParamNames << epar << ';';

        writeInitDataMembers(dataMembers);
        _out << eb;

        if (p->compactId() != -1)
        {
            _out << sp;
            _out << nl << "static get _iceCompactId()";
            _out << sb;
            _out << nl << "return " << p->compactId() << ";";
            _out << eb;
        }

        if (!dataMembers.empty())
        {
            _out << sp;
            _out << nl << "_iceWriteMemberImpl(ostr)";
            _out << sb;
            writeMarshalDataMembers(dataMembers);
            _out << eb;

            _out << sp;
            _out << nl << "_iceReadMemberImpl(istr)";
            _out << sb;
            writeUnmarshalDataMembers(dataMembers);
            _out << eb;
        }
    }
    _out << eb << ";";

    _out << sp;

    bool preserved = p->hasMetadata("preserve-slice") && !p->inheritsMetadata("preserve-slice");

    _out << nl << "Slice.defineValue(" << localScope << "." << name << ", "
         << "iceC_" << getLocalScope(scoped, "_") << "_ids[" << scopedPos << "], "
         << (preserved ? "true" : "false");
    if (p->compactId() >= 0)
    {
        _out << ", " << p->compactId();
    }
    _out << ");";

    return false;
}

bool
Slice::Gen::TypesVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    const string scope = p->scope();
    const string scoped = p->scoped();
    const string localScope = getLocalScope(scope);
    const string name = fixId(p->name());
    const string prxName = p->name() + "Prx";

    InterfaceList bases = p->bases();
    StringList ids = p->ids();

    StringList::const_iterator firstIter = ids.begin();
    StringList::const_iterator scopedIter = find(ids.begin(), ids.end(), scoped);
    assert(scopedIter != ids.end());
    StringList::difference_type scopedPos = IceUtilInternal::distance(firstIter, scopedIter);

    _out << sp;
    _out << nl << "const iceC_" << getLocalScope(scoped, "_") << "_ids = [";
    _out.inc();

    for(StringList::const_iterator q = ids.begin(); q != ids.end(); ++q)
    {
        if(q != ids.begin())
        {
            _out << ',';
        }
        _out << nl << '"' << *q << '"';
    }

    _out.dec();
    _out << nl << "];";

    _out << sp;
    writeDocComment(p, getDeprecateReason(p));
    _out << nl << localScope << "." << p->name() << " = class extends Ice.Object";
    _out << sb;

    if (!bases.empty())
    {
        _out << sp;
        _out << nl << "static get _iceImplements()";
        _out << sb;
        _out << nl << "return [";
        _out.inc();
        for (InterfaceList::const_iterator q = bases.begin(); q != bases.end();)
        {
            InterfaceDefPtr base = *q;
            _out << nl << getLocalScope(base->scope()) << "." << base->name();
            if (++q != bases.end())
            {
                _out << ",";
            }
        }
        _out.dec();
        _out << nl << "];";
        _out << eb;
    }
    _out << eb << ";";

    //
    // Generate a proxy class
    //
    string proxyType = localScope + '.' + prxName;
    _out << sp;
    _out << nl << proxyType << " = class extends Ice.ObjectPrx";
    _out << sb;

    if (!bases.empty())
    {
        _out << sp;
        _out << nl << "static get _implements()";
        _out << sb;
        _out << nl << "return [";

        _out.inc();
        for (InterfaceList::const_iterator q = bases.begin(); q != bases.end();)
        {
            InterfaceDefPtr base = *q;
            _out << nl << getLocalScope(base->scope()) << "." << base->name() << "Prx";
            if (++q != bases.end())
            {
                _out << ",";
            }
        }
        _out.dec();
        _out << "];";
        _out << eb;
    }

    _out << eb << ";";

    _out << sp << nl << "Slice.defineOperations("
         << localScope << "." << p->name() << ", "
         << proxyType << ", "
         << "iceC_" << getLocalScope(scoped, "_") << "_ids, "
         << scopedPos;

    const OperationList ops = p->operations();
    if (!ops.empty())
    {
        _out << ',';
        _out << sb;
        for (OperationList::const_iterator q = ops.begin(); q != ops.end(); ++q)
        {
            if (q != ops.begin())
            {
                _out << ',';
            }

            OperationPtr op = *q;
            const string name = fixId(op->name());
            const TypePtr ret = op->deprecatedReturnType();
            const MemberList inParams = op->params();
            const MemberList outParams = op->outParameters();

            //
            // Each operation descriptor is a property. The key is the "on-the-wire"
            // name, and the value is an array consisting of the following elements:
            //
            //  0: servant method name in case of a keyword conflict (e.g., "_while"),
            //     otherwise an empty string
            //  1: mode (undefined == Normal or int)
            //  2: sendMode (undefined == Normal or int)
            //  3: amd (undefined or 1)
            //  4: format (undefined == Default or int)
            //  5: return type (undefined if void, or [type, tag])
            //  6: in params (undefined if none, or array of [type, tag])
            //  7: out params (undefined if none, or array of [type, tag])
            //  8: exceptions (undefined if none, or array of types)
            //  9: sends classes (true or undefined)
            // 10: returns classes (true or undefined)
            //
            _out << nl << "\"" << op->name() << "\": ["; // Operation name over-the-wire.

            if (name != op->name())
            {
                _out << "\"" << name << "\""; // Native method name.
            }
            _out << ", ";

            if (op->mode() != Operation::Normal)
            {
                _out << sliceModeToIceMode(op->mode()); // Mode.
            }
            _out << ", ";

            if (op->sendMode() != Operation::Normal)
            {
                _out << sliceModeToIceMode(op->sendMode()); // Send mode.
            }
            _out << ", ";

            if (op->format() != DefaultFormat)
            {
                _out << opFormatTypeToString(op); // Format.
            }
            _out << ", ";

            //
            // Return type.
            //
            if (ret)
            {
                _out << '[' << encodeTypeForOperation(ret);
                const bool isObj = ret->isClassType();
                if (isObj)
                {
                    _out << ", true";
                }
                if (op->returnIsTagged())
                {
                    if (!isObj)
                    {
                        _out << ", ";
                    }
                    _out << ", " << op->returnTag();
                }
                _out << ']';
            }
            _out << ", ";

            //
            // In params.
            //
            if (!inParams.empty())
            {
                _out << '[';
                for (auto pli = inParams.begin(); pli != inParams.end(); ++pli)
                {
                    if (pli != inParams.begin())
                    {
                        _out << ", ";
                    }
                    TypePtr t = (*pli)->type();
                    _out << '[' << encodeTypeForOperation(t);
                    const bool isObj = t->isClassType();
                    if (isObj)
                    {
                        _out << ", true";
                    }
                    if ((*pli)->tagged())
                    {
                        if (!isObj)
                        {
                            _out << ", ";
                        }
                        _out << ", " << (*pli)->tag();
                    }
                    _out << ']';
                }
                _out << ']';
            }
            _out << ", ";

            //
            // Out params.
            //
            if (!outParams.empty())
            {
                _out << '[';
                for (auto pli = outParams.begin(); pli != outParams.end(); ++pli)
                {
                    if (pli != outParams.begin())
                    {
                        _out << ", ";
                    }
                    TypePtr t = (*pli)->type();
                    _out << '[' << encodeTypeForOperation(t);
                    const bool isObj = t->isClassType();
                    if (isObj)
                    {
                        _out << ", true";
                    }
                    if ((*pli)->tagged())
                    {
                        if (!isObj)
                        {
                            _out << ", ";
                        }
                        _out << ", " << (*pli)->tag();
                    }
                    _out << ']';
                }
                _out << ']';
            }
            _out << ",";

            //
            // User exceptions.
            //
            ExceptionList throws = op->throws();
            throws.sort();
            throws.unique();
            throws.sort(Slice::DerivedToBaseCompare());
            if (throws.empty())
            {
                _out << " ";
            }
            else
            {
                _out << nl << '[';
                _out.inc();
                for (ExceptionList::const_iterator eli = throws.begin(); eli != throws.end(); ++eli)
                {
                    if (eli != throws.begin())
                    {
                        _out << ',';
                    }
                    _out << nl << fixId((*eli)->scoped());
                }
                _out.dec();
                _out << nl << ']';
            }
            _out << ", ";

            if (op->sendsClasses(false))
            {
                _out << "true";
            }
            _out << ", ";

            if (op->returnsClasses(false))
            {
                _out << "true";
            }

            _out << ']';
        }
        _out << eb;
    }
    _out << ");";

    return false;
}

void
Slice::Gen::TypesVisitor::visitSequence(const SequencePtr& p)
{
    const TypePtr type = unwrapIfOptional(p->type());

    //
    // Stream helpers for sequences are lazy initialized as the required
    // types might not be available until later.
    //
    const string scope = getLocalScope(p->scope());
    const string name = fixId(p->name());
    const string propertyName = name + "Helper";
    const bool fixed = !type->isVariableLength();

    _out << sp;
    _out << nl << "Slice.defineSequence(" << scope << ", \"" << propertyName << "\", "
         << "\"" << getHelper(type) << "\"" << ", " << (fixed ? "true" : "false");
    if(type->isClassType())
    {
        _out<< ", \"" << typeToString(type) << "\"";
    }
    _out << ");";
}

bool
Slice::Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    const string scope = p->scope();
    const string localScope = getLocalScope(scope);
    const string name = fixId(p->name());
    const ExceptionPtr base = p->base();
    string baseRef;
    if(base)
    {
        baseRef = fixId(base->scoped());
    }
    else
    {
        baseRef = "Ice.UserException";
    }

    const MemberList allDataMembers = p->allDataMembers();
    const MemberList dataMembers = p->dataMembers();

    vector<string> allParamNames;
    for (const auto& member : allDataMembers)
    {
        allParamNames.push_back(fixId(member->name()));
    }

    vector<string> baseParamNames;
    MemberList baseDataMembers;

    if (base)
    {
        baseDataMembers = base->allDataMembers();
        for (const auto& member : baseDataMembers)
        {
            baseParamNames.push_back(fixId(member->name()));
        }
    }

    _out << sp;
    writeDocComment(p, getDeprecateReason(p));
    _out << nl << localScope << '.' << name << " = class extends " << baseRef;
    _out << sb;

    _out << nl << "constructor" << spar;

    for (const auto& member : baseDataMembers)
    {
        _out << fixId(member->name());
    }

    for (const auto& member : dataMembers)
    {
        string value;
        if(member->defaultValueType())
        {
            value = writeConstantValue(scope, member->type(), member->defaultValueType(), member->defaultValue());
        }
        else if(member->tagged())
        {
            value = "undefined";
        }
        else
        {
            value = getValue(scope, member->type());
        }
        _out << (fixId(member->name()) + (value.empty() ? value : (" = " + value)));
    }

    _out << "_cause = \"\"" << epar;
    _out << sb;
    _out << nl << "super" << spar << baseParamNames << "_cause" << epar << ';';
    writeInitDataMembers(dataMembers);
    _out << eb;

    _out << sp;
    _out << nl << "static get _parent()";
    _out << sb;
    _out << nl << "return " << baseRef << ";";
    _out << eb;

    _out << sp;
    _out << nl << "static get _id()";
    _out << sb;
    _out << nl << "return \"" << p->scoped() << "\";";
    _out << eb;

    _out << sp;
    _out << nl << "_mostDerivedType()";
    _out << sb;
    _out << nl << "return " << localScope << '.' << name << ";";
    _out << eb;

    if(!dataMembers.empty())
    {
        _out << sp;
        _out << nl << "_writeMemberImpl(ostr)";
        _out << sb;
        writeMarshalDataMembers(dataMembers);
        _out << eb;

        _out << sp;
        _out << nl << "_readMemberImpl(istr)";
        _out << sb;
        writeUnmarshalDataMembers(dataMembers);
        _out << eb;
    }

    if(p->usesClasses(false) && (!base || (base && !base->usesClasses(false))))
    {
        _out << sp;
        _out << nl << "_usesClasses()";
        _out << sb;
        _out << nl << "return true;";
        _out << eb;
    }

    _out << eb << ";";

    bool basePreserved = p->inheritsMetadata("preserve-slice");
    bool preserved = p->hasMetadata("preserve-slice");

    if(preserved && !basePreserved)
    {
        _out << sp;
        _out << nl << "Slice.PreservedUserException(" << localScope << '.' << name << ");";
    }

    return false;
}

bool
Slice::Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    const string scope = p->scope();
    const string localScope = getLocalScope(scope);
    const string name = fixId(p->name());

    const MemberList dataMembers = p->dataMembers();

    vector<string> paramNames;
    for (const auto& member : dataMembers)
    {
        paramNames.push_back(fixId(member->name()));
    }

    _out << sp;
    writeDocComment(p, getDeprecateReason(p));
    _out << nl << localScope << '.' << name << " = class";
    _out << sb;

    _out << nl << "constructor" << spar;

    for (const auto& member : dataMembers)
    {
        string value;
        if (member->defaultValueType())
        {
            value = writeConstantValue(scope, member->type(), member->defaultValueType(), member->defaultValue());
        }
        else if (member->tagged())
        {
            value = "undefined";
        }
        else
        {
            value = getValue(scope, member->type());
        }
        _out << (fixId(member->name()) + (value.empty() ? value : (" = " + value)));
    }

    _out << epar;
    _out << sb;
    writeInitDataMembers(dataMembers);
    _out << eb;

    _out << sp;
    _out << nl << "_write(ostr)";
    _out << sb;
    writeMarshalDataMembers(dataMembers);
    _out << eb;

    _out << sp;
    _out << nl << "_read(istr)";
    _out << sb;
    writeUnmarshalDataMembers(dataMembers);
    _out << eb;

    _out << sp;
    _out << nl << "static get minWireSize()";
    _out << sb;
    _out << nl << "return  " << p->minWireSize() << ";";
    _out << eb;

    _out << eb << ";";

    //
    // Only generate hashCode if this structure type is a legal dictionary key type.
    //
    bool containsSequence = false;
    bool legalKeyType = Dictionary::legalKeyType(p, containsSequence);

    _out << sp;
    _out << nl << "Slice.defineStruct(" << localScope << '.' << name << ", "
         << (legalKeyType ? "true" : "false") << ", "
         << (p->isVariableLength() ? "true" : "false") << ");";
    return false;
}

void
Slice::Gen::TypesVisitor::visitDictionary(const DictionaryPtr& p)
{
    const TypePtr keyType = p->keyType();
    const TypePtr valueType = p->valueType();

    //
    // For some key types, we have to use an equals() method to compare keys
    // rather than the native comparison operators.
    //
    bool keyUseEquals = false;
    BuiltinPtr b = BuiltinPtr::dynamicCast(keyType);
    if((b && (b->kind() == Builtin::KindLong || b->kind() == Builtin::KindULong || b->kind() == Builtin::KindVarLong ||
        b->kind() == Builtin::KindVarULong)) || StructPtr::dynamicCast(keyType))
    {
        keyUseEquals = true;
    }

    //
    // Stream helpers for dictionaries of objects are lazy initialized
    // as the required object type might not be available until later.
    //
    const string scope = getLocalScope(p->scope());
    const string name = fixId(p->name());
    const string propertyName = name + "Helper";
    bool fixed = !keyType->isVariableLength() && !valueType->isVariableLength();

    _out << sp;
    _out << nl << "Slice.defineDictionary(" << scope << ", \"" << name << "\", \"" << propertyName << "\", "
         << "\"" << getHelper(keyType) << "\", "
         << "\"" << getHelper(valueType) << "\", "
         << (fixed ? "true" : "false") << ", "
         << (keyUseEquals ? "Ice.HashMap.compareEquals" : "undefined");

    if(valueType->isClassType())
    {
        _out << ", \"" << typeToString(valueType) << "\"";
    }
    else
    {
        _out << ", undefined";
    }

    if(SequencePtr::dynamicCast(unwrapIfOptional(valueType)))
    {
        _out << ", Ice.ArrayUtil.equals";
    }
    _out << ");";
}

void
Slice::Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    const string scope = p->scope();
    const string localScope = getLocalScope(scope);
    const string name = fixId(p->name());

    _out << sp;
    writeDocComment(p, getDeprecateReason(p));
    _out << nl << localScope << '.' << name << " = Slice.defineEnum([";
    _out.inc();
    _out << nl;

    const EnumeratorList enumerators = p->enumerators();
    int i = 0;
    for(EnumeratorList::const_iterator en = enumerators.begin(); en != enumerators.end(); ++en)
    {
        if(en != enumerators.begin())
        {
            if(++i % 5 == 0)
            {
                _out << ',' << nl;
            }
            else
            {
                _out << ", ";
            }
        }
        _out << "['" << fixId((*en)->name()) << "', " << (*en)->value() << ']';
    }
    _out << "]);";
    _out.dec();

    //
    // EnumBase provides an equals() method
    //
}

void
Slice::Gen::TypesVisitor::visitConst(const ConstPtr& p)
{
    const string scope = p->scope();
    const string localScope = getLocalScope(scope);
    const string name = fixId(p->name());

    _out << sp;
    _out << nl << "Object.defineProperty(" << localScope << ", '" << name << "', {";
    _out.inc();
    _out << nl << "value: ";
    _out << writeConstantValue(scope, p->type(), p->valueType(), p->value());
    _out.dec();
    _out << nl << "});";
}

string
Slice::Gen::TypesVisitor::encodeTypeForOperation(const TypePtr& constType)
{
    TypePtr type = unwrapIfOptional(constType);
    assert(type);

    static const std::array<std::string, 17> builtinTable =
    {
        "1",  // bool
        "0",  // byte
        "2",  // short
        "?",  // ushort
        "3",  // int
        "?",  // uint
        "?",  // varint
        "?",  // varuint
        "4",  // long
        "?",  // ulong
        "?", // varlong
        "?", // varulong
        "5", // float
        "6", // double
        "7", // string
        "8", // Ice.ObjectPrx
        "9", // Ice.Value
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        return builtinTable[builtin->kind()];
    }

    InterfaceDeclPtr proxy = InterfaceDeclPtr::dynamicCast(type);
    if(proxy)
    {
        return "\"" + fixId(proxy->scoped() + "Prx") + "\"";
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        return "\"" + fixId(seq->scoped() + "Helper") + "\"";
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    if(d)
    {
        return "\"" + fixId(d->scoped() + "Helper") + "\"";
    }

    EnumPtr e = EnumPtr::dynamicCast(type);
    if(e)
    {
        return fixId(e->scoped()) + "._helper";
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        return fixId(st->scoped());
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        return "\"" + fixId(cl->scoped()) + "\"";
    }

    assert(0);
    return "???";
}

Slice::Gen::ExportVisitor::ExportVisitor(IceUtilInternal::Output& out, bool icejs, bool es6modules) :
    JsVisitor(out),
    _icejs(icejs),
    _es6modules(es6modules)
{
}

bool
Slice::Gen::ExportVisitor::visitModuleStart(const ModulePtr& p)
{
    const bool topLevel = UnitPtr::dynamicCast(p->container());
    if(topLevel)
    {
        const string localScope = getLocalScope(p->scope());
        const string name = localScope.empty() ? fixId(p->name()) : localScope + "." + p->name();
        if(find(_exported.begin(), _exported.end(), name) == _exported.end())
        {
            _exported.push_back(name);
            if(_es6modules)
            {
                _out << nl << "export { " << name << " };";
            }
            else
            {
                if(_icejs)
                {
                    _out.zeroIndent();
                    _out << nl << "/* slice2js browser-bundle-skip */";
                    _out.restoreIndent();
                }
                _out << nl << "exports." << name << " = " << name << ";";
                if(_icejs)
                {
                    _out.zeroIndent();
                    _out << nl << "/* slice2js browser-bundle-skip-end */";
                    _out.restoreIndent();
                }
            }
        }
    }
    return false;
}

Slice::Gen::TypeScriptRequireVisitor::TypeScriptRequireVisitor(IceUtilInternal::Output& out, bool icejs) :
    JsVisitor(out),
    _icejs(icejs),
    _nextImport(0)
{
}

string
Slice::Gen::TypeScriptRequireVisitor::nextImportPrefix()
{
    ostringstream ns;
    ns << "iceNS" << _nextImport++;
    return ns.str();
}

void
Slice::Gen::TypeScriptRequireVisitor::addImport(const TypePtr& type, const ContainedPtr& toplevel)
{
    TypePtr definition = unwrapIfOptional(type);
    if(!BuiltinPtr::dynamicCast(definition))
    {
        const string m1 = getModuleMetadata(definition);
        const string m2 = getModuleMetadata(toplevel);

        const string p1 = definition->definitionContext()->filename();
        const string p2 = toplevel->definitionContext()->filename();

        addImport(m1, m2, p1, p2);
    }
}

void
Slice::Gen::TypeScriptRequireVisitor::addImport(const ContainedPtr& definition, const ContainedPtr& toplevel)
{
    const string m1 = getModuleMetadata(definition);
    const string m2 = getModuleMetadata(toplevel);

    const string p1 = definition->definitionContext()->filename();
    const string p2 = toplevel->definitionContext()->filename();

    addImport(m1, m2, p1, p2);
}

void
Slice::Gen::TypeScriptRequireVisitor::addImport(const string& m1, const string& m2,
                                                const string& p1, const string& p2)
{
    //
    // Generate an import for a definition that is outside a JS module and comes from
    // a different definition context or for a definition defined in a module different
    // than the current module.
    //
    if(m1.empty())
    {
        if(p1 != p2)
        {
            string relpath = relativePath(p1, p2);

            string::size_type pos = relpath.rfind('.');
            if(pos != string::npos)
            {
                relpath.erase(pos);
            }

            for(vector<pair<string, string> >::const_iterator i = _imports.begin(); i != _imports.end(); ++i)
            {
                if(i->first == relpath)
                {
                    return;
                }
            }
            _imports.push_back(make_pair(relpath, nextImportPrefix()));
        }
    }
    else if(m1 != m2)
    {
        for(vector<pair<string, string> >::const_iterator i = _imports.begin(); i != _imports.end(); ++i)
        {
            if(i->first == m1)
            {
                return;
            }
        }
        _imports.push_back(make_pair(m1, nextImportPrefix()));
    }
}

bool
Slice::Gen::TypeScriptRequireVisitor::visitModuleStart(const ModulePtr& p)
{
    //
    // Import ice module if not building Ice
    //
    if(UnitPtr::dynamicCast(p->container()) && !_icejs && _imports.empty())
    {
        _imports.push_back(make_pair("ice", nextImportPrefix()));
    }
    return true;
}

bool
Slice::Gen::TypeScriptRequireVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    //
    // Add imports required for base class
    //
    if (p->base())
    {
        addImport(ContainedPtr::dynamicCast(p->base()), p);
    }

    // Add imports required for data members
    for (const auto& member : p->allDataMembers())
    {
        addImport(member->type(), p);
    }

    return false;
}

bool
Slice::Gen::TypeScriptRequireVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    //
    // Add imports required for base interfaces
    //
    InterfaceList bases = p->bases();
    for(InterfaceList::const_iterator i = bases.begin(); i != bases.end(); ++i)
    {
        addImport(ContainedPtr::dynamicCast(*i), p);
    }

    //
    // Add imports required for operation parameters and return type
    //
    const OperationList operationList = p->allOperations();
    for(OperationList::const_iterator i = operationList.begin(); i != operationList.end(); ++i)
    {
        const TypePtr ret = (*i)->deprecatedReturnType();
        if(ret && ret->definitionContext())
        {
            addImport(ret, p);
        }

        for (const auto& param : (*i)->allMembers())
        {
            addImport(param->type(), p);
        }
    }
    return false;
}

bool
Slice::Gen::TypeScriptRequireVisitor::visitStructStart(const StructPtr& p)
{
    // Add imports required for data members
    for (const auto& member : p->dataMembers())
    {
        addImport(member->type(), p);
    }
    return false;
}

bool
Slice::Gen::TypeScriptRequireVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    //
    // Add imports required for base exceptions
    //
    ExceptionPtr base = p->base();
    if(base)
    {
        addImport(ContainedPtr::dynamicCast(base), p);
    }

    // Add imports required for data members
    for (const auto& member : p->allDataMembers())
    {
        addImport(member->type(), p);
    }
    return false;
}

void
Slice::Gen::TypeScriptRequireVisitor::visitSequence(const SequencePtr& seq)
{
    //
    // Add import required for the sequence element type
    //
    addImport(seq->type(), seq);
}

void
Slice::Gen::TypeScriptRequireVisitor::visitDictionary(const DictionaryPtr& dict)
{
    //
    // Add imports required for the dictionary key and value types
    //
    addImport(dict->keyType(), dict);
    addImport(dict->valueType(), dict);
}

Slice::Gen::TypeScriptAliasVisitor::TypeScriptAliasVisitor(IceUtilInternal::Output& out) :
    JsVisitor(out)
{
}

void
Slice::Gen::TypeScriptAliasVisitor::addAlias(const ExceptionPtr& type, const ContainedPtr& toplevel)
{
    string m1 = getModuleMetadata(ContainedPtr::dynamicCast(type));
    string m2 = getModuleMetadata(toplevel);

    //
    // Do not add alias for a type defined in the current module
    //
    if(!m1.empty() && m1 == m2)
    {
        return;
    }

    const string prefix = importPrefix(ContainedPtr::dynamicCast(type), toplevel, imports());
    const string typeS = prefix + getUnqualified(fixId(type->scoped()), toplevel->scope(), prefix);

    addAlias(typeS, prefix, toplevel);
}

void
Slice::Gen::TypeScriptAliasVisitor::addAlias(const TypePtr& type, const ContainedPtr& toplevel)
{
    string m1 = getModuleMetadata(type);
    string m2 = getModuleMetadata(toplevel);

    //
    // Do not add alias for a type defined in the current module
    //
    if(!m1.empty() && m1 == m2)
    {
        return;
    }

    addAlias(typeToString(type, toplevel, imports(), true),
             importPrefix(type, toplevel, imports()),
             toplevel);
}

void
Slice::Gen::TypeScriptAliasVisitor::addAlias(const string& type, const string& prefix,
                                             const ContainedPtr& toplevel)
{
    const string scope = fixId(toplevel->scoped()) + ".";
    //
    // When using an import prefix we don't need an alias, prefixes use iceNSXX that is reserved
    // name prefix
    //
    string::size_type i = type.find(".");
    if(prefix.empty() && i != string::npos)
    {
        if(scope.find("." + type.substr(0, i + 1)) != string::npos)
        {
            for(vector<pair<string, string> >::const_iterator j = _aliases.begin(); j != _aliases.end(); ++j)
            {
                if(j->first == type)
                {
                    return;
                }
            }
            string alias = type;
            replace(alias.begin(), alias.end(), '.', '_');
            //
            // We prefix alias with iceA this avoid conflict with iceNSX used for
            // import prefixes
            //
            _aliases.push_back(make_pair(type, "iceA_" + alias));
        }
    }
}

bool
Slice::Gen::TypeScriptAliasVisitor::visitModuleStart(const ModulePtr&)
{
    return true;
}

bool
Slice::Gen::TypeScriptAliasVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    ModulePtr module = ModulePtr::dynamicCast(p->container());
    //
    // Add alias required for base class
    //
    if (p->base())
    {
        addAlias(TypePtr::dynamicCast(p->base()->declaration()), module);
    }

    // Add alias required for data members
    for (const auto& member : p->allDataMembers())
    {
        addAlias(member->type(), module);
    }

    return false;
}

bool
Slice::Gen::TypeScriptAliasVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    ModulePtr module = ModulePtr::dynamicCast(p->container());
    //
    // Add alias required for base interfaces
    //
    InterfaceList bases = p->bases();
    for(InterfaceList::const_iterator i = bases.begin(); i != bases.end(); ++i)
    {
        addAlias(TypePtr::dynamicCast((*i)->declaration()), module);
    }

    //
    // Add alias required for operation parameters
    //
    const OperationList operationList = p->allOperations();
    for(OperationList::const_iterator i = operationList.begin(); i != operationList.end(); ++i)
    {
        const TypePtr ret = (*i)->deprecatedReturnType();
        if(ret && ret->definitionContext())
        {
            addAlias(ret, module);
        }

        for (const auto& param : (*i)->allMembers())
        {
            addAlias(param->type(), module);
        }
    }
    return false;
}

bool
Slice::Gen::TypeScriptAliasVisitor::visitStructStart(const StructPtr& p)
{
    // Add alias required for data members
    for (const auto& member : p->dataMembers())
    {
        addAlias(member->type(), ModulePtr::dynamicCast(p->container()));
    }
    return false;
}

bool
Slice::Gen::TypeScriptAliasVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    ModulePtr module = ModulePtr::dynamicCast(p->container());
    //
    // Add alias required for base exception
    //
    ExceptionPtr base = p->base();
    if(base)
    {
        addAlias(base, module);
    }

    // Add alias required for data members
    for (const auto& member : p->allDataMembers())
    {
        addAlias(member->type(), module);
    }
    return false;
}

void
Slice::Gen::TypeScriptAliasVisitor::visitSequence(const SequencePtr& seq)
{
    addAlias(seq->type(), ModulePtr::dynamicCast(seq->container()));
}

void
Slice::Gen::TypeScriptAliasVisitor::visitDictionary(const DictionaryPtr& dict)
{
    ModulePtr module = ModulePtr::dynamicCast(dict->container());
    addAlias(dict->keyType(), module);
    addAlias(dict->valueType(), module);
}

void
Slice::Gen::TypeScriptAliasVisitor::writeAlias(const UnitPtr&)
{
    if(!_aliases.empty())
    {
        _out << sp;
        for(vector<pair<string, string> >::const_iterator i = _aliases.begin(); i != _aliases.end(); ++i)
        {
            _out << nl << "type " << i->second << " = " << i->first << ";";
        }
    }
}

Slice::Gen::TypeScriptVisitor::TypeScriptVisitor(::IceUtilInternal::Output& out,
                                                 const vector<pair<string, string> >& imports) :
    JsVisitor(out, imports),
    _wroteImports(false)
{
}

void
Slice::Gen::TypeScriptVisitor::writeImports()
{
    if(!_wroteImports)
    {
        for(vector<pair<string, string> >::const_iterator i = _imports.begin(); i != _imports.end(); ++i)
        {
            _out << nl << "import * as " << i->second << " from \"" << i->first << "\"";
        }
        _wroteImports = true;
    }
}

bool
Slice::Gen::TypeScriptVisitor::visitModuleStart(const ModulePtr& p)
{
    UnitPtr unit = UnitPtr::dynamicCast(p->container());
    if(unit)
    {
        _out << sp;
        writeImports();
        _out << nl << "export namespace " << fixId(p->name()) << sb;
    }
    else
    {
        _out << nl << "namespace " << fixId(p->name()) << sb;
    }
    return true;
}

void
Slice::Gen::TypeScriptVisitor::visitModuleEnd(const ModulePtr&)
{
    _out << eb; // namespace end
}

bool
Slice::Gen::TypeScriptVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    const string toplevelModule = getModuleMetadata(ContainedPtr::dynamicCast(p));
    const string icePrefix = importPrefix("Ice.", p);

    const MemberList dataMembers = p->dataMembers();
    const MemberList allDataMembers = p->allDataMembers();
    _out << sp;
    writeDocSummary(_out, p);
    _out << nl << "class " << fixId(p->name()) << " extends ";
    const string scope = p->scope();
    const string scoped = p->scoped();

    if (p->base())
    {
        const string prefix = importPrefix(ContainedPtr::dynamicCast(p->base()), p, imports());
        _out << prefix << getUnqualified(fixId(p->base()->scoped()), p->scope(), prefix);
    }
    else
    {
        _out << icePrefix << getUnqualified("Ice.Value", p->scope(), icePrefix);
    }
    _out << sb;
    _out << nl << "/**";
    _out << nl << " * One-shot constructor to initialize all data members.";
    for (const auto& member : allDataMembers)
    {
        if (CommentPtr comment = member->parseComment(false))
        {
            _out << nl << " * @param " << fixId(member->name()) << " " << getDocSentence(comment->overview());
        }
    }
    _out << nl << " */";
    _out << nl << "constructor" << spar;
    for (const auto& member : allDataMembers)
    {
        _out << (fixId(member->name()) + "?:" + typeToString(member->type(), p, imports(), true, false, true));
    }
    _out << epar << ";";
    for (const auto& member : dataMembers)
    {
        writeDocSummary(_out, member);
        _out << nl << fixId(member->name()) << ":" << typeToString(member->type(), p, imports(), true, false, true)
             << ";";
    }
    _out << eb;
    return false;
}

bool
Slice::Gen::TypeScriptVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    const string toplevelModule = getModuleMetadata(ContainedPtr::dynamicCast(p));
    const string icePrefix = importPrefix("Ice.", p);
    const OperationList ops = p->allOperations();

    //
    // Define servant an proxy types
    //
    _out << sp;
    _out << nl << "abstract class " << fixId(p->name() + "Prx")
         << " extends " << icePrefix << getUnqualified("Ice.ObjectPrx", p->scope(), icePrefix);
    _out << sb;

    for (OperationList::const_iterator q = ops.begin(); q != ops.end(); ++q)
    {
        const OperationPtr op = *q;
        const MemberList paramList = op->allMembers();
        const TypePtr ret = op->deprecatedReturnType();
        const MemberList inParams = op->params();
        const MemberList outParams = op->outParameters();

        const string contextParam = escapeParam(paramList, "context");
        CommentPtr comment = op->parseComment(false);
        const string contextDoc = "@param " + contextParam + " The Context map to send with the invocation.";
        const string asyncDoc = "The asynchronous result object for the invocation.";
        if (comment)
        {
            StringList postParams, returns;
            postParams.push_back(contextDoc);
            returns.push_back(asyncDoc);
            writeOpDocSummary(_out, op, comment, OpDocInParams, false, StringList(), postParams, returns);
        }
        _out << nl << fixId((*q)->name()) << spar;
        for (const auto& param : inParams)
        {
            _out << (fixId(param->name()) + (param->tagged() ? "?" : "") + ":" +
                     typeToString(param->type(), p, imports(), true, false, true));
        }
        _out << "context?:Map<string, string>";
        _out << epar;

        _out << ":" << icePrefix << getUnqualified("Ice.AsyncResult", p->scope(), icePrefix);
        if (!ret && outParams.empty())
        {
            _out << "<void>";
        }
        else if ((ret && outParams.empty()) || (!ret && outParams.size() == 1))
        {
            TypePtr t = ret ? ret : outParams.front()->type();
            _out << "<" << typeToString(t, p, imports(), true, false, true) << ">";
        }
        else
        {
            _out << "<[";
            if (ret)
            {
                _out << typeToString(ret, p, imports(), true, false, true) << ", ";
            }

            for (auto i = outParams.begin(); i != outParams.end();)
            {
                _out << typeToString((*i)->type(), p, imports(), true, false, true);
                if (++i != outParams.end())
                {
                    _out << ", ";
                }
            }

            _out << "]>";
        }

        _out << ";";
    }

    const string iceProxyPrefix = importPrefix("Ice.ObjectPrx", p);
    _out << sp;
    _out << nl << "/**";
    _out << nl << " * Downcasts a proxy without confirming the target object's type via a remote invocation.";
    _out << nl << " * @param prx The target proxy.";
    _out << nl << " * @return A proxy with the requested type.";
    _out << nl << " */";
    _out << nl << "static uncheckedCast(prx:" << iceProxyPrefix
         << getUnqualified("Ice.ObjectPrx", p->scope(), iceProxyPrefix) << ", "
         << "facet?:string):"
         << fixId(p->name() + "Prx") << ";";
    _out << nl << "/**";
    _out << nl << " * Downcasts a proxy after confirming the target object's type via a remote invocation.";
    _out << nl << " * @param prx The target proxy.";
    _out << nl << " * @param facet A facet name.";
    _out << nl << " * @param context The context map for the invocation.";
    _out << nl << " * @return A proxy with the requested type and facet, or nil if the target proxy is nil or the target";
    _out << nl << " * object does not support the requested type.";
    _out << nl << " */";
    _out << nl << "static checkedCast(prx:" << iceProxyPrefix
         << getUnqualified("Ice.ObjectPrx", p->scope(), iceProxyPrefix) << ", "
         << "facet?:string, contex?:Map<string, string>):" << iceProxyPrefix
         << getUnqualified("Ice.AsyncResult", p->scope(), iceProxyPrefix) << "<" << fixId(p->name() + "Prx") << ">;";
    _out << eb;

    _out << sp;
    _out << nl << "abstract class " << fixId(p->name())
         << " extends " << iceProxyPrefix << getUnqualified("Ice.Object", p->scope(), iceProxyPrefix);
    _out << sb;
    for (OperationList::const_iterator q = ops.begin(); q != ops.end(); ++q)
    {
        const OperationPtr op = *q;
        const MemberList paramList = op->allMembers();
        const TypePtr ret = op->deprecatedReturnType();
        const MemberList inParams = op->params();
        const MemberList outParams = op->outParameters();

        const string currentParam = escapeParam(inParams, "current");
        CommentPtr comment = p->parseComment(false);
        const string currentDoc = "@param " + currentParam + " The Current object for the invocation.";
        const string resultDoc = "The result or a promise like object that will "
                                 "be resolved with the result of the invocation.";
        if (comment)
        {
            StringList postParams, returns;
            postParams.push_back(currentDoc);
            returns.push_back(resultDoc);
            writeOpDocSummary(_out, op, comment, OpDocInParams, false, StringList(), postParams, returns);
        }
        _out << nl << "abstract " << fixId((*q)->name()) << spar;
        for (const auto& param : inParams)
        {
            _out << (fixId(param->name()) + ":" + typeToString(param->type(), p, imports(), true, false, true));
        }
        _out << ("current:" + iceProxyPrefix + getUnqualified("Ice.Current", p->scope(), iceProxyPrefix));
        _out << epar << ":";

        if (!ret && outParams.empty())
        {
            _out << "PromiseLike<void>|void";
        }
        else if ((ret && outParams.empty()) || (!ret && outParams.size() == 1))
        {
            TypePtr t = ret ? ret : outParams.front()->type();
            string returnType = typeToString(t, p, imports(), true, false, true);
            _out << "PromiseLike<" << returnType << ">|" << returnType;
        }
        else
        {
            ostringstream os;
            if (ret)
            {
                os << typeToString(ret, p, imports(), true, false, true) << ", ";
            }

            for (auto i = outParams.begin(); i != outParams.end();)
            {
                os << typeToString((*i)->type(), p, imports(), true, false, true);
                if (++i != outParams.end())
                {
                    os << ", ";
                }
            }
            _out << "PromiseLike<[" << os.str() << "]>|[" << os.str() << "]";
        }
        _out << ";";
    }
    _out << nl << "/**";
    _out << nl << " * Obtains the Slice type ID of this type.";
    _out << nl << " * @return The return value is always \"" + p->scoped() + "\".";
    _out << nl << " */";
    _out << nl << "static ice_staticId():string;";
    _out << eb;

    return false;
}

bool
Slice::Gen::TypeScriptVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    const string name = fixId(p->name());
    const MemberList dataMembers = p->dataMembers();
    const MemberList allDataMembers = p->allDataMembers();
    const string toplevelModule = getModuleMetadata(ContainedPtr::dynamicCast(p));
    const string icePrefix = importPrefix("Ice.", p);

    ExceptionPtr base = p->base();
    string baseRef;
    if(base)
    {
        const string prefix = importPrefix(ContainedPtr::dynamicCast(base), p, imports());
        baseRef = prefix + getUnqualified(fixId(base->scoped()), p->scope(), prefix);
    }
    else
    {
        baseRef = icePrefix + getUnqualified("Ice.UserException", p->scope(), icePrefix);
    }

    _out << sp;
    writeDocSummary(_out, p);
    _out << nl << "class " << name << " extends " << baseRef << sb;
    if(!allDataMembers.empty())
    {
        _out << nl << "/**";
        _out << nl << " * One-shot constructor to initialize all data members.";
        for (const auto& member : allDataMembers)
        {
            if (CommentPtr comment = member->parseComment(false))
            {
                _out << nl << " * @param " << fixId(member->name()) << " " << getDocSentence(comment->overview());
            }
        }
        _out << nl << " * @param ice_cause The error that cause this exception.";
        _out << nl << " */";
        _out << nl << "constructor" << spar;
        for (const auto& member : allDataMembers)
        {
            _out << (fixId(member->name()) + "?:" + typeToString(member->type(), p, imports(), true, false, true));
        }
        _out << "ice_cause?:string|Error";
        _out << epar << ";";
    }
    for (const auto& member : dataMembers)
    {
        _out << nl << fixId(member->name()) << ":" << typeToString(member->type(), p, imports(), true, false, true)
             << ";";
    }
    _out << eb;
    return false;
}

bool
Slice::Gen::TypeScriptVisitor::visitStructStart(const StructPtr& p)
{
    const string icePrefix = importPrefix("Ice.", p);
    const string name = fixId(p->name());
    const MemberList dataMembers = p->dataMembers();
    const string toplevelModule = getModuleMetadata(ContainedPtr::dynamicCast(p));
    _out << sp;
    writeDocSummary(_out, p);
    _out << nl << "class " << name << sb;
    _out << nl << "constructor" << spar;
    for (const auto& member : dataMembers)
    {
        _out << (fixId(member->name()) + "?:" + typeToString(member->type(), p, imports(), true, false, true));
    }
    _out << epar << ";";

    _out << nl << "clone():" << name << ";";
    _out << nl << "equals(rhs:any):boolean;";

    //
    // Only generate hashCode if this structure type is a legal dictionary key type.
    //
    bool containsSequence = false;
    bool legalKeyType = Dictionary::legalKeyType(p, containsSequence);
    if(legalKeyType)
    {
        _out << nl << "hashCode():number;";
    }

    for (const auto& member : dataMembers)
    {
        _out << nl << fixId(member->name()) << ":" << typeToString(member->type(), p, imports(), true, false, true)
             << ";";
    }

    //
    // Streaming API
    //
    _out << nl << "static write(outs:" << icePrefix << getUnqualified("Ice.OutputStream", p->scope(), icePrefix)
         << ", value:" << name << "):void;";
    _out << nl << "static read(ins:" << icePrefix << getUnqualified("Ice.InputStream", p->scope(), icePrefix) << "):"
         << name << ";";
    _out << eb;
    return false;
}

void
Slice::Gen::TypeScriptVisitor::visitSequence(const SequencePtr& p)
{
    const string icePrefix = importPrefix("Ice.", p);
    const string toplevelModule = getModuleMetadata(ContainedPtr::dynamicCast(p));
    const string name = fixId(p->name());
    _out << sp;
    writeDocSummary(_out, p);
    _out << nl << "type " << name << " = " << typeToString(p, p, imports(), true, true) << ";";

    _out << sp;
    _out << nl << "class " << fixId(p->name() + "Helper");
    _out << sb;
    //
    // Streaming API
    //
    _out << nl << "static write(outs:" << icePrefix << getUnqualified("Ice.OutputStream", p->scope(), icePrefix)
         << ", value:" << name << "):void;";
    _out << nl << "static read(ins:" << icePrefix << getUnqualified("Ice.InputStream", p->scope(), icePrefix) << "):"
         << name << ";";
    _out << eb;
}

void
Slice::Gen::TypeScriptVisitor::visitDictionary(const DictionaryPtr& p)
{
    const string icePrefix = importPrefix("Ice.", p);
    const string toplevelModule = getModuleMetadata(ContainedPtr::dynamicCast(p));
    const string name = fixId(p->name());
    _out << sp;
    writeDocSummary(_out, p);
    _out << nl << "class " << name << " extends " << typeToString(p, p, imports(), true, true);
    _out << sb;
    _out << eb;

    _out << sp;
    _out << nl << "class " << fixId(p->name() + "Helper");
    _out << sb;
    //
    // Streaming API
    //
    _out << nl << "static write(outs:" << icePrefix << getUnqualified("Ice.OutputStream", p->scope(), icePrefix)
         << ", value:" << name << "):void;";
    _out << nl << "static read(ins:" << icePrefix << getUnqualified("Ice.InputStream", p->scope(), icePrefix) << "):"
         << name << ";";
    _out << eb;
}

void
Slice::Gen::TypeScriptVisitor::visitEnum(const EnumPtr& p)
{
    _out << sp;
    writeDocSummary(_out, p);
    _out << nl << "class " << fixId(p->name());
    _out << sb;
    const EnumeratorList enumerators = p->enumerators();
    for(EnumeratorList::const_iterator en = enumerators.begin(); en != enumerators.end(); ++en)
    {
        writeDocSummary(_out, *en);
        _out << nl << "static readonly " << fixId((*en)->name()) << ":" << fixId(p->name()) << ";";
    }
    _out << nl;
    _out << nl << "static valueOf(value:number):" << fixId(p->name()) << ";";
    _out << nl << "equals(other:any):boolean;";
    _out << nl << "hashCode():number;";
    _out << nl << "toString():string;";
    _out << nl;
    _out << nl << "readonly name:string;";
    _out << nl << "readonly value:number;";
    _out << eb;
}

void
Slice::Gen::TypeScriptVisitor::visitConst(const ConstPtr& p)
{
    const string toplevelModule = getModuleMetadata(p->type());
    _out << sp;
    writeDocSummary(_out, p);
    _out << nl << "const " << fixId(p->name()) << ":" << typeToString(p->type(), p, imports(), true) << ";";
}
