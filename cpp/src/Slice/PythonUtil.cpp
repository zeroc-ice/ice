// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/PythonUtil.h>
#include <Slice/Checksum.h>
#include <IceUtil/Functional.h>
#ifdef __BCPLUSPLUS__
#  include <iterator>
#endif

using namespace std;
using namespace Slice;
using namespace IceUtil;

namespace Slice
{
namespace Python
{

class MetaDataVisitor : public ParserVisitor
{
public:

    virtual bool visitModuleStart(const ModulePtr&);
    virtual void visitClassDecl(const ClassDeclPtr&);
    virtual bool visitClassDefStart(const ClassDefPtr&);
    virtual bool visitExceptionStart(const ExceptionPtr&);
    virtual bool visitStructStart(const StructPtr&);
    virtual void visitOperation(const OperationPtr&);
    virtual void visitDataMember(const DataMemberPtr&);
    virtual void visitSequence(const SequencePtr&);
    virtual void visitDictionary(const DictionaryPtr&);
    virtual void visitEnum(const EnumPtr&);
    virtual void visitConst(const ConstPtr&);

private:

    //
    // Validates global metadata.
    //
    void validateGlobal(const DefinitionContextPtr&);

    //
    // Validates sequence metadata.
    //
    void validateSequence(const DefinitionContextPtr&, const string&, const TypePtr&, const StringList&);

    //
    // Checks a definition that doesn't currently support Python metadata.
    //
    void reject(const ContainedPtr&);

    StringSet _history;
};

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

    ModuleVisitor(Output&, set<string>&);

    virtual bool visitModuleStart(const ModulePtr&);

private:

    Output& _out;
    set<string>& _history;
};

//
// CodeVisitor generates the Python mapping for a translation unit.
//
class CodeVisitor : public ParserVisitor
{
public:

    CodeVisitor(IceUtil::Output&, set<string>&);

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

private:

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
    // Add a value to a hash code.
    //
    void writeHash(const string&, const TypePtr&, int&);

    //
    // Write Python metadata as a tuple.
    //
    void writeMetaData(const StringList&);

    //
    // Convert an operation mode into a string.
    //
    string getOperationMode(Slice::Operation::Mode);

    struct MemberInfo
    {
        string fixedName;
        TypePtr type;
        bool inherited;
        StringList metaData;
    };
    typedef list<MemberInfo> MemberInfoList;

    void collectClassMembers(const ClassDefPtr&, MemberInfoList&, bool);
    void collectExceptionMembers(const ExceptionPtr&, MemberInfoList&, bool);

