//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Gen.h"
#include "../Ice/Endian.h"
#include "../Slice/FileTracker.h"
#include "../Slice/Util.h"
#include "IceUtil/FileUtil.h"
#include "IceUtil/StringUtil.h"
#include "IceUtil/UUID.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

namespace
{
    string sliceModeToIceMode(Operation::Mode opMode)
    {
        switch (opMode)
        {
            case Operation::Normal:
                return "0";
            case Operation::Idempotent:
                return "2";
            default:
                assert(false);
        }

        return "???";
    }

    string opFormatTypeToString(const OperationPtr& op)
    {
        switch (op->format())
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

    void printHeader(IceUtilInternal::Output& out)
    {
        static const char* header = "//\n"
                                    "// Copyright (c) ZeroC, Inc. All rights reserved.\n"
                                    "//\n";

        out << header;
        out << "//\n";
        out << "// Ice version " << ICE_STRING_VERSION << "\n";
        out << "//\n";
    }

    string escapeParam(const ParamDeclList& params, const string& name)
    {
        string r = name;
        for (ParamDeclList::const_iterator p = params.begin(); p != params.end(); ++p)
        {
            if (Slice::JsGenerator::fixId((*p)->name()) == name)
            {
                r = name + "_";
                break;
            }
        }
        return r;
    }

    void writeDocLines(Output& out, const StringList& lines, bool commentFirst, const string& space = " ")
    {
        StringList l = lines;
        if (!commentFirst)
        {
            out << l.front();
            l.pop_front();
        }
        for (StringList::const_iterator i = l.begin(); i != l.end(); ++i)
        {
            out << nl << " *";
            if (!i->empty())
            {
                out << space << *i;
            }
        }
    }

    void writeSeeAlso(Output& out, const StringList& lines, const string& space = " ")
    {
        for (StringList::const_iterator i = lines.begin(); i != lines.end(); ++i)
        {
            out << nl << " *";
            if (!i->empty())
            {
                out << space << "@see " << *i;
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
        for (ParamDeclList::iterator p = params.begin(); p != params.end(); ++p)
        {
            map<string, StringList>::iterator q = paramDoc.find((*p)->name());
            if (q != paramDoc.end())
            {
                out << nl << " * @param " << Slice::JsGenerator::fixId(q->first) << " ";
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

        if (!doc->overview().empty())
        {
            writeDocLines(out, doc->overview(), true);
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

        out << nl << " */";
    }
}

Slice::JsVisitor::JsVisitor(Output& out, const vector<pair<string, string>>& imports) : _out(out), _imports(imports) {}

Slice::JsVisitor::~JsVisitor() {}

vector<pair<string, string>>
Slice::JsVisitor::imports() const
{
    return _imports;
}

void
Slice::JsVisitor::writeMarshalDataMembers(const DataMemberList& dataMembers, const DataMemberList& optionalMembers)
{
    for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        if (!(*q)->optional())
        {
            writeMarshalUnmarshalCode(_out, (*q)->type(), "this." + fixId((*q)->name()), true);
        }
    }

    for (DataMemberList::const_iterator q = optionalMembers.begin(); q != optionalMembers.end(); ++q)
    {
        writeOptionalMarshalUnmarshalCode(_out, (*q)->type(), "this." + fixId((*q)->name()), (*q)->tag(), true);
    }
}

void
Slice::JsVisitor::writeUnmarshalDataMembers(const DataMemberList& dataMembers, const DataMemberList& optionalMembers)
{
    for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        if (!(*q)->optional())
        {
            writeMarshalUnmarshalCode(_out, (*q)->type(), "this." + fixId((*q)->name()), false);
        }
    }

    for (DataMemberList::const_iterator q = optionalMembers.begin(); q != optionalMembers.end(); ++q)
    {
        writeOptionalMarshalUnmarshalCode(_out, (*q)->type(), "this." + fixId((*q)->name()), (*q)->tag(), false);
    }
}

void
Slice::JsVisitor::writeInitDataMembers(const DataMemberList& dataMembers)
{
    for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        const string m = fixId((*q)->name());
        _out << nl << "this." << m << " = " << m << ';';
    }
}

string
Slice::JsVisitor::getValue(const string& /*scope*/, const TypePtr& type)
{
    assert(type);

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        switch (builtin->kind())
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

    EnumPtr en = dynamic_pointer_cast<Enum>(type);
    if (en)
    {
        return fixId(en->scoped()) + '.' + fixId((*en->enumerators().begin())->name());
    }

    StructPtr st = dynamic_pointer_cast<Struct>(type);
    if (st)
    {
        return "new " + typeToString(type) + "()";
    }

    return "null";
}

string
Slice::JsVisitor::writeConstantValue(
    const string& /*scope*/,
    const TypePtr& type,
    const SyntaxTreeBasePtr& valueType,
    const string& value)
{
    ostringstream os;
    ConstPtr constant = dynamic_pointer_cast<Const>(valueType);
    if (constant)
    {
        os << fixId(constant->scoped());
    }
    else
    {
        BuiltinPtr bp = dynamic_pointer_cast<Builtin>(type);
        EnumPtr ep;
        if (bp && bp->kind() == Builtin::KindString)
        {
            //
            // For now, we generate strings in ECMAScript 5 format, with two \unnnn for astral characters
            //
            os << "\"" << toStringLiteral(value, "\b\f\n\r\t\v", "", ShortUCN, 0) << "\"";
        }
        else if (bp && bp->kind() == Builtin::KindLong)
        {
            // It should never fail as the Slice parser has already validated the value.
            int64_t l = std::stoll(value, nullptr, 0);

            //
            // JavaScript doesn't support 64 bit integer so long types are written as
            // two 32 bit words hi, low wrapped in the Ice.Long class.
            //
            // If slice2js runs in a big endian machine we need to swap the words, we do not
            // need to swap the word bytes as we just write each word as a number to the
            // output file.
            //
            if constexpr (endian::native == endian::big)
            {
                os << "new Ice.Long(" << (l & 0xFFFFFFFF) << ", " << ((l >> 32) & 0xFFFFFFFF) << ")";
            }
            else
            {
                os << "new Ice.Long(" << ((l >> 32) & 0xFFFFFFFF) << ", " << (l & 0xFFFFFFFF) << ")";
            }
        }
        else if ((ep = dynamic_pointer_cast<Enum>(type)))
        {
            EnumeratorPtr lte = dynamic_pointer_cast<Enumerator>(valueType);
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
    while ((nextPos = comment.find_first_of('\n', pos)) != string::npos)
    {
        result.push_back(string(comment, pos, nextPos - pos));
        pos = nextPos + 1;
    }
    string lastLine = string(comment, pos);
    if (lastLine.find_first_not_of(" \t\n\r") != string::npos)
    {
        result.push_back(lastLine);
    }

    return result;
}

void
Slice::JsVisitor::writeDocCommentFor(const ContainedPtr& p)
{
    StringList lines = splitComment(p);
    bool isDeprecated = p->isDeprecated(false);

    if (lines.empty() && !isDeprecated)
    {
        // There's nothing to write for this doc-comment.
        return;
    }

    _out << nl << "/**";

    for (const auto& line : lines)
    {
        //
        // @param must precede @return, so emit any extra parameter
        // when @return is seen.
        //
        if (line.empty())
        {
            _out << nl << " *";
        }
        else
        {
            _out << nl << " * " << line;
        }
    }

    if (isDeprecated)
    {
        _out << nl << " * @deprecated";
        if (auto reason = p->getDeprecationReason(false))
        {
            // If a reason was supplied, append it after the `@deprecated` tag.
            _out << " " << *reason;
        }
    }

    _out << nl << " **/";
}

Slice::Gen::Gen(const string& base, const vector<string>& includePaths, const string& dir, bool typeScript)
    : _includePaths(includePaths),
      _useStdout(false),
      _typeScript(typeScript)
{
    _fileBase = base;

    string::size_type pos = base.find_last_of("/\\");
    if (pos != string::npos)
    {
        _fileBase = base.substr(pos + 1);
    }

    string file = _fileBase + ".js";

    if (!dir.empty())
    {
        file = dir + '/' + file;
    }

    _jsout.open(file.c_str());
    if (!_jsout)
    {
        ostringstream os;
        os << "cannot open `" << file << "': " << IceUtilInternal::errorToString(errno);
        throw FileException(__FILE__, __LINE__, os.str());
    }
    FileTracker::instance()->addFile(file);

    if (typeScript)
    {
        file = _fileBase + ".d.ts";

        if (!dir.empty())
        {
            file = dir + '/' + file;
        }

        _tsout.open(file.c_str());
        if (!_tsout)
        {
            ostringstream os;
            os << "cannot open `" << file << "': " << IceUtilInternal::errorToString(errno);
            throw FileException(__FILE__, __LINE__, os.str());
        }
        FileTracker::instance()->addFile(file);
    }
}

Slice::Gen::Gen(
    const string& base,
    const vector<string>& includePaths,
    const string& /*dir*/,
    bool typeScript,
    ostream& out)
    : _jsout(out),
      _tsout(out),
      _includePaths(includePaths),
      _useStdout(true),
      _typeScript(typeScript)
{
    _fileBase = base;
    string::size_type pos = base.find_last_of("/\\");
    if (pos != string::npos)
    {
        _fileBase = base.substr(pos + 1);
    }
}

Slice::Gen::~Gen()
{
    if (_jsout.isOpen() || _useStdout)
    {
        _jsout << '\n';
        _jsout.close();
    }

    if (_tsout.isOpen() || _useStdout)
    {
        _tsout << '\n';
        _tsout.close();
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
        const string m = dc->findMetaData(prefix);
        if (!m.empty())
        {
            module = m.substr(prefix.size());
        }
    }

    if (_useStdout)
    {
        _jsout << "\n";
        _jsout << "/** slice2js: " << _fileBase << ".js generated begin module:\"" << module << "\" **/";
        _jsout << "\n";
    }
    printHeader(_jsout);
    printGeneratedHeader(_jsout, _fileBase + ".ice");

    //
    // Check for file "js:module:ice" metadata. If this is set then we are building Ice.
    //
    bool icejs = module == "ice";

    _jsout << nl << "/* eslint-disable */";
    _jsout << nl << "/* jshint ignore: start */";
    _jsout << nl;

    {
        ImportVisitor importVisitor(_jsout, _includePaths, icejs);
        p->visit(&importVisitor, false);
        set<string> importedModules = importVisitor.writeImports(p);

        ExportsVisitor exportsVisitor(_jsout, importedModules);
        p->visit(&exportsVisitor, false);
        set<string> exportedModules = exportsVisitor.exportedModules();

        set<string> seenModules = importedModules;
        seenModules.merge(exportedModules);

        TypesVisitor typesVisitor(_jsout);
        p->visit(&typesVisitor, false);
    }

    if (_useStdout)
    {
        _jsout << "\n";
        _jsout << "/** slice2js: generated end **/";
        _jsout << "\n";
    }

    if (_typeScript)
    {
        if (_useStdout)
        {
            _tsout << "\n";
            _tsout << "/** slice2js: " << _fileBase << ".d.ts generated begin module:\"" << module << "\" **/";
            _tsout << "\n";
        }
        printHeader(_tsout);
        printGeneratedHeader(_tsout, _fileBase + ".ice");

        TypeScriptImportVisitor importVisitor(_tsout, icejs);
        p->visit(&importVisitor, false);

        //
        // If at some point TypeScript adds an operator to refer to a type in the global scope
        // we can get rid of the TypeScriptAliasVisitor and use this. For now we need to generate
        // a type alias when there is an ambiguity.
        // see: https://github.com/Microsoft/TypeScript/issues/983
        //
        TypeScriptAliasVisitor aliasVisitor(_tsout);
        p->visit(&aliasVisitor, false);
        aliasVisitor.writeAlias(p);

        TypeScriptVisitor typeScriptVisitor(_tsout, importVisitor.imports());
        p->visit(&typeScriptVisitor, false);

        if (_useStdout)
        {
            _tsout << "\n";
            _tsout << "/** slice2js: generated end **/";
            _tsout << "\n";
        }
    }
}

Slice::Gen::ImportVisitor::ImportVisitor(IceUtilInternal::Output& out, vector<string> includePaths, bool icejs)
    : JsVisitor(out),
      _icejs(icejs),
      _seenClass(false),
      _seenInterface(false),
      _seenCompactId(false),
      _seenOperation(false),
      _seenStruct(false),
      _seenUserException(false),
      _seenEnum(false),
      _seenSeq(false),
      _seenDict(false),
      _seenObjectSeq(false),
      _seenObjectDict(false),
      _seenObjectProxyDict(false),
      _includePaths(includePaths)
{
    for (vector<string>::iterator p = _includePaths.begin(); p != _includePaths.end(); ++p)
    {
        *p = fullPath(*p);
    }
}

bool
Slice::Gen::ImportVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    _seenClass = true;
    if (p->compactId() >= 0)
    {
        _seenCompactId = true;
    }
    return true;
}

bool
Slice::Gen::ImportVisitor::visitInterfaceDefStart(const InterfaceDefPtr&)
{
    _seenInterface = true;
    return true;
}

bool
Slice::Gen::ImportVisitor::visitStructStart(const StructPtr&)
{
    _seenStruct = true;
    return false;
}

void
Slice::Gen::ImportVisitor::visitOperation(const OperationPtr&)
{
    _seenOperation = true;
}

bool
Slice::Gen::ImportVisitor::visitExceptionStart(const ExceptionPtr&)
{
    _seenUserException = true;
    return false;
}

void
Slice::Gen::ImportVisitor::visitSequence(const SequencePtr& seq)
{
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(seq->type());
    if (builtin)
    {
        switch (builtin->kind())
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
    _seenSeq = true;
}

void
Slice::Gen::ImportVisitor::visitDictionary(const DictionaryPtr& dict)
{
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(dict->valueType());
    if (builtin)
    {
        switch (builtin->kind())
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
    _seenDict = true;
}

void
Slice::Gen::ImportVisitor::visitEnum(const EnumPtr&)
{
    _seenEnum = true;
}

namespace
{
    string getJavaScriptModule(const DefinitionContextPtr& dc)
    {
        const string prefix = "js:module:";
        const string m = dc->findMetaData(prefix);
        if (!m.empty())
        {
            return m.substr(prefix.size());
        }
        return "";
    }

    // Convert a path to a module name, e.g., "../foo/bar/baz.ice" -> "__foo_bar_baz"
    string pathToModule(const string& path)
    {
        string module = removeExtension(path);

        size_t pos = module.find('/');
        if (pos == string::npos)
        {
            pos = module.find('\\');
        }

        if (pos != string::npos)
        {
            module = module.substr(pos + 1);

            // Replace remaining path separators ('/', '\') and ('.') with '_'
            replace(module.begin(), module.end(), '/', '_');
            replace(module.begin(), module.end(), '\\', '_');
            replace(module.begin(), module.end(), '.', '_');
        }
        return module;
    }

    bool ends_with(string_view s, string_view suffix)
    {
#if defined __cpp_lib_starts_ends_with
        return s.ends_with(suffix);
#else
        return s.size() >= suffix.size() && s.compare(s.size() - s.size(), std::string_view::npos, s) == 0;
#endif
    }
}

set<string>
Slice::Gen::ImportVisitor::writeImports(const UnitPtr& p)
{
    set<string> importedModules = {"Ice"};

    // The imports map maps JavaScript Modules to the set of Slice top-level modules that are imported from the
    // generated JavaScript code. The key is either the JavaScript module specified by "js:module:" metadata or the
    // relative path of the generated JavaScript file.
    map<string, set<string>> imports;

    // The JavaScript files that we import in generated code when building Ice. The user generated code imports
    // the "ice" package.
    set<string> jsIceImports;
    if (_icejs)
    {
        bool needStreamHelpers = false;
        if (_seenClass || _seenInterface || _seenObjectSeq || _seenObjectDict)
        {
            jsIceImports.insert("Object");
            jsIceImports.insert("Value");
            jsIceImports.insert("TypeRegistry");
            needStreamHelpers = true;
        }

        if (_seenInterface)
        {
            jsIceImports.insert("ObjectPrx");
            jsIceImports.insert("TypeRegistry");
        }

        if (_seenObjectProxySeq || _seenObjectProxyDict)
        {
            jsIceImports.insert("ObjectPrx");
        }

        if (_seenOperation)
        {
            jsIceImports.insert("Operation");
        }

        if (_seenStruct)
        {
            jsIceImports.insert("Struct");
        }

        if (_seenUserException)
        {
            jsIceImports.insert("Exception");
            jsIceImports.insert("TypeRegistry");
        }

        if (_seenEnum)
        {
            jsIceImports.insert("EnumBase");
        }

        if (_seenCompactId)
        {
            jsIceImports.insert("CompactIdRegistry");
        }

        jsIceImports.insert("Long");
        if (_seenDict || _seenObjectDict || _seenObjectProxyDict)
        {
            jsIceImports.insert("HashMap");
            jsIceImports.insert("HashUtil");
            needStreamHelpers = true;
        }

        if (_seenSeq || _seenObjectSeq)
        {
            jsIceImports.insert("ArrayUtil");
            needStreamHelpers = true;
        }

        if (needStreamHelpers)
        {
            jsIceImports.insert("StreamHelpers");
            jsIceImports.insert("Stream");
        }
    }
    else
    {
        imports["ice"] = set<string>{"Ice"};
    }

    StringList includes = p->includeFiles();

    // The JavaScript module we are building as specified by "js:module:" metadata.
    string jsModule = getJavaScriptModule(p->findDefinitionContext(p->topLevelFile()));

    // Iterate all the included files and generate an import statement for each top-level module in the included file.
    for (const auto& included : includes)
    {
        set<string> sliceTopLevelModules = p->getTopLevelModules(included);

        // The JavaScript module corresponding to the "js:module:" metadata in the included file.
        string jsImportedModule = getJavaScriptModule(p->findDefinitionContext(included));

        if (jsModule == jsImportedModule || jsImportedModule.empty())
        {
            // For Slice modules mapped to the same JavaScript module, or Slice files that doesn't use "js:module".
            // We import them using their Slice include relative path.
            string f = removeExtension(included) + ".js";
            if (IceUtilInternal::isAbsolutePath(f))
            {
                // If the include file is an absolute path, we need to generate a relative path.
                f = relativePath(f, p->topLevelFile());
            }
            else if (f[0] != '.')
            {
                // Make the import relative to the current directory.
                f = "./" + f;
            }
            imports[f] = sliceTopLevelModules;

            for (const auto& topLevelModule : sliceTopLevelModules)
            {
                importedModules.insert(topLevelModule);
            }
        }
        else
        {
            // When importing a generated Slice file from a different JavaScript module, we need to import all
            // top-level modules from the included file.

            if (imports.find(jsImportedModule) == imports.end())
            {
                imports[jsImportedModule] = set<string>{};
            }

            for (const auto& topLevelModule : sliceTopLevelModules)
            {
                imports[jsImportedModule].insert(topLevelModule);
            }
        }
    }

    set<string> aggregatedModules;

    // We first import the Ice runtime
    if (_icejs)
    {
        for (const string& m : jsIceImports)
        {
            _out << nl << "import * as Ice_" << m << " from \"../Ice/" << m << ".js\";";
        }

        for (const auto& [imported, modules] : imports)
        {
            if (modules.find("Ice") != modules.end())
            {
                _out << nl << "import { Ice as Ice_" << pathToModule(imported) << " } from \"" << imported << "\"";
            }
        }

        _out << sp;
        _out << nl << "const Ice = {";
        _out.inc();
        for (const string& m : jsIceImports)
        {
            _out << nl << "...Ice_" << m << ",";
        }

        for (auto& [imported, modules] : imports)
        {
            if (modules.find("Ice") != modules.end())
            {
                _out << nl << "...Ice_" << pathToModule(imported) << ",";

                modules.erase("Ice");
            }
        }

        _out.dec();
        _out << nl << "};";
    }
    else
    {
        // Import the required modules from "ice" JavaScript module.
        set<string> iceModules = imports["ice"];
        for (set<string>::const_iterator i = iceModules.begin(); i != iceModules.end();)
        {
            _out << nl << "import { ";
            _out << (*i);
            if (++i != iceModules.end())
            {
                _out << ", ";
            }
        }
        _out << " } from \"ice\";";
        // Remove Ice already imported from the list of imports.
        imports.erase("ice");
    }

    // Process the remaining imports, after importing "ice".
    _out << sp;
    for (const auto& [jsImportedModule, topLevelModules] : imports)
    {
        if (topLevelModules.empty())
        {
            // If topLevelModules is empty it means that the included file is not required by the generated code.
            continue;
        }

        if (ends_with(jsImportedModule, ".js"))
        {
            _out << nl << "import { ";
            _out.inc();
            for (const auto& topLevelModule : topLevelModules)
            {
                _out << nl << topLevelModule << " as " << topLevelModule << "_" << pathToModule(jsImportedModule)
                     << ", ";
                aggregatedModules.insert(topLevelModule);
            }
            _out.dec();
            _out << "} from \"" << jsImportedModule << "\"";
        }
        else
        {
            _out << nl << "import { ";
            _out.inc();
            for (const auto& topLevelModule : topLevelModules)
            {
                _out << nl << topLevelModule << ", ";
            }
            _out.dec();
            _out << "} from \"" << jsImportedModule << "\"";
        }
    }

    // Aggregate all Slice top-level-modules imported from .js files, for imports corresponding to js:module modules
    // there is no additional aggregation.
    for (const string& m : aggregatedModules)
    {
        if (m == "Ice")
        {
            // Ice module is already imported.
            continue;
        }
        _out << sp;
        _out << nl << "const " << m << " = {";
        _out.inc();
        for (const auto& [jsImportedModule, topLevelModules] : imports)
        {
            if (topLevelModules.find(m) != topLevelModules.end())
            {
                _out << nl << "..." << m << "_" << pathToModule(jsImportedModule) << ",";
            }
        }
        _out.dec();
        _out << nl << "};";
    }

    // TODO aggregate the sub-modules.
    // If module Foo.Bar was imported from multiple files, we need to aggregate them into a single
    // Foo.Bar module. This mut be done in a top-down order.
    // Foo.Bar = { ...Foo_Bar_1.Bar, ...Foo_Bar_2.Bar, ... }
    // Foo.Bar.Baz = { ...Foo_Bar_1.Bar.Baz, ...Foo_Bar_2.Bar.Baz, ... }

    return importedModules;
}

Slice::Gen::ExportsVisitor::ExportsVisitor(::IceUtilInternal::Output& out, std::set<std::string> importedModules)
    : JsVisitor(out),
      _importedModules(importedModules)
{
}

bool
Slice::Gen::ExportsVisitor::visitModuleStart(const ModulePtr& p)
{
    //
    // For a top-level module we write the following:
    //
    // export const Foo = {}; // When Foo is a module not previously imported
    // export { Foo }; // When Foo is a module previously imported
    //
    // For a nested module we write
    //
    // Foo.Bar = Foo.Bar || {};
    //
    const string scoped = getLocalScope(p->scoped());
    if (_exportedModules.insert(scoped).second)
    {
        const bool topLevel = dynamic_pointer_cast<Unit>(p->container()) != nullptr;
        _out << sp;
        _out << nl;
        if (topLevel)
        {
            if (_importedModules.find(scoped) == _importedModules.end())
            {
                _out << "export const " << scoped << " = {};";
            }
            else
            {
                _out << "export { " << scoped << " };";
            }
        }
        else
        {
            _out << scoped << " = " << scoped << " || {};";
        }
    }
    return true;
}

set<string>
Slice::Gen::ExportsVisitor::exportedModules() const
{
    return _exportedModules;
}

Slice::Gen::TypesVisitor::TypesVisitor(IceUtilInternal::Output& out) : JsVisitor(out) {}

bool
Slice::Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    const string scope = p->scope();
    const string scoped = p->scoped();
    const string localScope = getLocalScope(scope);
    const string name = fixId(p->name());
    ClassDefPtr base = p->base();
    string baseRef = base ? fixId(base->scoped()) : "Ice.Value";

    const DataMemberList allDataMembers = p->allDataMembers();
    const DataMemberList dataMembers = p->dataMembers();
    const DataMemberList optionalMembers = p->orderedOptionalDataMembers();

    vector<string> allParamNames;
    for (DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
    {
        allParamNames.push_back(fixId((*q)->name()));
    }

    vector<string> baseParamNames;
    DataMemberList baseDataMembers;

    if (base)
    {
        baseDataMembers = base->allDataMembers();
        for (DataMemberList::const_iterator q = baseDataMembers.begin(); q != baseDataMembers.end(); ++q)
        {
            baseParamNames.push_back(fixId((*q)->name()));
        }
    }

    _out << sp;
    writeDocCommentFor(p);
    _out << nl << localScope << '.' << name << " = class";
    _out << " extends " << baseRef;

    _out << sb;
    if (!allParamNames.empty())
    {
        _out << nl << "constructor" << spar;
        for (DataMemberList::const_iterator q = baseDataMembers.begin(); q != baseDataMembers.end(); ++q)
        {
            _out << fixId((*q)->name());
        }

        for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            string value;
            if ((*q)->optional())
            {
                if ((*q)->defaultValueType())
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
                if ((*q)->defaultValueType())
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
            writeMarshalDataMembers(dataMembers, optionalMembers);
            _out << eb;

            _out << sp;
            _out << nl << "_iceReadMemberImpl(istr)";
            _out << sb;
            writeUnmarshalDataMembers(dataMembers, optionalMembers);
            _out << eb;
        }
    }
    _out << eb << ";";

    _out << sp;

    _out << nl << "Ice.defineValue(" << localScope << "." << name << ", \"" << scoped << "\"";
    if (p->compactId() >= 0)
    {
        _out << ", " << p->compactId();
    }
    _out << ");";
    _out << nl << "Ice.TypeRegistry.declareValueType(\"" << localScope << '.' << name << "\", " << localScope << '.'
         << name << ");";

    return false;
}

bool
Slice::Gen::TypesVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    const string scope = p->scope();
    const string scoped = p->scoped();
    const string localScope = getLocalScope(scope);
    const string serviceType = localScope + '.' + fixId(p->name());
    const string proxyType = localScope + '.' + p->name() + "Prx";

    InterfaceList bases = p->bases();
    StringList ids = p->ids();

    _out << sp;
    _out << nl << "const iceC_" << getLocalScope(scoped, "_") << "_ids = [";
    _out.inc();

    for (StringList::const_iterator q = ids.begin(); q != ids.end(); ++q)
    {
        if (q != ids.begin())
        {
            _out << ',';
        }
        _out << nl << '"' << *q << '"';
    }

    _out.dec();
    _out << nl << "];";

    //
    // Define servant and proxy types
    //

    _out << sp;
    writeDocCommentFor(p);
    _out << nl << serviceType << " = class extends Ice.Object";
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
    // Generate a proxy class for interfaces
    //
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
    _out << nl << "Ice.TypeRegistry.declareProxyType(\"" << proxyType << "\", " << proxyType << ");";

    _out << sp;
    _out << nl << "Ice.defineOperations(";
    _out.inc();
    _out << nl << serviceType << "," << nl << proxyType << "," << nl << "iceC_" << getLocalScope(scoped, "_") << "_ids,"
         << nl << "\"" << scoped << "\"";

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
            const string opName = fixId(op->name());
            const ParamDeclList paramList = op->parameters();
            const TypePtr ret = op->returnType();
            ParamDeclList inParams, outParams;
            for (ParamDeclList::const_iterator pli = paramList.begin(); pli != paramList.end(); ++pli)
            {
                if ((*pli)->isOutParam())
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
            //  2: format (undefined == Default or int)
            //  3: return type (undefined if void, or [type, tag])
            //  4: in params (undefined if none, or array of [type, tag])
            //  5: out params (undefined if none, or array of [type, tag])
            //  6: exceptions (undefined if none, or array of types)
            //  7: sends classes (true or undefined)
            //  8: returns classes (true or undefined)
            //
            _out << nl << "\"" << op->name() << "\": ["; // Operation name over-the-wire.

            if (opName != op->name())
            {
                _out << "\"" << opName << "\""; // Native method name.
            }
            _out << ", ";

            if (op->mode() != Operation::Normal)
            {
                _out << sliceModeToIceMode(op->mode()); // Mode.
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
                if (op->returnIsOptional())
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
                for (ParamDeclList::const_iterator pli = inParams.begin(); pli != inParams.end(); ++pli)
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
                    if ((*pli)->optional())
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
                for (ParamDeclList::const_iterator pli = outParams.begin(); pli != outParams.end(); ++pli)
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
                    if ((*pli)->optional())
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

            if (op->sendsClasses())
            {
                _out << "true";
            }
            _out << ", ";

            if (op->returnsClasses())
            {
                _out << "true";
            }

            _out << ']';
        }
        _out << eb;
    }
    _out << ");";
    _out.dec();

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
    _out << nl << scope << "." << propertyName << " = Ice.StreamHelpers.generateSeqHelper(" << getHelper(type) << ", "
         << (fixed ? "true" : "false");
    if (type->isClassType())
    {
        _out << ", \"" << typeToString(type) << "\"";
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
    if (base)
    {
        baseRef = fixId(base->scoped());
    }
    else
    {
        baseRef = "Ice.UserException";
    }

    const DataMemberList allDataMembers = p->allDataMembers();
    const DataMemberList dataMembers = p->dataMembers();
    const DataMemberList optionalMembers = p->orderedOptionalDataMembers();

    vector<string> allParamNames;
    for (DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
    {
        allParamNames.push_back(fixId((*q)->name()));
    }

    vector<string> baseParamNames;
    DataMemberList baseDataMembers;

    if (p->base())
    {
        baseDataMembers = p->base()->allDataMembers();
        for (DataMemberList::const_iterator q = baseDataMembers.begin(); q != baseDataMembers.end(); ++q)
        {
            baseParamNames.push_back(fixId((*q)->name()));
        }
    }

    _out << sp;
    writeDocCommentFor(p);
    _out << nl << localScope << '.' << name << " = class extends " << baseRef;
    _out << sb;

    _out << nl << "constructor" << spar;

    for (DataMemberList::const_iterator q = baseDataMembers.begin(); q != baseDataMembers.end(); ++q)
    {
        _out << fixId((*q)->name());
    }

    for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string value;
        if ((*q)->optional())
        {
            if ((*q)->defaultValueType())
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
            if ((*q)->defaultValueType())
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

    _out << sp;
    _out << nl << "_mostDerivedType()";
    _out << sb;
    _out << nl << "return " << localScope << '.' << name << ";";
    _out << eb;

    if (!dataMembers.empty())
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

    if (p->usesClasses() && !(base && base->usesClasses()))
    {
        _out << sp;
        _out << nl << "_usesClasses()";
        _out << sb;
        _out << nl << "return true;";
        _out << eb;
    }

    _out << eb << ";";
    _out << nl << "Ice.TypeRegistry.declareUserExceptionType(";
    _out.inc();
    _out << nl << "\"" << localScope << '.' << name << "\",";
    _out << nl << localScope << '.' << name << ");";
    _out.dec();

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
    for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        paramNames.push_back(fixId((*q)->name()));
    }

    _out << sp;
    writeDocCommentFor(p);
    _out << nl << localScope << '.' << name << " = class";
    _out << sb;

    _out << nl << "constructor" << spar;

    for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string value;
        if ((*q)->optional())
        {
            if ((*q)->defaultValueType())
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
            if ((*q)->defaultValueType())
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

    _out << eb << ";";

    //
    // Only generate hashCode if this structure type is a legal dictionary key type.
    //
    bool legalKeyType = Dictionary::legalKeyType(p);

    _out << sp;
    _out << nl << "Ice.defineStruct(" << localScope << '.' << name << ", " << (legalKeyType ? "true" : "false") << ", "
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
    BuiltinPtr b = dynamic_pointer_cast<Builtin>(keyType);
    if ((b && b->kind() == Builtin::KindLong) || dynamic_pointer_cast<Struct>(keyType))
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
    _out << nl << "[" << scope << "." << name << ", " << scope << "." << propertyName << "] = Ice.defineDictionary("
         << getHelper(keyType) << ", " << getHelper(valueType) << ", " << (fixed ? "true" : "false") << ", "
         << (keyUseEquals ? "Ice.HashMap.compareEquals" : "undefined");

    if (valueType->isClassType())
    {
        _out << ", \"" << typeToString(valueType) << "\"";
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
    writeDocCommentFor(p);
    _out << nl << localScope << '.' << name << " = Ice.defineEnum([";
    _out.inc();
    _out << nl;

    const EnumeratorList enumerators = p->enumerators();
    int i = 0;
    for (EnumeratorList::const_iterator en = enumerators.begin(); en != enumerators.end(); ++en)
    {
        if (en != enumerators.begin())
        {
            if (++i % 5 == 0)
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
    _out << nl << "enumerable: true,";
    _out << nl << "value: ";
    _out << writeConstantValue(scope, p->type(), p->valueType(), p->value());
    _out.dec();
    _out << nl << "});";
}

string
Slice::Gen::TypesVisitor::encodeTypeForOperation(const TypePtr& type)
{
    assert(type);

    static const char* builtinTable[] = {
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
        "10", // Ice.Value
    };

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        return builtinTable[builtin->kind()];
    }

    InterfaceDeclPtr proxy = dynamic_pointer_cast<InterfaceDecl>(type);
    if (proxy)
    {
        return "\"" + fixId(proxy->scoped() + "Prx") + "\"";
    }

    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    if (seq)
    {
        return fixId(seq->scoped() + "Helper");
    }

    DictionaryPtr d = dynamic_pointer_cast<Dictionary>(type);
    if (d)
    {
        return fixId(d->scoped() + "Helper");
    }

    EnumPtr e = dynamic_pointer_cast<Enum>(type);
    if (e)
    {
        return fixId(e->scoped()) + "._helper";
    }

    StructPtr st = dynamic_pointer_cast<Struct>(type);
    if (st)
    {
        return fixId(st->scoped());
    }

    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
    if (cl)
    {
        return "\"" + fixId(cl->scoped()) + "\"";
    }

    return "???";
}

Slice::Gen::TypeScriptImportVisitor::TypeScriptImportVisitor(IceUtilInternal::Output& out, bool icejs)
    : JsVisitor(out),
      _icejs(icejs),
      _nextImport(0)
{
}

string
Slice::Gen::TypeScriptImportVisitor::nextImportPrefix()
{
    ostringstream ns;
    ns << "iceNS" << _nextImport++;
    return ns.str();
}

void
Slice::Gen::TypeScriptImportVisitor::addImport(const TypePtr& definition, const ContainedPtr& toplevel)
{
    if (!dynamic_pointer_cast<Builtin>(definition))
    {
        string m1 = getModuleMetadata(definition);
        const string m2 = getModuleMetadata(toplevel);

        string p1 = definition->definitionContext()->filename();
        const string p2 = toplevel->definitionContext()->filename();

        InterfaceDeclPtr p = dynamic_pointer_cast<InterfaceDecl>(definition);

        if (p)
        {
            if (p->definition() == 0)
            {
                string definedIn = getDefinedIn(p);
                if (!definedIn.empty())
                {
                    p1 = definedIn;
                }
            }
            else
            {
                m1 = getModuleMetadata(dynamic_pointer_cast<Contained>(p->definition()));
                p1 = p->definition()->definitionContext()->filename();
            }
        }
        addImport(m1, m2, p1, p2);
    }
}

void
Slice::Gen::TypeScriptImportVisitor::addImport(const ContainedPtr& definition, const ContainedPtr& toplevel)
{
    const string m1 = getModuleMetadata(definition);
    const string m2 = getModuleMetadata(toplevel);

    const string p1 = definition->definitionContext()->filename();
    const string p2 = toplevel->definitionContext()->filename();

    addImport(m1, m2, p1, p2);
}

void
Slice::Gen::TypeScriptImportVisitor::addImport(const string& m1, const string& m2, const string& p1, const string& p2)
{
    //
    // Generate an import for a definition that is outside a JS module and comes from
    // a different definition context or for a definition defined in a module different
    // than the current module.
    //
    if (m1.empty())
    {
        if (p1 != p2)
        {
            string relpath = relativePath(p1, p2);

            string::size_type pos = relpath.rfind('.');
            if (pos != string::npos)
            {
                relpath.erase(pos);
            }

            for (vector<pair<string, string>>::const_iterator i = _imports.begin(); i != _imports.end(); ++i)
            {
                if (i->first == relpath)
                {
                    return;
                }
            }
            _imports.push_back(make_pair(relpath, nextImportPrefix()));
        }
    }
    else if (m1 != m2)
    {
        for (vector<pair<string, string>>::const_iterator i = _imports.begin(); i != _imports.end(); ++i)
        {
            if (i->first == m1)
            {
                return;
            }
        }
        _imports.push_back(make_pair(m1, nextImportPrefix()));
    }
}

bool
Slice::Gen::TypeScriptImportVisitor::visitModuleStart(const ModulePtr& p)
{
    //
    // Import ice module if not building Ice
    //
    if (dynamic_pointer_cast<Unit>(p->container()) && !_icejs && _imports.empty())
    {
        _imports.push_back(make_pair("ice", nextImportPrefix()));
    }
    return true;
}

bool
Slice::Gen::TypeScriptImportVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    //
    // Add imports required for base class
    //
    ClassDefPtr base = p->base();
    if (base)
    {
        addImport(dynamic_pointer_cast<Contained>(base), p);
    }

    //
    // Add imports required for data members
    //
    const DataMemberList allDataMembers = p->allDataMembers();
    for (DataMemberList::const_iterator i = allDataMembers.begin(); i != allDataMembers.end(); ++i)
    {
        addImport((*i)->type(), p);
    }

    return false;
}

bool
Slice::Gen::TypeScriptImportVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    //
    // Add imports required for base interfaces
    //
    InterfaceList bases = p->bases();
    for (InterfaceList::const_iterator i = bases.begin(); i != bases.end(); ++i)
    {
        addImport(dynamic_pointer_cast<Contained>(*i), p);
    }

    //
    // Add imports required for operation parameters and return type
    //
    const OperationList operationList = p->allOperations();
    for (OperationList::const_iterator i = operationList.begin(); i != operationList.end(); ++i)
    {
        const TypePtr ret = (*i)->returnType();
        if (ret && ret->definitionContext())
        {
            addImport(ret, p);
        }

        const ParamDeclList paramList = (*i)->parameters();
        for (ParamDeclList::const_iterator j = paramList.begin(); j != paramList.end(); ++j)
        {
            addImport((*j)->type(), p);
        }
    }
    return false;
}

bool
Slice::Gen::TypeScriptImportVisitor::visitStructStart(const StructPtr& p)
{
    //
    // Add imports required for data members
    //
    const DataMemberList dataMembers = p->dataMembers();
    for (DataMemberList::const_iterator i = dataMembers.begin(); i != dataMembers.end(); ++i)
    {
        addImport((*i)->type(), p);
    }
    return false;
}

bool
Slice::Gen::TypeScriptImportVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    //
    // Add imports required for base exceptions
    //
    ExceptionPtr base = p->base();
    if (base)
    {
        addImport(dynamic_pointer_cast<Contained>(base), p);
    }

    //
    // Add imports required for data members
    //
    const DataMemberList allDataMembers = p->allDataMembers();
    for (DataMemberList::const_iterator i = allDataMembers.begin(); i != allDataMembers.end(); ++i)
    {
        addImport((*i)->type(), p);
    }
    return false;
}

void
Slice::Gen::TypeScriptImportVisitor::visitSequence(const SequencePtr& seq)
{
    //
    // Add import required for the sequence element type
    //
    addImport(seq->type(), seq);
}

void
Slice::Gen::TypeScriptImportVisitor::visitDictionary(const DictionaryPtr& dict)
{
    //
    // Add imports required for the dictionary key and value types
    //
    addImport(dict->keyType(), dict);
    addImport(dict->valueType(), dict);
}

Slice::Gen::TypeScriptAliasVisitor::TypeScriptAliasVisitor(IceUtilInternal::Output& out) : JsVisitor(out) {}

void
Slice::Gen::TypeScriptAliasVisitor::addAlias(const ExceptionPtr& type, const ContainedPtr& toplevel)
{
    string m1 = getModuleMetadata(dynamic_pointer_cast<Contained>(type));
    string m2 = getModuleMetadata(toplevel);

    //
    // Do not add alias for a type defined in the current module
    //
    if (!m1.empty() && m1 == m2)
    {
        return;
    }

    const string prefix = importPrefix(dynamic_pointer_cast<Contained>(type), toplevel, imports());
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
    if (!m1.empty() && m1 == m2)
    {
        return;
    }

    addAlias(typeToString(type, toplevel, imports(), true), importPrefix(type, toplevel, imports()), toplevel);
}

void
Slice::Gen::TypeScriptAliasVisitor::addAlias(const string& type, const string& prefix, const ContainedPtr& toplevel)
{
    const string scope = fixId(toplevel->scoped()) + ".";
    //
    // When using an import prefix we don't need an alias, prefixes use iceNSXX that is reserved
    // name prefix
    //
    string::size_type i = type.find(".");
    if (prefix.empty() && i != string::npos)
    {
        if (scope.find("." + type.substr(0, i + 1)) != string::npos)
        {
            for (vector<pair<string, string>>::const_iterator j = _aliases.begin(); j != _aliases.end(); ++j)
            {
                if (j->first == type)
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
Slice::Gen::TypeScriptAliasVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    ModulePtr module = dynamic_pointer_cast<Module>(p->container());
    //
    // Add alias required for base class
    //
    ClassDefPtr base = p->base();
    if (base)
    {
        addAlias(dynamic_pointer_cast<Type>(base->declaration()), module);
    }

    //
    // Add alias required for data members
    //
    const DataMemberList allDataMembers = p->allDataMembers();
    for (DataMemberList::const_iterator i = allDataMembers.begin(); i != allDataMembers.end(); ++i)
    {
        addAlias((*i)->type(), module);
    }

    return false;
}

bool
Slice::Gen::TypeScriptAliasVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    ModulePtr module = dynamic_pointer_cast<Module>(p->container());
    //
    // Add alias required for base interfaces
    //
    InterfaceList bases = p->bases();
    for (InterfaceList::const_iterator i = bases.begin(); i != bases.end(); ++i)
    {
        addAlias(dynamic_pointer_cast<Type>((*i)->declaration()), module);
    }

    //
    // Add alias required for operation parameters
    //
    const OperationList operationList = p->allOperations();
    for (OperationList::const_iterator i = operationList.begin(); i != operationList.end(); ++i)
    {
        const TypePtr ret = (*i)->returnType();
        if (ret && ret->definitionContext())
        {
            addAlias(ret, module);
        }

        const ParamDeclList paramList = (*i)->parameters();
        for (ParamDeclList::const_iterator j = paramList.begin(); j != paramList.end(); ++j)
        {
            addAlias((*j)->type(), module);
        }
    }
    return false;
}

bool
Slice::Gen::TypeScriptAliasVisitor::visitStructStart(const StructPtr& p)
{
    ModulePtr module = dynamic_pointer_cast<Module>(p->container());
    //
    // Add alias required for data members
    //
    const DataMemberList dataMembers = p->dataMembers();
    for (DataMemberList::const_iterator i = dataMembers.begin(); i != dataMembers.end(); ++i)
    {
        addAlias((*i)->type(), module);
    }
    return false;
}

bool
Slice::Gen::TypeScriptAliasVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    ModulePtr module = dynamic_pointer_cast<Module>(p->container());
    //
    // Add alias required for base exception
    //
    ExceptionPtr base = p->base();
    if (base)
    {
        addAlias(base, module);
    }

    //
    // Add alias required for data members
    //
    const DataMemberList allDataMembers = p->allDataMembers();
    for (DataMemberList::const_iterator i = allDataMembers.begin(); i != allDataMembers.end(); ++i)
    {
        addAlias((*i)->type(), module);
    }
    return false;
}

void
Slice::Gen::TypeScriptAliasVisitor::visitSequence(const SequencePtr& seq)
{
    addAlias(seq->type(), dynamic_pointer_cast<Module>(seq->container()));
}

void
Slice::Gen::TypeScriptAliasVisitor::visitDictionary(const DictionaryPtr& dict)
{
    ModulePtr module = dynamic_pointer_cast<Module>(dict->container());
    addAlias(dict->keyType(), module);
    addAlias(dict->valueType(), module);
}

void
Slice::Gen::TypeScriptAliasVisitor::writeAlias(const UnitPtr&)
{
    if (!_aliases.empty())
    {
        _out << sp;
        for (vector<pair<string, string>>::const_iterator i = _aliases.begin(); i != _aliases.end(); ++i)
        {
            _out << nl << "type " << i->second << " = " << i->first << ";";
        }
    }
}

Slice::Gen::TypeScriptVisitor::TypeScriptVisitor(
    ::IceUtilInternal::Output& out,
    const vector<pair<string, string>>& imports)
    : JsVisitor(out, imports),
      _wroteImports(false)
{
}

void
Slice::Gen::TypeScriptVisitor::writeImports()
{
    if (!_wroteImports)
    {
        for (vector<pair<string, string>>::const_iterator i = _imports.begin(); i != _imports.end(); ++i)
        {
            _out << nl << "import * as " << i->second << " from \"" << i->first << "\"";
        }
        _wroteImports = true;
    }
}

bool
Slice::Gen::TypeScriptVisitor::visitModuleStart(const ModulePtr& p)
{
    UnitPtr unit = dynamic_pointer_cast<Unit>(p->container());
    if (unit)
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
    const string toplevelModule = getModuleMetadata(dynamic_pointer_cast<Contained>(p));
    const string icePrefix = importPrefix("Ice.", p);

    const DataMemberList dataMembers = p->dataMembers();
    const DataMemberList allDataMembers = p->allDataMembers();
    _out << sp;
    writeDocSummary(_out, p);
    _out << nl << "class " << fixId(p->name()) << " extends ";
    const string scope = p->scope();
    const string scoped = p->scoped();
    ClassDefPtr base = p->base();
    if (base)
    {
        const string prefix = importPrefix(dynamic_pointer_cast<Contained>(base), p, imports());
        _out << prefix << getUnqualified(fixId(base->scoped()), p->scope(), prefix);
    }
    else
    {
        _out << icePrefix << getUnqualified("Ice.Value", p->scope(), icePrefix);
    }
    _out << sb;
    _out << nl << "/**";
    _out << nl << " * One-shot constructor to initialize all data members.";
    for (DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
    {
        CommentPtr comment = (*q)->parseComment(false);
        if (comment)
        {
            _out << nl << " * @param " << fixId((*q)->name()) << " " << getDocSentence(comment->overview());
        }
    }
    _out << nl << " */";
    _out << nl << "constructor" << spar;
    for (DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
    {
        _out << (fixId((*q)->name()) + "?:" + typeToString((*q)->type(), p, imports(), true, false, true));
    }
    _out << epar << ";";
    for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        writeDocSummary(_out, *q);
        _out << nl << fixId((*q)->name()) << ":" << typeToString((*q)->type(), p, imports(), true, false, true) << ";";
    }
    _out << eb;

    return false;
}

bool
Slice::Gen::TypeScriptVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    const string toplevelModule = getModuleMetadata(dynamic_pointer_cast<Contained>(p));
    const string icePrefix = importPrefix("Ice.", p);
    const OperationList ops = p->allOperations();

    //
    // Define servant an proxy types
    //
    _out << sp;
    _out << nl << "abstract class " << fixId(p->name() + "Prx") << " extends " << icePrefix
         << getUnqualified("Ice.ObjectPrx", p->scope(), icePrefix);
    _out << sb;

    for (OperationList::const_iterator q = ops.begin(); q != ops.end(); ++q)
    {
        const OperationPtr op = *q;
        const ParamDeclList paramList = op->parameters();
        const TypePtr ret = op->returnType();
        ParamDeclList inParams, outParams;
        for (ParamDeclList::const_iterator r = paramList.begin(); r != paramList.end(); ++r)
        {
            if ((*r)->isOutParam())
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
        if (comment)
        {
            StringList postParams, returns;
            postParams.push_back(contextDoc);
            returns.push_back(asyncDoc);
            writeOpDocSummary(_out, op, comment, OpDocInParams, false, StringList(), postParams, returns);
        }
        _out << nl << fixId((*q)->name()) << spar;
        for (ParamDeclList::const_iterator r = inParams.begin(); r != inParams.end(); ++r)
        {
            _out
                << (fixId((*r)->name()) + ((*r)->optional() ? "?" : "") + ":" +
                    typeToString((*r)->type(), p, imports(), true, false, true));
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

            for (ParamDeclList::const_iterator i = outParams.begin(); i != outParams.end();)
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

    _out << sp;
    _out << nl << "/**";
    _out << nl << " * Downcasts a proxy without confirming the target object's type via a remote invocation.";
    _out << nl << " * @param prx The target proxy.";
    _out << nl << " * @return A proxy with the requested type.";
    _out << nl << " */";
    _out << nl << "static uncheckedCast(prx:" << icePrefix << getUnqualified("Ice.ObjectPrx", p->scope(), icePrefix)
         << ", "
         << "facet?:string):" << fixId(p->name() + "Prx") << ";";
    _out << nl << "/**";
    _out << nl << " * Downcasts a proxy after confirming the target object's type via a remote invocation.";
    _out << nl << " * @param prx The target proxy.";
    _out << nl << " * @param facet A facet name.";
    _out << nl << " * @param context The context map for the invocation.";
    _out << nl
         << " * @return A proxy with the requested type and facet, or nil if the target proxy is nil or the target";
    _out << nl << " * object does not support the requested type.";
    _out << nl << " */";
    _out << nl << "static checkedCast(prx:" << icePrefix << getUnqualified("Ice.ObjectPrx", p->scope(), icePrefix)
         << ", "
         << "facet?:string, contex?:Map<string, string>):" << icePrefix
         << getUnqualified("Ice.AsyncResult", p->scope(), icePrefix) << "<" << fixId(p->name() + "Prx") << ">;";
    _out << eb;

    _out << sp;
    _out << nl << "abstract class " << fixId(p->name()) << " extends " << icePrefix
         << getUnqualified("Ice.Object", p->scope(), icePrefix);
    _out << sb;
    for (OperationList::const_iterator q = ops.begin(); q != ops.end(); ++q)
    {
        const OperationPtr op = *q;
        const ParamDeclList paramList = op->parameters();
        const TypePtr ret = op->returnType();
        ParamDeclList inParams, outParams;
        for (ParamDeclList::const_iterator r = paramList.begin(); r != paramList.end(); ++r)
        {
            if ((*r)->isOutParam())
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
        if (comment)
        {
            StringList postParams, returns;
            postParams.push_back(currentDoc);
            returns.push_back(resultDoc);
            writeOpDocSummary(_out, op, comment, OpDocInParams, false, StringList(), postParams, returns);
        }
        _out << nl << "abstract " << fixId((*q)->name()) << spar;
        for (ParamDeclList::const_iterator r = inParams.begin(); r != inParams.end(); ++r)
        {
            _out << (fixId((*r)->name()) + ":" + typeToString((*r)->type(), p, imports(), true, false, true));
        }
        _out << ("current:" + icePrefix + getUnqualified("Ice.Current", p->scope(), icePrefix));
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

            for (ParamDeclList::const_iterator i = outParams.begin(); i != outParams.end();)
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
    const DataMemberList dataMembers = p->dataMembers();
    const DataMemberList allDataMembers = p->allDataMembers();
    const string toplevelModule = getModuleMetadata(dynamic_pointer_cast<Contained>(p));
    const string icePrefix = importPrefix("Ice.", p);

    ExceptionPtr base = p->base();
    string baseRef;
    if (base)
    {
        const string prefix = importPrefix(dynamic_pointer_cast<Contained>(base), p, imports());
        baseRef = prefix + getUnqualified(fixId(base->scoped()), p->scope(), prefix);
    }
    else
    {
        baseRef = icePrefix + getUnqualified("Ice.UserException", p->scope(), icePrefix);
    }

    _out << sp;
    writeDocSummary(_out, p);
    _out << nl << "class " << name << " extends " << baseRef << sb;
    if (!allDataMembers.empty())
    {
        _out << nl << "/**";
        _out << nl << " * One-shot constructor to initialize all data members.";
        for (DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
        {
            CommentPtr comment = (*q)->parseComment(false);
            if (comment)
            {
                _out << nl << " * @param " << fixId((*q)->name()) << " " << getDocSentence(comment->overview());
            }
        }
        _out << nl << " * @param ice_cause The error that cause this exception.";
        _out << nl << " */";
        _out << nl << "constructor" << spar;
        for (DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
        {
            _out << (fixId((*q)->name()) + "?:" + typeToString((*q)->type(), p, imports(), true, false, true));
        }
        _out << "ice_cause?:string|Error";
        _out << epar << ";";
    }
    for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
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
    const string toplevelModule = getModuleMetadata(dynamic_pointer_cast<Contained>(p));
    _out << sp;
    writeDocSummary(_out, p);
    _out << nl << "class " << name << sb;
    _out << nl << "constructor" << spar;
    for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        _out << (fixId((*q)->name()) + "?:" + typeToString((*q)->type(), p, imports(), true, false, true));
    }
    _out << epar << ";";

    _out << nl << "clone():" << name << ";";
    _out << nl << "equals(rhs:any):boolean;";

    //
    // Only generate hashCode if this structure type is a legal dictionary key type.
    //
    if (Dictionary::legalKeyType(p))
    {
        _out << nl << "hashCode():number;";
    }

    for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        _out << nl << fixId((*q)->name()) << ":" << typeToString((*q)->type(), p, imports(), true, false, true) << ";";
    }

    //
    // Streaming API
    //
    _out << nl << "static write(outs:" << icePrefix << getUnqualified("Ice.OutputStream", p->scope(), icePrefix)
         << ", value:" << name << "):void;";
    _out << nl << "static read(ins:" << icePrefix << getUnqualified("Ice.InputStream", p->scope(), icePrefix)
         << "):" << name << ";";
    _out << eb;
    return false;
}

void
Slice::Gen::TypeScriptVisitor::visitSequence(const SequencePtr& p)
{
    const string icePrefix = importPrefix("Ice.", p);
    const string toplevelModule = getModuleMetadata(dynamic_pointer_cast<Contained>(p));
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
    _out << nl << "static read(ins:" << icePrefix << getUnqualified("Ice.InputStream", p->scope(), icePrefix)
         << "):" << name << ";";
    _out << eb;
}

void
Slice::Gen::TypeScriptVisitor::visitDictionary(const DictionaryPtr& p)
{
    const string icePrefix = importPrefix("Ice.", p);
    const string toplevelModule = getModuleMetadata(dynamic_pointer_cast<Contained>(p));
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
    _out << nl << "static read(ins:" << icePrefix << getUnqualified("Ice.InputStream", p->scope(), icePrefix)
         << "):" << name << ";";
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
    for (EnumeratorList::const_iterator en = enumerators.begin(); en != enumerators.end(); ++en)
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
