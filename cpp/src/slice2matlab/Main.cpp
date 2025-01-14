//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "../Ice/ConsoleUtil.h"
#include "../Ice/FileUtil.h"
#include "../Ice/Options.h"
#include "../Ice/OutputUtil.h"
#include "../Slice/FileTracker.h"
#include "../Slice/MetadataValidation.h"
#include "../Slice/Parser.h"
#include "../Slice/Preprocessor.h"
#include "../Slice/Util.h"
#include "Ice/CtrlCHandler.h"
#include "Ice/StringUtil.h"

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstring>
#include <iterator>
#include <mutex>

#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#    include <direct.h>
#else
#    include <unistd.h>
#endif

using namespace std;
using namespace Slice;
using namespace IceInternal;

namespace
{
    //
    // Split an absolute name into its components and return the components as a list of identifiers.
    //
    vector<string> splitAbsoluteName(const string& abs)
    {
        vector<string> ids;
        string::size_type start = 0;
        string::size_type pos;
        while ((pos = abs.find(".", start)) != string::npos)
        {
            assert(pos > start);
            ids.push_back(abs.substr(start, pos - start));
            start = pos + 1;
        }
        if (start != abs.size())
        {
            ids.push_back(abs.substr(start));
        }

        return ids;
    }

    string lookupKwd(const string& name)
    {
        //
        // Keyword list. *Must* be kept in alphabetical order.
        //
        static const string keywordList[] = {
            "break",       "case",       "catch",      "classdef", "continue", "else",   "elseif",  "end",
            "enumeration", "events",     "for",        "function", "global",   "if",     "methods", "otherwise",
            "parfor",      "persistent", "properties", "return",   "spmd",     "switch", "try",     "while"};
        bool found = binary_search(&keywordList[0], &keywordList[sizeof(keywordList) / sizeof(*keywordList)], name);
        return found ? name + "_" : name;
    }

    string fixIdent(const string& ident)
    {
        if (ident[0] != ':')
        {
            return lookupKwd(ident);
        }
        vector<string> ids = splitScopedName(ident);
        transform(ids.begin(), ids.end(), ids.begin(), [](const string& id) -> string { return lookupKwd(id); });
        stringstream result;
        for (const auto& id : ids)
        {
            result << "::" + id;
        }
        return result.str();
    }

    string fixEnumerator(const string& name)
    {
        assert(name[0] != ':');

        //
        // Method list. These represent the built-in methods for enumerators, inherited from uint8 or int32.
        // MATLAB does not allow an enumeration class to declare an enumerator having one of these names.
        //
        // *Must* be kept in alphabetical order.
        //
        static const string methodList[] = {
            "abs",
            "accumarray",
            "all",
            "and",
            "any",
            "bitand",
            "bitcmp",
            "bitget",
            "bitor",
            "bitset",
            "bitshift",
            "bitxor",
            "bsxfun",
            "cat",
            "ceil",
            "cell",
            "cellstr",
            "char",
            "complex",
            "conj",
            "conv2",
            "ctranspose",
            "cummax",
            "cummin",
            "cumprod",
            "cumsum",
            "diag",
            "diff",
            "double",
            "end",
            "eq",
            "fft",
            "fftn",
            "find",
            "fix",
            "floor",
            "full",
            "function_handle",
            "ge",
            "gt",
            "horzcat",
            "ifft",
            "ifftn",
            "imag",
            "int16",
            "int32",
            "int64",
            "int8",
            "intersect",
            "isempty",
            "isequal",
            "isequaln",
            "isequalwithequalnans",
            "isfinite",
            "isfloat",
            "isinf",
            "isinteger",
            "islogical",
            "ismember",
            "isnan",
            "isnumeric",
            "isreal",
            "isscalar",
            "issorted",
            "issparse",
            "isvector",
            "ldivide",
            "le",
            "length",
            "linsolve",
            "logical",
            "lt",
            "max",
            "min",
            "minus",
            "mldivide",
            "mod",
            "mpower",
            "mrdivide",
            "mtimes",
            "ndims",
            "ne",
            "nnz",
            "nonzeros",
            "not",
            "numel",
            "nzmax",
            "or",
            "permute",
            "plot",
            "plus",
            "power",
            "prod",
            "rdivide",
            "real",
            "rem",
            "reshape",
            "round",
            "setdiff",
            "setxor",
            "sign",
            "single",
            "size",
            "sort",
            "sortrowsc",
            "strcmp",
            "strcmpi",
            "strncmp",
            "strncmpi",
            "subsasgn",
            "subsindex",
            "subsref",
            "sum",
            "times",
            "transpose",
            "tril",
            "triu",
            "uint16",
            "uint32",
            "uint64",
            "uminus",
            "union",
            "uplus",
            "vertcat",
            "xor"};

        bool found = binary_search(&methodList[0], &methodList[sizeof(methodList) / sizeof(*methodList)], name);
        return found ? name + "_" : lookupKwd(name);
    }

    string fixOp(const string& name)
    {
        assert(name[0] != ':');

        //
        // An operation name must be escaped if it matches any of the identifiers in this list, in addition to the
        // MATLAB language keywords. The identifiers below represent the names of methods inherited from ObjectPrx
        // and handle.
        //
        // *Must* be kept in alphabetical order.
        //
        static const string idList[] = {
            "addlistener",
            "checkedCast",
            "delete",
            "eq",
            "findobj",
            "findprop",
            "ge",
            "gt",
            "isvalid",
            "le",
            "lt",
            "ne",
            "notify",
            "uncheckedCast"};
        bool found = binary_search(&idList[0], &idList[sizeof(idList) / sizeof(*idList)], name);
        return found ? name + "_" : fixIdent(name);
    }

    string fixExceptionMember(const string& ident)
    {
        //
        // User exceptions are subclasses of MATLAB's MException class. Subclasses cannot redefine a member that
        // conflicts with MException's properties. Unfortunately MException also has some undocumented non-public
        // properties that will cause run-time errors.
        //
        if (ident == "identifier" || ident == "message" || ident == "stack" || ident == "cause" ||
            ident == "type") // Undocumented
        {
            return ident + "_";
        }

        return fixIdent(ident);
    }

    string fixStructMember(const string& ident)
    {
        //
        // We define eq() and ne() methods for structures.
        //
        if (ident == "eq" || ident == "ne")
        {
            return ident + "_";
        }

        return fixIdent(ident);
    }

    string replace(string s, string patt, string val)
    {
        string r = s;
        string::size_type pos = r.find(patt);
        while (pos != string::npos)
        {
            r.replace(pos, patt.size(), val);
            pos += val.size();
            pos = r.find(patt, pos);
        }
        return r;
    }

    void writeCopyright(IceInternal::Output& out, const string& file)
    {
        string f = file;
        string::size_type pos = f.find_last_of("/");
        if (pos != string::npos)
        {
            f = f.substr(pos + 1);
        }

        out << nl << "% Copyright (c) ZeroC, Inc. All rights reserved.";
        out << nl << "% Generated from " << f << " by slice2matlab version " << ICE_STRING_VERSION;
        out << nl;
    }

    void openClass(const string& abs, const string& dir, IceInternal::Output& out)
    {
        vector<string> v = splitAbsoluteName(abs);
        assert(v.size() > 1);

        string path;
        if (!dir.empty())
        {
            path = dir + "/";
        }

        //
        // Create a package directory corresponding to each component.
        //
        for (vector<string>::size_type i = 0; i < v.size() - 1; i++)
        {
            path += "+" + lookupKwd(v[i]);
            if (!IceInternal::directoryExists(path))
            {
                int err = IceInternal::mkdir(path, 0777);
                // If slice2matlab is run concurrently, it's possible that another instance of slice2matlab has already
                // created the directory.
                if (err == 0 || (errno == EEXIST && IceInternal::directoryExists(path)))
                {
                    // Directory successfully created or already exists.
                }
                else
                {
                    ostringstream os;
                    os << "cannot create directory '" << path << "': " << IceInternal::errorToString(errno);
                    throw FileException(__FILE__, __LINE__, os.str());
                }
                FileTracker::instance()->addDirectory(path);
            }
            path += "/";
        }

        //
        // There are two options:
        //
        // 1) Create a subdirectory named "@ClassName" containing a file "ClassName.m".
        // 2) Create a file named "ClassName.m".
        //
        // The class directory is useful if you want to add additional supporting files for the class. We only
        // generate a single file for a class so we use option 2.
        //
        const string cls = lookupKwd(v[v.size() - 1]);
        path += cls + ".m";

        out.open(path);
        FileTracker::instance()->addFile(path);
    }

    //
    // Get the fully-qualified name of the given definition. If a suffix is provided,
    // it is prepended to the definition's unqualified name. If the nameSuffix
    // is provided, it is appended to the container's name.
    //
    string
    getAbsolute(const ContainedPtr& cont, const string& pfx = std::string(), const string& suffix = std::string())
    {
        string str = fixIdent(cont->scope() + pfx + cont->name() + suffix);
        if (str.find("::") == 0)
        {
            str.erase(0, 2);
        }

        return replace(str, "::", ".");
    }

    string typeToString(const TypePtr& type)
    {
        static const char* builtinTable[] = {
            "uint8",
            "logical",
            "int16",
            "int32",
            "int64",
            "single",
            "double",
            "char",
            "Ice.Object",    // Object
            "Ice.ObjectPrx", // ObjectPrx
            "Ice.Value"      // Value
        };

        if (!type)
        {
            return "void";
        }

        BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
        if (builtin)
        {
            return builtinTable[builtin->kind()];
        }

        ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
        if (cl)
        {
            return getAbsolute(cl);
        }

        InterfaceDeclPtr proxy = dynamic_pointer_cast<InterfaceDecl>(type);
        if (proxy)
        {
            return getAbsolute(proxy, "", "Prx");
        }

        DictionaryPtr dict = dynamic_pointer_cast<Dictionary>(type);
        if (dict)
        {
            if (dynamic_pointer_cast<Struct>(dict->keyType()))
            {
                return "struct";
            }
            else
            {
                return "containers.Map";
            }
        }

        ContainedPtr contained = dynamic_pointer_cast<Contained>(type);
        if (contained)
        {
            return getAbsolute(contained);
        }

        return "???";
    }

    string dictionaryTypeToString(const TypePtr& type, bool key)
    {
        assert(type);

        BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
        if (builtin)
        {
            switch (builtin->kind())
            {
                case Builtin::KindBool:
                case Builtin::KindByte:
                case Builtin::KindShort:
                {
                    //
                    // containers.Map supports a limited number of key types.
                    //
                    return key ? "int32" : typeToString(type);
                }
                case Builtin::KindInt:
                case Builtin::KindLong:
                case Builtin::KindString:
                {
                    return typeToString(type);
                }
                case Builtin::KindFloat:
                case Builtin::KindDouble:
                {
                    assert(!key);
                    return typeToString(type);
                }
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindValue:
                {
                    assert(!key);
                    return "any";
                }
                default:
                {
                    return "???";
                }
            }
        }

        EnumPtr en = dynamic_pointer_cast<Enum>(type);
        if (en)
        {
            //
            // containers.Map doesn't natively support enumerators as keys but we can work around it using int32.
            //
            return key ? "int32" : "any";
        }

        return "any";
    }

    bool declarePropertyType(const TypePtr& type, bool optional)
    {
        if (optional || dynamic_pointer_cast<Sequence>(type) || dynamic_pointer_cast<InterfaceDecl>(type) ||
            dynamic_pointer_cast<ClassDecl>(type))
        {
            return false;
        }

        BuiltinPtr b = dynamic_pointer_cast<Builtin>(type);
        if (b && (b->kind() == Builtin::KindObject || b->kind() == Builtin::KindObjectProxy ||
                  b->kind() == Builtin::KindValue))
        {
            return false;
        }

        return true;
    }

    string constantValue(const TypePtr& type, const SyntaxTreeBasePtr& valueType, const string& value)
    {
        ConstPtr constant = dynamic_pointer_cast<Const>(valueType);
        if (constant)
        {
            return getAbsolute(constant) + ".value";
        }
        else
        {
            BuiltinPtr bp;
            if ((bp = dynamic_pointer_cast<Builtin>(type)))
            {
                switch (bp->kind())
                {
                    case Builtin::KindString:
                    {
                        return "sprintf('" + toStringLiteral(value, "\a\b\f\n\r\t\v", "", Matlab, 255) + "')";
                    }
                    case Builtin::KindBool:
                    case Builtin::KindByte:
                    case Builtin::KindShort:
                    case Builtin::KindInt:
                    case Builtin::KindLong:
                    case Builtin::KindFloat:
                    case Builtin::KindDouble:
                    case Builtin::KindObject:
                    case Builtin::KindObjectProxy:
                    case Builtin::KindValue:
                    {
                        return value;
                    }

                    default:
                    {
                        return "???";
                    }
                }
            }
            else if (dynamic_pointer_cast<Enum>(type))
            {
                EnumeratorPtr e = dynamic_pointer_cast<Enumerator>(valueType);
                assert(e);
                return getAbsolute(e);
            }
            else
            {
                return value;
            }
        }
    }