    Output& _out;
    set<string>& _moduleHistory;
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

static string
getDictLookup(const ContainedPtr& cont, const string& suffix = string())
{
    string scope = Slice::Python::scopedToName(cont->scope());
    assert(!scope.empty());

    string package = Slice::Python::getPackageMetadata(cont);
    if(!package.empty())
    {
        scope = package + "." + scope;
    }

    return "_M_" + scope + "__dict__.has_key('" + suffix + Slice::Python::fixIdent(cont->name()) + "')";
}

//
// ModuleVisitor implementation.
//
Slice::Python::ModuleVisitor::ModuleVisitor(Output& out, set<string>& history) :
    _out(out), _history(history)
{
}

bool
Slice::Python::ModuleVisitor::visitModuleStart(const ModulePtr& p)
{
    if(p->includeLevel() > 0)
    {
        string abs = getAbsolute(p);
        if(_history.count(abs) == 0)
        {
            //
            // If this is a top-level module, then we check if it has package metadata.
            // If so, we need to emit statements to open each of the modules in the
            // package before we can open this module.
            //
            if(UnitPtr::dynamicCast(p->container()))
            {
                string pkg = getPackageMetadata(p);
                if(!pkg.empty())
                {
                    vector<string> v;
                    splitString(pkg, v, ".");
                    string mod;
                    for(vector<string>::iterator q = v.begin(); q != v.end(); ++q)
                    {
                        mod = mod.empty() ? *q : mod + "." + *q;
                        if(_history.count(mod) == 0)
                        {
                            _out << nl << "_M_" << mod << " = Ice.openModule('" << mod << "')";
                            _history.insert(mod);
                        }
                    }
                }
            }

            _out << sp << nl << "# Included module " << abs;
            _out << nl << "_M_" << abs << " = Ice.openModule('" << abs << "')";
            _history.insert(abs);
        }
    }

    return true;
}

//
// CodeVisitor implementation.
//
Slice::Python::CodeVisitor::CodeVisitor(Output& out, set<string>& moduleHistory) :
    _out(out), _moduleHistory(moduleHistory)
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
    string abs = getAbsolute(p);
    _out << sp << nl << "# Start of module " << abs;
    if(_moduleHistory.count(abs) == 0) // Don't emit this more than once for each module.
    {
        //
        // If this is a top-level module, then we check if it has package metadata.
        // If so, we need to emit statements to open each of the modules in the
        // package before we can open this module.
        //
        if(UnitPtr::dynamicCast(p->container()))
        {
            string pkg = getPackageMetadata(p);
            if(!pkg.empty())
            {
                vector<string> v;
                splitString(pkg, v, ".");
                string mod;
                for(vector<string>::iterator q = v.begin(); q != v.end(); ++q)
                {
                    mod = mod.empty() ? *q : mod + "." + *q;
                    if(_moduleHistory.count(mod) == 0) // Don't emit this more than once for each module.
                    {
                        _out << nl << "_M_" << mod << " = Ice.openModule('" << mod << "')";
                        _moduleHistory.insert(mod);
                    }
                }
            }
        }
        _out << nl << "_M_" << abs << " = Ice.openModule('" << abs << "')";
        _moduleHistory.insert(abs);
    }
    _out << nl << "__name__ = '" << abs << "'";
    _moduleStack.push_front(abs);
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
    if(_classHistory.count(scoped) == 0)
    {
        _out << sp << nl << "if not " << getDictLookup(p) << ':';
        _out.inc();
        string type = getAbsolute(p, "_t_");
        _out << nl << "_M_" << type << " = IcePy.declareClass('" << scoped << "')";
        if(!p->isLocal())
        {
            _out << nl << "_M_" << type << "Prx = IcePy.declareProxy('" << scoped << "')";
        }
        _out.dec();
        _classHistory.insert(scoped); // Avoid redundant declarations.
    }
}

bool
Slice::Python::CodeVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string scoped = p->scoped();
    string type = getAbsolute(p, "_t_");
    string abs = getAbsolute(p);
    string name = fixIdent(p->name());
    ClassList bases = p->bases();
    ClassDefPtr base;
    OperationList ops = p->operations();
    OperationList::iterator oli;

