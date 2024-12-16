//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Gen.h"
#include "../Ice/FileUtil.h"
#include "Ice/StringUtil.h"

#include <limits>
#ifndef _WIN32
#    include <unistd.h>
#else
#    include <direct.h>
#endif

#include "../Slice/FileTracker.h"
#include "../Slice/Util.h"
#include "DotNetNames.h"
#include "Ice/UUID.h"
#include <algorithm>
#include <cassert>
#include <iterator>
#include <string.h>

using namespace std;
using namespace Slice;
using namespace Ice;
using namespace IceInternal;

namespace
{
    string sliceModeToIceMode(Operation::Mode opMode)
    {
        string mode;
        switch (opMode)
        {
            case Operation::Normal:
            {
                mode = "Ice.OperationMode.Normal";
                break;
            }
            case Operation::Idempotent:
            {
                mode = "Ice.OperationMode.Idempotent";
                break;
            }
            default:
            {
                assert(false);
                break;
            }
        }
        return mode;
    }

    string opFormatTypeToString(const OperationPtr& op)
    {
        optional<FormatType> opFormat = op->format();
        if (opFormat)
        {
            switch (*opFormat)
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
        else
        {
            return "null";
        }
    }

    void emitObsoleteAttribute(const ContainedPtr& p1, Output& out)
    {
        if (p1->isDeprecated())
        {
            if (auto reason = p1->getDeprecationReason())
            {
                out << nl << "[global::System.Obsolete(\"" << *reason << "\")]";
            }
            else
            {
                out << nl << "[global::System.Obsolete]";
            }
        }
    }

    string getEscapedParamName(const OperationPtr& p, const string& name)
    {
        ParameterList params = p->parameters();

        for (ParameterList::const_iterator i = params.begin(); i != params.end(); ++i)
        {
            if ((*i)->name() == name)
            {
                return name + "_";
            }
        }
        return name;
    }

    string resultStructReturnValueName(const ParameterList& outParams)
    {
        for (ParameterList::const_iterator i = outParams.begin(); i != outParams.end(); ++i)
        {
            if ((*i)->name() == "returnValue")
            {
                return "returnValue_";
            }
        }
        return "returnValue";
    }
}

Slice::CsVisitor::CsVisitor(Output& out) : _out(out) {}

Slice::CsVisitor::~CsVisitor() {}

void
Slice::CsVisitor::writeMarshalUnmarshalParams(
    const ParameterList& params,
    const OperationPtr& op,
    bool marshal,
    const string& ns,
    bool resultStruct,
    bool publicNames,
    const string& customStream)
{
    ParameterList optionals;

    string paramPrefix = "";
    string returnValueS = "ret";

    if (op && resultStruct)
    {
        if ((op->returnType() && !params.empty()) || params.size() > 1)
        {
            paramPrefix = "ret.";
            returnValueS = resultStructReturnValueName(params);
        }
    }

    for (ParameterList::const_iterator pli = params.begin(); pli != params.end(); ++pli)
    {
        string param = paramPrefix.empty() && !publicNames ? "iceP_" + (*pli)->name() : fixId((*pli)->name());
        TypePtr type = (*pli)->type();
        if (!marshal && type->isClassType())
        {
            ostringstream os;
            os << '(' << typeToString(type, ns) << " v) => { " << paramPrefix << param << " = v; }";
            param = os.str();
        }
        else
        {
            param = paramPrefix + param;
        }

        if ((*pli)->optional())
        {
            optionals.push_back(*pli);
        }
        else
        {
            writeMarshalUnmarshalCode(_out, type, ns, param, marshal, customStream);
        }
    }

    TypePtr ret;

    if (op && op->returnType())
    {
        ret = op->returnType();
        string param;
        if (!marshal && ret->isClassType())
        {
            ostringstream os;
            os << '(' << typeToString(ret, ns) << " v) => {" << paramPrefix << returnValueS << " = v; }";
            param = os.str();
        }
        else
        {
            param = paramPrefix + returnValueS;
        }

        if (!op->returnIsOptional())
        {
            writeMarshalUnmarshalCode(_out, ret, ns, param, marshal, customStream);
        }
    }

    //
    // Sort optional parameters by tag.
    //
    class SortFn
    {
    public:
        static bool compare(const ParameterPtr& lhs, const ParameterPtr& rhs) { return lhs->tag() < rhs->tag(); }
    };
    optionals.sort(SortFn::compare);

    //
    // Handle optional parameters.
    //
    bool checkReturnType = op && op->returnIsOptional();
    for (ParameterList::const_iterator pli = optionals.begin(); pli != optionals.end(); ++pli)
    {
        if (checkReturnType && op->returnTag() < (*pli)->tag())
        {
            string param;
            if (!marshal && ret->isClassType())
            {
                ostringstream os;
                os << '(' << typeToString(ret, ns) << " v) => {" << paramPrefix << returnValueS << " = v; }";
                param = os.str();
            }
            else
            {
                param = paramPrefix + returnValueS;
            }
            writeOptionalMarshalUnmarshalCode(_out, ret, ns, param, op->returnTag(), marshal, customStream);
            checkReturnType = false;
        }

        string param = paramPrefix.empty() && !publicNames ? "iceP_" + (*pli)->name() : fixId((*pli)->name());
        TypePtr type = (*pli)->type();
        if (!marshal && type->isClassType())
        {
            ostringstream os;
            os << '(' << typeToString(type, ns) << " v) => {" << paramPrefix << param << " = v; }";
            param = os.str();
        }
        else
        {
            param = paramPrefix + param;
        }

        writeOptionalMarshalUnmarshalCode(_out, type, ns, param, (*pli)->tag(), marshal, customStream);
    }

    if (checkReturnType)
    {
        string param;
        if (!marshal && ret->isClassType())
        {
            ostringstream os;
            os << '(' << typeToString(ret, ns) << " v) => {" << paramPrefix << returnValueS << " = v; }";
            param = os.str();
        }
        else
        {
            param = paramPrefix + returnValueS;
        }
        writeOptionalMarshalUnmarshalCode(_out, ret, ns, param, op->returnTag(), marshal, customStream);
    }
}

void
Slice::CsVisitor::writeMarshalDataMember(
    const DataMemberPtr& member,
    const string& name,
    const string& ns,
    bool forStruct)
{
    if (member->optional())
    {
        assert(!forStruct);
        writeOptionalMarshalUnmarshalCode(_out, member->type(), ns, name, member->tag(), true, "ostr_");
    }
    else
    {
        string stream = forStruct ? "" : "ostr_";
        string memberName = name;
        if (forStruct)
        {
            memberName = "this." + memberName;
        }

        writeMarshalUnmarshalCode(_out, member->type(), ns, memberName, true, stream);
    }
}

void
Slice::CsVisitor::writeUnmarshalDataMember(
    const DataMemberPtr& member,
    const string& name,
    const string& ns,
    bool forStruct)
{
    string param = name;
    if (member->type()->isClassType())
    {
        ostringstream os;
        os << '(' << typeToString(member->type(), ns) << " v) => { this." << name << " = v; }";
        param = os.str();
    }
    else if (forStruct)
    {
        param = "this." + name;
    }

    if (member->optional())
    {
        assert(!forStruct);
        writeOptionalMarshalUnmarshalCode(_out, member->type(), ns, param, member->tag(), false, "istr_");
    }
    else
    {
        writeMarshalUnmarshalCode(_out, member->type(), ns, param, false, forStruct ? "" : "istr_");
    }
}

void
Slice::CsVisitor::writeInheritedOperations(const InterfaceDefPtr& p)
{
    InterfaceList bases = p->bases();
    if (!bases.empty())
    {
        OperationList allOps;
        for (InterfaceList::const_iterator q = bases.begin(); q != bases.end(); ++q)
        {
            OperationList tmp = (*q)->allOperations();
            allOps.splice(allOps.end(), tmp);
        }
        allOps.sort();
        allOps.unique();
        for (OperationList::const_iterator i = allOps.begin(); i != allOps.end(); ++i)
        {
            string retS;
            vector<string> params, args;
            string ns = getNamespace(p);
            string name = getDispatchParams(*i, retS, params, args, ns);
            _out << sp << nl << "public abstract " << retS << " " << name << spar << params << epar << ';';
        }
    }
}

void
Slice::CsVisitor::writeMarshaling(const ClassDefPtr& p)
{
    string name = fixId(p->name());
    string scoped = p->scoped();
    string ns = getNamespace(p);
    ClassDefPtr base = p->base();

    //
    // Marshaling support
    //
    DataMemberList allClassMembers = p->allClassDataMembers();
    DataMemberList members = p->dataMembers();
    DataMemberList optionalMembers = p->orderedOptionalDataMembers();
    DataMemberList classMembers = p->classDataMembers();

    _out << sp;
    _out << nl << "protected override void iceWriteImpl(Ice.OutputStream ostr_)";
    _out << sb;
    _out << nl << "ostr_.startSlice(ice_staticId(), " << p->compactId() << (!base ? ", true" : ", false") << ");";
    for (DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
    {
        if (!(*d)->optional())
        {
            writeMarshalDataMember(*d, fixId((*d)->name(), DotNet::ICloneable, true), ns);
        }
    }
    for (DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
    {
        writeMarshalDataMember(*d, fixId((*d)->name(), DotNet::ICloneable, true), ns);
    }
    _out << nl << "ostr_.endSlice();";
    if (base)
    {
        _out << nl << "base.iceWriteImpl(ostr_);";
    }
    _out << eb;

    _out << sp;
    _out << nl << "protected override void iceReadImpl(Ice.InputStream istr_)";
    _out << sb;
    _out << nl << "istr_.startSlice();";
    for (DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
    {
        if (!(*d)->optional())
        {
            writeUnmarshalDataMember(*d, fixId((*d)->name(), DotNet::ICloneable, true), ns);
        }
    }
    for (DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
    {
        writeUnmarshalDataMember(*d, fixId((*d)->name(), DotNet::ICloneable, true), ns);
    }
    _out << nl << "istr_.endSlice();";
    if (base)
    {
        _out << nl << "base.iceReadImpl(istr_);";
    }
    _out << eb;
}

string
Slice::CsVisitor::getParamAttributes(const ParameterPtr& p)
{
    ostringstream ostr;
    for (const auto& metadata : p->getMetadata())
    {
        if (metadata->directive() == "cs:attribute")
        {
            ostr << "[" << metadata->arguments() << "] ";
        }
    }
    return ostr.str();
}

vector<string>
Slice::CsVisitor::getParams(const OperationPtr& op, const string& ns)
{
    vector<string> params;
    ParameterList paramList = op->parameters();
    InterfaceDefPtr interface = op->interface();
    for (ParameterList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        string param = getParamAttributes(*q);
        if ((*q)->isOutParam())
        {
            param += "out ";
        }
        param += typeToString((*q)->type(), ns, (*q)->optional()) + " " + fixId((*q)->name());
        params.push_back(param);
    }
    return params;
}

vector<string>
Slice::CsVisitor::getInParams(const OperationPtr& op, const string& ns, bool internal)
{
    vector<string> params;

    string name = fixId(op->name());
    InterfaceDefPtr interface = op->interface();
    ParameterList paramList = op->inParameters();
    for (ParameterList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        params.push_back(
            getParamAttributes(*q) + typeToString((*q)->type(), ns, (*q)->optional()) + " " +
            (internal ? "iceP_" + (*q)->name() : fixId((*q)->name())));
    }
    return params;
}

vector<string>
Slice::CsVisitor::getOutParams(const OperationPtr& op, const string& ns, bool returnParam, bool outKeyword)
{
    vector<string> params;
    if (returnParam)
    {
        TypePtr ret = op->returnType();
        if (ret)
        {
            params.push_back(typeToString(ret, ns, op->returnIsOptional()) + " ret");
        }
    }

    ParameterList paramList = op->outParameters();
    for (ParameterList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        string s = getParamAttributes(*q);
        if (outKeyword)
        {
            s += "out ";
        }
        s += typeToString((*q)->type(), ns, (*q)->optional()) + ' ' + fixId((*q)->name());
        params.push_back(s);
    }

    return params;
}

vector<string>
Slice::CsVisitor::getArgs(const OperationPtr& op)
{
    vector<string> args;
    ParameterList paramList = op->parameters();
    for (ParameterList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        string arg = fixId((*q)->name());
        if ((*q)->isOutParam())
        {
            arg = "out " + arg;
        }
        args.push_back(arg);
    }
    return args;
}

vector<string>
Slice::CsVisitor::getInArgs(const OperationPtr& op, bool internal)
{
    vector<string> args;
    ParameterList paramList = op->parameters();
    for (ParameterList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if (!(*q)->isOutParam())
        {
            args.push_back(internal ? "iceP_" + (*q)->name() : fixId((*q)->name()));
        }
    }
    return args;
}

string
Slice::CsVisitor::getDispatchParams(
    const OperationPtr& op,
    string& retS,
    vector<string>& params,
    vector<string>& args,
    const string& ns)
{
    string name;
    InterfaceDefPtr interface = op->interface();
    ParameterList parameterss;

    if (interface->hasMetadata("amd") || op->hasMetadata("amd"))
    {
        name = op->name() + "Async";
        params = getInParams(op, ns);
        args = getInArgs(op);
        parameterss = op->inParameters();
        retS = taskResultType(op, ns, true);
    }
    else if (op->hasMarshaledResult())
    {
        name = fixId(op->name(), DotNet::Object, true);
        params = getInParams(op, ns);
        args = getInArgs(op);
        parameterss = op->inParameters();
        retS = resultType(op, ns, true);
    }
    else
    {
        name = fixId(op->name(), DotNet::Object, true);
        params = getParams(op, ns);
        args = getArgs(op);
        parameterss = op->parameters();
        retS = typeToString(op->returnType(), ns, op->returnIsOptional());
    }

    string currentParamName = getEscapedParamName(op, "current");
    params.push_back("Ice.Current " + currentParamName);
    args.push_back(currentParamName);
    return name;
}

void
Slice::CsVisitor::emitAttributes(const ContainedPtr& p)
{
    for (const auto& metadata : p->getMetadata())
    {
        if (metadata->directive() == "cs:attribute")
        {
            _out << nl << '[' << metadata->arguments() << ']';
        }
    }
}

void
Slice::CsVisitor::emitNonBrowsableAttribute()
{
    _out << nl
         << "[global::System.ComponentModel.EditorBrowsable(global::System.ComponentModel.EditorBrowsableState.Never)]";
}

string
Slice::CsVisitor::writeValue(const TypePtr& type, const string& ns)
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
            case Builtin::KindLong:
            {
                return "0";
                break;
            }
            case Builtin::KindFloat:
            {
                return "0.0f";
                break;
            }
            case Builtin::KindDouble:
            {
                return "0.0";
                break;
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
        return typeToString(type, ns) + "." + fixId((*en->enumerators().begin())->name());
    }

    StructPtr st = dynamic_pointer_cast<Struct>(type);
    if (st && !isMappedToClass(st))
    {
        return "default";
    }

    return "null";
}

void
Slice::CsVisitor::writeConstantValue(const TypePtr& type, const SyntaxTreeBasePtr& valueType, const string& value)
{
    ConstPtr constant = dynamic_pointer_cast<Const>(valueType);
    if (constant)
    {
        _out << fixId(constant->scoped()) << ".value";
    }
    else
    {
        BuiltinPtr bp = dynamic_pointer_cast<Builtin>(type);
        if (bp && bp->kind() == Builtin::KindString)
        {
            _out << "\"" << toStringLiteral(value, "\a\b\f\n\r\t\v\0", "", UCN, 0) << "\"";
        }
        else if (bp && bp->kind() == Builtin::KindLong)
        {
            _out << value << "L";
        }
        else if (bp && bp->kind() == Builtin::KindFloat)
        {
            _out << value << "F";
        }
        else if (dynamic_pointer_cast<Enum>(type))
        {
            EnumeratorPtr lte = dynamic_pointer_cast<Enumerator>(valueType);
            assert(lte);
            _out << fixId(lte->scoped());
        }
        else
        {
            _out << value;
        }
    }
}

void
Slice::CsVisitor::writeDataMemberInitializers(const DataMemberList& dataMembers, unsigned int baseTypes)
{
    // Generates "= null!" for each required field. This wouldn't be necessary if we actually generated required
    // fields and properties.
    for (const auto& q : dataMembers)
    {
        if (isMappedToRequiredField(q))
        {
            _out << nl << "this." << fixId(q->name(), baseTypes) << " = null!;";
        }
    }
}

string
Slice::CsVisitor::toCsIdent(const string& s)
{
    string::size_type pos = s.find('#');
    if (pos == string::npos)
    {
        return s;
    }

    string result = s;
    if (pos == 0)
    {
        return result.erase(0, 1);
    }

    result[pos] = '.';
    return result;
}

string
Slice::CsVisitor::editMarkup(const string& s)
{
    //
    // Strip HTML markup and javadoc links--VS doesn't display them.
    //
    string result = s;
    string::size_type pos = 0;
    do
    {
        pos = result.find('<', pos);
        if (pos != string::npos)
        {
            string::size_type endpos = result.find('>', pos);
            string::size_type wspos = result.find(' ', pos);
            if (endpos == string::npos && wspos == string::npos)
            {
                break;
            }

            if (wspos < endpos)
            {
                // If we found a whitespace before the end tag marker '>', The '<' char doesn't correspond to the start
                // of a HTML tag, and we replace it with &lt; escape code.
                result.replace(pos, 1, "&lt;", 4);
            }
            else
            {
                result.erase(pos, endpos - pos + 1);
            }
        }
    } while (pos != string::npos);

    // replace remaining '>' chars with '&gt;' escape code, tags have been already strip above.
    pos = 0;
    do
    {
        pos = result.find('>', pos);
        if (pos != string::npos)
        {
            result.replace(pos, 1, "&gt;", 4);
        }
    } while (pos != string::npos);

    const string link = "{@link";
    pos = 0;
    do
    {
        pos = result.find(link, pos);
        if (pos != string::npos)
        {
            result.erase(pos, link.size() + 1); // erase following white space too
            string::size_type endpos = result.find('}', pos);
            if (endpos != string::npos)
            {
                string ident = result.substr(pos, endpos - pos);
                result.erase(pos, endpos - pos + 1);
                result.insert(pos, toCsIdent(ident));
            }
        }
    } while (pos != string::npos);

    //
    // Strip @see sections because VS does not display them.
    //
    static const string seeTag = "@see";
    pos = 0;
    do
    {
        //
        // Look for the next @ and delete up to that, or
        // to the end of the string, if not found.
        //
        pos = result.find(seeTag, pos);
        if (pos != string::npos)
        {
            string::size_type next = result.find('@', pos + seeTag.size());
            if (next != string::npos)
            {
                result.erase(pos, next - pos);
            }
            else
            {
                result.erase(pos, string::npos);
            }
        }
    } while (pos != string::npos);

    //
    // Replace @param, @return, and @throws with corresponding <param>, <returns>, and <exception> tags.
    //
    static const string paramTag = "@param";
    pos = 0;
    do
    {
        pos = result.find(paramTag, pos);
        if (pos != string::npos)
        {
            result.erase(pos, paramTag.size() + 1);

            string::size_type startIdent = result.find_first_not_of(" \t", pos);
            if (startIdent != string::npos)
            {
                string::size_type endIdent = result.find_first_of(" \t", startIdent);
                if (endIdent != string::npos)
                {
                    string ident = result.substr(startIdent, endIdent - startIdent);
                    string::size_type endComment = result.find_first_of("@<", endIdent);
                    string comment = result.substr(
                        endIdent + 1,
                        endComment == string::npos ? endComment : endComment - endIdent - 1);
                    result.erase(startIdent, endComment == string::npos ? string::npos : endComment - startIdent);
                    string newComment = "<param name=\"" + ident + "\">" + comment + "</param>\n";
                    result.insert(startIdent, newComment);
                    pos = startIdent + newComment.size();
                }
            }
            else
            {
                pos += paramTag.size();
            }
        }
    } while (pos != string::npos);

    static const string returnTag = "@return";
    pos = result.find(returnTag);
    if (pos != string::npos)
    {
        result.erase(pos, returnTag.size() + 1);
        string::size_type endComment = result.find_first_of("@<", pos);
        string comment = result.substr(pos, endComment == string::npos ? endComment : endComment - pos);
        result.erase(pos, endComment == string::npos ? string::npos : endComment - pos);
        string newComment = "<returns>" + comment + "</returns>\n";
        result.insert(pos, newComment);
        pos = pos + newComment.size();
    }

    static const string throwsTag = "@throws";
    pos = 0;
    do
    {
        pos = result.find(throwsTag, pos);
        if (pos != string::npos)
        {
            result.erase(pos, throwsTag.size() + 1);

            string::size_type startIdent = result.find_first_not_of(" \t", pos);
            if (startIdent != string::npos)
            {
                string::size_type endIdent = result.find_first_of(" \t", startIdent);
                if (endIdent != string::npos)
                {
                    string ident = result.substr(startIdent, endIdent - startIdent);
                    string::size_type endComment = result.find_first_of("@<", endIdent);
                    string comment = result.substr(
                        endIdent + 1,
                        endComment == string::npos ? endComment : endComment - endIdent - 1);
                    result.erase(startIdent, endComment == string::npos ? string::npos : endComment - startIdent);
                    string newComment = "<exception name=\"" + ident + "\">" + comment + "</exception>\n";
                    result.insert(startIdent, newComment);
                    pos = startIdent + newComment.size();
                }
            }
            else
            {
                pos += throwsTag.size();
            }
        }
    } while (pos != string::npos);

    return result;
}

StringList
Slice::CsVisitor::splitIntoLines(const string& comment)
{
    string s = editMarkup(comment);
    StringList result;
    string::size_type pos = 0;
    string::size_type nextPos;
    while ((nextPos = s.find_first_of('\n', pos)) != string::npos)
    {
        result.push_back(string(s, pos, nextPos - pos));
        pos = nextPos + 1;
    }
    string lastLine = string(s, pos);
    if (lastLine.find_first_not_of(" \t\n\r") != string::npos)
    {
        result.push_back(lastLine);
    }
    return result;
}

void
Slice::CsVisitor::splitDocComment(const ContainedPtr& p, StringList& summaryLines, StringList& remarksLines)
{
    string s = p->docComment();

    const string paramTag = "@param";
    const string throwsTag = "@throws";
    const string exceptionTag = "@exception";
    const string returnTag = "@return";

    unsigned int i;

    for (i = 0; i < s.size(); ++i)
    {
        if (s[i] == '.' && (i + 1 >= s.size() || isspace(static_cast<unsigned char>(s[i + 1]))))
        {
            ++i;
            break;
        }
        else if (
            s[i] == '@' &&
            (s.substr(i, paramTag.size()) == paramTag || s.substr(i, throwsTag.size()) == throwsTag ||
             s.substr(i, exceptionTag.size()) == exceptionTag || s.substr(i, returnTag.size()) == returnTag))
        {
            break;
        }
    }

    summaryLines = splitIntoLines(trim(s.substr(0, i)));
    if (!summaryLines.empty())
    {
        remarksLines = splitIntoLines(trim(s.substr(i)));
    }
}

void
Slice::CsVisitor::writeDocComment(const ContainedPtr& p, const string& extraParam)
{
    StringList summaryLines;
    StringList remarksLines;
    splitDocComment(p, summaryLines, remarksLines);

    if (summaryLines.empty())
    {
        return;
    }

    _out << nl << "/// <summary>";

    for (StringList::const_iterator i = summaryLines.begin(); i != summaryLines.end(); ++i)
    {
        _out << nl << "///";
        if (!(*i).empty())
        {
            _out << " " << *i;
        }
    }

    bool summaryClosed = false;

    if (!remarksLines.empty())
    {
        for (StringList::const_iterator i = remarksLines.begin(); i != remarksLines.end(); ++i)
        {
            //
            // The first param, returns, or exception tag ends the description.
            //
            static const string paramTag = "<param";
            static const string returnsTag = "<returns";
            static const string exceptionTag = "<exception";

            if (!summaryClosed && (i->find(paramTag) != string::npos || i->find(returnsTag) != string::npos ||
                                   i->find(exceptionTag) != string::npos))
            {
                _out << nl << "/// </summary>";
                _out << nl << "/// " << *i;
                summaryClosed = true;
            }
            else
            {
                _out << nl << "///";
                if (!(*i).empty())
                {
                    _out << " " << *i;
                }
            }
        }
    }

    if (!summaryClosed)
    {
        _out << nl << "/// </summary>";
    }

    if (!extraParam.empty())
    {
        _out << nl << "/// " << extraParam;
    }

    _out << sp;
}

void
Slice::CsVisitor::writeDocCommentAMI(
    const OperationPtr& p,
    ParamDir paramType,
    const string& extraParam1,
    const string& extraParam2,
    const string& extraParam3)
{
    StringList summaryLines;
    StringList remarksLines;
    splitDocComment(p, summaryLines, remarksLines);

    if (summaryLines.empty())
    {
        return;
    }

    //
    // Output the leading comment block up until the first tag.
    //
    _out << nl << "/// <summary>";
    for (StringList::const_iterator i = summaryLines.begin(); i != summaryLines.end(); ++i)
    {
        _out << nl << "///";
        if (!(*i).empty())
        {
            _out << " " << *i;
        }
    }

    bool done = false;
    for (StringList::const_iterator i = remarksLines.begin(); i != remarksLines.end() && !done; ++i)
    {
        string::size_type pos = i->find('<');
        done = true;
        if (pos != string::npos)
        {
            if (pos != 0)
            {
                _out << nl << "/// " << i->substr(0, pos);
            }
        }
        else
        {
            _out << nl << "///";
            if (!(*i).empty())
            {
                _out << " " << *i;
            }
        }
    }
    _out << nl << "/// </summary>";

    //
    // Write the comments for the parameters.
    //
    writeDocCommentParam(p, paramType, false);

    if (!extraParam1.empty())
    {
        _out << nl << "/// " << extraParam1;
    }

    if (!extraParam2.empty())
    {
        _out << nl << "/// " << extraParam2;
    }

    if (!extraParam3.empty())
    {
        _out << nl << "/// " << extraParam3;
    }

    if (paramType == InParam)
    {
        _out << nl << "/// <returns>An asynchronous result object.</returns>";
    }
    else if (p->returnType())
    {
        //
        // Find the comment for the return value (if any).
        //
        static const string returnsTag = "<returns>";
        static const string returnsCloseTag = "</returns>";
        bool doneReturn = false;
        bool foundReturn = false;
        for (StringList::const_iterator i = remarksLines.begin(); i != remarksLines.end() && !doneReturn; ++i)
        {
            if (!foundReturn)
            {
                string::size_type pos = i->find(returnsTag);
                if (pos != string::npos)
                {
                    foundReturn = true;
                    string::size_type endpos = i->find(returnsCloseTag, pos + 1);
                    if (endpos != string::npos)
                    {
                        _out << nl << "/// " << i->substr(pos, endpos - pos + returnsCloseTag.size());
                        doneReturn = true;
                    }
                    else
                    {
                        _out << nl << "/// " << i->substr(pos);
                    }
                }
            }
            else
            {
                string::size_type pos = i->find(returnsCloseTag);
                if (pos != string::npos)
                {
                    _out << nl << "/// " << i->substr(0, pos + returnsCloseTag.size());
                    doneReturn = true;
                }
                else
                {
                    _out << nl << "///";
                    if (!(*i).empty())
                    {
                        _out << " " << *i;
                    }
                }
            }
        }
        if (foundReturn && !doneReturn)
        {
            _out << returnsCloseTag;
        }
    }
}

void
Slice::CsVisitor::writeDocCommentTaskAsyncAMI(
    const OperationPtr& p,
    const string& extraParam1,
    const string& extraParam2,
    const string& extraParam3)
{
    StringList summaryLines;
    StringList remarksLines;
    splitDocComment(p, summaryLines, remarksLines);

    if (summaryLines.empty())
    {
        return;
    }

    //
    // Output the leading comment block up until the first tag.
    //
    _out << nl << "/// <summary>";
    for (StringList::const_iterator i = summaryLines.begin(); i != summaryLines.end(); ++i)
    {
        _out << nl << "///";
        if (!(*i).empty())
        {
            _out << " " << *i;
        }
    }

    bool done = false;
    for (StringList::const_iterator i = remarksLines.begin(); i != remarksLines.end() && !done; ++i)
    {
        string::size_type pos = i->find('<');
        done = true;
        if (pos != string::npos)
        {
            if (pos != 0)
            {
                _out << nl << "/// " << i->substr(0, pos);
            }
        }
        else
        {
            _out << nl << "///";
            if (!(*i).empty())
            {
                _out << " " << *i;
            }
        }
    }
    _out << nl << "/// </summary>";

    //
    // Write the comments for the parameters.
    //
    writeDocCommentParam(p, InParam, false);

    if (!extraParam1.empty())
    {
        _out << nl << "/// " << extraParam1;
    }

    if (!extraParam2.empty())
    {
        _out << nl << "/// " << extraParam2;
    }

    if (!extraParam3.empty())
    {
        _out << nl << "/// " << extraParam3;
    }

    _out << nl << "/// <returns>The task object representing the asynchronous operation.</returns>";
}

void
Slice::CsVisitor::writeDocCommentAMD(const OperationPtr& p, const string& extraParam)
{
    StringList summaryLines;
    StringList remarksLines;
    splitDocComment(p, summaryLines, remarksLines);

    if (summaryLines.empty())
    {
        return;
    }

    //
    // Output the leading comment block up until the first tag.
    //
    _out << nl << "/// <summary>";
    for (StringList::const_iterator i = summaryLines.begin(); i != summaryLines.end(); ++i)
    {
        _out << nl << "///";
        if (!(*i).empty())
        {
            _out << " " << *i;
        }
    }

    bool done = false;
    for (StringList::const_iterator i = remarksLines.begin(); i != remarksLines.end() && !done; ++i)
    {
        string::size_type pos = i->find('<');
        done = true;
        if (pos != string::npos)
        {
            if (pos != 0)
            {
                _out << nl << "/// " << i->substr(0, pos);
            }
        }
        else
        {
            _out << nl << "///";
            if (!(*i).empty())
            {
                _out << " " << *i;
            }
        }
    }
    _out << nl << "/// </summary>";

    //
    // Write the comments for the parameters.
    //
    writeDocCommentParam(p, InParam, true);

    if (!extraParam.empty())
    {
        _out << nl << "/// " << extraParam;
    }

    _out << nl << "/// <returns>The task object representing the asynchronous operation.</returns>";
}

void
Slice::CsVisitor::writeDocCommentParam(const OperationPtr& p, ParamDir paramType, bool /*amd*/)
{
    //
    // Collect the names of the in- or -out parameters to be documented.
    //
    ParameterList tmp = p->parameters();
    vector<string> params;
    for (ParameterList::const_iterator q = tmp.begin(); q != tmp.end(); ++q)
    {
        if ((*q)->isOutParam() && paramType == OutParam)
        {
            params.push_back((*q)->name());
        }
        else if (!(*q)->isOutParam() && paramType == InParam)
        {
            params.push_back((*q)->name());
        }
    }

    //
    // Print the comments for all the parameters that appear in the parameter list.
    //
    StringList summaryLines;
    StringList remarksLines;
    splitDocComment(p, summaryLines, remarksLines);

    const string paramTag = "<param";
    StringList::const_iterator i = remarksLines.begin();
    while (i != remarksLines.end())
    {
        string line = *i++;
        if (line.find(paramTag) != string::npos)
        {
            string::size_type paramNamePos = line.find('"', paramTag.length());
            if (paramNamePos != string::npos)
            {
                string::size_type paramNameEndPos = line.find('"', paramNamePos + 1);
                string paramName = line.substr(paramNamePos + 1, paramNameEndPos - paramNamePos - 1);
                if (std::find(params.begin(), params.end(), paramName) != params.end())
                {
                    _out << nl << "/// " << line;
                    StringList::const_iterator j;
                    if (i == remarksLines.end())
                    {
                        break;
                    }
                    j = i++;
                    while (j != remarksLines.end())
                    {
                        string::size_type endpos = j->find("</param>");
                        if (endpos == string::npos)
                        {
                            i = j;
                            string s = *j++;
                            _out << nl << "///";
                            if (!s.empty())
                            {
                                _out << " " << s;
                            }
                        }
                        else
                        {
                            _out << nl << "/// " << *j++;
                            break;
                        }
                    }
                }
            }
        }
    }
}

void
Slice::CsVisitor::moduleStart(const ModulePtr& p)
{
    if (!dynamic_pointer_cast<Contained>(p->container()))
    {
        string ns = getNamespacePrefix(p);
        string name = fixId(p->name());
        if (!ns.empty())
        {
            _out << sp;
            _out << nl << "namespace " << ns;
            _out << sb;
        }
    }
}

void
Slice::CsVisitor::moduleEnd(const ModulePtr& p)
{
    if (!dynamic_pointer_cast<Contained>(p->container()))
    {
        if (!getNamespacePrefix(p).empty())
        {
            _out << eb;
        }
    }
}

Slice::Gen::Gen(const string& base, const vector<string>& includePaths, const string& dir) : _includePaths(includePaths)
{
    string fileBase = base;
    string::size_type pos = base.find_last_of("/\\");
    if (pos != string::npos)
    {
        fileBase = base.substr(pos + 1);
    }
    string file = fileBase + ".cs";
    if (!dir.empty())
    {
        file = dir + '/' + file;
    }

    _out.open(file.c_str());
    if (!_out)
    {
        ostringstream os;
        os << "cannot open `" << file << "': " << IceInternal::errorToString(errno);
        throw FileException(__FILE__, __LINE__, os.str());
    }
    FileTracker::instance()->addFile(file);
    printHeader();

    printGeneratedHeader(_out, fileBase + ".ice");

    _out << nl << "#nullable enable";
    _out << sp;
    _out << nl << "[assembly:Ice.Slice(\"" << fileBase << ".ice\")]";

    _out << sp;
    _out << nl << "#pragma warning disable CS1591 // Missing XML Comment";
    _out << nl << "#pragma warning disable CS1573 // Parameter has no matching param tag in the XML comment";
    _out << nl << "#pragma warning disable CS0612 // Type or member is obsolete";
    _out << nl << "#pragma warning disable CS0618 // Type or member is obsolete";
    _out << nl << "#pragma warning disable CS0619 // Type or member is obsolete";
}

Slice::Gen::~Gen()
{
    if (_out.isOpen())
    {
        _out << '\n';
        _out.close();
    }
}

void
Slice::Gen::generate(const UnitPtr& p)
{
    CsGenerator::validateMetadata(p);

    UnitVisitor unitVisitor(_out);
    p->visit(&unitVisitor);

    TypesVisitor typesVisitor(_out);
    p->visit(&typesVisitor);

    ResultVisitor resultVisitor(_out);
    p->visit(&resultVisitor);

    ProxyVisitor proxyVisitor(_out);
    p->visit(&proxyVisitor);

    HelperVisitor helperVisitor(_out);
    p->visit(&helperVisitor);

    DispatcherVisitor dispatcherVisitor(_out);
    p->visit(&dispatcherVisitor);

    DispatchAdapterVisitor dispatchAdapterVisitor(_out);
    p->visit(&dispatchAdapterVisitor);
}

void
Slice::Gen::printHeader()
{
    static const char* header = "//\n"
                                "// Copyright (c) ZeroC, Inc. All rights reserved.\n"
                                "//\n";

    _out << header;
    _out << "//\n";
    _out << "// Ice version " << ICE_STRING_VERSION << "\n";
    _out << "//\n";
}

Slice::Gen::UnitVisitor::UnitVisitor(IceInternal::Output& out) : CsVisitor(out) {}

bool
Slice::Gen::UnitVisitor::visitUnitStart(const UnitPtr& unit)
{
    DefinitionContextPtr dc = unit->findDefinitionContext(unit->topLevelFile());
    assert(dc);

    bool sep = false;
    for (const auto& metadata : dc->getMetadata())
    {
        if (metadata->directive() == "cs:attribute")
        {
            if (!sep)
            {
                _out << sp;
                sep = true;
            }
            _out << nl << '[' << metadata->arguments() << ']';
        }
    }
    return false;
}

Slice::Gen::TypesVisitor::TypesVisitor(IceInternal::Output& out) : CsVisitor(out) {}
bool
Slice::Gen::TypesVisitor::visitModuleStart(const ModulePtr& p)
{
    moduleStart(p);
    string name = fixId(p->name());
    _out << sp;
    emitAttributes(p);
    _out << nl << "namespace " << name;

    _out << sb;

    return true;
}

void
Slice::Gen::TypesVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
    moduleEnd(p);
}

bool
Slice::Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = p->name();
    string scoped = fixId(p->scoped());
    string ns = getNamespace(p);
    ClassDefPtr base = p->base();
    StringList baseNames;

    _out << sp;
    emitAttributes(p);

    _out << nl << "[Ice.SliceTypeId(\"" << p->scoped() << "\")]";
    if (p->compactId() >= 0)
    {
        _out << nl << "[Ice.CompactSliceTypeId(" << p->compactId() << ")]";
    }
    _out << nl << "public partial class " << fixId(name) << " : ";

    if (base)
    {
        _out << getUnqualified(base, ns);
        ;
    }
    else
    {
        _out << "Ice.Value";
    }

    _out << sb;
    return true;
}

void
Slice::Gen::TypesVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    string name = fixId(p->name());
    string ns = getNamespace(p);
    DataMemberList classMembers = p->classDataMembers();
    DataMemberList allClassMembers = p->allClassDataMembers();
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList allDataMembers = p->allDataMembers();
    ClassDefPtr base = p->base();

    _out << sp << nl << "partial void ice_initialize();";
    if (allDataMembers.empty())
    {
        _out << sp;
        _out << nl << "public " << name << spar << epar;
        _out << sb;
        _out << nl << "ice_initialize();";
        _out << eb;
    }
    else
    {
        // Primary constructor.
        _out << sp;
        _out << nl << "public " << name << spar;

        vector<string> parameters;
        vector<string> secondaryCtorParams;
        vector<string> secondaryCtorMemberNames;
        vector<string> secondaryCtorBaseParamNames;
        for (const auto& q : allDataMembers)
        {
            string memberName = fixId(q->name(), DotNet::ICloneable);
            string memberType = typeToString(q->type(), ns, q->optional());
            parameters.push_back(memberType + " " + memberName);

            // The secondary constructor initializes the fields that would be marked "required" if we generated the
            // required keyword.
            if (isMappedToRequiredField(q))
            {
                secondaryCtorParams.push_back(memberType + " " + memberName);

                if (find(dataMembers.begin(), dataMembers.end(), q) != dataMembers.end())
                {
                    secondaryCtorMemberNames.push_back(memberName);
                }
            }
        }
        _out << parameters << epar;

        if (base && allDataMembers.size() != dataMembers.size())
        {
            _out << " : base" << spar;
            vector<string> baseParamNames;
            DataMemberList baseDataMembers = base->allDataMembers();
            for (const auto& q : baseDataMembers)
            {
                string memberName = fixId(q->name(), DotNet::ICloneable);
                baseParamNames.push_back(memberName);

                // Look for required fields
                if (isMappedToRequiredField(q))
                {
                    secondaryCtorBaseParamNames.push_back(memberName);
                }
            }
            _out << baseParamNames << epar;
        }
        _out << sb;
        for (const auto& q : dataMembers)
        {
            const string memberName = fixId(q->name(), DotNet::ICloneable);
            if (isMappedToNonNullableReference(q))
            {
                _out << nl << "global::System.ArgumentNullException.ThrowIfNull(" << memberName << ");";
            }
            _out << nl << "this." << memberName << " = " << memberName << ';';
        }
        _out << nl << "ice_initialize();";
        _out << eb;

        // Secondary constructor. Can be parameterless.
        if (secondaryCtorParams.size() != parameters.size())
        {
            _out << sp;
            _out << nl << "public " << name << spar << secondaryCtorParams << epar;
            if (base && secondaryCtorBaseParamNames.size() > 0)
            {
                _out << " : base" << spar << secondaryCtorBaseParamNames << epar;
            }
            _out << sb;
            for (const auto& q : secondaryCtorMemberNames)
            {
                // All these parameters/fields are non-nullable and we don't tolerate null values.
                _out << nl << "global::System.ArgumentNullException.ThrowIfNull(" << q << ");";
                _out << nl << "this." << q << " = " << q << ';';
            }
            _out << nl << "ice_initialize();";
            _out << eb;
        }

        // Parameterless constructor. Required for unmarshaling.
        if (secondaryCtorParams.size() > 0)
        {
            _out << sp;
            emitNonBrowsableAttribute();
            _out << nl << "public " << name << "()";
            _out << sb;
            writeDataMemberInitializers(dataMembers, DotNet::ICloneable);
            _out << nl << "ice_initialize();";
            _out << eb;
        }
    }

    _out << sp;
    _out << nl << "public static new string ice_staticId() => \"" << p->scoped() << "\";";

    _out << nl << "public override string ice_id() => ice_staticId();";
    writeMarshaling(p);

    _out << eb;
}

bool
Slice::Gen::TypesVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    string name = p->name();
    string scoped = fixId(p->scoped());
    string ns = getNamespace(p);
    InterfaceList bases = p->bases();

