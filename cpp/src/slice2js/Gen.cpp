// Copyright (c) ZeroC, Inc.

#include "Gen.h"
#include "../Ice/Endian.h"
#include "../Ice/FileUtil.h"
#include "../Slice/FileTracker.h"
#include "../Slice/MetadataValidation.h"
#include "../Slice/Util.h"
#include "Ice/StringUtil.h"
#include "Ice/UUID.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>

using namespace std;
using namespace Slice;
using namespace Ice;
using namespace IceInternal;

namespace
{
    /// Returns a JsDoc formatted link to the provided Slice identifier.
    string jsLinkFormatter(const string& rawLink, const ContainedPtr&, const SyntaxTreeBasePtr& target)
    {
        ostringstream result;
        result << "{@link ";
        if (target)
        {
            if (auto builtinTarget = dynamic_pointer_cast<Builtin>(target))
            {
                result << JsGenerator::typeToJsString(builtinTarget, true);
            }
            else
            {
                if (auto operationTarget = dynamic_pointer_cast<Operation>(target))
                {
                    string targetScoped = operationTarget->interface()->mappedScoped(".").substr(1);

                    // link to the method on the proxy interface
                    result << targetScoped << "Prx." << operationTarget->mappedName();
                }
                else
                {
                    string targetScoped = dynamic_pointer_cast<Contained>(target)->mappedScoped(".").substr(1);
                    if (auto interfaceTarget = dynamic_pointer_cast<InterfaceDecl>(target))
                    {
                        // link to the proxy interface
                        result << targetScoped << "Prx";
                    }
                    else
                    {
                        result << targetScoped;
                    }
                }
            }
        }
        else
        {
            auto hashPos = rawLink.find('#');
            if (hashPos != string::npos)
            {
                // JavaScript TypeDoc doc processor doesn't accept # at the beginning of a link.
                if (hashPos != 0)
                {
                    result << rawLink.substr(0, hashPos) << "#";
                }
                result << rawLink.substr(hashPos + 1);
            }
            else
            {
                result << rawLink;
            }
        }
        result << "}";
        return result.str();
    }

    // Convert a path to a module name, e.g., "../foo/bar/baz.ice" -> "__foo_bar_baz"
    string pathToModule(const string& path)
    {
        string moduleName = removeExtension(path);

        size_t pos = moduleName.find('/');
        if (pos == string::npos)
        {
            pos = moduleName.find('\\');
        }

        if (pos != string::npos)
        {
            // Replace remaining path separators ('/', '\') and ('.') with '_'
            replace(moduleName.begin(), moduleName.end(), '/', '_');
            replace(moduleName.begin(), moduleName.end(), '\\', '_');
            replace(moduleName.begin(), moduleName.end(), '.', '_');
            // Replace @ in scoped moduleName names with _
            replace(moduleName.begin(), moduleName.end(), '@', '_');
        }

        return moduleName;
    }

    bool ends_with(string_view s, string_view suffix)
    {
#if defined __cpp_lib_starts_ends_with
        return s.ends_with(suffix);
#else
        return s.size() >= suffix.size() && s.compare(s.size() - s.size(), std::string_view::npos, s) == 0;
#endif
    }

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

    string opFormatTypeToString(FormatType opFormat)
    {
        switch (opFormat)
        {
            case CompactFormat:
                return "0";
            case SlicedFormat:
                return "1";
            default:
                assert(false);
                return "???";
        }
    }

    void printHeader(IceInternal::Output& out)
    {
        out << "// Copyright (c) ZeroC, Inc.";
        out << sp;
        out << nl << "// slice2js version " << ICE_STRING_VERSION;
    }

    string escapeParam(const ParameterList& params, const string& name)
    {
        for (const auto& param : params)
        {
            if (param->mappedName() == name)
            {
                return name + "_";
            }
        }
        return name;
    }

    void writeDocLines(Output& out, const StringList& lines, bool commentFirst, const string& space = " ")
    {
        if (lines.size() > 0)
        {
            StringList l = lines;
            if (!commentFirst)
            {
                out << l.front();
                l.pop_front();
            }

            for (const auto& i : l)
            {
                out << nl << " *";
                if (!i.empty())
                {
                    out << space << i;
                }
            }
        }
    }

    void writeSeeAlso(Output& out, const StringList& lines, const string& space = " ")
    {
        for (const auto& line : lines)
        {
            out << nl << " *";
            if (!line.empty())
            {
                out << space << "@see " << line;
            }
        }
    }

    void writeDeprecated(Output& out, const optional<DocComment>& comment, const ContainedPtr& contained)
    {
        // JavaScript doesn't provide a way to deprecate elements other than by using a comment, so we map both the
        // Slice @deprecated tag and the deprecated metadata argument to a `@deprecated` JSDoc tag.
        if ((comment && comment->isDeprecated()) || contained->isDeprecated())
        {
            out << nl << " * @deprecated";
            // If a reason was supplied, append it after the `@deprecated` tag. If no reason was supplied, fallback to
            // the deprecated metadata argument.
            if (!comment->deprecated().empty())
            {
                out << " ";
                writeDocLines(out, comment->deprecated(), false);
            }
            else if (auto deprecated = contained->getDeprecationReason())
            {
                out << " " << *deprecated;
            }
        }
    }

    string getDocSentence(const StringList& lines)
    {
        //
        // Extract the first sentence.
        //
        ostringstream ostr;
        for (auto i = lines.begin(); i != lines.end(); ++i)
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

    enum OpDocParamType
    {
        OpDocInParams,
        OpDocOutParams,
        OpDocAllParams
    };

    void writeOpDocExceptions(Output& out, const OperationPtr& op, const DocComment& doc)
    {
        for (const auto& [name, lines] : doc.exceptions())
        {
            // Try to locate the exception's definition using the name given in the comment.
            string mappedName = name;
            if (ExceptionPtr ex = op->container()->lookupException(name, false))
            {
                mappedName = ex->mappedScoped(".").substr(1);
            }
            out << nl << " * @throws {@link " << mappedName << "} ";
            writeDocLines(out, lines, false);
        }
    }
}

Slice::JsVisitor::JsVisitor(Output& out, const vector<pair<string, string>>& imports) : _out(out), _imports(imports) {}

Slice::JsVisitor::~JsVisitor() = default;

vector<pair<string, string>>
Slice::JsVisitor::imports() const
{
    return _imports;
}

void
Slice::JsVisitor::writeMarshalDataMembers(const DataMemberList& dataMembers, const DataMemberList& optionalMembers)
{
    for (const auto& dataMember : dataMembers)
    {
        if (!dataMember->optional())
        {
            writeMarshalUnmarshalCode(_out, dataMember->type(), "this." + dataMember->mappedName(), true);
        }
    }

    for (const auto& optionalMember : optionalMembers)
    {
        writeOptionalMarshalUnmarshalCode(
            _out,
            optionalMember->type(),
            "this." + optionalMember->mappedName(),
            optionalMember->tag(),
            true);
    }
}

void
Slice::JsVisitor::writeUnmarshalDataMembers(const DataMemberList& dataMembers, const DataMemberList& optionalMembers)
{
    for (const auto& dataMember : dataMembers)
    {
        if (!dataMember->optional())
        {
            writeMarshalUnmarshalCode(_out, dataMember->type(), "this." + dataMember->mappedName(), false);
        }
    }

    for (const auto& optionalMember : optionalMembers)
    {
        writeOptionalMarshalUnmarshalCode(
            _out,
            optionalMember->type(),
            "this." + optionalMember->mappedName(),
            optionalMember->tag(),
            false);
    }
}

string
Slice::JsVisitor::getValue(const TypePtr& type)
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
                return "0n";
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
        return (*en->enumerators().begin())->mappedScoped(".").substr(1);
    }

    StructPtr st = dynamic_pointer_cast<Struct>(type);
    if (st)
    {
        return "new " + typeToJsString(type) + "()";
    }

    return "null";
}