    string defaultValue(const DataMemberPtr& m)
    {
        if (m->defaultValueType())
        {
            return constantValue(m->type(), m->defaultValueType(), m->defaultValue());
        }
        else if (m->optional())
        {
            return isProxyType(m->type()) ? "[]" : "IceInternal.UnsetI.Instance";
        }
        else
        {
            BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(m->type());
            if (builtin)
            {
                switch (builtin->kind())
                {
                    case Builtin::KindString:
                        return "''";
                    case Builtin::KindBool:
                        return "false";
                    case Builtin::KindByte:
                    case Builtin::KindShort:
                    case Builtin::KindInt:
                    case Builtin::KindLong:
                    case Builtin::KindFloat:
                    case Builtin::KindDouble:
                        return "0";
                    case Builtin::KindObject:
                    case Builtin::KindObjectProxy:
                    case Builtin::KindValue:
                        return "[]";
                }
            }

            DictionaryPtr dict = dynamic_pointer_cast<Dictionary>(m->type());
            if (dict)
            {
                const TypePtr key = dict->keyType();
                const TypePtr value = dict->valueType();
                if (dynamic_pointer_cast<Struct>(key))
                {
                    //
                    // We use a struct array when the key is a structure type because we can't use containers.Map.
                    //
                    return "struct('key', {}, 'value', {})";
                }
                else
                {
                    ostringstream ostr;
                    ostr << "containers.Map('KeyType', '" << dictionaryTypeToString(key, true) << "', 'ValueType', '"
                         << dictionaryTypeToString(value, false) << "')";
                    return ostr.str();
                }
            }

            EnumPtr en = dynamic_pointer_cast<Enum>(m->type());
            if (en)
            {
                const EnumeratorList enumerators = en->enumerators();
                return getAbsolute(*enumerators.begin());
            }

            StructPtr st = dynamic_pointer_cast<Struct>(m->type());
            if (st)
            {
                return getAbsolute(st) + "()";
            }

            return "[]";
        }
    }

    bool isProxy(const TypePtr& type)
    {
        BuiltinPtr b = dynamic_pointer_cast<Builtin>(type);
        InterfaceDeclPtr p = dynamic_pointer_cast<InterfaceDecl>(type);
        return (b && b->kind() == Builtin::KindObjectProxy) || p;
    }

    bool needsConversion(const TypePtr& type)
    {
        SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
        if (seq)
        {
            return seq->type()->isClassType() || needsConversion(seq->type());
        }

        StructPtr st = dynamic_pointer_cast<Struct>(type);
        if (st)
        {
            for (const auto& dm : st->dataMembers())
            {
                if (needsConversion(dm->type()) || dm->type()->isClassType())
                {
                    return true;
                }
            }
            return false;
        }

        DictionaryPtr d = dynamic_pointer_cast<Dictionary>(type);
        if (d)
        {
            return needsConversion(d->valueType()) || d->valueType()->isClassType();
        }

        return false;
    }

    void convertValueType(
        IceInternal::Output& out,
        const string& dest,
        const string& src,
        const TypePtr& type,
        bool optional)
    {
        assert(needsConversion(type));

        if (optional)
        {
            out << nl << "if " << src << " ~= Ice.Unset";
            out.inc();
        }

        SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
        if (seq)
        {
            out << nl << dest << " = " << getAbsolute(seq) << ".convert(" << src << ");";
        }

        DictionaryPtr d = dynamic_pointer_cast<Dictionary>(type);
        if (d)
        {
            out << nl << dest << " = " << getAbsolute(d) << ".convert(" << src << ");";
        }

        StructPtr st = dynamic_pointer_cast<Struct>(type);
        if (st)
        {
            out << nl << dest << " = " << src << ".ice_convert();";
        }

        if (optional)
        {
            out.dec();
            out << nl << "end";
        }
    }

    /// Returns a MATLAB formatted link to the provided Slice identifier.
    string matlabLinkFormatter(string identifier, string memberComponent)
    {
        if (memberComponent.empty())
        {
            return fixIdent(identifier);
        }
        else if (identifier.empty())
        {
            return fixIdent(memberComponent);
        }
        else
        {
            return fixIdent(identifier) + "." + fixIdent(memberComponent);
        }
    }

    void writeDocLines(IceInternal::Output& out, const StringList& lines, bool commentFirst, const string& space = " ")
    {
        StringList l = lines;
        if (!commentFirst)
        {
            out << l.front();
            l.pop_front();
        }
        for (const auto& i : l)
        {
            out << nl << "%";
            if (!i.empty())
            {
                out << space << i;
            }
        }
    }

    void writeDocSentence(IceInternal::Output& out, const StringList& lines)
    {
        //
        // Write the first sentence.
        //
        for (auto i = lines.begin(); i != lines.end(); ++i)
        {
            const string ws = " \t";

            if (i->empty())
            {
                break;
            }
            if (i != lines.begin() && i->find_first_not_of(ws) == 0)
            {
                out << " ";
            }
            string::size_type pos = i->find('.');
            if (pos == string::npos)
            {
                out << *i;
            }
            else if (pos == i->size() - 1)
            {
                out << *i;
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
                    out << i->substr(0, pos + 1);
                    break;
                }
                else
                {
                    out << *i;
                }
            }
        }
    }

    void writeSeeAlso(IceInternal::Output& out, const StringList& seeAlso, const ContainerPtr& container)
    {
        assert(!seeAlso.empty());
        //
        // All references must be on one line.
        //
        out << nl << "% See also ";
        for (auto p = seeAlso.begin(); p != seeAlso.end(); ++p)
        {
            if (p != seeAlso.begin())
            {
                out << ", ";
            }

            string sym = *p;
            string rest;
            string::size_type pos = sym.find('#');
            if (pos != string::npos)
            {
                rest = sym.substr(pos + 1);
                sym = sym.substr(0, pos);
            }

            if (!sym.empty() || !rest.empty())
            {
                if (!sym.empty())
                {
                    ContainedList c = container->lookupContained(sym, false);
                    if (c.empty())
                    {
                        out << sym;
                    }
                    else
                    {
                        out << getAbsolute(c.front());
                    }

                    if (!rest.empty())
                    {
                        out << ".";
                    }
                }

                if (!rest.empty())
                {
                    out << rest;
                }
            }
        }
    }

    void writeDocSummary(IceInternal::Output& out, const ContainedPtr& p)
    {
        DocCommentPtr doc = p->parseDocComment(matlabLinkFormatter, true);
        if (!doc)
        {
            return;
        }

        string n = fixIdent(p->name());

        //
        // No leading newline.
        //
        out << "% " << n << "   Summary of " << n;

        StringList docOverview = doc->overview();
        if (!docOverview.empty())
        {
            out << nl << "%";
            writeDocLines(out, docOverview, true);
        }

        if (EnumPtr en = dynamic_pointer_cast<Enum>(p))
        {
            const EnumeratorList enumerators = en->enumerators();
            if (!enumerators.empty())
            {
                out << nl << "%";
                out << nl << "% " << n << " Properties:";
                for (const auto& enumerator : enumerators)
                {
                    out << nl << "%   " << fixEnumerator(enumerator->name());
                    DocCommentPtr enumeratorDoc = enumerator->parseDocComment(matlabLinkFormatter, true);
                    if (enumeratorDoc)
                    {
                        StringList enumeratorOverview = enumeratorDoc->overview();
                        if (!enumeratorOverview.empty())
                        {
                            out << " - ";
                            writeDocSentence(out, enumeratorOverview);
                        }
                    }
                }
            }
        }
        else if (StructPtr st = dynamic_pointer_cast<Struct>(p))
        {
            const DataMemberList members = st->dataMembers();
            if (!members.empty())
            {
                out << nl << "%";
                out << nl << "% " << n << " Properties:";
                for (const auto& member : members)
                {
                    out << nl << "%   " << fixIdent(member->name());
                    if (DocCommentPtr memberDoc = member->parseDocComment(matlabLinkFormatter, true))
                    {
                        StringList memberOverview = memberDoc->overview();
                        if (!memberOverview.empty())
                        {
                            out << " - ";
                            writeDocSentence(out, memberOverview);
                        }
                    }
                }
            }
        }
        else if (ExceptionPtr ex = dynamic_pointer_cast<Exception>(p))
        {
            const DataMemberList members = ex->dataMembers();
            if (!members.empty())
            {
                out << nl << "%";
                out << nl << "% " << n << " Properties:";
                for (const auto& member : members)
                {
                    out << nl << "%   " << fixExceptionMember(member->name());
                    if (DocCommentPtr memberDoc = member->parseDocComment(matlabLinkFormatter, true))
                    {
                        StringList memberOverview = memberDoc->overview();
                        if (!memberOverview.empty())
                        {
                            out << " - ";
                            writeDocSentence(out, memberOverview);
                        }
                    }
                }
            }
        }
        else if (ClassDefPtr cl = dynamic_pointer_cast<ClassDef>(p))
        {
            const DataMemberList members = cl->dataMembers();
            if (!members.empty())
            {
                out << nl << "%";
                out << nl << "% " << n << " Properties:";
                for (const auto& member : members)
                {
                    out << nl << "%   " << fixIdent(member->name());
                    if (DocCommentPtr memberDoc = member->parseDocComment(matlabLinkFormatter, true))
                    {
                        StringList memberOverview = memberDoc->overview();
                        if (!memberOverview.empty())
                        {
                            out << " - ";
                            writeDocSentence(out, memberOverview);
                        }
                    }
                }
            }
        }

        StringList docSeeAlso = doc->seeAlso();
        if (!docSeeAlso.empty())
        {
            out << nl << "%";
            writeSeeAlso(out, docSeeAlso, p->container());
        }

        StringList docDeprecated = doc->deprecated();
        if (!docDeprecated.empty())
        {
            out << nl << "%";
            out << nl << "% Deprecated: ";
            writeDocLines(out, docDeprecated, false);
        }
        else if (doc->isDeprecated())
        {
            out << nl << "%";
            out << nl << "% Deprecated";
        }

        out << nl;
    }

    void writeOpDocSummary(IceInternal::Output& out, const OperationPtr& p, bool async)
    {
        DocCommentPtr doc = p->parseDocComment(matlabLinkFormatter, true);
        if (!doc)
        {
            return;
        }

        out << nl << "% " << (async ? p->name() + "Async" : fixOp(p->name()));

        StringList docOverview = doc->overview();
        if (!docOverview.empty())
        {
            out << "   ";
            writeDocLines(out, docOverview, false);
        }

        out << nl << "%";
        out << nl << "% Parameters:";
        auto docParameters = doc->parameters();
        const ParameterList inParams = p->inParameters();
        string ctxName = "context";
        string resultName = "result";
        for (const auto& inParam : inParams)
        {
            if (inParam->name() == "context")
            {
                ctxName = "context_";
            }
            if (inParam->name() == "result")
            {
                resultName = "result_";
            }

            out << nl << "%   " << fixIdent(inParam->name()) << " (" << typeToString(inParam->type()) << ")";
            auto r = docParameters.find(inParam->name());
            if (r != docParameters.end() && !r->second.empty())
            {
                out << " - ";
                writeDocLines(out, r->second, false, "     ");
            }
        }
        out << nl << "%   " << ctxName << " (containers.Map) - Optional request context.";

        if (async)
        {
            out << nl << "%";
            out << nl << "% Returns (Ice.Future) - A future that will be completed with the results of the invocation.";
        }
        else
        {
            const ParameterList outParams = p->outParameters();
            if (p->returnType() || !outParams.empty())
            {
                for (const auto& outParam : outParams)
                {
                    if (outParam->name() == "result")
                    {
                        resultName = "result_";
                    }
                }

                out << nl << "%";
                if (p->returnType() && outParams.empty())
                {
                    out << nl << "% Returns (" << typeToString(p->returnType()) << ")";
                    StringList docReturns = doc->returns();
                    if (!docReturns.empty())
                    {
                        out << " - ";
                        writeDocLines(out, docReturns, false);
                    }
                }
                else if (!p->returnType() && outParams.size() == 1)
                {
                    out << nl << "% Returns (" << typeToString(outParams.front()->type()) << ")";
                    auto q = docParameters.find(outParams.front()->name());
                    if (q != docParameters.end() && !q->second.empty())
                    {
                        out << " - ";
                        writeDocLines(out, q->second, false);
                    }
                }
                else
                {
                    out << nl << "% Returns:";
                    if (p->returnType())
                    {
                        out << nl << "%   " << resultName << " (" << typeToString(p->returnType()) << ")";
                        StringList docReturns = doc->returns();
                        if (!docReturns.empty())
                        {
                            out << " - ";
                            writeDocLines(out, docReturns, false, "     ");
                        }
                    }
                    for (const auto& outParam : outParams)
                    {
                        out << nl << "%   " << fixIdent(outParam->name()) << " (" << typeToString(outParam->type())
                            << ")";
                        auto r = docParameters.find(outParam->name());
                        if (r != docParameters.end() && !r->second.empty())
                        {
                            out << " - ";
                            writeDocLines(out, r->second, false, "     ");
                        }
                    }
                }
            }
        }

        auto docExceptions = doc->exceptions();
        if (!docExceptions.empty())
        {
            out << nl << "%";
            out << nl << "% Exceptions:";
            for (const auto& docException : docExceptions)
            {
                //
                // Try to find the exception based on the name given in the doc comment.
                //
                out << nl << "%   ";
                ExceptionPtr ex = p->container()->lookupException(docException.first, false);
                if (ex)
                {
                    out << getAbsolute(ex);
                }
                else
                {
                    out << docException.first;
                }
                if (!docException.second.empty())
                {
                    out << " - ";
                    writeDocLines(out, docException.second, false, "     ");
                }
            }
        }

        StringList docSeeAlso = doc->seeAlso();
        if (!docSeeAlso.empty())
        {
            out << nl << "%";
            writeSeeAlso(out, docSeeAlso, p->container());
        }

        StringList docDeprecated = doc->deprecated();
        if (!docDeprecated.empty())
        {
            out << nl << "%";
            out << nl << "% Deprecated: ";
            writeDocLines(out, docDeprecated, false);
        }
        else if (doc->isDeprecated())
        {
            out << nl << "%";
            out << nl << "% Deprecated";
        }

        out << nl;
    }

    void writeProxyDocSummary(IceInternal::Output& out, const InterfaceDefPtr& p)
    {
        DocCommentPtr doc = p->parseDocComment(matlabLinkFormatter, true);
        if (!doc)
        {
            return;
        }

        string n = p->name() + "Prx";

        //
        // No leading newline.
        //
        out << "% " << n << "   Summary of " << n;

        StringList docOverview = doc->overview();
        if (!docOverview.empty())
        {
            out << nl << "%";
            writeDocLines(out, docOverview, true);
        }

        out << nl << "%";
        out << nl << "% " << n << " Methods:";
        const OperationList ops = p->operations();
        if (!ops.empty())
        {
            for (auto op : ops)
            {
                DocCommentPtr opdoc = op->parseDocComment(matlabLinkFormatter, true);
                out << nl << "%   " << fixOp(op->name());
                if (opdoc)
                {
                    StringList opdocOverview = opdoc->overview();
                    if (!opdocOverview.empty())
                    {
                        out << " - ";
                        writeDocSentence(out, opdocOverview);
                    }
                }
                out << nl << "%   " << op->name() << "Async";
                if (opdoc)
                {
                    StringList opdocOverview = opdoc->overview();
                    if (!opdocOverview.empty())
                    {
                        out << " - ";
                        writeDocSentence(out, opdocOverview);
                    }
                }
            }
        }
        out << nl << "%   checkedCast - Contacts the remote server to verify that the object implements this type.";
        out << nl << "%   uncheckedCast - Downcasts the given proxy to this type without contacting the remote server.";

        StringList docSeeAlso = doc->seeAlso();
        if (!docSeeAlso.empty())
        {
            out << nl << "%";
            writeSeeAlso(out, docSeeAlso, p->container());
        }

        StringList docDeprecated = doc->deprecated();
        if (!docDeprecated.empty())
        {
            out << nl << "%";
            out << nl << "% Deprecated: ";
            writeDocLines(out, docDeprecated, false);
        }
        else if (doc->isDeprecated())
        {
            out << nl << "%";
            out << nl << "% Deprecated";
        }

        out << nl;
    }

    void writeMemberDoc(IceInternal::Output& out, const DataMemberPtr& p)
    {
        DocCommentPtr doc = p->parseDocComment(matlabLinkFormatter, true);
        if (!doc)
        {
            return;
        }

        StringList docOverview = doc->overview();
        StringList docSeeAlso = doc->seeAlso();
        StringList docDeprecated = doc->deprecated();
        bool docIsDeprecated = doc->isDeprecated();

        string n;

        if (dynamic_pointer_cast<Exception>(p))
        {
            n = fixExceptionMember(p->name());
        }
        else
        {
            n = fixIdent(p->name());
        }

        out << nl << "% " << n;

        if (!docOverview.empty())
        {
            out << " - ";
            writeDocLines(out, docOverview, false);
        }

        if (!docSeeAlso.empty())
        {
            out << nl << "%";
            writeSeeAlso(out, docSeeAlso, p->container());
        }

        if (!docDeprecated.empty())
        {
            out << nl << "%";
            out << nl << "% Deprecated: ";
            writeDocLines(out, docDeprecated, false);
        }
        else if (docIsDeprecated)
        {
            out << nl << "%";
            out << nl << "% Deprecated";
        }
    }
}