    //
    // Define the class.
    //
    _out << sp << nl << "if not " << getDictLookup(p) << ':';
    _out.inc();
    _out << nl << "_M_" << abs << " = Ice.createTempClass()";
    _out << nl << "class " << name << '(';
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
            base = bases.front();
        }
    }
    _out << "):";

    _out.inc();

    //
    // __init__
    //
    _out << nl << "def __init__(self";
    MemberInfoList allMembers;
    collectClassMembers(p, allMembers, false);
    if(!allMembers.empty())
    {
        for(MemberInfoList::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
        {
            _out << ", " << q->fixedName << '=';
            writeDefaultValue(q->type);
        }
    }
    _out << "):";
    _out.inc();
    if(!base && !p->hasDataMembers() && !p->isAbstract())
    {
        _out << nl << "pass";
    }
    else
    {
        if(p->isAbstract())
        {
            _out << nl << "if __builtin__.type(self) == _M_" << abs << ':';
            _out.inc();
            _out << nl << "raise RuntimeError('" << abs << " is an abstract class')";
            _out.dec();
        }
        if(base)
        {
            _out << nl << getSymbol(base) << ".__init__(self";
            for(MemberInfoList::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
            {
                if(q->inherited)
                {
                    _out << ", " << q->fixedName;
                }
            }
            _out << ')';
        }
        for(MemberInfoList::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
        {
            if(!q->inherited)
            {
                _out << nl << "self." << q->fixedName << " = " << q->fixedName;;
            }
        }
    }
    _out.dec();

    if(!p->isLocal())
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

    //
    // __str__
    //
    _out << sp << nl << "def __str__(self):";
    _out.inc();
    _out << nl << "return IcePy.stringify(self, _M_" << getAbsolute(p, "_t_") << ")";
    _out.dec();
    _out << sp << nl << "__repr__ = __str__";

    _out.dec();

    //
    // Define the proxy class.
    //
    if(!p->isLocal())
    {
        _out << sp << nl << "_M_" << abs << "Prx = Ice.createTempClass()";
        _out << nl << "class " << name << "Prx(";
        if(bases.empty())
        {
            _out << "Ice.ObjectPrx";
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
            if(fixedOpName == "checkedCast" || fixedOpName == "uncheckedCast")
            {
                fixedOpName.insert(0, "_");
            }
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
            _out << ", _ctx=None):";
            _out.inc();
            _out << nl << "return _M_" << abs << "._op_" << (*oli)->name() << ".invoke(self, (" << inParams;
            if(!inParams.empty() && inParams.find(',') == string::npos)
            {
                _out << ", ";
            }
            _out << "), _ctx)";
            _out.dec();

            if(p->hasMetaData("ami") || (*oli)->hasMetaData("ami"))
            {
                _out << sp << nl << "def " << fixedOpName << "_async(self, _cb";
                if(!inParams.empty())
                {
                    _out << ", " << inParams;
                }
                _out << ", _ctx=None):";
                _out.inc();
                _out << nl << "return _M_" << abs << "._op_" << (*oli)->name() << ".invokeAsync(self, _cb, ("
                     << inParams;
                if(!inParams.empty() && inParams.find(',') == string::npos)
                {
                    _out << ", ";
                }
                _out << "), _ctx)";
                _out.dec();
            }
        }

        _out << sp << nl << "def checkedCast(proxy, facetOrCtx=None, _ctx=None):";
        _out.inc();
        _out << nl << "return _M_" << abs << "Prx.ice_checkedCast(proxy, '" << scoped << "', facetOrCtx, _ctx)";
        _out.dec();
        _out << nl << "checkedCast = staticmethod(checkedCast)";

        _out << sp << nl << "def uncheckedCast(proxy, facet=None):";
        _out.inc();
        _out << nl << "return _M_" << abs << "Prx.ice_uncheckedCast(proxy, facet)";
        _out.dec();
        _out << nl << "uncheckedCast = staticmethod(uncheckedCast)";

        _out.dec();

        _out << sp << nl << "_M_" << type << "Prx = IcePy.defineProxy('" << scoped << "', " << name << "Prx)";
    }

    if(_classHistory.count(scoped) == 0 && p->canBeCyclic())
    {
        //
        // Emit a forward declaration for the class in case a data member refers to this type.
        //
        _out << sp << nl << "_M_" << type << " = IcePy.declareClass('" << scoped << "')";
    }

    DataMemberList members = p->dataMembers();
    _out << sp << nl << "_M_" << type << " = IcePy.defineClass('" << scoped << "', " << name << ", ";
    writeMetaData(p->getMetaData());
    _out << ", " << (p->isAbstract() ? "True" : "False") << ", ";
    if(!base)
    {
        _out << "None";
    }
    else
    {
        _out << "_M_" << getAbsolute(base, "_t_");
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
            _out << "_M_" << getAbsolute(*q, "_t_");
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
    //   ('MemberName', MemberMetaData, MemberType)
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
        writeMetaData((*r)->getMetaData());
        _out << ", ";
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
    _out << nl << name << ".ice_type = _M_" << type;

    //
    // Define each operation. The arguments to the IcePy.Operation constructor are:
    //
    // 'opName', Mode, SendMode, AMD, (MetaData), (InParams), (OutParams), ReturnType, (Exceptions)
    //
    // where InParams and OutParams are tuples of type descriptions, and Exceptions
    // is a tuple of exception type ids.
    //
    if(!p->isLocal())
    {
        if(!ops.empty())
        {
            _out << sp;
        }
        for(OperationList::iterator s = ops.begin(); s != ops.end(); ++s)
        {
            ParamDeclList params = (*s)->parameters();
            ParamDeclList::iterator t;
            int count;

            _out << nl << name << "._op_" << (*s)->name() << " = IcePy.Operation('" << (*s)->name() << "', "
                 << getOperationMode((*s)->mode()) << ", " << getOperationMode((*s)->sendMode()) << ", "
                 << ((p->hasMetaData("amd") || (*s)->hasMetaData("amd")) ? "True" : "False") << ", ";
            writeMetaData((*s)->getMetaData());
            _out << ", (";
            for(t = params.begin(), count = 0; t != params.end(); ++t)
            {
                if(!(*t)->isOutParam())
                {
                    if(count > 0)
                    {
                        _out << ", ";
                    }
                    _out << '(';
                    _out << "'" << fixIdent((*t)->name()) << "', ";
                    writeMetaData((*t)->getMetaData());
                    _out << ", ";
                    writeType((*t)->type());
                    _out << ')';
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
                    _out << '(';
                    _out << "'" << fixIdent((*t)->name()) << "', ";
                    writeMetaData((*t)->getMetaData());
                    _out << ", ";
                    writeType((*t)->type());
                    _out << ')';
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
                _out << "_M_" << getAbsolute(*u, "_t_");
            }
            if(exceptions.size() == 1)
            {
                _out << ',';
            }
            _out << "))";

            string deprecateMetadata;
            if((*s)->findMetaData("deprecate", deprecateMetadata) || p->findMetaData("deprecate", deprecateMetadata))
            {
                string msg;
                string::size_type pos = deprecateMetadata.find(':');
                if(pos != string::npos && pos < deprecateMetadata.size() - 1)
                {
                    msg = deprecateMetadata.substr(pos + 1);
                }
                _out << nl << name << "._op_" << (*s)->name() << ".deprecate(\"" << msg << "\")";
            }
        }
    }

    registerName(name);

    if(!p->isLocal())
    {
        registerName(name + "Prx");
    }

    _out.dec();

    if(_classHistory.count(scoped) == 0)
    {
        _classHistory.insert(scoped); // Avoid redundant declarations.
    }

    return false;
}

bool
Slice::Python::CodeVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string scoped = p->scoped();
    string abs = getAbsolute(p);
    string name = fixIdent(p->name());

    _out << sp << nl << "if not " << getDictLookup(p) << ':';
    _out.inc();
    _out << nl << "_M_" << abs << " = Ice.createTempClass()";
    _out << nl << "class " << name << '(';
    ExceptionPtr base = p->base();
    string baseName;
    if(base)
    {
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
    MemberInfoList allMembers;
    collectExceptionMembers(p, allMembers, false);
    if(!allMembers.empty())
    {
        for(MemberInfoList::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
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
            for(MemberInfoList::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
            {
                if(q->inherited)
                {
                    _out << ", " << q->fixedName;
                }
            }
            _out << ')';
        }
        for(MemberInfoList::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
        {
            if(!q->inherited)
            {
                _out << nl << "self." << q->fixedName << " = " << q->fixedName;;
            }
        }
    }
    _out.dec();

    //
    // ice_name
    //
    _out << sp << nl << "def ice_name(self):";
    _out.inc();
    _out << nl << "return '" << scoped.substr(2) << "'";
    _out.dec();

    //
    // __str__
    //
    _out << sp << nl << "def __str__(self):";
    _out.inc();
    _out << nl << "return IcePy.stringifyException(self)";
    _out.dec();
    _out << sp << nl << "__repr__ = __str__";

    _out.dec();

    //
    // Emit the type information.
    //
    string type = getAbsolute(p, "_t_");
    _out << sp << nl << "_M_" << type << " = IcePy.defineException('" << scoped << "', " << name << ", ";
    writeMetaData(p->getMetaData());
    _out << ", ";
    if(!base)
    {
        _out << "None";
    }
    else
    {
         _out << "_M_" << getAbsolute(base, "_t_");
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
    //   ('MemberName', MemberMetaData, MemberType)
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a constructed type.
    //
    for(dmli = members.begin(); dmli != members.end(); ++dmli)
    {
        if(dmli != members.begin())
        {
            _out << ',' << nl;
        }
        _out << "('" << fixIdent((*dmli)->name()) << "', ";
        writeMetaData((*dmli)->getMetaData());
        _out << ", ";
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
    _out << "))";
    _out << nl << name << ".ice_type = _M_" << type;

    registerName(name);

    _out.dec();

    return false;
}

bool
Slice::Python::CodeVisitor::visitStructStart(const StructPtr& p)
{
    string scoped = p->scoped();
    string abs = getAbsolute(p);
    string name = fixIdent(p->name());
    MemberInfoList memberList;
    MemberInfoList::iterator r;

    {
        DataMemberList members = p->dataMembers();
        for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
        {
            memberList.push_back(MemberInfo());
            memberList.back().fixedName = fixIdent((*q)->name());
            memberList.back().type = (*q)->type();
            memberList.back().metaData = (*q)->getMetaData();
        }
    }

    _out << sp << nl << "if not " << getDictLookup(p) << ':';
    _out.inc();
    _out << nl << "_M_" << abs << " = Ice.createTempClass()";
    _out << nl << "class " << name << "(object):";
    _out.inc();
    _out << nl << "def __init__(self";
    for(r = memberList.begin(); r != memberList.end(); ++r)
    {
        _out << ", " << r->fixedName << '=';
        writeDefaultValue(r->type);
    }
    _out << "):";
    _out.inc();
    for(r = memberList.begin(); r != memberList.end(); ++r)
    {
        _out << nl << "self." << r->fixedName << " = " << r->fixedName;
    }
    _out.dec();

    _out << sp << nl << "def __hash__(self):";
    _out.inc();
    _out << nl << "_h = 0";
    int iter = 0;
    for(r = memberList.begin(); r != memberList.end(); ++r)
    {
        string s = "self." + r->fixedName;
        writeHash(s, r->type, iter);
    }
    _out << nl << "return _h % 0x7fffffff";
    _out.dec();

    _out << sp << nl << "def __eq__(self, other):";
    _out.inc();
    for(r = memberList.begin(); r != memberList.end(); ++r)
    {
        _out << nl << "if not self." << r->fixedName << " == other." << r->fixedName << ':';
        _out.inc();
        _out << nl << "return False";
        _out.dec();
    }
    _out << nl << "return True";
    _out.dec();

    //
    // __str__
    //
    _out << sp << nl << "def __str__(self):";
    _out.inc();
    _out << nl << "return IcePy.stringify(self, _M_" << getAbsolute(p, "_t_") << ")";
    _out.dec();
    _out << sp << nl << "__repr__ = __str__";

    _out.dec();

    //
    // Emit the type information.
    //
    _out << sp << nl << "_M_" << getAbsolute(p, "_t_") << " = IcePy.defineStruct('" << scoped << "', " << name << ", ";
    writeMetaData(p->getMetaData());
    _out << ", (";
    //
    // Data members are represented as a tuple:
    //
    //   ('MemberName', MemberMetaData, MemberType)
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a constructed type.
    //
    if(memberList.size() > 1)
    {
        _out.inc();
        _out << nl;
    }
    for(r = memberList.begin(); r != memberList.end(); ++r)
    {
        if(r != memberList.begin())
        {
            _out << ',' << nl;
        }
        _out << "('" << r->fixedName << "', ";
        writeMetaData(r->metaData);
        _out << ", ";
        writeType(r->type);
        _out << ')';
    }
    if(memberList.size() == 1)
    {
        _out << ',';
    }
    else if(memberList.size() > 1)
    {
        _out.dec();
        _out << nl;
    }
    _out << "))";

    registerName(name);

    _out.dec();

    return false;
}

void
Slice::Python::CodeVisitor::visitSequence(const SequencePtr& p)
{
    //
    // Emit the type information.
    //
    string scoped = p->scoped();
    _out << sp << nl << "if not " << getDictLookup(p, "_t_") << ':';
    _out.inc();
    _out << nl << "_M_" << getAbsolute(p, "_t_") << " = IcePy.defineSequence('" << scoped << "', ";
    writeMetaData(p->getMetaData());
    _out << ", ";
    writeType(p->type());
    _out << ")";
    _out.dec();
}

void
Slice::Python::CodeVisitor::visitDictionary(const DictionaryPtr& p)
{
    //
    // Emit the type information.
    //
    string scoped = p->scoped();
    _out << sp << nl << "if not " << getDictLookup(p, "_t_") << ':';
    _out.inc();
    _out << nl << "_M_" << getAbsolute(p, "_t_") << " = IcePy.defineDictionary('" << scoped << "', ";
    writeMetaData(p->getMetaData());
    _out << ", ";
    writeType(p->keyType());
    _out << ", ";
    writeType(p->valueType());
    _out << ")";
    _out.dec();
}

void
Slice::Python::CodeVisitor::visitEnum(const EnumPtr& p)
{
    string scoped = p->scoped();
    string abs = getAbsolute(p);
    string name = fixIdent(p->name());
    EnumeratorList enums = p->getEnumerators();
    EnumeratorList::iterator q;
    int i;

    _out << sp << nl << "if not " << getDictLookup(p) << ':';
    _out.inc();
    _out << nl << "_M_" << abs << " = Ice.createTempClass()";
    _out << nl << "class " << name << "(object):";
    _out.inc();
    _out << sp << nl << "def __init__(self, val):";
    _out.inc();
    {
        ostringstream assertion;
        assertion << "assert(val >= 0 and val < " << enums.size() << ')';
        _out << nl << assertion.str();
    }
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
        _out << nl << name << '.' << fixedEnum << " = " << name << '(' << idx.str() << ')';
    }

    //
    // Emit the type information.
    //
    _out << sp << nl << "_M_" << getAbsolute(p, "_t_") << " = IcePy.defineEnum('" << scoped << "', " << name
         << ", ";
    writeMetaData(p->getMetaData());
    _out << ", (";
    for(q = enums.begin(); q != enums.end(); ++q)
    {
        if(q != enums.begin())
        {
            _out << ", ";
        }
        string fixedEnum = fixIdent((*q)->name());
        _out << name << '.' << fixedEnum;
    }
    if(enums.size() == 1)
    {
        _out << ',';
    }
    _out << "))";

    registerName(name);

    _out.dec();
}

void
Slice::Python::CodeVisitor::visitConst(const ConstPtr& p)
{
    Slice::TypePtr type = p->type();
    string value = p->value();
    string name = fixIdent(p->name());

    _out << sp << nl << "_M_" << getAbsolute(p) << " = ";

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
            IceUtil::stringToInt64(value, l);
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
                                                   "_{}[]#()<>%:;.?*+-/^&|~!=, '";
            static const set<char> charSet(basicSourceChars.begin(), basicSourceChars.end());

            _out << "\"";                                       // Opening "

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
                        stringstream s;
                        s << "\\";                              // Print as octal if not in basic source character set.
                        s.flags(ios_base::oct);
                        s.width(3);
                        s.fill('0');
                        s << static_cast<unsigned>(uc);
                        _out << s.str();
                    }
                    else
                    {
                        _out << *c;                             // Print normally if in basic source character set.
                    }
                    break;
                }
                }
            }

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
    return "_M_" + getAbsolute(p);
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
                _out << "IcePy._t_LocalObject";
                break;
            }
        }
        return;
    }

    ProxyPtr prx = ProxyPtr::dynamicCast(p);
    if(prx)
    {
        _out << "_M_" << getAbsolute(prx->_class(), "_t_") << "Prx";
        return;
    }

    ContainedPtr cont = ContainedPtr::dynamicCast(p);
    assert(cont);
    _out << "_M_" << getAbsolute(cont, "_t_");
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

void
Slice::Python::CodeVisitor::writeHash(const string& name, const TypePtr& p, int& iter)
{
    SequencePtr seq = SequencePtr::dynamicCast(p);
    if(seq)
    {
        _out << nl << "if " << name << ':';
        _out.inc();
        _out << nl << "for _i" << iter << " in " << name << ':';
        _out.inc();
        ostringstream elem;
        elem << "_i" << iter;
        iter++;
        writeHash(elem.str(), seq->type(), iter);
        _out.dec();
        _out.dec();
        return;
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(p);
    if(dict)
    {
        _out << nl << "if " << name << ':';
        _out.inc();
        _out << nl << "for _i" << iter << " in " << name << ':';
        _out.inc();
        ostringstream key;
        key << "_i" << iter;
        ostringstream value;
        value << name << '[' << key.str() << ']';
        iter++;
        writeHash(key.str(), dict->keyType(), iter);
        writeHash(value.str(), dict->valueType(), iter);
        _out.dec();
        _out.dec();
        return;
    }

    _out << nl << "_h = 5 * _h + __builtin__.hash(" << name << ")";
}

void
Slice::Python::CodeVisitor::writeMetaData(const StringList& meta)
{
    int i = 0;
    _out << '(';
    for(StringList::const_iterator p = meta.begin(); p != meta.end(); ++p)
    {
        if(p->find("python:") == 0)
        {
            if(i > 0)
            {
                _out << ", ";
            }
            _out << "'" << *p << "'";
            ++i;
        }
    }
    if(i == 1)
    {
        _out << ',';
    }
    _out << ')';
}

string
Slice::Python::CodeVisitor::getOperationMode(Slice::Operation::Mode mode)
{
    string result;
    switch(mode)
    {
    case Operation::Normal:
        result = "Ice.OperationMode.Normal";
        break;
    case Operation::Nonmutating:
        result = "Ice.OperationMode.Nonmutating";
        break;
    case Operation::Idempotent:
        result = "Ice.OperationMode.Idempotent";
        break;
    }
    return result;
}

void
Slice::Python::CodeVisitor::collectClassMembers(const ClassDefPtr& p, MemberInfoList& allMembers, bool inherited)
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
        m.type = (*q)->type();
        m.inherited = inherited;
        m.metaData = (*q)->getMetaData();
        allMembers.push_back(m);
    }
}

