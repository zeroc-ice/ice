// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/PythonUtil.h>
#include <Slice/Checksum.h>
#include <IceUtil/Functional.h>

using namespace std;
using namespace Slice;
using namespace IceUtil;

namespace Slice
{
namespace Python
{

//
// ModuleVisitor finds all of the Slice modules whose include level is greater
// than 0 and emits a statement of the following form:
//
// _M_Foo = Ice.openModule('Foo')
//
// This statement allows the code generated for this translation unit to refer
// to types residing in those included modules.
//
class ModuleVisitor : public ParserVisitor
{
public:

    ModuleVisitor(Output&);

    virtual bool visitModuleStart(const ModulePtr&);

private:

    Output& _out;
    set<string> _history;
};

//
// CodeVisitor generates the Python mapping for a translation unit.
//
class CodeVisitor : public ParserVisitor
{
public:

    CodeVisitor(IceUtil::Output&);

    virtual bool visitModuleStart(const ModulePtr&);
    virtual void visitModuleEnd(const ModulePtr&);
    virtual void visitClassDecl(const ClassDeclPtr&);
    virtual bool visitClassDefStart(const ClassDefPtr&);
    virtual bool visitExceptionStart(const ExceptionPtr&);
    virtual bool visitStructStart(const StructPtr&);
    virtual void visitSequence(const SequencePtr&);
    virtual void visitDictionary(const DictionaryPtr&);
    virtual void visitEnum(const EnumPtr&);
    virtual void visitConst(const ConstPtr&);

protected:

    //
    // Return a Python symbol for the given parser element.
    //
    string getSymbol(const ContainedPtr&);

    //
    // Emit Python code to assign the given symbol in the current module.
    //
    void registerName(const string&);

    //
    // Emit the tuple for a Slice type.
    //
    void writeType(const TypePtr&);

    //
    // Write a default value for a given type.
    //
    void writeDefaultValue(const TypePtr&);

    //
    // Converts a scoped name into a type name (e.g., ::M::Foo -> M._t_Foo).
    //
    string scopedToType(const string&);

    struct ExceptionDataMember
    {
        string fixedName;
        TypePtr type;
        bool inherited;
    };

    void collectExceptionMembers(const ExceptionPtr&, list<ExceptionDataMember>&, bool);

    Output& _out;
    list<string> _moduleStack;
    set<string> _classHistory;
};

}
}

static string
lookupKwd(const string& name)
{
    //
    // Keyword list. *Must* be kept in alphabetical order.
    //
    static const string keywordList[] = 
    {       
        "and", "assert", "break", "class", "continue", "def", "del", "elif", "else", "except", "exec",
        "finally", "for", "from", "global", "if", "import", "in", "is", "lambda", "not", "or", "pass",
        "print", "raise", "return", "try", "while", "yield"
    };
    bool found =  binary_search(&keywordList[0],
                                &keywordList[sizeof(keywordList) / sizeof(*keywordList)],
                                name);
    return found ? "_" + name : name;
}

//
// Split a scoped name into its components and return the components as a list of (unscoped) identifiers.
//
static vector<string>
splitScopedName(const string& scoped)
{
    assert(scoped[0] == ':');
    vector<string> ids;
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

//
// ModuleVisitor implementation.
//
Slice::Python::ModuleVisitor::ModuleVisitor(Output& out) :
    _out(out)
{
}

bool
Slice::Python::ModuleVisitor::visitModuleStart(const ModulePtr& p)
{
    if(p->includeLevel() > 0)
    {
        string scoped = p->scoped();
        if(_history.count(scoped) == 0)
        {
            string name = scopedToName(scoped);
            _out << sp << nl << "# Included module " << name;
            _out << nl << "_M_" << name << " = Ice.openModule('" << name << "')";
            _history.insert(scoped);
        }
    }

    return true;
}

//
// CodeVisitor implementation.
//
Slice::Python::CodeVisitor::CodeVisitor(Output& out) :
    _out(out)
{
}

bool
Slice::Python::CodeVisitor::visitModuleStart(const ModulePtr& p)
{
    //
    // As each module is opened, we emit the statement
    //
    // __name__ = 'Foo'
    //
    // This renames the current module to 'Foo' so that subsequent
    // type definitions have the proper fully-qualified name.
    //
    // We also emit the statement
    //
    // _M_Foo = Ice.openModule('Foo')
    //
    // This allows us to create types in the module Foo.
    //
    string name = scopedToName(p->scoped());
    _out << sp << nl << "# Start of module " << name;
    _out << nl << "__name__ = '" << name << "'";
    _out << nl << "_M_" << name << " = Ice.openModule('" << name << "')";
    _moduleStack.push_front(name);
    return true;
}

void
Slice::Python::CodeVisitor::visitModuleEnd(const ModulePtr& p)
{
    assert(!_moduleStack.empty());
    _out << sp << nl << "# End of module " << _moduleStack.front();
    _moduleStack.pop_front();

    if(!_moduleStack.empty())
    {
        _out << sp << nl << "__name__ = '" << _moduleStack.front() << "'";
    }
}

void
Slice::Python::CodeVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    //
    // Emit forward declarations.
    //
    string scoped = p->scoped();
    if(!p->isLocal() && _classHistory.count(scoped) == 0)
    {
        string type = scopedToType(scoped);
        _out << sp << nl << "_M_" << type << " = IcePy.declareClass('" << scoped << "')";
        _out << nl << "_M_" << type << "Prx = IcePy.declareProxy('" << scoped << "')";
        _classHistory.insert(scoped); // Avoid extra declarations.
    }
}