//
// CodeVisitor generates the Matlab mapping for a translation unit.
//
class CodeVisitor final : public ParserVisitor
{
public:
    CodeVisitor(string);

    bool visitClassDefStart(const ClassDefPtr&) final;
    bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
    bool visitExceptionStart(const ExceptionPtr&) final;
    bool visitStructStart(const StructPtr&) final;
    void visitSequence(const SequencePtr&) final;
    void visitDictionary(const DictionaryPtr&) final;
    void visitEnum(const EnumPtr&) final;
    void visitConst(const ConstPtr&) final;

private:
    struct MemberInfo
    {
        string fixedName;
        bool inherited;
        DataMemberPtr dataMember;
    };
    using MemberInfoList = list<MemberInfo>;

    //
    // Convert an operation mode into a string.
    //
    string getOperationMode(Slice::Operation::Mode);

    void collectClassMembers(const ClassDefPtr&, MemberInfoList&, bool);
    void collectExceptionMembers(const ExceptionPtr&, MemberInfoList&, bool);

    struct ParamInfo
    {
        string fixedName;
        TypePtr type;
        bool optional;
        int tag;
        int pos;            // Only used for out params
        ParameterPtr param; // 0 == return value
    };
    using ParamInfoList = list<ParamInfo>;

    ParamInfoList getAllInParams(const OperationPtr&);
    void getInParams(const OperationPtr&, ParamInfoList&, ParamInfoList&);
    ParamInfoList getAllOutParams(const OperationPtr&);
    void getOutParams(const OperationPtr&, ParamInfoList&, ParamInfoList&);

    string getOptionalFormat(const TypePtr&);
    string getFormatType(FormatType);

    void marshal(IceInternal::Output&, const string&, const string&, const TypePtr&, bool, int);
    void unmarshal(IceInternal::Output&, const string&, const string&, const TypePtr&, bool, int);

    void unmarshalStruct(IceInternal::Output&, const StructPtr&, const string&);
    void convertStruct(IceInternal::Output&, const StructPtr&, const string&);

    void writeBaseClassArrayParams(IceInternal::Output&, const MemberInfoList&, bool);

    const string _dir;
};

//
// CodeVisitor implementation.
//
CodeVisitor::CodeVisitor(string dir) : _dir(std::move(dir)) {}

bool
CodeVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    const string name = fixIdent(p->name());
    const string scoped = p->scoped();
    const string abs = getAbsolute(p);
    const string self = name == "obj" ? "this" : "obj";

    ClassDefPtr base = p->base();

    IceInternal::Output out;
    openClass(abs, _dir, out);

    writeDocSummary(out, p);
    writeCopyright(out, p->file());

    out << nl << "classdef ";
    out << name;
    if (base)
    {
        out << " < " << getAbsolute(base);
    }
    else
    {
        out << " < Ice.Value";
    }
    out.inc();

    const DataMemberList members = p->dataMembers();
    if (!members.empty())
    {
        if (p->hasMetadata("protected"))
        {
            //
            // All members are protected.
            //
            out << nl << "properties(Access=protected)";
            out.inc();
            for (const auto& member : members)
            {
                writeMemberDoc(out, member);
                out << nl << fixIdent(member->name());
                if (declarePropertyType(member->type(), member->optional()))
                {
                    out << " " << typeToString(member->type());
                }
            }
            out.dec();
            out << nl << "end";
        }
        else
        {
            DataMemberList prot, pub;
            for (const auto& member : members)
            {
                if (member->hasMetadata("protected"))
                {
                    prot.push_back(member);
                }
                else
                {
                    pub.push_back(member);
                }
            }
            if (!pub.empty())
            {
                out << nl << "properties";
                out.inc();
                for (const auto& q : pub)
                {
                    writeMemberDoc(out, q);
                    out << nl << fixIdent(q->name());
                    if (declarePropertyType(q->type(), q->optional()))
                    {
                        out << " " << typeToString(q->type());
                    }
                }
                out.dec();
                out << nl << "end";
            }
            if (!prot.empty())
            {
                out << nl << "properties(Access=protected)";
                out.inc();
                for (const auto& q : prot)
                {
                    writeMemberDoc(out, q);
                    out << nl << fixIdent(q->name());
                    if (declarePropertyType(q->type(), q->optional()))
                    {
                        out << " " << typeToString(q->type());
                    }
                }
                out.dec();
                out << nl << "end";
            }
        }
    }

    MemberInfoList allMembers;
    collectClassMembers(p, allMembers, false);

    out << nl << "methods";
    out.inc();

    //
    // Constructor
    //
    if (allMembers.empty())
    {
        out << nl << "function " << self << " = " << name << spar << "noInit" << epar;
        out.inc();
        out << nl << "if nargin == 1 && ne(noInit, IceInternal.NoInit.Instance)";
        out.inc();
        out << nl << "narginchk(0,0);";
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";
    }
    else
    {
        vector<string> allNames;
        for (const auto& allMember : allMembers)
        {
            allNames.push_back(allMember.fixedName);
        }
        out << nl << "function " << self << " = " << name << spar << allNames << epar;
        out.inc();
        if (base)
        {
            out << nl << "if nargin == 0";
            out.inc();
            for (const auto& allMember : allMembers)
            {
                out << nl << allMember.fixedName << " = " << defaultValue(allMember.dataMember) << ';';
            }
            writeBaseClassArrayParams(out, allMembers, false);
            out.dec();
            out << nl << "elseif eq(" << allMembers.begin()->fixedName << ", IceInternal.NoInit.Instance)";
            out.inc();
            writeBaseClassArrayParams(out, allMembers, true);
            out.dec();
            out << nl << "else";
            out.inc();
            writeBaseClassArrayParams(out, allMembers, false);
            out.dec();
            out << nl << "end";

            out << nl << self << " = " << self << "@" << getAbsolute(base) << "(v{:});";

            out << nl << "if ne(" << allMembers.begin()->fixedName << ", IceInternal.NoInit.Instance)";
            out.inc();
            for (const auto& allMember : allMembers)
            {
                if (!allMember.inherited)
                {
                    out << nl << self << "." << allMember.fixedName << " = " << allMember.fixedName << ';';
                }
            }
            out.dec();
            out << nl << "end";
        }
        else
        {
            out << nl << "if nargin == 0";
            out.inc();
            for (const auto& allMember : allMembers)
            {
                out << nl << self << "." << allMember.fixedName << " = " << defaultValue(allMember.dataMember) << ';';
            }
            out.dec();
            out << nl << "elseif ne(" << allMembers.begin()->fixedName << ", IceInternal.NoInit.Instance)";
            out.inc();
            for (const auto& allMember : allMembers)
            {
                out << nl << self << "." << allMember.fixedName << " = " << allMember.fixedName << ';';
            }
            out.dec();
            out << nl << "end";
        }

        out.dec();
        out << nl << "end";
    }

    out << nl << "function id = ice_id(obj)";
    out.inc();
    out << nl << "id = obj.ice_staticId();";
    out.dec();
    out << nl << "end";

    out.dec();
    out << nl << "end";

    DataMemberList convertMembers;
    for (const auto& member : members)
    {
        if (needsConversion(member->type()))
        {
            convertMembers.push_back(member);
        }
    }

    if (!convertMembers.empty())
    {
        out << nl << "methods(Hidden=true)";
        out.inc();

        out << nl << "function r = iceDelayPostUnmarshal(~)";
        out.inc();
        out << nl << "r = true;";
        out.dec();
        out << nl << "end";
        out << nl << "function icePostUnmarshal(obj)";
        out.inc();
        for (const auto& convertMember : convertMembers)
        {
            string m = "obj." + fixIdent(convertMember->name());
            convertValueType(out, m, m, convertMember->type(), convertMember->optional());
        }
        if (base)
        {
            out << nl << "icePostUnmarshal@" << getAbsolute(base) << "(obj);";
        }
        out.dec();
        out << nl << "end";

        out.dec();
        out << nl << "end";
    }

    out << nl << "methods(Access=protected)";
    out.inc();

    const DataMemberList optionalMembers = p->orderedOptionalDataMembers();

    out << nl << "function iceWriteImpl(obj, os)";
    out.inc();
    out << nl << "os.startSlice('" << scoped << "', " << p->compactId() << (!base ? ", true" : ", false") << ");";
    for (const auto& member : members)
    {
        if (!member->optional())
        {
            marshal(out, "os", "obj." + fixIdent(member->name()), member->type(), false, 0);
        }
    }
    for (const auto& optionalMember : optionalMembers)
    {
        marshal(
            out,
            "os",
            "obj." + fixIdent(optionalMember->name()),
            optionalMember->type(),
            true,
            optionalMember->tag());
    }
    out << nl << "os.endSlice();";
    if (base)
    {
        out << nl << "iceWriteImpl@" << getAbsolute(base) << "(obj, os);";
    }
    out.dec();
    out << nl << "end";
    out << nl << "function iceReadImpl(obj, is)";
    out.inc();
    out << nl << "is.startSlice();";
    for (const auto& dm : members)
    {
        if (!dm->optional())
        {
            if (dm->type()->isClassType())
            {
                unmarshal(out, "is", "@obj.iceSetMember_" + fixIdent(dm->name()), dm->type(), false, 0);
            }
            else
            {
                unmarshal(out, "is", "obj." + fixIdent(dm->name()), dm->type(), false, 0);
            }
        }
    }
    for (const auto& dm : optionalMembers)
    {
        assert(!dm->type()->isClassType());
        unmarshal(out, "is", "obj." + fixIdent(dm->name()), dm->type(), true, dm->tag());
    }
    out << nl << "is.endSlice();";
    if (base)
    {
        out << nl << "iceReadImpl@" << getAbsolute(base) << "(obj, is);";
    }
    out.dec();
    out << nl << "end";

    DataMemberList classMembers = p->classDataMembers();
    if (!classMembers.empty())
    {
        //
        // For each class data member, we generate an "iceSetMember_<name>" method that is called when the
        // instance is eventually unmarshaled.
        //
        for (const auto& classMember : classMembers)
        {
            string m = fixIdent(classMember->name());
            out << nl << "function iceSetMember_" << m << "(obj, v)";
            out.inc();
            out << nl << "obj." << m << " = v;";
            out.dec();
            out << nl << "end";
        }
    }

    out.dec();
    out << nl << "end";

    out << nl << "methods(Static)";
    out.inc();
    out << nl << "function id = ice_staticId()";
    out.inc();
    out << nl << "id = '" << scoped << "';";
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";

    out.dec();
    out << nl << "end";
    out << nl;

    out.close();

    if (p->compactId() >= 0)
    {
        ostringstream ostr;
        ostr << "IceCompactId.TypeId_" << p->compactId();

        openClass(ostr.str(), _dir, out);

        out << nl << "classdef TypeId_" << p->compactId();
        out.inc();

        out << nl << "properties(Constant)";
        out.inc();
        out << nl << "typeId = '" << scoped << "'";
        out.dec();
        out << nl << "end";

        out.dec();
        out << nl << "end";
        out << nl;

        out.close();
    }

    return false;
}

