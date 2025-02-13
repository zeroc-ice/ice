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
    /// TODO: this is temporary and will be replaced when we add 'js:identifier' support.
    string jsLinkFormatter(const string& rawLink, const ContainedPtr&, const SyntaxTreeBasePtr&)
    {
        string result = "{@link ";

        auto hashPos = rawLink.find('#');
        if (hashPos != string::npos)
        {
            // JavaScript TypeDoc doc processor doesn't accept # at the beginning of a link.
            if (hashPos != 0)
            {
                result += Slice::JsGenerator::fixId(rawLink.substr(0, hashPos));
                result += "#";
            }
            result += Slice::JsGenerator::fixId(rawLink.substr(hashPos + 1));
        }
        else
        {
            result += Slice::JsGenerator::fixId(rawLink);
        }
        return result + "}";
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
        out << "// Copyright (c) ZeroC, Inc.\n";
        out << "//\n";
        out << "// Ice version " << ICE_STRING_VERSION << "\n";
        out << "//\n";
    }

    string escapeParam(const ParameterList& params, const string& name)
    {
        string r = name;
        for (const auto& param : params)
        {
            if (Slice::JsGenerator::fixId(param->name()) == name)
            {
                r = name + "_";
                break;
            }
        }
        return r;
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
        for (const auto& p : doc.exceptions())
        {
            //
            // Try to locate the exception's definition using the name given in the comment.
            //
            string name = p.first;
            ExceptionPtr ex = op->container()->lookupException(name, false);
            if (ex)
            {
                name = ex->scoped();
            }
            name = JsGenerator::fixId(name);
            out << nl << " * @throws {@link " << name << "} ";
            writeDocLines(out, p.second, false);
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
Slice::JsVisitor::writeMarshalDataMembers(
    const DataMemberList& dataMembers,
    const DataMemberList& optionalMembers,
    const ContainedPtr& contained)
{
    bool isStruct = dynamic_pointer_cast<Struct>(contained) != nullptr;
    bool isLegalKeyType = Dictionary::isLegalKeyType(dynamic_pointer_cast<Struct>(contained));

    for (const auto& dataMember : dataMembers)
    {
        if (!dataMember->optional())
        {
            writeMarshalUnmarshalCode(
                _out,
                dataMember->type(),
                "this." + fixDataMemberName(dataMember->name(), isStruct, isLegalKeyType),
                true);
        }
    }

    for (const auto& optionalMember : optionalMembers)
    {
        writeOptionalMarshalUnmarshalCode(
            _out,
            optionalMember->type(),
            "this." + fixDataMemberName(optionalMember->name(), isStruct, isLegalKeyType),
            optionalMember->tag(),
            true);
    }
}

void
Slice::JsVisitor::writeUnmarshalDataMembers(
    const DataMemberList& dataMembers,
    const DataMemberList& optionalMembers,
    const ContainedPtr& contained)
{
    bool isStruct = dynamic_pointer_cast<Struct>(contained) != nullptr;
    bool isLegalKeyType = Dictionary::isLegalKeyType(dynamic_pointer_cast<Struct>(contained));

    for (const auto& dataMember : dataMembers)
    {
        if (!dataMember->optional())
        {
            writeMarshalUnmarshalCode(
                _out,
                dataMember->type(),
                "this." + fixDataMemberName(dataMember->name(), isStruct, isLegalKeyType),
                false);
        }
    }

    for (const auto& optionalMember : optionalMembers)
    {
        writeOptionalMarshalUnmarshalCode(
            _out,
            optionalMember->type(),
            "this." + fixDataMemberName(optionalMember->name(), isStruct, isLegalKeyType),
            optionalMember->tag(),
            false);
    }
}

void
Slice::JsVisitor::writeInitDataMembers(const DataMemberList& dataMembers, const ContainedPtr& contained)
{
    bool isStruct = dynamic_pointer_cast<Struct>(contained) != nullptr;
    bool isLegalKeyType = Dictionary::isLegalKeyType(dynamic_pointer_cast<Struct>(contained));

    for (const auto& dataMember : dataMembers)
    {
        const string m = fixDataMemberName(dataMember->name(), isStruct, isLegalKeyType);
        _out << nl << "this." << m << " = " << fixId(dataMember->name()) << ';';
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
        return "new " + typeToJsString(type) + "()";
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

        _typeScriptOutput.open(file.c_str());
        if (!_typeScriptOutput)
        {
            ostringstream os;
            os << "cannot open '" << file << "': " << IceInternal::errorToString(errno);
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
    if (jsModule == "ice")
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
            jsIceImports.insert("UserException");
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
            needStreamHelpers = true;
        }

        if (needStreamHelpers)
        {
            jsIceImports.insert("StreamHelpers");
        }
    }
    else
    {
        imports["ice"] = set<string>{"Ice"};
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
    if (jsModule == "ice")
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
        for (auto i = iceModules.begin(); i != iceModules.end();)
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
    const string scoped = getLocalScope(p->scoped());
    if (_exportedModules.insert(scoped).second)
    {
        _out << sp;
        _out << nl;
        if (p->isTopLevel())
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

Slice::Gen::TypesVisitor::TypesVisitor(IceInternal::Output& out) : JsVisitor(out) {}

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
    for (const auto& allDataMember : allDataMembers)
    {
        allParamNames.push_back(fixId(allDataMember->name()));
    }

    vector<string> baseParamNames;
    DataMemberList baseDataMembers;

    if (base)
    {
        baseDataMembers = base->allDataMembers();
        for (const auto& baseDataMember : baseDataMembers)
        {
            baseParamNames.push_back(fixId(baseDataMember->name()));
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
        for (const auto& baseDataMember : baseDataMembers)
        {
            _out << fixId(baseDataMember->name());
        }

        for (const auto& dataMember : dataMembers)
        {
            string value;
            if (dataMember->optional())
            {
                if (dataMember->defaultValue())
                {
                    value = writeConstantValue(
                        scope,
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
                        scope,
                        dataMember->type(),
                        dataMember->defaultValueType(),
                        *dataMember->defaultValue());
                }
                else
                {
                    value = getValue(scope, dataMember->type());
                }
            }
            _out << (fixId(dataMember->name()) + (value.empty() ? value : (" = " + value)));
        }

        _out << epar << sb;
        _out << nl << "super" << spar << baseParamNames << epar << ';';
        writeInitDataMembers(dataMembers, p);
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
            writeMarshalDataMembers(dataMembers, optionalMembers, p);
            _out << eb;

            _out << sp;
            _out << nl << "_iceReadMemberImpl(istr)";
            _out << sb;
            writeUnmarshalDataMembers(dataMembers, optionalMembers, p);
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
        for (auto q = ops.begin(); q != ops.end(); ++q)
        {
            if (q != ops.begin())
            {
                _out << ',';
            }

            const OperationPtr& op = *q;
            const string opName = fixId(op->name());
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
            _out << ", ";

            if (op->mode() != Operation::Normal)
            {
                _out << sliceModeToIceMode(op->mode()); // Mode.
            }
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
        _out << ", \"" << typeToJsString(type) << "\"";
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
    for (const auto& allDataMember : allDataMembers)
    {
        allParamNames.push_back(fixId(allDataMember->name()));
    }

    vector<string> baseParamNames;
    DataMemberList baseDataMembers;

    if (p->base())
    {
        baseDataMembers = p->base()->allDataMembers();
        for (const auto& baseDataMember : baseDataMembers)
        {
            baseParamNames.push_back(fixId(baseDataMember->name()));
        }
    }

    _out << sp;
    writeDocCommentFor(p);
    _out << nl << localScope << '.' << name << " = class extends " << baseRef;
    _out << sb;

    _out << nl << "constructor" << spar;

    for (const auto& baseDataMember : baseDataMembers)
    {
        _out << fixId(baseDataMember->name());
    }

    for (const auto& dataMember : dataMembers)
    {
        string value;
        if (dataMember->optional())
        {
            if (dataMember->defaultValue())
            {
                value = writeConstantValue(
                    scope,
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
                    scope,
                    dataMember->type(),
                    dataMember->defaultValueType(),
                    *dataMember->defaultValue());
            }
            else
            {
                value = getValue(scope, dataMember->type());
            }
        }
        _out << (fixId(dataMember->name()) + (value.empty() ? value : (" = " + value)));
    }

    _out << "_cause = \"\"" << epar;
    _out << sb;
    _out << nl << "super" << spar << baseParamNames << "_cause" << epar << ';';
    writeInitDataMembers(dataMembers, p);
    _out << eb;

    _out << sp;
    _out << nl << "static get _parent()";
    _out << sb;
    _out << nl << "return " << baseRef << ";";
    _out << eb;

    _out << sp;
    _out << nl << "static get _ice_id()";
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
        writeMarshalDataMembers(dataMembers, optionalMembers, p);
        _out << eb;

        _out << sp;
        _out << nl << "_readMemberImpl(istr)";
        _out << sb;
        writeUnmarshalDataMembers(dataMembers, optionalMembers, p);
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
    for (const auto& dataMember : dataMembers)
    {
        paramNames.push_back(fixId(dataMember->name()));
    }

    _out << sp;
    writeDocCommentFor(p);
    _out << nl << localScope << '.' << name << " = class";
    _out << sb;

    _out << nl << "constructor" << spar;

    for (const auto& dataMember : dataMembers)
    {
        string value;
        if (dataMember->optional())
        {
            if (dataMember->defaultValue())
            {
                value = writeConstantValue(
                    scope,
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
                    scope,
                    dataMember->type(),
                    dataMember->defaultValueType(),
                    *dataMember->defaultValue());
            }
            else
            {
                value = getValue(scope, dataMember->type());
            }
        }
        _out << (fixId(dataMember->name()) + (value.empty() ? value : (" = " + value)));
    }

    _out << epar;
    _out << sb;
    writeInitDataMembers(dataMembers, p);
    _out << eb;

    _out << sp;
    _out << nl << "_write(ostr)";
    _out << sb;
    writeMarshalDataMembers(dataMembers, DataMemberList(), p);
    _out << eb;

    _out << sp;
    _out << nl << "_read(istr)";
    _out << sb;
    writeUnmarshalDataMembers(dataMembers, DataMemberList(), p);
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
        _out << ", \"" << typeToJsString(valueType) << "\"";
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

Slice::Gen::TypeScriptImportVisitor::TypeScriptImportVisitor(IceInternal::Output& out) : JsVisitor(out) {}

void
Slice::Gen::TypeScriptImportVisitor::addImport(const ContainedPtr& definition)
{
    string jsImportedModule = getJavaScriptModule(definition->definitionContext());
    if (jsImportedModule.empty())
    {
        string definedIn = definition->getMetadataArgs("js:defined-in").value_or("");
        if (!definedIn.empty())
        {
            _importedTypes[definition->scoped()] = "__module_" + pathToModule(definedIn) + ".";
            _importedModules.insert(removeExtension(definedIn) + ".js");
        }
        else
        {
            const string filename = definition->definitionContext()->filename();
            if (filename == _filename)
            {
                // For types defined in the same unit we use a __global_ prefix to refer to the global scope.
                _importedTypes[definition->scoped()] = "__global_";
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
                _importedTypes[definition->scoped()] = "__module_" + pathToModule(f) + ".";
                _importedModules.insert(removeExtension(f) + ".js");
            }
        }
    }
    else if (_module != jsImportedModule)
    {
        _importedTypes[definition->scoped()] = "__module_" + jsImportedModule + ".";
    }
    else
    {
        // For types defined in the same module, we use a __global_ prefix, to refer to the global scope.
        _importedTypes[definition->scoped()] = "__global_";
    }
}

bool
Slice::Gen::TypeScriptImportVisitor::visitUnitStart(const UnitPtr& unit)
{
    _module = getJavaScriptModule(unit->findDefinitionContext(unit->topLevelFile()));
    _filename = unit->topLevelFile();
    if (_module != "ice")
    {
        _importedModules.insert("ice");
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
    for (const auto& module : _importedModules)
    {
        if (ends_with(module, ".js"))
        {
            _out << nl << "import * as __module_" << pathToModule(module) << " from \"" << module << "\";";
        }
        else
        {
            _out << nl << "import * as __module_" << module << " from \"" << module << "\";";
        }
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
        "number",   // byte
        "boolean",  // bool
        "number",   // short
        "number",   // int
        "Ice.Long", // long
        "number",   // float
        "number",   // double
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
                t = _iceImportPrefix + typeScriptBuiltinTable[builtin->kind()];
                // For Slice long parameters we accept both number and Ice.Long, this is more convenient
                // for the user as he can use the native number type when the value is within the range of
                // number (2^53 - 1) and Ice.Long when the value is outside of this range.
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
        t = importPrefix(cl->scoped()) + fixId(cl->scoped());
        if (nullable)
        {
            t += " | null";
        }
    }

    InterfaceDeclPtr proxy = dynamic_pointer_cast<InterfaceDecl>(type);
    if (proxy)
    {
        t = importPrefix(proxy->scoped()) + fixId(proxy->scoped() + "Prx");
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
        t = importPrefix(contained->scoped()) + fixId(contained->scoped());
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
    _iceImportPrefix = _module == "ice" ? "" : "__module_ice.";
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
            auto pos = key.find("::", 2);
            assert(pos != string::npos);

            globalModules.insert(key.substr(2, pos - 2));
        }
    }

    for (const auto& module : globalModules)
    {
        _out << nl << "import __global_" << fixId(module) << " = " << fixId(module) << ";";
    }

    if (!_module.empty())
    {
        _out << eb; // module end
    }
}

bool
Slice::Gen::TypeScriptVisitor::visitModuleStart(const ModulePtr& p)
{
    UnitPtr unit = dynamic_pointer_cast<Unit>(p->container());
    if (unit && _module.empty())
    {
        _out << sp;
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
    const DataMemberList dataMembers = p->dataMembers();
    const DataMemberList allDataMembers = p->allDataMembers();
    _out << sp;
    writeDocCommentFor(p);
    _out << nl << "export class " << fixId(p->name()) << " extends ";
    ClassDefPtr base = p->base();
    if (base)
    {
        _out << importPrefix(base->scoped()) << fixId(base->scoped());
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
            _out << nl << " * @param " << fixId(dataMember->name()) << " " << getDocSentence(comment->overview());
        }
    }
    _out << nl << " */";
    _out << nl << "constructor" << spar;
    for (const auto& dataMember : allDataMembers)
    {
        _out << (fixId(dataMember->name()) + "?: " + typeToTsString(dataMember->type()));
    }
    _out << epar << ";";
    for (const auto& dataMember : dataMembers)
    {
        _out << sp;
        writeDocCommentFor(dataMember);
        _out << nl << fixDataMemberName(dataMember->name(), false, false) << ": "
             << typeToTsString(dataMember->type(), true) << ";";
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
                out << nl << " * @param " << Slice::JsGenerator::fixId(q->first) << " ";
                writeDocLines(out, q->second, false);
            }
        }
    }

    ParameterList outParams = op->outParameters();
    TypePtr ret = op->returnType();

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

    if ((outParams.size() + (ret ? 1 : 0)) > 1)
    {
        out << ", which resolves to an array with the following entries:";
    }
    else if (ret || outParams.size() == 1)
    {
        out << ", which resolves to:";
    }
    else
    {
        out << ".";
    }

    if (comment && ret)
    {
        out << nl << " * - " << typeToTsString(ret, true, false, op->returnIsOptional()) << " : ";
        writeDocLines(out, comment->returns(), false, "   ");
    }

    for (const auto& param : outParams)
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
    const string prx = p->name() + "Prx";
    _out << sp;
    writeDocCommentFor(p);
    _out << nl << "export class " << prx << " extends " << _iceImportPrefix << "Ice.ObjectPrx";
    _out << sb;

    _out << sp;
    _out << nl << "/**";
    _out << nl << " * Constructs a new " << prx << " proxy.";
    _out << nl << " * @param communicator - The communicator for the new proxy.";
    _out << nl << " * @param proxyString - The string representation of the proxy.";
    _out << nl << " * @returns The new " << prx << " proxy.";
    _out << nl << " * @throws ParseException - Thrown if the proxyString is not a valid proxy string.";
    _out << nl << " */";
    _out << nl << "constructor(communicator: " << _iceImportPrefix << "Ice.Communicator, proxyString: string);";

    _out << sp;
    _out << nl << "/**";
    _out << nl << " * Constructs a new " << prx << " proxy from an ObjectPrx. The new proxy is a clone of the";
    _out << nl << " * provided proxy.";
    _out << nl << " * @param prx - The proxy to clone.";
    _out << nl << " * @returns The new " << prx << " proxy.";
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
        _out << nl << fixId(op->name()) << spar;
        for (const auto& param : inParams)
        {
            // TypeScript doesn't allow optional parameters with '?' prefix before required parameters.
            const string optionalPrefix =
                param->optional() && areRemainingParamsOptional(paramList, param->name()) ? "?" : "";
            _out
                << (fixId(param->name()) + optionalPrefix + ": " +
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
         << "facet?: string): " << fixId(p->name() + "Prx") << ";";

    _out << sp;
    _out << nl << "/**";
    _out << nl << " * Downcasts a proxy after confirming the target object's type via a remote invocation.";
    _out << nl << " * @param prx The target proxy.";
    _out << nl << " * @param facet A facet name.";
    _out << nl << " * @param context The context map for the invocation.";
    _out << nl
         << " * @returns A proxy with the requested type and facet, or nil if the target proxy is nil or the target";
    _out << nl << " * object does not support the requested type.";
    _out << nl << " */";
    _out << nl << "static checkedCast(prx: " << _iceImportPrefix << "Ice.ObjectPrx"
         << ", "
         << "facet?: string, context?: Map<string, string>): " << _iceImportPrefix << "Ice.AsyncResult"
         << "<" << fixId(p->name() + "Prx") << " | null>;";
    _out << eb;

    _out << sp;
    writeDocCommentFor(p, false);
    _out << nl << "export abstract class " << fixId(p->name()) << " extends " << _iceImportPrefix << "Ice.Object";
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
        _out << nl << "abstract " << fixId(op->name()) << spar;
        for (const auto& param : inParams)
        {
            _out << (fixId(param->name()) + ": " + typeToTsString(param->type(), true, true, param->optional()));
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
    const string name = fixId(p->name());
    const DataMemberList dataMembers = p->dataMembers();
    const DataMemberList allDataMembers = p->allDataMembers();

    ExceptionPtr base = p->base();
    string baseRef;
    if (base)
    {
        baseRef = importPrefix(base->scoped()) + fixId(base->scoped());
    }
    else
    {
        baseRef = _iceImportPrefix + "Ice.UserException";
    }

    _out << sp;
    writeDocCommentFor(p);
    _out << nl << "export class " << name << " extends " << baseRef << sb;
    if (!allDataMembers.empty())
    {
        _out << nl << "/**";
        _out << nl << " * One-shot constructor to initialize all data members.";
        for (const auto& dataMember : allDataMembers)
        {
            if (auto comment = DocComment::parseFrom(dataMember, jsLinkFormatter))
            {
                _out << nl << " * @param " << fixId(dataMember->name()) << " " << getDocSentence(comment->overview());
            }
        }
        _out << nl << " */";
        _out << nl << "constructor" << spar;
        for (const auto& dataMember : allDataMembers)
        {
            _out << (fixId(dataMember->name()) + "?: " + typeToTsString(dataMember->type()));
        }
        _out << epar << ";";
    }

    for (const auto& dataMember : allDataMembers)
    {
        const string optionalModifier = dataMember->optional() ? "?" : "";
        _out << nl << fixId(dataMember->name()) << optionalModifier << ": " << typeToTsString(dataMember->type(), true)
             << ";";
    }
    _out << eb;
    return false;
}

bool
Slice::Gen::TypeScriptVisitor::visitStructStart(const StructPtr& p)
{
    const string name = fixId(p->name());
    const DataMemberList dataMembers = p->dataMembers();
    _out << sp;
    writeDocCommentFor(p);
    _out << nl << "export class " << name << sb;
    _out << nl << "constructor" << spar;
    for (const auto& dataMember : dataMembers)
    {
        // TODO why are all parameters optional?
        _out << (fixDataMemberName(dataMember->name(), false, false) + "?: " + typeToTsString(dataMember->type()));
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

    //
    // Only generate hashCode if this structure type is a legal dictionary key type.
    //
    bool isLegalKeyType = Dictionary::isLegalKeyType(p);
    if (isLegalKeyType)
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
        _out << nl << fixDataMemberName(dataMember->name(), true, isLegalKeyType) << ":"
             << typeToTsString(dataMember->type(), true) << ";";
    }

    _out << eb;
    return false;
}

void
Slice::Gen::TypeScriptVisitor::visitSequence(const SequencePtr& p)
{
    const string name = fixId(p->name());
    _out << sp;
    writeDocCommentFor(p);
    _out << nl << "export type " << name << " = " << typeToTsString(p) << ";";

    _out << sp;
    _out << nl << "/**";
    _out << nl << " * Helper class for encoding {@link " << name << "} into an OutputStream and";
    _out << nl << " * decoding {@link " << name << "} from an InputStream.";
    _out << nl << " */";
    _out << nl << "export class " << fixId(p->name() + "Helper");
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
    const string name = fixId(p->name());
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
    _out << nl << "export class " << fixId(p->name() + "Helper");
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
    _out << sp;
    writeDocCommentFor(p);
    _out << nl << "export class " << fixId(p->name()) << " extends " << _iceImportPrefix << "Ice.EnumBase";
    _out << sb;
    for (const auto& enumerator : p->enumerators())
    {
        _out << sp;
        writeDocCommentFor(enumerator);
        _out << nl << "static readonly " << fixId(enumerator->name()) << ": " << fixId(p->name()) << ";";
    }
    _out << nl;
    _out << nl << "/**";
    _out << nl << " * Returns the enumerator for the given value.";
    _out << nl << " *";
    _out << nl << " * @param value The enumerator value.";
    _out << nl << " * @returns The enumerator for the given value.";
    _out << nl << " */";
    _out << nl << "static valueOf(value: number): " << fixId(p->name()) << ";";
    _out << eb;
}

void
Slice::Gen::TypeScriptVisitor::visitConst(const ConstPtr& p)
{
    _out << sp;
    writeDocCommentFor(p);
    _out << nl << "export const " << fixId(p->name()) << ": " << typeToTsString(p->type()) << ";";
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

    // Pass this information off to the parser's metadata validation logic.
    Slice::validateMetadata(u, "js", knownMetadata);
}