    list<string> baseNames;

    _out << sp;
    emitAttributes(p);

    _out << nl << "[Ice.SliceTypeId(\"" << p->scoped() << "\")]";
    _out << nl << "public partial interface " << fixId(name);

    for (const auto& q : bases)
    {
        baseNames.push_back(getUnqualified(q, ns));
    }

    if (baseNames.empty())
    {
        baseNames.push_back("Ice.Object");
    }

    // Check for `cs:implements` metadata.
    for (const auto& metadata : p->getMetadata())
    {
        if (metadata->directive() == "cs:implements")
        {
            baseNames.push_back(metadata->arguments());
        }
    }

    _out << " : ";
    bool emitSep = false;
    for (const auto& baseName : baseNames)
    {
        if (emitSep)
        {
            _out << ", ";
        }
        emitSep = true;
        _out << baseName;
    }

    _out << sb;
    return true;
}

void
Slice::Gen::TypesVisitor::visitInterfaceDefEnd(const InterfaceDefPtr&)
{
    _out << eb;
}

void
Slice::Gen::TypesVisitor::visitOperation(const OperationPtr& op)
{
    InterfaceDefPtr interface = op->interface();
    string interfaceName = fixId(interface->name());
    string ns = getNamespace(interface);

    const bool amd = interface->hasMetadata("amd") || op->hasMetadata("amd");
    string retS;
    vector<string> params, args;
    string opName = getDispatchParams(op, retS, params, args, ns);
    _out << sp;
    if (amd)
    {
        writeDocCommentAMD(op, "<param name=\"" + args.back() + "\">The Current object for the dispatch.</param>");
    }
    else
    {
        writeDocComment(op, "<param name=\"" + args.back() + "\">The Current object for the dispatch.</param>");
    }
    emitAttributes(op);
    emitObsoleteAttribute(op, _out);
    _out << nl << retS << " " << opName << spar << params << epar << ";";
}

