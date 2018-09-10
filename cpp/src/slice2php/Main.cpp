// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/CtrlCHandler.h>
#include <IceUtil/IceUtil.h>
#include <IceUtil/InputUtil.h>
#include <IceUtil/Options.h>
#include <IceUtil/OutputUtil.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
#include <IceUtil/ConsoleUtil.h>
#include <Slice/Checksum.h>
#include <Slice/Preprocessor.h>
#include <Slice/FileTracker.h>
#include <Slice/PHPUtil.h>
#include <Slice/Parser.h>
#include <Slice/Util.h>
#include <cstring>
#include <climits>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#  include <direct.h>
#else
#  include <unistd.h>
#endif

using namespace std;
using namespace Slice;
using namespace Slice::PHP;
using namespace IceUtilInternal;

namespace
{

//
// Get the fully-qualified name of the given definition. If a suffix is provided,
// it is prepended to the definition's unqualified name. If the nameSuffix
// is provided, it is appended to the container's name.
//
string
getAbsolute(const ContainedPtr& cont, bool ns, const string& pfx = std::string(), const string& suffix = std::string())
{
    return scopedToName(cont->scope() + pfx + cont->name() + suffix, ns);
}

}

//
// CodeVisitor generates the PHP mapping for a translation unit.
//
class CodeVisitor : public ParserVisitor
{
public:

    CodeVisitor(IceUtilInternal::Output&, bool);

    virtual void visitClassDecl(const ClassDeclPtr&);
    virtual bool visitClassDefStart(const ClassDefPtr&);
    virtual bool visitExceptionStart(const ExceptionPtr&);
    virtual bool visitStructStart(const StructPtr&);
    virtual void visitSequence(const SequencePtr&);
    virtual void visitDictionary(const DictionaryPtr&);
    virtual void visitEnum(const EnumPtr&);
    virtual void visitConst(const ConstPtr&);

private:

    void startNamespace(const ContainedPtr&);
    void endNamespace();

    //
    // Return the PHP name for the given Slice type. When using namespaces,
    // this name is a relative (unqualified) name, otherwise this name is the
    // flattened absolute name.
    //
    string getName(const ContainedPtr&, const string& = string());

    //
    // Return the PHP variable for the given object's type.
    //
    string getTypeVar(const ContainedPtr&, const string& = string());

    //
    // Emit the array for a Slice type.
    //
    void writeType(const TypePtr&);
    string getType(const TypePtr&);

    //
    // Write a default value for a given type.
    //
    void writeDefaultValue(const DataMemberPtr&);

    struct MemberInfo
    {
        string fixedName;
        bool inherited;
        DataMemberPtr dataMember;
    };
    typedef list<MemberInfo> MemberInfoList;

    //
    // Write a member assignment statement for a constructor.
    //
    void writeAssign(const MemberInfo&);

    //
    // Write constant value.
    //
    void writeConstantValue(const TypePtr&, const SyntaxTreeBasePtr&, const string&);

    //
    // Write constructor parameters with default values.
    //
    void writeConstructorParams(const MemberInfoList&);

    //
    // Convert an operation mode into a string.
    //
    string getOperationMode(Slice::Operation::Mode, bool);

    void collectClassMembers(const ClassDefPtr&, MemberInfoList&, bool);
    void collectExceptionMembers(const ExceptionPtr&, MemberInfoList&, bool);

    Output& _out;
    bool _ns; // Using namespaces?
    list<string> _moduleStack; // TODO: Necessary?
    set<string> _classHistory; // TODO: Necessary?
};

//
// CodeVisitor implementation.
//
CodeVisitor::CodeVisitor(Output& out, bool ns) :
    _out(out),
    _ns(ns)
{
}

void
CodeVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    //
    // Do not generate any code for php:internal types, those are provided by
    // IcePHP C++ extension.
    //
    StringList metadata = p->getMetaData();
    if(find(metadata.begin(), metadata.end(), "php:internal") != metadata.end())
    {
        return;
    }

    //
    // Handle forward declarations.
    //
    string scoped = p->scoped();
    if(_classHistory.count(scoped) == 0)
    {
        startNamespace(p);

        string type = getTypeVar(p);
        _out << sp << nl << "global " << type << ';';

        bool isInterface = p->isInterface();
        if(!p->isLocal() && (isInterface || p->definition()->allOperations().size() > 0))
        {
            _out << nl << "global " << type << "Prx;";
        }
        _out << nl << "if(!isset(" << type << "))";
        _out << sb;
        _out << nl << type << " = IcePHP_declareClass('" << scoped << "');";
        if(!p->isLocal() && (isInterface || p->definition()->allOperations().size() > 0))
        {
            _out << nl << type << "Prx = IcePHP_declareProxy('" << scoped << "');";
        }
        _out << eb;

        endNamespace();

        _classHistory.insert(scoped); // Avoid redundant declarations.
    }
}