bool
CodeVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    const string name = fixIdent(p->name());
    const string scoped = p->scoped();
    const string abs = getAbsolute(p);
    InterfaceList bases = p->bases();

    //
    // Generate proxy class.
    //

    const string prxName = p->name() + "Prx";
    const string prxAbs = getAbsolute(p, "", "Prx");

    IceInternal::Output out;
    openClass(prxAbs, _dir, out);

    writeProxyDocSummary(out, p);
    writeCopyright(out, p->file());

    out << nl << "classdef " << prxName << " < ";
    if (!bases.empty())
    {
        for (auto q = bases.begin(); q != bases.end(); ++q)
        {
            if (q != bases.begin())
            {
                out << " & ";
            }
            out << getAbsolute(*q, "", "Prx");
        }
    }
    else
    {
        out << "Ice.ObjectPrx";
    }

    out.inc();

    out << nl << "methods";
    out.inc();

    //
    // Operations.
    //
    bool hasExceptions = false;
    const OperationList ops = p->operations();
    for (auto op : ops)
    {
        ParamInfoList requiredInParams, optionalInParams;
        getInParams(op, requiredInParams, optionalInParams);
        ParamInfoList requiredOutParams, optionalOutParams;
        getOutParams(op, requiredOutParams, optionalOutParams);
        const ParamInfoList allInParams = getAllInParams(op);
        const ParamInfoList allOutParams = getAllOutParams(op);
        const bool twowayOnly = op->returnsData();
        const ExceptionList exceptions = op->throws();

        if (!exceptions.empty())
        {
            hasExceptions = true;
        }

        //
        // Ensure no parameter is named "obj".
        //
        string self = "obj";
        for (const auto& allOutParam : allOutParams)
        {
            if (allOutParam.fixedName == "obj")
            {
                self = "obj_";
            }
        }
        for (const auto& allInParam : allInParams)
        {
            if (allInParam.fixedName == "obj")
            {
                self = "obj_";
            }
        }

        //
        // Synchronous method.
        //
        out << nl << "function ";
        if (allOutParams.size() > 1)
        {
            out << "[";
            for (auto r = allOutParams.begin(); r != allOutParams.end(); ++r)
            {
                if (r != allOutParams.begin())
                {
                    out << ", ";
                }
                out << r->fixedName;
            }
            out << "] = ";
        }
        else if (allOutParams.size() == 1)
        {
            out << allOutParams.begin()->fixedName << " = ";
        }
        out << fixOp(op->name()) << spar;

        out << self;
        for (const auto& allInParam : allInParams)
        {
            out << allInParam.fixedName;
        }
        out << "varargin"; // For the optional context
        out << epar;
        out.inc();

        writeOpDocSummary(out, op, false);

        if (!allInParams.empty())
        {
            if (op->format())
            {
                out << nl << "os_ = " << self << ".iceStartWriteParams(" << getFormatType(*op->format()) << ");";
            }
            else
            {
                out << nl << "os_ = " << self << ".iceStartWriteParams([]);";
            }
            for (const auto& requiredInParam : requiredInParams)
            {
                marshal(out, "os_", requiredInParam.fixedName, requiredInParam.type, false, 0);
            }
            for (const auto& optionalInParam : optionalInParams)
            {
                marshal(
                    out,
                    "os_",
                    optionalInParam.fixedName,
                    optionalInParam.type,
                    optionalInParam.optional,
                    optionalInParam.tag);
            }
            if (op->sendsClasses())
            {
                out << nl << "os_.writePendingValues();";
            }
            out << nl << self << ".iceEndWriteParams(os_);";
        }

        out << nl;
        if (!allOutParams.empty())
        {
            out << "is_ = ";
        }
        out << self << ".iceInvoke('" << op->name() << "', " << getOperationMode(op->mode()) << ", "
            << (twowayOnly ? "true" : "false") << ", " << (allInParams.empty() ? "[]" : "os_") << ", "
            << (!allOutParams.empty() ? "true" : "false");
        if (exceptions.empty())
        {
            out << ", {}";
        }
        else
        {
            out << ", " << prxAbs << "." << op->name() << "_ex_";
        }
        out << ", varargin{:});";

        if (twowayOnly && !allOutParams.empty())
        {
            out << nl << "is_.startEncapsulation();";
            //
            // To unmarshal results:
            //
            // * unmarshal all required out parameters
            // * unmarshal the required return value (if any)
            // * unmarshal all optional out parameters (this includes an optional return value)
            //
            ParamInfoList classParams;
            ParamInfoList convertParams;
            for (const auto& requiredOutParam : requiredOutParams)
            {
                if (requiredOutParam.param)
                {
                    string param;
                    if (requiredOutParam.type->isClassType())
                    {
                        out << nl << requiredOutParam.fixedName << "_h_ = IceInternal.ValueHolder();";
                        param = "@(v) " + requiredOutParam.fixedName + "_h_.set(v)";
                        classParams.push_back(requiredOutParam);
                    }
                    else
                    {
                        param = requiredOutParam.fixedName;
                    }
                    unmarshal(out, "is_", param, requiredOutParam.type, false, -1);

                    if (needsConversion(requiredOutParam.type))
                    {
                        convertParams.push_back(requiredOutParam);
                    }
                }
            }
            //
            // Now do the required return value if necessary.
            //
            if (!requiredOutParams.empty() && !requiredOutParams.begin()->param)
            {
                auto r = requiredOutParams.begin();
                string param;
                if (r->type->isClassType())
                {
                    out << nl << r->fixedName << "_h_ = IceInternal.ValueHolder();";
                    param = "@(v) " + r->fixedName + "_h_.set(v)";
                    classParams.push_back(*r);
                }
                else
                {
                    param = r->fixedName;
                }
                unmarshal(out, "is_", param, r->type, false, -1);

                if (needsConversion(r->type))
                {
                    convertParams.push_back(*r);
                }
            }
            //
            // Now unmarshal all optional out parameters. They are already sorted by tag.
            //
            for (const auto& optionalOutParam : optionalOutParams)
            {
                string param;
                if (optionalOutParam.type->isClassType())
                {
                    out << nl << optionalOutParam.fixedName << "_h_ = IceInternal.ValueHolder();";
                    param = "@(v) " + optionalOutParam.fixedName + "_h_.set(v)";
                    classParams.push_back(optionalOutParam);
                }
                else
                {
                    param = optionalOutParam.fixedName;
                }
                unmarshal(out, "is_", param, optionalOutParam.type, optionalOutParam.optional, optionalOutParam.tag);

                if (needsConversion(optionalOutParam.type))
                {
                    convertParams.push_back(optionalOutParam);
                }
            }
            if (op->returnsClasses())
            {
                out << nl << "is_.readPendingValues();";
            }
            out << nl << "is_.endEncapsulation();";
            //
            // After calling readPendingValues(), all callback functions have been invoked.
            // Now we need to collect the values.
            //
            for (const auto& classParam : classParams)
            {
                out << nl << classParam.fixedName << " = " << classParam.fixedName << "_h_.value;";
            }

            for (const auto& convertParam : convertParams)
            {
                convertValueType(
                    out,
                    convertParam.fixedName,
                    convertParam.fixedName,
                    convertParam.type,
                    convertParam.optional);
            }
        }

        out.dec();
        out << nl << "end";

        //
        // Asynchronous method.
        //
        out << nl << "function r_ = " << op->name() << "Async" << spar;
        out << self;
        for (const auto& allInParam : allInParams)
        {
            out << allInParam.fixedName;
        }
        out << "varargin"; // For the optional context
        out << epar;
        out.inc();

        writeOpDocSummary(out, op, true);

        if (!allInParams.empty())
        {
            if (op->format())
            {
                out << nl << "os_ = " << self << ".iceStartWriteParams(" << getFormatType(*op->format()) << ");";
            }
            else
            {
                out << nl << "os_ = " << self << ".iceStartWriteParams([]);";
            }
            for (const auto& requiredInParam : requiredInParams)
            {
                marshal(out, "os_", requiredInParam.fixedName, requiredInParam.type, false, 0);
            }
            for (const auto& optionalInParam : optionalInParams)
            {
                marshal(
                    out,
                    "os_",
                    optionalInParam.fixedName,
                    optionalInParam.type,
                    optionalInParam.optional,
                    optionalInParam.tag);
            }
            if (op->sendsClasses())
            {
                out << nl << "os_.writePendingValues();";
            }
            out << nl << self << ".iceEndWriteParams(os_);";
        }

        if (twowayOnly && !allOutParams.empty())
        {
            out << nl << "function varargout = unmarshal(is_)";
            out.inc();
            out << nl << "is_.startEncapsulation();";
            //
            // To unmarshal results:
            //
            // * unmarshal all required out parameters
            // * unmarshal the required return value (if any)
            // * unmarshal all optional out parameters (this includes an optional return value)
            //
            for (const auto& requiredOutParam : requiredOutParams)
            {
                if (requiredOutParam.param)
                {
                    string param;
                    if (requiredOutParam.type->isClassType())
                    {
                        out << nl << requiredOutParam.fixedName << " = IceInternal.ValueHolder();";
                        param = "@(v) " + requiredOutParam.fixedName + ".set(v)";
                    }
                    else
                    {
                        param = requiredOutParam.fixedName;
                    }
                    unmarshal(
                        out,
                        "is_",
                        param,
                        requiredOutParam.type,
                        requiredOutParam.optional,
                        requiredOutParam.tag);
                }
            }
            //
            // Now do the required return value if necessary.
            //
            if (!requiredOutParams.empty() && !requiredOutParams.begin()->param)
            {
                auto r = requiredOutParams.begin();
                string param;
                if (r->type->isClassType())
                {
                    out << nl << r->fixedName << " = IceInternal.ValueHolder();";
                    param = "@(v) " + r->fixedName + ".set(v)";
                }
                else
                {
                    param = r->fixedName;
                }
                unmarshal(out, "is_", param, r->type, false, -1);
            }
            //
            // Now unmarshal all optional out parameters. They are already sorted by tag.
            //
            for (const auto& optionalOutParam : optionalOutParams)
            {
                string param;
                if (optionalOutParam.type->isClassType())
                {
                    out << nl << optionalOutParam.fixedName << " = IceInternal.ValueHolder();";
                    param = "@(v) " + optionalOutParam.fixedName + ".set(v)";
                }
                else
                {
                    param = optionalOutParam.fixedName;
                }
                unmarshal(out, "is_", param, optionalOutParam.type, optionalOutParam.optional, optionalOutParam.tag);
            }
            if (op->returnsClasses())
            {
                out << nl << "is_.readPendingValues();";
            }
            out << nl << "is_.endEncapsulation();";
            for (const auto& requiredOutParam : requiredOutParams)
            {
                if (requiredOutParam.type->isClassType())
                {
                    out << nl << "varargout{" << requiredOutParam.pos << "} = " << requiredOutParam.fixedName
                        << ".value;";
                }
                else if (needsConversion(requiredOutParam.type))
                {
                    ostringstream dest;
                    dest << "varargout{" << requiredOutParam.pos << "}";
                    convertValueType(
                        out,
                        dest.str(),
                        requiredOutParam.fixedName,
                        requiredOutParam.type,
                        requiredOutParam.optional);
                }
                else
                {
                    out << nl << "varargout{" << requiredOutParam.pos << "} = " << requiredOutParam.fixedName << ';';
                }
            }
            for (const auto& optionalOutParam : optionalOutParams)
            {
                if (optionalOutParam.type->isClassType())
                {
                    out << nl << "varargout{" << optionalOutParam.pos << "} = " << optionalOutParam.fixedName
                        << ".value;";
                }
                else if (needsConversion(optionalOutParam.type))
                {
                    ostringstream dest;
                    dest << "varargout{" << optionalOutParam.pos << "}";
                    convertValueType(
                        out,
                        dest.str(),
                        optionalOutParam.fixedName,
                        optionalOutParam.type,
                        optionalOutParam.optional);
                }
                else
                {
                    out << nl << "varargout{" << optionalOutParam.pos << "} = " << optionalOutParam.fixedName << ';';
                }
            }
            out.dec();
            out << nl << "end";
        }

        out << nl << "r_ = " << self << ".iceInvokeAsync('" << op->name() << "', " << getOperationMode(op->mode())
            << ", " << (twowayOnly ? "true" : "false") << ", " << (allInParams.empty() ? "[]" : "os_") << ", "
            << allOutParams.size() << ", " << (twowayOnly && !allOutParams.empty() ? "@unmarshal" : "[]");
        if (exceptions.empty())
        {
            out << ", {}";
        }
        else
        {
            out << ", " << prxAbs << "." << op->name() << "_ex_";
        }
        out << ", varargin{:});";

        out.dec();
        out << nl << "end";
    }

    out.dec();
    out << nl << "end";

    out << nl << "methods(Static)";
    out.inc();
    out << nl << "function id = ice_staticId()";
    out.inc();
    out << nl << "id = '" << scoped << "';";
    out.dec();
    out << nl << "end";
    out << nl << "function r = ice_read(is)";
    out.inc();
    out << nl << "r = is.readProxy('" << prxAbs << "');";
    out.dec();
    out << nl << "end";
    out << nl << "function r = checkedCast(p, varargin)";
    out.inc();
    out << nl << "% checkedCast   Contacts the remote server to verify that the object implements this type.";
    out << nl << "%   Raises a local exception if a communication error occurs. You can optionally supply a";
    out << nl << "%   facet name and a context map.";
    out << nl << "%";
    out << nl << "% Parameters:";
    out << nl << "%   p - The proxy to be cast.";
    out << nl << "%   facet - The optional name of the desired facet.";
    out << nl << "%   context - The optional context map to send with the invocation.";
    out << nl << "%";
    out << nl << "% Returns (" << prxAbs << ") - A proxy for this type, or an empty array if the object"
        << " does not support this type.";
    out << nl << "r = Ice.ObjectPrx.iceCheckedCast(p, " << prxAbs << ".ice_staticId(), '" << prxAbs
        << "', varargin{:});";
    out.dec();
    out << nl << "end";
    out << nl << "function r = uncheckedCast(p, varargin)";
    out.inc();
    out << nl << "% uncheckedCast   Downcasts the given proxy to this type without contacting the remote server.";
    out << nl << "%   You can optionally specify a facet name.";
    out << nl << "%";
    out << nl << "% Parameters:";
    out << nl << "%   p - The proxy to be cast.";
    out << nl << "%   facet - The optional name of the desired facet.";
    out << nl << "%";
    out << nl << "% Returns (" << prxAbs << ") - A proxy for this type.";
    out << nl << "r = Ice.ObjectPrx.iceUncheckedCast(p, '" << prxAbs << "', varargin{:});";
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";

    // The constructor is inherited, even with multiple inheritance.

    if (hasExceptions)
    {
        //
        // Generate a constant property for each operation that throws user exceptions. The property is
        // a cell array containing the class names of the exceptions.
        //
        out << nl << "properties(Constant,Access=private)";
        out.inc();
        for (auto op : ops)
        {
            // Arrange exceptions into most-derived to least-derived order. If we don't
            // do this, a base exception handler can appear before a derived exception
            // handler, causing compiler warnings and resulting in the base exception
            // being marshaled instead of the derived exception.
            ExceptionList exceptions = op->throws();
            exceptions.sort(Slice::DerivedToBaseCompare());

            if (!exceptions.empty())
            {
                out << nl << op->name() << "_ex_ = { ";
                for (auto e = exceptions.begin(); e != exceptions.end(); ++e)
                {
                    if (e != exceptions.begin())
                    {
                        out << ", ";
                    }
                    out << "'" + getAbsolute(*e) + "'";
                }
                out << " }";
            }
        }
        out.dec();
        out << nl << "end";
    }

    out.dec();
    out << nl << "end";
    out << nl;

    out.close();

    return false;
}