void
Slice::Gen::TypesVisitor::visitSequence(const SequencePtr&)
{
    //
    // No need to generate anything for sequences.
    //
}

bool
Slice::Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string name = fixId(p->name());
    string ns = getNamespace(p);
    ExceptionPtr base = p->base();

    _out << sp;
    writeDocComment(p);
    emitObsoleteAttribute(p, _out);
    emitAttributes(p);
    _out << nl << "[Ice.SliceTypeId(\"" << p->scoped() << "\")]";
    _out << nl << "public partial class " << name << " : ";
    if (base)
    {
        _out << getUnqualified(base, ns);
    }
    else
    {
        _out << "Ice.UserException";
    }
    _out << sb;
    return true;
}

void
Slice::Gen::TypesVisitor::visitExceptionEnd(const ExceptionPtr& p)
{
    string name = fixId(p->name());
    string ns = getNamespace(p);
    DataMemberList allDataMembers = p->allDataMembers();
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList allClassMembers = p->allClassDataMembers();
    DataMemberList classMembers = p->classDataMembers();
    DataMemberList optionalMembers = p->orderedOptionalDataMembers();
    ExceptionPtr base = p->base();

    if (!allDataMembers.empty())
    {
        // Primary constructor.
        _out << sp;
        _out << nl << "public " << name << spar;

        vector<string> parameters;
        vector<string> secondaryCtorParams;
        vector<string> secondaryCtorMemberNames;
        vector<string> secondaryCtorBaseParamNames;

        for (const auto& q : allDataMembers)
        {
            string memberName = fixId(q->name(), DotNet::Exception);
            string memberType = typeToString(q->type(), ns, q->optional());
            parameters.push_back(memberType + " " + memberName);

            // The secondary constructor initializes the fields that would be marked "required" if we generated the
            // required keyword.
            if (isMappedToRequiredField(q))
            {
                secondaryCtorParams.push_back(memberType + " " + memberName);

                if (find(dataMembers.begin(), dataMembers.end(), q) != dataMembers.end())
                {
                    secondaryCtorMemberNames.push_back(memberName);
                }
            }
        }
        _out << parameters << epar;

        if (base && allDataMembers.size() != dataMembers.size())
        {
            _out << " : base" << spar;
            vector<string> baseParamNames;
            DataMemberList baseDataMembers = base->allDataMembers();
            for (const auto& q : baseDataMembers)
            {
                string memberName = fixId(q->name(), DotNet::Exception);
                baseParamNames.push_back(memberName);

                // Look for required fields
                if (isMappedToRequiredField(q))
                {
                    secondaryCtorBaseParamNames.push_back(memberName);
                }
            }
            _out << baseParamNames << epar;
        }
        _out << sb;
        for (const auto& q : dataMembers)
        {
            const string memberName = fixId(q->name(), DotNet::Exception);
            if (isMappedToNonNullableReference(q))
            {
                _out << nl << "global::System.ArgumentNullException.ThrowIfNull(" << memberName << ");";
            }
            _out << nl << "this." << memberName << " = " << memberName << ';';
        }
        _out << eb;

        // Secondary constructor. Can be parameterless.
        if (secondaryCtorParams.size() != parameters.size())
        {
            _out << sp;
            _out << nl << "public " << name << spar << secondaryCtorParams << epar;
            if (base && secondaryCtorBaseParamNames.size() > 0)
            {
                _out << " : base" << spar << secondaryCtorBaseParamNames << epar;
            }
            _out << sb;
            for (const auto& q : secondaryCtorMemberNames)
            {
                // All these parameters/fields are non-nullable and we don't tolerate null values.
                _out << nl << "global::System.ArgumentNullException.ThrowIfNull(" << q << ");";
                _out << nl << "this." << q << " = " << q << ';';
            }
            _out << eb;
        }

        // Parameterless constructor. Required for unmarshaling.
        if (secondaryCtorParams.size() > 0)
        {
            _out << sp;
            emitNonBrowsableAttribute();
            _out << nl << "public " << name << "()";
            _out << sb;
            writeDataMemberInitializers(dataMembers, DotNet::Exception);
            _out << eb;
        }
    }

    _out << sp;
    _out << nl << "public override string ice_id() => \"" << p->scoped() << "\";";

    string scoped = p->scoped();

    _out << sp;
    _out << nl << "protected override void iceWriteImpl(Ice.OutputStream ostr_)";
    _out << sb;
    _out << nl << "ostr_.startSlice(\"" << scoped << "\", -1, " << (!base ? "true" : "false") << ");";
    for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        if (!(*q)->optional())
        {
            writeMarshalDataMember(*q, fixId((*q)->name(), DotNet::Exception), ns);
        }
    }

    for (DataMemberList::const_iterator q = optionalMembers.begin(); q != optionalMembers.end(); ++q)
    {
        writeMarshalDataMember(*q, fixId((*q)->name(), DotNet::Exception), ns);
    }

    _out << nl << "ostr_.endSlice();";
    if (base)
    {
        _out << nl << "base.iceWriteImpl(ostr_);";
    }
    _out << eb;

    _out << sp;
    _out << nl << "protected override void iceReadImpl(Ice.InputStream istr_)";
    _out << sb;
    _out << nl << "istr_.startSlice();";

    for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        if (!(*q)->optional())
        {
            writeUnmarshalDataMember(*q, fixId((*q)->name(), DotNet::Exception), ns);
        }
    }

    for (DataMemberList::const_iterator q = optionalMembers.begin(); q != optionalMembers.end(); ++q)
    {
        writeUnmarshalDataMember(*q, fixId((*q)->name(), DotNet::Exception), ns);
    }
    _out << nl << "istr_.endSlice();";
    if (base)
    {
        _out << nl << "base.iceReadImpl(istr_);";
    }
    _out << eb;

    if (p->usesClasses() && !(base && base->usesClasses()))
    {
        _out << sp;
        _out << nl << "public override bool iceUsesClasses()";
        _out << sb;
        _out << nl << "return true;";
        _out << eb;
    }

    _out << eb;
}

