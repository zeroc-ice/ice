// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <JsUtil.h>
#include <Slice/Util.h>
#include <IceUtil/Functional.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#endif

#ifndef _WIN32
#include <unistd.h>
#endif

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

static string
lookupKwd(const string& name, bool mangleCasts = false)
{
    //
    // Keyword list. *Must* be kept in alphabetical order.
    //
    static const string keywordList[] =
    {
        "break", "case", "catch", "class", "const", "continue", "debugger", "default", "delete", "do", "else",
        "enum", "export", "extends", "false", "finally", "for", "function", "if", "implements", "import", "in",
        "instanceof", "interface", "let", "new", "null", "package", "private", "protected", "public", "return",
        "static", "super", "switch", "this", "throw", "true", "try", "typeof", "var", "void", "while", "with",
        "yield"
    };
    bool found = binary_search(&keywordList[0],
                               &keywordList[sizeof(keywordList) / sizeof(*keywordList)],
                               name,
                               Slice::CICompare());
    if(found)
    {
        return "_" + name;
    }
    if(mangleCasts && (name == "checkedCast" || name == "uncheckedCast"))
    {
        return "_" + name;
    }
    return name;
}

//
// Split a scoped name into its components and return the components as a list of (unscoped) identifiers.
//
static StringList
splitScopedName(const string& scoped)
{
    assert(scoped[0] == ':');
    StringList ids;
    string::size_type next = 0;
    string::size_type pos;
    while((pos = scoped.find("::", next)) != string::npos)
    {
        pos += 2;
        if(pos != scoped.size())
        {
            string::size_type endpos = scoped.find("::", pos);
            if(endpos != string::npos)
            {
                ids.push_back(scoped.substr(pos, endpos - pos));
            }
        }
        next = pos;
    }
    if(next != scoped.size())
    {
        ids.push_back(scoped.substr(next));
    }
    else
    {
        ids.push_back("");
    }

    return ids;
}

static StringList
fixIds(const StringList& ids)
{
    StringList newIds;
    for(StringList::const_iterator i = ids.begin(); i != ids.end(); ++i)
    {
        newIds.push_back(lookupKwd(*i));
    }
    return newIds;
}

static string
fixSuffix(const string& param)
{
    const string thisSuffix = "this.";
    string p = param;
    if(p.find(thisSuffix) == 0)
    {
        p = "self." + p.substr(thisSuffix.size());
    }
    return p;
}

bool
Slice::JsGenerator::isClassType(const TypePtr& type)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    return (builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(type);
}

//
// If the passed name is a scoped name, return the identical scoped name,
// but with all components that are JS keywords replaced by
// their "_"-prefixed version; otherwise, if the passed name is
// not scoped, but a JS keyword, return the "_"-prefixed name.
//
string
Slice::JsGenerator::fixId(const string& name, bool mangleCasts)
{
    if(name.empty())
    {
        return name;
    }
    if(name[0] != ':')
    {
        return lookupKwd(name, mangleCasts);
    }

    const StringList ids = splitScopedName(name);
    const StringList newIds = fixIds(ids);

    stringstream result;
    for(StringList::const_iterator j = newIds.begin(); j != newIds.end(); ++j)
    {
        if(j != newIds.begin())
        {
            result << '.';
        }
        result << *j;
    }
    return result.str();
}

string
Slice::JsGenerator::fixId(const ContainedPtr& cont, bool mangleCasts)
{
    return fixId(cont->name(), mangleCasts);
}

