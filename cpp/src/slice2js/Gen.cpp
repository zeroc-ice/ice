// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Functional.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/InputUtil.h>
#include <Gen.h>
#include <IceUtil/Iterator.h>
#include <IceUtil/UUID.h>
#include <Slice/Checksum.h>
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

string
getDeprecateReason(const ContainedPtr& p1, const ContainedPtr& p2, const string& type)
{
    string deprecateMetadata, deprecateReason;
    if(p1->findMetaData("deprecate", deprecateMetadata) ||
       (p2 != 0 && p2->findMetaData("deprecate", deprecateMetadata)))
    {
        deprecateReason = "This " + type + " has been deprecated.";
        const string prefix = "deprecate:";
        if(deprecateMetadata.find(prefix) == 0 && deprecateMetadata.size() > prefix.size())
        {
            deprecateReason = deprecateMetadata.substr(prefix.size());
        }
    }
    return deprecateReason;
}

void
printHeader(IceUtilInternal::Output& out)
{
    static const char* header =
        "// **********************************************************************\n"
        "//\n"
        "// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.\n"
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

string
escapeParam(const ParamDeclList& params, const string& name)
{
    string r = name;
    for(ParamDeclList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        if(Slice::JsGenerator::fixId((*p)->name()) == name)
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

    out << nl << " */";
}

enum OpDocParamType { OpDocInParams, OpDocOutParams, OpDocAllParams };

void
writeOpDocParams(Output& out, const OperationPtr& op, const CommentPtr& doc, OpDocParamType type,
                 const StringList& preParams = StringList(), const StringList& postParams = StringList())
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
    for (ParamDeclList::iterator p = params.begin(); p != params.end(); ++p)
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
Slice::JsVisitor::writeMarshalDataMembers(const DataMemberList& dataMembers, const DataMemberList& optionalMembers)
{
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        if(!(*q)->optional())
        {
            writeMarshalUnmarshalCode(_out, (*q)->type(), "this." + fixId((*q)->name()), true);
        }
    }

    for(DataMemberList::const_iterator q = optionalMembers.begin(); q != optionalMembers.end(); ++q)
    {
        writeOptionalMarshalUnmarshalCode(_out, (*q)->type(), "this." + fixId((*q)->name()), (*q)->tag(), true);
    }
}

void
Slice::JsVisitor::writeUnmarshalDataMembers(const DataMemberList& dataMembers, const DataMemberList& optionalMembers)
{
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        if(!(*q)->optional())
        {
            writeMarshalUnmarshalCode(_out, (*q)->type(), "this." + fixId((*q)->name()), false);
        }
    }

    for(DataMemberList::const_iterator q = optionalMembers.begin(); q != optionalMembers.end(); ++q)
    {
        writeOptionalMarshalUnmarshalCode(_out, (*q)->type(), "this." + fixId((*q)->name()), (*q)->tag(), false);
    }
}

void
Slice::JsVisitor::writeInitDataMembers(const DataMemberList& dataMembers)
{
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        const string m = fixId((*q)->name());
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
                break;
            }
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindInt:
            {
                return "0";
                break;
            }
            case Builtin::KindLong:
            {
                return "new Ice.Long(0, 0)";
                break;
            }
            case Builtin::KindFloat:
            case Builtin::KindDouble:
            {
                return "0.0";
                break;
            }
            case Builtin::KindString:
            {
                return "\"\"";
            }
            default:
            {
                return "null";
                break;
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
Slice::JsVisitor::writeConstantValue(const string& /*scope*/, const TypePtr& type, const SyntaxTreeBasePtr& valueType,
                                     const string& value)
{
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
        else if(bp && bp->kind() == Builtin::KindLong)
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

    string file = _fileBase + (typeScript ? ".d.ts" : ".js");

    if(!dir.empty())
    {
        file = dir + '/' + file;
    }

    _out.open(file.c_str());
    if(!_out)
    {
        ostringstream os;
        os << "cannot open `" << file << "': " << IceUtilInternal::errorToString(errno);
        throw FileException(__FILE__, __LINE__, os.str());
    }
    FileTracker::instance()->addFile(file);

    printHeader(_out);
    printGeneratedHeader(_out, _fileBase + ".ice");
}

Slice::Gen::Gen(const string& base, const vector<string>& includePaths, const string& /*dir*/, bool typeScript,
                ostream& out) :
    _out(out),
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

    printHeader(_out);
    printGeneratedHeader(_out, _fileBase + ".ice");
}

Slice::Gen::~Gen()
{
    if(_out.isOpen() || _useStdout)
    {
        _out << '\n';
    }
}

