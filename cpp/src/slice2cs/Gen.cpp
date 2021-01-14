//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/StringUtil.h>
#include <IceUtil/FileUtil.h>
#include <Gen.h>

#include <limits>
#ifndef _WIN32
#  include <unistd.h>
#else
#  include <direct.h>
#endif

#include <IceUtil/Iterator.h>
#include <IceUtil/UUID.h>
#include <Slice/FileTracker.h>
#include <Slice/Util.h>
#include <string.h>

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

namespace
{

bool
isIdempotent(const OperationPtr& operation)
{
    // TODO: eliminate Nonmutating enumerator in the parser together with the nonmutating metadata.
    return operation->mode() != Operation::Normal;
}

bool
isDefaultInitialized(const MemberPtr& member, bool considerDefaultValue)
{
    if (considerDefaultValue && member->defaultValueType())
    {
        return true;
    }

    if (OptionalPtr::dynamicCast(member->type()))
    {
        return true;
    }

    auto st = StructPtr::dynamicCast(member->type());
    if (st)
    {
        for (auto m: st->dataMembers())
        {
            if (!isDefaultInitialized(m, false))
            {
                return false;
            }
        }
        return true;
    }

    return isValueType(member->type());
}

string
opFormatTypeToString(const OperationPtr& op)
{
    // TODO: eliminate DefaultFormat in the parser (DefaultFormat means the communicator default that was removed in
    // Ice 4.0)
    switch (op->format())
    {
        case DefaultFormat:
        case CompactFormat:
            return "default"; // same as Compact
        case SlicedFormat:
            return "ZeroC.Ice.FormatType.Sliced";
        default:
            assert(false);
    }

    return "???";
}

void
emitDeprecate(const ContainedPtr& p1, bool checkContainer, Output& out)
{
    string reason = getDeprecateReason(p1, checkContainer);
    if(!reason.empty())
    {
        out << nl << "[global::System.Obsolete(\"" << reason << "\")]";
    }
}

string
getEscapedParamName(const OperationPtr& p, const string& name)
{
    for (const auto& param : p->params())
    {
        if (param->name() == name)
        {
            return name + "_";
        }
    }
    return name;
}

string
getEscapedParamName(const ExceptionPtr& p, const string& name)
{
    for (const auto& member : p->allDataMembers())
    {
        if (member->name() == name)
        {
            return name + "_";
        }
    }
    return name;
}

bool
hasDataMemberWithName(const MemberList& dataMembers, const string& name)
{
    return find_if(dataMembers.begin(), dataMembers.end(), [name](const auto& m)
                                                           {
                                                               return m->name() == name;
                                                           }) != dataMembers.end();
}

}

Slice::CsVisitor::CsVisitor(Output& out) :
    _out(out)
{
}

Slice::CsVisitor::~CsVisitor()
{
}

void
Slice::CsVisitor::writeMarshal(const OperationPtr& operation, bool returnType)
{
    string ns = getNamespace(operation->interface());

    MemberList members = returnType ? operation->returnType() : operation->params();
    auto [requiredMembers, taggedMembers] = getSortedMembers(members);

    int bitSequenceIndex = -1;
    size_t bitSequenceSize = returnType ? operation->returnBitSequenceSize() : operation->paramsBitSequenceSize();

    if (bitSequenceSize > 0)
    {
        _out << nl << "var bitSequence = ostr.WriteBitSequence(" << bitSequenceSize << ");";
    }

    bool write11ReturnLast = returnType && operation->hasReturnAndOut() && !members.front()->tagged() &&
        requiredMembers.size() > 1;
    if (write11ReturnLast)
    {
        _out << nl << "if (ostr.Encoding != ZeroC.Ice.Encoding.V11)";
        _out << sb;
    }

    // Two loops when write11ReturnLast is true.
    for (int i = 0; i < (write11ReturnLast ? 2 : 1); i++)
    {
        if (bitSequenceSize > 0)
        {
            bitSequenceIndex = 0;
        }

        for (const auto& member : requiredMembers)
        {
            if (!member->stream())
            {
                writeMarshalCode(_out,
                                member->type(),
                                bitSequenceIndex,
                                false,
                                ns,
                                members.size() == 1 ? "value" : "value." + fieldName(member));
            }
        }

        if (bitSequenceSize > 0)
        {
            assert(static_cast<size_t>(bitSequenceIndex) == bitSequenceSize);
        }

        for (const auto& member : taggedMembers)
        {
            writeTaggedMarshalCode(_out,
                                   OptionalPtr::dynamicCast(member->type()),
                                   false,
                                   ns,
                                   members.size() == 1 ? "value" : "value." + fieldName(member), member->tag());
        }

        if (members.back()->stream())
        {
            if (members.size() == 1)
            {
                _out << nl << "return socketStream => socketStream." << streamDataWriter(members.back()->type());
                _out << "(value, cancel);";
            }
            else
            {
                _out << nl << "var streamableValue = value." + fieldName(members.back()) << ";";
            }
            _out << nl << "return socketStream => socketStream." << streamDataWriter(members.back()->type());
            _out << "(" << (members.size() == 1 ? "value" : "streamableValue") << ", cancel);";
        }

        if (i == 0 && write11ReturnLast) // only for first loop
        {
            _out << eb;
            _out << nl;
            _out << "else";
            _out << sb;

            // Repeat after updating requiredMembers
            requiredMembers.push_back(requiredMembers.front());
            requiredMembers.pop_front();
        }
    }

    if (write11ReturnLast)
    {
        _out << eb;
    }
}

void
Slice::CsVisitor::writeUnmarshal(const OperationPtr& operation, bool returnType)
{
    string ns = getNamespace(operation->interface());

    MemberList members = returnType ? operation->returnType() : operation->params();
    auto [requiredMembers, taggedMembers] = getSortedMembers(members);

    int bitSequenceIndex = -1;
    size_t bitSequenceSize = returnType ? operation->returnBitSequenceSize() : operation->paramsBitSequenceSize();

    if (bitSequenceSize > 0)
    {
        _out << nl << "var bitSequence = istr.ReadBitSequence(" << bitSequenceSize << ");";
    }

    bool read11ReturnLast = returnType && operation->hasReturnAndOut() && requiredMembers.size() > 1 &&
        !members.front()->tagged();

    if (read11ReturnLast)
    {
        _out << nl << "if (istr.Encoding != ZeroC.Ice.Encoding.V11)";
        _out << sb;
    }

    // Two loops when write11ReturnLast is true.
    for (int i = 0; i < (read11ReturnLast ? 2 : 1); i++)
    {
        if (bitSequenceSize > 0)
        {
            bitSequenceIndex = 0;
        }

        for (const auto& member : requiredMembers)
        {
            if (!member->stream())
            {
                _out << nl << paramTypeStr(member, false);
                _out << " ";
                writeUnmarshalCode(_out, member->type(), bitSequenceIndex, ns, paramName(member, "iceP_"));
            }
        }
        if (bitSequenceSize > 0)
        {
            assert(static_cast<size_t>(bitSequenceIndex) == bitSequenceSize);
        }

        for (const auto &member : taggedMembers)
        {
            _out << nl << paramTypeStr(member, false) << " ";
            writeTaggedUnmarshalCode(_out,
                                     OptionalPtr::dynamicCast(member->type()),
                                     ns,
                                     paramName(member, "iceP_"),
                                     member->tag(),
                                     nullptr);
        }

        if (members.back()->stream())
        {
            _out << nl << paramTypeStr(members.back(), false) << " " << paramName(members.back(), "iceP_");
            _out << " = socketStream." << streamDataReader(members.back()->type()) << "();";
        }

        if (members.size() == 1)
        {
            _out << nl << "return " << paramName(members.front(), "iceP_") << ";";
        }
        else
        {
            _out << nl << "return " << spar << getNames(members, "iceP_") << epar << ";";
        }

        if (i == 0 && read11ReturnLast)
        {
            _out << eb;
            _out << nl;
            _out << "else";
            _out << sb;

            // Repeat after updating requiredMembers
            requiredMembers.push_back(requiredMembers.front());
            requiredMembers.pop_front();
        }
    }

    if (read11ReturnLast)
    {
        _out << eb;
    }
}

void
Slice::CsVisitor::writeMarshalDataMembers(const MemberList& p, const string& ns, unsigned int baseTypes)
{
#ifndef NDEBUG
    int currentTag = -1; // just to verify sortMembers sorts correctly
#endif

    auto [requiredMembers, taggedMembers] = getSortedMembers(p);
    int bitSequenceIndex = -1;
    // Tagged members are encoded in a dictionary and don't count towards the optional bit sequence size.
    size_t bitSequenceSize = getBitSequenceSize(requiredMembers);
    if (bitSequenceSize > 0)
    {
        _out << nl << "var bitSequence = ostr.WriteBitSequence(" << bitSequenceSize << ");";
        bitSequenceIndex = 0;
    }

    for (const auto& member : requiredMembers)
    {
#ifndef NDEBUG
            assert(currentTag == -1);
#endif
            writeMarshalCode(_out, member->type(), bitSequenceIndex, true, ns,
                "this." + fixId(fieldName(member), baseTypes));
    }
    for (const auto& member : taggedMembers)
    {
#ifndef NDEBUG
            assert(member->tag() > currentTag);
            currentTag = member->tag();
#endif
            writeTaggedMarshalCode(_out, OptionalPtr::dynamicCast(member->type()), true, ns,
                "this." + fixId(fieldName(member), baseTypes), member->tag());
    }

    if (bitSequenceSize > 0)
    {
        assert(static_cast<size_t>(bitSequenceIndex) == bitSequenceSize);
    }
}

void
Slice::CsVisitor::writeUnmarshalDataMembers(const MemberList& p, const string& ns, unsigned int baseTypes)
{
    auto [requiredMembers, taggedMembers] = getSortedMembers(p);
    int bitSequenceIndex = -1;
    // Tagged members are encoded in a dictionary and don't count towards the optional bit sequence size.
    size_t bitSequenceSize = getBitSequenceSize(requiredMembers);
    if (bitSequenceSize > 0)
    {
        _out << nl << "var bitSequence = istr.ReadBitSequence(" << bitSequenceSize << ");";
        bitSequenceIndex = 0;
    }

    for (const auto& member : requiredMembers)
    {
        _out << nl;
        writeUnmarshalCode(_out, member->type(), bitSequenceIndex, ns,
            "this." + fixId(fieldName(member), baseTypes));
    }
    for (const auto& member : taggedMembers)
    {
        _out << nl;
        writeTaggedUnmarshalCode(_out, OptionalPtr::dynamicCast(member->type()), ns,
            "this." + fixId(fieldName(member), baseTypes), member->tag(), member);
    }

    if (bitSequenceSize > 0)
    {
        assert(static_cast<size_t>(bitSequenceIndex) == bitSequenceSize);
    }
}

string
getParamAttributes(const MemberPtr& p)
{
    string result;
    for(const auto& s : p->getAllMetadata())
    {
        static const string prefix = "cs:attribute:";
        if(s.find(prefix) == 0)
        {
            result += "[" + s.substr(prefix.size()) + "] ";
        }
    }
    return result;
}

vector<string>
getInvocationParams(const OperationPtr& op, const string& ns)
{
    vector<string> params;
    for (const auto& p : op->params())
    {
        ostringstream param;
        param << getParamAttributes(p);
        param << CsGenerator::typeToString(p->type(), ns, true, true, p->stream()) << " " << paramName(p);
        params.push_back(param.str());
    }
    params.push_back("global::System.Collections.Generic.IReadOnlyDictionary<string, string>? " +
                     getEscapedParamName(op, "context") + " = null");
    params.push_back("global::System.Threading.CancellationToken " + getEscapedParamName(op, "cancel") + " = default");
    return params;
}

vector<string>
getInvocationParamsAMI(const OperationPtr& op, const string& ns, bool defaultValues, const string& prefix = "")
{
    vector<string> params;
    for (const auto& p : op->params())
    {
        ostringstream param;
        param << getParamAttributes(p);
        param << CsGenerator::typeToString(p->type(), ns, true, true, p->stream()) << " " << paramName(p, prefix);
        params.push_back(param.str());
    }

    string context = prefix.empty() ? getEscapedParamName(op, "context") : "context";
    string progress = prefix.empty() ? getEscapedParamName(op, "progress") : "progress";
    string cancel = prefix.empty() ? getEscapedParamName(op, "cancel") : "cancel";

    if(defaultValues)
    {
        params.push_back("global::System.Collections.Generic.IReadOnlyDictionary<string, string>? " + context +
                         " = null");
        params.push_back("global::System.IProgress<bool>? " + progress + " = null");
        params.push_back("global::System.Threading.CancellationToken " + cancel + " = default");
    }
    else
    {
        params.push_back("global::System.Collections.Generic.IReadOnlyDictionary<string, string>? " + context);
        params.push_back("global::System.IProgress<bool>? " + progress);
        params.push_back("global::System.Threading.CancellationToken " + cancel);
    }
    return params;
}

