//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/InputUtil.h>
#include <IceUtil/StringUtil.h>
#include <Slice/Parser.h>
#include <Slice/GrammarUtil.h>
#include <cstring>
#include <iterator>

// TODO: fix this warning once we no longer support VS2013 and earlier
#if defined(_MSC_VER)
#    pragma warning(disable:4589) // Constructor of abstract class 'Slice::Type' ignores initializer...
#endif

using namespace std;
using namespace Slice;

namespace
{
    struct UnderscoreSeparator : numpunct<char>
    {
        char do_thousands_sep() const override { return '_' ; }
        string do_grouping() const override { return "\3"; }
    };

    locale underscoreSeparatorLocale = locale(locale::classic(), new UnderscoreSeparator);
}

Slice::CompilerException::CompilerException(const char* file, int line, const string& r) :
    IceUtil::Exception(file, line),
    _reason(r)
{
}

string
Slice::CompilerException::ice_id() const
{
    return "::Slice::CompilerException";
}

void
Slice::CompilerException::ice_print(ostream& out) const
{
    IceUtil::Exception::ice_print(out);
    out << ": " << _reason;
}

Slice::CompilerException*
Slice::CompilerException::ice_cloneImpl() const
{
    return new CompilerException(*this);
}

void
Slice::CompilerException::ice_throw() const
{
    throw *this;
}

string
Slice::CompilerException::reason() const
{
    return _reason;
}

// Forward declare things from Bison and Flex the parser can use.
extern int slice_parse();
extern int slice_lineno;
extern FILE* slice_in;
extern int slice_debug;
extern int slice__flex_debug;

namespace
{
    bool
    isMutableAfterReturnType(const TypePtr& type)
    {
        // Returns true if the type contains data types which can be referenced by user code
        // and mutated after a dispatch returns.

        if (auto optional = OptionalPtr::dynamicCast(type))
        {
            return isMutableAfterReturnType(optional->underlying());
        }

        if (ClassDeclPtr::dynamicCast(type) || StructPtr::dynamicCast(type) || SequencePtr::dynamicCast(type)
            || DictionaryPtr::dynamicCast(type))
        {
            return true;
        }

        BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
        if (builtin && builtin->usesClasses())
        {
            return true;
        }

        return false;
    }
}

namespace Slice
{
    Unit* unit;
}

// ----------------------------------------------------------------------
// DefinitionContext
// ----------------------------------------------------------------------

Slice::DefinitionContext::DefinitionContext(int includeLevel, const StringList& metadata) :
    _includeLevel(includeLevel), _metadata(metadata)
{
    initSuppressedWarnings();
}

string
Slice::DefinitionContext::filename() const
{
    return _filename;
}

int
Slice::DefinitionContext::includeLevel() const
{
    return _includeLevel;
}

void
Slice::DefinitionContext::setFilename(const string& filename)
{
    _filename = filename;
}

void
Slice::DefinitionContext::setMetadata(const StringList& metadata)
{
    _metadata = metadata;
    initSuppressedWarnings();
}

string
Slice::DefinitionContext::findMetadata(const string& prefix) const
{
    for (const auto& metadata : _metadata)
    {
        if (metadata.find(prefix) == 0)
        {
            return metadata;
        }
    }
    return "";
}

StringList
Slice::DefinitionContext::getAllMetadata() const
{
    return _metadata;
}

bool
Slice::DefinitionContext::compatMode() const
{
    return findMetadata("3.7") == "3.7";
}

void
Slice::DefinitionContext::error(const string& file, int line, const string& msg) const
{
    emitError(file, line, msg);
    throw CompilerException(__FILE__, __LINE__, msg);
}

void
Slice::DefinitionContext::warning(WarningCategory category, const string& file, int line, const string& msg) const
{
    if (!suppressWarning(category))
    {
        emitWarning(file, line, msg);
    }
}

bool
Slice::DefinitionContext::suppressWarning(WarningCategory category) const
{
    return _suppressedWarnings.find(category) != _suppressedWarnings.end() ||
           _suppressedWarnings.find(All) != _suppressedWarnings.end();
}

void
Slice::DefinitionContext::initSuppressedWarnings()
{
    _suppressedWarnings.clear();
    const string prefix = "suppress-warning";
    string value = findMetadata(prefix);
    if (value == prefix)
    {
        _suppressedWarnings.insert(All);
    }
    else if (!value.empty())
    {
        assert(value.length() > prefix.length());
        if (value[prefix.length()] == ':')
        {
            value = value.substr(prefix.length() + 1);
            vector<string> results;
            IceUtilInternal::splitString(value, ",", results);
            for (const auto& p : results)
            {
                string s = IceUtilInternal::trim(p);
                if (s == "all")
                {
                    _suppressedWarnings.insert(All);
                }
                else if (s == "deprecated")
                {
                    _suppressedWarnings.insert(Deprecated);
                }
                else if (s == "invalid-metadata")
                {
                    _suppressedWarnings.insert(InvalidMetadata);
                }
                else if (s == "reserved-identifier")
                {
                    _suppressedWarnings.insert(ReservedIdentifier);
                }
                else
                {
                    warning(InvalidMetadata, "", -1, string("invalid category `") + s +
                            "' in file metadata suppress-warning");
                }
            }
        }
    }
}

// ----------------------------------------------------------------------
// Comment
// ----------------------------------------------------------------------

bool
Slice::Comment::isDeprecated() const
{
    return _isDeprecated;
}

StringList
Slice::Comment::deprecated() const
{
    return _deprecated;
}

StringList
Slice::Comment::overview() const
{
    return _overview;
}

StringList
Slice::Comment::misc() const
{
    return _misc;
}

StringList
Slice::Comment::seeAlso() const
{
    return _seeAlso;
}

StringList
Slice::Comment::returns() const
{
    return _returns;
}

map<string, StringList>
Slice::Comment::params() const
{
    return _params;
}

map<string, StringList>
Slice::Comment::exceptions() const
{
    return _exceptions;
}

Slice::Comment::Comment()
{
}

// ----------------------------------------------------------------------
// SyntaxTreeBase
// ----------------------------------------------------------------------

void
Slice::SyntaxTreeBase::destroy()
{
    _unit = nullptr;
}

UnitPtr
Slice::SyntaxTreeBase::unit() const
{
    return _unit;
}

DefinitionContextPtr
Slice::SyntaxTreeBase::definitionContext() const
{
    return _definitionContext;
}

void
Slice::SyntaxTreeBase::visit(ParserVisitor*, bool)
{
}

Slice::SyntaxTreeBase::SyntaxTreeBase(const UnitPtr& unt, const DefinitionContextPtr& definitionContext) :
    _unit(unt),
    _definitionContext(definitionContext)
{
    if (!_definitionContext && _unit)
    {
        _definitionContext = _unit->currentDefinitionContext();
    }
}

// ----------------------------------------------------------------------
// Type
// ----------------------------------------------------------------------

Slice::Type::Type(const UnitPtr& unt) :
    SyntaxTreeBase(unt)
{
}

// ----------------------------------------------------------------------
// Builtin
// ----------------------------------------------------------------------

string
Slice::Builtin::typeId() const
{
    if (_kind == KindObject || _kind == KindAnyClass)
    {
        return "::Ice::" + kindAsString();
    }
    else
    {
        return kindAsString();
    }
}

bool
Slice::Builtin::usesClasses() const
{
    return _kind == KindAnyClass;
}

size_t
Slice::Builtin::minWireSize() const
{
    switch(_kind)
    {
        case KindBool: return 1;
        case KindByte: return 1;
        case KindShort: return 2;
        case KindUShort: return 2;
        case KindInt: return 4;
        case KindUInt: return 4;
        case KindVarInt: return 1;
        case KindVarUInt: return 1;
        case KindLong: return 8;
        case KindULong: return 8;
        case KindVarLong: return 1;
        case KindVarULong: return 1;
        case KindFloat: return 4;
        case KindDouble: return 8;
        case KindString: return 1; // at least one byte for an empty string.
        case KindObject: return 3; // at least a 1-character identity name (2 bytes) + empty identity category (1 byte).
        case KindAnyClass: return 1; // at least one byte to marshal an index instead of an instance.
    }
    throw logic_error("");
}

string
Slice::Builtin::getTagFormat() const
{
    switch(_kind)
    {
        case KindBool:
        case KindByte:
            return "F1";
        case KindShort:
        case KindUShort:
            return "F2";
        case KindInt:
        case KindUInt:
        case KindFloat:
            return "F4";
        case KindLong:
        case KindULong:
        case KindDouble:
            return "F8";
        case KindVarInt:
        case KindVarUInt:
        case KindVarLong:
        case KindVarULong:
            return "VInt";
        case KindString:
            return "VSize";
        case KindAnyClass:
            return "Class";
        case KindObject:
            return "FSize";
    }
    throw logic_error("");
}

bool
Slice::Builtin::isVariableLength() const
{
    switch(_kind)
    {
        case KindVarInt:
        case KindVarUInt:
        case KindVarLong:
        case KindVarULong:
        case KindString:
        case KindObject:
        case KindAnyClass:
            return true;
        default:
            return false;
    }
}

bool
Slice::Builtin::isNumericType() const
{
    switch(_kind)
    {
        case KindByte:
        case KindShort:
        case KindUShort:
        case KindInt:
        case KindUInt:
        case KindVarInt:
        case KindVarUInt:
        case KindLong:
        case KindULong:
        case KindVarLong:
        case KindVarULong:
        case KindFloat:
        case KindDouble:
            return true;
        default:
            return false;
    }
}

bool
Slice::Builtin::isNumericTypeOrBool() const
{
    return isNumericType() || (_kind == KindBool);
}

bool
Slice::Builtin::isIntegralType() const
{
    switch(_kind)
    {
        case KindByte:
        case KindShort:
        case KindUShort:
        case KindInt:
        case KindUInt:
        case KindVarInt:
        case KindVarUInt:
        case KindLong:
        case KindULong:
        case KindVarLong:
        case KindVarULong:
            return true;
        default:
            return false;
    }
}

bool
Slice::Builtin::isUnsignedType() const
{
    switch(_kind)
    {
        case KindByte:
        case KindUShort:
        case KindUInt:
        case KindVarUInt:
        case KindULong:
        case KindVarULong:
            return true;
        default:
            return false;
    }
}

pair<int64_t, uint64_t>
Slice::Builtin::integralRange() const
{
    switch(_kind)
    {
        case KindByte:
            return make_pair<int64_t, uint64_t>(0, UINT8_MAX);
        case KindShort:
            return make_pair<int64_t, uint64_t>(INT16_MIN, INT16_MAX);
        case KindUShort:
            return make_pair<int64_t, uint64_t>(0, UINT16_MAX);
        case KindInt:
        case KindVarInt:
            return make_pair<int64_t, uint64_t>(INT32_MIN, INT32_MAX);
        case KindUInt:
        case KindVarUInt:
            return make_pair<int64_t, uint64_t>(0, UINT32_MAX);
        case KindLong:
            return make_pair<int64_t, uint64_t>(INT64_MIN, INT64_MAX);
        case KindVarLong:
            // the first 2 bits are used to encode the var-integer size, so the range is
            // -2^63 / 4 == -2^61 to (2^63 - 1) / 4 == 2^61 - 1.
            return make_pair<int64_t, uint64_t>(INT64_MIN / 4, INT64_MAX / 4);
        case KindULong:
            return make_pair<int64_t, uint64_t>(0, UINT64_MAX);
        case KindVarULong:
            return make_pair<int64_t, uint64_t>(0, UINT64_MAX / 4);
        default:
            assert(false);
            throw logic_error("");
    }
}

Slice::Builtin::Kind
Slice::Builtin::kind() const
{
    return _kind;
}

string
Slice::Builtin::kindAsString() const
{
    return builtinTable[_kind];
}

optional<Slice::Builtin::Kind>
Slice::Builtin::kindFromString(string_view str)
{
    for (size_t i = 0; i < builtinTable.size(); i++)
    {
        if (str == builtinTable[i])
        {
            return static_cast<Kind>(i);
        }
    }
    return nullopt;
}

Slice::Builtin::Builtin(const UnitPtr& ut, Kind kind) :
    SyntaxTreeBase(ut),
    Type(ut),
    _kind(kind)
{
    // Builtin types do not have a definition context.
    _definitionContext = nullptr;
}

// ----------------------------------------------------------------------
// Contained
// ----------------------------------------------------------------------

ContainerPtr
Slice::Contained::container() const
{
    return _container;
}

string
Slice::Contained::name() const
{
    return _name;
}

string
Slice::Contained::scoped() const
{
    return _scoped;
}

string
Slice::Contained::scope() const
{
    auto idx = _scoped.rfind("::");
    assert(idx != string::npos);
    return string(_scoped, 0, idx + 2);
}

string
Slice::Contained::flattenedScope() const
{
    string s = scope();
    string::size_type pos = 0;
    while ((pos = s.find("::", pos)) != string::npos)
    {
        s.replace(pos, 2, "_");
    }
    return s;
}

string
Slice::Contained::file() const
{
    return _file;
}

int
Slice::Contained::line() const
{
    return _line;
}

string
Slice::Contained::comment() const
{
    return _comment;
}

namespace
{
    void
    trimLines(StringList& l)
    {
        // Remove empty trailing lines.
        while (!l.empty() && l.back().empty())
        {
            l.pop_back();
        }
    }

    StringList
    splitComment(const string& c, bool stripMarkup)
    {
        string comment = c;

        if (stripMarkup)
        {
            // Strip HTML markup and javadoc links.
            for (auto pos = comment.find('<'); pos != string::npos; pos = comment.find('<', pos))
            {
                auto endpos = comment.find('>', pos);
                if (endpos == string::npos)
                {
                    break;
                }
                comment.erase(pos, endpos - pos + 1);
            }

            // Replace links of the form {@link Type#member} with "Type.member".
            const string link = "{@link";
            for (auto pos = comment.find(link); pos != string::npos; pos = comment.find(link, pos))
            {
                comment.erase(pos, link.size() + 1); // Erase trailing white space too.

                auto endpos = comment.find('}', pos);
                if (endpos != string::npos)
                {
                    comment.erase(endpos, 1);

                    auto hashpos = comment.find('#');
                    comment[hashpos] = '.';
                }
            }
        }

        StringList result;
        string::size_type pos = 0;
        string::size_type nextPos;
        while ((nextPos = comment.find('\n', pos)) != string::npos)
        {
            result.push_back(IceUtilInternal::trim(string(comment, pos, nextPos - pos)));
            pos = nextPos + 1;
        }
        string lastLine = IceUtilInternal::trim(string(comment, pos));
        if (!lastLine.empty())
        {
            result.push_back(lastLine);
        }

        trimLines(result);
        return result;
    }