bool
CodeVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    //
    // Do not generate any code for php:internal types, those are provided by
    // IcePHP C++ extension.
    //
    StringList metadata = p->getMetaData();
    if(find(metadata.begin(), metadata.end(), "php:internal") != metadata.end())
    {
        return false;
    }

    string scoped = p->scoped();
    string name = getName(p);
    string type = getTypeVar(p);
    string abs = getAbsolute(p, _ns);
    string prxName = getName(p, "Prx");
    string prxType = getTypeVar(p, "Prx");
    string prxAbs = getAbsolute(p, _ns, "", "Prx");
    ClassList bases = p->bases();
    ClassDefPtr base;
    OperationList ops = p->operations();
    DataMemberList members = p->dataMembers();
    bool isInterface = p->isInterface();
    bool isAbstract = isInterface || p->allOperations().size() > 0; // Don't use isAbstract() - see bug 3739

    startNamespace(p);

    _out << sp << nl << "global " << type << ';';
    if(!p->isLocal() && isAbstract)
    {
        _out << nl << "global " << prxType << ';';
    }

    //
    // Define the class.
    //
    if(isInterface)
    {
        if(p->isLocal())
        {
            _out << nl << "interface " << name;
            if(bases.empty())
            {
                if(!p->isLocal())
                {
                    _out << " extends " << scopedToName("::Ice::Object", _ns);
                }
            }
            else
            {
                _out << " extends ";
                for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
                {
                    if(q != bases.begin())
                    {
                        _out << ", ";
                    }
                    _out << getAbsolute(*q, _ns);
                }
            }
            _out << sb;
            for(OperationList::iterator oli = ops.begin(); oli != ops.end(); ++oli)
            {
                _out << nl << "public function " << fixIdent((*oli)->name()) << '(';
                ParamDeclList params = (*oli)->parameters();
                for(ParamDeclList::iterator q = params.begin(); q != params.end(); ++q)
                {
                    if(q != params.begin())
                    {
                        _out << ", ";
                    }
                    _out << '$' << fixIdent((*q)->name());
                }
                _out << ");";
            }

            _out << eb;
        }
    }
    else
    {
        _out << nl;
        _out << "class " << name;
        if(!bases.empty() && !bases.front()->isInterface())
        {
            base = bases.front();
            bases.pop_front();
        }
        if(base)
        {
            _out << " extends " << getAbsolute(base, _ns);
        }
        else
        {
            if(!p->isLocal())
            {
                _out << " extends " << scopedToName("::Ice::Value", _ns);
            }
        }

        //
        // Value objects don't implement any interfaces.
        //
        if(p->isLocal() && !bases.empty())
        {
            _out << " implements ";
            for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
            {
                if(q != bases.begin())
                {
                    _out << ", ";
                }
                _out << getAbsolute(*q, _ns);
            }
        }

        _out << sb;

        //
        // __construct
        //
        _out << nl << "public function __construct(";
        MemberInfoList allMembers;
        collectClassMembers(p, allMembers, false);
        writeConstructorParams(allMembers);
        _out << ")";
        _out << sb;
        if(base)
        {
            _out << nl << "parent::__construct(";
            int count = 0;
            for(MemberInfoList::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
            {
                if(q->inherited)
                {
                    if(count)
                    {
                        _out << ", ";
                    }
                    _out << '$' << q->fixedName;
                    ++count;
                }
            }
            _out << ");";
        }
        {
            for(MemberInfoList::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
            {
                if(!q->inherited)
                {
                    writeAssign(*q);
                }
            }
        }
        _out << eb;

        if(!p->isLocal())
        {
            //
            // ice_ice
            //
            _out << sp << nl << "public function ice_id()";
            _out << sb;
            _out << nl << "return '" << scoped << "';";
            _out << eb;

            //
            // ice_staticId
            //
            _out << sp << nl << "public static function ice_staticId()";
            _out << sb;
            _out << nl << "return '" << scoped << "';";
            _out << eb;
        }

        //
        // __toString
        //
        _out << sp << nl << "public function __toString()";
        _out << sb;
        _out << nl << "global " << type << ';';
        _out << nl << "return IcePHP_stringify($this, " << type << ");";
        _out << eb;

        if(!members.empty())
        {
            _out << sp;
            bool isProtected = p->hasMetaData("protected");
            for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
            {
                _out << nl;
                if(isProtected || (*q)->hasMetaData("protected"))
                {
                    _out << "protected ";
                }
                else
                {
                    _out << "public ";
                }
                _out << "$" << fixIdent((*q)->name()) << ";";
            }
        }

        _out << eb; // End of class.
    }

    //
    // Define the proxy class.
    //
    if(!p->isLocal() && isAbstract)
    {
        _out << sp << nl << "class " << prxName << "Helper";
        _out << sb;

        _out << sp << nl << "public static function checkedCast($proxy, $facetOrContext=null, $context=null)";
        _out << sb;
        _out << nl << "return $proxy->ice_checkedCast('" << scoped << "', $facetOrContext, $context);";
        _out << eb;

        _out << sp << nl << "public static function uncheckedCast($proxy, $facet=null)";
        _out << sb;
        _out << nl << "return $proxy->ice_uncheckedCast('" << scoped << "', $facet);";
        _out << eb;

        _out << sp << nl << "public static function ice_staticId()";
        _out << sb;
        _out << nl << "return '" << scoped << "';";
        _out << eb;

        _out << eb;
    }

    if(_classHistory.count(scoped) == 0 && p->canBeCyclic())
    {
        //
        // Emit a forward declaration for the class in case a data member refers to this type.
        //
        _out << sp << nl << type << " = IcePHP_declareClass('" << scoped << "');";
        if(!p->isLocal() && isAbstract)
        {
            _out << nl << prxType << " = IcePHP_declareProxy('" << scoped << "');";
        }
    }

    {
        string type;
        vector<string> seenType;
        if(base || (!p->isLocal() && !isInterface))
        {
            _out << sp << nl << "global ";
            if(!base)
            {
                type = "$Ice__t_Value";
            }
            else
            {
                type = getTypeVar(base);
            }
            _out << type << ";";
        }
        seenType.push_back(type);

        for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
        {
            string type = getType((*q)->type());
            if(find(seenType.begin(), seenType.end(), type) == seenType.end())
            {
                seenType.push_back(type);
                _out << nl << "global " << type << ";";
            }
        }
    }

    //
    // Emit the type information.
    //
    const bool preserved = p->hasMetaData("preserve-slice") || p->inheritsMetaData("preserve-slice");
    _out << nl << type << " = IcePHP_defineClass('" << scoped << "', '" << escapeName(abs) << "', "
         << p->compactId() << ", " << (preserved ? "true" : "false") << ", "
         << (isInterface ? "true" : "false") << ", ";
    if(!base)
    {
        if(p->isLocal() || isInterface)
        {
            _out << "null";
        }
        else
        {
            _out << "$Ice__t_Value";
        }
    }
    else
    {
        _out << getTypeVar(base);
    }
    _out << ", ";
    //
    // Members
    //
    // Data members are represented as an array:
    //
    //   ('MemberName', MemberType, Optional, Tag)
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a constructed type.
    //
    if(!members.empty())
    {
        _out << "array(";
        for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
        {
            if(q != members.begin())
            {
                _out << ',';
            }
            _out.inc();
            _out << nl << "array('" << fixIdent((*q)->name()) << "', ";
            writeType((*q)->type());
            _out << ", " << ((*q)->optional() ? "true" : "false") << ", "
                 << ((*q)->optional() ? (*q)->tag() : 0) << ')';
            _out.dec();
        }
        _out << ')';
    }
    else
    {
        _out << "null";
    }
    _out << ");";

    if(!p->isLocal() && isAbstract)
    {
        _out << sp << nl << "global ";
        if(!base || base->allOperations().empty())
        {
            _out << "$Ice__t_ObjectPrx";
        }
        else
        {
            _out << getTypeVar(base, "Prx");
        }
        _out << ";";
        _out << nl << prxType << " = IcePHP_defineProxy('" << scoped << "', ";
        if(!base || base->allOperations().empty())
        {
            _out << "$Ice__t_ObjectPrx";
        }
        else
        {
            _out << getTypeVar(base, "Prx");
        }
        _out << ", ";
        //
        // Interfaces
        //
        if(!bases.empty())
        {
            _out << "array(";
            for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
            {
                if(q != bases.begin())
                {
                    _out << ", ";
                }
                _out << getTypeVar(*q, "Prx");
            }
            _out << ')';
        }
        else
        {
            _out << "null";
        }
        _out << ");";

        //
        // Define each operation. The arguments to IcePHP_defineOperation are:
        //
        // $ClassType, 'opName', Mode, SendMode, FormatType, (InParams), (OutParams), ReturnParam, (Exceptions)
        //
        // where InParams and OutParams are arrays of type descriptions, and Exceptions
        // is an array of exception type ids.
        //
        if(!ops.empty())
        {
            _out << sp;
            vector<string> seenTypes;
            for(OperationList::const_iterator p = ops.begin(); p != ops.end(); ++p)
            {
                ParamDeclList params = (*p)->parameters();
                for(ParamDeclList::const_iterator q = params.begin(); q != params.end(); ++q)
                {
                    string type = getType((*q)->type());
                    if(find(seenTypes.begin(), seenTypes.end(), type) == seenTypes.end())
                    {
                        seenTypes.push_back(type);
                        _out << nl << "global " << type << ";";
                    }
                }

                if((*p)->returnType())
                {
                    string type = getType((*p)->returnType());
                    if(find(seenTypes.begin(), seenTypes.end(), type) == seenTypes.end())
                    {
                        seenTypes.push_back(type);
                        _out << nl << "global " << type << ";";
                    }
                }
            }

            for(OperationList::iterator oli = ops.begin(); oli != ops.end(); ++oli)
            {
                ParamDeclList params = (*oli)->parameters();
                ParamDeclList::iterator t;
                int count;

                _out << nl << "IcePHP_defineOperation(" << prxType << ", '" << (*oli)->name() << "', "
                     << getOperationMode((*oli)->mode(), _ns) << ", " << getOperationMode((*oli)->sendMode(), _ns)
                     << ", " << static_cast<int>((*oli)->format()) << ", ";
                for(t = params.begin(), count = 0; t != params.end(); ++t)
                {
                    if(!(*t)->isOutParam())
                    {
                        if(count == 0)
                        {
                            _out << "array(";
                        }
                        else if(count > 0)
                        {
                            _out << ", ";
                        }
                        _out << "array(";
                        writeType((*t)->type());
                        if((*t)->optional())
                        {
                            _out << ", " << (*t)->tag();
                        }
                        _out << ')';
                        ++count;
                    }
                }
                if(count > 0)
                {
                    _out << ')';
                }
                else
                {
                    _out << "null";
                }
                _out << ", ";
                for(t = params.begin(), count = 0; t != params.end(); ++t)
                {
                    if((*t)->isOutParam())
                    {
                        if(count == 0)
                        {
                            _out << "array(";
                        }
                        else if(count > 0)
                        {
                            _out << ", ";
                        }
                        _out << "array(";
                        writeType((*t)->type());
                        if((*t)->optional())
                        {
                            _out << ", " << (*t)->tag();
                        }
                        _out << ')';
                        ++count;
                    }
                }
                if(count > 0)
                {
                    _out << ')';
                }
                else
                {
                    _out << "null";
                }
                _out << ", ";
                TypePtr returnType = (*oli)->returnType();
                if(returnType)
                {
                    //
                    // The return type has the same format as an in/out parameter:
                    //
                    // Type, Optional?, OptionalTag
                    //
                    _out << "array(";
                    writeType(returnType);
                    if((*oli)->returnIsOptional())
                    {
                        _out << ", " << (*oli)->returnTag();
                    }
                    _out << ')';
                }
                else
                {
                    _out << "null";
                }
                _out << ", ";
                ExceptionList exceptions = (*oli)->throws();
                if(!exceptions.empty())
                {
                    _out << "array(";
                    for(ExceptionList::iterator u = exceptions.begin(); u != exceptions.end(); ++u)
                    {
                        if(u != exceptions.begin())
                        {
                            _out << ", ";
                        }
                        _out << getTypeVar(*u);
                    }
                    _out << ')';
                }
                else
                {
                    _out << "null";
                }
                _out << ");";
            }
        }
    }

    endNamespace();

    if(_classHistory.count(scoped) == 0)
    {
        _classHistory.insert(scoped); // Avoid redundant declarations.
    }

    return false;
}

bool
CodeVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    //
    // Do not generate any code for php:internal types, those are provided by
    // IcePHP C++ extension.
    //
    StringList metadata = p->getMetaData();
    if(find(metadata.begin(), metadata.end(), "php:internal") != metadata.end())
    {
        return false;
    }

    string scoped = p->scoped();
    string name = getName(p);
    string type = getTypeVar(p);
    string abs = getAbsolute(p, _ns);

    startNamespace(p);

    _out << sp << nl << "global " << type << ';';
    _out << nl << "class " << name << " extends ";
    ExceptionPtr base = p->base();
    string baseName;
    if(base)
    {
        baseName = getAbsolute(base, _ns);
        _out << baseName;
    }
    else if(p->isLocal())
    {
        _out << scopedToName("::Ice::LocalException", _ns);
    }
    else
    {
        _out << scopedToName("::Ice::UserException", _ns);
    }
    _out << sb;

    DataMemberList members = p->dataMembers();

    //
    // __construct
    //
    _out << nl << "public function __construct(";
    MemberInfoList allMembers;
    collectExceptionMembers(p, allMembers, false);
    writeConstructorParams(allMembers);
    _out << ")";
    _out << sb;
    if(base)
    {
        _out << nl << "parent::__construct(";
        int count = 0;
        for(MemberInfoList::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
        {
            if(q->inherited)
            {
                if(count)
                {
                    _out << ", ";
                }
                _out << '$' << q->fixedName;
                ++count;
            }
        }
        _out << ");";
    }
    for(MemberInfoList::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
    {
        if(!q->inherited)
        {
            writeAssign(*q);
        }
    }
    _out << eb;

    //
    // ice_id
    //
    _out << sp << nl << "public function ice_id()";
    _out << sb;
    _out << nl << "return '" << scoped << "';";
    _out << eb;

    //
    // __toString
    //
    _out << sp << nl << "public function __toString()";
    _out << sb;
    _out << nl << "global " << type << ';';
    _out << nl << "return IcePHP_stringifyException($this, " << type << ");";
    _out << eb;

    if(!members.empty())
    {
        _out << sp;
        for(DataMemberList::iterator dmli = members.begin(); dmli != members.end(); ++dmli)
        {
            _out << nl << "public $" << fixIdent((*dmli)->name()) << ";";
        }
    }

    _out << eb;

    vector<string> seenType;
    for(DataMemberList::iterator dmli = members.begin(); dmli != members.end(); ++dmli)
    {
        string type = getType((*dmli)->type());
        if(find(seenType.begin(), seenType.end(), type) == seenType.end())
        {
            seenType.push_back(type);
            _out << nl << "global " << type << ";";
        }
    }

    //
    // Emit the type information.
    //
    const bool preserved = p->hasMetaData("preserve-slice") || p->inheritsMetaData("preserve-slice");
    _out << sp << nl << type << " = IcePHP_defineException('" << scoped << "', '" << escapeName(abs) << "', "
         << (preserved ? "true" : "false") << ", ";
    if(!base)
    {
        _out << "null";
    }
    else
    {
        _out << getTypeVar(base);
    }
    _out << ", ";
    //
    // Data members are represented as an array:
    //
    //   ('MemberName', MemberType, Optional, Tag)
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a constructed type.
    //
    if(!members.empty())
    {
        _out << "array(";
        for(DataMemberList::iterator dmli = members.begin(); dmli != members.end(); ++dmli)
        {
            if(dmli != members.begin())
            {
                _out << ',';
            }
            _out.inc();
            _out << nl << "array('" << fixIdent((*dmli)->name()) << "', ";
            writeType((*dmli)->type());
            _out << ", " << ((*dmli)->optional() ? "true" : "false") << ", "
                 << ((*dmli)->optional() ? (*dmli)->tag() : 0) << ')';
            _out.dec();
        }
        _out << ')';
    }
    else
    {
        _out << "null";
    }
    _out << ");";

    endNamespace();

    return false;
}

bool
CodeVisitor::visitStructStart(const StructPtr& p)
{
    //
    // Do not generate any code for php:internal types, those are provided by
    // IcePHP C++ extension.
    //
    StringList metadata = p->getMetaData();
    if(find(metadata.begin(), metadata.end(), "php:internal") != metadata.end())
    {
        return false;
    }

    string scoped = p->scoped();
    string name = getName(p);
    string type = getTypeVar(p);
    string abs = getAbsolute(p, _ns);
    MemberInfoList memberList;

    {
        DataMemberList members = p->dataMembers();
        for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
        {
            memberList.push_back(MemberInfo());
            memberList.back().fixedName = fixIdent((*q)->name());
            memberList.back().inherited = false;
            memberList.back().dataMember = *q;
        }
    }

    startNamespace(p);

    _out << sp << nl << "global " << type << ';';

    _out << nl << "class " << name;
    _out << sb;
    _out << nl << "public function __construct(";
    writeConstructorParams(memberList);
    _out << ")";
    _out << sb;
    for(MemberInfoList::iterator r = memberList.begin(); r != memberList.end(); ++r)
    {
        writeAssign(*r);
    }
    _out << eb;

    //
    // __toString
    //
    _out << sp << nl << "public function __toString()";
    _out << sb;
    _out << nl << "global " << type << ';';
    _out << nl << "return IcePHP_stringify($this, " << type << ");";
    _out << eb;

    if(!memberList.empty())
    {
        _out << sp;
        for(MemberInfoList::iterator r = memberList.begin(); r != memberList.end(); ++r)
        {
            _out << nl << "public $" << r->fixedName << ';';
        }
    }

    _out << eb;

    _out << sp;
    vector<string> seenType;
    for(MemberInfoList::iterator r = memberList.begin(); r != memberList.end(); ++r)
    {
        string type = getType(r->dataMember->type());
        if(find(seenType.begin(), seenType.end(), type) == seenType.end())
        {
            seenType.push_back(type);
            _out << nl << "global " << type << ";";
        }
    }
    //
    // Emit the type information.
    //
    _out << nl << type << " = IcePHP_defineStruct('" << scoped << "', '" << escapeName(abs) << "', array(";
    //
    // Data members are represented as an array:
    //
    //   ('MemberName', MemberType)
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a constructed type.
    //
    for(MemberInfoList::iterator r = memberList.begin(); r != memberList.end(); ++r)
    {
        if(r != memberList.begin())
        {
            _out << ",";
        }
        _out.inc();
        _out << nl << "array('" << r->fixedName << "', ";
        writeType(r->dataMember->type());
        _out << ')';
        _out.dec();
    }
    _out << "));";
    endNamespace();

    return false;
}

void
CodeVisitor::visitSequence(const SequencePtr& p)
{
    //
    // Do not generate any code for php:internal types, those are provided by
    // IcePHP C++ extension.
    //
    StringList metadata = p->getMetaData();
    if(find(metadata.begin(), metadata.end(), "php:internal") != metadata.end())
    {
        return;
    }

    string type = getTypeVar(p);
    TypePtr content = p->type();

    startNamespace(p);

    //
    // Emit the type information.
    //
    string scoped = p->scoped();
    _out << sp << nl << "global " << type << ';';
    _out << sp << nl << "if(!isset(" << type << "))";
    _out << sb;
    _out << nl << "global " << getType(content) << ";";
    _out << nl << type << " = IcePHP_defineSequence('" << scoped << "', ";
    writeType(content);
    _out << ");";
    _out << eb;

    endNamespace();
}

void
CodeVisitor::visitDictionary(const DictionaryPtr& p)
{
    //
    // Do not generate any code for php:internal types, those are provided by
    // IcePHP C++ extension.
    //
    StringList metadata = p->getMetaData();
    if(find(metadata.begin(), metadata.end(), "php:internal") != metadata.end())
    {
        return;
    }

    TypePtr keyType = p->keyType();
    BuiltinPtr b = BuiltinPtr::dynamicCast(keyType);

    const UnitPtr unit = p->unit();
    const DefinitionContextPtr dc = unit->findDefinitionContext(p->file());
    assert(dc);
    if(b)
    {
        switch(b->kind())
        {
            case Slice::Builtin::KindBool:
            case Slice::Builtin::KindByte:
            case Slice::Builtin::KindShort:
            case Slice::Builtin::KindInt:
            case Slice::Builtin::KindLong:
            case Slice::Builtin::KindString:
                //
                // These types are acceptable as dictionary keys.
                //
                break;

            case Slice::Builtin::KindFloat:
            case Slice::Builtin::KindDouble:
            {
                dc->warning(InvalidMetaData, p->file(), p->line(), "dictionary key type not supported in PHP");
                break;
            }

            case Slice::Builtin::KindObject:
            case Slice::Builtin::KindObjectProxy:
            case Slice::Builtin::KindLocalObject:
            case Slice::Builtin::KindValue:
                assert(false);
        }
    }
    else if(!EnumPtr::dynamicCast(keyType))
    {
        dc->warning(InvalidMetaData, p->file(), p->line(), "dictionary key type not supported in PHP");
    }

    string type = getTypeVar(p);

    startNamespace(p);

    //
    // Emit the type information.
    //
    string scoped = p->scoped();
    _out << sp << nl << "global " << type << ';';
    _out << sp << nl << "if(!isset(" << type << "))";
    _out << sb;
    _out << nl << "global " << getType(p->keyType()) << ";";
    _out << nl << "global " << getType(p->valueType()) << ";";
    _out << nl << type << " = IcePHP_defineDictionary('" << scoped << "', ";
    writeType(p->keyType());
    _out << ", ";
    writeType(p->valueType());
    _out << ");";
    _out << eb;

    endNamespace();
}

void
CodeVisitor::visitEnum(const EnumPtr& p)
{
    //
    // Do not generate any code for php:internal types, those are provided by
    // IcePHP C++ extension.
    //
    StringList metadata = p->getMetaData();
    if(find(metadata.begin(), metadata.end(), "php:internal") != metadata.end())
    {
        return;
    }

    string scoped = p->scoped();
    string name = getName(p);
    string type = getTypeVar(p);
    string abs = getAbsolute(p, _ns);
    EnumeratorList enums = p->enumerators();

    startNamespace(p);

    _out << sp << nl << "global " << type << ';';
    _out << nl << "class " << name;
    _out << sb;

    {
        long i = 0;
        for(EnumeratorList::iterator q = enums.begin(); q != enums.end(); ++q, ++i)
        {
            _out << nl << "const " << fixIdent((*q)->name()) << " = " << (*q)->value() << ';';
        }
    }

    _out << eb;

    //
    // Emit the type information.
    //
    _out << sp << nl << type << " = IcePHP_defineEnum('" << scoped << "', array(";
    for(EnumeratorList::iterator q = enums.begin(); q != enums.end(); ++q)
    {
        if(q != enums.begin())
        {
            _out << ", ";
        }
        _out << "'" << (*q)->name() << "', " << (*q)->value();
    }
    _out << "));";

    endNamespace();
}

void
CodeVisitor::visitConst(const ConstPtr& p)
{
    //
    // Do not generate any code for php:internal types, those are provided by
    // IcePHP C++ extension.
    //
    StringList metadata = p->getMetaData();
    if(find(metadata.begin(), metadata.end(), "php:internal") != metadata.end())
    {
        return;
    }

    string name = getName(p);
    string type = getTypeVar(p);
    string abs = getAbsolute(p, _ns);

    startNamespace(p);

    _out << sp << nl << "if(!defined('" << escapeName(abs) << "'))";
    _out << sb;
    if(_ns)
    {
        _out << sp << nl << "define(__NAMESPACE__ . '\\\\" << name << "', ";
    }
    else
    {
        _out << sp << nl << "define('" << name << "', ";
    }

    writeConstantValue(p->type(), p->valueType(), p->value());

    _out << ");";
    _out << eb;

    endNamespace();
}

void
CodeVisitor::startNamespace(const ContainedPtr& cont)
{
    if(_ns)
    {
        string scope = cont->scope();
        scope = scope.substr(2); // Removing leading '::'
        scope = scope.substr(0, scope.length() - 2); // Removing trailing '::'
        _out << sp << nl << "namespace " << scopedToName(scope, true);
        _out << sb;
    }
}

void
CodeVisitor::endNamespace()
{
    if(_ns)
    {
        _out << eb;
    }
}

string
CodeVisitor::getTypeVar(const ContainedPtr& p, const string& suffix)
{
    return "$" + getAbsolute(p, false, "_t_", suffix);
}

string
CodeVisitor::getName(const ContainedPtr& p, const string& suffix)
{
    if(_ns)
    {
        return fixIdent(p->name() + suffix);
    }
    else
    {
        return getAbsolute(p, false, "", suffix);
    }
}

void
CodeVisitor::writeType(const TypePtr& p)
{
    _out << getType(p);
}

string
CodeVisitor::getType(const TypePtr& p)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(p);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindBool:
            {
                return "$IcePHP__t_bool";
            }
            case Builtin::KindByte:
            {
                return "$IcePHP__t_byte";
            }
            case Builtin::KindShort:
            {
                return "$IcePHP__t_short";
            }
            case Builtin::KindInt:
            {
                return "$IcePHP__t_int";
            }
            case Builtin::KindLong:
            {
                return "$IcePHP__t_long";
            }
            case Builtin::KindFloat:
            {
                return "$IcePHP__t_float";
            }
            case Builtin::KindDouble:
            {
                return "$IcePHP__t_double";
            }
            case Builtin::KindString:
            {
                return "$IcePHP__t_string";
            }
            case Builtin::KindObject:
            case Builtin::KindValue:
            {
                return "$Ice__t_Value";
            }
            case Builtin::KindObjectProxy:
            {
                return "$Ice__t_ObjectPrx";
            }
            case Builtin::KindLocalObject:
            {
                return "$Ice__t_LocalObject";
            }
        }
    }

    ProxyPtr prx = ProxyPtr::dynamicCast(p);
    if(prx)
    {
        ClassDefPtr def = prx->_class()->definition();
        if(def->isInterface() || def->allOperations().size() > 0)
        {
            return getTypeVar(prx->_class(), "Prx");
        }
        else
        {
            return "$Ice__t_ObjectPrx";
        }
    }

    ContainedPtr cont = ContainedPtr::dynamicCast(p);
    assert(cont);
    return getTypeVar(cont);
}