vector<string>
getInvocationArgsAMI(const OperationPtr& op,
                     const string& context = "",
                     const string& progress = "null",
                     const string cancellationToken = "global::System.Threading.CancellationToken.None",
                     const string& async = "true")
{
    vector<string> args = getNames(op->params());

    if(context.empty())
    {
        args.push_back(getEscapedParamName(op, "context"));
    }
    else
    {
        args.push_back(context);
    }

    args.push_back(progress);
    args.push_back(cancellationToken);
    args.push_back(async);

    return args;
}

void
Slice::CsVisitor::emitCommonAttributes()
{
   // _out << nl << "[global::System.CodeDom.Compiler.GeneratedCode(\"slice2cs\", \"" << ICE_STRING_VERSION << "\")]";
}

void
Slice::CsVisitor::emitEditorBrowsableNeverAttribute()
{
    _out << nl << "[global::System.ComponentModel.EditorBrowsable("
         << "global::System.ComponentModel.EditorBrowsableState.Never)]";
}

void
Slice::CsVisitor::emitEqualityOperators(const string& name)
{
    _out << sp;
    _out << nl << "/// <summary>The equality operator == returns true if its operands are equal, false otherwise."
         << "</summary>";
    _out << nl << "/// <param name=\"lhs\">The left hand side operand.</param>";
    _out << nl << "/// <param name=\"rhs\">The right hand side operand.</param>";
    _out << nl << "/// <returns><c>true</c> if the operands are equal, otherwise <c>false</c>.</returns>";
    _out << nl << "public static bool operator ==(" << name << " lhs, " << name << " rhs)";
    _out << " => lhs.Equals(rhs);";

    _out << sp;
    _out << nl << "/// <summary>The inequality operator != returns true if its operands are not equal, false otherwise."
         << "</summary>";
    _out << nl << "/// <param name=\"lhs\">The left hand side operand.</param>";
    _out << nl << "/// <param name=\"rhs\">The right hand side operand.</param>";
    _out << nl << "/// <returns><c>true</c> if the operands are not equal, otherwise <c>false</c>.</returns>";
    _out << nl << "public static bool operator !=(" << name << " lhs, " << name << " rhs)";
    _out << " => !lhs.Equals(rhs);";
}

void
Slice::CsVisitor::emitCustomAttributes(const ContainedPtr& p)
{
    StringList metadata = p->getAllMetadata();
    for(StringList::const_iterator i = metadata.begin(); i != metadata.end(); ++i)
    {
        static const string prefix = "cs:attribute:";
        if(i->find(prefix) == 0)
        {
            _out << nl << '[' << i->substr(prefix.size()) << ']';
        }
    }
}

void
Slice::CsVisitor::emitTypeIdAttribute(const string& typeId)
{
    _out << nl << "[ZeroC.Ice.TypeId(\"" << typeId << "\")]";
}

string
Slice::CsVisitor::writeValue(const TypePtr& type, const string& ns)
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
            {
                return "0.0f";
            }
            case Builtin::KindDouble:
            {
                return "0.0";
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
        return typeToString(type, ns) + "." + fixId((*en->enumerators().begin())->name());
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        return "new " + typeToString(type, ns) + "()";
    }

    return "null";
}

void
Slice::CsVisitor::writeDataMemberDefaultValues(const MemberList& members, const string& ns, unsigned int baseTypes)
{
    // This helper function is called only for class/exception data members.

    for (const auto& p: members)
    {
        TypePtr memberType = p->type();
        if (p->defaultValueType())
        {
            _out << nl << "this." << fixId(fieldName(p), baseTypes) << " = ";
            writeConstantValue(_out, memberType, p->defaultValueType(), p->defaultValue(), ns);
            _out << ";";
        }
    }
}

void
Slice::CsVisitor::writeSuppressNonNullableWarnings(const MemberList& members, unsigned int baseTypes)
{
    // This helper function is called only for class/exception data members.

    for (const auto& p: members)
    {
        TypePtr memberType = p->type();
        if (!OptionalPtr::dynamicCast(memberType))
        {
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(memberType);

            if (memberType->isClassType() ||
                memberType->isInterfaceType() ||
                SequencePtr::dynamicCast(memberType) ||
                DictionaryPtr::dynamicCast(memberType) ||
                (builtin && builtin->kind() == Builtin::KindString))
            {
                // This is to suppress compiler warnings for non-nullable fields.
                _out << nl << "this." << fixId(fieldName(p), baseTypes) << " = null!;";
            }
        }
    }
}

namespace
{

//
// Convert the identifier part of a Java doc Link tag to a CSharp identifier. If the identifier
// is an interface the link should point to the corresponding generated proxy, we apply the
// case conversions required to match the C# generated code.
//
string
csharpIdentifier(const ContainedPtr& contained, const string& identifier)
{
    string ns = getNamespace(contained);
    string typeName;
    string memberName;
    string::size_type pos = identifier.find('#');
    if(pos == 0)
    {
        memberName = identifier.substr(1);
    }
    else if(pos == string::npos)
    {
        typeName = identifier;
    }
    else
    {
        typeName = identifier.substr(0, pos);
        memberName = identifier.substr(pos + 1);
    }

    // lookup the Slice definition for the identifier
    ContainedPtr definition;
    if(typeName.empty())
    {
        definition = contained;
    }
    else
    {
        TypeList types = contained->unit()->lookupTypeNoBuiltin(typeName, false, true);

        if (types.empty())
        {
            definition = nullptr;
        }
        else
        {
            TypePtr type = types.front();
            if (auto typealias = TypeAliasPtr::dynamicCast(type))
            {
                type = typealias->underlying();
            }
            definition = ContainedPtr::dynamicCast(type);
        }
    }

    ostringstream os;
    if(!definition || !normalizeCase(definition))
    {
        if(typeName == "::Ice::Object")
        {
            os << "Ice.IObjectPrx";
        }
        else
        {
            os << fixId(typeName);
        }
    }
    else
    {
        InterfaceDefPtr def = InterfaceDefPtr::dynamicCast(definition);
        if(!def)
        {
            InterfaceDeclPtr decl = InterfaceDeclPtr::dynamicCast(definition);
            if(decl)
            {
                def = decl->definition();
            }
        }

        if(def)
        {
            os << getUnqualified(fixId(definition->scope()) + interfaceName(def), ns) << "Prx";
        }
        else
        {
            typeName = fixId(typeName);
            pos = typeName.rfind(".");
            if(pos == string::npos)
            {
                os << pascalCase(fixId(typeName));
            }
            else
            {
                os << typeName.substr(0, pos) << "." << pascalCase(typeName.substr(pos + 1));
            }
        }
    }

    if(!memberName.empty())
    {
        os << "." << (definition && normalizeCase(definition) ? pascalCase(fixId(memberName)) : fixId(memberName));
    }
    string result = os.str();
    //
    // strip global:: prefix if present, it is not supported in doc comment cref attributes
    //
    const string global = "global::";
    if(result.find(global) == 0)
    {
        result = result.substr(global.size());
    }
    return result;
}

vector<string>
splitLines(const string& s)
{
    vector<string> lines;
    istringstream is(s);
    for(string line; getline(is, line, '\n');)
    {
        lines.push_back(trim(line));
    }
    return lines;
}

//
// Transform a Java doc style tag to a C# doc style tag, returns a map indexed by the C#
// tag name attribute and the value contains all the lines in the comment.
//
// @param foo is the Foo argument -> {"foo": ["foo is the Foo argument"]}
//
map<string, vector<string>>
processTag(const string& sourceTag, const string& s)
{
    map<string, vector<string>> result;
    for(string::size_type pos = s.find(sourceTag); pos != string::npos; pos = s.find(sourceTag, pos + 1))
    {
        string::size_type startIdent = s.find_first_not_of(" \t", pos + sourceTag.size());
        string::size_type endIdent = s.find_first_of(" \t", startIdent);
        string::size_type endComment = s.find_first_of("@", endIdent);
        if(endIdent != string::npos)
        {
            string ident = s.substr(startIdent, endIdent - startIdent);
            string comment = s.substr(endIdent + 1,
                                      endComment == string::npos ? endComment : endComment - endIdent - 1);
            result[ident] = splitLines(trim(comment));
        }
    }
    return result;
}

CommentInfo
processComment(const ContainedPtr& contained, const string& deprecateReason)
{
    //
    // Strip HTML markup and javadoc links that are not displayed by Visual Studio.
    //
    string data = contained->comment();
    for(string::size_type pos = data.find('<'); pos != string::npos; pos = data.find('<', pos))
    {
        string::size_type endpos = data.find('>', pos);
        if(endpos == string::npos)
        {
            break;
        }
        data.erase(pos, endpos - pos + 1);
    }

    const string link = "{@link ";
    for(string::size_type pos = data.find(link); pos != string::npos; pos = data.find(link, pos))
    {
        data.erase(pos, link.size());
        string::size_type endpos = data.find('}', pos);
        if(endpos != string::npos)
        {
            string ident = data.substr(pos, endpos - pos);
            data.erase(pos, endpos - pos + 1);
            data.insert(pos, csharpIdentifier(contained, ident));
        }
    }

    const string see = "{@see ";
    for(string::size_type pos = data.find(see); pos != string::npos; pos = data.find(see, pos))
    {
        string::size_type endpos = data.find('}', pos);
        if(endpos != string::npos)
        {
            string ident = data.substr(pos + see.size(), endpos - pos - see.size());
            data.erase(pos, endpos - pos + 1);
            data.insert(pos, "<see cref=\"" + csharpIdentifier(contained, ident) + "\"/>");
        }
    }

    CommentInfo comment;

    const string paramTag = "@param";
    const string throwsTag = "@throws";
    const string exceptionTag = "@exception";
    const string returnTag = "@return";

    string::size_type pos;
    for(pos = data.find('@'); pos != string::npos; pos = data.find('@', pos + 1))
    {
        if((data.substr(pos, paramTag.size()) == paramTag ||
            data.substr(pos, throwsTag.size()) == throwsTag ||
            data.substr(pos, exceptionTag.size()) == exceptionTag ||
            data.substr(pos, returnTag.size()) == returnTag))
        {
            break;
        }
    }

    if(pos > 0)
    {
        ostringstream os;
        os << trim(data.substr(0, pos));
        if(!deprecateReason.empty())
        {
            os << "<para>" << deprecateReason << "</para>";
        }
        comment.summaryLines = splitLines(os.str());
    }

    if(comment.summaryLines.empty() && !deprecateReason.empty())
    {
        comment.summaryLines.push_back(deprecateReason);
    }

    comment.params = processTag("@param", data);
    comment.exceptions = processTag("@throws", data);

    pos = data.find(returnTag);
    if(pos != string::npos)
    {
        pos += returnTag.size();
        string::size_type endComment = data.find("@", pos);
        comment.returnLines = splitLines(
            trim(data.substr(pos , endComment == string::npos ? endComment : endComment - pos)));
    }

    return comment;
}

}

void writeDocCommentLines(IceUtilInternal::Output& out, const vector<string>& lines)
{
    for(vector<string>::const_iterator i = lines.begin(); i != lines.end(); ++i)
    {
        if(i == lines.begin())
        {
            out << *i;
        }
        else
        {
            out << nl << "///";
            if(!i->empty())
            {
                out << " " << (*i);
            }
        }
    }
}

void writeDocCommentLines(IceUtilInternal::Output& out,
                          const vector<string>& lines,
                          const string& tag,
                          const string& name = "",
                          const string& value = "")
{
    if (!lines.empty())
    {
        out << nl << "/// <" << tag;
        if (!name.empty())
        {
            out << " " << name << "=\"" << value << "\"";
        }
        out << ">";
        writeDocCommentLines(out, lines);
        out << "</" << tag << ">";
    }
}

void
Slice::CsVisitor::writeTypeDocComment(const ContainedPtr& p, const string& deprecateReason)
{
    CommentInfo comment = processComment(p, deprecateReason);
    writeDocCommentLines(_out, comment.summaryLines, "summary");
}

void
Slice::CsVisitor::writeProxyDocComment(const InterfaceDefPtr& p, const std::string& deprecatedReason)
{
    CommentInfo comment = processComment(p, deprecatedReason);
    comment.summaryLines.insert(comment.summaryLines.cbegin(),
        "Proxy interface used to call remote Ice objects that implement Slice interface " + p->name() + ".");
    comment.summaryLines.push_back("<seealso cref=\"" + fixId(interfaceName(p)) + "\"/>.");
    writeDocCommentLines(_out, comment.summaryLines, "summary");
}

void
Slice::CsVisitor::writeServantDocComment(const InterfaceDefPtr& p, const std::string& deprecatedReason)
{
    CommentInfo comment = processComment(p, deprecatedReason);
    comment.summaryLines.insert(comment.summaryLines.cbegin(),
        "Interface used to implement servants for Slice interface " + p->name() + ".");
    comment.summaryLines.push_back("<seealso cref=\"" + interfaceName(p) + "Prx\"/>.");
    writeDocCommentLines(_out, comment.summaryLines, "summary");
}