void
Slice::Python::CodeVisitor::collectExceptionMembers(const ExceptionPtr& p, MemberInfoList& allMembers, bool inherited)
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
        m.type = (*q)->type();
        m.inherited = inherited;
        m.metaData = (*q)->getMetaData();
        allMembers.push_back(m);
    }
}

static string
normalizePath(const string& path)
{
    string result = path;
    replace(result.begin(), result.end(), '\\', '/');
    string::size_type pos;
    while((pos = result.find("//")) != string::npos)
    {
        result.replace(pos, 2, "/");
    }
    return result;
}

static string
changeInclude(const string& inc, const vector<string>& includePaths)
{
    string orig = normalizePath(inc);
    string curr = orig; // The current shortest pathname.

    //
    // Compare the pathname of the included file against each of the include directories.
    // If any of the include directories match the leading part of the included file,
    // then select the include directory whose removal results in the shortest pathname.
    //
    for(vector<string>::const_iterator p = includePaths.begin(); p != includePaths.end(); ++p)
    {
        string includePath = *p;

        if(orig.compare(0, p->size(), *p) == 0)
        {
            string s = orig.substr(p->size());
            if(s.size() < curr.size())
            {
                curr = s;
            }
        }
    }

    string::size_type pos = curr.rfind('.');
    if(pos != string::npos)
    {
        curr.erase(pos);
    }

    return curr;
}