bool
CodeVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    const string name = fixIdent(p->name());
    const string scoped = p->scoped();
    const string abs = getAbsolute(p);

    IceInternal::Output out;
    openClass(abs, _dir, out);

    writeDocSummary(out, p);
    writeCopyright(out, p->file());

    ExceptionPtr base = p->base();

    out << nl << "classdef " << name;
    if (base)
    {
        out << " < " << getAbsolute(base);
    }
    else
    {
        out << " < Ice.UserException";
    }
    out.inc();

    const DataMemberList members = p->dataMembers();
    if (!members.empty())
    {
        out << nl << "properties";
        out.inc();
        for (const auto& member : members)
        {
            writeMemberDoc(out, member);
            out << nl << fixExceptionMember(member->name());
            if (declarePropertyType(member->type(), member->optional()))
            {
                out << " " << typeToString(member->type());
            }
        }
        out.dec();
        out << nl << "end";
    }

    MemberInfoList allMembers;
    collectExceptionMembers(p, allMembers, false);

    vector<string> allNames;
    MemberInfoList convertMembers;
    for (const auto& allMember : allMembers)
    {
        allNames.push_back(allMember.fixedName);

        if (!allMember.inherited && needsConversion(allMember.dataMember->type()))
        {
            convertMembers.push_back(allMember);
        }
    }
    out << nl << "methods";
    out.inc();

    const string self = name == "obj" ? "this" : "obj";

    //
    // Constructor
    //
    out << nl << "function " << self << " = " << name << spar << "errID" << "msg" << epar;
    out.inc();
    string errID = abs;
    const string msg = abs;
    //
    // The ID argument must use colon separators.
    //
    string::size_type pos = errID.find('.');
    assert(pos != string::npos);
    while (pos != string::npos)
    {
        errID[pos] = ':';
        pos = errID.find('.', pos);
    }

    out << nl << "if nargin == 0";
    out.inc();
    out << nl << "errID = '" << errID << "';";
    out << nl << "msg = '" << msg << "';";
    out.dec();
    out << nl << "else";
    out.inc();
    out << nl << "assert(nargin == 2, 'Invalid number of arguments');";
    out.dec();
    out << nl << "end";

    if (!base)
    {
        out << nl << self << " = " << self << "@Ice.UserException" << spar << "errID" << "msg" << epar << ';';
    }
    else
    {
        out << nl << self << " = " << self << "@" << getAbsolute(base) << spar << "errID" << "msg" << epar << ';';
    }
    out.dec();
    out << nl << "end";

    out << nl << "function id = ice_id(~)";
    out.inc();
    out << nl << "id = '" << scoped << "';";
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";

    const DataMemberList classMembers = p->classDataMembers();
    if (!classMembers.empty() || !convertMembers.empty())
    {
        out << nl << "methods(Hidden=true)";
        out.inc();
        out << nl << "function obj = icePostUnmarshal(obj)";
        out.inc();
        for (const auto& classMember : classMembers)
        {
            string m = fixExceptionMember(classMember->name());
            out << nl << "obj." << m << " = obj." << m << ".value;";
        }
        for (const auto& convertMember : convertMembers)
        {
            string m = "obj." + convertMember.fixedName;
            convertValueType(out, m, m, convertMember.dataMember->type(), convertMember.dataMember->optional());
        }
        if (base && base->usesClasses())
        {
            out << nl << "obj = icePostUnmarshal@" << getAbsolute(base) << "(obj);";
        }
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";
    }

    out << nl << "methods(Access=protected)";
    out.inc();

    out << nl << "function obj = iceReadImpl(obj, is)";
    out.inc();
    out << nl << "is.startSlice();";
    for (const auto& dm : members)
    {
        string m = fixExceptionMember(dm->name());
        if (!dm->optional())
        {
            if (dm->type()->isClassType())
            {
                out << nl << "obj." << m << " = IceInternal.ValueHolder();";
                unmarshal(out, "is", "@(v) obj." + m + ".set(v)", dm->type(), false, 0);
            }
            else
            {
                unmarshal(out, "is", "obj." + m, dm->type(), false, 0);
            }
        }
    }
    const DataMemberList optionalMembers = p->orderedOptionalDataMembers();
    for (const auto& dm : optionalMembers)
    {
        string m = fixExceptionMember(dm->name());
        if (dm->type()->isClassType())
        {
            out << nl << "obj." << m << " = IceInternal.ValueHolder();";
            unmarshal(out, "is", "@(v) obj." + m + ".set(v)", dm->type(), true, dm->tag());
        }
        else
        {
            unmarshal(out, "is", "obj." + m, dm->type(), true, dm->tag());
        }
    }
    out << nl << "is.endSlice();";
    if (base)
    {
        out << nl << "obj = iceReadImpl@" << getAbsolute(base) << "(obj, is);";
    }
    out.dec();
    out << nl << "end";

    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";
    out << nl;

    out.close();

    return false;
}

bool
CodeVisitor::visitStructStart(const StructPtr& p)
{
    const string name = fixIdent(p->name());
    const string scoped = p->scoped();
    const string abs = getAbsolute(p);

    IceInternal::Output out;
    openClass(abs, _dir, out);

    writeDocSummary(out, p);
    writeCopyright(out, p->file());

    const DataMemberList members = p->dataMembers();
    const DataMemberList classMembers = p->classDataMembers();

    out << nl << "classdef " << name;

    out.inc();
    out << nl << "properties";
    out.inc();
    vector<string> memberNames;
    DataMemberList convertMembers;
    for (const auto& member : members)
    {
        const string m = fixStructMember(member->name());
        memberNames.push_back(m);
        writeMemberDoc(out, member);
        out << nl << m;
        if (declarePropertyType(member->type(), false))
        {
            out << " " << typeToString(member->type());
        }

        if (needsConversion(member->type()))
        {
            convertMembers.push_back(member);
        }
    }
    out.dec();
    out << nl << "end";

    out << nl << "methods";
    out.inc();
    string self = name == "obj" ? "this" : "obj";
    out << nl << "function " << self << " = " << name << spar << memberNames << epar;
    out.inc();
    out << nl << "if nargin == 0";
    out.inc();
    for (const auto& member : members)
    {
        out << nl << self << "." << fixStructMember(member->name()) << " = " << defaultValue(member) << ';';
    }
    out.dec();
    out << nl << "elseif ne(" << fixStructMember((*members.begin())->name()) << ", IceInternal.NoInit.Instance)";
    out.inc();
    for (const auto& memberName : memberNames)
    {
        out << nl << self << "." << memberName << " = " << memberName << ';';
    }
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";
    out << nl << "function r = eq(obj, other)";
    out.inc();
    out << nl << "r = isequal(obj, other);";
    out.dec();
    out << nl << "end";
    out << nl << "function r = ne(obj, other)";
    out.inc();
    out << nl << "r = ~isequal(obj, other);";
    out.dec();
    out << nl << "end";

    if (!convertMembers.empty() || !classMembers.empty())
    {
        out << nl << "function obj = ice_convert(obj)";
        out.inc();
        convertStruct(out, p, "obj");
        out.dec();
        out << nl << "end";
    }

    out.dec();
    out << nl << "end";

    out << nl << "methods(Static)";
    out.inc();
    out << nl << "function r = ice_read(is)";
    out.inc();
    out << nl << "r = " << abs << "(IceInternal.NoInit.Instance);";
    unmarshalStruct(out, p, "r");
    out.dec();
    out << nl << "end";

    out << nl << "function ice_write(os, v)";
    out.inc();
    out << nl << "if isempty(v)";
    out.inc();
    out << nl << "v = " << abs << "();";
    out.dec();
    out << nl << "end";
    for (const auto& member : members)
    {
        marshal(out, "os", "v." + fixStructMember(member->name()), member->type(), false, 0);
    }
    out.dec();
    out << nl << "end";

    if (!p->usesClasses())
    {
        out << nl << "function r = ice_readOpt(is, tag)";
        out.inc();
        out << nl << "if is.readOptional(tag, " << getOptionalFormat(p) << ")";
        out.inc();
        if (p->isVariableLength())
        {
            out << nl << "is.skip(4);";
        }
        else
        {
            out << nl << "is.skipSize();";
        }
        out << nl << "r = " << abs << ".ice_read(is);";
        out.dec();
        out << nl << "else";
        out.inc();
        out << nl << "r = Ice.Unset;";
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";

        out << nl << "function ice_writeOpt(os, tag, v)";
        out.inc();
        out << nl << "if v ~= Ice.Unset && os.writeOptional(tag, " << getOptionalFormat(p) << ")";
        out.inc();
        if (p->isVariableLength())
        {
            out << nl << "pos = os.startSize();";
            out << nl << abs << ".ice_write(os, v);";
            out << nl << "os.endSize(pos);";
        }
        else
        {
            out << nl << "os.writeSize(" << p->minWireSize() << ");";
            out << nl << abs << ".ice_write(os, v);";
        }
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";
    }

    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";
    out << nl;

    out.close();

    return false;
}