void
Slice::CsVisitor::writeOperationDocComment(const OperationPtr& p, const string& deprecateReason,
                                           bool dispatch, bool async)
{
    CommentInfo comment = processComment(p, deprecateReason);
    writeDocCommentLines(_out, comment.summaryLines, "summary");
    writeParamDocComment(p, comment, InParam);

    auto returnType = p->returnType();

    if(dispatch)
    {
        _out << nl << "/// <param name=\"" << getEscapedParamName(p, "current")
             << "\">The Current object for the dispatch.</param>";
    }
    else
    {
        _out << nl << "/// <param name=\"" << getEscapedParamName(p, "context")
             << "\">Context map to send with the invocation.</param>";

        if(async)
        {
            _out << nl << "/// <param name=\"" << getEscapedParamName(p, "progress")
                 << "\">Sent progress provider.</param>";
        }
    }
    _out << nl << "/// <param name=\"" << getEscapedParamName(p, "cancel")
         << "\">A cancellation token that receives the cancellation requests.</param>";

    if(dispatch && p->hasMarshaledResult())
    {
        _out << nl << "/// <returns>The operation marshaled result.</returns>";
    }
    else if(async)
    {
        _out << nl << "/// <returns>The task object representing the asynchronous operation.</returns>";
    }
    else if(returnType.size() == 1)
    {
        writeDocCommentLines(_out, comment.returnLines, "returns");
    }
    else if(returnType.size() > 1)
    {
        _out << nl << "/// <returns>Named tuple with the following fields:";

        for(const auto& param : returnType)
        {
            string name = paramName(param);
            if (name == "ReturnValue" && !comment.returnLines.empty())
            {
                _out << nl << "/// <para> " << name << ": ";
                writeDocCommentLines(_out, comment.returnLines);
                _out << "</para>";
            }
            else
            {
                auto i = comment.params.find(name);
                if(i != comment.params.end())
                {
                    _out << nl << "/// <para> " << name << ": ";
                    writeDocCommentLines(_out, i->second);
                    _out << "</para>";
                }
            }
        }
        _out << "</returns>";
    }

    for(const auto& e : comment.exceptions)
    {
        writeDocCommentLines(_out, e.second, "exceptions", "cref", e.first);
    }
}

void
Slice::CsVisitor::writeParamDocComment(const OperationPtr& op, const CommentInfo& comment, ParamDir paramType)
{
    // Collect the names of the in- or -out parameters to be documented.
    MemberList parameters = (paramType == InParam) ? op->params() : op->outParameters();
    for (const auto& param : parameters)
    {
        auto i = comment.params.find(param->name());
        if(i != comment.params.end())
        {
            writeDocCommentLines(_out, i->second, "param", "name", fixId(param->name()));
        }
    }
}

void
Slice::CsVisitor::openNamespace(const ModulePtr& p, string prefix)
{
    if (prefix.empty())
    {
        if (_namespaceStack.empty())
        {
            // If it's a top-level module, check if it's itself enclosed in a namespace.
            prefix = getNamespacePrefix(p);
        }
        else
        {
            prefix = _namespaceStack.top();
        }
    }
    if (!prefix.empty())
    {
        prefix += ".";
    }

    if (p->hasOnlySubModules())
    {
        _namespaceStack.push(prefix + fixId(p->name()));
    }
    else
    {
        _out << sp;
        emitCustomAttributes(p);
        _out << nl << "namespace " << prefix << fixId(p->name());
        _out << sb;

        _namespaceStack.push("");
    }
}

void
Slice::CsVisitor::closeNamespace()
{
    if (_namespaceStack.top().empty())
    {
        _out << eb;
    }
    _namespaceStack.pop();
}

Slice::Gen::Gen(const string& base, const vector<string>& includePaths, const string& dir, bool impl) :
    _includePaths(includePaths)
{
    string fileBase = base;
    string::size_type pos = base.find_last_of("/\\");
    if(pos != string::npos)
    {
        fileBase = base.substr(pos + 1);
    }
    string file = fileBase + ".cs";
    string fileImpl = fileBase + "I.cs";

    if(!dir.empty())
    {
        file = dir + '/' + file;
        fileImpl = dir + '/' + fileImpl;
    }

    _out.open(file.c_str());
    if(!_out)
    {
        ostringstream os;
        os << "cannot open `" << file << "': " << IceUtilInternal::errorToString(errno);
        throw FileException(__FILE__, __LINE__, os.str());
    }
    FileTracker::instance()->addFile(file);

    printHeader();
    printGeneratedHeader(_out, fileBase + ".ice");

    _out << nl << "#nullable enable";
    // Disable some analyzer warnings in the generated code
    _out << nl << "#pragma warning disable SA1300 // Element must begin with upper case letter";
    _out << nl << "#pragma warning disable SA1306 // Field names must begin with lower case letter";
    _out << nl << "#pragma warning disable SA1309 // Field names must not begin with underscore";
    _out << nl << "#pragma warning disable SA1312 // Variable names must begin with lower case letter";
    _out << nl << "#pragma warning disable SA1313 // Parameter names must begin with lower case letter";
    _out << nl << "#pragma warning disable CA1707 // Remove the underscores from member name";

    _out << sp << nl << "#pragma warning disable 1591"; // See bug 3654
    if(impl)
    {
        IceUtilInternal::structstat st;
        if(!IceUtilInternal::stat(fileImpl, &st))
        {
            ostringstream os;
            os << "`" << fileImpl << "' already exists - will not overwrite";
            throw FileException(__FILE__, __LINE__, os.str());
        }

        _impl.open(fileImpl.c_str());
        if(!_impl)
        {
            ostringstream os;
            os << ": cannot open `" << fileImpl << "': " << IceUtilInternal::errorToString(errno);
            throw FileException(__FILE__, __LINE__, os.str());
        }

        FileTracker::instance()->addFile(fileImpl);
    }
}

Slice::Gen::~Gen()
{
    if(_out.isOpen())
    {
        _out << '\n';
    }
    if(_impl.isOpen())
    {
        _impl << '\n';
    }
}

void
Slice::Gen::generate(const UnitPtr& p)
{
    CsGenerator::validateMetadata(p);

    UnitVisitor unitVisitor(_out);
    p->visit(&unitVisitor, false);

    TypesVisitor typesVisitor(_out);
    p->visit(&typesVisitor, false);

    ProxyVisitor proxyVisitor(_out);
    p->visit(&proxyVisitor, false);

    DispatcherVisitor dispatcherVisitor(_out, false);
    p->visit(&dispatcherVisitor, false);

    DispatcherVisitor asyncDispatcherVisitor(_out, true);
    p->visit(&asyncDispatcherVisitor, false);

    ClassFactoryVisitor classFactoryVisitor(_out);
    p->visit(&classFactoryVisitor, false);

    CompactIdVisitor compactIdVisitor(_out);
    p->visit(&compactIdVisitor, false);

    RemoteExceptionFactoryVisitor remoteExceptionFactoryVisitor(_out);
    p->visit(&remoteExceptionFactoryVisitor, false);
}

void
Slice::Gen::generateImpl(const UnitPtr& p)
{
    ImplVisitor implVisitor(_impl);
    p->visit(&implVisitor, false);
}

void
Slice::Gen::closeOutput()
{
    _out.close();
    _impl.close();
}

void
Slice::Gen::printHeader()
{
    _out << "// Copyright (c) ZeroC, Inc. All rights reserved.\n\n";
    _out << "// Ice version " << ICE_STRING_VERSION << "\n";
}

Slice::Gen::UnitVisitor::UnitVisitor(IceUtilInternal::Output& out) :
    CsVisitor(out)
{
}

bool
Slice::Gen::UnitVisitor::visitUnitStart(const UnitPtr& p)
{
    DefinitionContextPtr dc = p->findDefinitionContext(p->topLevelFile());
    assert(dc);
    StringList globalMetadata = dc->getAllMetadata();

    static const string attributePrefix = "cs:attribute:";

    bool sep = false;
    for(StringList::const_iterator q = globalMetadata.begin(); q != globalMetadata.end(); ++q)
    {
        string::size_type pos = q->find(attributePrefix);
        if(pos == 0 && q->size() > attributePrefix.size())
        {
            if(!sep)
            {
                _out << sp;
                sep = true;
            }
            string attrib = q->substr(pos + attributePrefix.size());
            _out << nl << '[' << attrib << ']';
        }
    }
    return false;
}

Slice::Gen::TypesVisitor::TypesVisitor(IceUtilInternal::Output& out) :
    CsVisitor(out)
{
}

bool
Slice::Gen::TypesVisitor::visitModuleStart(const ModulePtr& p)
{
    if (p->hasClassDefs() || p->hasConsts() || p->hasEnums() || p->hasExceptions() || p->hasStructs())
    {
        openNamespace(p);

        // Write constants if there are any
        if (!p->consts().empty())
        {
            emitCommonAttributes();
            _out << nl << "public static partial class Constants";
            _out << sb;
            bool firstOne = true;
            for (auto q : p->consts())
            {
                if (firstOne)
                {
                    firstOne = false;
                }
                else
                {
                    _out << sp;
                }

                // TODO: doc comments

                string name = fixId(q->name());
                string ns = getNamespace(q);
                emitCustomAttributes(q);
                _out << nl << "public const " << typeToString(q->type(), ns) << " " << name << " = ";
                writeConstantValue(_out, q->type(), q->valueType(), q->value(), ns);
                _out << ";";
            }
            _out << eb;
        }
        return true;
    }
    else
    {
        // don't generate a file with an empty namespace
        return false;
    }
}

void
Slice::Gen::TypesVisitor::visitModuleEnd(const ModulePtr&)
{
    closeNamespace();
}

bool
Slice::Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = p->name();
    string scoped = fixId(p->scoped(), Slice::ObjectType);
    string ns = getNamespace(p);
    _out << sp;
    writeTypeDocComment(p, getDeprecateReason(p));

    emitCommonAttributes();
    emitTypeIdAttribute(p->scoped());
    emitCustomAttributes(p);
    _out << nl << "public partial class " << fixId(name) << " : "
         << (p->base() ? getUnqualified(p->base(), ns) : "ZeroC.Ice.AnyClass")
         << sb;
    return true;
}