string
Slice::JsGenerator::getOptionalFormat(const TypePtr& type)
{
    BuiltinPtr bp = BuiltinPtr::dynamicCast(type);
    if(bp)
    {
        switch(bp->kind())
        {
        case Builtin::KindByte:
        case Builtin::KindBool:
        {
            return "Ice.OptionalFormat.F1";
        }
        case Builtin::KindShort:
        {
            return "Ice.OptionalFormat.F2";
        }
        case Builtin::KindInt:
        case Builtin::KindFloat:
        {
            return "Ice.OptionalFormat.F4";
        }
        case Builtin::KindLong:
        case Builtin::KindDouble:
        {
            return "Ice.OptionalFormat.F8";
        }
        case Builtin::KindString:
        {
            return "Ice.OptionalFormat.VSize";
        }
        case Builtin::KindObject:
        {
            return "Ice.OptionalFormat.Class";
        }
        case Builtin::KindObjectProxy:
        {
            return "Ice.OptionalFormat.FSize";
        }
        case Builtin::KindLocalObject:
        {
            assert(false);
            break;
        }
        }
    }

    if(EnumPtr::dynamicCast(type))
    {
        return "Ice.OptionalFormat.Size";
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        return seq->type()->isVariableLength() ? "Ice.OptionalFormat.FSize" : "Ice.OptionalFormat.VSize";
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    if(d)
    {
        return (d->keyType()->isVariableLength() || d->valueType()->isVariableLength()) ?
            "Ice.OptionalFormat.FSize" : "Ice.OptionalFormat.VSize";
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        return st->isVariableLength() ? "Ice.OptionalFormat.FSize" : "Ice.OptionalFormat.VSize";
    }

    if(ProxyPtr::dynamicCast(type))
    {
        return "Ice.OptionalFormat.FSize";
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    assert(cl);
    return "Ice.OptionalFormat.Class";
}

string
Slice::JsGenerator::getStaticId(const TypePtr& type)
{
    BuiltinPtr b = BuiltinPtr::dynamicCast(type);
    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);

    assert((b && b->kind() == Builtin::KindObject) || cl);

    if(b)
    {
        return "Ice.ObjectImpl.ice_staticId()";
    }
    else if(cl->isInterface())
    {
        ContainedPtr cont = ContainedPtr::dynamicCast(cl->container());
        assert(cont);
        return fixId(cont->scoped()) + "." + cl->name() + "Disp_.ice_staticId()";
    }
    else
    {
        return fixId(cl->scoped()) + ".ice_staticId()";
    }
}

string
Slice::JsGenerator::typeToString(const TypePtr& type, bool optional)
{
    if(!type)
    {
        return "void";
    }

    static const char* builtinTable[] =
    {
        "Number",           // byte
        "Boolean",          // bool
        "Number",           // short
        "Number",           // int
        "Number",           // long
        "Number",           // float
        "Number",           // double
        "String",
        "Ice.Object",
        "Ice.ObjectPrx",
        "Object"
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        return builtinTable[builtin->kind()];
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        return fixId(proxy->_class()->scoped() + "Prx");
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        return typeToString(seq->type()) + "[]";
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    if(d)
    {
        return "Ice.HashMap";
    }

    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
    {
        return fixId(contained->scoped());
    }

    return "???";
}

string
Slice::JsGenerator::getLocalScope(const string& scope)
{
    assert(!scope.empty());

    //
    // Remove trailing "::" if present.
    //
    string fixedScope;
    if(scope[scope.size() - 1] == ':')
    {
        assert(scope[scope.size() - 2] == ':');
        fixedScope = scope.substr(0, scope.size() - 2);
    }
    else
    {
        fixedScope = scope;
    }
    if(fixedScope.empty())
    {
        return "";
    }
    const StringList ids = fixIds(splitScopedName(fixedScope));

    //
    // Return local scope for "::A::B::C" as A.B.C
    //
    stringstream result;
    for(StringList::const_iterator i = ids.begin(); i != ids.end(); ++i)
    {
        if(i != ids.begin())
        {
            result << '.';
        }
        result << *i;
    }
    return result.str();
}

string
Slice::JsGenerator::getReference(const string& scope, const string& target)
{
    //
    // scope and target should be fully-qualified symbols.
    //
    assert(!scope.empty() && scope[0] == ':' && !target.empty() && target[0] == ':');

    //
    // Check whether the target is in the given scope.
    //
    if(target.find(scope) == 0)
    {
        //
        // Remove scope from target, but keep the leading "::".
        //
        const string rem = target.substr(scope.size() - 2);
        assert(!rem.empty());
        const StringList ids = fixIds(splitScopedName(rem));
        stringstream result;
        result << getLocalScope(scope);
        for(StringList::const_iterator i = ids.begin(); i != ids.end(); ++i)
        {
            result << '.' << *i;
        }
        return result.str();
    }
    else
    {
        return fixId(target);
    }
}

void
Slice::JsGenerator::writeMarshalUnmarshalCode(Output &out,
                                              const TypePtr& type,
                                              const string& param,
                                              bool marshal)
{
    string stream = marshal ? "__os" : "__is";

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindByte:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeByte(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readByte()" << ';';
                }
                return;
            }
            case Builtin::KindBool:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeBool(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readBool()" << ';';
                }
                return;
            }
            case Builtin::KindShort:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeShort(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readShort()" << ';';
                }
                return;
            }
            case Builtin::KindInt:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeInt(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readInt()" << ';';
                }
                return;
            }
            case Builtin::KindLong:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeLong(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readLong()" << ';';
                }
                return;
            }
            case Builtin::KindFloat:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeFloat(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readFloat()" << ';';
                }
                return;
            }
            case Builtin::KindDouble:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeDouble(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readDouble()" << ';';
                }
                return;
            }
            case Builtin::KindString:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeString(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readString()" << ';';
                }
                return;
            }
            case Builtin::KindObject:
            {
                // Handle by isClassType below.
                break;
            }
            case Builtin::KindObjectProxy:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeProxy(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readProxy();";
                }
                return;
            }
            case Builtin::KindLocalObject:
            {
                assert(false);
                return;
            }
        }
    }

    if(EnumPtr::dynamicCast(type))
    {
        if(marshal)
        {
            out << nl << typeToString(type) << ".__write(" << stream << ", " << param << ");";
        }
        else
        {
            out << nl << param << " = " << typeToString(type) << ".__read(" << stream << ");";
        }
        return;
    }

    if(ProxyPtr::dynamicCast(type) || StructPtr::dynamicCast(type))
    {
        if(marshal)
        {
            out << nl << typeToString(type) << ".write(" << stream << ", " << param << ");";
        }
        else
        {
            out << nl << param << " = " << typeToString(type) << ".read(" << stream << ", " << param << ");";
        }
        return;
    }

    if(isClassType(type))
    {
        if(marshal)
        {
            out << nl << stream << ".writeObject(" << param << ");";
        }
        else
        {
            out << nl << stream << ".readObject(function(__o){ " << fixSuffix(param) << " = __o; }, "
                << typeToString(type) << ");";
        }
        return;
    }

    if(SequencePtr::dynamicCast(type) || DictionaryPtr::dynamicCast(type))
    {
        if(marshal)
        {
            out << nl << getHelper(type) <<".write(" << stream << ", " << param << ");";
        }
        else
        {
            out << nl << param << " = " << getHelper(type) << ".read(" << stream << ");";
        }
        return;
    }

    assert(false);
}