bool
Slice::Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    string name = fixId(p->name());
    string ns = getNamespace(p);
    _out << sp;
    const bool classMapping = isMappedToClass(p);

    emitObsoleteAttribute(p, _out);

    emitAttributes(p);
    _out << nl << "public " << (classMapping ? "sealed partial record class" : "partial record struct") << ' ' << name;

    // Check for cs:implements metadata.
    list<string> baseNames;
    for (const auto& metadata : p->getMetadata())
    {
        if (metadata->directive() == "cs:implements")
        {
            baseNames.push_back(metadata->arguments());
        }
    }

    if (!baseNames.empty())
    {
        _out << " : ";
        bool emitSep = false;
        for (const auto& baseName : baseNames)
        {
            if (emitSep)
            {
                _out << ", ";
            }
            emitSep = true;
            _out << getUnqualified(baseName, ns);
        }
    }

    _out << sb;
    return true;
}

void
Slice::Gen::TypesVisitor::visitStructEnd(const StructPtr& p)
{
    string name = fixId(p->name());
    string scope = fixId(p->scope());
    string ns = getNamespace(p);
    DataMemberList classMembers = p->classDataMembers();
    DataMemberList dataMembers = p->dataMembers();
    const bool classMapping = isMappedToClass(p);

    _out << sp << nl << "partial void ice_initialize();";

    if (classMapping)
    {
        // We generate a constructor that initializes all required fields (collections and structs mapped to
        // classes). It can be parameterless.

        vector<string> ctorParams;
        vector<string> ctorParamNames;
        for (const auto& q : dataMembers)
        {
            if (isMappedToRequiredField(q))
            {
                string memberName = fixId(q->name(), DotNet::ICloneable);
                string memberType = typeToString(q->type(), ns, false);
                ctorParams.push_back(memberType + " " + memberName);
                ctorParamNames.push_back(memberName);
            }
        }

        // We only generate this ctor if it's different from the primary constructor.
        if (ctorParams.size() != dataMembers.size())
        {
            _out << sp;
            _out << nl << "public " << name << spar << ctorParams << epar;
            _out << sb;
            for (const auto& q : ctorParamNames)
            {
                _out << nl << "global::System.ArgumentNullException.ThrowIfNull(" << q << ");";
                _out << nl << "this." << q << " = " << q << ';';
            }
            // All the other fields keep their default values (explicit or implicit).
            _out << nl << "ice_initialize();";
            _out << eb;
        }
    }

    _out << sp;
    _out << nl << "public " << name << spar;
    vector<string> parameters;
    for (const auto& q : dataMembers)
    {
        string memberName = fixId(q->name(), classMapping ? DotNet::ICloneable : 0);
        string memberType = typeToString(q->type(), ns, false);
        parameters.push_back(memberType + " " + memberName);
    }
    _out << parameters << epar;
    _out << sb;
    for (const auto& q : dataMembers)
    {
        string paramName = fixId(q->name(), classMapping ? DotNet::ICloneable : 0);
        if (isMappedToNonNullableReference(q))
        {
            _out << nl << "global::System.ArgumentNullException.ThrowIfNull(" << paramName << ");";
        }
        _out << nl << "this." << paramName << " = " << paramName << ';';
    }
    _out << nl << "ice_initialize();";
    _out << eb;

    // Unmarshaling constructor
    _out << sp;
    _out << nl << "public " << name << "(Ice.InputStream istr)";
    _out << sb;
    for (const auto& q : dataMembers)
    {
        writeUnmarshalDataMember(q, fixId(q->name(), classMapping ? DotNet::ICloneable : 0), ns, true);
    }
    _out << nl << "ice_initialize();";
    _out << eb;

    _out << sp;
    _out << nl << "public void ice_writeMembers(Ice.OutputStream ostr)";
    _out << sb;
    for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        writeMarshalDataMember(*q, fixId((*q)->name(), classMapping ? DotNet::ICloneable : 0), ns, true);
    }
    _out << eb;

    _out << sp;
    _out << nl << "public static void ice_write(Ice.OutputStream ostr, " << name << " v)";
    _out << sb;
    _out << nl << "v.ice_writeMembers(ostr);";
    _out << eb;

    _out << sp;
    _out << nl << "public static " << name << " ice_read(Ice.InputStream istr) => new(istr);";
    _out << eb;
}