void
CodeVisitor::visitSequence(const SequencePtr& p)
{
    const TypePtr content = p->type();

    const BuiltinPtr b = dynamic_pointer_cast<Builtin>(content);
    if (b)
    {
        switch (b->kind())
        {
            case Builtin::KindBool:
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindInt:
            case Builtin::KindLong:
            case Builtin::KindFloat:
            case Builtin::KindDouble:
            case Builtin::KindString:
            {
                return;
            }
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindValue:
            {
                break;
            }
        }
    }

    EnumPtr enumContent = dynamic_pointer_cast<Enum>(content);
    SequencePtr seqContent = dynamic_pointer_cast<Sequence>(content);
    StructPtr structContent = dynamic_pointer_cast<Struct>(content);
    DictionaryPtr dictContent = dynamic_pointer_cast<Dictionary>(content);

    const string name = fixIdent(p->name());
    const string scoped = p->scoped();
    const string abs = getAbsolute(p);
    const bool cls = content->isClassType();
    const bool proxy = isProxy(content);
    const bool convert = needsConversion(content);

    IceInternal::Output out;
    openClass(abs, _dir, out);

    writeCopyright(out, p->file());

    out << nl << "classdef " << name;
    out.inc();
    out << nl << "methods(Static)";
    out.inc();

    out << nl << "function write(os, seq)";
    out.inc();
    out << nl << "sz = length(seq);";
    out << nl << "os.writeSize(sz);";
    out << nl << "for i = 1:sz";
    out.inc();
    //
    // Aside from the primitive types, only enum and struct sequences are mapped to arrays. The rest are mapped
    // to cell arrays. We can't use the same subscript syntax for both.
    //
    if (enumContent || structContent)
    {
        marshal(out, "os", "seq(i)", content, false, 0);
    }
    else
    {
        marshal(out, "os", "seq{i}", content, false, 0);
    }
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";

    out << nl << "function r = read(is)";
    out.inc();
    out << nl << "sz = is.readSize();";
    if (cls)
    {
        //
        // For a sequence<class>, read() returns an instance of IceInternal.CellArrayHandle that we later replace
        // with the cell array. See convert().
        //
        out << nl << "if sz == 0";
        out.inc();
        out << nl << "r = {};";
        out.dec();
        out << nl << "else";
        out.inc();
        out << nl << "r = IceInternal.CellArrayHandle();";
        out << nl << "r.array = cell(1, sz);";
        out << nl << "for i = 1:sz";
        out.inc();
        //
        // Ice.CellArrayHandle defines a set() method that we call from the lambda.
        //
        unmarshal(out, "is", "@(v) r.set(i, v)", content, false, 0);
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";
    }
    else if ((b && b->kind() == Builtin::KindString) || dictContent || seqContent || proxy)
    {
        //
        // These types require a cell array.
        //
        out << nl << "if sz == 0";
        out.inc();
        out << nl << "r = {};";
        out.dec();
        out << nl << "else";
        out.inc();
        out << nl << "r = cell(1, sz);";
        out << nl << "for i = 1:sz";
        out.inc();
        unmarshal(out, "is", "r{i}", content, false, 0);
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";
    }
    else if (enumContent)
    {
        const EnumeratorList enumerators = enumContent->enumerators();
        out << nl << "r = " << getAbsolute(enumContent) << ".empty();";
        out << nl << "if sz > 0";
        out.inc();
        out << nl << "r(1, sz) = " << getAbsolute(*enumerators.begin()) << ";";
        out << nl << "for i = 1:sz";
        out.inc();
        unmarshal(out, "is", "r(i)", content, false, 0);
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";
    }
    else if (structContent)
    {
        //
        // The most efficient way to build a sequence of structs is to pre-allocate the array using the
        // syntax "arr(1, sz) = Type()". Additionally, we also have to inline the unmarshaling code for
        // the struct members.
        //
        out << nl << "r = " << getAbsolute(structContent) << ".empty();";
        out << nl << "if sz > 0";
        out.inc();
        out << nl << "r(1, sz) = " << getAbsolute(structContent) << "();";
        out << nl << "for i = 1:sz";
        out.inc();
        unmarshalStruct(out, structContent, "r(i)");
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";
    }
    else
    {
        assert(false);
    }
    out.dec();
    out << nl << "end";

    if (!p->type()->usesClasses())
    {
        out << nl << "function writeOpt(os, tag, seq)";
        out.inc();
        out << nl << "if seq ~= Ice.Unset && os.writeOptional(tag, " << getOptionalFormat(p) << ")";
        out.inc();
        if (p->type()->isVariableLength())
        {
            out << nl << "pos = os.startSize();";
            out << nl << abs << ".write(os, seq);";
            out << nl << "os.endSize(pos);";
        }
        else
        {
            //
            // The element is a fixed-size type. If the element type is bool or byte, we do NOT write an extra size.
            //
            const size_t sz = p->type()->minWireSize();
            if (sz > 1)
            {
                out << nl << "len = length(seq);";
                out << nl << "if len > 254";
                out.inc();
                out << nl << "os.writeSize(len * " << sz << " + 5);";
                out.dec();
                out << nl << "else";
                out.inc();
                out << nl << "os.writeSize(len * " << sz << " + 1);";
                out.dec();
                out << nl << "end";
            }
            out << nl << abs << ".write(os, seq);";
        }
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";

        out << nl << "function r = readOpt(is, tag)";
        out.inc();
        out << nl << "if is.readOptional(tag, " << getOptionalFormat(p) << ")";
        out.inc();
        if (p->type()->isVariableLength())
        {
            out << nl << "is.skip(4);";
        }
        else if (p->type()->minWireSize() > 1)
        {
            out << nl << "is.skipSize();";
        }
        out << nl << "r = " << abs << ".read(is);";
        out.dec();
        out << nl << "else";
        out.inc();
        out << nl << "r = Ice.Unset;";
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";
    }

    if (cls || convert)
    {
        out << nl << "function r = convert(seq)";
        out.inc();
        if (cls)
        {
            out << nl << "if isempty(seq)";
            out.inc();
            out << nl << "r = seq;";
            out.dec();
            out << nl << "else";
            out.inc();
            out << nl << "r = seq.array;";
            out.dec();
            out << nl << "end";
        }
        else
        {
            assert(structContent || seqContent || dictContent);
            if (structContent)
            {
                //
                // Inline the conversion.
                //
                out << nl << "r = seq;";
                out << nl << "for i = 1:length(seq)";
                out.inc();
                convertStruct(out, structContent, "r(i)");
                out.dec();
                out << nl << "end";
            }
            else
            {
                out << nl << "sz = length(seq);";
                out << nl << "if sz > 0";
                out.inc();
                out << nl << "r = cell(1, sz);";
                out << nl << "for i = 1:sz";
                out.inc();
                convertValueType(out, "r{i}", "seq{i}", content, false);
                out << nl << "end";
                out.dec();
                out.dec();
                out << nl << "else";
                out.inc();
                out << nl << "r = seq;";
                out.dec();
                out << nl << "end";
            }
        }
        out.dec();
        out << nl << "end";
    }

    out.dec();
    out << nl << "end";

    out.dec();
    out << nl << "end";
    out << nl;

    out.close();
}

void
CodeVisitor::visitDictionary(const DictionaryPtr& p)
{
    const TypePtr key = p->keyType();
    const TypePtr value = p->valueType();
    const bool cls = value->isClassType();
    const bool convert = needsConversion(value);

    const StructPtr st = dynamic_pointer_cast<Struct>(key);

    const string name = fixIdent(p->name());
    const string scoped = p->scoped();
    const string abs = getAbsolute(p);
    const string self = name == "obj" ? "this" : "obj";

    IceInternal::Output out;
    openClass(abs, _dir, out);

    writeCopyright(out, p->file());

    out << nl << "classdef " << name;
    out.inc();
    out << nl << "methods(Access=private)";
    out.inc();
    //
    // Declare a private constructor so that programs can't instantiate this type. They need to use new().
    //
    out << nl << "function " << self << " = " << name << "()";
    out.inc();
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";
    out << nl << "methods(Static)";
    out.inc();

    out << nl << "function write(os, d)";
    out.inc();
    out << nl << "if isempty(d)";
    out.inc();
    out << nl << "os.writeSize(0);";
    out.dec();
    out << nl << "else";
    out.inc();
    if (st)
    {
        out << nl << "sz = length(d);";
        out << nl << "os.writeSize(sz);";
        out << nl << "for i = 1:sz";
        out.inc();
        marshal(out, "os", "d(i).key", key, false, 0);
        marshal(out, "os", "d(i).value", value, false, 0);
        out.dec();
        out << nl << "end";
    }
    else
    {
        out << nl << "sz = d.Count;";
        out << nl << "os.writeSize(sz);";
        out << nl << "keys = d.keys();";
        out << nl << "values = d.values();";
        out << nl << "for i = 1:sz";
        out.inc();
        out << nl << "k = keys{i};";
        out << nl << "v = values{i};";
        marshal(out, "os", "k", key, false, 0);
        marshal(out, "os", "v", value, false, 0);
        out.dec();
        out << nl << "end";
    }
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";

    out << nl << "function r = read(is)";
    out.inc();
    out << nl << "sz = is.readSize();";
    if (st)
    {
        //
        // We use a struct array when the key is a structure type because we can't use containers.Map.
        //
        out << nl << "r = struct('key', {}, 'value', {});";
    }
    else
    {
        out << nl << "r = containers.Map('KeyType', '" << dictionaryTypeToString(key, true) << "', 'ValueType', '"
            << dictionaryTypeToString(value, false) << "');";
    }
    out << nl << "for i = 1:sz";
    out.inc();

    unmarshal(out, "is", "k", key, false, 0);

    if (cls)
    {
        out << nl << "v = IceInternal.ValueHolder();";
        unmarshal(out, "is", "@(v_) v.set(v_)", value, false, 0);
    }
    else
    {
        unmarshal(out, "is", "v", value, false, 0);
    }

    if (st)
    {
        out << nl << "r(i).key = k;";
        out << nl << "r(i).value = v;";
    }
    else if (dynamic_pointer_cast<Enum>(key))
    {
        out << nl << "r(int32(k)) = v;";
    }
    else
    {
        out << nl << "r(k) = v;";
    }

    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";

    if (!p->valueType()->usesClasses())
    {
        out << nl << "function writeOpt(os, tag, d)";
        out.inc();
        out << nl << "if d ~= Ice.Unset && os.writeOptional(tag, " << getOptionalFormat(p) << ")";
        out.inc();
        if (key->isVariableLength() || value->isVariableLength())
        {
            out << nl << "pos = os.startSize();";
            out << nl << abs << ".write(os, d);";
            out << nl << "os.endSize(pos);";
        }
        else
        {
            const size_t sz = key->minWireSize() + value->minWireSize();
            if (cls)
            {
                out << nl << "len = length(d.array);";
            }
            else
            {
                out << nl << "len = length(d);";
            }
            out << nl << "if len > 254";
            out.inc();
            out << nl << "os.writeSize(len * " << sz << " + 5);";
            out.dec();
            out << nl << "else";
            out.inc();
            out << nl << "os.writeSize(len * " << sz << " + 1);";
            out.dec();
            out << nl << "end";
            out << nl << abs << ".write(os, d);";
        }
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";

        out << nl << "function r = readOpt(is, tag)";
        out.inc();
        out << nl << "if is.readOptional(tag, " << getOptionalFormat(p) << ")";
        out.inc();
        if (key->isVariableLength() || value->isVariableLength())
        {
            out << nl << "is.skip(4);";
        }
        else
        {
            out << nl << "is.skipSize();";
        }
        out << nl << "r = " << abs << ".read(is);";
        out.dec();
        out << nl << "else";
        out.inc();
        out << nl << "r = Ice.Unset;";
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";
    }

    if (cls || convert)
    {
        out << nl << "function r = convert(d)";
        out.inc();
        if (st)
        {
            out << nl << "for i = 1:length(d)";
            out.inc();
            if (cls)
            {
                //
                // Each entry has a temporary ValueHolder that we need to replace with the actual value.
                //
                out << nl << "d(i).value = d(i).value.value;";
            }
            else
            {
                convertValueType(out, "d(i).value", "d(i).value", value, false);
            }
            out.dec();
            out << nl << "end";
        }
        else
        {
            out << nl << "keys = d.keys();";
            out << nl << "values = d.values();";
            out << nl << "for i = 1:d.Count";
            out.inc();
            out << nl << "k = keys{i};";
            out << nl << "v = values{i};";
            if (cls)
            {
                //
                // Each entry has a temporary ValueHolder that we need to replace with the actual value.
                //
                out << nl << "d(k) = v.value;";
            }
            else
            {
                convertValueType(out, "d(k)", "v", value, false);
            }
            out.dec();
            out << nl << "end";
        }
        out << nl << "r = d;";
        out.dec();
        out << nl << "end";
    }

    out.dec();
    out << nl << "end";

    out.dec();
    out << nl << "end";
    out << nl;

    out.close();
}