    bool
    parseCommentLine(const string& l, const string& tag, bool namedTag, string& name, string& doc)
    {
        doc.clear();

        if (l.find(tag) == 0)
        {
            const string ws = " \t";

            if (namedTag)
            {
                auto n = l.find_first_not_of(ws, tag.size());
                if (n == string::npos)
                {
                    return false; // Malformed line, ignore it.
                }
                auto end = l.find_first_of(ws, n);
                if (end == string::npos)
                {
                    return false; // Malformed line, ignore it.
                }
                name = l.substr(n, end - n);
                n = l.find_first_not_of(ws, end);
                if (n != string::npos)
                {
                    doc = l.substr(n);
                }
            }
            else
            {
                name.clear();

                auto n = l.find_first_not_of(ws, tag.size());
                if (n == string::npos)
                {
                    return false; // Malformed line, ignore it.
                }
                doc = l.substr(n);
            }
            return true;
        }
        return false;
    }
}

CommentPtr
Slice::Contained::parseComment(bool stripMarkup) const
{
    CommentPtr comment = new Comment;

    // First check metadata for a deprecated tag.
    string deprecateMetadata;
    if (findMetadata("deprecate", deprecateMetadata))
    {
        comment->_isDeprecated = true;
        if (deprecateMetadata.find("deprecate:") == 0 && deprecateMetadata.size() > 10)
        {
            comment->_deprecated.push_back(IceUtilInternal::trim(deprecateMetadata.substr(10)));
        }
    }
    else
    {
        comment->_isDeprecated = false;
    }

    if (!comment->_isDeprecated && _comment.empty())
    {
        return nullptr;
    }

    // Split up the comment into lines.
    StringList lines = splitComment(_comment, stripMarkup);

    StringList::const_iterator i;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        const string l = *i;
        if (l[0] == '@')
        {
            break;
        }
        comment->_overview.push_back(l);
    }

    enum State { StateMisc, StateParam, StateThrows, StateReturn, StateDeprecated };
    State state = StateMisc;
    string name;
    const string ws = " \t";
    const string paramTag = "@param";
    const string throwsTag = "@throws";
    const string exceptionTag = "@exception";
    const string returnTag = "@return";
    const string deprecatedTag = "@deprecated";
    const string seeTag = "@see";
    for (; i != lines.end(); ++i)
    {
        const string l = IceUtilInternal::trim(*i);
        string line;
        if (parseCommentLine(l, paramTag, true, name, line) && !line.empty())
        {
            state = StateParam;
            comment->_params[name] = { line }; // The first line of the description.
        }
        else if (parseCommentLine(l, throwsTag, true, name, line) && !line.empty())
        {
            state = StateThrows;
            comment->_exceptions[name] = { line }; // The first line of the description.
        }
        else if (parseCommentLine(l, exceptionTag, true, name, line) && !line.empty())
        {
            state = StateThrows;
            comment->_exceptions[name] = { line }; // The first line of the description.
        }
        else if (parseCommentLine(l, seeTag, false, name, line) && !line.empty())
        {
            comment->_seeAlso.push_back(line);
        }
        else if (parseCommentLine(l, returnTag, false, name, line) && !line.empty())
        {
            state = StateReturn;
            comment->_returns.push_back(line); // The first line of the description.
        }
        else if (parseCommentLine(l, deprecatedTag, false, name, line))
        {
            comment->_isDeprecated = true;
            if (!line.empty())
            {
                state = StateDeprecated;
                comment->_deprecated.push_back(line); // The first line of the description.
            }
        }
        else if (!l.empty())
        {
            if (l[0] == '@')
            {
                // Treat all other tags as miscellaneous comments.
                state = StateMisc;
            }

            switch(state)
            {
                case StateMisc:
                {
                    comment->_misc.push_back(l);
                    break;
                }
                case StateParam:
                {
                    assert(!name.empty());
                    StringList sl;
                    if (comment->_params.find(name) != comment->_params.end())
                    {
                        sl = comment->_params[name];
                    }
                    sl.push_back(l);
                    comment->_params[name] = sl;
                    break;
                }
                case StateThrows:
                {
                    assert(!name.empty());
                    StringList sl;
                    if (comment->_exceptions.find(name) != comment->_exceptions.end())
                    {
                        sl = comment->_exceptions[name];
                    }
                    sl.push_back(l);
                    comment->_exceptions[name] = sl;
                    break;
                }
                case StateReturn:
                {
                    comment->_returns.push_back(l);
                    break;
                }
                case StateDeprecated:
                {
                    comment->_deprecated.push_back(l);
                    break;
                }
            }
        }
    }

    trimLines(comment->_overview);
    trimLines(comment->_deprecated);
    trimLines(comment->_misc);
    trimLines(comment->_returns);

    return comment;
}

int
Slice::Contained::includeLevel() const
{
    return _includeLevel;
}

bool
Slice::Contained::hasMetadata(const string& meta) const
{
    return find(_metadata.begin(), _metadata.end(), meta) != _metadata.end();
}

bool
Slice::Contained::hasMetadataWithPrefix(const string& prefix) const
{
    return !findMetadataWithPrefix(prefix).empty();
}

optional<string>
Slice::Contained::findMetadata(const string& directive) const
{
    // TODO this is temporary until we can fully replace the current metadata logic.
    return Slice::findMetadata(directive, parseMetadata(_metadata));
}

bool
Slice::Contained::findMetadata(const string& prefix, string& meta) const
{
    for (const auto& p : _metadata)
    {
        if (p.find(prefix) == 0)
        {
            meta = p;
            return true;
        }
    }
    return false;
}

string
Slice::Contained::findMetadataWithPrefix(const string& prefix) const
{
    string meta;
    if (findMetadata(prefix, meta))
    {
        return meta.substr(prefix.size());
    }
    return "";
}

list<string>
Slice::Contained::getAllMetadata() const
{
    return _metadata;
}

void
Slice::Contained::setMetadata(const list<string>& metadata)
{
    _metadata = metadata;
}

FormatType
Slice::Contained::parseFormatMetadata() const
{
    FormatType result = DefaultFormat; // TODO: replace FormatType here by a std::optional<FormatType>
                                       // and eliminate DefaultFormat (replaced by not-set).

    string tag = findMetadataWithPrefix("format:");
    if (!tag.empty())
    {
        if (tag == "compact")
        {
            result = CompactFormat;
        }
        else if (tag == "sliced")
        {
            result = SlicedFormat;
        }
        else if (tag != "default") // TODO: Allow "default" to be specified as a format value?
        {
            // TODO: How to handle invalid format?
        }
    }
    return result;
}

bool
Slice::Contained::operator<(const Contained& rhs) const
{
    return _scoped < rhs._scoped;
}

bool
Slice::Contained::operator==(const Contained& rhs) const
{
    return _scoped == rhs._scoped;
}

Slice::Contained::Contained(const ContainerPtr& container, const string& name) :
    SyntaxTreeBase(container->unit()),
    _container(container),
    _name(name)
{
    if (ContainedPtr cont = ContainedPtr::dynamicCast(_container))
    {
        _scoped = cont->scoped();
    }
    _scoped += "::" + _name;
    assert(_unit);
    _unit->addContent(this);
    _file = _unit->currentFile();
    _line = _unit->currentLine();
    _comment = _unit->currentComment();
    _includeLevel = _unit->currentIncludeLevel();
}

// ----------------------------------------------------------------------
// Container
// ----------------------------------------------------------------------

void
Slice::Container::destroy()
{
    _introducedMap.clear();
    SyntaxTreeBase::destroy();
}

TypeList
Slice::Container::lookupType(const string& scoped, bool printError)
{
    // Remove whitespace.
    string sc = scoped;
    sc.erase(remove_if(sc.begin(), sc.end(), ::isspace), sc.end());

    // Check for builtin type.
    if (auto kind = Builtin::kindFromString(sc))
    {
        return { _unit->builtin(kind.value()) };
    }

    // Not a builtin type, try to look up a constructed type.
    return lookupTypeNoBuiltin(scoped, printError);
}

TypeList
Slice::Container::lookupTypeNoBuiltin(const string& scoped, bool printError, bool ignoreUndefined)
{
    // Remove whitespace.
    string sc = scoped;
    sc.erase(remove_if(sc.begin(), sc.end(), ::isspace), sc.end());

    // Absolute scoped name?
    if (sc.size() >= 2 && sc[0] == ':')
    {
        return _unit->lookupTypeNoBuiltin(sc.substr(2), printError);
    }

    TypeList results;
    bool typeError = false;
    vector<string> errors;

    ContainedList matches = _unit->findContents(thisScope() + sc);
    for (const auto& match : matches)
    {
        if (InterfaceDefPtr::dynamicCast(match) || ClassDefPtr::dynamicCast(match))
        {
            continue; // Ignore interface and class definitions.
        }

        if (printError && match->scoped() != (thisScope() + sc))
        {
            errors.push_back(match->kindOf() + " `" + scoped
                             + "' is capitalized inconsistently with its previous name: `" + match->scoped() + "'");
        }

        if (ExceptionPtr ex = ExceptionPtr::dynamicCast(match))
        {
            if (printError)
            {
                _unit->error("`" + sc + "' is an exception, which cannot be used as a type");
            }
            return TypeList();
        }

        TypePtr type = TypePtr::dynamicCast(match);
        if (!type)
        {
            typeError = true;
            if (printError)
            {
                errors.push_back("`" + sc + "' is not a type");
            }
            break; // Possible that correct match is higher in scope
        }
        results.push_back(type);
    }

    if (results.empty())
    {
        if (ContainedPtr contained = ContainedPtr::dynamicCast(this))
        {
            results = contained->container()->lookupTypeNoBuiltin(sc, printError, typeError || ignoreUndefined);
        }
        else if (!typeError)
        {
            if (printError && !ignoreUndefined)
            {
                _unit->error("type `" + sc + "' is not defined");
            }
            return TypeList();
        }
    }

    // Do not emit errors if there was a type error but a match was found in a higher scope.
    if (printError && !(typeError && !results.empty()))
    {
        for (const auto& error : errors)
        {
            _unit->error(error);
        }
    }
    return results;
}

ContainedList
Slice::Container::lookupContained(const string& scoped, bool printError)
{
    // Remove whitespace.
    string sc = scoped;
    sc.erase(remove_if(sc.begin(), sc.end(), ::isspace), sc.end());

    // Absolute scoped name?
    if (sc.size() >= 2 && sc[0] == ':')
    {
        return _unit->lookupContained(sc.substr(2), printError);
    }

    ContainedList matches = _unit->findContents(thisScope() + sc);
    ContainedList results;
    for (const auto& match : matches)
    {
        if (InterfaceDefPtr::dynamicCast(match) || ClassDefPtr::dynamicCast(match))
        {
            continue; // ignore definitions
        }

        results.push_back(match);

        if (printError && match->scoped() != (thisScope() + sc))
        {
            string containerName;
            if (ContainedPtr container = ContainedPtr::dynamicCast(match->container()))
            {
                containerName = "in " + container->kindOf() + " `" + container->scoped() + "'";
            }
            else
            {
                containerName = "at global scope";
            }

            _unit->error(match->kindOf() + " `" + scoped + "' is capitalized inconsistently with its previous name: `"
                         + match->scoped() + "'");
            _unit->note(match, match->kindOf() + " `" + match->name() + "' is defined " + containerName);
        }
    }

    if (results.empty())
    {
        ContainedPtr contained = ContainedPtr::dynamicCast(this);
        if (!contained)
        {
            if (printError)
            {
                _unit->error("`" + sc + "' is not defined");
            }
            return ContainedList();
        }
        return contained->container()->lookupContained(sc, printError);
    }
    else
    {
        return results;
    }
}

ExceptionPtr
Slice::Container::lookupException(const string& scoped, bool printError)
{
    ContainedList contained = lookupContained(scoped, printError);
    if (contained.empty())
    {
        return nullptr;
    }

    ExceptionList exceptions;
    for (const auto& p : contained)
    {
        ExceptionPtr ex = ExceptionPtr::dynamicCast(p);
        if (!ex)
        {
            if (printError)
            {
                _unit->error("`" + scoped + "' is not an exception");
            }
            return nullptr;
        }
        exceptions.push_back(ex);
    }
    assert(exceptions.size() == 1);
    return exceptions.front();
}

// Find enumerators using the old unscoped enumerators lookup
EnumeratorList
Slice::Container::enumerators(const string& scoped) const
{
    EnumeratorList result;
    string::size_type lastColon = scoped.rfind(':');

    if (lastColon == string::npos)
    {
        // check all enclosing scopes
        ContainerPtr container = const_cast<Container*>(this);
        do
        {
            if (ModulePtr module = ModulePtr::dynamicCast(container))
            {
                for (const auto& en : module->enums())
                {
                    ContainedList cl = en->lookupContained(scoped, false);
                    if (!cl.empty())
                    {
                        result.push_back(EnumeratorPtr::dynamicCast(cl.front()));
                    }
                }
            }

            if (ContainedPtr contained = ContainedPtr::dynamicCast(container))
            {
                container = contained->container();
            }
            else
            {
                container = nullptr;
            }
        }
        while (result.empty() && container);
    }
    else
    {
        // Find the referenced scope
        ContainerPtr container = const_cast<Container*>(this);
        string scope = scoped.substr(0, scoped.rfind("::"));
        ContainedList cl = container->lookupContained(scope, false);
        if (!cl.empty())
        {
            if (ModulePtr module = ModulePtr::dynamicCast(cl.front()))
            {
                string name = scoped.substr(lastColon + 1);
                for (const auto& en : module->enums())
                {
                    ContainedList cl2 = en->lookupContained(name, false);
                    if (!cl2.empty())
                    {
                        result.push_back(EnumeratorPtr::dynamicCast(cl2.front()));
                    }
                }
            }
        }
    }
    return result;
}

bool
Slice::Container::hasContentsWithMetadata(const string& meta) const
{
    for (const auto& content : contents())
    {
        if (content->hasMetadata(meta))
        {
            return true;
        }

        ContainerPtr container = ContainerPtr::dynamicCast(content);
        if (container && container->hasContentsWithMetadata(meta))
        {
            return true;
        }
    }
    return false;
}

string
Slice::Container::thisScope() const
{
    if (ContainedPtr contained = ContainedPtr::dynamicCast(const_cast<Container*>(this)))
    {
        return contained->scoped() + "::";
    }
    return "::";
}