void
Slice::Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    string name = fixId(p->name());
    string ns = getNamespace(p);
    string scoped = fixId(p->scoped());
    EnumeratorList enumerators = p->enumerators();
    const bool hasExplicitValues = p->hasExplicitValues();

    _out << sp;
    emitObsoleteAttribute(p, _out);
    writeDocComment(p);
    emitAttributes(p);
    _out << nl << "public enum " << name;
    _out << sb;
    for (EnumeratorList::const_iterator en = enumerators.begin(); en != enumerators.end(); ++en)
    {
        if (en != enumerators.begin())
        {
            _out << ',';
        }
        writeDocComment(*en, "");
        _out << nl << fixId((*en)->name());
        if (hasExplicitValues)
        {
            _out << " = " << (*en)->value();
        }
    }
    _out << eb;

    _out << sp;
    _out << nl << "public sealed class " << p->name() << "Helper";
    _out << sb;
    _out << sp;
    _out << nl << "public static void write(Ice.OutputStream ostr, " << name << " v)";
    _out << sb;
    writeMarshalUnmarshalCode(_out, p, ns, "v", true);
    _out << eb;

    _out << sp;
    _out << nl << "public static " << name << " read(Ice.InputStream istr)";
    _out << sb;
    _out << nl << name << " v;";
    writeMarshalUnmarshalCode(_out, p, ns, "v", false);
    _out << nl << "return v;";
    _out << eb;

    _out << eb;
}

void
Slice::Gen::TypesVisitor::visitConst(const ConstPtr& p)
{
    string name = fixId(p->name());
    _out << sp;
    emitAttributes(p);
    _out << nl << "public abstract class " << name;
    _out << sb;
    _out << sp << nl << "public const " << typeToString(p->type(), "") << " value = ";
    writeConstantValue(p->type(), p->valueType(), p->value());
    _out << ";";
    _out << eb;
}