void
CodeVisitor::writeDefaultValue(const DataMemberPtr& m)
{
    TypePtr p = m->type();
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(p);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindBool:
            {
                _out << "false";
                break;
            }
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindInt:
            case Builtin::KindLong:
            {
                _out << "0";
                break;
            }
            case Builtin::KindFloat:
            case Builtin::KindDouble:
            {
                _out << "0.0";
                break;
            }
            case Builtin::KindString:
            {
                _out << "''";
                break;
            }
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindLocalObject:
            case Builtin::KindValue:
            {
                _out << "null";
                break;
            }
        }
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(p);
    if(en)
    {
        EnumeratorList enums = en->enumerators();
        _out << getAbsolute(en, _ns) << "::" << fixIdent(enums.front()->name());
        return;
    }

    //
    // PHP does not allow the following construct:
    //
    // function foo($theStruct=new MyStructType)
    //
    // Instead we use null as the default value and allocate an instance in
    // the constructor.
    //
    if(StructPtr::dynamicCast(p))
    {
        _out << "null";
        return;
    }

    _out << "null";
}

void
CodeVisitor::writeAssign(const MemberInfo& info)
{
    StructPtr st = StructPtr::dynamicCast(info.dataMember->type());
    if(st)
    {
        _out << nl << "$this->" << info.fixedName << " = is_null($" << info.fixedName << ") ? new "
             << getAbsolute(st, _ns) << " : $" << info.fixedName << ';';
    }
    else
    {
        _out << nl << "$this->" << info.fixedName << " = $" << info.fixedName << ';';
    }
}