bool
Slice::Python::CodeVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string scoped = p->scoped();
    string type = scopedToType(scoped);
    string fixedScoped = scopedToName(scoped);
    string fixedName = fixIdent(p->name());
    ClassList bases = p->bases();
    OperationList ops = p->operations();
    OperationList::iterator oli;

    //
    // Define the class.
    //
    _out << sp << nl << "_M_" << fixedScoped << " = Ice.createTempClass()";
    _out << nl << "class " << fixedName << '(';
    string baseScoped;
    if(bases.empty())
    {
        if(p->isLocal())
        {
            _out << "Ice.LocalObject";
        }
        else
        {
            _out << "Ice.Object";
        }
    }
    else
    {
        for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
        {
            if(q != bases.begin())
            {
                _out << ", ";
            }
            _out << getSymbol(*q);
        }
        if(!bases.front()->isInterface())
        {
            baseScoped = bases.front()->scoped();
        }
    }
    _out << "):";

    _out.inc();
    if(p->isLocal())
    {
        _out << nl << "pass";
    }
    else
    {
        //
        // ice_ids
        //
        ClassList allBases = p->allBases();
        StringList ids;
#if defined(__IBMCPP__) && defined(NDEBUG)
//
// VisualAge C++ 6.0 does not see that ClassDef is a Contained,
// when inlining is on. The code below issues a warning: better
// than an error!
//
        transform(allBases.begin(), allBases.end(), back_inserter(ids),
                  IceUtil::constMemFun<string,ClassDef>(&Contained::scoped));
#else
        transform(allBases.begin(), allBases.end(), back_inserter(ids), IceUtil::constMemFun(&Contained::scoped));
#endif
        StringList other;
        other.push_back(scoped);
        other.push_back("::Ice::Object");
        other.sort();
        ids.merge(other);
        ids.unique();
        _out << sp << nl << "def ice_ids(self, current=None):";
        _out.inc();
        _out << nl << "return (";
        for(StringList::iterator q = ids.begin(); q != ids.end(); ++q)
        {
            if(q != ids.begin())
            {
                _out << ", ";
            }
            _out << "'" << *q << "'";
        }
        _out << ')';
        _out.dec();

        //
        // ice_id
        //
        _out << sp << nl << "def ice_id(self, current=None):";
        _out.inc();
        _out << nl << "return '" << scoped << "'";
        _out.dec();
    }

    if(!ops.empty())
    {
        //
        // Emit a placeholder for each operation.
        //
        _out << sp
             << nl << "#"
             << nl << "# Operation signatures."
             << nl << "#";
        for(oli = ops.begin(); oli != ops.end(); ++oli)
        {
            string fixedOpName = fixIdent((*oli)->name());
            if(!p->isLocal() && (p->hasMetaData("amd") || (*oli)->hasMetaData("amd")))
            {
                _out << nl << "# def " << fixedOpName << "_async(self, _cb";

                ParamDeclList params = (*oli)->parameters();

                for(ParamDeclList::iterator pli = params.begin(); pli != params.end(); ++pli)
                {
                    if(!(*pli)->isOutParam())
                    {
                        _out << ", " << fixIdent((*pli)->name());
                    }
                }
                if(!p->isLocal())
                {
                    _out << ", current=None";
                }
                _out << "):";
            }
            else
            {
                _out << nl << "# def " << fixedOpName << "(self";

                ParamDeclList params = (*oli)->parameters();

                for(ParamDeclList::iterator pli = params.begin(); pli != params.end(); ++pli)
                {
                    if(!(*pli)->isOutParam())
                    {
                        _out << ", " << fixIdent((*pli)->name());
                    }
                }
                if(!p->isLocal())
                {
                    _out << ", current=None";
                }
                _out << "):";
            }
        }
    }
    _out.dec();

    //
    // Define the proxy class.
    //
    if(!p->isLocal())
    {
        _out << sp << nl << "_M_" << fixedScoped << "Prx = Ice.createTempClass()";
        _out << nl << "class " << fixedName << "Prx(";
        if(bases.empty())
        {
            _out << "IcePy.ObjectPrx";
        }
        else
        {
            ClassList::const_iterator q = bases.begin();
            while(q != bases.end())
            {
                _out << getSymbol(*q) << "Prx";
                if(++q != bases.end())
                {
                    _out << ", ";
                }
            }
        }
        _out << "):";
        _out.inc();

        for(oli = ops.begin(); oli != ops.end(); ++oli)
        {
            string fixedOpName = fixIdent((*oli)->name());
            TypePtr ret = (*oli)->returnType();
            ParamDeclList paramList = (*oli)->parameters();
            string inParams;

            for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
            {
                if(!(*q)->isOutParam())
                {
                    if(!inParams.empty())
                    {
                        inParams.append(", ");
                    }
                    inParams.append(fixIdent((*q)->name()));
                }
            }

            _out << sp << nl << "def " << fixedOpName << "(self";
            if(!inParams.empty())
            {
                _out << ", " << inParams;
            }
            _out << ", ctx=None):";
            _out.inc();
            _out << nl << "return _M_" << fixedScoped << "._op_" << (*oli)->name() << ".invoke(self, (" << inParams;
            if(!inParams.empty() && inParams.find(',') == string::npos)
            {
                _out << ", ";
            }
            _out << "), ctx)";
            _out.dec();

            if(p->hasMetaData("ami") || (*oli)->hasMetaData("ami"))
            {
                _out << sp << nl << "def " << fixedOpName << "_async(self, _cb";
                if(!inParams.empty())
                {
                    _out << ", " << inParams;
                }
                _out << ", ctx=None):";
                _out.inc();
                _out << nl << "return _M_" << fixedScoped << "._op_" << (*oli)->name() << ".invokeAsync(self, _cb, ("
                     << inParams;
                if(!inParams.empty() && inParams.find(',') == string::npos)
                {
                    _out << ", ";
                }
                _out << "), ctx)";
                _out.dec();
            }
        }

        _out << sp << nl << "def checkedCast(proxy, facet=''):";
        _out.inc();
        _out << nl << "return _M_" << fixedScoped << "Prx.ice_checkedCast(proxy, '" << scoped << "', facet)";
        _out.dec();
        _out << nl << "checkedCast = staticmethod(checkedCast)";

        _out << sp << nl << "def uncheckedCast(proxy, facet=''):";
        _out.inc();
        _out << nl << "return _M_" << fixedScoped << "Prx.ice_uncheckedCast(proxy, facet)";
        _out.dec();
        _out << nl << "uncheckedCast = staticmethod(uncheckedCast)";

        _out.dec();

        _out << sp << nl << "_M_" << type << "Prx = IcePy.defineProxy('" << scoped << "', " << fixedName << "Prx)";
    }

    //
    // Emit the type information for a non-local class.
    //
    if(!p->isLocal())
    {
        if(_classHistory.count(scoped) == 0 && p->canBeCyclic())
        {
            //
            // Emit a forward declaration for the class in case a data member refers to this type.
            //
            _out << sp << nl << "_M_" << type << " = IcePy.declareClass('" << scoped << "')";
        }

        DataMemberList members = p->dataMembers();
        _out << sp << nl << "_M_" << type << " = IcePy.defineClass('" << scoped << "', " << fixedName << ", "
             << (p->isAbstract() ? "True" : "False") << ", ";
        if(baseScoped.empty())
        {
            _out << "None";
        }
        else
        {
            _out << "_M_" << scopedToType(baseScoped);
        }
        _out << ", (";
        //
        // Interfaces
        //
        int interfaceCount = 0;
        for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
        {
            if((*q)->isInterface())
            {
                if(interfaceCount > 0)
                {
                    _out << ", ";
                }
                _out << "_M_" << scopedToType((*q)->scoped());
                ++interfaceCount;
            }
        }
        if(interfaceCount == 1)
        {
            _out << ',';
        }
        //
        // Members
        //
        // Data members are represented as a tuple:
        //
        //   ('MemberName', MemberType)
        //
        // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a constructed type.
        //
        _out << "), (";
        if(members.size() > 1)
        {
            _out.inc();
            _out << nl;
        }
        for(DataMemberList::iterator r = members.begin(); r != members.end(); ++r)
        {
            if(r != members.begin())
            {
                _out << ',' << nl;
            }
            _out << "('" << fixIdent((*r)->name()) << "', ";
            writeType((*r)->type());
            _out << ')';
        }
        if(members.size() == 1)
        {
            _out << ',';
        }
        else if(members.size() > 1)
        {
            _out.dec();
            _out << nl;
        }
        _out << "))";

        //
        // Define each operation. The arguments to the IcePy.Operation constructor are:
        //
        // 'opName', Mode, (InParams), (OutParams), ReturnType, (Exceptions)
        //
        // where InParams and OutParams are tuples of type descriptions, and Exceptions
        // is a tuple of exception type ids.
        //
        if(!ops.empty())
        {
            _out << sp;
        }
        for(OperationList::iterator s = ops.begin(); s != ops.end(); ++s)
        {
            ParamDeclList params = (*s)->parameters();
            ParamDeclList::iterator t;
            int count;

            _out << nl << fixedName << "._op_" << (*s)->name() << " = IcePy.Operation('" << (*s)->name() << "', ";
            switch((*s)->mode())
            {
            case Operation::Normal:
                _out << "Ice.OperationMode.Normal";
                break;
            case Operation::Nonmutating:
                _out << "Ice.OperationMode.Nonmutating";
                break;
            case Operation::Idempotent:
                _out << "Ice.OperationMode.Idempotent";
                break;
            }
            _out << ", " << ((p->hasMetaData("amd") || (*s)->hasMetaData("amd")) ? "True" : "False") << ", (";
            for(t = params.begin(), count = 0; t != params.end(); ++t)
            {
                if(!(*t)->isOutParam())
                {
                    if(count > 0)
                    {
                        _out << ", ";
                    }
                    writeType((*t)->type());
                    ++count;
                }
            }
            if(count == 1)
            {
                _out << ',';
            }
            _out << "), (";
            for(t = params.begin(), count = 0; t != params.end(); ++t)
            {
                if((*t)->isOutParam())
                {
                    if(count > 0)
                    {
                        _out << ", ";
                    }
                    writeType((*t)->type());
                    ++count;
                }
            }
            if(count == 1)
            {
                _out << ',';
            }
            _out << "), ";
            TypePtr returnType = (*s)->returnType();
            if(returnType)
            {
                writeType(returnType);
            }
            else
            {
                _out << "None";
            }
            _out << ", (";
            ExceptionList exceptions = (*s)->throws();
            for(ExceptionList::iterator u = exceptions.begin(); u != exceptions.end(); ++u)
            {
                if(u != exceptions.begin())
                {
                    _out << ", ";
                }
                _out << "_M_" << scopedToType((*u)->scoped());
            }
            if(exceptions.size() == 1)
            {
                _out << ',';
            }
            _out << "))";
        }
    }

    registerName(fixedName);

    if(!p->isLocal())
    {
        registerName(fixedName + "Prx");
    }

    return false;
}