void
Slice::Gen::TypesVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    string name = fixId(p->name());
    string ns = getNamespace(p);
    MemberList dataMembers = p->dataMembers();
    MemberList allDataMembers = p->allDataMembers();
    bool hasBaseClass = p->base();

    _out << sp;
    emitEditorBrowsableNeverAttribute();
    _out << nl << "public static readonly new ZeroC.Ice.InputStreamReader<" << name << "> IceReader =";
    _out.inc();
    _out << nl << "istr => istr.ReadClass<" << name << ">(IceTypeId);";
    _out.dec();

    _out << sp;
    emitEditorBrowsableNeverAttribute();
    _out << nl << "public static readonly new ZeroC.Ice.InputStreamReader<" << name << "?> IceReaderIntoNullable =";
    _out.inc();
    _out << nl << "istr => istr.ReadNullableClass<" << name << ">(IceTypeId);";
    _out.dec();

    _out << sp;
    emitEditorBrowsableNeverAttribute();
    _out << nl << "public static " << (hasBaseClass ? "new " : "") << "string IceTypeId => _iceAllTypeIds[0];";

    _out << sp;
    emitEditorBrowsableNeverAttribute();
    _out << nl << "public static readonly new ZeroC.Ice.OutputStreamWriter<" << name << "> IceWriter =";
    _out.inc();
    _out << nl << "(ostr, value) => ostr.WriteClass(value, IceTypeId);";
    _out.dec();

    _out << sp;
    emitEditorBrowsableNeverAttribute();
    _out << nl << "public static readonly new ZeroC.Ice.OutputStreamWriter<" << name << "?> IceWriterFromNullable =";
    _out.inc();
    _out << nl << "(ostr, value) => ostr.WriteNullableClass(value, IceTypeId);";
    _out.dec();

    _out << sp;
    _out << nl << "private static readonly string[] _iceAllTypeIds = ZeroC.Ice.TypeExtensions.GetAllIceTypeIds(typeof("
         << name << "));";

    bool partialInitialize = !hasDataMemberWithName(allDataMembers, "Initialize");
    if(partialInitialize)
    {
        _out << sp << nl << "partial void Initialize();";
    }

    if (allDataMembers.empty())
    {
        // There is always at least another constructor, so we need to generate the parameterless constructor.
        _out << sp;
        _out << nl << "/// <summary>Constructs a new instance of <see cref=\"" << name << "\"/>.</summary>";
        _out << nl << "public " << name << spar << epar;
        if (partialInitialize)
        {
            _out << " => Initialize();";
        }
    }
    else
    {
        // "One-shot" constructor
        _out << sp;
        _out << nl << "/// <summary>Constructs a new instance of <see cref=\"" << name << "\"/>.</summary>";
        for (const auto& member : allDataMembers)
        {
            CommentInfo comment = processComment(member, "");
            writeDocCommentLines(_out, comment.summaryLines, "param", "name", paramName(member));
        }
        _out << nl << "public " << name
             << spar
             << mapfn<MemberPtr>(allDataMembers,
                                 [&ns](const auto& i)
                                 {
                                     return typeToString(i->type(), ns) + " " + fixId(i->name());
                                 })
             << epar;
        if (hasBaseClass && allDataMembers.size() != dataMembers.size())
        {
            _out.inc();
            _out << nl << ": base" << spar;
            vector<string> baseParamNames;
            for (const auto& d : p->base()->allDataMembers())
            {
                baseParamNames.push_back(fixId(d->name()));
            }
            _out << baseParamNames << epar;
            _out.dec();
        } // else we call implicitly the parameterless constructor of the base class (if there is a base class).

        _out << sb;
        for (const auto& d: dataMembers)
        {
            _out << nl << "this." << fixId(fieldName(d), Slice::ObjectType) << " = "
                 << fixId(d->name(), Slice::ObjectType) << ";";
        }
        if (partialInitialize)
        {
            _out << nl << "Initialize();";
        }
        _out << eb;

        // Second public constructor for all data members minus those with a default initializer. Can be parameterless.
        MemberList allMandatoryDataMembers;
        for (const auto& member: allDataMembers)
        {
            if (!isDefaultInitialized(member, true))
            {
                allMandatoryDataMembers.push_back(member);
            }
        }

        if (allMandatoryDataMembers.size() < allDataMembers.size()) // else, it's identical to the first ctor.
        {
            _out << sp;
            _out << nl << "/// <summary>Constructs a new instance of <see cref=\"" << name << "\"/>.</summary>";
            for (const auto& member : allMandatoryDataMembers)
            {
                CommentInfo comment = processComment(member, "");
                writeDocCommentLines(_out, comment.summaryLines, "param", "name", paramName(member));
            }
            _out << nl << "public " << name
                << spar
                << mapfn<MemberPtr>(allMandatoryDataMembers,
                                    [&ns](const auto &i) {
                                         return typeToString(i->type(), ns) + " " + fixId(i->name());
                                    })
                << epar;
            if (hasBaseClass)
            {
                vector<string> baseParamNames;
                for (const auto& d: p->base()->allDataMembers())
                {
                    if (!isDefaultInitialized(d, true))
                    {
                        baseParamNames.push_back(fixId(d->name()));
                    }
                }
                if (!baseParamNames.empty())
                {
                    _out.inc();
                    _out << nl << ": base" << spar << baseParamNames << epar;
                    _out.dec();
                }
                // else we call implicitly the parameterless constructor of the base class.
            }
            _out << sb;
            for (const auto& d : dataMembers)
            {
                if (!isDefaultInitialized(d, true))
                {
                    _out << nl << "this." << fixId(fieldName(d), Slice::ObjectType) << " = "
                         << fixId(d->name(), Slice::ObjectType) << ";";
                }
            }
            writeDataMemberDefaultValues(dataMembers, ns, ObjectType);
            if (partialInitialize)
            {
                _out << nl << "Initialize();";
            }
            _out << eb;
        }
    }

    // protected internal constructor used for unmarshaling (always generated).
    // the factory parameter is used to distinguish this ctor from the parameterless ctor that users may want to add to
    // the partial class; it's not used otherwise.
    _out << sp;
    if (!hasBaseClass)
    {
        _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(";
        _out.inc();
        _out << nl << "\"Microsoft.Performance\","
            << nl << "\"CA1801: Review unused parameters\","
            << nl << "Justification=\"Special constructor used for Ice unmarshaling\")]";
        _out.dec();
    }
    _out << nl << "protected internal " << name << "(ZeroC.Ice.InputStream? istr)";
    if (hasBaseClass)
    {
        // We call the base class constructor to initialize the base class fields.
        _out.inc();
        _out << nl << ": base(istr)";
        _out.dec();
    }
    _out << sb;
    writeSuppressNonNullableWarnings(dataMembers, ObjectType);
    _out << eb;

    writeMarshaling(p);
    _out << eb;
}

void
Slice::Gen::TypesVisitor::writeMarshaling(const ClassDefPtr& p)
{
    string name = fixId(p->name());
    string scoped = p->scoped();
    string ns = getNamespace(p);
    ClassList allBases = p->allBases();

    // Marshaling support
    MemberList members = p->dataMembers();
    const bool basePreserved = p->inheritsMetadata("preserve-slice");
    const bool preserved = p->hasMetadata("preserve-slice");

    ClassDefPtr base = p->base();

    if(preserved && !basePreserved)
    {
        _out << sp;
        _out << nl << "protected override ZeroC.Ice.SlicedData? IceSlicedData { get; set; }";
    }

    _out << sp;
    _out << nl << "protected override void IceWrite(ZeroC.Ice.OutputStream ostr, bool firstSlice)";
    _out << sb;
    _out << nl << "if (firstSlice)";
    _out << sb;
    _out << nl << "ostr.IceStartFirstSlice(_iceAllTypeIds";
    if (preserved || basePreserved)
    {
        _out << ", IceSlicedData";
    }
    if (p->compactId() >= 0)
    {
        _out << ", compactId: " << p->compactId();
    }
    _out << ");";
    _out << eb;
    _out << nl << "else";
    _out << sb;
    _out << nl << "ostr.IceStartNextSlice(IceTypeId);";
    _out << eb;

    writeMarshalDataMembers(members, ns, 0);

    if(base)
    {
        _out << nl << "ostr.IceEndSlice(false);";
        _out << nl << "base.IceWrite(ostr, false);";
    }
    else
    {
         _out << nl << "ostr.IceEndSlice(true);"; // last slice
    }
    _out << eb;

    _out << sp;

    _out << nl << "protected override void IceRead(ZeroC.Ice.InputStream istr, bool firstSlice)";
    _out << sb;
    _out << nl << "if (firstSlice)";
    _out << sb;
    if (preserved || basePreserved)
    {
        _out << nl << "IceSlicedData = ";
    }
    else
    {
        _out << nl << "_ = ";
    }
    _out << "istr.IceStartFirstSlice();";
    _out << eb;
    _out << nl << "else";
    _out << sb;
    _out << nl << "istr.IceStartNextSlice();";
    _out << eb;

    writeUnmarshalDataMembers(members, ns, 0);

    _out << nl << "istr.IceEndSlice();";
    if (base)
    {
        _out << nl << "base.IceRead(istr, false);";
    }
    // This slice and its base slices (if any) are now fully initialized.
    if (!hasDataMemberWithName(p->allDataMembers(), "Initialize"))
    {
        _out << nl << "Initialize();";
    }
    _out << eb;
}

bool
Slice::Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string name = fixId(p->name());
    string ns = getNamespace(p);
    ExceptionPtr base = p->base();

    _out << sp;
    writeTypeDocComment(p, getDeprecateReason(p));
    emitDeprecate(p, false, _out);

    emitCommonAttributes();
    emitTypeIdAttribute(p->scoped());
    emitCustomAttributes(p);
    _out << nl << "public partial class " << name << " : ";
    if(base)
    {
        _out << getUnqualified(base, ns);
    }
    else
    {
        _out << "ZeroC.Ice.RemoteException";
    }
    _out << sb;
    return true;
}

void
Slice::Gen::TypesVisitor::visitExceptionEnd(const ExceptionPtr& p)
{
    string name = fixId(p->name());
    string ns = getNamespace(p);
    MemberList allDataMembers = p->allDataMembers();
    MemberList dataMembers = p->dataMembers();

    string messageParamName = getEscapedParamName(p, "message");
    string innerExceptionParamName = getEscapedParamName(p, "innerException");
    string retryPolicyParamName = getEscapedParamName(p, "retryPolicy");

    bool hasPublicParameterlessCtor = true;
    vector<string> allParameters;
    for (const auto& member : allDataMembers)
    {
        string memberName = fixId(member->name());
        string memberType = typeToString(member->type(), ns);
        allParameters.push_back(memberType + " " + memberName);

        if (hasPublicParameterlessCtor)
        {
            hasPublicParameterlessCtor = isDefaultInitialized(member, true);
        }
    }

    vector<string> baseParamNames;
    if (p->base())
    {
        for (const auto& member : p->base()->allDataMembers())
        {
            baseParamNames.push_back(fixId(member->name()));
        }
    }

    _out << nl << "private readonly string[] _iceAllTypeIds = ZeroC.Ice.TypeExtensions.GetAllIceTypeIds(typeof("
         << name << "));";

    // Up to 2 "one-shot" constructors
    for (int i = 0; i < 2; i++)
    {
        if (allParameters.size() > 0)
        {
            if (i == 0)
            {
                // Add retryPolicy last
                allParameters.push_back("ZeroC.Ice.RetryPolicy " + retryPolicyParamName + " = default");
                baseParamNames.push_back(retryPolicyParamName);
            }
            _out << sp;
            _out << nl << "/// <summary>Constructs a new instance of <see cref=\"" << name << "\"/>.</summary>";

            if (i > 0)
            {
                _out << nl << "/// <param name=\"" << messageParamName
                     << "\">Message that describes the exception.</param>";
            }
            for (const auto& member : p->allDataMembers())
            {
                CommentInfo comment = processComment(member, "");
                writeDocCommentLines(_out, comment.summaryLines, "param", "name", paramName(member));
            }

            if (i > 0)
            {
                _out << nl << "/// <param name=\"" << innerExceptionParamName
                     << "\">The exception that is the cause of the current exception.</param>";
            }
            _out << nl << "/// <param name=\"" << retryPolicyParamName
                 << "\">The retry policy for the exception.</param>";
            _out << nl << "public " << name << spar << allParameters << epar;
            _out.inc();
            if (baseParamNames.size() > 0)
            {
                _out << nl << ": base" << spar << baseParamNames << epar;
            }
            // else we use the base's parameterless ctor.
            _out.dec();
            _out << sb;
            for (const auto& member : dataMembers)
            {
                string memberName = fixId(fieldName(member), Slice::ExceptionType);
                _out << nl << "this." << memberName << " = " << fixId(member->name()) << ';';
            }
            _out << eb;
        }

        if (i == 0)
        {
            if (allParameters.size() > 0)
            {
                allParameters.erase(prev(allParameters.end()));
                baseParamNames.erase(prev(baseParamNames.end()));
            }
            // Insert message first
            allParameters.insert(allParameters.cbegin(), "string? " + messageParamName);
            baseParamNames.insert(baseParamNames.cbegin(), messageParamName);

            // Add innerException and retryPolicy last
            allParameters.push_back("global::System.Exception? " + innerExceptionParamName + " = null");
            baseParamNames.push_back(innerExceptionParamName);

            allParameters.push_back("ZeroC.Ice.RetryPolicy " + retryPolicyParamName + " = default");
            baseParamNames.push_back(retryPolicyParamName);
        }
    }

    // public parameterless constructor (not always generated, see class comment)
    if (hasPublicParameterlessCtor)
    {
        _out << sp;
        _out << nl << "/// <summary>Constructs a new instance of <see cref=\"" << name << "\"/>.</summary>";
        _out << nl << "/// <param name=\"" << retryPolicyParamName << "\">The retry policy for the exception.</param>";
        _out << nl << "public " << name << "(ZeroC.Ice.RetryPolicy retryPolicy = default)";
        _out.inc();
        _out << nl << ": base(retryPolicy)";
        _out.dec();
        _out << sb;
        writeDataMemberDefaultValues(dataMembers, ns, Slice::ExceptionType);
        _out << eb;
    }

    // protected internal constructor used for unmarshaling (always generated).
    _out << sp;
    _out << nl << "protected internal " << name << "(string? message, ZeroC.Ice.RemoteExceptionOrigin? origin)";
    // We call the base class constructor to initialize the base class fields.
    _out.inc();
    _out << nl << ": base(message, origin)";
    _out.dec();
    _out << sb;
    writeSuppressNonNullableWarnings(dataMembers, Slice::ExceptionType);
    _out << eb;

    string scoped = p->scoped();
    ExceptionPtr base = p->base();

    // Remote exceptions are always "preserved".

    _out << sp;
    _out << nl << "protected override void IceRead(ZeroC.Ice.InputStream istr, bool firstSlice)";
    _out << sb;
    _out << nl << "if (firstSlice)";
    _out << sb;
    _out << nl << "IceSlicedData = istr.IceStartFirstSlice();";
    _out << nl << "ConvertToUnhandled = true;";
    _out << eb;
    _out << nl << "else";
    _out << sb;
    _out << nl << "istr.IceStartNextSlice();";
    _out << eb;
    writeUnmarshalDataMembers(dataMembers, ns, Slice::ExceptionType);
    _out << nl << "istr.IceEndSlice();";

    if (base)
    {
        _out << nl << "base.IceRead(istr, false);";
    }
    _out << eb;

    _out << sp;
    _out << nl << "protected override void IceWrite(ZeroC.Ice.OutputStream ostr, bool firstSlice)";
    _out << sb;
    _out << nl << "if (firstSlice)";
    _out << sb;
    _out << nl << "ostr.IceStartFirstSlice(_iceAllTypeIds, IceSlicedData, errorMessage: Message, origin: Origin);";
    _out << eb;
    _out << nl << "else";
    _out << sb;
    _out << nl << "ostr.IceStartNextSlice(_iceAllTypeIds[0]);";
    _out << eb;
    writeMarshalDataMembers(dataMembers, ns, Slice::ExceptionType);

    if(base)
    {
        _out << nl << "ostr.IceEndSlice(false);"; // the current slice is not last slice
        _out << nl << "base.IceWrite(ostr, false);"; // the next one is not the first slice
    }
    else
    {
        _out << nl << "ostr.IceEndSlice(true);"; // this is the last slice.
    }
    _out << eb;

    _out << eb;
}