void
Slice::Gen::TypesVisitor::visitDataMember(const DataMemberPtr& p)
{
    unsigned int baseTypes = 0;
    bool isClass = false;
    bool isProtected = false;
    const bool isOptional = p->optional();

    ContainedPtr cont = dynamic_pointer_cast<Contained>(p->container());
    assert(cont);
    bool isProperty = cont->hasMetadata("cs:property");

    StructPtr st = dynamic_pointer_cast<Struct>(cont);
    ExceptionPtr ex = dynamic_pointer_cast<Exception>(cont);
    ClassDefPtr cl = dynamic_pointer_cast<ClassDef>(cont);
    string ns = getNamespace(cont);
    if (st)
    {
        if (isMappedToClass(st))
        {
            baseTypes = DotNet::ICloneable;
        }
    }
    else if (ex)
    {
        baseTypes = DotNet::Exception;
    }
    else
    {
        assert(cl);
        baseTypes = DotNet::ICloneable;
        isClass = true;
        isProtected = cont->hasMetadata("protected") || p->hasMetadata("protected");
    }

    _out << sp;

    emitObsoleteAttribute(p, _out);

    string type = typeToString(p->type(), ns, isOptional);
    string dataMemberName = fixId(p->name(), baseTypes, isClass);

    emitAttributes(p);
    if (isProtected)
    {
        _out << nl << "protected";
    }
    else
    {
        _out << nl << "public";
    }
    _out << ' ' << type << ' ' << dataMemberName;

    bool addSemicolon = true;
    if (isProperty)
    {
        _out << " { get; set; }";
        addSemicolon = false;
    }

    // Generate the default value for this field unless the enclosing type is a struct.
    if (!st || isMappedToClass(st))
    {
        if (p->defaultValueType())
        {
            _out << " = ";
            writeConstantValue(p->type(), p->defaultValueType(), p->defaultValue());
            addSemicolon = true;
        }
        else if (!p->optional())
        {
            BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(p->type());
            if (builtin && builtin->kind() == Builtin::KindString)
            {
                // This behavior is unfortunate but kept for backwards compatibility.
                _out << " = \"\"";
                addSemicolon = true;
            }
        }
    }

    if (addSemicolon)
    {
        _out << ';';
    }
}

Slice::Gen::ResultVisitor::ResultVisitor(::IceInternal::Output& out) : CsVisitor(out) {}

namespace
{
    bool hasResultType(const ModulePtr& p)
    {
        InterfaceList interfaces = p->interfaces();
        for (InterfaceList::const_iterator i = interfaces.begin(); i != interfaces.end(); ++i)
        {
            InterfaceDefPtr interface = *i;
            OperationList operations = interface->operations();
            for (OperationList::const_iterator j = operations.begin(); j != operations.end(); ++j)
            {
                OperationPtr op = *j;
                ParameterList outParams = op->outParameters();
                TypePtr ret = op->returnType();
                if (outParams.size() > 1 || (ret && outParams.size() > 0))
                {
                    return true;
                }
            }
        }

        for (const auto& module : p->modules())
        {
            if (hasResultType(module))
            {
                return true;
            }
        }

        return false;
    }
}

bool
Slice::Gen::ResultVisitor::visitModuleStart(const ModulePtr& p)
{
    if (hasResultType(p))
    {
        moduleStart(p);
        _out << sp << nl << "namespace " << fixId(p->name());
        _out << sb;
        return true;
    }
    return false;
}

void
Slice::Gen::ResultVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
    moduleEnd(p);
}

void
Slice::Gen::ResultVisitor::visitOperation(const OperationPtr& p)
{
    InterfaceDefPtr interface = p->interface();
    string ns = getNamespace(interface);
    ParameterList outParams = p->outParameters();
    TypePtr ret = p->returnType();

    if (outParams.size() > 1 || (ret && outParams.size() > 0))
    {
        string name = resultStructName(interface->name(), p->name());

        string retS;
        string retSName;
        if (ret)
        {
            retS = typeToString(ret, ns, p->returnIsOptional());
            retSName = resultStructReturnValueName(outParams);
        }

        _out << sp;
        _out << nl << "public record struct " << name;
        _out << spar;
        if (ret)
        {
            _out << (retS + " " + retSName);
        }

        for (const auto& q : outParams)
        {
            _out << (typeToString(q->type(), ns, q->optional()) + " " + fixId(q->name()));
        }
        _out << epar;
        _out << ";";
    }

    if (p->hasMarshaledResult())
    {
        string name = resultStructName(interface->name(), p->name(), true);

        _out << sp;
        _out << nl << "public readonly record struct " << name << " : Ice.MarshaledResult";
        _out << sb;

        //
        // Marshaling constructor
        //
        _out << nl << "public " << name << spar << getOutParams(p, ns, true, false) << "Ice.Current current" << epar;
        _out << sb;
        _out << nl << "_ostr = Ice.CurrentExtensions.startReplyStream(current);";
        _out << nl << "_ostr.startEncapsulation(current.encoding, " << opFormatTypeToString(p) << ");";
        writeMarshalUnmarshalParams(outParams, p, true, ns, false, true, "_ostr");
        if (p->returnsClasses())
        {
            _out << nl << "_ostr.writePendingValues();";
        }
        _out << nl << "_ostr.endEncapsulation();";
        _out << eb;
        _out << sp;
        _out << nl << "public Ice.OutputStream outputStream => _ostr;";
        _out << sp;
        _out << nl << "private readonly Ice.OutputStream _ostr;";
        _out << eb;
    }
}

Slice::Gen::ProxyVisitor::ProxyVisitor(IceInternal::Output& out) : CsVisitor(out) {}

bool
Slice::Gen::ProxyVisitor::visitModuleStart(const ModulePtr& p)
{
    if (!p->contains<InterfaceDef>())
    {
        return false;
    }

    moduleStart(p);
    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::ProxyVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
    moduleEnd(p);
}

bool
Slice::Gen::ProxyVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    string name = p->name();
    string ns = getNamespace(p);
    InterfaceList bases = p->bases();

    _out << sp;
    writeDocComment(p);
    _out << nl << "public interface " << name << "Prx : ";

    vector<string> baseInterfaces;
    for (InterfaceList::const_iterator q = bases.begin(); q != bases.end(); ++q)
    {
        baseInterfaces.push_back(getUnqualified(*q, ns, "", "Prx"));
    }

    if (baseInterfaces.empty())
    {
        baseInterfaces.push_back("Ice.ObjectPrx");
    }

    for (vector<string>::const_iterator q = baseInterfaces.begin(); q != baseInterfaces.end();)
    {
        _out << *q;
        if (++q != baseInterfaces.end())
        {
            _out << ", ";
        }
    }
    _out << sb;

    return true;
}

void
Slice::Gen::ProxyVisitor::visitInterfaceDefEnd(const InterfaceDefPtr&)
{
    _out << eb;
}

void
Slice::Gen::ProxyVisitor::visitOperation(const OperationPtr& p)
{
    InterfaceDefPtr interface = p->interface();
    string ns = getNamespace(interface);
    string name = fixId(p->name(), DotNet::ICloneable, true);
    vector<string> inParams = getInParams(p, ns);
    ParameterList inParameters = p->inParameters();
    string retS = typeToString(p->returnType(), ns, p->returnIsOptional());

    {
        //
        // Write the synchronous version of the operation.
        //
        string context = getEscapedParamName(p, "context");
        _out << sp;
        writeDocComment(p, "<param name=\"" + context + "\">The Context map to send with the invocation.</param>");
        emitObsoleteAttribute(p, _out);
        _out << nl << retS << " " << name << spar << getParams(p, ns)
             << ("global::System.Collections.Generic.Dictionary<string, string>? " + context + " = null") << epar
             << ';';
    }

    {
        //
        // Write the async version of the operation (using Async Task API)
        //
        string context = getEscapedParamName(p, "context");
        string cancel = getEscapedParamName(p, "cancel");
        string progress = getEscapedParamName(p, "progress");

        _out << sp;
        writeDocCommentTaskAsyncAMI(
            p,
            "<param name=\"" + context + "\">Context map to send with the invocation.</param>",
            "<param name=\"" + progress + "\">Sent progress provider.</param>",
            "<param name=\"" + cancel + "\">A cancellation token that receives the cancellation requests.</param>");
        emitObsoleteAttribute(p, _out);
        _out << nl << taskResultType(p, ns);
        _out << " " << p->name() << "Async" << spar << inParams
             << ("global::System.Collections.Generic.Dictionary<string, string>? " + context + " = null")
             << ("global::System.IProgress<bool>? " + progress + " = null")
             << ("global::System.Threading.CancellationToken " + cancel + " = default") << epar << ";";
    }
}

Slice::Gen::DispatchAdapterVisitor::DispatchAdapterVisitor(IceInternal::Output& out) : CsVisitor(out) {}

bool
Slice::Gen::DispatchAdapterVisitor::visitModuleStart(const ModulePtr& p)
{
    if (!p->contains<InterfaceDef>())
    {
        return false;
    }

    moduleStart(p);
    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::DispatchAdapterVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
    moduleEnd(p);
}

bool
Slice::Gen::DispatchAdapterVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    if (p->allOperations().empty())
    {
        return false;
    }

    _out << sp;
    _out << nl << "public partial interface " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::DispatchAdapterVisitor::visitInterfaceDefEnd(const InterfaceDefPtr&)
{
    _out << eb;
}