void
Slice::Gen::generate(const UnitPtr& p)
{
    //
    // Check for "js:module:ice" metadata. If this
    // is set then we are building Ice.
    //
    ModuleList modules = p->modules();
    bool icejs = false;
    for(ModuleList::const_iterator i = modules.begin(); i != modules.end(); i++)
    {
        if(p->topLevelFile() == (*i)->definitionContext()->filename() &&
           getModuleMetadata(ContainedPtr::dynamicCast(*i)) == "ice")
        {
            icejs = true;
            break;
        }
    }

    //
    // Check for global "js:es6-module" metadata.
    //
    DefinitionContextPtr dc = p->findDefinitionContext(p->topLevelFile());
    assert(dc);
    StringList globalMetaData = dc->getMetaData();
    bool es6module = find(globalMetaData.begin(), globalMetaData.end(), "js:es6-module") != globalMetaData.end();

    if(_typeScript)
    {
        TypeScriptRequireVisitor requireVisitor(_out);
        p->visit(&requireVisitor, false);

        //
        // If at some point TypeScript adds an operator to refer to a type in the global scope
        // we can get rid of the TypeScriptAliasVisitor and use this. For now we need to generate
        // a type alias when there is an abiguity.
        // see: https://github.com/Microsoft/TypeScript/issues/983
        //
        TypeScriptAliasVisitor aliasVisitor(_out);
        p->visit(&aliasVisitor, false);
        aliasVisitor.writeAlias(p);

        TypeScriptVisitor typeScriptVisitor(_out, requireVisitor.imports());
        p->visit(&typeScriptVisitor, false);
    }
    else
    {
        _out << nl << "/* eslint-disable */";
        _out << nl << "/* jshint ignore: start */";
        _out << nl;

        if(!es6module)
        {
            if(icejs)
            {
                _out.zeroIndent();
                _out << nl << "/* slice2js browser-bundle-skip */";
                _out.restoreIndent();
            }
            _out << nl << "(function(module, require, exports)";
            _out << sb;
            if(icejs)
            {
                _out.zeroIndent();
                _out << nl << "/* slice2js browser-bundle-skip-end */";
                _out.restoreIndent();
            }
        }
        RequireVisitor requireVisitor(_out, _includePaths, icejs, es6module);
        p->visit(&requireVisitor, false);
        vector<string> seenModules = requireVisitor.writeRequires(p);

        TypesVisitor typesVisitor(_out, seenModules, icejs);
        p->visit(&typesVisitor, false);

        //
        // Export the top-level modules.
        //
        ExportVisitor exportVisitor(_out, icejs, es6module);
        p->visit(&exportVisitor, false);

        if(!es6module)
        {
            if(icejs)
            {
                _out.zeroIndent();
                _out << nl << "/* slice2js browser-bundle-skip */";
                _out.restoreIndent();
            }

            _out << eb;
            _out << nl << "(typeof(global) !== \"undefined\" && typeof(global.process) !== \"undefined\" ? module : undefined,"
                 << nl << " typeof(global) !== \"undefined\" && typeof(global.process) !== \"undefined\" ? require :"
                 << nl << " (typeof WorkerGlobalScope !== \"undefined\" && self instanceof WorkerGlobalScope) ? self.Ice._require : window.Ice._require,"
                 << nl << " typeof(global) !== \"undefined\" && typeof(global.process) !== \"undefined\" ? exports :"
                 << nl << " (typeof WorkerGlobalScope !== \"undefined\" && self instanceof WorkerGlobalScope) ? self : window));";

            if(icejs)
            {
                _out.zeroIndent();
                _out << nl << "/* slice2js browser-bundle-skip-end */";
                _out.restoreIndent();
            }
        }
    }
}

void
Slice::Gen::closeOutput()
{
    _out.close();
}

Slice::Gen::RequireVisitor::RequireVisitor(IceUtilInternal::Output& out, vector<string> includePaths,
                                           bool icejs, bool es6modules) :
    JsVisitor(out),
    _icejs(icejs),
    _es6modules(es6modules),
    _seenClass(false),
    _seenCompactId(false),
    _seenOperation(false),
    _seenStruct(false),
    _seenUserException(false),
    _seenLocalException(false),
    _seenEnum(false),
    _seenObjectSeq(false),
    _seenObjectDict(false),
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
    _seenClass = true; // Set regardless of whether p->isLocal()
    if(p->compactId() >= 0)
    {
        _seenCompactId = true;
    }
    return !p->isLocal(); // Look for operations.
}

bool
Slice::Gen::RequireVisitor::visitStructStart(const StructPtr&)
{
    _seenStruct = true; // Set regardless of whether p->isLocal()
    return false;
}

void
Slice::Gen::RequireVisitor::visitOperation(const OperationPtr&)
{
    _seenOperation = true;
}

bool
Slice::Gen::RequireVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    if(p->isLocal())
    {
        _seenLocalException = true;
    }
    else
    {
        _seenUserException = true;
    }

    return false;
}

void
Slice::Gen::RequireVisitor::visitSequence(const SequencePtr& seq)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
    if(builtin)
    {
        switch(builtin->kind())
        {
        case Builtin::KindObject:
            _seenObjectSeq = true;
            break;
        case Builtin::KindObjectProxy:
            _seenObjectProxySeq = true;
            break;
        default:
            break;
        }
    }
}