void
CodeVisitor::visitEnum(const EnumPtr& p)
{
    const string name = fixIdent(p->name());
    const string scoped = p->scoped();
    const string abs = getAbsolute(p);
    const EnumeratorList enumerators = p->enumerators();

    IceInternal::Output out;
    openClass(abs, _dir, out);

    writeDocSummary(out, p);
    writeCopyright(out, p->file());

    out << nl << "classdef " << name;
    if (p->maxValue() <= 255)
    {
        out << " < uint8";
    }
    else
    {
        out << " < int32";
    }

    out.inc();
    out << nl << "enumeration";
    out.inc();
    for (const auto& enumerator : enumerators)
    {
        writeDocSummary(out, enumerator);
        out << nl << fixEnumerator(enumerator->name()) << " (" << enumerator->value() << ")";
    }
    out.dec();
    out << nl << "end";

    out << nl << "methods(Static)";
    out.inc();

    out << nl << "function ice_write(os, v)";
    out.inc();
    out << nl << "if isempty(v)";
    out.inc();
    string firstEnum = fixEnumerator(enumerators.front()->name());
    out << nl << "os.writeEnum(int32(" << abs << "." << firstEnum << "), " << p->maxValue() << ");";
    out.dec();
    out << nl << "else";
    out.inc();
    out << nl << "os.writeEnum(int32(v), " << p->maxValue() << ");";
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";

    out << nl << "function r = ice_read(is)";
    out.inc();
    out << nl << "v = is.readEnum(" << p->maxValue() << ");";
    out << nl << "r = " << abs << ".ice_getValue(v);";
    out.dec();
    out << nl << "end";

    out << nl << "function ice_writeOpt(os, tag, v)";
    out.inc();
    out << nl << "if v ~= Ice.Unset && os.writeOptional(tag, " << getOptionalFormat(p) << ")";
    out.inc();
    out << nl << abs << ".ice_write(os, v);";
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";

    out << nl << "function r = ice_readOpt(is, tag)";
    out.inc();
    out << nl << "if is.readOptional(tag, " << getOptionalFormat(p) << ")";
    out.inc();
    out << nl << "r = " << abs << ".ice_read(is);";
    out.dec();
    out << nl << "else";
    out.inc();
    out << nl << "r = Ice.Unset;";
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";

    out << nl << "function r = ice_getValue(v)";
    out.inc();
    out << nl << "switch v";
    out.inc();
    for (const auto& enumerator : enumerators)
    {
        out << nl << "case " << enumerator->value();
        out.inc();
        out << nl << "r = " << abs << "." << fixEnumerator(enumerator->name()) << ";";
        out.dec();
    }
    out << nl << "otherwise";
    out.inc();
    out << nl << "throw(Ice.MarshalException(sprintf('enumerator value %d is out of range', v)));";
    out.dec();
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";

    out.dec();
    out << nl << "end";

    out.dec();
    out << nl << "end";
    out << nl;
    out.close();
}

void
CodeVisitor::visitConst(const ConstPtr& p)
{
    const string name = fixIdent(p->name());
    const string scoped = p->scoped();
    const string abs = getAbsolute(p);

    IceInternal::Output out;
    openClass(abs, _dir, out);

    writeDocSummary(out, p);
    writeCopyright(out, p->file());

    out << nl << "classdef " << name;

    out.inc();
    out << nl << "properties(Constant)";
    out.inc();
    out << nl << "value " << typeToString(p->type()) << " = " << constantValue(p->type(), p->valueType(), p->value());
    out.dec();
    out << nl << "end";

    out.dec();
    out << nl << "end";
    out << nl;
    out.close();
    out.close();
}

string
CodeVisitor::getOperationMode(Slice::Operation::Mode mode)
{
    switch (mode)
    {
        case Operation::Normal:
            return "0";
        case Operation::Idempotent:
            return "2";
        default:
            return "???";
    }
}

void
CodeVisitor::collectClassMembers(const ClassDefPtr& p, MemberInfoList& allMembers, bool inherited)
{
    ClassDefPtr base = p->base();
    if (base)
    {
        collectClassMembers(base, allMembers, true);
    }

    DataMemberList members = p->dataMembers();

    for (const auto& member : members)
    {
        MemberInfo m;
        m.fixedName = fixIdent(member->name());
        m.inherited = inherited;
        m.dataMember = member;
        allMembers.push_back(m);
    }
}

void
CodeVisitor::collectExceptionMembers(const ExceptionPtr& p, MemberInfoList& allMembers, bool inherited)
{
    ExceptionPtr base = p->base();
    if (base)
    {
        collectExceptionMembers(base, allMembers, true);
    }

    DataMemberList members = p->dataMembers();

    for (const auto& member : members)
    {
        MemberInfo m;
        m.fixedName = fixExceptionMember(member->name());
        m.inherited = inherited;
        m.dataMember = member;
        allMembers.push_back(m);
    }
}

CodeVisitor::ParamInfoList
CodeVisitor::getAllInParams(const OperationPtr& op)
{
    const ParameterList l = op->inParameters();
    ParamInfoList r;
    for (const auto& p : l)
    {
        ParamInfo info;
        info.fixedName = fixIdent(p->name());
        info.type = p->type();
        info.optional = p->optional();
        info.tag = p->tag();
        info.param = p;
        r.push_back(info);
    }
    return r;
}

void
CodeVisitor::getInParams(const OperationPtr& op, ParamInfoList& required, ParamInfoList& optional)
{
    const ParamInfoList params = getAllInParams(op);
    for (const auto& param : params)
    {
        if (param.optional)
        {
            optional.push_back(param);
        }
        else
        {
            required.push_back(param);
        }
    }

    //
    // Sort optional parameters by tag.
    //
    class SortFn
    {
    public:
        static bool compare(const ParamInfo& lhs, const ParamInfo& rhs) { return lhs.tag < rhs.tag; }
    };
    optional.sort(SortFn::compare);
}

CodeVisitor::ParamInfoList
CodeVisitor::getAllOutParams(const OperationPtr& op)
{
    ParameterList params = op->outParameters();
    ParamInfoList l;
    int pos = 1;

    if (op->returnType())
    {
        ParamInfo info;
        info.fixedName = "result";
        info.pos = pos++;

        for (const auto& param : params)
        {
            if (param->name() == "result")
            {
                info.fixedName = "result_";
                break;
            }
        }
        info.type = op->returnType();
        info.optional = op->returnIsOptional();
        info.tag = op->returnTag();
        l.push_back(info);
    }

    for (const auto& param : params)
    {
        ParamInfo info;
        info.fixedName = fixIdent(param->name());
        info.type = param->type();
        info.optional = param->optional();
        info.tag = param->tag();
        info.pos = pos++;
        info.param = param;
        l.push_back(info);
    }

    return l;
}

void
CodeVisitor::getOutParams(const OperationPtr& op, ParamInfoList& required, ParamInfoList& optional)
{
    const ParamInfoList params = getAllOutParams(op);
    for (const auto& param : params)
    {
        if (param.optional)
        {
            optional.push_back(param);
        }
        else
        {
            required.push_back(param);
        }
    }

    //
    // Sort optional parameters by tag.
    //
    class SortFn
    {
    public:
        static bool compare(const ParamInfo& lhs, const ParamInfo& rhs) { return lhs.tag < rhs.tag; }
    };
    optional.sort(SortFn::compare);
}

string
CodeVisitor::getOptionalFormat(const TypePtr& type)
{
    return "Ice.OptionalFormat." + type->getOptionalFormat();
}

string
CodeVisitor::getFormatType(FormatType type)
{
    switch (type)
    {
        case CompactFormat:
            return "Ice.FormatType.CompactFormat";
        case SlicedFormat:
            return "Ice.FormatType.SlicedFormat";
        default:
            assert(false);
            return "???";
    }
}

void
CodeVisitor::marshal(
    IceInternal::Output& out,
    const string& stream,
    const string& v,
    const TypePtr& type,
    bool optional,
    int tag)
{
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindByte:
            {
                if (optional)
                {
                    out << nl << stream << ".writeByteOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeByte(" << v << ");";
                }
                return;
            }
            case Builtin::KindBool:
            {
                if (optional)
                {
                    out << nl << stream << ".writeBoolOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeBool(" << v << ");";
                }
                return;
            }
            case Builtin::KindShort:
            {
                if (optional)
                {
                    out << nl << stream << ".writeShortOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeShort(" << v << ");";
                }
                return;
            }
            case Builtin::KindInt:
            {
                if (optional)
                {
                    out << nl << stream << ".writeIntOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeInt(" << v << ");";
                }
                return;
            }
            case Builtin::KindLong:
            {
                if (optional)
                {
                    out << nl << stream << ".writeLongOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeLong(" << v << ");";
                }
                return;
            }
            case Builtin::KindFloat:
            {
                if (optional)
                {
                    out << nl << stream << ".writeFloatOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeFloat(" << v << ");";
                }
                return;
            }
            case Builtin::KindDouble:
            {
                if (optional)
                {
                    out << nl << stream << ".writeDoubleOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeDouble(" << v << ");";
                }
                return;
            }
            case Builtin::KindString:
            {
                if (optional)
                {
                    out << nl << stream << ".writeStringOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeString(" << v << ");";
                }
                return;
            }
            case Builtin::KindObject:
            case Builtin::KindValue:
            {
                // Handled by isClassType below.
                break;
            }
            case Builtin::KindObjectProxy:
            {
                if (optional)
                {
                    out << nl << stream << ".writeProxyOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeProxy(" << v << ");";
                }
                return;
            }
        }
    }

    InterfaceDeclPtr prx = dynamic_pointer_cast<InterfaceDecl>(type);
    if (prx)
    {
        if (optional)
        {
            out << nl << stream << ".writeProxyOpt(" << tag << ", " << v << ");";
        }
        else
        {
            out << nl << stream << ".writeProxy(" << v << ");";
        }
        return;
    }

    if (type->isClassType())
    {
        assert(!optional); // Optional classes are disallowed by the parser.
        out << nl << stream << ".writeValue(" << v << ");";
        return;
    }

    StructPtr st = dynamic_pointer_cast<Struct>(type);
    if (st)
    {
        const string typeS = getAbsolute(st);
        if (optional)
        {
            out << nl << typeS << ".ice_writeOpt(" << stream << ", " << tag << ", " << v << ");";
        }
        else
        {
            out << nl << typeS << ".ice_write(" << stream << ", " << v << ");";
        }
        return;
    }

    EnumPtr en = dynamic_pointer_cast<Enum>(type);
    if (en)
    {
        const string typeS = getAbsolute(en);
        if (optional)
        {
            out << nl << typeS << ".ice_writeOpt(" << stream << ", " << tag << ", " << v << ");";
        }
        else
        {
            out << nl << typeS << ".ice_write(" << stream << ", " << v << ");";
        }
        return;
    }

    DictionaryPtr dict = dynamic_pointer_cast<Dictionary>(type);
    if (dict)
    {
        if (optional)
        {
            out << nl << getAbsolute(dict) << ".writeOpt(" << stream << ", " << tag << ", " << v << ");";
        }
        else
        {
            out << nl << getAbsolute(dict) << ".write(" << stream << ", " << v << ");";
        }
        return;
    }

    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    if (seq)
    {
        const TypePtr content = seq->type();
        const BuiltinPtr b = dynamic_pointer_cast<Builtin>(content);

        if (b && b->kind() != Builtin::KindObject && b->kind() != Builtin::KindObjectProxy &&
            b->kind() != Builtin::KindValue)
        {
            static const char* builtinTable[] =
                {"Byte", "Bool", "Short", "Int", "Long", "Float", "Double", "String", "???", "???", "???", "???"};
            string bs = builtinTable[b->kind()];
            out << nl << stream << ".write" << builtinTable[b->kind()] << "Seq";
            if (optional)
            {
                out << "Opt(" << tag << ", ";
            }
            else
            {
                out << "(";
            }
            out << v << ");";
            return;
        }

        if (optional)
        {
            out << nl << getAbsolute(seq) << ".writeOpt(" << stream << ", " << tag << ", " << v << ");";
        }
        else
        {
            out << nl << getAbsolute(seq) << ".write(" << stream << ", " << v << ");";
        }
        return;
    }

    assert(false);
}