void
Slice::Gen::DispatchAdapterVisitor::visitOperation(const OperationPtr& op)
{
    InterfaceDefPtr interface = op->interface();
    string ns = getNamespace(interface);
    string interfaceName = fixId(interface->name());
    const bool amd = interface->hasMetadata("amd") || op->hasMetadata("amd");

    _out << sp;
    _out << nl << "protected static " << (amd ? "async " : "")
         << "global::System.Threading.Tasks.ValueTask<Ice.OutgoingResponse> iceD_" << op->name() << "Async(";
    _out.inc();
    _out << nl << interfaceName << " obj,";
    _out << nl << "Ice.IncomingRequest request)";
    _out.dec();
    _out << sb;

    TypePtr ret = op->returnType();
    ParameterList inParams = op->inParameters();
    ParameterList outParams = op->outParameters();

    _out << nl << "Ice.ObjectImpl.iceCheckMode(" << sliceModeToIceMode(op->mode()) << ", request.current.mode);";
    if (!inParams.empty())
    {
        // Unmarshal 'in' parameters.
        _out << nl << "var istr = request.inputStream;";
        _out << nl << "istr.startEncapsulation();";
        for (const auto& pli : inParams)
        {
            string param = "iceP_" + pli->name();
            string typeS = typeToString(pli->type(), ns, pli->optional());

            _out << nl << typeS << ' ' << param << (pli->type()->isClassType() ? " = null;" : ";");
        }
        writeMarshalUnmarshalParams(inParams, nullptr, false, ns);
        if (op->sendsClasses())
        {
            _out << nl << "istr.readPendingValues();";
        }
        _out << nl << "istr.endEncapsulation();";
    }
    else
    {
        _out << nl << "request.inputStream.skipEmptyEncapsulation();";
    }

    vector<string> inArgs;
    for (const auto& pli : inParams)
    {
        inArgs.push_back("iceP_" + pli->name());
    }

    if (op->hasMarshaledResult())
    {
        if (amd)
        {
            _out << nl << "var result = await obj." << op->name() << "Async" << spar << inArgs << "request.current"
                 << epar << ".ConfigureAwait(false);";
            _out << nl << "return new Ice.OutgoingResponse(result.outputStream);";
        }
        else
        {
            _out << nl << "var result = obj." << fixId(op->name(), DotNet::ICloneable, true) << spar << inArgs
                 << "request.current" << epar << ";";
            _out << nl << "return new (new Ice.OutgoingResponse(result.outputStream));";
        }
    }
    else if (amd)
    {
        string opName = op->name() + "Async";
        string retS = resultType(op, ns);
        _out << nl;

        if (!retS.empty())
        {
            _out << "var result = ";
        }

        _out << "await obj." << opName << spar << inArgs << "request.current" << epar << ".ConfigureAwait(false);";

        if (retS.empty())
        {
            _out << nl << "return Ice.CurrentExtensions.createEmptyOutgoingResponse(request.current);";
        }
        else
        {
            // Adapt to marshaling helper below.
            string resultParam = !ret && outParams.size() == 1 ? "iceP_" + outParams.front()->name() : "ret";

            _out << nl << "return Ice.CurrentExtensions.createOutgoingResponse(";
            _out.inc();
            _out << nl << "request.current,";
            _out << nl << "result,";
            _out << nl << "static (ostr, " << resultParam << ") =>";
            _out << sb;
            writeMarshalUnmarshalParams(outParams, op, true, ns, true);
            if (op->returnsClasses())
            {
                _out << nl << "ostr.writePendingValues();";
            }
            _out << eb;
            if (op->format())
            {
                _out << "," << nl << opFormatTypeToString(op);
            }
            _out << ");";
            _out.dec();
        }
    }
    else
    {
        string opName = op->name();
        for (const auto& pli : outParams)
        {
            string typeS = typeToString(pli->type(), ns, pli->optional());
            _out << nl << typeS << ' ' << "iceP_" + pli->name() << ";";
        }
        _out << nl;
        if (ret)
        {
            _out << "var ret = ";
        }
        _out << "obj." << fixId(opName, DotNet::ICloneable, true) << spar << inArgs;
        for (const auto& pli : outParams)
        {
            _out << "out iceP_" + pli->name();
        }
        _out << "request.current" << epar << ";";

        if (outParams.empty() && !ret)
        {
            _out << nl << "return new(Ice.CurrentExtensions.createEmptyOutgoingResponse(request.current));";
        }
        else
        {
            _out << nl << "var ostr = Ice.CurrentExtensions.startReplyStream(request.current);";
            _out << nl << "ostr.startEncapsulation(request.current.encoding, " << opFormatTypeToString(op) << ");";
            writeMarshalUnmarshalParams(outParams, op, true, ns);
            if (op->returnsClasses())
            {
                _out << nl << "ostr.writePendingValues();";
            }
            _out << nl << "ostr.endEncapsulation();";
            _out << nl << "return new(new Ice.OutgoingResponse(ostr));";
        }
    }
    _out << eb;
}

Slice::Gen::HelperVisitor::HelperVisitor(IceInternal::Output& out) : CsVisitor(out) {}

bool
Slice::Gen::HelperVisitor::visitModuleStart(const ModulePtr& p)
{
    if (!p->contains<InterfaceDef>() && !p->contains<Sequence>() && !p->contains<Dictionary>())
    {
        return false;
    }

    moduleStart(p);
    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::HelperVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
    moduleEnd(p);
}

bool
Slice::Gen::HelperVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    string name = p->name();
    string ns = getNamespace(p);
    InterfaceList bases = p->bases();

    _out << sp;
    _out << nl << "public sealed class " << name << "PrxHelper : "
         << "Ice.ObjectPrxHelperBase, " << name << "Prx";
    _out << sb;

    OperationList ops = p->allOperations();

    for (const auto& op : ops)
    {
        string opName = fixId(op->name(), DotNet::ICloneable, true);
        TypePtr ret = op->returnType();
        string retS = typeToString(ret, ns, op->returnIsOptional());

        vector<string> params = getParams(op, ns);
        vector<string> argsAMI = getInArgs(op);

        ParameterList outParams = op->outParameters();

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();

        //
        // Arrange exceptions into most-derived to least-derived order. If we don't
        // do this, a base exception handler can appear before a derived exception
        // handler, causing compiler warnings and resulting in the base exception
        // being marshaled instead of the derived exception.
        //
        throws.sort(Slice::DerivedToBaseCompare());

        string context = getEscapedParamName(op, "context");

        _out << sp;
        _out << nl << "public " << retS << " " << opName << spar << params
             << ("global::System.Collections.Generic.Dictionary<string, string>? " + context + " = null") << epar;
        _out << sb;
        _out << nl << "try";
        _out << sb;

        _out << nl;

        if (ret || !outParams.empty())
        {
            if (outParams.empty())
            {
                _out << "return ";
            }
            else if (ret || outParams.size() > 1)
            {
                _out << "var result_ = ";
            }
            else
            {
                _out << fixId(outParams.front()->name()) << " = ";
            }
        }
        _out << "_iceI_" << op->name() << "Async" << spar << argsAMI << context << "null"
             << "global::System.Threading.CancellationToken.None"
             << "true" << epar;

        if (ret || outParams.size() > 0)
        {
            _out << ".Result;";
        }
        else
        {
            _out << ".Wait();";
        }

        if ((ret && outParams.size() > 0) || outParams.size() > 1)
        {
            for (ParameterList::const_iterator i = outParams.begin(); i != outParams.end(); ++i)
            {
                ParameterPtr param = *i;
                _out << nl << fixId(param->name()) << " = result_." << fixId(param->name()) << ";";
            }

            if (ret)
            {
                _out << nl << "return result_." << resultStructReturnValueName(outParams) << ";";
            }
        }
        _out << eb;
        _out << nl << "catch (global::System.AggregateException ex_)";
        _out << sb;
        _out << nl << "throw ex_.InnerException!;";
        _out << eb;
        _out << eb;
    }

    // Async invocation
    for (const auto& op : ops)
    {
        vector<string> paramsAMI = getInParams(op, ns);
        vector<string> argsAMI = getInArgs(op);

        string opName = op->name();

        ParameterList inParams = op->inParameters();
        ParameterList outParams = op->outParameters();

        string context = getEscapedParamName(op, "context");
        string cancel = getEscapedParamName(op, "cancel");
        string progress = getEscapedParamName(op, "progress");

        TypePtr ret = op->returnType();

        string returnTypeS = resultType(op, ns);

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();

        //
        // Arrange exceptions into most-derived to least-derived order. If we don't
        // do this, a base exception handler can appear before a derived exception
        // handler, causing compiler warnings and resulting in the base exception
        // being marshaled instead of the derived exception.
        //
        throws.sort(Slice::DerivedToBaseCompare());

        //
        // Write the public Async method.
        //
        _out << sp;
        _out << nl << "public global::System.Threading.Tasks.Task";
        if (!returnTypeS.empty())
        {
            _out << "<" << returnTypeS << ">";
        }
        _out << " " << opName << "Async" << spar << paramsAMI
             << ("global::System.Collections.Generic.Dictionary<string, string>? " + context + " = null")
             << ("global::System.IProgress<bool>? " + progress + " = null")
             << ("global::System.Threading.CancellationToken " + cancel + " = default") << epar;

        _out << sb;
        _out << nl << "return _iceI_" << opName << "Async" << spar << argsAMI << context << progress << cancel
             << "false" << epar << ";";
        _out << eb;

        //
        // Write the Async method implementation.
        //
        _out << sp;
        _out << nl << "private global::System.Threading.Tasks.Task";
        if (!returnTypeS.empty())
        {
            _out << "<" << returnTypeS << ">";
        }
        _out << " _iceI_" << opName << "Async" << spar << getInParams(op, ns, true)
             << "global::System.Collections.Generic.Dictionary<string, string>? context"
             << "global::System.IProgress<bool>? progress"
             << "global::System.Threading.CancellationToken cancel"
             << "bool synchronous" << epar;
        _out << sb;

        string flatName = "_" + opName + "_name";
        if (op->returnsData())
        {
            _out << nl << "iceCheckTwowayOnly(" << flatName << ");";
        }
        if (returnTypeS.empty())
        {
            _out << nl << "var completed = "
                 << "new Ice.Internal.OperationTaskCompletionCallback<object>(progress, cancel);";
        }
        else
        {
            _out << nl << "var completed = "
                 << "new Ice.Internal.OperationTaskCompletionCallback<" << returnTypeS << ">(progress, cancel);";
        }

        _out << nl << "_iceI_" << opName << spar << getInArgs(op, true) << "context"
             << "synchronous"
             << "completed" << epar << ";";
        _out << nl << "return completed.Task;";

        _out << eb;

        _out << sp << nl << "private const string " << flatName << " = \"" << op->name() << "\";";

        //
        // Write the common invoke method
        //
        _out << sp << nl;
        _out << "private void _iceI_" << op->name() << spar << getInParams(op, ns, true)
             << "global::System.Collections.Generic.Dictionary<string, string>? context"
             << "bool synchronous"
             << "Ice.Internal.OutgoingAsyncCompletionCallback completed" << epar;
        _out << sb;

        if (returnTypeS.empty())
        {
            _out << nl << "var outAsync = getOutgoingAsync<object>(completed);";
        }
        else
        {
            _out << nl << "var outAsync = getOutgoingAsync<" << returnTypeS << ">(completed);";
        }

        _out << nl << "outAsync.invoke(";
        _out.inc();
        _out << nl << flatName << ",";
        _out << nl << sliceModeToIceMode(op->mode()) << ",";
        _out << nl << opFormatTypeToString(op) << ",";
        _out << nl << "context,";
        _out << nl << "synchronous";
        if (!inParams.empty())
        {
            _out << ",";
            _out << nl << "write: (Ice.OutputStream ostr) =>";
            _out << sb;
            writeMarshalUnmarshalParams(inParams, nullptr, true, ns);
            if (op->sendsClasses())
            {
                _out << nl << "ostr.writePendingValues();";
            }
            _out << eb;
        }

        if (!throws.empty())
        {
            _out << ",";
            _out << nl << "userException: (Ice.UserException ex) =>";
            _out << sb;
            _out << nl << "try";
            _out << sb;
            _out << nl << "throw ex;";
            _out << eb;

            //
            // Generate a catch block for each legal user exception.
            //
            for (ExceptionList::const_iterator i = throws.begin(); i != throws.end(); ++i)
            {
                _out << nl << "catch(" << getUnqualified(*i, ns) << ")";
                _out << sb;
                _out << nl << "throw;";
                _out << eb;
            }

            _out << nl << "catch(Ice.UserException)";
            _out << sb;
            _out << eb;

            _out << eb;
        }

        if (ret || !outParams.empty())
        {
            _out << ",";
            _out << nl << "read: (Ice.InputStream istr) =>";
            _out << sb;
            if (outParams.empty())
            {
                _out << nl << returnTypeS << " ret" << (ret->isClassType() ? " = null;" : ";");
            }
            else if (ret || outParams.size() > 1)
            {
                // Generated OpResult struct
                _out << nl << "var ret = new " << returnTypeS << "();";
            }
            else
            {
                TypePtr t = outParams.front()->type();
                _out << nl << typeToString(t, ns, (outParams.front()->optional())) << " iceP_"
                     << outParams.front()->name() << (t->isClassType() ? " = null;" : ";");
            }

            writeMarshalUnmarshalParams(outParams, op, false, ns, true);
            if (op->returnsClasses())
            {
                _out << nl << "istr.readPendingValues();";
            }

            if (!ret && outParams.size() == 1)
            {
                _out << nl << "return iceP_" << outParams.front()->name() << ";";
            }
            else
            {
                _out << nl << "return ret;";
            }
            _out << eb;
        }
        _out << ");";
        _out.dec();
        _out << eb;
    }

    _out << sp << nl << "public static " << name
         << "Prx createProxy(Ice.Communicator communicator, string proxyString) =>";
    _out.inc();
    _out << nl << "new " << name << "PrxHelper(Ice.ObjectPrxHelper.createProxy(communicator, proxyString));";
    _out.dec();

    _out << sp << nl << "public static " << name
         << "Prx? checkedCast(Ice.ObjectPrx? b, global::System.Collections.Generic.Dictionary<string, string>? ctx = "
            "null) =>";
    _out.inc();
    _out << nl << "b is not null && b.ice_isA(ice_staticId(), ctx) ? new " << name << "PrxHelper(b) : null;";
    _out.dec();

    _out << sp << nl << "public static " << name
         << "Prx? checkedCast(Ice.ObjectPrx? b, string f, global::System.Collections.Generic.Dictionary<string, "
            "string>? ctx = null) =>";
    _out.inc();
    _out << nl << "checkedCast(b?.ice_facet(f), ctx);";
    _out.dec();

    _out << sp << nl << "[return: global::System.Diagnostics.CodeAnalysis.NotNullIfNotNull(nameof(b))]";
    _out << sp << nl << "public static " << name << "Prx? uncheckedCast(Ice.ObjectPrx? b) =>";
    _out.inc();
    _out << nl << "b is not null ? new " << name << "PrxHelper(b) : null;";
    _out.dec();

    _out << sp << nl << "[return: global::System.Diagnostics.CodeAnalysis.NotNullIfNotNull(nameof(b))]";
    _out << sp << nl << "public static " << name << "Prx? uncheckedCast(Ice.ObjectPrx? b, string f) =>";
    _out.inc();
    _out << nl << "uncheckedCast(b?.ice_facet(f));";
    _out.dec();

    string scoped = p->scoped();
    StringList ids = p->ids();

    //
    // Need static-readonly for arrays in C# (not const)
    //
    _out << sp << nl << "private static readonly string[] _ids =";
    _out << sb;

    {
        StringList::const_iterator q = ids.begin();
        while (q != ids.end())
        {
            _out << nl << '"' << *q << '"';
            if (++q != ids.end())
            {
                _out << ',';
            }
        }
    }
    _out << eb << ";";

    _out << sp << nl << "public static string ice_staticId() => \"" << scoped << "\";";

    _out << sp << nl << "public static void write(Ice.OutputStream ostr, " << name << "Prx? v)";
    _out << sb;
    _out << nl << "ostr.writeProxy(v);";
    _out << eb;

    _out << sp << nl << "public static " << name << "Prx? read(Ice.InputStream istr) =>";
    _out.inc();
    _out << nl << "istr.readProxy() is Ice.ObjectPrx proxy ? new " << name << "PrxHelper(proxy) : null;";
    _out.dec();

    return true;
}