bool
Slice::Container::checkIntroduced(const string& scoped, ContainedPtr namedThing)
{
    if (scoped[0] == ':') // Only unscoped names introduce anything.
    {
        return true;
    }

    // Split off first component.
    string::size_type pos = scoped.find("::");
    string firstComponent = pos == string::npos ? scoped : scoped.substr(0, pos);

    // If we don't have a type, the thing that is introduced is the contained for the first component.
    if (!namedThing)
    {
        ContainedList cl = lookupContained(firstComponent, false);
        if (cl.empty())
        {
            return true; // Ignore types whose creation failed previously.
        }
        namedThing = cl.front();
    }
    else
    {
        // For each scope, get the container until we have the container
        // for the first scope (which is the introduced one).
        ContainerPtr c;
        bool first = true;
        while (pos != string::npos)
        {
            if (first)
            {
                c = namedThing->container();
                first = false;
            }
            else
            {
                if (ContainedPtr contained = ContainedPtr::dynamicCast(c))
                {
                    c = contained->container();
                }
            }
            pos = scoped.find("::", pos + 2);
        }
        if (ContainedPtr contained = ContainedPtr::dynamicCast(c))
        {
            namedThing = contained;
        }
    }

    // Check if the first component is in the introduced map of this scope.
    auto it = _introducedMap.find(firstComponent);
    if (it == _introducedMap.end())
    {
        // We've just introduced the first component to the current scope and should insert it
        _introducedMap[firstComponent] = namedThing;
    }
    else
    {
        // We've previously introduced the first component to the current scope, check that it has not changed meaning.
        if (it->second != namedThing)
        {
            // Data members and parameters are in their own scope.
            if ((MemberPtr::dynamicCast(it->second) && !MemberPtr::dynamicCast(namedThing)) ||
               (!MemberPtr::dynamicCast(it->second) && MemberPtr::dynamicCast(namedThing)))
            {
                return true;
            }

            _unit->error("`" + firstComponent + "' has changed meaning");
            return false;
        }
    }
    return true;
}

Slice::Container::Container(const UnitPtr& ut) :
    SyntaxTreeBase(ut)
{
}

bool
Slice::Container::validateConstant(const string& name, const TypePtr& lhsType, SyntaxTreeBasePtr& valueType,
                                   const string& value, bool isConstant)
{
    if (!lhsType)
    {
        return false;
    }

    TypePtr type = lhsType;
    if (!isConstant)
    {
        // The type of the data member can be T?
        if (auto optional = OptionalPtr::dynamicCast(type))
        {
            type = optional->underlying();
        }
    }

    // First verify that it is legal to specify a constant or default value for the given type.

    BuiltinPtr b = BuiltinPtr::dynamicCast(type);
    EnumPtr e = EnumPtr::dynamicCast(type);

    if (b)
    {
        if (b->kind() == Builtin::KindAnyClass || b->kind() == Builtin::KindObject)
        {
            if (isConstant)
            {
                _unit->error("constant `" + name + "' has illegal type: `" + b->kindAsString() + "'");
            }
            else
            {
                _unit->error("default value not allowed for data member `" + name + "' of type `" +
                             b->kindAsString() + "'");
            }
            return false;
        }
    }
    else if (!e)
    {
        if (isConstant)
        {
            _unit->error("constant `" + name + "' has illegal type");
        }
        else
        {
            _unit->error("default value not allowed for data member `" + name + "'");
        }
        return false;
    }

    // If valueType is a ConstPtr, it means the constant or data member being defined refers to another constant.
    const ConstPtr constant = ConstPtr::dynamicCast(valueType);

    // isConstant indicates whether a constant or a data member (with a default value) is being defined.
    const string desc = isConstant ? "constant" : "data member";

    // Next, verify that the type of the constant or data member is compatible with the given value.
    if (b)
    {
        BuiltinPtr lt;
        if (constant)
        {
            lt = BuiltinPtr::dynamicCast(constant->type());
        }
        else
        {
            lt = BuiltinPtr::dynamicCast(valueType);
        }

        if (lt)
        {
            bool ok = false;
            if (b->kind() == lt->kind())
            {
                ok = true;
            }
            else if (b->isIntegralType())
            {
                ok = lt->isIntegralType();
            }
            else if (b->isNumericType())
            {
                ok = lt->isNumericType();
            }

            if (!ok)
            {
                _unit->error("initializer of type `" + lt->kindAsString() + "' is incompatible with the type `" +
                             b->kindAsString() + "' of " + desc + " `" + name + "'");
                return false;
            }
        }
        else
        {
            _unit->error("type of initializer is incompatible with the type `" + b->kindAsString() + "' of " + desc +
                         " `" + name + "'");
            return false;
        }

        // Check that numeric values are within the type's legal range.
        if (b->isIntegralType())
        {
            auto [min, max] = b->integralRange();
            try
            {
                if (b->kind() == Builtin::KindULong)
                {
                    stoull(value); // throws out_of_range if value is out of range
                }
                else
                {
                    auto val = stoll(value);
                    if (val < min || val > static_cast<int64_t>(max))
                    {
                        throw out_of_range("");
                    }
                }
            }
            catch (const out_of_range&)
            {
                ostringstream oss;
                oss.imbue(underscoreSeparatorLocale);
                oss << "initializer `" << value << "' for " << desc << " " << name << " is outside the range of "
                    << b->kindAsString() << ": [" << min << ".." << max << "]";
                _unit->error(oss.str());
                return false;
            }
        }
    }

    if (e)
    {
        if (constant)
        {
            if (e != EnumPtr::dynamicCast(constant->type()))
            {
                _unit->error("type of initializer is incompatible with the type of " + desc + " `" + name + "'");
                return false;
            }
        }
        else
        {
            if (valueType)
            {
                EnumeratorPtr lte = EnumeratorPtr::dynamicCast(valueType);
                if (!lte)
                {
                    _unit->error("type of initializer is incompatible with the type of " + desc + " `" + name + "'");
                    return false;
                }
                EnumeratorList elist = e->enumerators();
                if (find(elist.begin(), elist.end(), lte) == elist.end())
                {
                    _unit->error("enumerator `" + value + "' is not defined in enumeration `" + e->scoped() + "'");
                    return false;
                }
            }
            else
            {
                // Check if value designates an enumerator of e
                string newVal = value;
                string::size_type lastColon = value.rfind(':');
                if (lastColon != string::npos && lastColon + 1 < value.length())
                {
                    newVal = value.substr(0, lastColon + 1) + e->name() + "::" + value.substr(lastColon + 1);
                }

                ContainedList clist = e->lookupContained(newVal, false);
                if (clist.empty())
                {
                    _unit->error("`" + value + "' does not designate an enumerator of `" + e->scoped() + "'");
                    return false;
                }

                if (EnumeratorPtr lte = EnumeratorPtr::dynamicCast(clist.front()))
                {
                    valueType = lte;
                    if (lastColon != string::npos)
                    {
                        _unit->warning(Deprecated, string("referencing enumerator `") + lte->name() +
                                       "' in its enumeration's enclosing scope is deprecated");
                    }
                }
                else
                {
                    _unit->error("type of initializer is incompatible with the type of " + desc + " `" + name + "'");
                    return false;
                }
            }
        }
    }
    return true;
}

// ----------------------------------------------------------------------
// DataMemberContainer
// ----------------------------------------------------------------------

void
Slice::DataMemberContainer::destroy()
{
    _dataMembers.clear();
    Container::destroy();
}

MemberPtr
Slice::DataMemberContainer::createDataMember(const string& name, const TypePtr& type, bool tagged, int tag,
                                             const SyntaxTreeBasePtr& defaultValueType, const string& defaultValue,
                                             const string& defaultLiteral)
{
    _unit->checkType(type);
    if (!checkForRedefinition(this, name,  "data member"))
    {
        return nullptr;
    }

    // Check whether the data member has the same name as it's enclosing container.
    const string typeName = this->kindOf();
    if (name == this->name())
    {
        _unit->error("data member `" + name + "' cannot have the same name as its enclosing " + typeName);
        _unit->note(this, typeName + " `" + this->name() + "' is defined here");
        return nullptr;
    }

    if (ciequals(name, this->name()))
    {
        _unit->error("data member `" + name + "' differs only in capitalization from its enclosing " + typeName +
                     " named `" + this->name() + "'");
        _unit->note(this, typeName + " `" + this->name() + "' is defined here");
        return nullptr;
    }

    SyntaxTreeBasePtr dvt = defaultValueType;
    string dv = defaultValue;
    string dl = defaultLiteral;

    if (dvt || (EnumPtr::dynamicCast(type) && !dv.empty()))
    {
        // Validate the default value.
        if (!validateConstant(name, type, dvt, dv, false))
        {
            // Create the data member anyway, just without the default value.
            dvt = nullptr;
            dv.clear();
            dl.clear();
        }
    }

    if (tagged && tag > -1)
    {
        // Validate the tag.
        for (const auto& member : _dataMembers)
        {
            if (member->tagged() && tag == member->tag())
            {
                _unit->error("tag for data member `" + name + "' is already in use");
                break;
            }
        }
    }

    MemberPtr member = new Member(this, name, type, tagged, tag, false, dvt, dv, dl);
    _dataMembers.push_back(member);
    return member;
}

MemberList
Slice::DataMemberContainer::dataMembers() const
{
    return _dataMembers;
}

pair<MemberList, MemberList>
Slice::DataMemberContainer::sortedDataMembers() const
{
    return getSortedMembers(_dataMembers);
}

bool
Slice::DataMemberContainer::hasDataMembers() const
{
    return !_dataMembers.empty();
}

bool
Slice::DataMemberContainer::hasBaseDataMembers() const
{
    return false;
}

ContainedList
Slice::DataMemberContainer::contents() const
{
    ContainedList result;
    result.insert(result.end(), _dataMembers.begin(), _dataMembers.end());
    return result;
}

bool
Slice::DataMemberContainer::uses(const ContainedPtr&) const
{
    // No uses() implementation here. DataMember has its own uses().
    return false;
}

Slice::DataMemberContainer::DataMemberContainer(const ContainerPtr& container, const std::string& name) :
    SyntaxTreeBase(container->unit()),
    Container(container->unit()),
    Contained(container, name)
{
}

// ----------------------------------------------------------------------
// Module
// ----------------------------------------------------------------------

void
Slice::Module::destroy()
{
    for (const auto& content : _contents)
    {
        content->destroy();
    }
    _contents.clear();
    Container::destroy();
}

ContainedList
Slice::Module::contents() const
{
    return _contents;
}

bool
Slice::Module::uses(const ContainedPtr&) const
{
    return false;
}

string
Slice::Module::kindOf() const
{
    return "module";
}

void
Slice::Module::visit(ParserVisitor* visitor, bool all)
{
    if (visitor->visitModuleStart(this))
    {
        for (const auto& content : _contents)
        {
            if (all || content->includeLevel() == 0)
            {
                content->visit(visitor, all);
            }
        }
        visitor->visitModuleEnd(this);
    }
}

ModulePtr
Slice::Module::createModule(const string& name)
{
    // We use binary or here to avoid short-circuiting and ensure both checks are always run.
    if (!checkForRedefinition(this, name, "module") | !checkIdentifier(name))
    {
        return nullptr;
    }

    ModulePtr q = new Module(this, name);
    _contents.push_back(q);
    return q;
}

ClassDefPtr
Slice::Module::createClassDef(const string& name, int id, const ClassDefPtr& base)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    for (const auto& match : matches)
    {
        if (ClassDeclPtr::dynamicCast(match))
        {
            continue; // all good
        }

        bool differsOnlyInCase = match->name() != name;
        if (ClassDefPtr::dynamicCast(match))
        {
            if (differsOnlyInCase)
            {
                _unit->error("class definition `" + name
                             + "' is capitalized inconsistently with its previous name: `" + match->name() + "'");
                _unit->note(match, "class `" + match->name() + "' was originally defined here");
            }
            else
            {
                _unit->error("redefinition of class `" + name + "'");
                _unit->note(match, "class `" + name + "' was originally defined here");
            }
        }
        else if (differsOnlyInCase)
        {
            _unit->error("class definition `" + name + "' differs only in capitalization from the "
                        + match->kindOf() + " named `" + match->name() + "'");
            _unit->note(match, match->kindOf() + " `" + match->name() + "' is defined here");
        }
        else
        {
            bool declared = InterfaceDeclPtr::dynamicCast(match);
            _unit->error("class `" + name + "' was previously " + (declared ? "declared" : "defined") + " as "
                        + prependA(match->kindOf()));
            _unit->note(match, match->kindOf() + " `" + name + "' was originally defined here");
        }
        return nullptr;
    }

    if (!checkIdentifier(name))
    {
        return nullptr;
    }
    if (_name == "")
    {
        _unit->error("`" + name + "': a class can only be defined at module scope");
    }

    ClassDefPtr def = new ClassDef(this, name, id, base);
    _contents.push_back(def);

    for (const auto& match : matches)
    {
        if (ClassDeclPtr decl = ClassDeclPtr::dynamicCast(match))
        {
            decl->_definition = def;
        }
    }

    // Implicitly create a class declaration for each class
    // definition. This way the code generator can rely on always
    // having a class declaration available for lookup.
    def->_declaration = createClassDecl(name);

    return def;
}

ClassDeclPtr
Slice::Module::createClassDecl(const string& name)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    for (const auto& match : matches)
    {
        if (ClassDefPtr::dynamicCast(match) || ClassDeclPtr::dynamicCast(match))
        {
            continue;
        }

        if (match->name() != name)
        {
            _unit->error("class declaration `" + name + "' differs only in capitalization from the " + match->kindOf()
                        + " named `" + match->name() + "'");
            _unit->note(match, match->kindOf() + " `" + match->name() + "' is defined here");
        }
        else
        {
            bool declared = InterfaceDeclPtr::dynamicCast(match);
            _unit->error("class `" + name + "' was previously " + (declared ? "declared" : "defined") + " as "
                        + prependA(match->kindOf()));
            _unit->note(match, match->kindOf() + " `" + name + "' was originally defined here");
        }
        return nullptr;
    }

    if (!checkIdentifier(name))
    {
        return nullptr;
    }
    if (_name == "")
    {
        _unit->error("`" + name + "': a class can only be defined at module scope");
    }

    // Multiple declarations are permissible. But if we do already
    // have a declaration for the class in this container, we don't
    // create another one.
    ClassDefPtr def;
    for (const auto& content : _contents)
    {
        if (content->name() == name)
        {
            if (ClassDeclPtr decl = ClassDeclPtr::dynamicCast(content))
            {
                return decl;
            }

            def = ClassDefPtr::dynamicCast(content);
            assert(def);
        }
    }

    ClassDeclPtr decl = new ClassDecl(this, name);
    _contents.push_back(decl);

    if (def)
    {
        decl->_definition = def;
    }

    return decl;
}