bool
Slice::Python::CodeVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string scoped = p->scoped();
    string fixedScoped = scopedToName(scoped);
    string fixedName = fixIdent(p->name());
    _out << sp << nl << "_M_" << fixedScoped << " = Ice.createTempClass()";
    _out << nl << "class " << fixedName << '(';
    ExceptionPtr base = p->base();
    string baseScoped, baseName;
    if(base)
    {
        baseScoped = base->scoped();
        baseName = getSymbol(base);
        _out << baseName;
    }
    else if(p->isLocal())
    {
        _out << "Ice.LocalException";
    }
    else
    {
        _out << "Ice.UserException";
    }
    _out << "):";
    _out.inc();

    DataMemberList members = p->dataMembers();
    DataMemberList::iterator dmli;

    //
    // __init__
    //
    _out << nl << "def __init__(self";
    list<ExceptionDataMember> allMembers;
    collectExceptionMembers(p, allMembers, false);
    if(!allMembers.empty())
    {
        for(list<ExceptionDataMember>::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
        {
            _out << ", " << q->fixedName << '=';
            writeDefaultValue(q->type);
        }
    }
    _out << "):";
    _out.inc();
    if(!base && members.empty())
    {
        _out << nl << "pass";
    }
    else
    {
        if(base)
        {
            _out << nl << baseName << ".__init__(self";
            for(list<ExceptionDataMember>::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
            {
                if(q->inherited)
                {
                    _out << ", " << q->fixedName;
                }
            }
            _out << ')';
        }
        for(list<ExceptionDataMember>::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
        {
            if(!q->inherited)
            {
                _out << nl << "self." << q->fixedName << " = " << q->fixedName;;
            }
        }
    }
    _out.dec();

    //
    // ice_id
    //
    _out << sp << nl << "def ice_id(self):";
    _out.inc();
    _out << nl << "return '" << scoped << "'";
    _out.dec();

    //
    // __str__
    //
    _out << sp << nl << "def __str__(self):";
    _out.inc();
    if(allMembers.empty())
    {
        _out << nl << "return '" << fixedScoped << "'";
    }
    else
    {
        _out << nl << "return '" << fixedScoped << ":'";
        _out.inc();
        for(list<ExceptionDataMember>::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
        {
            _out << " +\\"
                 << nl << "'\\n" << q->fixedName << ": ' + str(self." << q->fixedName << ')';
        }
        _out.dec();
    }
    _out.dec();

    _out.dec();

    //
    // Emit the type information for a non-local exception.
    //
    if(!p->isLocal())
    {
        _out << sp << nl << "_M_" << scopedToType(scoped) << " = IcePy.defineException('" << scoped << "', "
             << fixedName << ", ";
        if(baseScoped.empty())
        {
            _out << "None";
        }
        else
        {
             _out << "_M_" << scopedToType(baseScoped);
        }
        _out << ", (";
        if(members.size() > 1)
        {
            _out.inc();
            _out << nl;
        }
        //
        // Data members are represented as a tuple:
        //
        //   ('MemberName', MemberType)
        //
        // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a constructed type.
        //
        for(dmli = members.begin(); dmli != members.end(); ++dmli)
        {
            if(dmli != members.begin())
            {
                _out << ',' << nl;
            }
            _out << "(\"" << fixIdent((*dmli)->name()) << "\", ";
            writeType((*dmli)->type());
            _out << ')';
        }
        if(members.size() == 1)
        {
            _out << ',';
        }
        else if(members.size() > 1)
        {
            _out.dec();
            _out << nl;
        }
        _out << "), " << (p->usesClasses() ? "True" : "False") << ")";
    }

    registerName(fixedName);

    return false;
}

bool
Slice::Python::CodeVisitor::visitStructStart(const StructPtr& p)
{
    string scoped = p->scoped();
    string fixedScoped = scopedToName(scoped);
    string fixedName = fixIdent(p->name());
    DataMemberList members = p->dataMembers();
    DataMemberList::iterator q;

    _out << sp << nl << "_M_" << fixedScoped << " = Ice.createTempClass()";
    _out << nl << "class " << fixedName << "(object):";
    _out.inc();
    _out << nl << "def __init__(self";
    for(q = members.begin(); q != members.end(); ++q)
    {
        _out << ", " << fixIdent((*q)->name()) << '=';
        writeDefaultValue((*q)->type());
    }
    _out << "):";
    _out.inc();
    for(q = members.begin(); q != members.end(); ++q)
    {
        string s = fixIdent((*q)->name());
        _out << nl << "self." << s << " = " << s;
    }
    _out.dec();

    _out.dec();

    //
    // Emit the type information for a non-local struct.
    //
    if(!p->isLocal())
    {
        _out << sp << nl << "_M_" << scopedToType(scoped) << " = IcePy.defineStruct('" << scoped << "', "
             << fixedName << ", (";
        //
        // Data members are represented as a tuple:
        //
        //   ('MemberName', MemberType)
        //
        // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a constructed type.
        //
        if(members.size() > 1)
        {
            _out.inc();
            _out << nl;
        }
        for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
        {
            if(q != members.begin())
            {
                _out << ',' << nl;
            }
            _out << "(\"" << fixIdent((*q)->name()) << "\", ";
            writeType((*q)->type());
            _out << ')';
        }
        if(members.size() == 1)
        {
            _out << ',';
        }
        else if(members.size() > 1)
        {
            _out.dec();
            _out << nl;
        }
        _out << "))";
    }

    registerName(fixedName);

    return false;
}

void
Slice::Python::CodeVisitor::visitSequence(const SequencePtr& p)
{
    //
    // Emit the type information for a non-local sequence.
    //
    if(!p->isLocal())
    {
        string scoped = p->scoped();
        _out << sp << nl << "_M_" << scopedToType(scoped) << " = IcePy.defineSequence('" << scoped << "', ";
        writeType(p->type());
        _out << ")";
    }
}

void
Slice::Python::CodeVisitor::visitDictionary(const DictionaryPtr& p)
{
    //
    // Emit the type information for a non-local dictionary.
    //
    if(!p->isLocal())
    {
        string scoped = p->scoped();
        _out << sp << nl << "_M_" << scopedToType(scoped) << " = IcePy.defineDictionary('" << scoped << "', ";
        writeType(p->keyType());
        _out << ", ";
        writeType(p->valueType());
        _out << ")";
    }
}

void
Slice::Python::CodeVisitor::visitEnum(const EnumPtr& p)
{
    string scoped = p->scoped();
    string fixedScoped = scopedToName(scoped);
    string fixedName = fixIdent(p->name());
    EnumeratorList enums = p->getEnumerators();
    EnumeratorList::iterator q;
    int i;

    _out << sp << nl << "_M_" << fixedScoped << " = Ice.createTempClass()";
    _out << nl << "class " << fixedName << "(object):";
    _out.inc();
    _out << sp << nl << "def __init__(self, val):";
    _out.inc();
    _out << nl << "self.value = val";
    _out.dec();
    _out << sp << nl << "def __str__(self):";
    _out.inc();
    for(q = enums.begin(), i = 0; q != enums.end(); ++q, ++i)
    {
        _out << nl;
        if(q == enums.begin())
        {
            _out << "if";
        }
        else
        {
            _out << "elif";
        }
        ostringstream idx;
        idx << i;
        _out << " self.value == " << idx.str() << ':';
        _out.inc();
        _out << nl << "return '" << (*q)->name() << "'";
        _out.dec();
    }
    _out << nl << "return None";
    _out.dec();
    _out << sp << nl << "__repr__ = __str__";
    _out << sp << nl << "def __hash__(self):";
    _out.inc();
    _out << nl << "return self.value";
    _out.dec();
    _out << sp << nl << "def __cmp__(self, other):";
    _out.inc();
    _out << nl << "return cmp(self.value, other.value)";
    _out.dec();
    _out.dec();

    _out << sp;
    for(q = enums.begin(), i = 0; q != enums.end(); ++q, ++i)
    {
        string fixedEnum = fixIdent((*q)->name());
        ostringstream idx;
        idx << i;
        _out << nl << fixedName << '.' << fixedEnum << " = " << fixedName << '(' << idx.str() << ')';
    }

    //
    // Emit the type information for a non-local enum.
    //
    if(!p->isLocal())
    {
        _out << sp << nl << "_M_" << scopedToType(scoped) << " = IcePy.defineEnum('" << scoped << "', " << fixedName
             << ", (";
        for(EnumeratorList::iterator q = enums.begin(); q != enums.end(); ++q)
        {
            if(q != enums.begin())
            {
                _out << ", ";
            }
            string fixedEnum = fixIdent((*q)->name());
            _out << fixedName << '.' << fixedEnum;
        }
        if(enums.size() == 1)
        {
            _out << ',';
        }
        _out << "))";
    }

    registerName(fixedName);
}

void
Slice::Python::CodeVisitor::visitConst(const ConstPtr& p)
{
    Slice::TypePtr type = p->type();
    string value = p->value();
    string name = fixIdent(p->name());

    _out << sp << nl << "_M_" << scopedToName(p->scoped()) << " = ";

    Slice::BuiltinPtr b = Slice::BuiltinPtr::dynamicCast(type);
    Slice::EnumPtr en = Slice::EnumPtr::dynamicCast(type);
    if(b)
    {
        switch(b->kind())
        {
        case Slice::Builtin::KindBool:
        {
            _out << (value == "true" ? "True" : "False");
            break;
        }
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
            string::size_type pos;
            IceUtil::stringToInt64(value, l, pos);
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
            //
            // Expand strings into the basic source character set. We can't use isalpha() and the like
            // here because they are sensitive to the current locale.
            //
            static const string basicSourceChars = "abcdefghijklmnopqrstuvwxyz"
                                                   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                                   "0123456789"
                                                   "_{}[]#()<>%:;,?*+=/^&|~!=, '";
            static const set<char> charSet(basicSourceChars.begin(), basicSourceChars.end());

            _out << "\"";                                       // Opening "

            ios_base::fmtflags originalFlags = _out.flags();    // Save stream state
            streamsize originalWidth = _out.width();
            ostream::char_type originalFill = _out.fill();

            for(string::const_iterator c = value.begin(); c != value.end(); ++c)
            {
                switch(*c)
                {
                case '"':
                {
                    _out << "\\\"";
                    break;
                }
                case '\\':
                {
                    _out << "\\\\";
                    break;
                }
                case '\r':
                {
                    _out << "\\r";
                    break;
                }
                case '\n':
                {
                    _out << "\\n";
                    break;
                }
                case '\t':
                {
                    _out << "\\t";
                    break;
                }
                case '\b':
                {
                    _out << "\\b";
                    break;
                }
                case '\f':
                {
                    _out << "\\f";
                    break;
                }
                default:
                {
                    if(charSet.find(*c) == charSet.end())
                    {
                        unsigned char uc = *c;                  // Char may be signed, so make it positive.
                        _out << "\\";                           // Print as octal if not in basic source character set.
                        _out.flags(ios_base::oct);
                        _out.width(3);
                        _out.fill('0');
                        _out << static_cast<unsigned>(uc);
                    }
                    else
                    {
                        _out << *c;                             // Print normally if in basic source character set.
                    }
                    break;
                }
                }
            }

            _out.fill(originalFill);                            // Restore stream state
            _out.width(originalWidth);
            _out.flags(originalFlags);

            _out << "\"";                                       // Closing "
            break;
        }

        case Slice::Builtin::KindObject:
        case Slice::Builtin::KindObjectProxy:
        case Slice::Builtin::KindLocalObject:
            assert(false);
        }
    }
    else if(en)
    {
        string enumName = getSymbol(en);
        string::size_type colon = value.rfind(':');
        string enumerator;
        if(colon != string::npos)
        {
            enumerator = fixIdent(value.substr(colon + 1));
        }
        else
        {
            enumerator = fixIdent(value);
        }
        _out << enumName << '.' << enumerator;
    }
    else
    {
        assert(false); // Unknown const type.
    }
}

string
Slice::Python::CodeVisitor::getSymbol(const ContainedPtr& p)
{
    //
    // An explicit reference to another type must always be prefixed with "_M_".
    //
    return "_M_" + scopedToName(p->scoped());
}

void
Slice::Python::CodeVisitor::registerName(const string& name)
{
    assert(!_moduleStack.empty());
    _out << sp << nl << "_M_" << _moduleStack.front() << '.' << name << " = " << name;
    _out << nl << "del " << name;
}

void
Slice::Python::CodeVisitor::writeType(const TypePtr& p)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(p);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindBool:
            {
                _out << "IcePy._t_bool";
                break;
            }
            case Builtin::KindByte:
            {
                _out << "IcePy._t_byte";
                break;
            }
            case Builtin::KindShort:
            {
                _out << "IcePy._t_short";
                break;
            }
            case Builtin::KindInt:
            {
                _out << "IcePy._t_int";
                break;
            }
            case Builtin::KindLong:
            {
                _out << "IcePy._t_long";
                break;
            }
            case Builtin::KindFloat:
            {
                _out << "IcePy._t_float";
                break;
            }
            case Builtin::KindDouble:
            {
                _out << "IcePy._t_double";
                break;
            }
            case Builtin::KindString:
            {
                _out << "IcePy._t_string";
                break;
            }
            case Builtin::KindObject:
            {
                _out << "IcePy._t_Object";
                break;
            }
            case Builtin::KindObjectProxy:
            {
                _out << "IcePy._t_ObjectPrx";
                break;
            }
            case Builtin::KindLocalObject:
            {
                assert(false);
                break;
            }
        }
        return;
    }

    ProxyPtr prx = ProxyPtr::dynamicCast(p);
    if(prx)
    {
        _out << "_M_" << scopedToType(prx->_class()->scoped()) << "Prx";
        return;
    }

    ContainedPtr cont = ContainedPtr::dynamicCast(p);
    assert(cont);
    _out << "_M_" << scopedToType(cont->scoped());
}