void
Slice::Gen::HelperVisitor::visitInterfaceDefEnd(const InterfaceDefPtr& p)
{
    string name = p->name();

    _out << sp;
    _out << nl << "protected override Ice.ObjectPrxHelperBase iceNewInstance(Ice.Internal.Reference reference) => new "
         << name << "PrxHelper(reference);";

    _out << sp;
    _out << nl << "private " << name << "PrxHelper(Ice.ObjectPrx proxy)";
    _out.inc();
    _out << nl << ": base(proxy)";
    _out.dec();
    _out << sb;
    _out << eb;

    _out << sp;
    _out << nl << "private " << name << "PrxHelper(Ice.Internal.Reference reference)";
    _out.inc();
    _out << nl << ": base(reference)";
    _out.dec();
    _out << sb;
    _out << eb;

    _out << eb;
}

void
Slice::Gen::HelperVisitor::visitSequence(const SequencePtr& p)
{
    string ns = getNamespace(p);
    string typeS = typeToString(p, ns);
    _out << sp;
    _out << nl << "public sealed class " << p->name() << "Helper";
    _out << sb;

    _out << sp << nl << "public static void write(Ice.OutputStream ostr, " << typeS << " v)";
    _out << sb;
    writeSequenceMarshalUnmarshalCode(_out, p, ns, "v", true, false);
    _out << eb;

    _out << sp << nl << "public static " << typeS << " read(Ice.InputStream istr)";
    _out << sb;
    _out << nl << typeS << " v;";
    writeSequenceMarshalUnmarshalCode(_out, p, ns, "v", false, false);
    _out << nl << "return v;";
    _out << eb;
    _out << eb;

    if (auto metadata = p->getMetadataArgs("cs:generic"))
    {
        string_view type = *metadata;
        if (type == "List" || type == "LinkedList" || type == "Queue" || type == "Stack")
        {
            return;
        }

        if (!p->type()->isClassType())
        {
            return;
        }

        //
        // The sequence is a custom sequence with elements of class type.
        // Emit a dummy class that causes a compile-time error if the
        // custom sequence type does not implement an indexer.
        //
        _out << sp;
        _out << nl << "public class " << p->name() << "_Tester";
        _out << sb;
        _out << nl << p->name() << "_Tester()";
        _out << sb;
        _out << nl << typeS << " test = new " << typeS << "();";
        _out << nl << "test[0] = null;";
        _out << eb;
        _out << eb;
    }
}

void
Slice::Gen::HelperVisitor::visitDictionary(const DictionaryPtr& p)
{
    TypePtr key = p->keyType();
    TypePtr value = p->valueType();

    string genericType = p->getMetadataArgs("cs:generic").value_or("Dictionary");

    string ns = getNamespace(p);
    string keyS = typeToString(key, ns);
    string valueS = typeToString(value, ns);
    string name = "global::System.Collections.Generic." + genericType + "<" + keyS + ", " + valueS + ">";

    _out << sp;
    _out << nl << "public sealed class " << p->name() << "Helper";
    _out << sb;

    _out << sp << nl << "public static void write(";
    _out.useCurrentPosAsIndent();
    _out << "Ice.OutputStream ostr,";
    _out << nl << name << " v)";
    _out.restoreIndent();
    _out << sb;
    _out << nl << "if(v == null)";
    _out << sb;
    _out << nl << "ostr.writeSize(0);";
    _out << eb;
    _out << nl << "else";
    _out << sb;
    _out << nl << "ostr.writeSize(v.Count);";
    _out << nl << "foreach(global::System.Collections.";
    _out << "Generic.KeyValuePair<" << keyS << ", " << valueS << ">";
    _out << " e in v)";
    _out << sb;
    writeMarshalUnmarshalCode(_out, key, ns, "e.Key", true);
    writeMarshalUnmarshalCode(_out, value, ns, "e.Value", true);
    _out << eb;
    _out << eb;
    _out << eb;

    _out << sp << nl << "public static " << name << " read(Ice.InputStream istr)";
    _out << sb;
    _out << nl << "int sz = istr.readSize();";
    _out << nl << name << " r = new " << name << "();";
    _out << nl << "for(int i = 0; i < sz; ++i)";
    _out << sb;
    _out << nl << keyS << " k;";
    writeMarshalUnmarshalCode(_out, key, ns, "k", false);

    if (value->isClassType())
    {
        ostringstream os;
        os << '(' << typeToString(value, ns) << " v) => { r[k] = v; }";
        writeMarshalUnmarshalCode(_out, value, ns, os.str(), false);
    }
    else
    {
        _out << nl << valueS << " v;";
        writeMarshalUnmarshalCode(_out, value, ns, "v", false);
        _out << nl << "r[k] = v;";
    }
    _out << eb;
    _out << nl << "return r;";
    _out << eb;

    _out << eb;
}

Slice::Gen::DispatcherVisitor::DispatcherVisitor(::IceInternal::Output& out) : CsVisitor(out) {}

bool
Slice::Gen::DispatcherVisitor::visitModuleStart(const ModulePtr& p)
{
    if (!p->contains<InterfaceDef>())
    {
        return false;
    }

    moduleStart(p);
    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::DispatcherVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
    moduleEnd(p);
}

bool
Slice::Gen::DispatcherVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    InterfaceList bases = p->bases();
    string name = p->name();
    string ns = getNamespace(p);

    _out << sp;
    _out << nl << "public abstract class " << name << "Disp_ : Ice.ObjectImpl, ";

    _out << fixId(name);

    _out << sb;

    for (const auto& op : p->operations())
    {
        string retS;
        vector<string> params, args;
        string opName = getDispatchParams(op, retS, params, args, ns);
        _out << sp << nl << "public abstract " << retS << " " << opName << spar << params << epar << ';';
    }

    writeInheritedOperations(p);

    _out << sp;
    _out << nl << "public override string ice_id(Ice.Current current) => ice_staticId();";

    _out << sp;
    _out << nl << "public static new string ice_staticId() => \"" << p->scoped() << "\";";

    writeDispatch(p);

    return true;
}
void
Slice::Gen::DispatcherVisitor::visitInterfaceDefEnd(const InterfaceDefPtr&)
{
    _out << eb;
}

void
Slice::Gen::DispatcherVisitor::writeDispatch(const InterfaceDefPtr& p)
{
    string name = fixId(p->name());
    string scoped = p->scoped();
    string ns = getNamespace(p);

    OperationList ops = p->operations();
    OperationList allOps = p->allOperations();
    if (!allOps.empty())
    {
        _out << sp;
        _out << nl
             << "public override global::System.Threading.Tasks.ValueTask<Ice.OutgoingResponse> "
                "dispatchAsync(Ice.IncomingRequest request) =>";
        _out.inc();
        _out << nl << "request.current.operation switch";
        _out << sb;
        for (const auto& op : allOps)
        {
            string opName = op->name();
            _out << nl << '"' << opName << "\" => " << getUnqualified(op->interface(), ns) << ".iceD_" << opName
                 << "Async(this, request),";
        }
        for (const auto& opName : {"ice_id", "ice_ids", "ice_isA", "ice_ping"})
        {
            _out << nl << '"' << opName << "\" => Ice.Object.iceD_" << opName << "Async(this, request),";
        }
        _out << nl << "_ => throw new Ice.OperationNotExistException()";
        _out << eb;
        _out << ";";
        _out.dec();
    }
}