InterfaceDefPtr
Slice::Module::createInterfaceDef(const string& name, const InterfaceList& bases)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    for (const auto& match : matches)
    {
        if (InterfaceDeclPtr::dynamicCast(match))
        {
            continue; // all good
        }

        bool differsOnlyInCase = match->name() != name;
        if (InterfaceDefPtr::dynamicCast(match))
        {
            if (differsOnlyInCase)
            {
                _unit->error("interface definition `" + name
                             + "' is capitalized inconsistently with its previous name: `" + match->name() + "'");
                _unit->note(match, "interface `" + match->name() + "' was originally defined here");
            }
            else
            {
                _unit->error("redefinition of interface `" + name + "'");
                _unit->note(match, "interface `" + name + "' was originally defined here");
            }
        }
        else if (differsOnlyInCase)
        {
            _unit->error("interface definition `" + name + "' differs only in capitalization from the "
                        + match->kindOf() + " named `" + match->name() + "'");
            _unit->note(match, match->kindOf() + " `" + match->name() + "' is defined here");
        }
        else
        {
            bool declared = ClassDeclPtr::dynamicCast(match);
            _unit->error("interface `" + name + "' was previously " + (declared ? "declared" : "defined") + " as "
                        + prependA(match->kindOf()));
            _unit->note(match, match->kindOf() + " `" + name + "' was originally defined here");
        }
        return nullptr;
    }

    if (!checkIdentifier(name))
    {
        return nullptr;
    }
    if (_name == "")
    {
        _unit->error("`" + name + "': an interface can only be defined at module scope");
    }

    InterfaceDecl::checkBasesAreLegal(name, bases, _unit);

    InterfaceDefPtr def = new InterfaceDef(this, name, bases);
    _contents.push_back(def);

    for (const auto& match : matches)
    {
        if (InterfaceDeclPtr decl = InterfaceDeclPtr::dynamicCast(match))
        {
            decl->_definition = def;
        }
    }

    // Implicitly create an interface declaration for each interface
    // definition. This way the code generator can rely on always
    // having an interface declaration available for lookup.
    def->_declaration = createInterfaceDecl(name);

    return def;
}

InterfaceDeclPtr
Slice::Module::createInterfaceDecl(const string& name)
{
    ContainedList matches = _unit->findContents(thisScope() + name);
    for (const auto& match : matches)
    {
        if (InterfaceDefPtr::dynamicCast(match) || InterfaceDeclPtr::dynamicCast(match))
        {
            continue;
        }

        if (match->name() != name)
        {
            _unit->error("interface declaration `" + name + "' differs only in capitalization from the "
                        + match->kindOf() + " named `" + match->name() + "'");
            _unit->note(match, match->kindOf() + " `" + match->name() + "' is defined here");
        }
        else
        {
            bool declared = ClassDeclPtr::dynamicCast(match);
            _unit->error("interface `" + name + "' was previously " + (declared ? "declared" : "defined") + " as "
                        + prependA(match->kindOf()));
            _unit->note(match, match->kindOf() + " `" + name + "' was originally defined here");
        }
        return nullptr;
    }

    if (!checkIdentifier(name))
    {
        return nullptr;
    }
    if (_name == "")
    {
        _unit->error("`" + name + "': an interface can only be defined at module scope");
    }

    // Multiple declarations are permissible. But if we do already have a declaration for the interface in this
    // container, we don't create another one.
    InterfaceDefPtr def;
    for (const auto& content : _contents)
    {
        if (content->name() == name)
        {
            if (InterfaceDeclPtr decl = InterfaceDeclPtr::dynamicCast(content))
            {
                return decl;
            }

            def = InterfaceDefPtr::dynamicCast(content);
            assert(def);
        }
    }

    InterfaceDeclPtr decl = new InterfaceDecl(this, name);
    _contents.push_back(decl);

    if (def)
    {
        decl->_definition = def;
    }

    return decl;
}

ExceptionPtr
Slice::Module::createException(const string& name, const ExceptionPtr& base, NodeType nt)
{
    if (!checkForRedefinition(this, name, "exception"))
    {
        return nullptr;
    }

    checkIdentifier(name); // Don't return here -- we create the exception anyway.
    if (nt == Real && _name == "")
    {
        _unit->error("`" + name + "': an exception can only be defined at module scope");
    }

    ExceptionPtr p = new Exception(this, name, base);
    _contents.push_back(p);
    return p;
}

StructPtr
Slice::Module::createStruct(const string& name, NodeType nt)
{
    if (!checkForRedefinition(this, name, "struct"))
    {
        return nullptr;
    }

    checkIdentifier(name); // Don't return here -- we create the struct anyway.
    if (nt == Real && _name == "")
    {
        _unit->error("`" + name + "': a structure can only be defined at module scope");
    }

    StructPtr p = new Struct(this, name);
    _contents.push_back(p);
    return p;
}

TypeAliasPtr
Slice::Module::createTypeAlias(const string& name, const TypePtr& type, const StringList& metadata)
{
    if (!checkForRedefinition(this, name, "typealias"))
    {
        return nullptr;
    }

    checkIdentifier(name); // Don't return here -- we create the alias anyway.
    if (_name == "")
    {
        _unit->error("`" + name + "': a typealias can only be defined at module scope");
    }

    // Ensure the type isn't an alias itself. These should already be unwrapped by the grammar.
    assert(!TypeAliasPtr::dynamicCast(type));

    // Metadata cannot be specified on aliases of classes and interfaces.
    if (!metadata.empty())
    {
        if (ClassDeclPtr::dynamicCast(type))
        {
            _unit->error("illegal metadata on `" + name + "': metadata cannot be specified on aliases of classes");
        }
        else if (InterfaceDeclPtr::dynamicCast(type))
        {
            _unit->error("illegal metadata on `" + name + "': metadata cannot be specified on aliases of interfaces");
        }
    }

    TypeAliasPtr alias = new TypeAlias(this, name, type, metadata);
    _contents.push_back(alias);
    return alias;
}

SequencePtr
Slice::Module::createSequence(const string& name, const TypePtr& type, const StringList& metadata)
{
    _unit->checkType(type);
    if (!checkForRedefinition(this, name, "sequence"))
    {
        return nullptr;
    }

    checkIdentifier(name); // Don't return here -- we create the sequence anyway.
    if (_name == "")
    {
        _unit->error("`" + name + "': a sequence can only be defined at module scope");
    }

    SequencePtr p = new Sequence(this, name, type, metadata);
    _contents.push_back(p);
    return p;
}

DictionaryPtr
Slice::Module::createDictionary(const string& name, const TypePtr& keyType, const StringList& keyMetadata,
                                const TypePtr& valueType, const StringList& valueMetadata)
{
    _unit->checkType(keyType);
    _unit->checkType(valueType);
    if (!checkForRedefinition(this, name, "dictionary"))
    {
        return nullptr;
    }

    checkIdentifier(name); // Don't return here -- we create the dictionary anyway.
    if (_name == "")
    {
        _unit->error("`" + name + "': a dictionary can only be defined at module scope");
    }

    bool containsSequence = false;
    if (!Dictionary::legalKeyType(keyType, containsSequence))
    {
        _unit->error("dictionary `" + name + "' uses an illegal key type");
        return nullptr;
    }
    if (containsSequence)
    {
        _unit->warning(Deprecated, "use of sequences in dictionary keys has been deprecated");
    }

    DictionaryPtr p = new Dictionary(this, name, keyType, keyMetadata, valueType, valueMetadata);
    _contents.push_back(p);
    return p;
}

EnumPtr
Slice::Module::createEnum(const string& name, bool unchecked, NodeType nt)
{
    if (!checkForRedefinition(this, name, "enumeration"))
    {
        return nullptr;
    }

    checkIdentifier(name); // Don't return here -- we create the enumeration anyway.
    if (nt == Real && _name == "")
    {
        _unit->error("`" + name + "': an enumeration can only be defined at module scope");
    }

    EnumPtr p = new Enum(this, name, unchecked);
    _contents.push_back(p);
    return p;
}

ConstPtr
Slice::Module::createConst(const string name, const TypePtr& constType, const StringList& metadata,
                           const SyntaxTreeBasePtr& valueType, const string& value, const string& literal,
                           NodeType nt)
{
    if (!checkForRedefinition(this, name, "constant"))
    {
        return nullptr;
    }

    checkIdentifier(name); // Don't return here -- we create the constant anyway.
    if (nt == Real && _name == "")
    {
        _unit->error("`" + name + "': a constant can only be defined at module scope");
    }

    // Validate the constant and its value; for enums, find enumerator
    SyntaxTreeBasePtr resolvedValueType = valueType;
    if (nt == Real && !validateConstant(name, constType, resolvedValueType, value, true))
    {
        return nullptr;
    }

    ConstPtr p = new Const(this, name, constType, metadata, resolvedValueType, value, literal);
    _contents.push_back(p);
    return p;
}

EnumList
Slice::Module::enums() const
{
    EnumList result;
    for (const auto& content : _contents)
    {
        if (EnumPtr e = EnumPtr::dynamicCast(content))
        {
            result.push_back(e);
        }
    }
    return result;
}

ConstList
Slice::Module::consts() const
{
    ConstList result;
    for (const auto& content : _contents)
    {
        if (ConstPtr c = ConstPtr::dynamicCast(content))
        {
            result.push_back(c);
        }
    }
    return result;
}

bool
Slice::Module::hasConsts() const
{
    for (const auto& content : _contents)
    {
        if (ConstPtr::dynamicCast(content))
        {
            return true;
        }

        ModulePtr submodule = ModulePtr::dynamicCast(content);
        if (submodule && submodule->hasConsts())
        {
            return true;
        }
    }
    return false;
}

bool
Slice::Module::hasStructs() const
{
    for (const auto& content : _contents)
    {
        if (StructPtr::dynamicCast(content))
        {
            return true;
        }

        ModulePtr submodule = ModulePtr::dynamicCast(content);
        if (submodule && submodule->hasStructs())
        {
            return true;
        }
    }
    return false;
}

bool
Slice::Module::hasExceptions() const
{
    for (const auto& content : _contents)
    {
        if (ExceptionPtr::dynamicCast(content))
        {
            return true;
        }

        ModulePtr submodule = ModulePtr::dynamicCast(content);
        if (submodule && submodule->hasExceptions())
        {
            return true;
        }
    }
    return false;
}

bool
Slice::Module::hasEnums() const
{
    for (const auto& content : _contents)
    {
        if (EnumPtr::dynamicCast(content))
        {
            return true;
        }

        ModulePtr submodule = ModulePtr::dynamicCast(content);
        if (submodule && submodule->hasEnums())
        {
            return true;
        }
    }
    return false;
}

bool
Slice::Module::hasClassDecls() const
{
    for (const auto& content : _contents)
    {
        if (ClassDeclPtr::dynamicCast(content))
        {
            return true;
        }

        ModulePtr submodule = ModulePtr::dynamicCast(content);
        if (submodule && submodule->hasClassDecls())
        {
            return true;
        }
    }
    return false;
}

bool
Slice::Module::hasClassDefs() const
{
    for (const auto& content : _contents)
    {
        if (ClassDefPtr::dynamicCast(content))
        {
            return true;
        }

        ModulePtr submodule = ModulePtr::dynamicCast(content);
        if (submodule && submodule->hasClassDefs())
        {
            return true;
        }
    }
    return false;
}

bool
Slice::Module::hasInterfaceDecls() const
{
    for (const auto& content : _contents)
    {
        if (InterfaceDeclPtr::dynamicCast(content))
        {
            return true;
        }

        ModulePtr submodule = ModulePtr::dynamicCast(content);
        if (submodule && submodule->hasInterfaceDecls())
        {
            return true;
        }
    }
    return false;
}

bool
Slice::Module::hasInterfaceDefs() const
{
    for (const auto& content : _contents)
    {
        if (InterfaceDefPtr::dynamicCast(content))
        {
            return true;
        }

        ModulePtr submodule = ModulePtr::dynamicCast(content);
        if (submodule && submodule->hasInterfaceDefs())
        {
            return true;
        }
    }
    return false;
}

bool
Slice::Module::hasOnlyClassDecls() const
{
    for (const auto& content : _contents)
    {
        if (ModulePtr submodule = ModulePtr::dynamicCast(content))
        {
            if (!submodule->hasOnlyClassDecls())
            {
                return false;
            }
        }
        else if (!ClassDeclPtr::dynamicCast(content))
        {
            return false;
        }
    }
    return true;
}

bool
Slice::Module::hasOnlyInterfaces() const
{
    for (const auto& content : _contents)
    {
        if (ModulePtr submodule = ModulePtr::dynamicCast(content))
        {
            if (!submodule->hasOnlyInterfaces())
            {
                return false;
            }
        }
        else if (!InterfaceDeclPtr::dynamicCast(content) && !InterfaceDefPtr::dynamicCast(content))
        {
            return false;
        }
    }
    return true;
}

bool
Slice::Module::hasNonClassTypes() const
{
    for (const auto& content : _contents)
    {
        if (auto submodule = ModulePtr::dynamicCast(content))
        {
            if (submodule->hasNonClassTypes())
            {
                return true;
            }
        }
        else if (!ClassDeclPtr::dynamicCast(content) && !ClassDefPtr::dynamicCast(content) &&
                 TypePtr::dynamicCast(content))
        {
            return true;
        }
        else if (ExceptionPtr::dynamicCast(content) || ConstPtr::dynamicCast(content))
        {
            return true;
        }
    }
    return false;
}

Slice::Module::Module(const ContainerPtr& container, const string& name) :
    SyntaxTreeBase(container->unit()),
    Container(container->unit()),
    Contained(container, name)
{
}

bool
Slice::Module::hasOnlySubModules() const
{
    for (const auto& contained : _contents)
    {
        if (!ModulePtr::dynamicCast(contained))
        {
            return false;
        }
    }
    return true;
}

// ----------------------------------------------------------------------
// ClassDecl
// ----------------------------------------------------------------------

void
Slice::ClassDecl::destroy()
{
    _definition = nullptr;
    SyntaxTreeBase::destroy();
}

ClassDefPtr
Slice::ClassDecl::definition() const
{
    return _definition;
}

string
Slice::ClassDecl::typeId() const
{
    return scoped();
}

bool
Slice::ClassDecl::uses(const ContainedPtr&) const
{
    return false;
}

bool
Slice::ClassDecl::usesClasses() const
{
    return true;
}

size_t
Slice::ClassDecl::minWireSize() const
{
    return 1; // Can be a single byte when the instance is marshaled as an index.
}

string
Slice::ClassDecl::getTagFormat() const
{
    return "Class";
}

bool
Slice::ClassDecl::isVariableLength() const
{
    return true;
}

string
Slice::ClassDecl::kindOf() const
{
    return "class";
}

void
Slice::ClassDecl::visit(ParserVisitor* visitor, bool)
{
    visitor->visitClassDecl(this);
}