void
Slice::Gen::RequireVisitor::visitDictionary(const DictionaryPtr& dict)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(dict->valueType());
    if(builtin)
    {
        switch(builtin->kind())
        {
        case Builtin::KindObject:
            _seenObjectDict = true;
            break;
        case Builtin::KindObjectProxy:
            _seenObjectProxyDict = true;
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
        if(_seenClass || _seenObjectSeq || _seenObjectDict)
        {
            requires["Ice"].push_back("Ice/Object");
            requires["Ice"].push_back("Ice/Value");
        }
        if(_seenClass)
        {
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

        if(_seenLocalException || _seenUserException)
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
        _out << nl << "import { Ice } from \"ice\";";
        _out << nl << "const _ModuleRegistry = Ice._ModuleRegistry;";

        seenModules.push_back("Ice");

        for(StringList::const_iterator i = includes.begin(); i != includes.end(); ++i)
        {
            set<string> modules = p->getTopLevelModules(*i);
            vector<string> newModules;
            bool externals = false; // is there any external modules?
            for(set<string>::const_iterator j = modules.begin(); j != modules.end(); ++j)
            {
                if(find(seenModules.begin(), seenModules.end(), *j) == seenModules.end())
                {
                    seenModules.push_back(*j);
                    if(!_icejs && iceBuiltinModule(*j))
                    {
                        _out << nl << "import { " << *j << " } from \"ice\";";
                    }
                    else
                    {
                        newModules.push_back(*j);
                        externals = true;
                    }
                }
            }

            if(externals)
            {
                _out << nl << "import ";
                if(!newModules.empty())
                {
                    _out << "{ ";
                    for(vector<string>::const_iterator j = newModules.begin(); j != newModules.end();)
                    {
                        _out << *j;
                        ++j;
                        if(j != newModules.end())
                        {
                            _out << ", ";
                        }
                    }
                    _out << " } from ";
                }

                string result = relativePath(*i, p->topLevelFile());
                string::size_type pos;
                if((pos = result.rfind('.')) != string::npos)
                {
                    result.erase(pos);
                }
                _out << "\"" << result << "\";";
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
    //
    // Don't need to generate any code for local interfaces.
    //
    if(p->isInterface() && p->isLocal())
    {
        return false;
    }
    const string scope = p->scope();
    const string scoped = p->scoped();
    const string localScope = getLocalScope(scope);
    const string name = fixId(p->name());
    const string prxName = p->name() + "Prx";

    ClassList bases = p->bases();
    ClassDefPtr base;
    string baseRef;
    const bool hasBaseClass = !bases.empty() && !bases.front()->isInterface();
    if(hasBaseClass)
    {
        base = bases.front();
        bases.erase(bases.begin());
        baseRef = fixId(base->scoped());
    }
    else
    {
        baseRef = "Ice.Value";
    }

    const DataMemberList allDataMembers = p->allDataMembers();
    const DataMemberList dataMembers = p->dataMembers();
    const DataMemberList optionalMembers = p->orderedOptionalDataMembers();

    vector<string> allParamNames;
    for(DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
    {
        allParamNames.push_back(fixId((*q)->name()));
    }

    vector<string> baseParamNames;
    DataMemberList baseDataMembers;

    if(base)
    {
        baseDataMembers = base->allDataMembers();
        for(DataMemberList::const_iterator q = baseDataMembers.begin(); q != baseDataMembers.end(); ++q)
        {
            baseParamNames.push_back(fixId((*q)->name()));
        }
    }

    ClassList allBases = p->allBases();
    StringList ids;
    transform(allBases.begin(), allBases.end(), back_inserter(ids), ::IceUtil::constMemFun(&Contained::scoped));
    StringList other;
    other.push_back(scoped);
    other.push_back("::Ice::Object");
    other.sort();
    ids.merge(other);
    ids.unique();

    StringList::const_iterator firstIter = ids.begin();
    StringList::const_iterator scopedIter = find(ids.begin(), ids.end(), scoped);
    assert(scopedIter != ids.end());
    StringList::difference_type scopedPos = IceUtilInternal::distance(firstIter, scopedIter);

    if(!p->isLocal())
    {
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
    }

    if(!p->isInterface() || p->isLocal())
    {
        _out << sp;
        writeDocComment(p, getDeprecateReason(p, 0, "type"));
        _out << nl << localScope << '.' << name << " = class";
        if(!p->isLocal() || hasBaseClass)
        {
            _out << " extends " << baseRef;
        }

        _out << sb;
        if(!allParamNames.empty())
        {
            _out << nl << "constructor" << spar;
            for(DataMemberList::const_iterator q = baseDataMembers.begin(); q != baseDataMembers.end(); ++q)
            {
                _out << fixId((*q)->name());
            }

            for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
            {
                string value;
                if((*q)->optional())
                {
                    if((*q)->defaultValueType())
                    {
                        value = writeConstantValue(scope, (*q)->type(), (*q)->defaultValueType(), (*q)->defaultValue());
                    }
                    else
                    {
                        value = "undefined";
                    }
                }
                else
                {
                    if((*q)->defaultValueType())
                    {
                        value = writeConstantValue(scope, (*q)->type(), (*q)->defaultValueType(), (*q)->defaultValue());
                    }
                    else
                    {
                        value = getValue(scope, (*q)->type());
                    }
                }
                _out << (fixId((*q)->name()) + (value.empty() ? value : (" = " + value)));
            }

            _out << epar << sb;
            if(!p->isLocal() || hasBaseClass)
            {
                _out << nl << "super" << spar << baseParamNames << epar << ';';
            }
            writeInitDataMembers(dataMembers);
            _out << eb;

            if(!p->isLocal())
            {
                if(p->compactId() != -1)
                {
                    _out << sp;
                    _out << nl << "static get _iceCompactId()";
                    _out << sb;
                    _out << nl << "return " << p->compactId() << ";";
                    _out << eb;
                }

                if(!dataMembers.empty())
                {
                    _out << sp;
                    _out << nl << "_iceWriteMemberImpl(ostr)";
                    _out << sb;
                    writeMarshalDataMembers(dataMembers, optionalMembers);
                    _out << eb;

                    _out << sp;
                    _out << nl << "_iceReadMemberImpl(istr)";
                    _out << sb;
                    writeUnmarshalDataMembers(dataMembers, optionalMembers);
                    _out << eb;
                }
            }
        }
        _out << eb << ";";

        _out << sp;
        if(!p->isLocal())
        {
            bool preserved = p->hasMetaData("preserve-slice") && !p->inheritsMetaData("preserve-slice");

            _out << nl << "Slice.defineValue(" << localScope << "." << name << ", "
                 << "iceC_" << getLocalScope(scoped, "_") << "_ids[" << scopedPos << "], "
                 << (preserved ? "true" : "false") ;
            if(p->compactId() >= 0)
            {
                _out << ", " << p->compactId();
            }
            _out << ");";
        }
    }

    //
    // Define servant an proxy types for non local classes
    //
    if(!p->isLocal() && (p->isInterface() || !p->allOperations().empty()))
    {
        _out << sp;
        writeDocComment(p, getDeprecateReason(p, 0, "type"));
        _out << nl << localScope << "." << (p->isInterface() ? p->name() :  p->name() + "Disp") << " = class extends ";
        if(hasBaseClass && !base->allOperations().empty())
        {
            _out << getLocalScope(base->scope())  << "." << base->name() << "Disp";
        }
        else
        {
            _out << "Ice.Object";
        }
        _out << sb;

        if(!bases.empty())
        {
            _out << sp;
            _out << nl << "static get _iceImplements()";
            _out << sb;
            _out << nl << "return [";
            _out.inc();
            for(ClassList::const_iterator q = bases.begin(); q != bases.end();)
            {
                ClassDefPtr base = *q;
                if(base->isInterface())
                {
                    _out << nl << getLocalScope(base->scope()) << "." <<
                        (base->isInterface() ? base->name() : base->name() + "Disp");
                    if(++q != bases.end())
                    {
                        _out << ",";
                    }
                }
                else
                {
                    q++;
                }
            }
            _out.dec();
            _out << nl << "];";
            _out << eb;
        }
        _out << eb << ";";

        //
        // Generate a proxy class for interfaces or classes with operations.
        //
        string proxyType = "undefined";
        if(p->isInterface() || !p->allOperations().empty())
        {
            proxyType = localScope + '.' + prxName;
            string baseProxy = "Ice.ObjectPrx";
            if(!p->isInterface() && base && base->allOperations().size() > 0)
            {
                baseProxy = (getLocalScope(base->scope()) + "." + base->name() + "Prx");
            }

            _out << sp;
            _out << nl << proxyType << " = class extends " << baseProxy;
            _out << sb;

            if(!bases.empty())
            {
                _out << sp;
                _out << nl << "static get _implements()";
                _out << sb;
                _out << nl << "return [";

                _out.inc();
                for(ClassList::const_iterator q = bases.begin(); q != bases.end();)
                {
                    ClassDefPtr base = *q;
                    if(base->isInterface())
                    {
                        _out << nl << getLocalScope(base->scope()) << "." << base->name() << "Prx";
                        if(++q != bases.end())
                        {
                            _out << ",";
                        }
                    }
                    else
                    {
                        q++;
                    }
                }
                _out.dec();
                _out << "];";
                _out << eb;
            }

            _out << eb << ";";
        }

        _out << sp << nl << "Slice.defineOperations("
             << localScope << "." << (p->isInterface() ? p->name() : p->name() + "Disp") << ", "
             << proxyType << ", "
             << "iceC_" << getLocalScope(scoped, "_") << "_ids, "
             << scopedPos;

        const OperationList ops = p->operations();
        if(!ops.empty())
        {
            _out << ',';
            _out << sb;
            for(OperationList::const_iterator q = ops.begin(); q != ops.end(); ++q)
            {
                if(q != ops.begin())
                {
                    _out << ',';
                }

                OperationPtr op = *q;
                const string name = fixId(op->name());
                const ParamDeclList paramList = op->parameters();
                const TypePtr ret = op->returnType();
                ParamDeclList inParams, outParams;
                for(ParamDeclList::const_iterator pli = paramList.begin(); pli != paramList.end(); ++pli)
                {
                    if((*pli)->isOutParam())
                    {
                        outParams.push_back(*pli);
                    }
                    else
                    {
                        inParams.push_back(*pli);
                    }
                }

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

                if(name != op->name())
                {
                    _out << "\"" << name << "\""; // Native method name.
                }
                _out << ", ";

                if(op->mode() != Operation::Normal)
                {
                    _out << sliceModeToIceMode(op->mode()); // Mode.
                }
                _out << ", ";

                if(op->sendMode() != Operation::Normal)
                {
                    _out << sliceModeToIceMode(op->sendMode()); // Send mode.
                }
                _out << ", ";

                if(op->format() != DefaultFormat)
                {
                    _out << opFormatTypeToString(op); // Format.
                }
                _out << ", ";

                //
                // Return type.
                //
                if(ret)
                {
                    _out << '[' << encodeTypeForOperation(ret);
                    const bool isObj = isClassType(ret);
                    if(isObj)
                    {
                        _out << ", true";
                    }
                    if(op->returnIsOptional())
                    {
                        if(!isObj)
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
                if(!inParams.empty())
                {
                    _out << '[';
                    for(ParamDeclList::const_iterator pli = inParams.begin(); pli != inParams.end(); ++pli)
                    {
                        if(pli != inParams.begin())
                        {
                            _out << ", ";
                        }
                        TypePtr t = (*pli)->type();
                        _out << '[' << encodeTypeForOperation(t);
                        const bool isObj = isClassType(t);
                        if(isObj)
                        {
                            _out << ", true";
                        }
                        if((*pli)->optional())
                        {
                            if(!isObj)
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
                if(!outParams.empty())
                {
                    _out << '[';
                    for(ParamDeclList::const_iterator pli = outParams.begin(); pli != outParams.end(); ++pli)
                    {
                        if(pli != outParams.begin())
                        {
                            _out << ", ";
                        }
                        TypePtr t = (*pli)->type();
                        _out << '[' << encodeTypeForOperation(t);
                        const bool isObj = isClassType(t);
                        if(isObj)
                        {
                            _out << ", true";
                        }
                        if((*pli)->optional())
                        {
                            if(!isObj)
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
#if defined(__SUNPRO_CC)
                throws.sort(derivedToBaseCompare);
#else
                throws.sort(Slice::DerivedToBaseCompare());
#endif
                if(throws.empty())
                {
                    _out << " ";
                }
                else
                {
                    _out << nl << '[';
                    _out.inc();
                    for(ExceptionList::const_iterator eli = throws.begin(); eli != throws.end(); ++eli)
                    {
                        if(eli != throws.begin())
                        {
                            _out << ',';
                        }
                        _out << nl << fixId((*eli)->scoped());
                    }
                    _out.dec();
                    _out << nl << ']';
                }
                _out << ", ";

                if(op->sendsClasses(false))
                {
                    _out << "true";
                }
                _out << ", ";

                if(op->returnsClasses(false))
                {
                    _out << "true";
                }

                _out << ']';
            }
            _out << eb;
        }
        _out << ");";
    }
    return false;
}

void
Slice::Gen::TypesVisitor::visitSequence(const SequencePtr& p)
{
    const TypePtr type = p->type();

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
    if(isClassType(type))
    {
        bool isinterface =
            ClassDeclPtr::dynamicCast(type) &&
            ClassDeclPtr::dynamicCast(type)->definition() &&
            ClassDeclPtr::dynamicCast(type)->definition()->isInterface();
        _out<< ", \"" << (isinterface ? "Ice.Value" : typeToString(type)) << "\"";
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
        baseRef = p->isLocal() ? "Ice.LocalException" : "Ice.UserException";
    }

    const DataMemberList allDataMembers = p->allDataMembers();
    const DataMemberList dataMembers = p->dataMembers();
    const DataMemberList optionalMembers = p->orderedOptionalDataMembers();

    vector<string> allParamNames;
    for(DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
    {
        allParamNames.push_back(fixId((*q)->name()));
    }

    vector<string> baseParamNames;
    DataMemberList baseDataMembers;

    if(p->base())
    {
        baseDataMembers = p->base()->allDataMembers();
        for(DataMemberList::const_iterator q = baseDataMembers.begin(); q != baseDataMembers.end(); ++q)
        {
            baseParamNames.push_back(fixId((*q)->name()));
        }
    }

    _out << sp;
    writeDocComment(p, getDeprecateReason(p, 0, "type"));
    _out << nl << localScope << '.' << name << " = class extends " << baseRef;
    _out << sb;

    _out << nl << "constructor" << spar;

    for(DataMemberList::const_iterator q = baseDataMembers.begin(); q != baseDataMembers.end(); ++q)
    {
        _out << fixId((*q)->name());
    }

    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string value;
        if((*q)->optional())
        {
            if((*q)->defaultValueType())
            {
                value = writeConstantValue(scope, (*q)->type(), (*q)->defaultValueType(), (*q)->defaultValue());
            }
            else
            {
                value = "undefined";
            }
        }
        else
        {
            if((*q)->defaultValueType())
            {
                value = writeConstantValue(scope, (*q)->type(), (*q)->defaultValueType(), (*q)->defaultValue());
            }
            else
            {
                value = getValue(scope, (*q)->type());
            }
        }
        _out << (fixId((*q)->name()) + (value.empty() ? value : (" = " + value)));
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

    if(!p->isLocal())
    {
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
            writeMarshalDataMembers(dataMembers, optionalMembers);
            _out << eb;

            _out << sp;
            _out << nl << "_readMemberImpl(istr)";
            _out << sb;
            writeUnmarshalDataMembers(dataMembers, optionalMembers);
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
    }

    _out << eb << ";";

    bool basePreserved = p->inheritsMetaData("preserve-slice");
    bool preserved = p->hasMetaData("preserve-slice");

    if(!p->isLocal() && preserved && !basePreserved)
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

    const DataMemberList dataMembers = p->dataMembers();

    vector<string> paramNames;
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        paramNames.push_back(fixId((*q)->name()));
    }

    _out << sp;
    writeDocComment(p, getDeprecateReason(p, 0, "type"));
    _out << nl << localScope << '.' << name << " = class";
    _out << sb;

    _out << nl << "constructor" << spar;

    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string value;
        if((*q)->optional())
        {
            if((*q)->defaultValueType())
            {
                value = writeConstantValue(scope, (*q)->type(), (*q)->defaultValueType(), (*q)->defaultValue());
            }
            else
            {
                value = "undefined";
            }
        }
        else
        {
            if((*q)->defaultValueType())
            {
                value = writeConstantValue(scope, (*q)->type(), (*q)->defaultValueType(), (*q)->defaultValue());
            }
            else
            {
                value = getValue(scope, (*q)->type());
            }
        }
        _out << (fixId((*q)->name()) + (value.empty() ? value : (" = " + value)));
    }

    _out << epar;
    _out << sb;
    writeInitDataMembers(dataMembers);
    _out << eb;

    if(!p->isLocal())
    {
        _out << sp;
        _out << nl << "_write(ostr)";
        _out << sb;
        writeMarshalDataMembers(dataMembers, DataMemberList());
        _out << eb;

        _out << sp;
        _out << nl << "_read(istr)";
        _out << sb;
        writeUnmarshalDataMembers(dataMembers, DataMemberList());
        _out << eb;

        _out << sp;
        _out << nl << "static get minWireSize()";
        _out << sb;
        _out << nl << "return  " << p->minWireSize() << ";";
        _out << eb;
    }

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
    if((b && b->kind() == Builtin::KindLong) || StructPtr::dynamicCast(keyType))
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

    bool isinterface =
            ClassDeclPtr::dynamicCast(valueType) &&
            ClassDeclPtr::dynamicCast(valueType)->definition() &&
            ClassDeclPtr::dynamicCast(valueType)->definition()->isInterface();

    _out << sp;
    _out << nl << "Slice.defineDictionary(" << scope << ", \"" << name << "\", \"" << propertyName << "\", "
         << "\"" << getHelper(keyType) << "\", "
         << "\"" << (isinterface ? "Ice.Value" : getHelper(valueType)) << "\", "
         << (fixed ? "true" : "false") << ", "
         << (keyUseEquals ? "Ice.HashMap.compareEquals" : "undefined");

    if(isClassType(valueType))
    {
        _out << ", \"" << typeToString(valueType) << "\"";
    }
    else
    {
        _out << ", undefined";
    }

    if(SequencePtr::dynamicCast(valueType))
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
    writeDocComment(p, getDeprecateReason(p, 0, "type"));
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
Slice::Gen::TypesVisitor::encodeTypeForOperation(const TypePtr& type)
{
    assert(type);

    static const char* builtinTable[] =
    {
        "0",  // byte
        "1",  // bool
        "2",  // short
        "3",  // int
        "4",  // long
        "5",  // float
        "6",  // double
        "7",  // string
        "8",  // Ice.Object
        "9",  // Ice.ObjectPrx
        "??", // LocalObject
        "10", // Ice.Value
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        return builtinTable[builtin->kind()];
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        ClassDefPtr def = proxy->_class()->definition();
        if(def->isInterface() || def->allOperations().size() > 0)
        {
            return "\"" + fixId(proxy->_class()->scoped() + "Prx") + "\"";
        }
        else
        {
            return "Ice.ObjectPrx";
        }
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
        if(cl->isInterface())
        {
            return "\"Ice.Value\"";
        }
        else
        {
            return "\"" + fixId(cl->scoped()) + "\"";
        }
    }

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

Slice::Gen::TypeScriptRequireVisitor::TypeScriptRequireVisitor(IceUtilInternal::Output& out) :
    JsVisitor(out),
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
Slice::Gen::TypeScriptRequireVisitor::addImport(const TypePtr& definition, const ContainedPtr& toplevel)
{
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
    if(UnitPtr::dynamicCast(p->container()))
    {
        const string prefix = "js:module:";
        string m;
        findMetaData(prefix, p->getMetaData(), m);
        if(_imports.empty() && m != "ice")
        {
            _imports.push_back(make_pair("ice", nextImportPrefix()));
        }
    }
    return true;
}

bool
Slice::Gen::TypeScriptRequireVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    //
    // Add imports required for base classes
    //
    ClassList bases = p->bases();
    for(ClassList::const_iterator i = bases.begin(); i != bases.end(); ++i)
    {
        addImport(ContainedPtr::dynamicCast(*i), p);
    }

    //
    // Add imports required for data members
    //
    const DataMemberList allDataMembers = p->allDataMembers();
    for(DataMemberList::const_iterator i = allDataMembers.begin(); i != allDataMembers.end(); ++i)
    {
        addImport((*i)->type(), p);
    }

    //
    // Add imports required for operation parameters and return type
    //
    const OperationList operationList = p->allOperations();
    for(OperationList::const_iterator i = operationList.begin(); i != operationList.end(); ++i)
    {
        const TypePtr ret = (*i)->returnType();
        if(ret && ret->definitionContext())
        {
            addImport(ret, p);
        }

        const ParamDeclList paramList = (*i)->parameters();
        for(ParamDeclList::const_iterator j = paramList.begin(); j != paramList.end(); ++j)
        {
            addImport((*j)->type(), p);
        }
    }
    return false;
}

bool
Slice::Gen::TypeScriptRequireVisitor::visitStructStart(const StructPtr& p)
{
    //
    // Add imports required for data members
    //
    const DataMemberList dataMembers = p->dataMembers();
    for(DataMemberList::const_iterator i = dataMembers.begin(); i != dataMembers.end(); ++i)
    {
        addImport((*i)->type(), p);
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

    //
    // Add imports required for data members
    //
    const DataMemberList allDataMembers = p->allDataMembers();
    for(DataMemberList::const_iterator i = allDataMembers.begin(); i != allDataMembers.end(); ++i)
    {
        addImport((*i)->type(), p);
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
    // Add alias required for base classes
    //
    ClassList bases = p->bases();
    for(ClassList::const_iterator i = bases.begin(); i != bases.end(); ++i)
    {
        addAlias(TypePtr::dynamicCast((*i)->declaration()), module);
    }

    //
    // Add alias required for data members
    //
    const DataMemberList allDataMembers = p->allDataMembers();
    for(DataMemberList::const_iterator i = allDataMembers.begin(); i != allDataMembers.end(); ++i)
    {
        addAlias((*i)->type(), module);
    }

    //
    // Add alias required for operation parameters
    //
    const OperationList operationList = p->allOperations();
    for(OperationList::const_iterator i = operationList.begin(); i != operationList.end(); ++i)
    {
        const TypePtr ret = (*i)->returnType();
        if(ret && ret->definitionContext())
        {
            addAlias(ret, module);
        }

        const ParamDeclList paramList = (*i)->parameters();
        for(ParamDeclList::const_iterator j = paramList.begin(); j != paramList.end(); ++j)
        {
            addAlias((*j)->type(), module);
        }
    }
    return false;
}

bool
Slice::Gen::TypeScriptAliasVisitor::visitStructStart(const StructPtr& p)
{
    ModulePtr module = ModulePtr::dynamicCast(p->container());
    //
    // Add alias required for data members
    //
    const DataMemberList dataMembers = p->dataMembers();
    for(DataMemberList::const_iterator i = dataMembers.begin(); i != dataMembers.end(); ++i)
    {
        addAlias((*i)->type(), module);
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

    //
    // Add alias required for data members
    //
    const DataMemberList allDataMembers = p->allDataMembers();
    for(DataMemberList::const_iterator i = allDataMembers.begin(); i != allDataMembers.end(); ++i)
    {
        addAlias((*i)->type(), module);
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
        string module = getModuleMetadata(ContainedPtr::dynamicCast(p));

        _out << sp;
        if(module.empty())
        {
            writeImports();
            _out << nl << "export namespace " << fixId(p->name()) << sb;
        }
        else
        {
            _out << nl << "declare module \"" << fixId(module) << "\"" << sb;
            writeImports();
            _out << nl << "namespace " << fixId(p->name()) << sb;
        }
    }
    else
    {
        _out << nl << "namespace " << fixId(p->name()) << sb;
    }
    return true;
}

void
Slice::Gen::TypeScriptVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb; // namespace end

    if(UnitPtr::dynamicCast(p->container()))
    {
        string module = getModuleMetadata(ContainedPtr::dynamicCast(p));
        if(!module.empty())
        {
            _out << eb; // module end
        }
    }
}

bool
Slice::Gen::TypeScriptVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    const string toplevelModule = getModuleMetadata(ContainedPtr::dynamicCast(p));
    const string icePrefix = importPrefix("Ice.", p);
    if(p->isDelegate())
    {
        // A delegate only has one operation
        OperationPtr op = p->allOperations().front();
        CommentPtr comment = op->parseComment(false);
        if(comment)
        {
            writeOpDocSummary(_out, op, comment, OpDocAllParams, true, StringList(), StringList(), comment->returns());
        }
        _out << nl << "type " << fixId(p->name()) << " = " << spar;
        ParamDeclList paramList = op->parameters();
        for(ParamDeclList::iterator q = paramList.begin(); q != paramList.end(); ++q)
        {
            ostringstream os;
            os << fixId((*q)->name()) << ":";
            if((*q)->isOutParam())
            {
                os << icePrefix << getUnqualified("Ice.Holder", p->scope(), icePrefix) << "<";
            }
            os << typeToString((*q)->type(), p, imports(), true, false, true);
            if((*q)->isOutParam())
            {
                os << ">";
            }
            _out << os.str();
        }
        _out << epar << " => ";
        TypePtr ret = op->returnType();
        if(ret)
        {
            _out << typeToString(ret, p, imports(), true, false, true) << ";";
        }
        else
        {
            _out << "void;";
        }
    }
    else if(p->isLocal())
    {
        const ClassList bases = p->bases();
        const DataMemberList dataMembers = p->dataMembers();
        const DataMemberList allDataMembers = p->allDataMembers();

        _out << sp;
        writeDocSummary(_out, p);
        _out << nl;
        _out << (p->isInterface() ? "interface" : "class") << " " << fixId(p->name());
        if(!bases.empty() && !bases.front()->isInterface())
        {
            const string prefix = importPrefix(ContainedPtr::dynamicCast(bases.front()), p, imports());
            _out << " extends " << prefix << getUnqualified(fixId(bases.front()->scoped()), p->scope(), prefix);
        }
        _out << sb;
        if(!p->isInterface())
        {
            _out << nl << "/**";
            _out << nl << " * One-shot constructor to initialize all data members.";
            for(DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
            {
                CommentPtr comment = (*q)->parseComment(false);
                if(comment)
                {
                    _out << nl << " * @param " << fixId((*q)->name()) << " " << getDocSentence(comment->overview());
                }
            }
            _out << nl << " */";
            _out << nl << "constructor" << spar;
            for(DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
            {
                _out << (fixId((*q)->name()) + "?:" + typeToString((*q)->type(), p, imports(), true, false, true));
            }
            _out << epar << ";";

            for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
            {
                writeDocSummary(_out, *q);
                _out << nl << fixId((*q)->name()) << ":" << typeToString((*q)->type(), p, imports(), true, false, true)
                     << ";";
            }
        }

        OperationList allOperations = p->allOperations();
        for(OperationList::const_iterator q = allOperations.begin(); q != allOperations.end(); ++q)
        {
            OperationPtr op = *q;
            ParamDeclList params = op->parameters();
            ParamDeclList inParams, outParams;
            for(ParamDeclList::const_iterator r = params.begin(); r != params.end(); ++r)
            {
                if((*r)->isOutParam())
                {
                    outParams.push_back(*r);
                }
                else
                {
                    inParams.push_back(*r);
                }
            }

            TypePtr ret = op->returnType();
            bool async = op->hasMetaData("js:async") || op->hasMetaData("async-oneway");
            CommentPtr comment = op->parseComment(false);
            if(comment)
            {
                StringList returns;
                if(async)
                {
                    returns.push_back("@returns The asynchronous result object for the invocation.");
                }
                else if(ret)
                {
                    returns = comment->returns();
                }
                writeOpDocSummary(_out, op, comment, OpDocAllParams, true, StringList(), StringList(), returns);
            }

            _out << nl << fixId((*q)->name()) << spar;
            for(ParamDeclList::const_iterator r = inParams.begin(); r != inParams.end(); r++)
            {
                _out << (fixId((*r)->name()) + ":" + typeToString((*r)->type(), p, imports(), true, false, true));
            }

            for(ParamDeclList::const_iterator r = outParams.begin(); r != outParams.end(); r++)
            {
                const string prefix = importPrefix("Ice.Holder", p);
                _out << (fixId((*r)->name()) + ":"  + prefix +
                         getUnqualified("Ice.Holder", p->scope(), prefix) + "<" +
                         typeToString((*r)->type(), p, imports(), true, false, true) + ">");
            }

            _out << epar;

            _out << ":";
            if(async)
            {
               _out << icePrefix << getUnqualified("Ice.AsyncResultBase", p->scope(), icePrefix) << "<";
            }

            if(ret)
            {
                _out << typeToString(ret, p, imports(), true, false, true);
            }
            else
            {
                _out << "void";
            }

            if (async)
            {
                _out << ">";
            }
            _out << ";";
        }

        if(p->hasMetaData("js:comparable"))
        {
            _out << nl << "equals(rhs:any):boolean";
        }
        _out << eb;
    }
    else
    {
        const OperationList ops = p->allOperations();
        if(p->isInterface() || !ops.empty())
        {
            //
            // Define servant an proxy types for non local classes
            //
            _out << sp;
            _out << nl << "abstract class " << fixId(p->name() + "Prx")
                 << " extends " << icePrefix << getUnqualified("Ice.ObjectPrx", p->scope(), icePrefix);
            _out << sb;

            for(OperationList::const_iterator q = ops.begin(); q != ops.end(); ++q)
            {
                const OperationPtr op = *q;
                const ParamDeclList paramList = op->parameters();
                const TypePtr ret = op->returnType();
                ParamDeclList inParams, outParams;
                for(ParamDeclList::const_iterator r = paramList.begin(); r != paramList.end(); ++r)
                {
                    if((*r)->isOutParam())
                    {
                        outParams.push_back(*r);
                    }
                    else
                    {
                        inParams.push_back(*r);
                    }
                }

                const string contextParam = escapeParam(paramList, "context");
                CommentPtr comment = op->parseComment(false);
                const string contextDoc = "@param " + contextParam + " The Context map to send with the invocation.";
                const string asyncDoc = "The asynchronous result object for the invocation.";
                if(comment)
                {
                    StringList postParams, returns;
                    postParams.push_back(contextDoc);
                    returns.push_back(asyncDoc);
                    writeOpDocSummary(_out, op, comment, OpDocInParams, false, StringList(), postParams, returns);
                }
                _out << nl << fixId((*q)->name()) << spar;
                for(ParamDeclList::const_iterator r = inParams.begin(); r != inParams.end(); ++r)
                {
                    _out << (fixId((*r)->name()) +
                             ((*r)->optional() ? "?" : "") +
                             ":" +
                             typeToString((*r)->type(), p, imports(), true, false, true));
                }
                _out << "context?:Map<string, string>";
                _out << epar;

                _out << ":" << icePrefix << getUnqualified("Ice.AsyncResult", p->scope(), icePrefix);
                if(!ret && outParams.empty())
                {
                    _out << "<void>";
                }
                else if((ret && outParams.empty()) || (!ret && outParams.size() == 1))
                {
                    TypePtr t = ret ? ret : outParams.front()->type();
                    _out << "<" << typeToString(t, p, imports(), true, false, true) << ">";
                }
                else
                {
                    _out << "<[";
                    if(ret)
                    {
                        _out << typeToString(ret, p, imports(), true, false, true) << ", ";
                    }

                    for(ParamDeclList::const_iterator i = outParams.begin(); i != outParams.end();)
                    {
                        _out << typeToString((*i)->type(), p, imports(), true, false, true);
                        if(++i != outParams.end())
                        {
                            _out << ", ";
                        }
                    }

                    _out << "]>";
                }

                _out << ";";
            }

            const string icePrefix = importPrefix("Ice.ObjectPrx", p);
            _out << sp;
            _out << nl << "/**";
            _out << nl << " * Downcasts a proxy without confirming the target object's type via a remote invocation.";
            _out << nl << " * @param prx The target proxy.";
            _out << nl << " * @return A proxy with the requested type.";
            _out << nl << " */";
            _out << nl << "static uncheckedCast(prx:" << icePrefix
                 << getUnqualified("Ice.ObjectPrx", p->scope(), icePrefix) << ", "
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
            _out << nl << "static checkedCast(prx:" << icePrefix
                 << getUnqualified("Ice.ObjectPrx", p->scope(), icePrefix) << ", "
                 << "facet?:string, contex?:Map<string, string>):" << icePrefix
                 << getUnqualified("Ice.AsyncResult", p->scope(), icePrefix) << "<" << fixId(p->name() + "Prx") << ">;";
            _out << eb;

            _out << sp;
            _out << nl << "abstract class " << fixId(p->name() + (p->isInterface() ? "" : "Disp"))
                 << " extends " << icePrefix << getUnqualified("Ice.Object", p->scope(), icePrefix);
            _out << sb;
            for(OperationList::const_iterator q = ops.begin(); q != ops.end(); ++q)
            {
                const OperationPtr op = *q;
                const ParamDeclList paramList = op->parameters();
                const TypePtr ret = op->returnType();
                ParamDeclList inParams, outParams;
                for(ParamDeclList::const_iterator r = paramList.begin(); r != paramList.end(); ++r)
                {
                    if((*r)->isOutParam())
                    {
                        outParams.push_back(*r);
                    }
                    else
                    {
                        inParams.push_back(*r);
                    }
                }

                const string currentParam = escapeParam(inParams, "current");
                CommentPtr comment = p->parseComment(false);
                const string currentDoc = "@param " + currentParam + " The Current object for the invocation.";
                const string resultDoc = "The result or a promise like object that will "
                    "be resolved with the result of the invocation.";
                if(comment)
                {
                    StringList postParams, returns;
                    postParams.push_back(currentDoc);
                    returns.push_back(resultDoc);
                    writeOpDocSummary(_out, op, comment, OpDocInParams, false, StringList(), postParams, returns);
                }
                _out << nl << "abstract " << fixId((*q)->name()) << spar;
                for(ParamDeclList::const_iterator r = inParams.begin(); r != inParams.end(); ++r)
                {
                    _out << (fixId((*r)->name()) + ":" + typeToString((*r)->type(), p, imports(), true, false, true));
                }
                _out << ("current:" + icePrefix + getUnqualified("Ice.Current", p->scope(), icePrefix));
                _out << epar << ":";

                if(!ret && outParams.empty())
                {
                    _out << "PromiseLike<void>|void";
                }
                else if((ret && outParams.empty()) || (!ret && outParams.size() == 1))
                {
                    TypePtr t = ret ? ret : outParams.front()->type();
                    string returnType = typeToString(t, p, imports(), true, false, true);
                    _out << "PromiseLike<" << returnType  << ">|" << returnType;
                }
                else
                {
                    ostringstream os;
                    if(ret)
                    {
                        os << typeToString(ret, p, imports(), true, false, true) << ", ";
                    }

                    for(ParamDeclList::const_iterator i = outParams.begin(); i != outParams.end();)
                    {
                        os << typeToString((*i)->type(), p, imports(), true, false, true);
                        if(++i != outParams.end())
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
        }

        if(!p->isInterface())
        {
            const DataMemberList dataMembers = p->dataMembers();
            const DataMemberList allDataMembers = p->allDataMembers();
            _out << sp;
            writeDocSummary(_out, p);
            _out << nl << "class " << fixId(p->name()) << " extends ";
            const string scope = p->scope();
            const string scoped = p->scoped();
            ClassList bases = p->bases();
            if(!bases.empty() && !bases.front()->isInterface())
            {
                ClassDefPtr base = bases.front();
                const string prefix = importPrefix(ContainedPtr::dynamicCast(base), p, imports());
                _out << prefix << getUnqualified(fixId(base->scoped()), p->scope(), prefix);
            }
            else
            {
                _out << icePrefix << getUnqualified("Ice.Value", p->scope(), icePrefix);
            }
            _out << sb;
            _out << nl << "/**";
            _out << nl << " * One-shot constructor to initialize all data members.";
            for(DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
            {
                CommentPtr comment = (*q)->parseComment(false);
                if(comment)
                {
                    _out << nl << " * @param " << fixId((*q)->name()) << " " << getDocSentence(comment->overview());
                }
            }
            _out << nl << " */";
            _out << nl << "constructor" << spar;
            for(DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
            {
                _out << (fixId((*q)->name()) + "?:" + typeToString((*q)->type(), p, imports(), true, false, true));
            }
            _out << epar << ";";
            for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
            {
                writeDocSummary(_out, *q);
                _out << nl << fixId((*q)->name()) << ":" << typeToString((*q)->type(), p, imports(), true, false, true)
                     << ";";
            }
            _out << eb;
        }
    }
    return false;
}

bool
Slice::Gen::TypeScriptVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    const string name = fixId(p->name());
    const DataMemberList dataMembers = p->dataMembers();
    const DataMemberList allDataMembers = p->allDataMembers();
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
        baseRef = p->isLocal() ?
            icePrefix + getUnqualified("Ice.LocalException", p->scope(), icePrefix) :
            icePrefix + getUnqualified("Ice.UserException", p->scope(), icePrefix);
    }

    _out << sp;
    writeDocSummary(_out, p);
    _out << nl << "class " << name << " extends " << baseRef << sb;
    if(!allDataMembers.empty())
    {
        _out << nl << "/**";
        _out << nl << " * One-shot constructor to initialize all data members.";
        for(DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
        {
            CommentPtr comment = (*q)->parseComment(false);
            if(comment)
            {
                _out << nl << " * @param " << fixId((*q)->name()) << " " << getDocSentence(comment->overview());
            }
        }
        _out << nl << " */";
        _out << nl << "constructor" << spar;
        for(DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
        {
            _out << (fixId((*q)->name()) + "?:" + typeToString((*q)->type(), p, imports(), true, false, true));
        }
        _out << epar << ";";
    }
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        _out << nl << fixId((*q)->name()) << ":" << typeToString((*q)->type(), p, imports(), true, false, true) << ";";
    }
    _out << eb;
    return false;
}

bool
Slice::Gen::TypeScriptVisitor::visitStructStart(const StructPtr& p)
{
    const string icePrefix = importPrefix("Ice.", p);
    const string name = fixId(p->name());
    const DataMemberList dataMembers = p->dataMembers();
    const string toplevelModule = getModuleMetadata(ContainedPtr::dynamicCast(p));
    _out << sp;
    writeDocSummary(_out, p);
    _out << nl << "class " << name << sb;
    _out << nl << "constructor" << spar;
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        _out << (fixId((*q)->name()) + "?:" + typeToString((*q)->type(), p, imports(), true, false, true));
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

    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        _out << nl << fixId((*q)->name()) << ":" << typeToString((*q)->type(), p, imports(), true, false, true) << ";";
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