void
Slice::JsGenerator::writeOptionalMarshalUnmarshalCode(Output &out,
                                                      const TypePtr& type,
                                                      const string& param,
                                                      int tag,
                                                      bool marshal)
{
    string stream = marshal ? "__os" : "__is";

    if(isClassType(type))
    {
        if(marshal)
        {
            out << nl << stream << ".writeOptObject(" << tag << ", " << param << ");";
        }
        else
        {
            out << nl << stream << ".readOptObject(" << tag << ", function(__o){ " << fixSuffix(param)
                << " = __o; }, " << typeToString(type) << ");";
        }
        return;
    }

    if(EnumPtr::dynamicCast(type))
    {
        if(marshal)
        {
            out << nl << typeToString(type) <<".__writeOpt(" << stream << ", " << tag << ", " << param << ");";
        }
        else
        {
            out << nl << param << " = " << typeToString(type) << ".__readOpt(" << stream << ", " << tag << ");";
        }
        return;
    }

    if(marshal)
    {
        out << nl << getHelper(type) <<".writeOpt(" << stream << ", " << tag << ", " << param << ");";
    }
    else
    {
        out << nl << param << " = " << getHelper(type) << ".readOpt(" << stream << ", " << tag << ");";
    }
}

std::string
Slice::JsGenerator::getHelper(const TypePtr& type)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindByte:
            {
                return "Ice.ByteHelper";
            }
            case Builtin::KindBool:
            {
                return "Ice.BoolHelper";
            }
            case Builtin::KindShort:
            {
                return "Ice.ShortHelper";
            }
            case Builtin::KindInt:
            {
                return "Ice.IntHelper";
            }
            case Builtin::KindLong:
            {
                return "Ice.LongHelper";
            }
            case Builtin::KindFloat:
            {
                return "Ice.FloatHelper";
            }
            case Builtin::KindDouble:
            {
                return "Ice.DoubleHelper";
            }
            case Builtin::KindString:
            {
                return "Ice.StringHelper";
            }
            case Builtin::KindObject:
            {
                return "Ice.ObjectHelper";
            }
            case Builtin::KindObjectProxy:
            {
                return "Ice.ObjectPrx";
            }
            case Builtin::KindLocalObject:
            {
                assert(false);
                break;
            }
        }
    }

    if(EnumPtr::dynamicCast(type))
    {
        return typeToString(type) + ".__helper";
    }

    if(ProxyPtr::dynamicCast(type) || StructPtr::dynamicCast(type))
    {
        return typeToString(type);
    }

    if(SequencePtr::dynamicCast(type) || DictionaryPtr::dynamicCast(type))
    {
        stringstream s;
        s << getLocalScope(ContainedPtr::dynamicCast(type)->scoped()) << "Helper";
        return s.str();
    }

    if(ClassDeclPtr::dynamicCast(type))
    {
        return "Ice.ObjectHelper";
    }

    assert(false);
    return "???";
}