void
CodeVisitor::writeConstantValue(const TypePtr& type, const SyntaxTreeBasePtr& valueType, const string& value)
{
    ConstPtr constant = ConstPtr::dynamicCast(valueType);
    if(constant)
    {
        _out << getAbsolute(constant, _ns);
    }
    else
    {
        Slice::BuiltinPtr b = Slice::BuiltinPtr::dynamicCast(type);
        Slice::EnumPtr en = Slice::EnumPtr::dynamicCast(type);
        if(b)
        {
            switch(b->kind())
            {
                case Slice::Builtin::KindBool:
                case Slice::Builtin::KindByte:
                case Slice::Builtin::KindShort:
                case Slice::Builtin::KindInt:
                case Slice::Builtin::KindFloat:
                case Slice::Builtin::KindDouble:
                {
                    _out << value;
                    break;
                }
                case Slice::Builtin::KindLong:
                {
                    IceUtil::Int64 l;
                    IceUtilInternal::stringToInt64(value, l);
                    //
                    // The platform's 'long' type may not be 64 bits, so we store 64-bit
                    // values as a string.
                    //
                    if(sizeof(IceUtil::Int64) > sizeof(long) && (l < LONG_MIN || l > LONG_MAX))
                    {
                        _out << "'" << value << "'";
                    }
                    else
                    {
                        _out << value;
                    }
                    break;
                }
                case Slice::Builtin::KindString:
                {
                    // PHP 7.x also supports an EC6UCN-like notation, see:
                    // https://wiki.php.net/rfc/unicode_escape
                    //
                    _out << "\"" << toStringLiteral(value, "\f\n\r\t\v\x1b", "$", Octal, 0) << "\"";
                    break;
                }
                case Slice::Builtin::KindObject:
                case Slice::Builtin::KindObjectProxy:
                case Slice::Builtin::KindLocalObject:
                case Slice::Builtin::KindValue:
                    assert(false);
            }
        }
        else if(en)
        {
            EnumeratorPtr lte = EnumeratorPtr::dynamicCast(valueType);
            assert(lte);
            _out << getAbsolute(en, _ns) << "::" << fixIdent(lte->name());
        }
        else
        {
            assert(false); // Unknown const type.
        }
    }
}