bool
Slice::Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    string name = fixId(p->name());
    string ns = getNamespace(p);
    _out << sp;

    writeTypeDocComment(p, getDeprecateReason(p));
    emitDeprecate(p, false, _out);
    emitCommonAttributes();
    emitCustomAttributes(p);
    _out << nl << "public ";
    if(p->hasMetadata("cs:readonly"))
    {
        _out << "readonly ";
    }

    _out << "partial struct " << name << " : global::System.IEquatable<" << name << ">, ZeroC.Ice.IStreamableStruct";

    _out << sb;

    _out << sp;
    _out << nl << "/// <summary>A <see cref=\"ZeroC.Ice.InputStreamReader{T}\"/> used to read <see cref=\""
         << name << "\"/> instances.</summary>";
    _out << nl << "public static readonly ZeroC.Ice.InputStreamReader<" << name << "> IceReader =";
    _out.inc();
    _out << nl << "istr => new " << name << "(istr);";
    _out.dec();

    _out << sp;
    _out << nl << "/// <summary>A <see cref=\"ZeroC.Ice.OutputStreamWriter{T}\"/> used to write <see cref=\""
         << name << "\"/> instances.</summary>";
    _out << nl << "public static readonly ZeroC.Ice.OutputStreamWriter<" << name << "> IceWriter =";
    _out.inc();
    _out << nl << "(ostr, value) => value.IceWrite(ostr);";
    _out.dec();
    return true;
}

void
Slice::Gen::TypesVisitor::visitStructEnd(const StructPtr& p)
{
    string name = fixId(p->name());
    string scope = fixId(p->scope());
    string ns = getNamespace(p);
    MemberList dataMembers = p->dataMembers();

    emitEqualityOperators(name);

    bool partialInitialize = !hasDataMemberWithName(dataMembers, "Initialize");

    _out << sp;
    _out << nl << "/// <summary>Constructs a new instance of <see cref=\"" << name << "\"/>.</summary>";
    for (const auto& member : dataMembers)
    {
        CommentInfo comment = processComment(member, "");
        writeDocCommentLines(_out, comment.summaryLines, "param", "name", paramName(member));
    }
    _out << nl << "public ";
    _out << name
         << spar
         << mapfn<MemberPtr>(dataMembers,
                             [&ns](const auto& i)
                             {
                                 return typeToString(i->type(), ns) + " " + fixId(i->name());
                             })
         << epar;
    _out << sb;
    for(const auto& i : dataMembers)
    {
        string paramName = fixId(i->name());
        string memberName = fixId(fieldName(i), Slice::ObjectType);
        _out << nl << (paramName == memberName ? "this." : "") << memberName  << " = " << paramName << ";";
    }
    if(partialInitialize)
    {
        _out << nl << "Initialize();";
    }
    _out << eb;

    _out << sp;
    _out << nl << "/// <summary>Constructs a new instance of <see cref=\"" << name << "\"/>.</summary>";
    _out << nl << "/// <param name=\"istr\">The <see cref=\"ZeroC.Ice.InputStream\"/> being used to unmarshal the "
         << "instance.</param>";
    _out << nl << "public " << name << "(ZeroC.Ice.InputStream istr)";
    _out << sb;

    writeUnmarshalDataMembers(dataMembers, ns, 0);

    if(partialInitialize)
    {
        _out << nl << "Initialize();";
    }

    _out << eb;

    _out << sp;
    _out << nl << "/// <inheritdoc/>";
    _out << nl << "public readonly override bool Equals(object? other) => other is " << name
         << " value && this.Equals(value);";

    if (!p->hasMetadata("cs:custom-equals"))
    {
        // Default implementation for Equals and GetHashCode
        _out << sp;
        _out << nl << "/// <inheritdoc/>";
        _out << nl << "public readonly bool Equals(" << name << " other)";

        _out << " =>";
        _out.inc();
        _out << nl;
        for (auto q = dataMembers.begin(); q != dataMembers.end();)
        {
            string mName = fixId(fieldName(*q), Slice::ObjectType);
            string lhs = "this." + mName;
            string rhs = "other." + mName;

            TypePtr mType = unwrapIfOptional((*q)->type());

            if (mType->isInterfaceType())
            {
                _out << "ZeroC.Ice.IObjectPrx.Equals(" << lhs << ", " << rhs << ")";
            }
            else if (SequencePtr::dynamicCast(mType))
            {
                // We always check for null values because a default-initialized struct will have null fields even for
                // non nullable fields.

                if (dataMembers.size() > 1)
                {
                    _out << "(";
                }
                _out << lhs << " == " << rhs << " ||";
                _out.inc();
                _out << nl << "(" << lhs << " != null && " << rhs << " != null && ";
                _out << "global::System.Linq.Enumerable.SequenceEqual(" << lhs << ", " << rhs << ")";
                _out << ")";
                if (dataMembers.size() > 1)
                {
                    _out << ")";
                }
                _out.dec();
            }
            else if (DictionaryPtr::dynamicCast(mType))
            {
                if (dataMembers.size() > 1)
                {
                    _out << "(";
                }
                _out << lhs << " == " << rhs << " ||";
                _out.inc();
                _out << nl << "(" << lhs << " != null && " << rhs << " != null && ";
                _out << "ZeroC.Ice.DictionaryExtensions.DictionaryEqual(" << lhs << ", " << rhs << ")";
                _out << ")";
                if (dataMembers.size() > 1)
                {
                    _out << ")";
                }
                _out.dec();
            }
            else
            {
                _out << lhs << " == " << rhs;
            }

            if (++q != dataMembers.end())
            {
                _out << " &&" << nl;
            }
            else
            {
                _out << ";";
            }
        }
        _out.dec();

        _out << sp;
        _out << nl << "/// <inheritdoc/>";
        _out << nl << "public readonly override int GetHashCode()";
        _out << sb;
        _out << nl << "var hash = new global::System.HashCode();";
        for (const auto& dataMember : dataMembers)
        {
            string obj = "this." + fixId(fieldName(dataMember), Slice::ObjectType);
            TypePtr mType = unwrapIfOptional(dataMember->type());
            if (SequencePtr::dynamicCast(mType))
            {
                _out << nl << "if (" << obj << " != null)";
                _out << sb;
                _out << nl << "hash.Add(ZeroC.Ice.EnumerableExtensions.GetSequenceHashCode(" << obj << "));";
                _out << eb;
            }
            else if (DictionaryPtr::dynamicCast(mType))
            {
                _out << nl << "if (" << obj << " != null)";
                _out << sb;
                _out << nl << "hash.Add(ZeroC.Ice.DictionaryExtensions.GetDictionaryHashCode(" << obj << "));";
                _out << eb;
            }
            else
            {
                _out << nl << "hash.Add(" << obj << ");";
            }
        }
        _out << nl << "return hash.ToHashCode();";
        _out << eb;
    }

    _out << sp;
    _out << nl << "/// <summary>Marshals the struct by writing its fields to the "
        << "<see cref=\"ZeroC.Ice.OutputStream\"/>.</summary>";
    _out << nl << "/// <param name=\"ostr\">The stream to write to.</param>";
    _out << nl << "public readonly void IceWrite(ZeroC.Ice.OutputStream ostr)";
    _out << sb;
    writeMarshalDataMembers(dataMembers, ns, 0);
    _out << eb;

    if (partialInitialize)
    {
        _out << sp;
        _out << nl << "/// <summary>The constructor calls the Initialize partial method after initializing "
             << "the fields.</summary>";
        _out << nl << "partial void Initialize();";
    }

    _out << eb;
}

void
Slice::Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    string name = fixId(p->name());
    string ns = getNamespace(p);
    string scoped = fixId(p->scoped());
    EnumeratorList enumerators = p->enumerators();

    // When the number of enumerators is smaller than the distance between the min and max values, the values are not
    // consecutive and we need to use a set to validate the value during unmarshaling.
    // Note that the values are not necessarily in order, e.g. we can use a simple range check for
    // enum E { A = 3, B = 2, C = 1 } during unmarshaling.
    const bool useSet = !p->isUnchecked() &&
        static_cast<int64_t>(enumerators.size()) < p->maxValue() - p->minValue() + 1;
    string underlying = p->underlying() ? typeToString(p->underlying(), "") : "int";

    _out << sp;
    emitDeprecate(p, false, _out);
    writeTypeDocComment(p, getDeprecateReason(p));
    emitCommonAttributes();
    emitCustomAttributes(p);
    _out << nl << "public enum " << name << " : " << underlying;
    _out << sb;
    bool firstEn = true;
    for (const auto& en : enumerators)
    {
        if (firstEn)
        {
            firstEn = false;
        }
        else
        {
            _out << ',';
            _out << sp;
        }

        writeTypeDocComment(en, getDeprecateReason(en));
        _out << nl << fixId(en->name());
        if (p->explicitValue())
        {
            _out << " = " << en->value();
        }
    }
    _out << eb;

    _out << sp;
    emitCommonAttributes();
    _out << nl << "/// <summary>Helper class for marshaling and unmarshaling <see cref=\"" << name << "\"/>.</summary>";
    _out << nl << "public static class " << p->name() << "Helper";
    _out << sb;
    if (useSet)
    {
        _out << sp;
        _out << nl << "public static readonly global::System.Collections.Generic.HashSet<" << underlying
            << "> EnumeratorValues =";
        _out.inc();
        _out << nl << "new global::System.Collections.Generic.HashSet<" << underlying << "> { ";
        firstEn = true;
        for (const auto& en : enumerators)
        {
            if (firstEn)
            {
                firstEn = false;
            }
            else
            {
                _out << ", ";
            }
            _out << en->value();
        }
        _out << " };";
        _out.dec();
    }

    _out << sp;
    _out << nl << "public static readonly ZeroC.Ice.InputStreamReader<" << name << "> IceReader = Read" << p->name()
        << ";";

    _out << sp;
    _out << nl << "public static readonly ZeroC.Ice.OutputStreamWriter<" << name << "> IceWriter = Write;";

    _out << sp;
    _out << nl << "public static " << name << " As" << p->name() << "(this " << underlying << " value) =>";
    if (p->isUnchecked())
    {
        _out << " (" << name << ")value;";
    }
    else
    {
        _out.inc();
        if (useSet)
        {
            _out << nl << "EnumeratorValues.Contains(value)";
        }
        else
        {
            _out << nl << p->minValue() << " <= value && value <= " << p->maxValue();
        }
        _out << " ? (" << name
             << ")value : throw new ZeroC.Ice.InvalidDataException($\"invalid enumerator value `{value}' for "
             << fixId(p->scoped()) << "\");";
        _out.dec();
    }

    _out << sp;
    _out << nl << "public static " << name << " Read" << p->name() << "(this ZeroC.Ice.InputStream istr) =>";
    _out.inc();
    _out << nl << "As" << p->name() << "(istr.";
    if (p->underlying())
    {
        _out << "Read" << builtinSuffix(p->underlying()) << "()";
    }
    else
    {
        _out << "ReadSize()";
    }
    _out << ");";
    _out.dec();

    _out << sp;
    _out << nl << "public static void Write(this ZeroC.Ice.OutputStream ostr, " << name << " value) =>";
    _out.inc();
    if (p->underlying())
    {
        _out << nl << "ostr.Write" << builtinSuffix(p->underlying()) << "((" << underlying << ")value);";
    }
    else
    {
        _out << nl << "ostr.WriteSize((int)value);";
    }
    _out.dec();
    _out << eb;
}

void
Slice::Gen::TypesVisitor::visitDataMember(const MemberPtr& p)
{
    ContainedPtr cont = ContainedPtr::dynamicCast(p->container());
    assert(cont);

    _out << sp;

    bool readonly = StructPtr::dynamicCast(cont) && cont->hasMetadata("cs:readonly");

    writeTypeDocComment(p, getDeprecateReason(p, true));
    emitDeprecate(p, true, _out);
    emitCustomAttributes(p);
    _out << nl << "public ";
    if(readonly)
    {
        _out << "readonly ";
    }
    _out << typeToString(p->type(), getNamespace(cont));
    _out << " " << fixId(fieldName(p), ExceptionPtr::dynamicCast(cont) ? Slice::ExceptionType : Slice::ObjectType);
    _out << ";";
}

Slice::Gen::ProxyVisitor::ProxyVisitor(IceUtilInternal::Output& out) :
    CsVisitor(out)
{
}

bool
Slice::Gen::ProxyVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasInterfaceDefs())
    {
        return false;
    }
    openNamespace(p);
    return true;
}

void
Slice::Gen::ProxyVisitor::visitModuleEnd(const ModulePtr&)
{
    closeNamespace();
}