void
Slice::Python::generate(const UnitPtr& un, bool all, bool checksum, const vector<string>& includePaths, Output& out)
{
    Slice::Python::MetaDataVisitor visitor;
    un->visit(&visitor, false);

    out << nl << "import Ice, IcePy, __builtin__";

    if(!all)
    {
        vector<string> paths = includePaths;
        for(vector<string>::iterator p = paths.begin(); p != paths.end(); ++p)
        {
            if(p->size() && (*p)[p->size() - 1] != '/')
            {
                *p += '/';
            }
            *p = normalizePath(*p);
        }

        StringList includes = un->includeFiles();
        for(StringList::const_iterator q = includes.begin(); q != includes.end(); ++q)
        {
            string file = changeInclude(*q, paths);
            replace(file.begin(), file.end(), '/', '_');
            out << nl << "import " << file << "_ice";
        }
    }

    set<string> moduleHistory;

    ModuleVisitor moduleVisitor(out, moduleHistory);
    un->visit(&moduleVisitor, true);

    CodeVisitor codeVisitor(out, moduleHistory);
    un->visit(&codeVisitor, false);

    if(checksum)
    {
        ChecksumMap checksums = createChecksums(un);
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
Slice::Python::splitString(const string& str, vector<string>& args, const string& delim)
{
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

string
Slice::Python::getPackageMetadata(const ContainedPtr& cont)
{
    string package;

    DefinitionContextPtr dc = cont->definitionContext();
    if(dc)
    {
        static const string prefix = "python:package:";
        string metadata = dc->findMetaData(prefix);
        if(!metadata.empty())
        {
            package = metadata.substr(prefix.size());
        }
    }

    return package;
}

string
Slice::Python::getAbsolute(const ContainedPtr& cont, const string& suffix)
{
    string scope = scopedToName(cont->scope());

    string package = getPackageMetadata(cont);
    if(!package.empty())
    {
        if(!scope.empty())
        {
            scope = package + "." + scope;
        }
        else
        {
            scope = package + ".";
        }
    }

    if(suffix.empty())
    {
        return scope + fixIdent(cont->name());
    }
    else
    {
        return scope + suffix + fixIdent(cont->name());
    }
}

void
Slice::Python::printHeader(IceUtil::Output& out)
{
    static const char* header =
"# **********************************************************************\n"
"#\n"
"# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.\n"
"#\n"
"# This copy of Ice is licensed to you under the terms described in the\n"
"# ICE_LICENSE file included in this distribution.\n"
"#\n"
"# **********************************************************************\n"
        ;

    out << header;
    out << "\n# Ice version " << ICE_STRING_VERSION;
}

bool
Slice::Python::MetaDataVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!ModulePtr::dynamicCast(p->container()))
    {
        //
        // We only need to validate global metadata for top-level modules.
        //
        validateGlobal(p->definitionContext());
    }
    reject(p);
    return true;
}

void
Slice::Python::MetaDataVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    reject(p);
}