Slice::ClassDecl::ClassDecl(const ContainerPtr& container, const string& name) :
    SyntaxTreeBase(container->unit()),
    Contained(container, name),
    Type(container->unit())
{
}

// ----------------------------------------------------------------------
// ClassDef
// ----------------------------------------------------------------------

void
Slice::ClassDef::destroy()
{
    _declaration = nullptr;
    _base = nullptr;
    DataMemberContainer::destroy();
}

MemberPtr
Slice::ClassDef::createDataMember(const string& name, const TypePtr& type, bool tagged, int tag,
                                  const SyntaxTreeBasePtr& defaultValueType, const string& defaultValue,
                                  const string& defaultLiteral)
{
    // Check whether any bases have defined something with the same name already.
    if (_base)
    {
        for (const auto& member : _base->allDataMembers())
        {
            if (member->name() == name)
            {
                ClassDefPtr container = ClassDefPtr::dynamicCast(member->container());
                _unit->error("data member `" + name + "' is already defined in a base class");
                _unit->note(member, "data member `" + name + "' was originally defined in class `" + container->name()
                            + "'");
                return nullptr;
            }

            if (ciequals(member->name(), name))
            {
                ClassDefPtr container = ClassDefPtr::dynamicCast(member->container());
                _unit->error("data member `" + name + "' differs only in capitalization from the data member named `"
                             + member->name() + "', which is defined in a base class");
                _unit->note(member, "data member `" + member->name() + "' is defined in the class `" + container->name()
                            + "'");
            }
        }
    }

    return DataMemberContainer::createDataMember(name, type, tagged, tag, defaultValueType, defaultValue,
                                                 defaultLiteral);
}

ClassDeclPtr
Slice::ClassDef::declaration() const
{
    return _declaration;
}

ClassDefPtr
Slice::ClassDef::base() const
{
    return _base;
}

ClassList
Slice::ClassDef::allBases() const
{
    ClassList result;
    if (_base)
    {
        result.push_back(_base);
        result.merge(_base->allBases());
    }
    return result;
}

// Return the data members of this class and its parent classes, in base-to-derived order.
MemberList
Slice::ClassDef::allDataMembers() const
{
    MemberList result = _dataMembers;
    // Check if we have a base class. If so, recursively get the data members of the base(s).
    if (_base)
    {
        result.splice(result.begin(), _base->allDataMembers());
    }
    return result;
}

bool
Slice::ClassDef::isA(const string& id) const
{
    if (id == _scoped)
    {
        return true;
    }
    return _base && _base->isA(id);
}

bool
Slice::ClassDef::inheritsMetadata(const string& meta) const
{
    return _base && (_base->hasMetadata(meta) || _base->inheritsMetadata(meta));
}

bool
Slice::ClassDef::hasBaseDataMembers() const
{
    return _base && !_base->allDataMembers().empty();
}

string
Slice::ClassDef::kindOf() const
{
    return "class";
}

void
Slice::ClassDef::visit(ParserVisitor* visitor, bool all)
{
    if (visitor->visitClassDefStart(this))
    {
        for (const auto& member : _dataMembers)
        {
            if (all || member->includeLevel() == 0)
            {
                member->visit(visitor, all);
            }
        }
        visitor->visitClassDefEnd(this);
    }
}

int
Slice::ClassDef::compactId() const
{
    return _compactId;
}

StringList
Slice::ClassDef::ids() const
{
    StringList ids;
    ClassList bases = allBases();
    transform(bases.begin(), bases.end(), back_inserter(ids), [](const auto& c) { return c->scoped(); });
    StringList other;
    other.push_back(scoped());
    other.push_back("::Ice::Object");
    other.sort();
    ids.merge(other);
    ids.unique();
    return ids;
}

Slice::ClassDef::ClassDef(const ContainerPtr& container, const string& name, int id, const ClassDefPtr& base) :
    SyntaxTreeBase(container->unit()),
    Container(container->unit()),
    Contained(container, name),
    DataMemberContainer(container, name),
    _base(base),
    _compactId(id)
{
    if (_compactId >= 0)
    {
        _unit->addTypeId(_compactId, scoped());
    }
}

// ----------------------------------------------------------------------
// InterfaceDecl
// ----------------------------------------------------------------------

void
Slice::InterfaceDecl::destroy()
{
    _definition = nullptr;
    SyntaxTreeBase::destroy();
}

InterfaceDefPtr
Slice::InterfaceDecl::definition() const
{
    return _definition;
}

string
Slice::InterfaceDecl::typeId() const
{
    return scoped();
}

bool
Slice::InterfaceDecl::uses(const ContainedPtr&) const
{
    return false;
}

bool
Slice::InterfaceDecl::usesClasses() const
{
    return false;
}

size_t
Slice::InterfaceDecl::minWireSize() const
{
    return 3; // See Object
}

string
Slice::InterfaceDecl::getTagFormat() const
{
    return "FSize"; // with the 1.1 encoding, the size of a proxy is encoded using a fixed-length size
}

bool
Slice::InterfaceDecl::isVariableLength() const
{
    return true;
}

string
Slice::InterfaceDecl::kindOf() const
{
    return "interface";
}

void
Slice::InterfaceDecl::visit(ParserVisitor* visitor, bool)
{
    visitor->visitInterfaceDecl(this);
}

void
Slice::InterfaceDecl::checkBasesAreLegal(const string& name, const InterfaceList& bases,
                                     const UnitPtr& ut)
{
    // Check whether, for multiple inheritance, any of the bases define
    // the same operations.
    if (bases.size() > 1)
    {
        // We have multiple inheritance. Build a list of paths through the
        // inheritance graph, such that multiple inheritance is legal if
        // the union of the names defined in classes on each path are disjoint.
        GraphPartitionList gpl;
        for (const auto& base : bases)
        {
            InterfaceList cl;
            gpl.push_back(cl);
            addPartition(gpl, gpl.rbegin(), base);
        }

        // We now have a list of partitions, with each partition containing
        // a list of class definitions. Turn the list of partitions of class
        // definitions into a list of sets of strings, with each
        // set containing the names of operations and data members defined in
        // the classes in each partition.
        StringPartitionList spl = toStringPartitionList(gpl);

        // Multiple inheritance is legal if no two partitions contain a common
        // name (that is, if the union of the intersections of all possible pairs
        // of partitions is empty).
        checkPairIntersections(spl, name, ut);
    }
}

Slice::InterfaceDecl::InterfaceDecl(const ContainerPtr& container, const string& name) :
    SyntaxTreeBase(container->unit()),
    Contained(container, name),
    Type(container->unit())
{
}

// Return true if the interface definition cdp is on one of the interface lists in gpl, false otherwise.
bool
Slice::InterfaceDecl::isInList(const GraphPartitionList& gpl, const InterfaceDefPtr& cdp)
{
    for (const auto& i : gpl)
    {
        if (find(i.begin(), i.end(), cdp) != i.end())
        {
            return true;
        }
    }
    return false;
}

void
Slice::InterfaceDecl::addPartition(GraphPartitionList& gpl,
                               GraphPartitionList::reverse_iterator tail,
                               const InterfaceDefPtr& base)
{
    // If this base is on one of the partition lists already, do nothing.
    if (isInList(gpl, base))
    {
        return;
    }
    // Put the current base at the end of the current partition.
    tail->push_back(base);
    // If the base has bases in turn, recurse, adding the first base
    // of base (the left-most "grandbase") to the current partition.
    if (base->bases().size())
    {
        addPartition(gpl, tail, *(base->bases().begin()));
    }
    // If the base has multiple bases, each of the "grandbases"
    // except for the left-most (which we just dealt with)
    // adds a new partition.
    if (base->bases().size() > 1)
    {
        InterfaceList grandBases = base->bases();
        InterfaceList::const_iterator i = grandBases.begin();
        while (++i != grandBases.end())
        {
            InterfaceList cl;
            gpl.push_back(cl);
            addPartition(gpl, gpl.rbegin(), *i);
        }
    }
}

// Convert the list of partitions of interface definitions into a
// list of lists, with each member list containing the operation
// names defined by the interfaces in each partition.
Slice::InterfaceDecl::StringPartitionList
Slice::InterfaceDecl::toStringPartitionList(const GraphPartitionList& gpl)
{
    StringPartitionList spl;
    for (const auto& part : gpl)
    {
        StringList sl;
        spl.push_back(sl);
        for (const auto& interface : part)
        {
            for (const auto& operation : interface->operations())
            {
                spl.rbegin()->push_back(operation->name());
            }
        }
    }
    return spl;
}

// For all (unique) pairs of string lists, check whether an identifier in one list occurs
// in the other and, if so, complain.
void
Slice::InterfaceDecl::checkPairIntersections(const StringPartitionList& l, const string& name, const UnitPtr& ut)
{
    set<string> reported;
    for (StringPartitionList::const_iterator i = l.begin(); i != l.end(); ++i)
    {
        StringPartitionList::const_iterator cursor = i;
        ++cursor;
        for (StringPartitionList::const_iterator j = cursor; j != l.end(); ++j)
        {
            for (const auto& s1 : *i)
            {
                for (const auto& s2 : *j)
                {
                    if (s1 == s2 && reported.find(s1) == reported.end())
                    {
                        ut->error("ambiguous multiple inheritance: `" + name + "' inherits operation `" + s1
                                  + "' from two or more unrelated base interfaces");
                        reported.insert(s1);
                    }
                    else if (ciequals(s1, s2) && reported.find(s1) == reported.end()
                            && reported.find(s2) == reported.end())
                    {
                        ut->error("ambiguous multiple inheritance: `" + name + "' inherits operations `" + s1 + "' and `"
                                  + s2 + "', which differ only in capitalization, from unrelated base interfaces");
                        reported.insert(s1);
                        reported.insert(s2);
                    }
                }
            }
        }
    }
}

// ----------------------------------------------------------------------
// InterfaceDef
// ----------------------------------------------------------------------

void
Slice::InterfaceDef::destroy()
{
    _declaration = nullptr;
    _bases.clear();
    _operations.clear();
    Container::destroy();
}

OperationPtr
Slice::InterfaceDef::createOperation(const string& name,
                                     Operation::Mode mode)
{
    if (!checkForRedefinition(this, name, "operation"))
    {
        return nullptr;
    }

    // Check whether enclosing interface has the same name.
    if (name == this->name())
    {
        _unit->error("interface name `" + name + "' cannot be used as operation name");
        return nullptr;
    }

    if (ciequals(name, this->name()))
    {
        _unit->error("operation `" + name + "' differs only in capitalization from enclosing interface name `"
                     + this->name() + "'");
        return nullptr;
    }

    // Check whether any base has an operation with the same name already
    for (const auto& base : _bases)
    {
        for (const auto& operation : base->allOperations())
        {
            if (operation->name() == name)
            {
                InterfaceDefPtr container = InterfaceDefPtr::dynamicCast(operation->container());
                _unit->error("operation `" + name + "' is already defined in a base interface");
                _unit->note(operation, "operation `" + name + "' was originally defined in interface `"
                            + container->name() + "'");
                return nullptr;
            }

            if (ciequals(operation->name(), name))
            {
                InterfaceDefPtr container = InterfaceDefPtr::dynamicCast(operation->container());
                _unit->error("operation `" + name + "' differs only in capitalization from the operation named `"
                             + operation->name() + "', which is defined in a base interface");
                _unit->note(operation, "operation `" + operation->name() + "' is defined in the interface `"
                            + container->name() + "'");
            }
        }
    }

    OperationPtr op = new Operation(this, name, mode);
    _operations.push_back(op);
    return op;
}

InterfaceDeclPtr
Slice::InterfaceDef::declaration() const
{
    return _declaration;
}

InterfaceList
Slice::InterfaceDef::bases() const
{
    return _bases;
}

InterfaceList
Slice::InterfaceDef::allBases() const
{
    InterfaceList result = _bases;
    result.sort();
    result.unique();
    for (const auto& base : _bases)
    {
        result.merge(base->allBases());
        result.unique();
    }
    return result;
}

OperationList
Slice::InterfaceDef::operations() const
{
    return _operations;
}

OperationList
Slice::InterfaceDef::allOperations() const
{
    OperationList result;
    for (const auto& base : _bases)
    {
        for (const auto& operation : base->allOperations())
        {
            if (find(result.begin(), result.end(), operation) == result.end())
            {
                result.push_back(operation);
            }
        }
    }

    for (const auto& operation : _operations)
    {
        if (find(result.begin(), result.end(), operation) == result.end())
        {
            result.push_back(operation);
        }
    }

    return result;
}

bool
Slice::InterfaceDef::isA(const string& id) const
{
    if (id == _scoped)
    {
        return true;
    }
    for (const auto& base : _bases)
    {
        if (base->isA(id))
        {
            return true;
        }
    }
    return false;
}

bool
Slice::InterfaceDef::inheritsMetadata(const string& meta) const
{
    for (const auto& base : _bases)
    {
        if (base->hasMetadata(meta) || base->inheritsMetadata(meta))
        {
            return true;
        }
    }
    return false;
}

ContainedList
Slice::InterfaceDef::contents() const
{
    ContainedList result;
    result.insert(result.end(), _operations.begin(), _operations.end());
    return result;
}

bool
Slice::InterfaceDef::uses(const ContainedPtr&) const
{
    // No uses() implementation here.
    return false;
}

string
Slice::InterfaceDef::kindOf() const
{
    return "interface";
}

void
Slice::InterfaceDef::visit(ParserVisitor* visitor, bool all)
{
    if (visitor->visitInterfaceDefStart(this))
    {
        for (const auto& operation : _operations)
        {
            if (all || operation->includeLevel() == 0)
            {
                operation->visit(visitor, all);
            }
        }
        visitor->visitInterfaceDefEnd(this);
    }
}

StringList
Slice::InterfaceDef::ids() const
{
    StringList ids;
    InterfaceList bases = allBases();
    transform(bases.begin(), bases.end(), back_inserter(ids), [](const auto& c) { return c->scoped(); });
    StringList other;
    other.push_back(scoped());
    other.push_back("::Ice::Object");
    other.sort();
    ids.merge(other);
    ids.unique();
    return ids;
}

Slice::InterfaceDef::InterfaceDef(const ContainerPtr& container, const string& name, const InterfaceList& bases) :
    SyntaxTreeBase(container->unit()),
    Container(container->unit()),
    Contained(container, name),
    _bases(bases)
{
}

// ----------------------------------------------------------------------
// Optional
// ----------------------------------------------------------------------

Slice::Optional::Optional(const TypePtr& underlying) :
    SyntaxTreeBase(underlying->unit(), underlying->definitionContext()),
    Type(underlying->unit()),
    _underlying(underlying)
{
}

string
Slice::Optional::typeId() const
{
    return _underlying->typeId();
}