void
CodeVisitor::unmarshal(
    IceInternal::Output& out,
    const string& stream,
    const string& v,
    const TypePtr& type,
    bool optional,
    int tag)
{
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindByte:
            {
                if (optional)
                {
                    out << nl << v << " = " << stream << ".readByteOpt(" << tag << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readByte();";
                }
                break;
            }
            case Builtin::KindBool:
            {
                if (optional)
                {
                    out << nl << v << " = " << stream << ".readBoolOpt(" << tag << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readBool();";
                }
                break;
            }
            case Builtin::KindShort:
            {
                if (optional)
                {
                    out << nl << v << " = " << stream << ".readShortOpt(" << tag << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readShort();";
                }
                break;
            }
            case Builtin::KindInt:
            {
                if (optional)
                {
                    out << nl << v << " = " << stream << ".readIntOpt(" << tag << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readInt();";
                }
                break;
            }
            case Builtin::KindLong:
            {
                if (optional)
                {
                    out << nl << v << " = " << stream << ".readLongOpt(" << tag << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readLong();";
                }
                break;
            }
            case Builtin::KindFloat:
            {
                if (optional)
                {
                    out << nl << v << " = " << stream << ".readFloatOpt(" << tag << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readFloat();";
                }
                break;
            }
            case Builtin::KindDouble:
            {
                if (optional)
                {
                    out << nl << v << " = " << stream << ".readDoubleOpt(" << tag << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readDouble();";
                }
                break;
            }
            case Builtin::KindString:
            {
                if (optional)
                {
                    out << nl << v << " = " << stream << ".readStringOpt(" << tag << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readString();";
                }
                break;
            }
            case Builtin::KindObject:
            case Builtin::KindValue:
            {
                assert(!optional); // Optional classes are disallowed by the parser.
                out << nl << stream << ".readValue(" << v << ", 'Ice.Value');";
                break;
            }
            case Builtin::KindObjectProxy:
            {
                if (optional)
                {
                    out << nl << v << " = " << stream << ".readProxyOpt(" << tag << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readProxy();";
                }
                break;
            }
        }
        return;
    }

    InterfaceDeclPtr prx = dynamic_pointer_cast<InterfaceDecl>(type);
    if (prx)
    {
        const string typeS = getAbsolute(prx, "", "Prx");
        if (optional)
        {
            out << nl << v << " = " << stream << ".readProxyOpt(" << tag << ", '" << typeS << "');";
        }
        else
        {
            out << nl << v << " = " << typeS << ".ice_read(" << stream << ");";
        }

        return;
    }

    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
    if (cl)
    {
        assert(!optional); // Optional classes are disallowed by the parser.
        const string cls = getAbsolute(cl);
        out << nl << stream << ".readValue(" << v << ", '" << cls << "');";
        return;
    }

    StructPtr st = dynamic_pointer_cast<Struct>(type);
    if (st)
    {
        const string typeS = getAbsolute(st);
        if (optional)
        {
            out << nl << v << " = " << typeS << ".ice_readOpt(" << stream << ", " << tag << ");";
        }
        else
        {
            out << nl << v << " = " << typeS << ".ice_read(" << stream << ");";
        }
        return;
    }

    EnumPtr en = dynamic_pointer_cast<Enum>(type);
    if (en)
    {
        const string typeS = getAbsolute(en);
        if (optional)
        {
            out << nl << v << " = " << typeS << ".ice_readOpt(" << stream << ", " << tag << ");";
        }
        else
        {
            out << nl << v << " = " << typeS << ".ice_read(" << stream << ");";
        }
        return;
    }

    DictionaryPtr dict = dynamic_pointer_cast<Dictionary>(type);
    if (dict)
    {
        if (optional)
        {
            out << nl << v << " = " << getAbsolute(dict) << ".readOpt(" << stream << ", " << tag << ");";
        }
        else
        {
            out << nl << v << " = " << getAbsolute(dict) << ".read(" << stream << ");";
        }
        return;
    }

    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    if (seq)
    {
        const TypePtr content = seq->type();
        const BuiltinPtr b = dynamic_pointer_cast<Builtin>(content);

        if (b && b->kind() != Builtin::KindObject && b->kind() != Builtin::KindObjectProxy &&
            b->kind() != Builtin::KindValue)
        {
            static const char* builtinTable[] =
                {"Byte", "Bool", "Short", "Int", "Long", "Float", "Double", "String", "???", "???", "???", "???"};
            string bs = builtinTable[b->kind()];
            out << nl << v << " = " << stream << ".read" << builtinTable[b->kind()] << "Seq";
            if (optional)
            {
                out << "Opt(" << tag << ");";
            }
            else
            {
                out << "();";
            }
            return;
        }

        if (optional)
        {
            out << nl << v << " = " << getAbsolute(seq) << ".readOpt(" << stream << ", " << tag << ");";
        }
        else
        {
            out << nl << v << " = " << getAbsolute(seq) << ".read(" << stream << ");";
        }
        return;
    }

    assert(false);
}

void
CodeVisitor::unmarshalStruct(IceInternal::Output& out, const StructPtr& p, const string& v)
{
    for (const auto& dm : p->dataMembers())
    {
        string m = fixStructMember(dm->name());
        if (dm->type()->isClassType())
        {
            out << nl << m << "_ = IceInternal.ValueHolder();";
            out << nl << v << "." << m << " = " << m << "_;";
            unmarshal(out, "is", "@(v_) " + m + "_.set(v_)", dm->type(), false, 0);
        }
        else
        {
            unmarshal(out, "is", v + "." + m, dm->type(), false, 0);
        }
    }
}

void
CodeVisitor::convertStruct(IceInternal::Output& out, const StructPtr& p, const string& v)
{
    for (const auto& dm : p->dataMembers())
    {
        string m = fixStructMember(dm->name());
        if (needsConversion(dm->type()))
        {
            convertValueType(out, v + "." + m, v + "." + m, dm->type(), false);
        }
        else if (dm->type()->isClassType())
        {
            out << nl << v << "." << m << " = " << v << "." << m << ".value;";
        }
    }
}

void
CodeVisitor::writeBaseClassArrayParams(IceInternal::Output& out, const MemberInfoList& members, bool noInit)
{
    out << nl << "v = { ";
    bool first = true;
    for (const auto& member : members)
    {
        if (member.inherited)
        {
            if (first)
            {
                out << (noInit ? "IceInternal.NoInit.Instance" : member.fixedName);
                first = false;
            }
            else
            {
                out << ", " << (noInit ? "[]" : member.fixedName);
            }
        }
    }
    out << " };";
}

namespace
{
    mutex globalMutex;
    bool interrupted = false;
}

static void
interruptedCallback(int /*signal*/)
{
    lock_guard lock(globalMutex);
    interrupted = true;
}

static void
usage(const string& n)
{
    consoleErr << "Usage: " << n << " [options] slice-files...\n";
    consoleErr << "Options:\n"
                  "-h, --help               Show this message.\n"
                  "-v, --version            Display the Ice version.\n"
                  "-DNAME                   Define NAME as 1.\n"
                  "-DNAME=DEF               Define NAME as DEF.\n"
                  "-UNAME                   Remove any definition for NAME.\n"
                  "-IDIR                    Put DIR in the include file search path.\n"
                  "-E                       Print preprocessor output on stdout.\n"
                  "--output-dir DIR         Create files in the directory DIR.\n"
                  "-d, --debug              Print debug messages.\n"
                  "--depend                 Generate Makefile dependencies.\n"
                  "--depend-xml             Generate dependencies in XML format.\n"
                  "--depend-file FILE       Write dependencies to FILE instead of standard output.\n"
                  "--validate               Validate command line options.\n"
                  "--all                    Generate code for Slice definitions in included files.\n"
                  "--list-generated         Emit list of generated files in XML format.\n";
}

int
compile(const vector<string>& argv)
{
    IceInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("", "validate");
    opts.addOpt("D", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
    opts.addOpt("U", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
    opts.addOpt("I", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
    opts.addOpt("E");
    opts.addOpt("", "output-dir", IceInternal::Options::NeedArg);
    opts.addOpt("", "depend");
    opts.addOpt("", "depend-xml");
    opts.addOpt("", "depend-file", IceInternal::Options::NeedArg, "");
    opts.addOpt("", "list-generated");
    opts.addOpt("d", "debug");
    opts.addOpt("", "all");

    bool validate = find(argv.begin(), argv.end(), "--validate") != argv.end();

    vector<string> args;
    try
    {
        args = opts.parse(argv);
    }
    catch (const IceInternal::BadOptException& e)
    {
        consoleErr << argv[0] << ": error: " << e.what() << endl;
        if (!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if (opts.isSet("help"))
    {
        usage(argv[0]);
        return EXIT_SUCCESS;
    }

    if (opts.isSet("version"))
    {
        consoleErr << ICE_STRING_VERSION << endl;
        return EXIT_SUCCESS;
    }

    vector<string> cppArgs;
    vector<string> optargs = opts.argVec("D");
    for (const auto& arg : optargs)
    {
        cppArgs.push_back("-D" + arg);
    }

    optargs = opts.argVec("U");
    for (const auto& arg : optargs)
    {
        cppArgs.push_back("-U" + arg);
    }

    vector<string> includePaths = opts.argVec("I");
    for (const auto& includePath : includePaths)
    {
        cppArgs.push_back("-I" + Preprocessor::normalizeIncludePath(includePath));
    }

    bool preprocess = opts.isSet("E");

    string output = opts.optArg("output-dir");

    bool depend = opts.isSet("depend");

    bool dependxml = opts.isSet("depend-xml");

    string dependFile = opts.optArg("depend-file");

    bool debug = opts.isSet("debug");

    bool all = opts.isSet("all");

    bool listGenerated = opts.isSet("list-generated");

    if (args.empty())
    {
        consoleErr << argv[0] << ": error: no input file" << endl;
        if (!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if (depend && dependxml)
    {
        consoleErr << argv[0] << ": error: cannot specify both --depend and --depend-xml" << endl;
        if (!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if (validate)
    {
        return EXIT_SUCCESS;
    }

    int status = EXIT_SUCCESS;

    Ice::CtrlCHandler ctrlCHandler;
    ctrlCHandler.setCallback(interruptedCallback);

    ostringstream os;
    if (dependxml)
    {
        os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dependencies>" << endl;
    }

    for (auto i = args.begin(); i != args.end(); ++i)
    {
        //
        // Ignore duplicates.
        //
        auto p = find(args.begin(), args.end(), *i);
        if (p != i)
        {
            continue;
        }

        if (depend || dependxml)
        {
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
            FILE* cppHandle = icecpp->preprocess(false, "-D__SLICE2MATLAB__");

            if (cppHandle == nullptr)
            {
                return EXIT_FAILURE;
            }

            UnitPtr u = Unit::createUnit(false);
            int parseStatus = u->parse(*i, cppHandle, debug);
            u->destroy();

            if (parseStatus == EXIT_FAILURE)
            {
                return EXIT_FAILURE;
            }

            if (!icecpp->printMakefileDependencies(
                    os,
                    depend ? Preprocessor::MATLAB : Preprocessor::SliceXML,
                    includePaths,
                    "-D__SLICE2MATLAB__"))
            {
                return EXIT_FAILURE;
            }

            if (!icecpp->close())
            {
                return EXIT_FAILURE;
            }
        }
        else
        {
            FileTracker::instance()->setSource(*i);

            PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
            FILE* cppHandle = icecpp->preprocess(true, "-D__SLICE2MATLAB__");

            if (cppHandle == nullptr)
            {
                return EXIT_FAILURE;
            }

            if (preprocess)
            {
                char buf[4096];
                while (fgets(buf, static_cast<int>(sizeof(buf)), cppHandle) != nullptr)
                {
                    if (fputs(buf, stdout) == EOF)
                    {
                        return EXIT_FAILURE;
                    }
                }
                if (!icecpp->close())
                {
                    return EXIT_FAILURE;
                }
            }
            else
            {
                UnitPtr u = Unit::createUnit(all);
                int parseStatus = u->parse(*i, cppHandle, debug);

                if (!icecpp->close())
                {
                    u->destroy();
                    return EXIT_FAILURE;
                }

                if (parseStatus == EXIT_FAILURE)
                {
                    status = EXIT_FAILURE;
                }
                else
                {
                    string base = icecpp->getBaseName();
                    string::size_type pos = base.find_last_of("/\\");
                    if (pos != string::npos)
                    {
                        base.erase(0, pos + 1);
                    }

                    try
                    {
                        // 'slice2matlab' doesn't have any language-specific metadata, so we call `validateMetadata`
                        // with an empty list.  This ensures that the validation still runs, and will reject any
                        // 'matlab' metadata the user might think exists.
                        Slice::validateMetadata(u, "matlab", {});

                        CodeVisitor codeVisitor(output);
                        u->visit(&codeVisitor);
                    }
                    catch (const Slice::FileException& ex)
                    {
                        //
                        // If a file could not be created, then cleanup any created files.
                        //
                        FileTracker::instance()->cleanup();
                        u->destroy();
                        consoleErr << argv[0] << ": error: " << ex.what() << endl;
                        status = EXIT_FAILURE;
                        FileTracker::instance()->error();
                        break;
                    }
                }

                u->destroy();
            }
        }

        {
            lock_guard lock(globalMutex);
            if (interrupted)
            {
                FileTracker::instance()->cleanup();
                return EXIT_FAILURE;
            }
        }
    }

    if (dependxml)
    {
        os << "</dependencies>\n";
    }

    if (depend || dependxml)
    {
        writeDependencies(os.str(), dependFile);
    }

    if (listGenerated)
    {
        FileTracker::instance()->dumpxml();
    }

    return status;
}

#ifdef _WIN32
int
wmain(int argc, wchar_t* argv[])
#else
int
main(int argc, char* argv[])
#endif
{
    vector<string> args = Slice::argvToArgs(argc, argv);
    try
    {
        return compile(args);
    }
    catch (const std::exception& ex)
    {
        consoleErr << args[0] << ": error:" << ex.what() << endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        consoleErr << args[0] << ": error:"
                   << "unknown exception" << endl;
        return EXIT_FAILURE;
    }
}