bool
Slice::Python::MetaDataVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    reject(p);
    return true;
}

bool
Slice::Python::MetaDataVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    reject(p);
    return true;
}

bool
Slice::Python::MetaDataVisitor::visitStructStart(const StructPtr& p)
{
    reject(p);
    return true;
}

void
Slice::Python::MetaDataVisitor::visitOperation(const OperationPtr& p)
{
    DefinitionContextPtr dc = p->definitionContext();
    assert(dc);

    TypePtr ret = p->returnType();
    if(ret)
    {
        validateSequence(dc, p->line(), ret, p->getMetaData());
    }

    ParamDeclList params = p->parameters();
    for(ParamDeclList::iterator q = params.begin(); q != params.end(); ++q)
    {
        validateSequence(dc, (*q)->line(), (*q)->type(), (*q)->getMetaData());
    }
}

void
Slice::Python::MetaDataVisitor::visitDataMember(const DataMemberPtr& p)
{
    validateSequence(p->definitionContext(), p->line(), p->type(), p->getMetaData());
}

void
Slice::Python::MetaDataVisitor::visitSequence(const SequencePtr& p)
{
    validateSequence(p->definitionContext(), p->line(), p, p->getMetaData());
}

void
Slice::Python::MetaDataVisitor::visitDictionary(const DictionaryPtr& p)
{
    reject(p);
}