string
Slice::JsVisitor::writeConstantValue(const TypePtr& type, const SyntaxTreeBasePtr& valueType, const string& value)
{
    ostringstream os;
    ConstPtr constant = dynamic_pointer_cast<Const>(valueType);
    if (constant)
    {
        os << constant->mappedScoped(".").substr(1);
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
            os << value << "n";
        }
        else if ((ep = dynamic_pointer_cast<Enum>(type)))
        {
            EnumeratorPtr lte = dynamic_pointer_cast<Enumerator>(valueType);
            assert(lte);
            os << lte->mappedScoped(".").substr(1);
        }
        else
        {
            os << value;
        }
    }
    return os.str();
}

void
Slice::JsVisitor::writeDocCommentFor(const ContainedPtr& p, bool includeDeprecated)
{
    assert(!dynamic_pointer_cast<Operation>(p));
    optional<DocComment> comment = DocComment::parseFrom(p, jsLinkFormatter);
    if (!comment && (!includeDeprecated || !p->isDeprecated()))
    {
        // There's nothing to write for this doc-comment.
        return;
    }

    _out << nl << "/**";

    if (comment)
    {
        if (!comment->overview().empty())
        {
            writeDocLines(_out, comment->overview(), true);
        }

        if (!comment->seeAlso().empty())
        {
            writeSeeAlso(_out, comment->seeAlso());
        }
    }

    if (includeDeprecated)
    {
        writeDeprecated(_out, comment, p);
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

    _javaScriptOutput.open(file.c_str());
    if (!_javaScriptOutput)
    {
        ostringstream os;
        os << "cannot open '" << file << "': " << IceInternal::errorToString(errno);
        throw FileException(os.str());
    }
    FileTracker::instance()->addFile(file);

    if (typeScript)
    {
        file = _fileBase + ".d.ts";

        if (!dir.empty())
        {
            file = dir + '/' + file;
        }

        _typeScriptOutput.open(file.c_str());
        if (!_typeScriptOutput)
        {
            ostringstream os;
            os << "cannot open '" << file << "': " << IceInternal::errorToString(errno);
            throw FileException(os.str());
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
    : _javaScriptOutput(out),
      _typeScriptOutput(out),
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
    if (_javaScriptOutput.isOpen() || _useStdout)
    {
        _javaScriptOutput << '\n';
        _javaScriptOutput.close();
    }

    if (_typeScriptOutput.isOpen() || _useStdout)
    {
        _typeScriptOutput << '\n';
        _typeScriptOutput.close();
    }
}

void
Slice::Gen::generate(const UnitPtr& p)
{
    validateMetadata(p);

    string module = getJavaScriptModule(p->findDefinitionContext(p->topLevelFile()));

    if (_useStdout)
    {
        _javaScriptOutput << "\n";
        _javaScriptOutput << "/** slice2js: " << _fileBase << ".js generated begin module:\"" << module << "\" **/";
        _javaScriptOutput << "\n";
    }
    printHeader(_javaScriptOutput);
    printGeneratedHeader(_javaScriptOutput, _fileBase + ".ice");

    _javaScriptOutput << sp;
    _javaScriptOutput << nl << "/* eslint-disable */";
    _javaScriptOutput << nl << "/* jshint ignore: start */";
    _javaScriptOutput << nl;

    {
        ImportVisitor importVisitor(_javaScriptOutput, _includePaths);
        p->visit(&importVisitor);
        set<string> importedModules = importVisitor.writeImports(p);

        ExportsVisitor exportsVisitor(_javaScriptOutput, importedModules);
        p->visit(&exportsVisitor);
        set<string> exportedModules = exportsVisitor.exportedModules();

        set<string> seenModules = importedModules;
        seenModules.merge(exportedModules);

        TypesVisitor typesVisitor(_javaScriptOutput);
        p->visit(&typesVisitor);
    }

    if (_useStdout)
    {
        _javaScriptOutput << "\n";
        _javaScriptOutput << "/** slice2js: generated end **/";
        _javaScriptOutput << "\n";
    }

    if (_typeScript)
    {
        if (_useStdout)
        {
            _typeScriptOutput << "\n";
            _typeScriptOutput << "/** slice2js: " << _fileBase << ".d.ts generated begin module:\"" << module
                              << "\" **/";
            _typeScriptOutput << "\n";
        }
        printHeader(_typeScriptOutput);
        printGeneratedHeader(_typeScriptOutput, _fileBase + ".ice");

        TypeScriptImportVisitor importVisitor(_typeScriptOutput);
        p->visit(&importVisitor);
        map<string, string> importedTypes = importVisitor.writeImports();

        TypeScriptVisitor typeScriptVisitor(_typeScriptOutput, importedTypes);
        p->visit(&typeScriptVisitor);

        if (_useStdout)
        {
            _typeScriptOutput << "\n";
            _typeScriptOutput << "/** slice2js: generated end **/";
            _typeScriptOutput << "\n";
        }
    }
}

Slice::Gen::ImportVisitor::ImportVisitor(IceInternal::Output& out, vector<string> includePaths)
    : JsVisitor(out),
      _includePaths(std::move(includePaths))
{
    for (auto& includePath : _includePaths)
    {
        includePath = fullPath(includePath);
    }
}

bool
Slice::Gen::ImportVisitor::visitClassDefStart(const ClassDefPtr&)
{
    _seenClass = true;
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

set<string>
Slice::Gen::ImportVisitor::writeImports(const UnitPtr& p)
{
    // The JavaScript module we are building as specified by "js:module:" metadata.
    string jsModule = getJavaScriptModule(p->findDefinitionContext(p->topLevelFile()));
    set<string> importedModules = {"Ice"};

    // The imports map maps JavaScript Modules to the set of Slice top-level modules that are imported from the
    // generated JavaScript code. The key is either the JavaScript module specified by "js:module:" metadata or the
    // relative path of the generated JavaScript file.
    map<string, set<string>> imports;

    // The JavaScript files that we import in generated code when building Ice. The user generated code imports
    // the "ice" package.
    set<string> jsIceImports;
    if (jsModule == "@zeroc/ice")
    {
        bool needStreamHelpers = false;
        if (_seenClass || _seenInterface || _seenObjectSeq || _seenObjectDict)
        {
            jsIceImports.insert("DefaultSliceLoader");
            jsIceImports.insert("Object");
            jsIceImports.insert("Value");
            jsIceImports.insert("TypeRegistry");
            needStreamHelpers = true;
        }

        if (_seenInterface)
        {
            jsIceImports.insert("DefaultSliceLoader");
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
            jsIceImports.insert("DefaultSliceLoader");
            jsIceImports.insert("UserException");
            jsIceImports.insert("TypeRegistry");
        }

        if (_seenEnum)
        {
            jsIceImports.insert("EnumBase");
        }

        if (_seenDict || _seenObjectDict || _seenObjectProxyDict)
        {
            jsIceImports.insert("HashMap");
            jsIceImports.insert("HashUtil");
            needStreamHelpers = true;
        }

        if (_seenSeq || _seenObjectSeq)
        {
            needStreamHelpers = true;
        }

        if (needStreamHelpers)
        {
            jsIceImports.insert("StreamHelpers");
        }
    }
    else
    {
        imports["@zeroc/ice"] = set<string>{"Ice"};
    }

    StringList includes = p->includeFiles();

    // Iterate all the included files and generate an import statement for each top-level module in the included file.
    for (const auto& included : includes)
    {
        set<string> sliceTopLevelModules = p->getTopLevelModules(included);

        // The JavaScript module corresponding to the 'js:module' metadata in the included file.
        string jsImportedModule = getJavaScriptModule(p->findDefinitionContext(included));

        if (jsModule == jsImportedModule || jsImportedModule.empty())
        {
            // For Slice modules mapped to the same JavaScript module, or Slice files that doesn't use "js:module".
            // We import them using their Slice include relative path.
            string f = removeExtension(included) + ".js";
            if (IceInternal::isAbsolutePath(f))
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
    if (jsModule == "@zeroc/ice")
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
        // Import the required modules from "@zeroc/ice" JavaScript module.
        set<string> iceModules = imports["@zeroc/ice"];
        for (auto i = iceModules.begin(); i != iceModules.end();)
        {
            _out << nl << "import { ";
            _out << (*i);
            if (++i != iceModules.end())
            {
                _out << ", ";
            }
        }
        _out << " } from \"@zeroc/ice\";";
        // Remove Ice already imported from the list of imports.
        imports.erase("@zeroc/ice");
    }

    // Process the remaining imports, after importing "@zeroc/ice".
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

Slice::Gen::ExportsVisitor::ExportsVisitor(::IceInternal::Output& out, std::set<std::string> importedModules)
    : JsVisitor(out),
      _importedModules(std::move(importedModules))
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
    const string scoped = p->mappedScoped(".").substr(1);
    if (_exportedModules.insert(scoped).second)
    {
        if (p->isTopLevel())
        {
            if (_importedModules.find(scoped) == _importedModules.end())
            {
                _out << nl << "export const " << scoped << " = {};";
            }
            else
            {
                _out << nl << "export { " << scoped << " };";
            }
        }
        else
        {
            _out << nl << scoped << " = " << scoped << " || {};";
        }
    }
    return true;
}

set<string>
Slice::Gen::ExportsVisitor::exportedModules() const
{
    return _exportedModules;
}

Slice::Gen::TypesVisitor::TypesVisitor(IceInternal::Output& out) : JsVisitor(out) {}

bool
Slice::Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    const string scopedName = p->mappedScoped(".").substr(1);
    ClassDefPtr base = p->base();
    string baseRef = base ? base->mappedScoped(".").substr(1) : "Ice.Value";

    const DataMemberList dataMembers = p->dataMembers();
    const DataMemberList optionalMembers = p->orderedOptionalDataMembers();

    vector<string> allParamNames;
    for (const auto& member : p->allDataMembers())
    {
        allParamNames.push_back(member->mappedName());
    }

    vector<string> baseParamNames;
    DataMemberList baseDataMembers;

    if (base)
    {
        baseDataMembers = base->allDataMembers();
        for (const auto& baseDataMember : baseDataMembers)
        {
            baseParamNames.push_back(baseDataMember->mappedName());
        }
    }

    _out << sp;
    writeDocCommentFor(p);
    _out << nl << scopedName << " = class";
    _out << " extends " << baseRef;

    _out << sb;
    if (!allParamNames.empty())
    {
        _out << nl << "constructor" << spar;
        for (const auto& baseDataMember : baseDataMembers)
        {
            _out << baseDataMember->mappedName();
        }

        for (const auto& dataMember : dataMembers)
        {
            string value;
            if (dataMember->optional())
            {
                if (dataMember->defaultValue())
                {
                    value = writeConstantValue(
                        dataMember->type(),
                        dataMember->defaultValueType(),
                        *dataMember->defaultValue());
                }
                else
                {
                    value = "undefined";
                }
            }
            else
            {
                if (dataMember->defaultValue())
                {
                    value = writeConstantValue(
                        dataMember->type(),
                        dataMember->defaultValueType(),
                        *dataMember->defaultValue());
                }
                else
                {
                    value = getValue(dataMember->type());
                }
            }
            _out << (dataMember->mappedName() + (value.empty() ? value : (" = " + value)));
        }

        _out << epar << sb;
        _out << nl << "super" << spar << baseParamNames << epar << ';';
        for (const auto& member : dataMembers)
        {
            const string memberName = member->mappedName();
            _out << nl << "this." << memberName << " = " << memberName << ';';
        }
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

    _out << nl << "Ice.defineClass(" << scopedName << ", \"" << p->scoped() << "\"";
    if (p->compactId() != -1)
    {
        _out << ", " << p->compactId();
    }
    _out << ");";

    _out << sp;
    _out << nl << "Ice.TypeRegistry.declareValueType(\"" << scopedName << "\", " << scopedName << ");";

    return false;
}

bool
Slice::Gen::TypesVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    const string serviceType = p->mappedScoped(".").substr(1);
    const string proxyType = serviceType + "Prx";
    const string flattenedIdsName = "iceC_" + p->mappedScoped("_").substr(1) + "_ids";

    InterfaceList bases = p->bases();
    StringList ids = p->ids();

    _out << sp;
    _out << nl << "const " << flattenedIdsName << " = [";
    _out.inc();

    for (auto q = ids.begin(); q != ids.end(); ++q)
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
    writeDocCommentFor(p, false);
    _out << nl << serviceType << " = class extends Ice.Object";
    _out << sb;

    if (!bases.empty())
    {
        _out << sp;
        _out << nl << "static get _iceImplements()";
        _out << sb;
        _out << nl << "return [";
        _out.inc();
        for (auto q = bases.begin(); q != bases.end();)
        {
            InterfaceDefPtr base = *q;
            _out << nl << base->mappedScoped(".").substr(1);
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
    writeDocCommentFor(p);
    _out << nl << proxyType << " = class extends Ice.ObjectPrx";
    _out << sb;

    if (!bases.empty())
    {
        _out << sp;
        _out << nl << "static get _implements()";
        _out << sb;
        _out << nl << "return [";

        _out.inc();
        for (auto q = bases.begin(); q != bases.end();)
        {
            InterfaceDefPtr base = *q;

            _out << nl << base->mappedScoped(".").substr(1) + "Prx";
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
    _out << sp;
    _out << nl << "Ice.TypeRegistry.declareProxyType(\"" << proxyType << "\", " << proxyType << ");";

    _out << sp;
    _out << nl << "Ice.defineOperations(";
    _out.inc();
    _out << nl << serviceType << "," << nl << proxyType << "," << nl << flattenedIdsName << "," << nl << "\""
         << p->scoped() << "\"";

    const OperationList ops = p->operations();
    if (!ops.empty())
    {
        _out << ',';
        _out << sb;
        for (auto q = ops.begin(); q != ops.end(); ++q)
        {
            if (q != ops.begin())
            {
                _out << ',';
            }

            const OperationPtr& op = *q;
            const string opName = op->mappedName();
            const ParameterList paramList = op->parameters();
            const TypePtr ret = op->returnType();
            ParameterList inParams, outParams;
            for (const auto& pli : paramList)
            {
                if (pli->isOutParam())
                {
                    outParams.push_back(pli);
                }
                else
                {
                    inParams.push_back(pli);
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
            _out << ", " << sliceModeToIceMode(op->mode()); // Mode.
            _out << ", ";

            if (op->format())
            {
                _out << opFormatTypeToString(*op->format()); // Format.
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

            // User exceptions.
            // Arrange exceptions into most-derived to least-derived order. If we don't
            // do this, a base exception handler can appear before a derived exception
            // handler, causing compiler warnings and resulting in the base exception
            // being marshaled instead of the derived exception.
            ExceptionList throws = op->throws();
            throws.sort(Slice::DerivedToBaseCompare());

            if (throws.empty())
            {
                _out << " ";
            }
            else
            {
                _out << nl << '[';
                _out.inc();
                for (auto eli = throws.begin(); eli != throws.end(); ++eli)
                {
                    if (eli != throws.begin())
                    {
                        _out << ',';
                    }
                    _out << nl << (*eli)->mappedScoped(".").substr(1);
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
    // Stream helpers for sequences are lazy initialized as the required types might not be available until later.
    const string helperName = p->mappedScoped(".").substr(1) + "Helper";
    const TypePtr type = p->type();
    const bool fixed = !type->isVariableLength();

    _out << sp;
    _out << nl << helperName << " = Ice.StreamHelpers.generateSeqHelper(" << getHelper(type) << ", "
         << (fixed ? "true" : "false");
    if (type->isClassType())
    {
        _out << ", \"" << typeToJsString(type) << "\"";
    }

    _out << ");";
}

bool
Slice::Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    const string scopedName = p->mappedScoped(".").substr(1);
    const ExceptionPtr base = p->base();
    string baseRef;

    if (base)
    {
        baseRef = base->mappedScoped(".").substr(1);
    }
    else
    {
        baseRef = "Ice.UserException";
    }

    const DataMemberList dataMembers = p->dataMembers();
    const DataMemberList optionalMembers = p->orderedOptionalDataMembers();

    vector<string> baseParamNames;
    DataMemberList baseDataMembers;

    if (base)
    {
        baseDataMembers = base->allDataMembers();
        for (const auto& baseDataMember : baseDataMembers)
        {
            baseParamNames.push_back(baseDataMember->mappedName());
        }
    }

    _out << sp;
    writeDocCommentFor(p);
    _out << nl << scopedName << " = class extends " << baseRef;
    _out << sb;

    _out << nl << "constructor" << spar;

    for (const auto& baseDataMember : baseDataMembers)
    {
        _out << baseDataMember->mappedName();
    }

    for (const auto& dataMember : dataMembers)
    {
        string value;
        if (dataMember->optional())
        {
            if (dataMember->defaultValue())
            {
                value =
                    writeConstantValue(dataMember->type(), dataMember->defaultValueType(), *dataMember->defaultValue());
            }
            else
            {
                value = "undefined";
            }
        }
        else
        {
            if (dataMember->defaultValue())
            {
                value =
                    writeConstantValue(dataMember->type(), dataMember->defaultValueType(), *dataMember->defaultValue());
            }
            else
            {
                value = getValue(dataMember->type());
            }
        }
        _out << (dataMember->mappedName() + (value.empty() ? value : (" = " + value)));
    }

    _out << "_cause = \"\"" << epar;
    _out << sb;
    _out << nl << "super" << spar << baseParamNames << "_cause" << epar << ';';
    for (const auto& member : dataMembers)
    {
        const string memberName = member->mappedName();
        _out << nl << "this." << memberName << " = " << memberName << ';';
    }
    _out << eb;

    _out << sp;
    _out << nl << "static get _parent()";
    _out << sb;
    _out << nl << "return " << baseRef << ";";
    _out << eb;

    // TODO: use methods added by Ice.defineClass instead.
    _out << sp;
    _out << nl << "static get _ice_id()";
    _out << sb;
    _out << nl << "return \"" << p->scoped() << "\";";
    _out << eb;

    _out << sp;
    _out << nl << "_mostDerivedType()";
    _out << sb;
    _out << nl << "return " << scopedName << ";";
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
    _out << sp;

    _out << nl << "Ice.defineClass(" << scopedName << ", \"" << p->scoped() << "\");";

    return false;
}

bool
Slice::Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    const string scopedName = p->mappedScoped(".").substr(1);
    const DataMemberList dataMembers = p->dataMembers();

    _out << sp;
    writeDocCommentFor(p);
    _out << nl << scopedName << " = class";
    _out << sb;

    _out << nl << "constructor" << spar;

    for (const auto& dataMember : dataMembers)
    {
        string value;
        if (dataMember->optional())
        {
            if (dataMember->defaultValue())
            {
                value =
                    writeConstantValue(dataMember->type(), dataMember->defaultValueType(), *dataMember->defaultValue());
            }
            else
            {
                value = "undefined";
            }
        }
        else
        {
            if (dataMember->defaultValue())
            {
                value =
                    writeConstantValue(dataMember->type(), dataMember->defaultValueType(), *dataMember->defaultValue());
            }
            else
            {
                value = getValue(dataMember->type());
            }
        }
        _out << (dataMember->mappedName() + (value.empty() ? value : (" = " + value)));
    }

    _out << epar;
    _out << sb;
    for (const auto& member : dataMembers)
    {
        const string memberName = member->mappedName();
        _out << nl << "this." << memberName << " = " << memberName << ';';
    }
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
    bool legalKeyType = Dictionary::isLegalKeyType(p);

    _out << sp;
    _out << nl << "Ice.defineStruct(" << scopedName << ", " << (legalKeyType ? "true" : "false") << ", "
         << (p->isVariableLength() ? "true" : "false") << ");";
    return false;
}

void
Slice::Gen::TypesVisitor::visitDictionary(const DictionaryPtr& p)
{
    const TypePtr keyType = p->keyType();
    const TypePtr valueType = p->valueType();

    // For some key types, we have to use an equals() method to compare keys
    // rather than the native comparison operators.
    bool keyUseEquals = false;
    BuiltinPtr b = dynamic_pointer_cast<Builtin>(keyType);
    if ((b && b->kind() == Builtin::KindLong) || dynamic_pointer_cast<Struct>(keyType))
    {
        keyUseEquals = true;
    }

    // Stream helpers for dictionaries of objects are lazy initialized
    // as the required object type might not be available until later.
    const string scopedName = p->mappedScoped(".").substr(1);
    const string helperName = scopedName + "Helper";
    bool fixed = !keyType->isVariableLength() && !valueType->isVariableLength();

    _out << sp;
    _out << nl << "[" << scopedName << ", " << helperName << "] = Ice.defineDictionary(" << getHelper(keyType) << ", "
         << getHelper(valueType) << ", " << (fixed ? "true" : "false") << ", "
         << (keyUseEquals ? "Ice.HashMap.compareEquals" : "undefined");

    if (valueType->isClassType())
    {
        _out << ", \"" << typeToJsString(valueType) << "\"";
    }
    _out << ");";
}

void
Slice::Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    const string scopedName = p->mappedScoped(".").substr(1);
    _out << sp;
    writeDocCommentFor(p);
    _out << nl << scopedName << " = Ice.defineEnum([";
    _out.inc();
    _out << nl;

    const EnumeratorList enumerators = p->enumerators();
    int i = 0;
    for (auto en = enumerators.begin(); en != enumerators.end(); ++en)
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
        _out << "['" << (*en)->mappedName() << "', " << (*en)->value() << ']';
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
    string scope = p->mappedScope(".").substr(1);
    scope.pop_back(); // Remove the trailing '.' from the scope.

    _out << sp;
    _out << nl << "Object.defineProperty(" << scope << ", '" << p->mappedName() << "', {";
    _out.inc();
    _out << nl << "enumerable: true,";
    _out << nl << "value: ";
    _out << writeConstantValue(p->type(), p->valueType(), p->value());
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
        return "\"" + proxy->mappedScoped(".").substr(1) + "Prx" + "\"";
    }

    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    if (seq)
    {
        return seq->mappedScoped(".").substr(1) + "Helper";
    }

    DictionaryPtr d = dynamic_pointer_cast<Dictionary>(type);
    if (d)
    {
        return d->mappedScoped(".").substr(1) + "Helper";
    }

    EnumPtr e = dynamic_pointer_cast<Enum>(type);
    if (e)
    {
        return e->mappedScoped(".").substr(1) + "._helper";
    }

    StructPtr st = dynamic_pointer_cast<Struct>(type);
    if (st)
    {
        return st->mappedScoped(".").substr(1);
    }

    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
    if (cl)
    {
        return "\"" + cl->mappedScoped(".").substr(1) + "\"";
    }

    return "???";
}

Slice::Gen::TypeScriptImportVisitor::TypeScriptImportVisitor(IceInternal::Output& out) : JsVisitor(out) {}

void
Slice::Gen::TypeScriptImportVisitor::addImport(const ContainedPtr& definition)
{
    const string definitionId = definition->mappedScoped(".").substr(1);

    string jsImportedModule = getJavaScriptModule(definition->definitionContext());
    if (jsImportedModule.empty())
    {
        string definedIn = definition->getMetadataArgs("js:defined-in").value_or("");
        if (!definedIn.empty())
        {
            _importedTypes[definitionId] = "__module_" + pathToModule(definedIn) + ".";
            _importedModules.insert(removeExtension(definedIn) + ".js");
        }
        else
        {
            const string filename = definition->definitionContext()->filename();
            if (filename == _filename)
            {
                // For types defined in the same unit we use a __global_ prefix to refer to the global scope.
                _importedTypes[definitionId] = "__global_";
            }
            else
            {
                string f = filename;
                if (IceInternal::isAbsolutePath(f))
                {
                    // If the include file is an absolute path, we need to generate a relative path.
                    f = relativePath(f, _filename);
                }
                else if (f[0] != '.')
                {
                    // Make the import relative to the current file.
                    f = "./" + f;
                }
                _importedTypes[definitionId] = "__module_" + pathToModule(f) + ".";
                _importedModules.insert(removeExtension(f) + ".js");
            }
        }
    }
    else if (_module != jsImportedModule)
    {
        _importedTypes[definitionId] = "__module_" + pathToModule(jsImportedModule) + ".";
    }
    else
    {
        // For types defined in the same module, we use a __global_ prefix, to refer to the global scope.
        _importedTypes[definitionId] = "__global_";
    }
}

bool
Slice::Gen::TypeScriptImportVisitor::visitUnitStart(const UnitPtr& unit)
{
    _module = getJavaScriptModule(unit->findDefinitionContext(unit->topLevelFile()));
    _filename = unit->topLevelFile();
    if (_module != "@zeroc/ice")
    {
        _importedModules.insert("@zeroc/ice");
    }
    StringList includes = unit->includeFiles();
    // Iterate all the included files and generate an import statement for each top-level module in the included file.
    for (const auto& included : includes)
    {
        // The JavaScript module corresponding to the 'js:module' metadata in the included file.
        string jsImportedModule = getJavaScriptModule(unit->findDefinitionContext(included));

        if (_module != jsImportedModule)
        {
            if (jsImportedModule.empty())
            {
                string f = removeExtension(included) + ".js";
                if (IceInternal::isAbsolutePath(f))
                {
                    // If the include file is an absolute path, we need to generate a relative path.
                    f = relativePath(f, _filename);
                }
                else if (f[0] != '.')
                {
                    // Make the import relative to the current file.
                    f = "./" + f;
                }
                _importedModules.insert(f);
            }
            else
            {
                _importedModules.insert(jsImportedModule);
            }
        }
    }
    return true;
}

bool
Slice::Gen::TypeScriptImportVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    // Add imports required for the base class type.
    ClassDefPtr base = p->base();
    if (base)
    {
        addImport(dynamic_pointer_cast<Contained>(base));
    }

    // Add imports required for data members types.
    for (const auto& dataMember : p->allDataMembers())
    {
        ContainedPtr type = dynamic_pointer_cast<Contained>(dataMember->type());
        if (type)
        {
            addImport(type);
        }
    }
    return false;
}

bool
Slice::Gen::TypeScriptImportVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    // Add imports required for base interfaces types.
    for (const auto& base : p->bases())
    {
        addImport(base);
    }

    // Add imports required for operation parameters and return types.
    for (const auto& op : p->allOperations())
    {
        auto ret = dynamic_pointer_cast<Contained>(op->returnType());
        if (ret)
        {
            addImport(ret);
        }

        for (const auto& param : op->parameters())
        {
            auto type = dynamic_pointer_cast<Contained>(param->type());
            if (type)
            {
                addImport(type);
            }
        }
    }
    return false;
}

bool
Slice::Gen::TypeScriptImportVisitor::visitStructStart(const StructPtr& p)
{
    // Add imports required for data member types.
    for (const auto& dataMember : p->dataMembers())
    {
        auto type = dynamic_pointer_cast<Contained>(dataMember->type());
        if (type)
        {
            addImport(type);
        }
    }
    return false;
}

bool
Slice::Gen::TypeScriptImportVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    // Add imports required for base exception types.
    ExceptionPtr base = p->base();
    if (base)
    {
        addImport(dynamic_pointer_cast<Contained>(base));
    }

    const DataMemberList allDataMembers = p->allDataMembers();
    for (const auto& dataMember : p->allDataMembers())
    {
        auto type = dynamic_pointer_cast<Contained>(dataMember->type());
        if (type)
        {
            addImport(type);
        }
    }
    return false;
}

void
Slice::Gen::TypeScriptImportVisitor::visitSequence(const SequencePtr& seq)
{
    // Add import required for the sequence element type.
    auto type = dynamic_pointer_cast<Contained>(seq->type());
    if (type)
    {
        addImport(type);
    }
}

void
Slice::Gen::TypeScriptImportVisitor::visitDictionary(const DictionaryPtr& dict)
{
    //
    // Add imports required for the dictionary key and value types
    //
    auto keyType = dynamic_pointer_cast<Contained>(dict->keyType());
    if (keyType)
    {
        addImport(keyType);
    }

    auto valueType = dynamic_pointer_cast<Contained>(dict->valueType());
    if (valueType)
    {
        addImport(valueType);
    }
}

std::map<std::string, std::string>
Slice::Gen::TypeScriptImportVisitor::writeImports()
{
    _out << sp;
    for (const auto& moduleName : _importedModules)
    {
        _out << nl << "import * as __module_" << pathToModule(moduleName) << " from \"" << moduleName << "\";";
    }
    return _importedTypes;
}

string
Slice::Gen::TypeScriptVisitor::importPrefix(const string& s) const
{
    const auto it = _importedTypes.find(s);
    if (it != _importedTypes.end())
    {
        return it->second;
    }
    return "";
}

string
Slice::Gen::TypeScriptVisitor::typeToTsString(const TypePtr& type, bool nullable, bool forParameter, bool optional)
    const
{
    if (!type)
    {
        assert(!optional);
        return "void";
    }

    string t;

    static const char* typeScriptBuiltinTable[] = {
        "number",  // byte
        "boolean", // bool
        "number",  // short
        "number",  // int
        "BigInt",  // long
        "number",  // float
        "number",  // double
        "string",
        "Ice.Value", // Ice.Object
        "Ice.ObjectPrx",
        "Ice.Value"};

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindLong:
            {
                t = typeScriptBuiltinTable[builtin->kind()];
                // For Slice long parameters we accept both number and BigInt.
                if (forParameter)
                {
                    t += " | number";
                }
                break;
            }
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindValue:
            {
                t = _iceImportPrefix + typeScriptBuiltinTable[builtin->kind()];
                if (nullable)
                {
                    t += " | null";
                }
                break;
            }
            default:
            {
                t = typeScriptBuiltinTable[builtin->kind()];
                break;
            }
        }
    }

    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
    if (cl)
    {
        const string scopedName = cl->mappedScoped(".").substr(1);
        t = importPrefix(scopedName) + scopedName;
        if (nullable)
        {
            t += " | null";
        }
    }

    InterfaceDeclPtr proxy = dynamic_pointer_cast<InterfaceDecl>(type);
    if (proxy)
    {
        const string scopedName = proxy->mappedScoped(".").substr(1);
        t = importPrefix(scopedName) + scopedName + "Prx";
        if (nullable)
        {
            t += " | null";
        }
    }

    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    if (seq)
    {
        builtin = dynamic_pointer_cast<Builtin>(seq->type());
        if (builtin && builtin->kind() == Builtin::KindByte)
        {
            t = "Uint8Array";
        }
        else
        {
            const string seqType = typeToTsString(seq->type(), nullable);
            if (seqType.find('|') != string::npos)
            {
                t = "(" + seqType + ")[]";
            }
            else
            {
                t = seqType + "[]";
            }
        }
    }

    DictionaryPtr d = dynamic_pointer_cast<Dictionary>(type);
    if (d)
    {
        const TypePtr keyType = d->keyType();
        builtin = dynamic_pointer_cast<Builtin>(keyType);
        ostringstream os;
        if ((builtin && builtin->kind() == Builtin::KindLong) || dynamic_pointer_cast<Struct>(keyType))
        {
            os << _iceImportPrefix << "Ice.HashMap<";
        }
        else
        {
            os << "Map<";
        }
        os << typeToTsString(d->keyType(), nullable) << ", " << typeToTsString(d->valueType(), nullable) << ">";
        t = os.str();
    }

    ContainedPtr contained = dynamic_pointer_cast<Contained>(type);
    if (t.empty() && contained)
    {
        const string scopedName = contained->mappedScoped(".").substr(1);
        t = importPrefix(scopedName) + scopedName;
    }

    // For optional parameters we use "?:" instead of Ice.Optional<T>
    if (optional)
    {
        t += " | undefined";
    }

    return t;
}

Slice::Gen::TypeScriptVisitor::TypeScriptVisitor(
    IceInternal::Output& out,
    std::map<std::string, std::string> importedTypes)
    : JsVisitor(out),
      _importedTypes(std::move(importedTypes))
{
}

bool
Slice::Gen::TypeScriptVisitor::visitUnitStart(const UnitPtr& unit)
{
    _module = getJavaScriptModule(unit->findDefinitionContext(unit->topLevelFile()));
    _iceImportPrefix = _module == "@zeroc/ice" ? "" : "__module__zeroc_ice.";
    if (!_module.empty())
    {
        _out << sp;
        _out << nl << "declare module \"" << _module << "\"" << sb;
    }
    return true;
}

void
Slice::Gen::TypeScriptVisitor::visitUnitEnd(const UnitPtr&)
{
    set<string> globalModules;
    for (const auto& [key, value] : _importedTypes)
    {
        if (value == "__global_")
        {
            // find the top level module for the type.
            auto pos = key.find('.');
            assert(pos != string::npos);

            globalModules.insert(key.substr(0, pos));
        }
    }

    for (const auto& moduleName : globalModules)
    {
        _out << nl << "import __global_" << moduleName << " = " << moduleName << ";";
    }

    if (!_module.empty())
    {
        _out << eb; // module end
    }
}

bool
Slice::Gen::TypeScriptVisitor::visitModuleStart(const ModulePtr& p)
{
    if (p->isTopLevel() && _module.empty())
    {
        _out << sp;
        _out << nl << "export namespace " << p->mappedName() << sb;
    }
    else
    {
        _out << nl << "namespace " << p->mappedName() << sb;
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
    const DataMemberList dataMembers = p->dataMembers();
    const DataMemberList allDataMembers = p->allDataMembers();
    _out << sp;
    writeDocCommentFor(p);
    _out << nl << "export class " << p->mappedName() << " extends ";
    ClassDefPtr base = p->base();
    if (base)
    {
        const string baseName = base->mappedScoped(".").substr(1);
        _out << importPrefix(baseName) << baseName;
    }
    else
    {
        _out << _iceImportPrefix << "Ice.Value";
    }
    _out << sb;
    _out << nl << "/**";
    _out << nl << " * One-shot constructor to initialize all data members.";
    for (const auto& dataMember : allDataMembers)
    {
        if (auto comment = DocComment::parseFrom(dataMember, jsLinkFormatter))
        {
            _out << nl << " * @param " << dataMember->mappedName() << " " << getDocSentence(comment->overview());
        }
    }
    _out << nl << " */";
    _out << nl << "constructor" << spar;
    for (const auto& dataMember : allDataMembers)
    {
        _out << (dataMember->mappedName() + "?: " + typeToTsString(dataMember->type()));
    }
    _out << epar << ";";
    for (const auto& dataMember : dataMembers)
    {
        _out << sp;
        writeDocCommentFor(dataMember);
        _out << nl << dataMember->mappedName() << ": " << typeToTsString(dataMember->type(), true) << ";";
    }
    _out << eb;

    return false;
}

namespace
{
    bool areRemainingParamsOptional(const ParameterList& params, const string& name)
    {
        auto it = params.begin();
        do
        {
            it++;
        } while (it != params.end() && (*it)->name() != name);

        for (; it != params.end(); ++it)
        {
            if (!(*it)->optional())
            {
                return false;
            }
        }
        return true;
    }
}

void
Slice::Gen::TypeScriptVisitor::writeOpDocSummary(Output& out, const OperationPtr& op, bool forDispatch)
{
    out << nl << "/**";

    map<string, StringList> paramDoc;
    optional<DocComment> comment = DocComment::parseFrom(op, jsLinkFormatter);
    if (comment)
    {
        if (!comment->overview().empty())
        {
            writeDocLines(out, comment->overview(), true);
        }

        paramDoc = comment->parameters();
        for (const auto& param : op->inParameters())
        {
            auto q = paramDoc.find(param->name());
            if (q != paramDoc.end())
            {
                out << nl << " * @param " << param->mappedName() << " ";
                writeDocLines(out, q->second, false);
            }
        }
    }

    if (forDispatch)
    {
        const string currentParam = escapeParam(op->inParameters(), "current");
        out << nl << " * @param " + currentParam + " The Current object for the dispatch.";
        out << nl << " * @returns A promise like object representing the result of the dispatch";
    }
    else
    {
        const string contextParam = escapeParam(op->inParameters(), "context");
        out << nl << " * @param " + contextParam + " The Context map to send with the invocation.";
        out << nl << " * @returns An {@link Ice.AsyncResult} object representing the result of the invocation";
    }

    if (op->returnsMultipleValues())
    {
        out << ", which resolves to an array with the following entries:";
    }
    else if (op->returnsAnyValues())
    {
        out << ", which resolves to:";
    }
    else
    {
        out << ".";
    }

    const TypePtr ret = op->returnType();
    if (comment && ret)
    {
        out << nl << " * - " << typeToTsString(ret, true, false, op->returnIsOptional()) << " : ";
        writeDocLines(out, comment->returns(), false, "   ");
    }

    for (const auto& param : op->outParameters())
    {
        auto q = paramDoc.find(param->name());
        if (q != paramDoc.end())
        {
            out << nl << " * - " << typeToTsString(param->type(), true, false, param->optional()) << " : ";
            writeDocLines(out, q->second, false, "   ");
        }
    }

    if (comment)
    {
        writeOpDocExceptions(out, op, *comment);

        if (!comment->seeAlso().empty())
        {
            writeSeeAlso(out, comment->seeAlso());
        }
    }

    if (!forDispatch)
    {
        writeDeprecated(out, comment, op);
    }

    out << nl << " */";
}

bool
Slice::Gen::TypeScriptVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    //
    // Define servant an proxy types
    //
    const string prxName = p->mappedName() + "Prx";
    _out << sp;
    writeDocCommentFor(p);
    _out << nl << "export class " << prxName << " extends " << _iceImportPrefix << "Ice.ObjectPrx";
    _out << sb;

    _out << sp;
    _out << nl << "/**";
    _out << nl << " * Constructs a new " << prxName << " proxy.";
    _out << nl << " * @param communicator - The communicator for the new proxy.";
    _out << nl << " * @param proxyString - The string representation of the proxy.";
    _out << nl << " * @returns The new " << prxName << " proxy.";
    _out << nl << " * @throws ParseException - Thrown if the proxyString is not a valid proxy string.";
    _out << nl << " */";
    _out << nl << "constructor(communicator: " << _iceImportPrefix << "Ice.Communicator, proxyString: string);";

    _out << sp;
    _out << nl << "/**";
    _out << nl << " * Constructs a new " << prxName << " proxy from an ObjectPrx. The new proxy is a clone of the";
    _out << nl << " * provided proxy.";
    _out << nl << " * @param prx - The proxy to clone.";
    _out << nl << " * @returns The new " << prxName << " proxy.";
    _out << nl << " */";
    _out << nl << "constructor(prx: " << _iceImportPrefix << "Ice.ObjectPrx);";

    for (const auto& op : p->allOperations())
    {
        const ParameterList paramList = op->parameters();
        const TypePtr ret = op->returnType();
        ParameterList inParams, outParams;
        for (const auto& param : paramList)
        {
            if (param->isOutParam())
            {
                outParams.push_back(param);
            }
            else
            {
                inParams.push_back(param);
            }
        }

        _out << sp;
        writeOpDocSummary(_out, op, false);
        _out << nl << op->mappedName() << spar;
        for (const auto& param : inParams)
        {
            // TypeScript doesn't allow optional parameters with '?' prefix before required parameters.
            const string optionalPrefix =
                param->optional() && areRemainingParamsOptional(paramList, param->name()) ? "?" : "";
            _out
                << (param->mappedName() + optionalPrefix + ": " +
                    typeToTsString(param->type(), true, true, param->optional()));
        }
        _out << "context?: Map<string, string>";
        _out << epar;

        _out << ": " << _iceImportPrefix << "Ice.AsyncResult";
        if (!ret && outParams.empty())
        {
            _out << "<void>";
        }
        else if ((ret && outParams.empty()) || (!ret && outParams.size() == 1))
        {
            TypePtr t = ret ? ret : outParams.front()->type();
            bool optional = ret ? op->returnIsOptional() : outParams.front()->optional();
            _out << "<" << typeToTsString(t, true, false, optional) << ">";
        }
        else
        {
            _out << "<[";
            if (ret)
            {
                _out << typeToTsString(ret, true, false, op->returnIsOptional()) << ", ";
            }

            for (auto i = outParams.begin(); i != outParams.end();)
            {
                _out << typeToTsString((*i)->type(), true, false, (*i)->optional());
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
    _out << nl << " * @returns A proxy with the requested type.";
    _out << nl << " */";
    _out << nl << "static uncheckedCast(prx: " << _iceImportPrefix << "Ice.ObjectPrx"
         << ", "
         << "facet?: string): " << prxName << ";";

    _out << sp;
    _out << nl << "/**";
    _out << nl << " * Downcasts a proxy after confirming the target object's type via a remote invocation.";
    _out << nl << " * @param prx The target proxy.";
    _out << nl << " * @param facet A facet name.";
    _out << nl << " * @param context The request context.";
    _out << nl
         << " * @returns A proxy with the requested type and facet, or nil if the target proxy is nil or the target";
    _out << nl << " * object does not support the requested type.";
    _out << nl << " */";
    _out << nl << "static checkedCast(prx: " << _iceImportPrefix << "Ice.ObjectPrx"
         << ", "
         << "facet?: string, context?: Map<string, string>): " << _iceImportPrefix << "Ice.AsyncResult"
         << "<" << prxName << " | null>;";
    _out << eb;

    _out << sp;
    writeDocCommentFor(p, false);
    _out << nl << "export abstract class " << p->mappedName() << " extends " << _iceImportPrefix << "Ice.Object";
    _out << sb;
    for (const auto& op : p->allOperations())
    {
        const ParameterList paramList = op->parameters();
        const TypePtr ret = op->returnType();
        ParameterList inParams, outParams;
        for (const auto& param : paramList)
        {
            if (param->isOutParam())
            {
                outParams.push_back(param);
            }
            else
            {
                inParams.push_back(param);
            }
        }

        _out << sp;
        writeOpDocSummary(_out, op, true);
        _out << nl << "abstract " << op->mappedName() << spar;
        for (const auto& param : inParams)
        {
            _out << (param->mappedName() + ": " + typeToTsString(param->type(), true, true, param->optional()));
        }
        _out << ("current: " + _iceImportPrefix + "Ice.Current");
        _out << epar << ": ";

        if (!ret && outParams.empty())
        {
            _out << "PromiseLike<void>|void";
        }
        else if ((ret && outParams.empty()) || (!ret && outParams.size() == 1))
        {
            TypePtr t = ret ? ret : outParams.front()->type();
            string returnType = typeToTsString(t, true, false, op->returnIsOptional());
            _out << "PromiseLike<" << returnType << ">|" << returnType;
        }
        else
        {
            ostringstream os;
            if (ret)
            {
                os << typeToTsString(ret, true, false, op->returnIsOptional()) << ", ";
            }

            for (auto i = outParams.begin(); i != outParams.end();)
            {
                os << typeToTsString((*i)->type(), true, false, (*i)->optional());
                if (++i != outParams.end())
                {
                    os << ", ";
                }
            }
            _out << "PromiseLike<[" << os.str() << "]>|[" << os.str() << "]";
        }
        _out << ";";
    }

    _out << sp;
    _out << nl << "/**";
    _out << nl << " * Obtains the Slice type ID of this type.";
    _out << nl << " * @returns The return value is always \"" + p->scoped() + "\".";
    _out << nl << " */";
    _out << nl << "static ice_staticId(): string;";
    _out << eb;

    return false;
}

bool
Slice::Gen::TypeScriptVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    const DataMemberList dataMembers = p->dataMembers();
    const DataMemberList allDataMembers = p->allDataMembers();

    ExceptionPtr base = p->base();
    string baseRef;
    if (base)
    {
        const string baseName = base->mappedScoped(".").substr(1);
        baseRef = importPrefix(baseName) + baseName;
    }
    else
    {
        baseRef = _iceImportPrefix + "Ice.UserException";
    }

    _out << sp;
    writeDocCommentFor(p);
    _out << nl << "export class " << p->mappedName() << " extends " << baseRef << sb;
    if (!allDataMembers.empty())
    {
        _out << nl << "/**";
        _out << nl << " * One-shot constructor to initialize all data members.";
        for (const auto& dataMember : allDataMembers)
        {
            if (auto comment = DocComment::parseFrom(dataMember, jsLinkFormatter))
            {
                _out << nl << " * @param " << dataMember->mappedName() << " " << getDocSentence(comment->overview());
            }
        }
        _out << nl << " */";
        _out << nl << "constructor" << spar;
        for (const auto& dataMember : allDataMembers)
        {
            _out << (dataMember->mappedName() + "?: " + typeToTsString(dataMember->type()));
        }
        _out << epar << ";";
    }

    for (const auto& dataMember : allDataMembers)
    {
        const string optionalModifier = dataMember->optional() ? "?" : "";
        _out << nl << dataMember->mappedName() << optionalModifier << ": " << typeToTsString(dataMember->type(), true)
             << ";";
    }
    _out << eb;
    return false;
}

bool
Slice::Gen::TypeScriptVisitor::visitStructStart(const StructPtr& p)
{
    const string name = p->mappedName();
    const DataMemberList dataMembers = p->dataMembers();

    _out << sp;
    writeDocCommentFor(p);
    _out << nl << "export class " << name << sb;
    _out << nl << "constructor" << spar;
    for (const auto& dataMember : dataMembers)
    {
        // TODO why are all parameters optional?
        _out << (dataMember->mappedName() + "?: " + typeToTsString(dataMember->type()));
    }
    _out << epar << ";";

    _out << sp;
    _out << nl << "/**";
    _out << nl << " * A deep copy of the current object.";
    _out << nl << " *";
    _out << nl << " * @returns A deep copy of the current object.";
    _out << nl << " */";
    _out << nl << "clone(): " << name << ";";

    _out << sp;
    _out << nl << "/**";
    _out << nl << " * Determines whether the specified object is equal to the current object.";
    _out << nl << " *";
    _out << nl << " * @param other The object to compare with the current object.";
    _out << nl << " * @returns `true` if the specified object is equal to the current object, `false` otherwise.";
    _out << nl << " */";
    _out << nl << "equals(other: any): boolean;";

    // Only generate hashCode if this structure type is a legal dictionary key type.
    if (Dictionary::isLegalKeyType(p))
    {
        _out << sp;
        _out << nl << "/**";
        _out << nl << " * Returns the hash code of the object.";
        _out << nl << " *";
        _out << nl << " * @returns The hash code of the object.";
        _out << nl << " */";
        _out << nl << "hashCode(): number;";
    }

    _out << sp;
    _out << nl << "/**";
    _out << nl << " * Writes the {@link " << name << "} value to the given OutputStream.";
    _out << nl << " *";
    _out << nl << " * @param outs The OutputStream to write to.";
    _out << nl << " * @param value The value to write.";
    _out << nl << " */";
    _out << nl << "static write(outs: " << _iceImportPrefix << "Ice.OutputStream, value:" << name << "): void;";

    _out << sp;
    _out << nl << "/**";
    _out << nl << " * Reads {@link " << name << "} from the given InputStream.";
    _out << nl << " *";
    _out << nl << " * @param ins The InputStream to read from.";
    _out << nl << " * @returns The read {@link " << name << "} value.";
    _out << nl << " */";
    _out << nl << "static read(ins: " << _iceImportPrefix << "Ice.InputStream): " << name << ";";

    for (const auto& dataMember : dataMembers)
    {
        _out << sp;
        writeDocCommentFor(dataMember);
        _out << nl << dataMember->mappedName() << ":" << typeToTsString(dataMember->type(), true) << ";";
    }

    _out << eb;
    return false;
}

void
Slice::Gen::TypeScriptVisitor::visitSequence(const SequencePtr& p)
{
    const string name = p->mappedName();

    _out << sp;
    writeDocCommentFor(p);
    _out << nl << "export type " << name << " = " << typeToTsString(p) << ";";

    _out << sp;
    _out << nl << "/**";
    _out << nl << " * Helper class for encoding {@link " << name << "} into an OutputStream and";
    _out << nl << " * decoding {@link " << name << "} from an InputStream.";
    _out << nl << " */";
    _out << nl << "export class " << name + "Helper";
    _out << sb;

    _out << nl << "/**";
    _out << nl << " * Writes the {@link " << name << "} value to the given OutputStream.";
    _out << nl << " *";
    _out << nl << " * @param outs The OutputStream to write to.";
    _out << nl << " * @param value The value to write.";
    _out << nl << " */";
    _out << nl << "static write(outs: " << _iceImportPrefix << "Ice.OutputStream, value: " << name << "): void;";

    _out << sp;
    _out << nl << "/**";
    _out << nl << " * Reads {@link " << name << "} from the given InputStream.";
    _out << nl << " *";
    _out << nl << " * @param ins The InputStream to read from.";
    _out << nl << " * @returns The read {@link " << name << "} value.";
    _out << nl << " */";
    _out << nl << "static read(ins: " << _iceImportPrefix << "Ice.InputStream): " << name << ";";
    _out << eb;
}

void
Slice::Gen::TypeScriptVisitor::visitDictionary(const DictionaryPtr& p)
{
    const string name = p->mappedName();
    _out << sp;
    writeDocCommentFor(p);
    string dictionaryType = typeToTsString(p);
    if (dictionaryType.find("Ice.") == 0)
    {
        dictionaryType = _iceImportPrefix + dictionaryType;
    }
    _out << nl << "export class " << name << " extends " << dictionaryType;
    _out << sb;
    _out << eb;

    _out << sp;
    _out << nl << "/**";
    _out << nl << " * Helper class for encoding {@link " << name << "} into an OutputStream and";
    _out << nl << " * decoding {@link " << name << "} from an InputStream.";
    _out << nl << " */";
    _out << nl << "export class " << name + "Helper";
    _out << sb;

    _out << nl << "/**";
    _out << nl << " * Writes the {@link " << name << "} value to the given OutputStream.";
    _out << nl << " *";
    _out << nl << " * @param outs The OutputStream to write to.";
    _out << nl << " * @param value The value to write.";
    _out << nl << " */";
    _out << nl << "static write(outs: " << _iceImportPrefix << "Ice.OutputStream, value: " << name << "): void;";

    _out << sp;
    _out << nl << "/**";
    _out << nl << " * Reads {@link " << name << "} from the given InputStream.";
    _out << nl << " *";
    _out << nl << " * @param ins The InputStream to read from.";
    _out << nl << " * @returns The read {@link " << name << "} value.";
    _out << nl << " */";
    _out << nl << "static read(ins: " << _iceImportPrefix << "Ice.InputStream): " << name << ";";
    _out << eb;
}

void
Slice::Gen::TypeScriptVisitor::visitEnum(const EnumPtr& p)
{
    const string name = p->mappedName();

    _out << sp;
    writeDocCommentFor(p);
    _out << nl << "export class " << name << " extends " << _iceImportPrefix << "Ice.EnumBase";
    _out << sb;
    for (const auto& enumerator : p->enumerators())
    {
        _out << sp;
        writeDocCommentFor(enumerator);
        _out << nl << "static readonly " << enumerator->mappedName() << ": " << name << ";";
    }
    _out << nl;
    _out << nl << "/**";
    _out << nl << " * Returns the enumerator for the given value.";
    _out << nl << " *";
    _out << nl << " * @param value The enumerator value.";
    _out << nl << " * @returns The enumerator for the given value.";
    _out << nl << " */";
    _out << nl << "static valueOf(value: number): " << name << ";";
    _out << eb;
}

void
Slice::Gen::TypeScriptVisitor::visitConst(const ConstPtr& p)
{
    _out << sp;
    writeDocCommentFor(p);
    _out << nl << "export const " << p->mappedName() << ": " << typeToTsString(p->type()) << ";";
}

void
Slice::Gen::validateMetadata(const UnitPtr& u)
{
    map<string, MetadataInfo> knownMetadata;

    // "js:module"
    MetadataInfo moduleInfo = {
        .validOn = {typeid(Unit)},
        .acceptedArgumentKind = MetadataArgumentKind::SingleArgument,
    };
    knownMetadata.emplace("js:module", std::move(moduleInfo));

    // "js:defined-in"
    MetadataInfo definedInInfo = {
        .validOn = {typeid(InterfaceDecl), typeid(ClassDecl)},
        .acceptedArgumentKind = MetadataArgumentKind::SingleArgument,
    };
    knownMetadata.emplace("js:defined-in", std::move(definedInInfo));

    // "js:identifier"
    MetadataInfo identifierInfo = {
        .validOn =
            {typeid(Module),
             typeid(InterfaceDecl),
             typeid(Operation),
             typeid(ClassDecl),
             typeid(Slice::Exception),
             typeid(Struct),
             typeid(Sequence),
             typeid(Dictionary),
             typeid(Enum),
             typeid(Enumerator),
             typeid(Const),
             typeid(Parameter),
             typeid(DataMember)},
        .acceptedArgumentKind = MetadataArgumentKind::SingleArgument,
    };
    knownMetadata.emplace("js:identifier", std::move(identifierInfo));

    // Pass this information off to the parser's metadata validation logic.
    Slice::validateMetadata(u, "js", std::move(knownMetadata));
}