bool
Slice::Optional::usesClasses() const
{
    return _underlying->usesClasses();
}

bool
Slice::Optional::isClassType() const
{
    return _underlying->isClassType();
}

bool
Slice::Optional::isInterfaceType() const
{
    return _underlying->isInterfaceType();
}

size_t
Slice::Optional::minWireSize() const
{
    if (isClassType())
    {
        return 1;
    }
    else if (isInterfaceType())
    {
        return 2;
    }
    else
    {
        return 0;
    }
}

string
Slice::Optional::getTagFormat() const
{
    return _underlying->getTagFormat();
}

bool
Slice::Optional::isVariableLength() const
{
    return _underlying->isVariableLength();
}

// ----------------------------------------------------------------------
// Exception
// ----------------------------------------------------------------------

void
Slice::Exception::destroy()
{
    _base = nullptr;
    DataMemberContainer::destroy();
}

MemberPtr
Slice::Exception::createDataMember(const string& name, const TypePtr& type, bool tagged, int tag,
                                   const SyntaxTreeBasePtr& defaultValueType, const string& defaultValue,
                                   const string& defaultLiteral)
{
    if (_base)
    {
        // Check whether any bases have defined a member with the same name already.
        for (const auto& member : _base->allDataMembers())
        {
            if (member->name() == name)
            {
                ExceptionPtr container = ExceptionPtr::dynamicCast(member->container());
                _unit->error("data member `" + name + "' is already defined in a base exception");
                _unit->note(member, "data member `" + name + "' was originally defined in exception `"
                            + container->name() + "'");
                return nullptr;
            }

            if (ciequals(member->name(), name))
            {
                ExceptionPtr container = ExceptionPtr::dynamicCast(member->container());
                _unit->error("data member `" + name + "' differs only in capitalization from the data member named `"
                                + member->name() + "', which is defined in a base exception");
                _unit->note(member, "data member `" + member->name() + "' is defined in the exception `"
                            + container->name() + "'");
            }
        }
    }

    return DataMemberContainer::createDataMember(name, type, tagged, tag, defaultValueType, defaultValue,
                                                 defaultLiteral);
}

// Return the data members of this exception and its parent exceptions, in base-to-derived order.
MemberList
Slice::Exception::allDataMembers() const
{
    MemberList result = _dataMembers;
    // Check if we have a base exception. If so, recursively get the data members of the base exception(s).
    if (_base)
    {
        result.splice(result.begin(), _base->allDataMembers());
    }
    return result;
}

ExceptionPtr
Slice::Exception::base() const
{
    return _base;
}

ExceptionList
Slice::Exception::allBases() const
{
    ExceptionList result;
    if (_base)
    {
        result = _base->allBases();
        result.push_front(_base);
    }
    return result;
}

bool
Slice::Exception::isBaseOf(const ExceptionPtr& other) const
{
    string scoped = this->scoped();
    if (scoped == other->scoped())
    {
        return false;
    }

    for (const auto& base : other->allBases())
    {
        if (scoped == base->scoped())
        {
            return true;
        }
    }
    return false;
}

bool
Slice::Exception::usesClasses(bool includeTagged) const
{
    for (const auto& member : _dataMembers)
    {
        if (member->type()->usesClasses() && (includeTagged || !member->tagged()))
        {
            return true;
        }
    }
    return (_base && _base->usesClasses(includeTagged));
}

bool
Slice::Exception::inheritsMetadata(const string& meta) const
{
    return (_base && (_base->hasMetadata(meta) || _base->inheritsMetadata(meta)));
}

bool
Slice::Exception::hasBaseDataMembers() const
{
    return _base && !_base->allDataMembers().empty();
}

string
Slice::Exception::kindOf() const
{
    return "exception";
}

void
Slice::Exception::visit(ParserVisitor* visitor, bool all)
{
    if (visitor->visitExceptionStart(this))
    {
        for (const auto& member : _dataMembers)
        {
            if (all || member->includeLevel() == 0)
            {
                member->visit(visitor, all);
            }
        }
        visitor->visitExceptionEnd(this);
    }
}

Slice::Exception::Exception(const ContainerPtr& container, const string& name, const ExceptionPtr& base) :
    SyntaxTreeBase(container->unit()),
    Container(container->unit()),
    Contained(container, name),
    DataMemberContainer(container, name),
    _base(base)
{
}

// ----------------------------------------------------------------------
// Struct
// ----------------------------------------------------------------------

MemberPtr
Slice::Struct::createDataMember(const string& name, const TypePtr& type, bool tagged, int tag,
                                const SyntaxTreeBasePtr& defaultValueType, const string& defaultValue,
                                const string& defaultLiteral)
{
    // Structs cannot contain themselves.
    if (type.get() == this)
    {
        _unit->error("struct `" + this->name() + "' cannot contain itself");
        return nullptr;
    }

    if (tagged)
    {
        _unit->error("tagged data members are not supported in structs");
    }

    return DataMemberContainer::createDataMember(name, type, tagged, tag, defaultValueType, defaultValue,
                                                 defaultLiteral);
}

string
Slice::Struct::typeId() const
{
    return scoped();
}

bool
Slice::Struct::usesClasses() const
{
    for (const auto& member : _dataMembers)
    {
        if (member->type()->usesClasses())
        {
            return true;
        }
    }
    return false;
}

size_t
Slice::Struct::minWireSize() const
{
    // At least the sum of the minimum member sizes.
    size_t sz = 0;
    for (const auto& member : _dataMembers)
    {
        sz += member->type()->minWireSize();
    }
    return sz;
}

string
Slice::Struct::getTagFormat() const
{
    return isVariableLength() ? "FSize" : "VSize";
}

bool
Slice::Struct::isVariableLength() const
{
    for (const auto& member : _dataMembers)
    {
        if (member->type()->isVariableLength())
        {
            return true;
        }
    }
    return false;
}

string
Slice::Struct::kindOf() const
{
    return "struct";
}

void
Slice::Struct::visit(ParserVisitor* visitor, bool all)
{
    if (visitor->visitStructStart(this))
    {
        for (const auto& member : _dataMembers)
        {
            if (all || member->includeLevel() == 0)
            {
                member->visit(visitor, all);
            }
        }
        visitor->visitStructEnd(this);
    }
}

Slice::Struct::Struct(const ContainerPtr& container, const string& name) :
    SyntaxTreeBase(container->unit()),
    Container(container->unit()),
    Contained(container, name),
    DataMemberContainer(container, name),
    Type(container->unit())
{
}

// ----------------------------------------------------------------------
// TypeAlias
// ----------------------------------------------------------------------

void
Slice::TypeAlias::destroy()
{
    _underlying = nullptr;
    SyntaxTreeBase::destroy();
}

string
Slice::TypeAlias::typeId() const
{
    return _underlying->typeId();
}

bool
Slice::TypeAlias::uses(const ContainedPtr& contained) const
{
    ContainedPtr contained2 = ContainedPtr::dynamicCast(_underlying);
    return (contained2 && contained2 == contained);
}

bool
Slice::TypeAlias::usesClasses() const
{
    return _underlying->usesClasses();
}

string
Slice::TypeAlias::kindOf() const
{
    return "typealias";
}

bool
Slice::TypeAlias::isClassType() const
{
    return _underlying->isClassType();
}

bool
Slice::TypeAlias::isInterfaceType() const
{
    return _underlying->isInterfaceType();
}

size_t
Slice::TypeAlias::minWireSize() const
{
    return _underlying->minWireSize();
}

string
Slice::TypeAlias::getTagFormat() const
{
    return _underlying->getTagFormat();
}

bool
Slice::TypeAlias::isVariableLength() const
{
    return _underlying->isVariableLength();
}

void
Slice::TypeAlias::visit(ParserVisitor* visitor, bool)
{
    visitor->visitTypeAlias(this);
}

Slice::TypeAlias::TypeAlias(const ContainerPtr& container, const string& name, const TypePtr& underlying,
                            const StringList& typeMetadata) :
    SyntaxTreeBase(container->unit()),
    Contained(container, name),
    Type(container->unit()),
    _underlying(underlying),
    _typeMetadata(typeMetadata)
{
    if (auto optional = OptionalPtr::dynamicCast(_underlying))
    {
        _unit->error("`" + name + "': optional types cannot be type-aliased");
        _underlying = optional->underlying();
    }
}

// ----------------------------------------------------------------------
// Sequence
// ----------------------------------------------------------------------

TypePtr
Slice::Sequence::type() const
{
    return _type;
}

StringList
Slice::Sequence::typeMetadata() const
{
    return _typeMetadata;
}

string
Slice::Sequence::typeId() const
{
    return scoped();
}

bool
Slice::Sequence::uses(const ContainedPtr& contained) const
{
    ContainedPtr contained2 = ContainedPtr::dynamicCast(_type);
    return (contained2 && contained2 == contained);
}

bool
Slice::Sequence::usesClasses() const
{
    return _type->usesClasses();
}

size_t
Slice::Sequence::minWireSize() const
{
    return 1; // An empty sequence.
}

string
Slice::Sequence::getTagFormat() const
{
    return _type->isVariableLength() ? "FSize" : "VSize";
}

bool
Slice::Sequence::isVariableLength() const
{
    return true;
}

string
Slice::Sequence::kindOf() const
{
    return "sequence";
}

void
Slice::Sequence::visit(ParserVisitor* visitor, bool)
{
    visitor->visitSequence(this);
}

Slice::Sequence::Sequence(const ContainerPtr& container, const string& name, const TypePtr& type,
                          const StringList& typeMetadata) :
    SyntaxTreeBase(container->unit()),
    Contained(container, name),
    Type(container->unit()),
    _type(type),
    _typeMetadata(typeMetadata)
{
}

// ----------------------------------------------------------------------
// Dictionary
// ----------------------------------------------------------------------

TypePtr
Slice::Dictionary::keyType() const
{
    return _keyType;
}

TypePtr
Slice::Dictionary::valueType() const
{
    return _valueType;
}

StringList
Slice::Dictionary::keyMetadata() const
{
    return _keyMetadata;
}

StringList
Slice::Dictionary::valueMetadata() const
{
    return _valueMetadata;
}

string
Slice::Dictionary::typeId() const
{
    return scoped();
}

bool
Slice::Dictionary::uses(const ContainedPtr& contained) const
{
    {
        ContainedPtr contained2 = ContainedPtr::dynamicCast(_keyType);
        if (contained2 && contained2 == contained)
        {
            return true;
        }
    }

    {
        ContainedPtr contained2 = ContainedPtr::dynamicCast(_valueType);
        if (contained2 && contained2 == contained)
        {
            return true;
        }
    }

    return false;
}

bool
Slice::Dictionary::usesClasses() const
{
    return _valueType->usesClasses();
}

size_t
Slice::Dictionary::minWireSize() const
{
    return 1; // An empty dictionary.
}

string
Slice::Dictionary::getTagFormat() const
{
    return _keyType->isVariableLength() || _valueType->isVariableLength() ? "FSize" : "VSize";
}

bool
Slice::Dictionary::isVariableLength() const
{
    return true;
}

string
Slice::Dictionary::kindOf() const
{
    return "dictionary";
}

void
Slice::Dictionary::visit(ParserVisitor* visitor, bool)
{
    visitor->visitDictionary(this);
}

//
// Check that the key type of a dictionary is legal. Legal types are
// integral types, string, and sequences and structs containing only
// other legal key types.
//
// Note: Allowing sequences in dictionary keys has been deprecated as
//       of Ice 3.3.0.
//
bool
Slice::Dictionary::legalKeyType(const TypePtr& type, bool& containsSequence)
{
    if (BuiltinPtr builtin = BuiltinPtr::dynamicCast(type))
    {
        switch (builtin->kind())
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
            case Builtin::KindString:
                return true;
            default:
                return false;
        }
    }

    if (EnumPtr::dynamicCast(type))
    {
        return true;
    }

    if (SequencePtr seq = SequencePtr::dynamicCast(type))
    {
        containsSequence = true;
        if (legalKeyType(seq->type(), containsSequence))
        {
            return true;
        }
    }

    if (StructPtr str = StructPtr::dynamicCast(type))
    {
        for (const auto& member : str->dataMembers())
        {
            if (!legalKeyType(member->type(), containsSequence))
            {
                return false;
            }
        }
        return true;
    }

    return false;
}

Slice::Dictionary::Dictionary(const ContainerPtr& container, const string& name, const TypePtr& keyType,
                              const StringList& keyMetadata, const TypePtr& valueType,
                              const StringList& valueMetadata) :
    SyntaxTreeBase(container->unit()),
    Contained(container, name),
    Type(container->unit()),
    _keyType(keyType),
    _valueType(valueType),
    _keyMetadata(keyMetadata),
    _valueMetadata(valueMetadata)
{
}

// ----------------------------------------------------------------------
// Enum
// ----------------------------------------------------------------------

void
Slice::Enum::destroy()
{
    _enumerators.clear();
    _underlying = nullptr;
    Container::destroy();
}

EnumeratorPtr
Slice::Enum::createEnumerator(const string& name)
{
    checkForRedefinition(this, name, "enumerator");
    checkIdentifier(name);
    EnumeratorPtr p = new Enumerator(this, name);
    _enumerators.push_back(p);
    return p;
}

EnumeratorPtr
Slice::Enum::createEnumerator(const string& name, int64_t value)
{
    checkForRedefinition(this, name, "enumerator");
    checkIdentifier(name);
    EnumeratorPtr p = new Enumerator(this, name, value);
    _enumerators.push_back(p);
    return p;
}

EnumeratorList
Slice::Enum::enumerators() const
{
    return _enumerators;
}

BuiltinPtr
Slice::Enum::underlying() const
{
    return _underlying;
}

bool
Slice::Enum::explicitValue() const
{
    return _explicitValue;
}

int64_t
Slice::Enum::minValue() const
{
    return _minValue;
}

int64_t
Slice::Enum::maxValue() const
{
    return _maxValue;
}

ContainedList
Slice::Enum::contents() const
{
    ContainedList result;
    result.insert(result.end(), _enumerators.begin(), _enumerators.end());
    return result;
}

string
Slice::Enum::typeId() const
{
    return scoped();
}

bool
Slice::Enum::uses(const ContainedPtr&) const
{
    return false;
}

bool
Slice::Enum::usesClasses() const
{
    return false;
}

size_t
Slice::Enum::minWireSize() const
{
    return _underlying ? _underlying->minWireSize() : 1;
}

string
Slice::Enum::getTagFormat() const
{
    return _underlying ? _underlying->getTagFormat() : "Size";
}

bool
Slice::Enum::isVariableLength() const
{
    return !_underlying;
}

string
Slice::Enum::kindOf() const
{
    return "enumeration";
}