void
Slice::Python::CodeVisitor::writeDefaultValue(const TypePtr& p)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(p);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindBool:
            {
                _out << "False";
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
            {
                _out << "None";
                break;
            }
        }
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(p);
    if(en)
    {
        EnumeratorList enums = en->getEnumerators();
        _out << getSymbol(en) << "." << fixIdent(enums.front()->name());
        return;
    }

    StructPtr st = StructPtr::dynamicCast(p);
    if(st)
    {
        _out << getSymbol(st) << "()";
        return;
    }

    _out << "None";
}

string
Slice::Python::CodeVisitor::scopedToType(const string& scoped)
{
    string result = scopedToName(scoped);
    string::size_type pos = result.rfind('.');
    assert(pos != string::npos);
    result.insert(pos + 1, "_t_");
    return result;
}

void
Slice::Python::CodeVisitor::collectExceptionMembers(const ExceptionPtr& p, list<ExceptionDataMember>& allMembers,
                                                    bool inherited)
{
    ExceptionPtr base = p->base();
    if(base)
    {
        collectExceptionMembers(base, allMembers, true);
    }

    DataMemberList members = p->dataMembers();

    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
    {
        ExceptionDataMember m;
        m.fixedName = fixIdent((*q)->name());
        m.type = (*q)->type();
        m.inherited = inherited;
        allMembers.push_back(m);
    }
}