bool
Slice::Gen::ProxyVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    string name = p->name();
    string ns = getNamespace(p);

    _out << sp;
    writeProxyDocComment(p, getDeprecateReason(p));
    emitCommonAttributes();
    emitTypeIdAttribute(p->scoped());
    emitCustomAttributes(p);
    _out << nl << "public partial interface " << interfaceName(p) << "Prx : ";

    vector<string> baseInterfaces =
        mapfn<InterfaceDefPtr>(p->bases(), [&ns](const auto& c)
                           {
                               return getUnqualified(getNamespace(c) + "." +
                                                     interfaceName(c) + "Prx", ns);
                           });

    if(baseInterfaces.empty())
    {
        baseInterfaces.push_back("ZeroC.Ice.IObjectPrx");
    }

    for(vector<string>::const_iterator q = baseInterfaces.begin(); q != baseInterfaces.end();)
    {
        _out << *q;
        if(++q != baseInterfaces.end())
        {
            _out << ", ";
        }
    }
    _out << sb;

    // Generate nested Request and Response classes if this interface has operations.

    if (!p->operations().empty())
    {
        bool generateResponseClass = false;

        _out << nl << "/// <summary>Provides a <see cref=\"ZeroC.Ice.OutgoingRequestFrame\"/> factory method for each "
             << "remote operation defined in <see cref=\"" << interfaceName(p) << "Prx\"/>.</summary>";
        _out << nl << "public static new class Request";
        _out << sb;
        for (auto operation : p->operations())
        {
            auto params = operation->params();
            size_t paramCount = params.size();
            bool inValue = false;

            _out << sp;
            _out << nl << "/// <summary>Creates an <see cref=\"ZeroC.Ice.OutgoingRequestFrame\"/> for "
                 << fixId(operationName(operation)) << " operation.</summary>";
            _out << nl << "/// <param name=\"proxy\">Proxy to the target Ice Object.</param>";
            if (paramCount > 0)
            {
                _out << nl << "/// <param name=\"args\">The remote operation arguments.</param>";
            }
            _out << nl << "/// <param name=\"context\">The context to write into the request.</param>";
            _out << nl << "/// <param name=\"cancel\">A cancellation token that receives the cancellation requests."
                 << "</param>";
            _out << nl << "public static ZeroC.Ice.OutgoingRequestFrame " << fixId(operationName(operation))
                << "(ZeroC.Ice.IObjectPrx proxy, ";

            if (paramCount > 0)
            {
                inValue = paramCount > 1;
                _out << (inValue ? "in " : "") << toTupleType(params, true) << " args" << ", ";
            }
            _out << "global::System.Collections.Generic.IReadOnlyDictionary<string, string>? context, "
                 << "global::System.Threading.CancellationToken cancel) =>";
            _out.inc();

            if (paramCount == 0)
            {
                _out << nl << "ZeroC.Ice.OutgoingRequestFrame.WithEmptyArgs(proxy, \"";
                _out << operation->name() << "\", "
                    << "idempotent: " << (operation->isIdempotent() ? "true" : "false") << ", context, cancel);";
            }
            else
            {
                _out << nl << "ZeroC.Ice.OutgoingRequestFrame.WithArgs(";
                _out.inc();
                _out << nl << "proxy,"
                     << nl << "\"" << operation->name() << "\","
                     << nl << "idempotent: " << (operation->isIdempotent() ? "true" : "false") << ","
                     << nl << "compress: " << (opCompressArgs(operation) ? "true" : "false") << ","
                     << nl << "format: " << opFormatTypeToString(operation) << ","
                     << nl << "context,"
                     << nl << (inValue ? "in " : "") << "args,"
                     << nl;
                writeOutgoingRequestWriter(operation);
                _out << "," << nl << "cancel);";
                _out.dec();
            }
            _out.dec();

            generateResponseClass = generateResponseClass || !operation->returnType().empty();
        }
        _out << eb;

        if (generateResponseClass)
        {
            _out << sp;
            _out << nl << "/// <summary>Holds a <see cref=\"ZeroC.Ice.InputStreamReader{T}\"/> for each non-void "
                 << "remote operation defined in <see cref=\"" << interfaceName(p) << "Prx\"/>.</summary>";
            _out << nl << "public static new class Response";
            _out << sb;
            for (auto operation : p->operations())
            {
                auto returns = operation->returnType();
                if (returns.size() > 0)
                {
                    _out << sp;
                    string opName = fixId(operationName(operation));
                    _out << nl << "/// <summary>The <see cref=\"ZeroC.Ice.ResponseReader{T}\"/> for the return type "
                         << "of operation " << opName << ".</summary>";
                    _out << nl << "public static " << toTupleType(returns, false) << ' ' << opName;
                    _out << "(ZeroC.Ice.IObjectPrx proxy, ZeroC.Ice.IncomingResponseFrame response) =>";
                    _out.inc();
                    _out << nl;
                    _out << "response.ReadReturnValue(proxy, ";
                    writeIncomingResponseReader(operation);
                    _out << ");";
                    _out.dec();
                }
            }
            _out << eb;
        }
    }

    return true;
}

void
Slice::Gen::ProxyVisitor::visitInterfaceDefEnd(const InterfaceDefPtr& p)
{
    string ns = getNamespace(p);
    InterfaceList bases = p->bases();

    string name = interfaceName(p) + "Prx";
    //
    // Proxy static methods
    //
    _out << sp;
    _out << nl << "/// <summary>Factory for <see cref=\"" << name << "\"/> proxies.</summary>";
    _out << nl << "public static readonly new ZeroC.Ice.ProxyFactory<" << name << "> Factory =";
    _out.inc();
    _out << nl << "(reference) => new _" << p->name() << "Prx(reference);";
    _out.dec();

    _out << sp;
    _out << nl << "/// <summary>An <see cref=\"ZeroC.Ice.InputStreamReader{T}\"/> used to read "
         << "<see cref=\"" << name << "\"/> proxies.</summary>";
    _out << nl << "public static readonly new ZeroC.Ice.InputStreamReader<" << name << "> IceReader =";
    _out.inc();
    _out << nl << "istr => istr.ReadProxy(Factory);";
    _out.dec();

    _out << sp;
    _out << nl << "// <summary>An <see cref=\"InputStreamReader{T}\"/> used to read <see cref=\"" << name
         << "\"/> nullable proxies.</summary>";
    _out << nl << "public static readonly new ZeroC.Ice.InputStreamReader<" << name << "?> IceReaderIntoNullable =";
    _out.inc();
    _out << nl << "istr => istr.ReadNullableProxy(Factory);";
    _out.dec();

    _out << sp;
    _out << nl << "/// <summary>Converts the string representation of a proxy to its <see cref=\"" << name << "\"/> "
         << "equivalent.</summary>";
    _out << nl << "/// <param name=\"s\">The proxy string representation.</param>";
    _out << nl << "/// <param name=\"communicator\">The communicator for the new proxy</param>";
    _out << nl << "/// <returns>The new proxy</returns>";
    _out << nl << "/// <exception cref=\"global::System.FormatException\"><c>s</c> does not contain a valid string "
         << "representation of a proxy.</exception>";
    _out << nl << "public static new " << name << " Parse(string s, ZeroC.Ice.Communicator communicator) => "
         << "new _" << p->name() << "Prx(ZeroC.Ice.Reference.Parse(s, communicator));";

    _out << sp;
    _out << nl << "/// <summary>Converts the string representation of a proxy to its <see cref=\"" << name
         << "\"/> equivalent.</summary>";
    _out << nl << "/// <param name=\"s\">The proxy string representation.</param>";
    _out << nl << "/// <param name=\"communicator\">The communicator for the new proxy</param>";
    _out << nl << "/// <param name=\"prx\">When this method returns it contains the new proxy, if the conversion "
         << "succeeded or null if the conversion failed.</param>";
    _out << nl << "/// <returns><c>true</c> if the s parameter was converted successfully; otherwise, <c>false</c>."
         << "</returns>";
    _out << nl << "public static bool TryParse("
         << "string s, ZeroC.Ice.Communicator communicator, "
         << "out " <<name << "? prx)";
    _out << sb;
    _out << nl << "try";
    _out << sb;
    _out << nl << "prx = new _" << p->name() << "Prx(ZeroC.Ice.Reference.Parse(s, communicator));";
    _out << eb;
    _out << nl << "catch (global::System.Exception)";
    _out << sb;
    _out << nl << "prx = null;";
    _out << nl << "return false;";
    _out << eb;
    _out << nl << "return true;";
    _out << eb;

    _out << eb;

    //
    // Proxy instance
    //
    _out << sp;
    _out << nl << "internal sealed class _" << p->name() << "Prx : ZeroC.Ice.ObjectPrx, "
         << name;
    _out << sb;

    _out << sp;
    _out << nl << "internal _" << p->name() << "Prx(ZeroC.Ice.Reference reference)";
    _out.inc();
    _out << nl << ": base(reference)";
    _out.dec();
    _out << sb;
    _out << eb;

    _out << sp;
    _out << nl << "ZeroC.Ice.IObjectPrx ZeroC.Ice.IObjectPrx.IceClone(ZeroC.Ice.Reference reference) => new _"
         << p->name() << "Prx(reference);";

    _out << eb;
}

void
Slice::Gen::ProxyVisitor::visitOperation(const OperationPtr& operation)
{
    auto returnType = operation->returnType();
    auto params = operation->params();

    InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(operation->container());
    string deprecateReason = getDeprecateReason(operation, true);

    string ns = getNamespace(interface);
    string opName = operationName(operation);
    string name = fixId(opName);
    string asyncName = opName + "Async";
    bool oneway = operation->hasMetadata("oneway");

    TypePtr ret = operation->deprecatedReturnType();
    string retS = typeToString(operation->deprecatedReturnType(), ns);

    string context = getEscapedParamName(operation, "context");
    string cancel = getEscapedParamName(operation, "cancel");
    string progress = getEscapedParamName(operation, "progress");

    bool voidOp = returnType.empty();

    // Write the synchronous version of the operation.
    _out << sp;
    writeOperationDocComment(operation, deprecateReason, false, false);
    if (!deprecateReason.empty())
    {
        _out << nl << "[global::System.Obsolete(\"" << deprecateReason << "\")]";
    }
    _out << nl << returnTypeStr(operation, ns, false) << " " << name << spar << getInvocationParams(operation, ns)
         << epar << " =>";
    _out.inc();

    _out << nl << "IceInvoke(Request." << name << "(this, ";
    if (params.size() > 0)
    {
        _out << toTuple(params) << ", ";
    }
    _out << context << ", " << cancel << "), ";
    if (voidOp)
    {
        _out << (oneway ? "oneway: true" : "IsOneway") << ");";
    }
    else
    {
        _out << "Response." << name << ");";
    }
    _out.dec();

    // Write the async version of the operation
    _out << sp;
    writeOperationDocComment(operation, deprecateReason, false, true);
    if (!deprecateReason.empty())
    {
        _out << nl << "[global::System.Obsolete(\"" << deprecateReason << "\")]";
    }

    _out << nl << returnTaskStr(operation, ns, false) << " " << asyncName << spar
        << getInvocationParamsAMI(operation, ns, true) << epar << " =>";
    _out.inc();

    _out << nl << "IceInvokeAsync(Request." << name << "(this, ";
    if (params.size() > 0)
    {
        _out << toTuple(params) << ", ";
    }
    _out << context << ", " << cancel << "), ";
    if (voidOp)
    {
        _out << (oneway ? "oneway: true" : "IsOneway") << ", ";
    }
    else
    {
        _out << "Response." << name << ", ";
    }
    _out << progress << ");";
    _out.dec();

    // TODO: move this check to the Slice parser.
    if (oneway && !voidOp)
    {
        const UnitPtr ut = operation->unit();
        const DefinitionContextPtr dc = ut->findDefinitionContext(operation->file());
        assert(dc);
        dc->error(operation->file(), operation->line(), "only void operations can be marked oneway");
    }
}

void
Slice::Gen::ProxyVisitor::writeOutgoingRequestWriter(const OperationPtr& operation)
{
    InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(operation->container());
    string ns = getNamespace(interface);

    auto params = operation->params();
    assert(!params.empty());

    // When the operation's parameter is a T? where T is an interface or a class, there is a built-in writer, so
    // defaultWriter is true.
    bool defaultWriter = params.size() == 1 && operation->paramsBitSequenceSize() == 0 && !params.front()->tagged();
    if (defaultWriter)
    {
        // This includes operations with a single struct parameter.
        if (params.front()->stream())
        {
            _out << "SocketStream.Ice" << streamDataWriter(params.front()->type());
        }
        else
        {
            _out << outputStreamWriter(params.front()->type(), ns, true, true);
        }
    }
    else
    {
        _out << "(ZeroC.Ice.OutputStream ostr, ";
        _out << (params.size() > 1 ? "in " : "") << toTupleType(params, true) << " value";
        if (params.back()->stream())
        {
            _out << ", global::System.Threading.CancellationToken cancel";
        }
        _out << ") =>";
        _out << sb;
        writeMarshal(operation, false);
        _out << eb;
    }
}