void
Slice::Enum::visit(ParserVisitor* visitor, bool)
{
    visitor->visitEnum(this);
}

void
Slice::Enum::initUnderlying(const TypePtr& type)
{
    assert(_enumerators.empty());

    // initUnderlying is called with a null parameter when the Slice definition does not specify an underlying type.
    if (type)
    {
        BuiltinPtr underlying = BuiltinPtr::dynamicCast(type);
        if (!underlying || !underlying->isIntegralType() || underlying->isVariableLength() ||
            underlying->minWireSize() > 4)
        {
            _unit->error("an enum's underlying type must be byte, short, ushort, int, or uint");
        }
        else
        {
            _underlying = underlying;
        }
    }
}

Slice::Enum::Enum(const ContainerPtr& container, const string& name, bool unchecked) :
    SyntaxTreeBase(container->unit()),
    Container(container->unit()),
    Contained(container, name),
    Type(container->unit()),
    _unchecked(unchecked),
    _underlying(nullptr),
    _explicitValue(false),
    _minValue(INT64_MAX),
    _maxValue(INT64_MIN),
    _lastValue(-1)
{
}

int64_t
Slice::Enum::newEnumerator(const EnumeratorPtr& enumerator)
{
    int64_t rangeMin = 0;
    uint64_t rangeMax = INT32_MAX;
    if (_underlying)
    {
        tie(rangeMin, rangeMax) = _underlying->integralRange();
    }

    if (enumerator->explicitValue())
    {
        _explicitValue = true;
        _lastValue  = enumerator->value();
    }
    else
    {
        _lastValue++;
    }

    if (_lastValue < rangeMin || _lastValue > static_cast<int64_t>(rangeMax))
    {
        ostringstream oss;
        oss.imbue(underscoreSeparatorLocale);
        oss << "value " << _lastValue << " for enumerator `" << enumerator->name() << "'";

        oss << " is outside the range of " << (_underlying ? _underlying->kindAsString() : "its enum") << ": ["
            << rangeMin << ".." << rangeMax << "]";
        _unit->error(oss.str());
    }

    bool checkForDuplicates = true;
    if (_lastValue > _maxValue)
    {
        _maxValue = _lastValue;
        checkForDuplicates = false;
    }
    if (_lastValue < _minValue)
    {
        _minValue = _lastValue;
        checkForDuplicates = false;
    }

    if (checkForDuplicates)
    {
        for (const auto& en : _enumerators)
        {
            if (en != enumerator && en->value() == _lastValue)
            {
                _unit->error("enumerator `" + enumerator->name() + "' has the same value as enumerator `" +
                             en->name() + "'");
            }
        }
    }

    return _lastValue;
}

// ----------------------------------------------------------------------
// Enumerator
// ----------------------------------------------------------------------

EnumPtr
Slice::Enumerator::type() const
{
    return EnumPtr::dynamicCast(container());
}

bool
Slice::Enumerator::uses(const ContainedPtr&) const
{
    return false;
}

string
Slice::Enumerator::kindOf() const
{
    return "enumerator";
}

bool
Slice::Enumerator::explicitValue() const
{
    return _explicitValue;
}

int64_t
Slice::Enumerator::value() const
{
    return _value;
}

Slice::Enumerator::Enumerator(const EnumPtr& container, const string& name) :
    SyntaxTreeBase(container->unit()),
    Contained(container, name),
    _explicitValue(false),
    _value(-1)
{
    _value = container->newEnumerator(this);
}

Slice::Enumerator::Enumerator(const EnumPtr& container, const string& name, int64_t value) :
    SyntaxTreeBase(container->unit()),
    Contained(container, name),
    _explicitValue(true),
    _value(value)
{
    container->newEnumerator(this);
}

// ----------------------------------------------------------------------
// Const
// ----------------------------------------------------------------------

TypePtr
Slice::Const::type() const
{
    return _type;
}

StringList
Slice::Const::typeMetadata() const
{
    return _typeMetadata;
}

SyntaxTreeBasePtr
Slice::Const::valueType() const
{
    return _valueType;
}

string
Slice::Const::value() const
{
    return _value;
}

string
Slice::Const::literal() const
{
    return _literal;
}

bool
Slice::Const::uses(const ContainedPtr& contained) const
{
    ContainedPtr contained2 = ContainedPtr::dynamicCast(_type);
    return (contained2 && contained2 == contained);
}

string
Slice::Const::kindOf() const
{
    return "constant";
}

void
Slice::Const::visit(ParserVisitor* visitor, bool)
{
    visitor->visitConst(this);
}

Slice::Const::Const(const ContainerPtr& container, const string& name, const TypePtr& type,
                    const StringList& typeMetadata, const SyntaxTreeBasePtr& valueType, const string& value,
                    const string& literal) :
    SyntaxTreeBase(container->unit()),
    Contained(container, name),
    _type(type),
    _typeMetadata(typeMetadata),
    _valueType(valueType),
    _value(value),
    _literal(literal)
{
    if (!valueType)
    {
        cerr << "const " << name << " created with null valueType" << endl;
    }

}

// ----------------------------------------------------------------------
// Operation
// ----------------------------------------------------------------------

InterfaceDefPtr
Slice::Operation::interface() const
{
    return InterfaceDefPtr::dynamicCast(_container);
}

size_t
Slice::Operation::paramsBitSequenceSize() const
{
    return getBitSequenceSize(_params);
}

size_t
Slice::Operation::returnBitSequenceSize() const
{
    return getBitSequenceSize(_returnType);
}

void
Slice::Operation::destroy()
{
    _params.clear();
    _returnType.clear();
    _throws.clear();
    Container::destroy();
}

TypePtr
Slice::Operation::deprecatedReturnType() const
{
    if (_hasReturnType)
    {
        return _returnType.front()->_type;
    }
    return nullptr;
}

bool
Slice::Operation::returnIsTagged() const
{
    if (_hasReturnType)
    {
        return _returnType.front()->_tagged;
    }
    return false;
}

int
Slice::Operation::returnTag() const
{
    if (_hasReturnType)
    {
        return _returnType.front()->_tag;
    }
    return -1;
}

Operation::Mode
Slice::Operation::mode() const
{
    return _mode;
}

Operation::Mode
Slice::Operation::sendMode() const
{
    if (_mode == Operation::Idempotent && hasMetadata("nonmutating"))
    {
        return Operation::Nonmutating;
    }
    else
    {
        return _mode;
    }
}

bool
Slice::Operation::hasMarshaledResult() const
{
    InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(container());
    assert(interface);
    if (interface->hasMetadata("marshaled-result") || hasMetadata("marshaled-result"))
    {
        for (const auto& returnValue : _returnType)
        {
            if (isMutableAfterReturnType(returnValue->type()))
            {
                return true;
            }
        }
    }
    return false;
}

MemberPtr
Slice::Operation::createParameter(const string& name, const TypePtr& type, bool isOutParam, bool tagged, int tag,
                                  bool stream)
{
    _unit->checkType(type);

    if (isOutParam)
    {
        // If the operation returns multiple values, but none of those are out parameters, it must be a return-tuple,
        // in which case using out parameters isn't allowed, and an error is thrown.
        if (!_usesOutParams && _returnType.size() > 1)
        {
            _unit->error("an operation returning a tuple cannot have out parameters");
        }

        _usesOutParams = true;
        // Check if the out parameter conflicts with a single return value (which is implicitely named 'returnValue').
        if (hasSingleReturnType() && ciequals(name, "returnValue"))
        {
            _unit->error("the identifier `" + name + "' cannot be used for out parameters of non-void operations");
            return nullptr;
        }
    }
    else if (_usesOutParams)
    {
        // In parameters must be declared before out parameters.
        _unit->error("`" + name + "': in parameters cannot follow out parameters");
    }

    if (!checkForRedefinition(this, name, "parameter"))
    {
        return nullptr;
    }

    MemberList& params = isOutParam ? _returnType : _params;

    if (!params.empty() && params.back()->stream())
    {
        _unit->error(stream ? "operations can have a single stream parameter as the last parameter" :
                              ("stream parameter `" + params.back()->name() + "' must be the last parameter"));
    }

    if (tagged && tag > -1)
    {
        // Check for a duplicate tag.
        for (const auto& param : params)
        {
            if (param->tagged() && param->tag() == tag)
            {
                _unit->error("tag for parameter `" + name + "' is already in use");
            }
        }
    }

    MemberPtr param = new Member(this, name, type, tagged, tag, stream);
    params.push_back(param);
    return param;
}

MemberPtr
Slice::Operation::createReturnMember(const std::string& name, const TypePtr& type, bool tagged, int tag, bool stream)
{
    _hasReturnType = true;

    _unit->checkType(type);
    if (tagged && tag > -1)
    {
        // Check for a duplicate tag.
        for (const auto& param : _returnType)
        {
            if (param->tagged() && param->tag() == tag)
            {
                _unit->error("tag for return value `" + name + "' is already in use");
            }
        }
    }

    if (!_returnType.empty() && _returnType.back()->stream())
    {
        _unit->error("stream return value `" + _returnType.back()->name() + "' must be the last return value");
    }

    if (!checkForRedefinition(this, name, "parameter"))
    {
        return nullptr;
    }

    MemberPtr returnMember = new Member(this, name, type, tagged, tag, stream);
    _returnType.push_back(returnMember);
    return returnMember;
}

MemberList
Slice::Operation::params() const
{
    return _params;
}

MemberList
Slice::Operation::outParameters() const
{
    if (!_usesOutParams)
    {
        return MemberList();
    }

    MemberList outParameters = _returnType;
    // Check if the operation had a return type and if so, remove it, so we're left with only out parameters.
    if (_hasReturnType)
    {
        outParameters.pop_front();
    }
    return outParameters;
}

MemberList
Slice::Operation::returnType() const
{
    return _returnType;
}

MemberList
Slice::Operation::allMembers() const
{
    MemberList result;
    result.insert(result.end(), _params.begin(), _params.end());
    if (_usesOutParams)
    {
        // Skip the return type if it's present, so only out parameters are added to result.
        auto start = _hasReturnType ? next(_returnType.begin()) : _returnType.begin();
        result.insert(result.end(), start, _returnType.end());
    }
    return result;
}

ExceptionList
Slice::Operation::throws() const
{
    return _throws;
}

void
Slice::Operation::setExceptionList(const ExceptionList& el)
{
    _throws = el;

    // Check that no exception occurs more than once in the throws clause.
    ExceptionList uniqueExceptions = el;
    uniqueExceptions.sort();
    uniqueExceptions.unique();
    if (uniqueExceptions.size() != el.size())
    {
        // At least one exception appears twice.
        ExceptionList tmp = el;
        tmp.sort();
        ExceptionList duplicates;
        set_difference(tmp.begin(), tmp.end(),
                       uniqueExceptions.begin(), uniqueExceptions.end(),
                       back_inserter(duplicates));
        string msg = "operation `" + name() + "' has a throws clause with ";
        if (duplicates.size() == 1)
        {
            msg += "a ";
        }
        msg += "duplicate exception";
        if (duplicates.size() > 1)
        {
            msg += "s";
        }
        ExceptionList::const_iterator i = duplicates.begin();
        msg += ": `" + (*i)->name() + "'";
        for (i = ++i; i != duplicates.end(); ++i)
        {
            msg += ", `" + (*i)->name() + "'";
        }
        _unit->error(msg);
    }
}

ContainedList
Slice::Operation::contents() const
{
    ContainedList result;
    result.insert(result.end(), _params.begin(), _params.end());
    if (_usesOutParams)
    {
        // Skip the return type if it's present, so only out parameters are added to result.
        auto start = _hasReturnType ? next(_returnType.begin()) : _returnType.begin();
        result.insert(result.end(), start, _returnType.end());
    }
    return result;

}

bool
Slice::Operation::uses(const ContainedPtr& contained) const
{
    if (_hasReturnType)
    {
        ContainedPtr contained2 = ContainedPtr::dynamicCast(_returnType.front()->_type);
        if (contained2 && contained2 == contained)
        {
            return true;
        }
    }

    if (ExceptionPtr exception2 = ExceptionPtr::dynamicCast(contained))
    {
        for (const auto& exception : _throws)
        {
            if (exception == exception2)
            {
                return true;
            }
        }
    }

    return false;
}

bool
Slice::Operation::sendsClasses(bool includeTagged) const
{
    for (const auto& param : _params)
    {
        if (param->type()->usesClasses() && (includeTagged || !param->tagged()))
        {
            return true;
        }
    }
    return false;
}

bool
Slice::Operation::returnsClasses(bool includeTagged) const
{
    for (const auto& returnValue : _returnType)
    {
        if (returnValue->type()->usesClasses() && (includeTagged || !returnValue->tagged()))
        {
            return true;
        }
    }
    return false;
}

bool
Slice::Operation::returnsData() const
{
    return !_returnType.empty() || !_throws.empty();
}

bool
Slice::Operation::returnsMultipleValues() const
{
    return _returnType.size() > 1;
}

bool
Slice::Operation::hasReturnAndOut() const
{
    return _hasReturnType && _usesOutParams;
}

bool
Slice::Operation::hasSingleReturnType() const
{
    return _hasReturnType && (_usesOutParams || _returnType.size() == 1);
}

FormatType
Slice::Operation::format() const
{
    FormatType format = parseFormatMetadata();
    if (format == DefaultFormat)
    {
        ContainedPtr cont = ContainedPtr::dynamicCast(container());
        assert(cont);
        format = cont->parseFormatMetadata();
    }
    return format;
}

string
Slice::Operation::kindOf() const
{
    return "operation";
}

void
Slice::Operation::visit(ParserVisitor* visitor, bool)
{
    visitor->visitOperation(this);
}

Slice::Operation::Operation(const ContainerPtr& container,
                            const string& name,
                            Mode mode) :
    SyntaxTreeBase(container->unit()),
    Contained(container, name),
    Container(container->unit()),
    _usesOutParams(false),
    _hasReturnType(false),
    _mode(mode)
{
}

// ----------------------------------------------------------------------
// Member
// ----------------------------------------------------------------------

TypePtr
Slice::Member::type() const
{
    return _type;
}

bool
Slice::Member::tagged() const
{
    return _tagged;
}

int
Slice::Member::tag() const
{
    return _tag;
}

bool
Slice::Member::stream() const
{
    return _stream;
}

string
Slice::Member::defaultValue() const
{
    return _defaultValue;
}

string
Slice::Member::defaultLiteral() const
{
    return _defaultLiteral;
}

SyntaxTreeBasePtr
Slice::Member::defaultValueType() const
{
    return _defaultValueType;
}

bool
Slice::Member::uses(const ContainedPtr& contained) const
{
    ContainedPtr contained2 = ContainedPtr::dynamicCast(_type);
    return (contained2 && contained2 == contained);
}