static string
changeInclude(const string& orig, const vector<string>& includePaths)
{
    string file = orig;
    replace(file.begin(), file.end(), '\\', '/');

    for(vector<string>::const_iterator p = includePaths.begin(); p != includePaths.end(); ++p)
    {
        string includePath = *p;
        replace(includePath.begin(), includePath.end(), '\\', '/');

        if(orig.compare(0, includePath.length(), *p) == 0)
        {
            string s = orig.substr(includePath.length());
            if(s.size() < file.size())
            {
                file = s;
            }
        }
    }

    string::size_type pos = file.rfind('.');
    if(pos != string::npos)
    {
        file.erase(pos);
    }

    return file;
}

void
Slice::Python::generate(const UnitPtr& unit, bool all, bool checksum, const vector<string>& includePaths, Output& out)
{
    out << nl << "import Ice, IcePy";

    if(!all)
    {
        vector<string> paths = includePaths;
        for(vector<string>::iterator p = paths.begin(); p != paths.end(); ++p)
        {
            if(p->length() && (*p)[p->length() - 1] != '/')
            {
                *p += '/';
            }
        }

        StringList includes = unit->includeFiles();
        for(StringList::const_iterator q = includes.begin(); q != includes.end(); ++q)
        {
            string file = changeInclude(*q, paths);
            replace(file.begin(), file.end(), '/', '_');
            out << nl << "import " << file << "_ice";
        }
    }

    ModuleVisitor moduleVisitor(out);
    unit->visit(&moduleVisitor, true);

    CodeVisitor codeVisitor(out);
    unit->visit(&codeVisitor, false);

    if(checksum)
    {
        ChecksumMap checksums = createChecksums(unit);
        if(!checksums.empty())
        {
            out << sp;
            for(ChecksumMap::const_iterator p = checksums.begin(); p != checksums.end(); ++p)
            {
                out << nl << "Ice.sliceChecksums[\"" << p->first << "\"] = \"";
                ostringstream str;
                str.flags(ios_base::hex);
                str.fill('0');
                for(vector<unsigned char>::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
                {
                    str << (int)(*q);
                }
                out << str.str() << "\"";
            }
        }
    }

    out << nl; // Trailing newline.
}

bool
Slice::Python::splitString(const string& str, vector<string>& args)
{
    string delim = " \t\n\r";
    string::size_type beg;
    string::size_type end = 0;
    while(true)
    {
        beg = str.find_first_not_of(delim, end);
        if(beg == string::npos)
        {
            break;
        }

        //
        // Check for quoted argument.
        //
        char ch = str[beg];
        if(ch == '"' || ch == '\'')
        {
            beg++;
            end = str.find(ch, beg);
            if(end == string::npos)
            {
                return false;
            }
            args.push_back(str.substr(beg, end - beg));
            end++; // Skip end quote.
        }
        else
        {
            end = str.find_first_of(delim + "'\"", beg);
            if(end == string::npos)
            {
                end = str.length();
            }
            args.push_back(str.substr(beg, end - beg));
        }
    }

    return true;
}

string
Slice::Python::scopedToName(const string& scoped)
{
    string result = fixIdent(scoped);
    if(result.find("::") == 0)
    {
        result.erase(0, 2);
    }

    string::size_type pos;
    while((pos = result.find("::")) != string::npos)
    {
        result.replace(pos, 2, ".");
    }

    return result;
}

string
Slice::Python::fixIdent(const string& ident)
{
    if(ident[0] != ':')
    {
        return lookupKwd(ident);
    }
    vector<string> ids = splitScopedName(ident);
    transform(ids.begin(), ids.end(), ids.begin(), ptr_fun(lookupKwd));
    stringstream result;
    for(vector<string>::const_iterator i = ids.begin(); i != ids.end(); ++i)
    {
        result << "::" + *i;
    }
    return result.str();
}

void
Slice::Python::printHeader(IceUtil::Output& out)
{
    static const char* header =
"# **********************************************************************\n"
"#\n"
"# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.\n"
"#\n"
"# This copy of Ice is licensed to you under the terms described in the\n"
"# ICE_LICENSE file included in this distribution.\n"
"#\n"
"# **********************************************************************\n"
        ;

    out << header;
    out << "\n# Ice version " << ICE_STRING_VERSION;
}