void
CodeVisitor::writeConstructorParams(const MemberInfoList& members)
{
    for(MemberInfoList::const_iterator p = members.begin(); p != members.end(); ++p)
    {
        if(p != members.begin())
        {
            _out << ", ";
        }
        _out << '$' << p->fixedName << "=";

        const DataMemberPtr member = p->dataMember;
        if(member->defaultValueType())
        {
            writeConstantValue(member->type(), member->defaultValueType(), member->defaultValue());
        }
        else if(member->optional())
        {
            _out << (_ns ? scopedToName("::Ice::None", _ns) : "Ice_Unset");
        }
        else
        {
            writeDefaultValue(member);
        }
    }
}

string
CodeVisitor::getOperationMode(Slice::Operation::Mode mode, bool /*ns*/)
{
    ostringstream ostr;
    ostr << static_cast<int>(mode);
    return ostr.str();
}

void
CodeVisitor::collectClassMembers(const ClassDefPtr& p, MemberInfoList& allMembers, bool inherited)
{
    ClassList bases = p->bases();
    if(!bases.empty() && !bases.front()->isInterface())
    {
        collectClassMembers(bases.front(), allMembers, true);
    }

    DataMemberList members = p->dataMembers();

    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
    {
        MemberInfo m;
        m.fixedName = fixIdent((*q)->name());
        m.inherited = inherited;
        m.dataMember = *q;
        allMembers.push_back(m);
    }
}