string
Slice::Member::kindOf() const
{
    if (OperationPtr::dynamicCast(this->container()))
    {
        return "parameter";
    }
    return "data member";
}

void
Slice::Member::visit(ParserVisitor* visitor, bool)
{
    assert(!OperationPtr::dynamicCast(this->container())); // Ensure this isn't being called for a parameter.
    visitor->visitDataMember(this);
}

OperationPtr
Slice::Member::operation() const
{
    return OperationPtr::dynamicCast(container());
}

Slice::Member::Member(const ContainerPtr& container, const string& name, const TypePtr& type,
                      bool tagged, int tag, bool stream, const SyntaxTreeBasePtr& defaultValueType,
                      const string& defaultValue, const string& defaultLiteral) :
    SyntaxTreeBase(container->unit()),
    Contained(container, name),
    _type(type),
    _tagged(tagged),
    _tag(tag),
    _stream(stream),
    _defaultValueType(defaultValueType),
    _defaultValue(defaultValue),
    _defaultLiteral(defaultLiteral)
{
}

// ----------------------------------------------------------------------
// Unit
// ----------------------------------------------------------------------

UnitPtr
Slice::Unit::createUnit(bool all, const StringList& defaultFileMetadata)
{
    return new Unit(all, defaultFileMetadata);
}

ModulePtr
Slice::Unit::createModule(const string& name)
{
    _unit->addTopLevelModule(_unit->currentFile(), name);

    // We use binary or here to avoid short-circuiting and ensure both checks are always run.
    if (!checkForRedefinition(this, name, "module") | !checkIdentifier(name))
    {
        return nullptr;
    }

    ModulePtr q = new Module(this, name);
    _modules.push_back(q);
    return q;
}

bool
Slice::Unit::compatMode() const
{
    return currentDefinitionContext()->compatMode();
}

void
Slice::Unit::checkType(const TypePtr& type)
{
    if (compatMode() && InterfaceDeclPtr::dynamicCast(type))
    {
        error("interface by value is no longer supported: remove [[3.7]] or specify an optional proxy");
    }
}

void
Slice::Unit::setComment(const string& comment)
{
    if (comment.empty() || comment[0] != '*')
    {
        return;
    }

    _currentComment = "";

    string::size_type end = 0;
    while (true)
    {
        string::size_type begin;
        if (end == 0)
        {
            // Skip past the initial whitespace.
            begin = comment.find_first_not_of(" \t\r\n*", end);
        }
        else
        {
            // Skip more whitespace but retain blank lines.
            begin = comment.find_first_not_of(" \t*", end);
        }

        if (begin == string::npos)
        {
            break;
        }

        end = comment.find('\n', begin);
        if (end != string::npos)
        {
            if (end + 1 > begin)
            {
                _currentComment += comment.substr(begin, end + 1 - begin);
            }
            ++end;
        }
        else
        {
            end = comment.find_last_not_of(" \t\r\n*");
            if (end != string::npos)
            {
                if (end + 1 > begin)
                {
                    _currentComment += comment.substr(begin, end + 1 - begin);
                }
            }
            break;
        }
    }
}

void
Slice::Unit::addToComment(const string& comment)
{
    if (!_currentComment.empty())
    {
        _currentComment += '\n';
    }
    _currentComment += comment;
}

string
Slice::Unit::currentComment()
{
    string comment = "";
    comment.swap(_currentComment);
    return comment;
}

string
Slice::Unit::currentFile() const
{
    if (DefinitionContextPtr dc = currentDefinitionContext())
    {
        return dc->filename();
    }
    else
    {
        return string();
    }
}

string
Slice::Unit::topLevelFile() const
{
    return _topLevelFile;
}

int
Slice::Unit::currentLine() const
{
    return slice_lineno;
}

int
Slice::Unit::setCurrentFile(const std::string& currentFile, int lineNumber)
{
    enum LineType { File, Push, Pop };
    LineType type = File;

    if (lineNumber == 0 && (_currentIncludeLevel > 0 || currentFile != _topLevelFile))
    {
        if (++_currentIncludeLevel == 1)
        {
            if (find(_includeFiles.begin(), _includeFiles.end(), currentFile) == _includeFiles.end())
            {
                _includeFiles.push_back(currentFile);
            }
        }
        pushDefinitionContext();
        _currentComment = "";
        type = Push;
    }
    else
    {
        DefinitionContextPtr dc = currentDefinitionContext();
        if (dc != 0 && !dc->filename().empty() && dc->filename() != currentFile)
        {
            --_currentIncludeLevel;
            popDefinitionContext();
            _currentComment = "";
            type = Pop;
        }
    }

    if (!currentFile.empty())
    {
        DefinitionContextPtr dc = currentDefinitionContext();
        assert(dc);
        dc->setFilename(currentFile);
        _definitionContextMap.insert(make_pair(currentFile, dc));
    }

    return static_cast<int>(type);
}

int
Slice::Unit::currentIncludeLevel() const
{
    return _all ? 0 : _currentIncludeLevel;
}

void
Slice::Unit::addFileMetadata(const StringList& metadata)
{
    DefinitionContextPtr dc = currentDefinitionContext();
    assert(dc);
    // Append the file metadata to any existing metadata (e.g., default file metadata).
    StringList l = dc->getAllMetadata();
    copy(metadata.begin(), metadata.end(), back_inserter(l));
    dc->setMetadata(l);
}

void
Slice::Unit::error(const string& s)
{
    emitError(currentFile(), currentLine(), s);
    _errors++;
}

void
Slice::Unit::warning(WarningCategory category, const string& msg) const
{
    if (_definitionContextStack.empty())
    {
        emitWarning(currentFile(), currentLine(), msg);
    }
    else
    {
        _definitionContextStack.top()->warning(category, currentFile(), currentLine(), msg);
    }
}

void
Slice::Unit::note(ContainedPtr contained, const string& msg) const
{
    emitNote(contained->file(), contained->line(), msg);
}

ContainerPtr
Slice::Unit::currentContainer() const
{
    assert(!_containerStack.empty());
    return _containerStack.top();
}

ModulePtr
Slice::Unit::currentModule() const
{
    ContainerPtr container = currentContainer();
    while (container)
    {
        if (UnitPtr ut = UnitPtr::dynamicCast(container))
        {
            return ut->_globalModule;
        }
        else if (ModulePtr module = ModulePtr::dynamicCast(container))
        {
            return module;
        }
        ContainedPtr contained = ContainedPtr::dynamicCast(container);
        container = contained->container();
    }
    assert(false);
    return nullptr;
}

void
Slice::Unit::pushContainer(const ContainerPtr& cont)
{
    _containerStack.push(cont);
}

void
Slice::Unit::popContainer()
{
    assert(!_containerStack.empty());
    _containerStack.pop();
}

DefinitionContextPtr
Slice::Unit::currentDefinitionContext() const
{
    DefinitionContextPtr dc;
    if (!_definitionContextStack.empty())
    {
        dc = _definitionContextStack.top();
    }
    return dc;
}

void
Slice::Unit::pushDefinitionContext()
{
    _definitionContextStack.push(new DefinitionContext(_currentIncludeLevel, _defaultFileMetadata));
}

void
Slice::Unit::popDefinitionContext()
{
    assert(!_definitionContextStack.empty());
    _definitionContextStack.pop();
}

DefinitionContextPtr
Slice::Unit::findDefinitionContext(const string& file) const
{
    const auto def = _definitionContextMap.find(file);
    if (def != _definitionContextMap.end())
    {
        return def->second;
    }
    return nullptr;
}

void
Slice::Unit::addContent(const ContainedPtr& contained)
{
    string scoped = IceUtilInternal::toLower(contained->scoped());
    _contentMap[scoped].push_back(contained);
}

void
Slice::Unit::removeContent(const ContainedPtr& contained)
{
    string scoped = IceUtilInternal::toLower(contained->scoped());
    auto def = _contentMap.find(scoped);
    assert(def != _contentMap.end());
    for (ContainedList::iterator q = def->second.begin(); q != def->second.end(); ++q)
    {
        if (q->get() == contained.get())
        {
            def->second.erase(q);
            return;
        }
    }
    assert(false);
}

ContainedList
Slice::Unit::findContents(const string& scoped) const
{
    assert(!scoped.empty());
    assert(scoped[0] == ':');

    string name = IceUtilInternal::toLower(scoped);
    const auto content = _contentMap.find(name);
    if (content != _contentMap.end())
    {
        return content->second;
    }
    else
    {
        return ContainedList();
    }
}

void
Slice::Unit::addTypeId(int compactId, const std::string& typeId)
{
    _typeIds.insert(make_pair(compactId, typeId));
}

std::string
Slice::Unit::getTypeId(int compactId) const
{
    const auto p = _typeIds.find(compactId);
    if (p != _typeIds.end())
    {
        return p->second;
    }
    return string();
}

bool
Slice::Unit::hasCompactTypeId() const
{
    return _typeIds.size() > 0;
}

StringList
Slice::Unit::includeFiles() const
{
    return _includeFiles;
}

StringList
Slice::Unit::allFiles() const
{
    StringList result;
    for (const auto& def : _definitionContextMap)
    {
        result.push_back(def.first);
    }
    return result;
}

int
Slice::Unit::parse(const string& filename, FILE* file, bool debug)
{
    slice_debug = debug ? 1 : 0;
    slice__flex_debug = debug ? 1 : 0;

    assert(!Slice::unit);
    Slice::unit = this;

    _currentComment = "";
    _currentIncludeLevel = 0;
    _topLevelFile = fullPath(filename);
    pushContainer(this);
    pushDefinitionContext();
    setCurrentFile(_topLevelFile, 0);

    slice_in = file;
    int status = slice_parse();
    if (_errors)
    {
        status = EXIT_FAILURE;
    }

    if (status == EXIT_FAILURE)
    {
        while (!_containerStack.empty())
        {
            popContainer();
        }
        while (!_definitionContextStack.empty())
        {
            popDefinitionContext();
        }
    }
    else
    {
        assert(_containerStack.size() == 1);
        popContainer();
        assert(_definitionContextStack.size() == 1);
        popDefinitionContext();
    }

    Slice::unit = nullptr;
    return status;
}

void
Slice::Unit::destroy()
{
    _contentMap.clear();
    _globalModule = nullptr;
    _modules.clear();
    _builtins.clear();
    Container::destroy();
}

void
Slice::Unit::visit(ParserVisitor* visitor, bool all)
{
    if (visitor->visitUnitStart(this))
    {
        for (const auto& module : _modules)
        {
            if (all || module->includeLevel() == 0)
            {
                module->visit(visitor, all);
            }
        }
        visitor->visitUnitEnd(this);
    }
}

ContainedList
Slice::Unit::contents() const
{
    ContainedList result;
    result.insert(result.end(), _modules.begin(), _modules.end());
    return result;
}

bool
Slice::Unit::hasExceptions() const
{
    for (const auto& module : _modules)
    {
        if (module->hasExceptions())
        {
            return true;
        }
    }
    return _globalModule->hasExceptions();
}

bool
Slice::Unit::hasClassDecls() const
{
    for (const auto& module : _modules)
    {
        if (module->hasClassDecls())
        {
            return true;
        }
    }
    return _globalModule->hasClassDecls();
}

bool
Slice::Unit::hasClassDefs() const
{
    for (const auto& module : _modules)
    {
        if (module->hasClassDefs())
        {
            return true;
        }
    }
    return _globalModule->hasClassDefs();
}

bool
Slice::Unit::hasInterfaceDecls() const
{
    for (const auto& module : _modules)
    {
        if (module->hasInterfaceDecls())
        {
            return true;
        }
    }
    return _globalModule->hasInterfaceDecls();
}

bool
Slice::Unit::hasInterfaceDefs() const
{
    for (const auto& module : _modules)
    {
        if (module->hasInterfaceDefs())
        {
            return true;
        }
    }
    return _globalModule->hasInterfaceDefs();
}

BuiltinPtr
Slice::Unit::builtin(Builtin::Kind kind)
{
    auto p = _builtins.find(kind);
    if (p != _builtins.end())
    {
        return p->second;
    }
    BuiltinPtr builtin = new Builtin(this, kind);
    _builtins.insert(make_pair(kind, builtin));
    return builtin;
}

OptionalPtr
Slice::Unit::optionalBuiltin(Builtin::Kind kind)
{
    auto p = _optionalBuiltins.find(kind);
    if (p != _optionalBuiltins.end())
    {
        return p->second;
    }
    OptionalPtr optionalBuiltin = new Optional(builtin(kind));
    _optionalBuiltins.insert(make_pair(kind, optionalBuiltin));
    return optionalBuiltin;
}

void
Slice::Unit::addTopLevelModule(const string& file, const string& module)
{
    auto i = _fileTopLevelModules.find(file);
    if (i == _fileTopLevelModules.end())
    {
        set<string> modules;
        modules.insert(module);
        _fileTopLevelModules.insert(make_pair(file, modules));
    }
    else
    {
        i->second.insert(module);
    }
}

set<string>
Slice::Unit::getTopLevelModules(const string& file) const
{
    const auto i = _fileTopLevelModules.find(file);
    if (i == _fileTopLevelModules.end())
    {
        return set<string>();
    }
    else
    {
        return i->second;
    }
}

Slice::Unit::Unit(bool all, const StringList& defaultFileMetadata) :
    SyntaxTreeBase(nullptr),
    Container(nullptr),
    _all(all),
    _defaultFileMetadata(defaultFileMetadata),
    _errors(0),
    _currentIncludeLevel(0)
{
    _unit = this;
    _globalModule = new Module(this, "");
}

// ----------------------------------------------------------------------
// CICompare
// ----------------------------------------------------------------------

bool
Slice::CICompare::operator()(const string& s1, const string& s2) const
{
    string::const_iterator p1 = s1.begin();
    string::const_iterator p2 = s2.begin();
    while (p1 != s1.end() && p2 != s2.end() &&
          ::tolower(static_cast<unsigned char>(*p1)) == ::tolower(static_cast<unsigned char>(*p2)))
    {
        ++p1;
        ++p2;
    }
    if (p1 == s1.end() && p2 == s2.end())
    {
        return false;
    }
    else if (p1 == s1.end())
    {
        return true;
    }
    else if (p2 == s2.end())
    {
        return false;
    }
    else
    {
        return ::tolower(static_cast<unsigned char>(*p1)) < ::tolower(static_cast<unsigned char>(*p2));
    }
}

// ----------------------------------------------------------------------
// DerivedToBaseCompare
// ----------------------------------------------------------------------

bool
Slice::DerivedToBaseCompare::operator()(const ExceptionPtr& e1, const ExceptionPtr& e2) const
{
    return e2->isBaseOf(e1);
}