void
Slice::Gen::ProxyVisitor::writeIncomingResponseReader(const OperationPtr& operation)
{
    InterfaceDefPtr interface = operation->interface();
    string ns = getNamespace(interface);

    auto returnType = operation->returnType();

    bool defaultReader = returnType.size() == 1 && operation->returnBitSequenceSize() == 0 &&
        !returnType.front()->tagged();

    if (defaultReader)
    {
        if(returnType.front()->stream())
        {
            _out << "ZeroC.Ice.SocketStream.Ice" << streamDataReader(returnType.front()->type());
        }
        else
        {
            _out << inputStreamReader(returnType.front()->type(), ns);
        }
    }
    else if (returnType.size() > 0)
    {
        if (returnType.back()->stream())
        {
            _out << "(istr, socketStream) =>";
        }
        else
        {
            _out << "istr =>";
        }
        _out << sb;
        writeUnmarshal(operation, true);
        _out << eb;
    }
}

Slice::Gen::DispatcherVisitor::DispatcherVisitor(::IceUtilInternal::Output& out, bool generateAllAsync) :
    CsVisitor(out), _generateAllAsync(generateAllAsync)
{
}

bool
Slice::Gen::DispatcherVisitor::visitModuleStart(const ModulePtr& p)
{
    if (!p->hasInterfaceDefs())
    {
        return false;
    }

    openNamespace(p);
    return true;
}

void
Slice::Gen::DispatcherVisitor::visitModuleEnd(const ModulePtr&)
{
    closeNamespace();
}

bool
Slice::Gen::DispatcherVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    InterfaceList bases = p->bases();
    string name = interfaceName(p, _generateAllAsync);
    string ns = getNamespace(p);

    _out << sp;
    writeServantDocComment(p, getDeprecateReason(p));
    emitCommonAttributes();
    emitTypeIdAttribute(p->scoped());
    emitCustomAttributes(p);
    _out << nl << "public partial interface " << fixId(name) << " : ";
    if (bases.empty())
    {
        _out << "ZeroC.Ice.IObject";
    }
    else
    {
        for(InterfaceList::const_iterator q = bases.begin(); q != bases.end();)
        {
            _out << getUnqualified(getNamespace(*q) + "." + interfaceName(*q, _generateAllAsync), ns);
            if(++q != bases.end())
            {
                _out << ", ";
            }
        }
    }

    _out << sb;

    // Generate nested Request and Response classes if needed.
    auto operationList = p->operations();
    bool generateRequestClass =
        find_if(operationList.begin(), operationList.end(), [](const auto& op) {
            return !op->params().empty() && (op->params().size() > 1 || !op->params().front()->stream());
            }) != operationList.end();

    bool generateResponseClass =
        find_if(operationList.begin(), operationList.end(), [](const auto& op) { return !op->returnType().empty(); })
            != operationList.end();

    if (generateRequestClass)
    {
        _out << nl << "/// <summary>Holds a <see cref=\"ZeroC.Ice.InputStreamReader{T}\"/> for each remote operation "
             << "with parameter(s)";
        _out << nl << "/// defined in <see cref=\"" << name << "\"/>.</summary>";
        _out << nl << "public static new class Request";
        _out << sb;

        for (auto operation : operationList)
        {
            auto params = operation->params();
            if (!params.empty() && (params.size() > 1 || !params.front()->stream()))
            {
                string propertyName = fixId(operationName(operation));
                _out << sp;
                _out << nl << "/// <summary>The <see cref=\"ZeroC.Ice.RequestReader{T}\"/> for the parameter"
                     << (params.size() > 1 ? "s " : " ")
                     << "of operation " << propertyName << ".</summary>";

                _out << nl << "public static " << toTupleType(params, false) << ' ' << fixId(operationName(operation));
                _out << "(ZeroC.Ice.Connection connection, ZeroC.Ice.IncomingRequestFrame request) =>";
                _out.inc();
                _out << nl;
                _out << "request.ReadArgs(connection, ";
                writeIncomingRequestReader(operation);
                _out << ");";
                _out.dec();
            }
        }
        _out << eb;
        _out << sp;
    }

    if (generateResponseClass)
    {
        _out << nl << "/// <summary>Provides a <see cref=\"ZeroC.Ice.OutgoingResponseFrame\"/> factory method "
             << "for each non-void remote operation";
        _out << nl << "/// defined in the <see cref=\"" << name << "\"/>.</summary>";
        _out << nl << "public static new class Response";
        _out << sb;
        for (auto operation : operationList)
        {
            auto returns = operation->returnType();
            size_t returnCount = returns.size();

            if (returnCount > 0)
            {
                _out << sp;
                bool inValue = returnCount > 1;
                _out << nl << "/// <summary>Creates an <see cref=\"ZeroC.Ice.OutgoingResponseFrame\"/> for operation "
                     << fixId(operationName(operation)) << ".</summary>";
                _out << nl << "/// <param name=\"current\">Holds decoded header data and other information about the "
                     << "current request.</param>";
                _out << nl << "/// <param name=\"returnValue\">The return value to write into the new frame.</param>";
                _out << nl << "/// <returns>A new <see cref=\"ZeroC.Ice.OutgoingResponseFrame\"/>.</returns>";
                _out << nl << "public static ZeroC.Ice.OutgoingResponseFrame "<< fixId(operationName(operation))
                    << "(ZeroC.Ice.Current current, "
                    << (inValue ? "in " : "") << toTupleType(returns, true) << " returnValue) =>";
                _out.inc();
                _out << nl << "ZeroC.Ice.OutgoingResponseFrame.WithReturnValue(";
                _out.inc();
                _out << nl << "current,"
                    << nl << "compress: " << (opCompressReturn(operation) ? "true" : "false") << ","
                    << nl << "format: " << opFormatTypeToString(operation) << ","
                    << nl << (inValue ? "in " : "") << "returnValue,"
                    << nl;
                writeOutgoingResponseWriter(operation);
                _out << ");";
                _out.dec();
                _out.dec();
            }
        }
        _out << eb;
        _out << sp;
    }

    // The _ice prefix is in case the user "extends" the partial generated interface.
    _out << nl << "private static readonly string _iceTypeId = ZeroC.Ice.TypeExtensions.GetIceTypeId(typeof("
        << name << "))!;";
    _out << nl
        << "private static readonly string[] _iceAllTypeIds = ZeroC.Ice.TypeExtensions.GetAllIceTypeIds(typeof("
        << name << "));";

    for (const auto& op : p->operations())
    {
        writeReturnValueStruct(op);
        writeMethodDeclaration(op);
    }

    _out << sp;
    _out << nl << "global::System.Threading.Tasks.ValueTask<string> ZeroC.Ice.IObject.IceIdAsync("
         << "ZeroC.Ice.Current current, "
         << "global::System.Threading.CancellationToken cancel) => new(_iceTypeId);";
    _out << sp;
    _out << nl << "global::System.Threading.Tasks.ValueTask<global::System.Collections.Generic.IEnumerable<string>> "
         << "ZeroC.Ice.IObject.IceIdsAsync(ZeroC.Ice.Current current, "
         << "global::System.Threading.CancellationToken cancel) => new(_iceAllTypeIds);";

    _out << sp;
    _out << nl << "global::System.Threading.Tasks.ValueTask<ZeroC.Ice.OutgoingResponseFrame> ZeroC.Ice.IObject"
         << ".DispatchAsync("
         << "ZeroC.Ice.IncomingRequestFrame request, "
         << "ZeroC.Ice.Current current, "
         << "global::System.Threading.CancellationToken cancel) =>";
    _out.inc();
    _out << nl << "DispatchAsync(this, request, current, cancel);";
    _out.dec();

    _out << sp;
    _out << nl << "// This protected static DispatchAsync allows a derived class to override the instance DispatchAsync";
    _out << nl << "// and reuse the generated implementation.";
    _out << nl << "protected static global::System.Threading.Tasks.ValueTask<ZeroC.Ice.OutgoingResponseFrame> "
         << "DispatchAsync(" << fixId(name) << " servant, "
         << "ZeroC.Ice.IncomingRequestFrame request, "
         << "ZeroC.Ice.Current current, "
         << "global::System.Threading.CancellationToken cancel) =>";
    _out.inc();
    _out << nl << "current.Operation switch";
    _out << sb;
    vector<pair<string, string>> allOpNames;
    for(const auto& op : p->allOperations())
    {
        allOpNames.push_back(make_pair(op->name(), operationName(op)));
    }
    allOpNames.push_back(make_pair("ice_id", "IceId"));
    allOpNames.push_back(make_pair("ice_ids", "IceIds"));
    allOpNames.push_back(make_pair("ice_isA", "IceIsA"));
    allOpNames.push_back(make_pair("ice_ping", "IcePing"));

    for(const auto& opName : allOpNames)
    {
        _out << nl << "\"" << opName.first << "\" => " << "servant.IceD" << opName.second
             << "Async(request, current, cancel),";
    }

    _out << nl << "_ => throw new ZeroC.Ice.OperationNotExistException()";

    _out << eb << ";"; // switch expression
    _out.dec(); // method
    return true;
}

void
Slice::Gen::DispatcherVisitor::writeReturnValueStruct(const OperationPtr& operation)
{
    InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(operation->container());
    string ns = getNamespace(interface);
    const string opName = pascalCase(operation->name());
    const string name = opName + "MarshaledReturnValue";
    auto returnType = operation->returnType();

    if (operation->hasMarshaledResult())
    {
        _out << sp;
        _out << nl << "/// <summary>Helper struct used to marshal the return value of " << opName << " operation."
             << "</summary>";
        _out << nl << "public struct " << name << " : global::System.IEquatable<" << name << ">";
        _out << sb;
        _out << nl << "/// <summary>The frame holding the marshaled response.</summary>";
        _out << nl << "public ZeroC.Ice.OutgoingResponseFrame Response { get; }";

        emitEqualityOperators(name);
        _out << sp;

        _out << nl << "/// <summary>Constructs a new <see cref=\"" << name  << "\"/> instance that";
        _out << nl << "/// immediately marshals the return value of operation " << opName << ".</summary>";
        _out << nl << "public " << name << spar
             << getNames(returnType, [](const auto& p)
                                    {
                                        return paramTypeStr(p) + " " + paramName(p);
                                    })
             << ("ZeroC.Ice.Current " + getEscapedParamName(operation, "current"))
             << epar;
        _out << sb;
        _out << nl << "Response = ZeroC.Ice.OutgoingResponseFrame.WithReturnValue(";
        _out.inc();
        _out << nl << getEscapedParamName(operation, "current") << ", "
             << "compress: " << (opCompressReturn(operation) ? "true" : "false") << ", "
             << "format: " << opFormatTypeToString(operation) << ", "
             << toTuple(returnType) << ",";
        if(returnType.size() > 1)
        {
            _out << nl << "(ZeroC.Ice.OutputStream ostr, " << toTupleType(returnType, true) << " value) =>";
            _out << sb;
            writeMarshal(operation, true);
            _out << eb;
        }
        else
        {
            _out << nl << "(ostr, value) =>";
            _out << sb;
            writeMarshal(operation, true);
            _out << eb;
        }
        _out << ");";
        _out.dec();
        _out << eb;

        _out << sp;
        _out << nl << "/// <inheritdoc/>";
        _out << nl << "public bool Equals(" << name << " other) => Response == other.Response;";

        _out << sp;
        _out << nl << "/// <inheritdoc/>";
        _out << nl << "public override bool Equals(object? other) => other is " << name << " value && Equals(value);";

        _out << sp;
        _out << nl << "/// <inheritdoc/>";
        _out << nl << "public override int GetHashCode() => Response.GetHashCode();";

        _out << eb;
    }
}

void
Slice::Gen::DispatcherVisitor::writeMethodDeclaration(const OperationPtr& operation)
{
    InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(operation->container());
    string ns = getNamespace(interface);
    string deprecateReason = getDeprecateReason(operation, true);
    bool amd = _generateAllAsync || interface->hasMetadata("amd") || operation->hasMetadata("amd");
    const string name = fixId(operationName(operation) + (amd ? "Async" : ""));

    _out << sp;
    writeOperationDocComment(operation, deprecateReason, true, amd);
    _out << nl << "public ";

    if(amd)
    {
        _out << returnTaskStr(operation, ns, true);
    }
    else
    {
        _out << returnTypeStr(operation, ns, true);
    }

    _out << " " << name << spar;
    _out << getNames(operation->params(),
                     [](const auto& param)
                     {
                        return paramTypeStr(param, false) + " " + paramName(param);
                     });
    _out << ("ZeroC.Ice.Current " + getEscapedParamName(operation, "current"));
    _out << ("global::System.Threading.CancellationToken " + getEscapedParamName(operation, "cancel"));
    _out << epar << ';';
}