void
Slice::Python::MetaDataVisitor::visitEnum(const EnumPtr& p)
{
    reject(p);
}

void
Slice::Python::MetaDataVisitor::visitConst(const ConstPtr& p)
{
    reject(p);
}

void
Slice::Python::MetaDataVisitor::validateGlobal(const DefinitionContextPtr& dc)
{
    StringList globalMetaData = dc->getMetaData();

    static const string prefix = "python:";

    for(StringList::const_iterator p = globalMetaData.begin(); p != globalMetaData.end(); ++p)
    {
        string s = *p;
        if(_history.count(s) == 0)
        {
            if(s.find(prefix) == 0)
            {
                static const string packagePrefix = "python:package:";
                if(s.find(packagePrefix) != 0 || s.size() == packagePrefix.size())
                {
                    cout << dc->filename() << ": warning: ignoring invalid global metadata `" << s << "'" << endl;
                }
            }
            _history.insert(s);
        }
    }
}

void
Slice::Python::MetaDataVisitor::validateSequence(const DefinitionContextPtr& dc, const string& line,
                                                 const TypePtr& type, const StringList& meta)
{
    static const string prefix = "python:";

    for(StringList::const_iterator p = meta.begin(); p != meta.end(); ++p)
    {
        string s = *p;
        if(s.find(prefix) == 0)
        {
            string::size_type pos = s.find(':', prefix.size());
            if(pos != string::npos && s.substr(prefix.size(), pos - prefix.size()) == "seq")
            {
                static const string seqPrefix = "python:seq:";
                string arg = s.substr(seqPrefix.size(), pos - seqPrefix.size());
                if(SequencePtr::dynamicCast(type))
                {
                    if(arg == "tuple" || arg == "list" || arg == "default")
                    {
                        continue;
                    }
                }
            }
            cout << dc->filename() << ":" << line << ": warning: ignoring metadata `" << s << "'" << endl;
        }
    }
}

void
Slice::Python::MetaDataVisitor::reject(const ContainedPtr& cont)
{
    StringList localMetaData = cont->getMetaData();

    static const string prefix = "python:";

    for(StringList::const_iterator p = localMetaData.begin(); p != localMetaData.end(); ++p)
    {
        if(p->find(prefix) == 0)
        {
            DefinitionContextPtr dc = cont->definitionContext();
            assert(dc);
            cout << dc->filename() << ":" << cont->line() << ": warning: ignoring metadata `" << *p << "'" << endl;
        }
    }
}