void
CodeVisitor::collectExceptionMembers(const ExceptionPtr& p, MemberInfoList& allMembers, bool inherited)
{
    ExceptionPtr base = p->base();
    if(base)
    {
        collectExceptionMembers(base, allMembers, true);
    }

    DataMemberList members = p->dataMembers();

    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
    {
        MemberInfo m;
        m.fixedName = fixIdent((*q)->name());
        m.inherited = inherited;
        m.dataMember = *q;
        allMembers.push_back(m);
    }
}

static void
generate(const UnitPtr& un, bool all, bool checksum, bool ns, const vector<string>& includePaths, Output& out)
{
    if(!all)
    {
        vector<string> paths = includePaths;
        for(vector<string>::iterator p = paths.begin(); p != paths.end(); ++p)
        {
            *p = fullPath(*p);
        }

        StringList includes = un->includeFiles();
        if(!includes.empty())
        {
            if(ns)
            {
                out << sp;
                out << nl << "namespace";
                out << sb;
            }
            for(StringList::const_iterator q = includes.begin(); q != includes.end(); ++q)
            {
                string file = changeInclude(*q, paths);
                out << nl << "require_once '" << file << ".php';";
            }
            if(ns)
            {
                out << eb;
            }
        }
    }

    CodeVisitor codeVisitor(out, ns);
    un->visit(&codeVisitor, false);

    if(checksum)
    {
        ChecksumMap checksums = createChecksums(un);
        if(!checksums.empty())
        {
            out << sp;
            if(ns)
            {
                out << "namespace"; // Global namespace.
                out << sb;
                out << "new Ice\\SliceChecksumInit(array(";
                for(ChecksumMap::const_iterator p = checksums.begin(); p != checksums.end();)
                {
                    out << nl << "\"" << p->first << "\" => \"";
                    ostringstream str;
                    str.flags(ios_base::hex);
                    str.fill('0');
                    for(vector<unsigned char>::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
                    {
                        str << static_cast<int>(*q);
                    }
                    out << str.str() << "\"";
                    if(++p != checksums.end())
                    {
                        out << ",";
                    }
                }
                out << "));";
                out << eb;
            }
            else
            {
                out << nl << "global $Ice_sliceChecksums;";
                for(ChecksumMap::const_iterator p = checksums.begin(); p != checksums.end(); ++p)
                {
                    out << nl << "$Ice_sliceChecksums[\"" << p->first << "\"] = \"";
                    ostringstream str;
                    str.flags(ios_base::hex);
                    str.fill('0');
                    for(vector<unsigned char>::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
                    {
                        str << static_cast<int>(*q);
                    }
                    out << str.str() << "\";";
                }
            }
        }
    }

    out << nl; // Trailing newline.
}

static void
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

namespace
{

IceUtil::Mutex* globalMutex = 0;
bool interrupted = false;

class Init
{
public:

    Init()
    {
        globalMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete globalMutex;
        globalMutex = 0;
    }
};

Init init;

}

static void
interruptedCallback(int /*signal*/)
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(globalMutex);

    interrupted = true;
}

static void
usage(const string& n)
{
    consoleErr << "Usage: " << n << " [options] slice-files...\n";
    consoleErr <<
        "Options:\n"
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
        "--no-namespace           Do not use PHP namespaces (deprecated).\n"
        "--checksum               Generate checksums for Slice definitions.\n"
        "--ice                    Allow reserved Ice prefix in Slice identifiers\n"
        "                         deprecated: use instead [[\"ice-prefix\"]] metadata.\n"
        "--underscore             Allow underscores in Slice identifiers\n"
        "                         deprecated: use instead [[\"underscore\"]] metadata.\n"
        ;
}

int
compile(const vector<string>& argv)
{
    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("", "validate");
    opts.addOpt("D", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("U", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("I", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("E");
    opts.addOpt("", "output-dir", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "depend");
    opts.addOpt("", "depend-xml");
    opts.addOpt("", "depend-file", IceUtilInternal::Options::NeedArg, "");
    opts.addOpt("d", "debug");
    opts.addOpt("", "ice");
    opts.addOpt("", "underscore");
    opts.addOpt("", "all");
    opts.addOpt("", "checksum");
    opts.addOpt("n", "no-namespace");

    bool validate = find(argv.begin(), argv.end(), "--validate") != argv.end();

    vector<string> args;
    try
    {
        args = opts.parse(argv);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        consoleErr << argv[0] << ": error: " << e.reason << endl;
        if(!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if(opts.isSet("help"))
    {
        usage(argv[0]);
        return EXIT_SUCCESS;
    }

    if(opts.isSet("version"))
    {
        consoleErr << ICE_STRING_VERSION << endl;
        return EXIT_SUCCESS;
    }

    vector<string> cppArgs;
    vector<string> optargs = opts.argVec("D");
    for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
    {
        cppArgs.push_back("-D" + *i);
    }

    optargs = opts.argVec("U");
    for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
    {
        cppArgs.push_back("-U" + *i);
    }

    vector<string> includePaths = opts.argVec("I");
    for(vector<string>::const_iterator i = includePaths.begin(); i != includePaths.end(); ++i)
    {
        cppArgs.push_back("-I" + Preprocessor::normalizeIncludePath(*i));
    }

    bool preprocess = opts.isSet("E");

    string output = opts.optArg("output-dir");

    bool depend = opts.isSet("depend");

    bool dependxml = opts.isSet("depend-xml");

    string dependFile = opts.optArg("depend-file");

    bool debug = opts.isSet("debug");

    bool ice = opts.isSet("ice");

    bool underscore = opts.isSet("underscore");

    bool all = opts.isSet("all");

    bool checksum = opts.isSet("checksum");

    bool ns = !opts.isSet("no-namespace");

    if(args.empty())
    {
        consoleErr << argv[0] << ": error: no input file" << endl;
        if(!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if(depend && dependxml)
    {
        consoleErr << argv[0] << ": error: cannot specify both --depend and --depend-xml" << endl;
        if(!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if(validate)
    {
        return EXIT_SUCCESS;
    }

    int status = EXIT_SUCCESS;

    IceUtil::CtrlCHandler ctrlCHandler;
    ctrlCHandler.setCallback(interruptedCallback);

    ostringstream os;
    if(dependxml)
    {
        os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dependencies>" << endl;
    }

    for(vector<string>::const_iterator i = args.begin(); i != args.end(); ++i)
    {
        //
        // Ignore duplicates.
        //
        vector<string>::iterator p = find(args.begin(), args.end(), *i);
        if(p != i)
        {
            continue;
        }

        if(depend || dependxml)
        {
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
            FILE* cppHandle = icecpp->preprocess(false, "-D__SLICE2PHP__");

            if(cppHandle == 0)
            {
                return EXIT_FAILURE;
            }

            UnitPtr u = Unit::createUnit(false, false, ice, underscore);
            int parseStatus = u->parse(*i, cppHandle, debug);
            u->destroy();

            if(parseStatus == EXIT_FAILURE)
            {
                return EXIT_FAILURE;
            }

            if(!icecpp->printMakefileDependencies(os, depend ? Preprocessor::PHP : Preprocessor::SliceXML,
                                                  includePaths, "-D__SLICE2PHP__"))
            {
                return EXIT_FAILURE;
            }

            if(!icecpp->close())
            {
                return EXIT_FAILURE;
            }
        }
        else
        {
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
            FILE* cppHandle = icecpp->preprocess(false, "-D__SLICE2PHP__");

            if(cppHandle == 0)
            {
                return EXIT_FAILURE;
            }

            if(preprocess)
            {
                char buf[4096];
                while(fgets(buf, static_cast<int>(sizeof(buf)), cppHandle) != ICE_NULLPTR)
                {
                    if(fputs(buf, stdout) == EOF)
                    {
                        return EXIT_FAILURE;
                    }
                }
                if(!icecpp->close())
                {
                    return EXIT_FAILURE;
                }
            }
            else
            {
                UnitPtr u = Unit::createUnit(false, all, ice, underscore);
                int parseStatus = u->parse(*i, cppHandle, debug);

                if(!icecpp->close())
                {
                    u->destroy();
                    return EXIT_FAILURE;
                }

                if(parseStatus == EXIT_FAILURE)
                {
                    status = EXIT_FAILURE;
                }
                else
                {
                    string base = icecpp->getBaseName();
                    string::size_type pos = base.find_last_of("/\\");
                    if(pos != string::npos)
                    {
                        base.erase(0, pos + 1);
                    }

                    string file = base + ".php";
                    if(!output.empty())
                    {
                        file = output + '/' + file;
                    }

                    try
                    {
                        IceUtilInternal::Output out;
                        out.open(file.c_str());
                        if(!out)
                        {
                            ostringstream os;
                            os << "cannot open`" << file << "': " << IceUtilInternal::errorToString(errno);
                            throw FileException(__FILE__, __LINE__, os.str());
                        }
                        FileTracker::instance()->addFile(file);

                        out << "<?php\n";
                        printHeader(out);
                        printGeneratedHeader(out, base + ".ice");

                        //
                        // Generate the PHP mapping.
                        //
                        generate(u, all, checksum, ns, includePaths, out);

                        out << "?>\n";
                        out.close();
                    }
                    catch(const Slice::FileException& ex)
                    {
                        // If a file could not be created, then cleanup any
                        // created files.
                        FileTracker::instance()->cleanup();
                        u->destroy();
                        consoleErr << argv[0] << ": error: " << ex.reason() << endl;
                        return EXIT_FAILURE;
                    }
                    catch(const string& err)
                    {
                        FileTracker::instance()->cleanup();
                        consoleErr << argv[0] << ": error: " << err << endl;
                        status = EXIT_FAILURE;
                    }
                }

                u->destroy();
            }
        }

        {
            IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(globalMutex);

            if(interrupted)
            {
                FileTracker::instance()->cleanup();
                return EXIT_FAILURE;
            }
        }
    }

    if(dependxml)
    {
        os << "</dependencies>\n";
    }

    if(depend || dependxml)
    {
        writeDependencies(os.str(), dependFile);
    }

    return status;
}

#ifdef _WIN32
int wmain(int argc, wchar_t* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    vector<string> args = Slice::argvToArgs(argc, argv);
    try
    {
        return compile(args);
    }
    catch(const std::exception& ex)
    {
        consoleErr << args[0] << ": error:" << ex.what() << endl;
        return EXIT_FAILURE;
    }
    catch(...)
    {
        consoleErr << args[0] << ": error:" << "unknown exception" << endl;
        return EXIT_FAILURE;
    }
}