void
Slice::Gen::DispatcherVisitor::visitOperation(const OperationPtr& operation)
{
    InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(operation->container());
    bool amd = _generateAllAsync || interface->hasMetadata("amd") || operation->hasMetadata("amd");
    string ns = getNamespace(interface);
    string opName = operationName(operation);
    string name = fixId(opName + (amd ? "Async" : ""));
    string internalName = "IceD" + opName + "Async";

    auto params = operation->params();
    auto returnType = operation->returnType();

    _out << sp;
    _out << nl << "protected ";
    if (amd)
    {
        _out << "async ";
    }
    _out << "global::System.Threading.Tasks.ValueTask<ZeroC.Ice.OutgoingResponseFrame>";
    _out << " " << internalName << "("
         << "ZeroC.Ice.IncomingRequestFrame request, "
         << "ZeroC.Ice.Current current, "
         << "global::System.Threading.CancellationToken cancel)";
    _out << sb;

    if (!isIdempotent(operation))
    {
         _out << nl << "IceCheckNonIdempotent(current);";
    }

    // Even when the parameters are empty, we verify the encapsulation is indeed empty (can contain tagged params
    // that we skip).
    if (params.empty())
    {
        _out << nl << "request.ReadEmptyArgs();";
    }
    else if(params.size() == 1 && params.front()->stream())
    {
        _out << nl << "var " << paramName(params.front(), "iceP_") << " = request.ReadArgs(";
        _out << "ZeroC.Ice.SocketStream.Ice" << streamDataReader(params.front()->type());
        _out << ");";
    }
    else
    {
        _out << nl << "var " << (params.size() == 1 ? paramName(params.front(), "iceP_") : "args")
            << " = Request." << fixId(opName) << "(current.Connection, request);";
    }

    // The 'this.' is necessary only when the operation name matches one of our local variable (current, istr etc.)

    if (operation->hasMarshaledResult())
    {
        if (amd)
        {
            _out << nl << "var returnValue = await this." << name << spar;
            if (params.size() > 1)
            {
                _out << getNames(params, [](const MemberPtr& param) { return "args." + fieldName(param); });
            }
            else if (params.size() == 1)
            {
                _out << paramName(params.front(), "iceP_");
            }
            _out << "current" << "cancel" << epar << ".ConfigureAwait(false);";
            _out << nl << "return returnValue.Response;";
        }
        else
        {
            _out << nl << "return new global::System.Threading.Tasks.ValueTask<ZeroC.Ice.OutgoingResponseFrame>(this."
                 << name << spar;
            if (params.size() > 1)
            {
                _out << getNames(params, [](const MemberPtr& param) { return "args." + fieldName(param); });
            }
            else if (params.size() == 1)
            {
                _out << paramName(params.front(), "iceP_");
            }
            _out << "current" << "cancel" << epar << ".Response);";
        }
        _out << eb;
    }
    else
    {
        _out << nl;
        if (returnType.size() >= 1)
        {
            _out << "var returnValue = ";
        }

        if (amd)
        {
            _out << "await ";
        }
        _out << "this." << name << spar;
        if (params.size() > 1)
        {
            _out << getNames(params, [](const MemberPtr& param) { return "args." + fieldName(param); });
        }
        else if (params.size() == 1)
        {
            _out << paramName(params.front(), "iceP_");
        }
        _out << "current" << "cancel" << epar;
        if (amd)
        {
            _out << ".ConfigureAwait(false)";
        }
        _out << ";";

        if (returnType.size() == 0)
        {
            if (amd)
            {
                _out << nl << "return ZeroC.Ice.OutgoingResponseFrame.WithVoidReturnValue(current);";
            }
            else
            {
                _out << nl << "return new global::System.Threading.Tasks.ValueTask<ZeroC.Ice.OutgoingResponseFrame>(";
                _out.inc();
                _out << nl << "ZeroC.Ice.OutgoingResponseFrame.WithVoidReturnValue(current));";
                _out.dec();
            }
        }
        else
        {
            _out << nl << "return ";
            if (!amd)
            {
                _out << "new global::System.Threading.Tasks.ValueTask<ZeroC.Ice.OutgoingResponseFrame>(";
                _out.inc();
                _out << nl;
            }
            _out << "Response." << fixId(opName) << "(current, returnValue)";

            if (!amd)
            {
                _out << ")";
                _out.dec();
            }
            _out << ";";
        }
        _out << eb;
    }
}

void
Slice::Gen::DispatcherVisitor::visitInterfaceDefEnd(const InterfaceDefPtr&)
{
    _out << eb; // interface
}

void
Slice::Gen::DispatcherVisitor::writeIncomingRequestReader(const OperationPtr& operation)
{
    InterfaceDefPtr interface = operation->interface();
    string ns = getNamespace(interface);

    auto params = operation->params();
    bool defaultReader = params.size() == 1 && operation->paramsBitSequenceSize() == 0 && !params.front()->tagged();

    if (defaultReader)
    {
        if (params.front()->stream())
        {
            _out << "ZeroC.Ice.SocketStream.Ice" << streamDataReader(params.front()->type());
        }
        else
        {
            _out << inputStreamReader(params.front()->type(), ns);
        }
    }
    else if (params.size() > 0)
    {
        if (params.back()->stream())
        {
            _out << "(istr, socketStream) =>";
        }
        else
        {
            _out << "istr =>";
        }
        _out << sb;
        writeUnmarshal(operation, false);
        _out << eb;
    }
}

void
Slice::Gen::DispatcherVisitor::writeOutgoingResponseWriter(const OperationPtr& operation)
{
    InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(operation->container());
    string ns = getNamespace(interface);

    auto returns = operation->returnType();
    assert(!returns.empty());

    // When the operation returns a T? where T is an interface or a class, there is a built-in writer, so defaultWriter
    // is true.
    bool defaultWriter = returns.size() == 1 && operation->returnBitSequenceSize() == 0 && !returns.front()->tagged();
    if (defaultWriter)
    {
        // This includes operations with a single struct return type.
        if (returns.front()->stream())
        {
            _out << "SocketStream.Ice" << streamDataWriter(returns.front()->type());
        }
        else
        {
            _out << outputStreamWriter(returns.front()->type(), ns, true, true);
        }
    }
    else
    {
        _out << "(ZeroC.Ice.OutputStream ostr, ";
        _out << (returns.size() > 1 ? "in " : "") << toTupleType(returns, true) << " value";
        if(returns.back()->stream())
        {
            _out << ", global::System.Threading.CancellationToken cancel";
        }
        _out << ") =>";
        _out << sb;
        writeMarshal(operation, true);
        _out << eb;
    }
}

Slice::Gen::ImplVisitor::ImplVisitor(IceUtilInternal::Output& out) :
    CsVisitor(out)
{
}

bool
Slice::Gen::ImplVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasInterfaceDefs())
    {
        return false;
    }

    openNamespace(p);
    return true;
}

void
Slice::Gen::ImplVisitor::visitModuleEnd(const ModulePtr&)
{
    closeNamespace();
}

bool
Slice::Gen::ImplVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    _out << sp << nl << "public class " << p->name() << "I : " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::ImplVisitor::visitOperation(const OperationPtr& op)
{
    InterfaceDefPtr interface = op->interface();
    string ns = getNamespace(interface);
    string opName = operationName(op);

    auto returnType = op->returnType();

    _out << sp << nl;

    if(interface->hasMetadata("amd") || op->hasMetadata("amd"))
    {
        _out << "public override " << returnTaskStr(op, ns, true) << " " << opName << "Async" << spar
             << getNames(op->params())
             << ("ZeroC.Ice.Current " + getEscapedParamName(op, "current"))
             << epar;
        _out << sb;

        for(const auto& p : returnType)
        {
            _out << nl << paramTypeStr(p, true) << " " << paramName(p) << " = " << writeValue(p->type(), ns)
                << ';';
        }

        if(returnType.size() == 0)
        {
            _out << nl << "global::System.Threading.Tasks.Task.CompletedTask;";
        }
        else if(op->hasMarshaledResult() || returnType.size() > 1)
        {
            _out << nl << "return new " << opName << (op->hasMarshaledResult() ? "MarshaledResult" : "Result")
                 << spar << getNames(returnType);

            if(op->hasMarshaledResult())
            {
                _out << ("ZeroC.Ice.Current " + getEscapedParamName(op, "current"));
            }
            _out << epar << ";";
        }
        else
        {
            _out << nl << "return " << paramName(returnType.front()) << ";";
        }
        _out << eb;
    }
    else
    {
        _out << "public override " << returnTypeStr(op, ns, true) << " " << opName << spar
             << getNames(op->params())
             << ("ZeroC.Ice.Current " + getEscapedParamName(op, "current"))
             << epar;
        _out << sb;

        if(op->hasMarshaledResult())
        {
            _out << nl << "return new " << opName << "MarshaledResult"
                 << spar << getNames(returnType)
                 << ("ZeroC.Ice.Current " + getEscapedParamName(op, "current"))
                 << epar << ";";
        }
        else
        {
            // TODO: return tuple!
        }
        _out << eb;
    }
}

void
Slice::Gen::ImplVisitor::visitInterfaceDefEnd(const InterfaceDefPtr&)
{
    _out << eb;
}

Slice::Gen::ClassFactoryVisitor::ClassFactoryVisitor(IceUtilInternal::Output& out) :
    CsVisitor(out)
{
}

bool
Slice::Gen::ClassFactoryVisitor::visitModuleStart(const ModulePtr& p)
{
    if (p->hasClassDefs())
    {
        string prefix;
        // We are generating code for a top-level module
        if (!ContainedPtr::dynamicCast(p->container()))
        {
            prefix = "ZeroC.Ice.ClassFactory";
        }
        openNamespace(p, prefix);
        return true;
    }
    else
    {
        return false;
    }
}

void
Slice::Gen::ClassFactoryVisitor::visitModuleEnd(const ModulePtr&)
{
    closeNamespace();
}

bool
Slice::Gen::ClassFactoryVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = fixId(p->name());
    _out << sp;
    emitCommonAttributes();
    emitEditorBrowsableNeverAttribute();
    _out << nl << "public static class " << name;
    _out << sb;

    // If the enclosing namespace starts with ::ZeroC::, add global:: prefix to ZeroC type references.
    string ns = getNamespace(p);
    string prefix = ns.rfind("ZeroC.", 0) == 0 ? "global::" : "";

    _out << nl << "public static " << prefix << "ZeroC.Ice.AnyClass Create() =>";
    _out.inc();
    _out << nl << "new global::" << ns << "." << name << "((" << prefix << "ZeroC.Ice.InputStream?)null);";
    _out.dec();
    _out << eb;

    return false;
}

Slice::Gen::CompactIdVisitor::CompactIdVisitor(IceUtilInternal::Output& out) :
    CsVisitor(out)
{
}

bool
Slice::Gen::CompactIdVisitor::visitUnitStart(const UnitPtr& p)
{
    // The CompactIdVisitor does not visit modules, only the unit.
    if (p->hasCompactTypeId())
    {
        _out << sp << nl << "namespace ZeroC.Ice.ClassFactory";
        _out << sb;
        return true;
    }
    return false;
}

void
Slice::Gen::CompactIdVisitor::visitUnitEnd(const UnitPtr&)
{
    _out << eb;
}

bool
Slice::Gen::CompactIdVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if (p->compactId() >= 0)
    {
        _out << sp;
        emitCommonAttributes();
        emitEditorBrowsableNeverAttribute();

        // If the enclosing namespace starts with ::ZeroC::, add global:: prefix to ZeroC type references.
        string ns = getNamespace(p);
        string prefix = ns.rfind("ZeroC.", 0) == 0 ? "global::" : "";

        _out << nl << "public static class CompactId_" << p->compactId();
        _out << sb;
        _out << nl << "public static " << prefix << "ZeroC.Ice.AnyClass Create() =>";
        _out.inc();
        _out << nl << "new global::" << ns << "." << fixId(p->name()) << "((" << prefix
            << "ZeroC.Ice.InputStream?)null);";
        _out.dec();
        _out << eb;
    }
    return false;
}

Slice::Gen::RemoteExceptionFactoryVisitor::RemoteExceptionFactoryVisitor(IceUtilInternal::Output& out) :
    CsVisitor(out)
{
}

bool
Slice::Gen::RemoteExceptionFactoryVisitor::visitModuleStart(const ModulePtr& p)
{
    if (p->hasExceptions())
    {
        string prefix;
        // We are generating code for a top-level module
        if (!ContainedPtr::dynamicCast(p->container()))
        {
            prefix = "ZeroC.Ice.RemoteExceptionFactory";
        }
        openNamespace(p, prefix);
        return true;
    }
    else
    {
        return false;
    }
}

void
Slice::Gen::RemoteExceptionFactoryVisitor::visitModuleEnd(const ModulePtr&)
{
    closeNamespace();
}

bool
Slice::Gen::RemoteExceptionFactoryVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string name = fixId(p->name());
    _out << sp;
    emitCommonAttributes();
    emitEditorBrowsableNeverAttribute();

    // If the enclosing namespace starts with ::ZeroC::, add global:: prefix to ZeroC type references.
    string ns = getNamespace(p);
    string prefix = ns.rfind("ZeroC.", 0) == 0 ? "global::" : "";

    _out << nl << "public static class " << name;
    _out << sb;
    _out << nl << "public static " << prefix << "ZeroC.Ice.RemoteException Create(string? message, "
         << prefix << "ZeroC.Ice.RemoteExceptionOrigin? origin) =>";
    _out.inc();
    _out << nl << "new global::" << ns << "." << name << "(message, origin);";
    _out.dec();
    _out << eb;
    return false;
}
