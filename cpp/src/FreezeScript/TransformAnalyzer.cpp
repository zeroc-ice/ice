// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <FreezeScript/TransformAnalyzer.h>
#include <FreezeScript/Util.h>
#include <IceUtil/OutputUtil.h>
#include <map>

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

namespace FreezeScript
{

//
// AnalyzeTransformVisitor visits the old definitions and compares them with
// the new definitions.
//
class AnalyzeTransformVisitor : public ParserVisitor
{
public:

    AnalyzeTransformVisitor(XMLOutput&, const UnitPtr&, bool, vector<string>&, vector<string>&);

    void addDatabase(const string&, const TypePtr&, const TypePtr&, const TypePtr&, const TypePtr&);

    virtual bool visitClassDefStart(const ClassDefPtr&);
    virtual bool visitStructStart(const StructPtr&);
    virtual void visitSequence(const SequencePtr&);
    virtual void visitDictionary(const DictionaryPtr&);
    virtual void visitEnum(const EnumPtr&);

private:

    void compareMembers(const DataMemberList&, const DataMemberList&);
    void compareTypes(const string&, const TypePtr&, const TypePtr&);
    void typeChange(const string&, const TypePtr&, const TypePtr&);
    bool checkClasses(const ClassDeclPtr&, const ClassDeclPtr&);

    XMLOutput& _out;
    UnitPtr _newUnit;
    bool _ignoreTypeChanges;
    vector<string>& _missingTypes;
    vector<string>& _errors;
};

//
// InitVisitor visits the new definitions to find any that are not present
// in the old definitions, and generates init elements for them.
//
class AnalyzeInitVisitor : public ParserVisitor
{
public:

    AnalyzeInitVisitor(XMLOutput&, const UnitPtr&);

    virtual bool visitClassDefStart(const ClassDefPtr&);
    virtual bool visitStructStart(const StructPtr&);
    virtual void visitSequence(const SequencePtr&);
    virtual void visitDictionary(const DictionaryPtr&);
    virtual void visitEnum(const EnumPtr&);

private:

    void typeChange(const TypePtr&, const string&, const string&);

    XMLOutput& _out;
    UnitPtr _oldUnit;
};

}

////////////////////////////////////
// AnalyzeTransformVisitor
////////////////////////////////////

FreezeScript::AnalyzeTransformVisitor::AnalyzeTransformVisitor(XMLOutput& out, const UnitPtr& newUnit,
                                                               bool ignoreTypeChanges, vector<string>& missingTypes,
                                                               vector<string>& errors) :
    _out(out), _newUnit(newUnit), _ignoreTypeChanges(ignoreTypeChanges), _missingTypes(missingTypes), _errors(errors)
{
}

void
FreezeScript::AnalyzeTransformVisitor::addDatabase(const string& name, const TypePtr& oldKey, const TypePtr& newKey,
                                                   const TypePtr& oldValue, const TypePtr& newValue)
{
    _out << "\n";
    _out << se("database");

    if(!name.empty())
    {
        _out << attr("name", name);
    }

    string oldKeyName = oldKey ? typeToString(oldKey) : string("UNKNOWN");
    string newKeyName = newKey ? typeToString(newKey) : string("UNKNOWN");
    if(oldKeyName == newKeyName)
    {
        _out << attr("key", oldKeyName);
    }
    else
    {
        _out << attr("key", oldKeyName + "," + newKeyName);
    }

    string oldValueName = oldValue ? typeToString(oldValue) : string("UNKNOWN");
    string newValueName = newValue ? typeToString(newValue) : string("UNKNOWN");
    if(oldValueName == newValueName)
    {
        _out << attr("value", oldValueName);
    }
    else
    {
        _out << attr("value", oldValueName + "," + newValueName);
    }

    _out << se("record");
    if(oldKey && newKey)
    {
        compareTypes("database key", oldKey, newKey);
    }
    if(oldValue && newValue)
    {
        compareTypes("database value", oldValue, newValue);
    }
    _out << ee;

    _out << ee;
}

bool
FreezeScript::AnalyzeTransformVisitor::visitClassDefStart(const ClassDefPtr& v)
{
    if(v->isInterface() || v->isLocal())
    {
        return false;
    }

    string scoped = v->scoped();
    if(ignoreType(scoped))
    {
        return false;
    }

    TypeList l = _newUnit->lookupTypeNoBuiltin(scoped, false);
    if(l.empty())
    {
        _missingTypes.push_back(scoped);
        return false;
    }

    //
    // Allow transforming from class to struct.
    //
    ClassDeclPtr decl = ClassDeclPtr::dynamicCast(l.front());
    StructPtr newStruct = StructPtr::dynamicCast(l.front());
    if(!newStruct && (!decl || decl->isInterface()))
    {
        if(!_ignoreTypeChanges)
        {
            typeChange(scoped, v->declaration(), l.front());
        }
        return false;
    }

    ClassDefPtr newClass;
    if(decl)
    {
        newClass = decl->definition();
        if(!newClass)
        {
            _missingTypes.push_back(scoped);
            return false;
        }
    }

    DataMemberList oldMembers, newMembers;

    if(newClass)
    {
        oldMembers = v->dataMembers();
        newMembers = newClass->dataMembers();
    }
    else
    {
        oldMembers = v->allDataMembers();
        newMembers = newStruct->dataMembers();
    }

    _out.newline();
    _out.newline();
    if(newStruct)
    {
        _out << "<!-- struct " << scoped << " -->";
    }
    else
    {
        _out << "<!-- class " << scoped << " -->";
    }
    _out << se("transform") << attr("type", scoped);

    compareMembers(oldMembers, newMembers);

    _out << ee;

    return false;
}

bool
FreezeScript::AnalyzeTransformVisitor::visitStructStart(const StructPtr& v)
{
    if(v->isLocal())
    {
        return false;
    }

    string scoped = v->scoped();
    if(ignoreType(scoped))
    {
        return false;
    }

    TypeList l = _newUnit->lookupTypeNoBuiltin(scoped, false);
    if(l.empty())
    {
        _missingTypes.push_back(scoped);
        return false;
    }

    //
    // Allow transforming from struct to class.
    //
    StructPtr newStruct = StructPtr::dynamicCast(l.front());
    ClassDeclPtr decl = ClassDeclPtr::dynamicCast(l.front());
    ClassDefPtr newClass;
    if(decl)
    {
        newClass = decl->definition();
        if(!newClass)
        {
            _missingTypes.push_back(scoped);
            return false;
        }
    }
    else if(!newStruct)
    {
        if(!_ignoreTypeChanges)
        {
            typeChange(scoped, v, l.front());
        }
        return false;
    }

    _out.newline();
    _out.newline();
    if(newClass)
    {
        _out << "<!-- class " << scoped << " -->";
    }
    else
    {
        _out << "<!-- struct " << scoped << " -->";
    }
    _out << se("transform") << attr("type", scoped);

    DataMemberList oldMembers, newMembers;

    if(newClass)
    {
        oldMembers = v->dataMembers();
        newMembers = newClass->allDataMembers();
    }
    else
    {
        oldMembers = v->dataMembers();
        newMembers = newStruct->dataMembers();
    }

    compareMembers(oldMembers, newMembers);

    _out << ee;

    return false;
}

void
FreezeScript::AnalyzeTransformVisitor::visitSequence(const SequencePtr& v)
{
    if(v->isLocal())
    {
        return;
    }

    string scoped = v->scoped();
    if(ignoreType(scoped))
    {
        return;
    }

    TypeList l = _newUnit->lookupTypeNoBuiltin(scoped, false);
    if(l.empty())
    {
        _missingTypes.push_back(scoped);
        return;
    }

    SequencePtr newSeq = SequencePtr::dynamicCast(l.front());
    if(!newSeq)
    {
        if(!_ignoreTypeChanges)
        {
            typeChange(scoped, v, l.front());
        }
        return;
    }

    _out.newline();
    _out.newline();
    _out << "<!-- sequence " << scoped << " -->";
    _out << se("transform") << attr("type", scoped);

    compareTypes(scoped + " sequence type", v->type(), newSeq->type());

    _out << ee;
}

void
FreezeScript::AnalyzeTransformVisitor::visitDictionary(const DictionaryPtr& v)
{
    if(v->isLocal())
    {
        return;
    }

    string scoped = v->scoped();
    if(ignoreType(scoped))
    {
        return;
    }

    TypeList l = _newUnit->lookupTypeNoBuiltin(scoped, false);
    if(l.empty())
    {
        _missingTypes.push_back(scoped);
        return;
    }

    DictionaryPtr newDict = DictionaryPtr::dynamicCast(l.front());
    if(!newDict)
    {
        if(!_ignoreTypeChanges)
        {
            typeChange(scoped, v, l.front());
        }
        return;
    }

    _out.newline();
    _out.newline();
    _out << "<!-- dictionary " << scoped << " -->";
    _out << se("transform") << attr("type", scoped);

    compareTypes(scoped + " key type", v->keyType(), newDict->keyType());
    compareTypes(scoped + " value type", v->valueType(), newDict->valueType());

    _out << ee;
}

void
FreezeScript::AnalyzeTransformVisitor::visitEnum(const EnumPtr& v)
{
    if(v->isLocal())
    {
        return;
    }

    string scoped = v->scoped();
    if(ignoreType(scoped))
    {
        return;
    }

    TypeList l = _newUnit->lookupTypeNoBuiltin(scoped, false);
    if(l.empty())
    {
        _missingTypes.push_back(scoped);
        return;
    }

    EnumPtr newEnum = EnumPtr::dynamicCast(l.front());
    if(!newEnum)
    {
        if(!_ignoreTypeChanges)
        {
            typeChange(scoped, v, l.front());
        }
        return;
    }

    map<string, int> m;
    {
        Slice::EnumeratorList enumerators = newEnum->getEnumerators();
        int i = 0;
        for(Slice::EnumeratorList::iterator p = enumerators.begin(); p != enumerators.end(); ++p, ++i)
        {
            m.insert(map<string, int>::value_type((*p)->name(), i));
        }
    }

    _out.newline();
    _out.newline();
    _out << "<!-- enum " << scoped << " -->";
    _out << se("transform") << attr("type", scoped);

    Slice::EnumeratorList enumerators = v->getEnumerators();
    int i = 0;
    for(Slice::EnumeratorList::iterator p = enumerators.begin(); p != enumerators.end(); ++p, ++i)
    {
        map<string, int>::const_iterator q = m.find((*p)->name());
        if(q == m.end())
        {
            _out.newline();
            _out << "<!-- NOTICE: enumerator `" << (*p)->name() << "' has been removed -->";
        }
        else if(q->second != i)
        {
            _out.newline();
            _out << "<!-- NOTICE: enumerator `" << (*p)->name() << "' has changed position -->";
        }
    }

    _out << ee;
}

void
FreezeScript::AnalyzeTransformVisitor::compareMembers(const DataMemberList& oldMembers,
                                                      const DataMemberList& newMembers)
{
    map<string, DataMemberPtr> oldMap, newMap;

    for(DataMemberList::const_iterator p = oldMembers.begin(); p != oldMembers.end(); ++p)
    {
        oldMap.insert(pair<string, DataMemberPtr>((*p)->name(), *p));
    }

    for(DataMemberList::const_iterator p = newMembers.begin(); p != newMembers.end(); ++p)
    {
        newMap.insert(pair<string, DataMemberPtr>((*p)->name(), *p));
    }

    for(DataMemberList::const_iterator p = oldMembers.begin(); p != oldMembers.end(); ++p)
    {
        string name = (*p)->name();
        map<string, DataMemberPtr>::iterator q = newMap.find(name);
        if(q == newMap.end())
        {
            _out.newline();
            _out << "<!-- NOTICE: " << name << " has been removed -->";
        }
        else
        {
            TypePtr oldType = (*p)->type();
            TypePtr newType = q->second->type();
            compareTypes(name, oldType, newType);

            //
            // Remove this entry from the map to indicate that we've
            // already seen it.
            //
            newMap.erase(q);
        }
    }

    //
    // Iterate over the remaining entries in newMap. These represent
    // members that were added in the new definition.
    //
    for(map<string, DataMemberPtr>::iterator q = newMap.begin(); q != newMap.end(); ++q)
    {
        _out.newline();
        _out << "<!-- NOTICE: " << q->first << " has been added -->";
    }
}

void
FreezeScript::AnalyzeTransformVisitor::compareTypes(const string& desc, const TypePtr& oldType, const TypePtr& newType)
{
    assert(!oldType->isLocal());
    if(newType->isLocal())
    {
        ostringstream ostr;
        ostr << desc << " has changed to a local type";
        _errors.push_back(ostr.str());
        return;
    }

    BuiltinPtr b = BuiltinPtr::dynamicCast(oldType);
    if(b)
    {
        BuiltinPtr newb = BuiltinPtr::dynamicCast(newType);
        switch(b->kind())
        {
        case Builtin::KindByte:
        {
            if(newb)
            {
                switch(newb->kind())
                {
                case Builtin::KindByte:
                case Builtin::KindShort:
                case Builtin::KindInt:
                case Builtin::KindLong:
                case Builtin::KindString:
                {
                    return;
                }
                case Builtin::KindBool:
                case Builtin::KindFloat:
                case Builtin::KindDouble:
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindLocalObject:
                {
                    break;
                }
                }
            }

            break;
        }
        case Builtin::KindBool:
        {
            if(newb && (newb->kind() == Builtin::KindBool || newb->kind() == Builtin::KindString))
            {
                return;
            }

            break;
        }
        case Builtin::KindShort:
        {
            if(newb)
            {
                switch(newb->kind())
                {
                case Builtin::KindByte:
                case Builtin::KindShort:
                case Builtin::KindInt:
                case Builtin::KindLong:
                case Builtin::KindString:
                {
                    return;
                }
                case Builtin::KindBool:
                case Builtin::KindFloat:
                case Builtin::KindDouble:
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindLocalObject:
                {
                    break;
                }
                }
            }

            break;
        }
        case Builtin::KindInt:
        {
            if(newb)
            {
                switch(newb->kind())
                {
                case Builtin::KindByte:
                case Builtin::KindShort:
                case Builtin::KindInt:
                case Builtin::KindLong:
                case Builtin::KindString:
                {
                    return;
                }
                case Builtin::KindBool:
                case Builtin::KindFloat:
                case Builtin::KindDouble:
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindLocalObject:
                {
                    break;
                }
                }
            }

            break;
        }
        case Builtin::KindLong:
        {
            if(newb)
            {
                switch(newb->kind())
                {
                case Builtin::KindByte:
                case Builtin::KindShort:
                case Builtin::KindInt:
                case Builtin::KindLong:
                case Builtin::KindString:
                {
                    return;
                }
                case Builtin::KindBool:
                case Builtin::KindFloat:
                case Builtin::KindDouble:
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindLocalObject:
                {
                    break;
                }
                }
            }

            break;
        }
        case Builtin::KindFloat:
        {
            if(newb)
            {
                switch(newb->kind())
                {
                case Builtin::KindFloat:
                case Builtin::KindDouble:
                case Builtin::KindString:
                {
                    return;
                }
                case Builtin::KindByte:
                case Builtin::KindShort:
                case Builtin::KindInt:
                case Builtin::KindLong:
                case Builtin::KindBool:
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindLocalObject:
                {
                    break;
                }
                }
            }

            break;
        }
        case Builtin::KindDouble:
        {
            if(newb)
            {
                switch(newb->kind())
                {
                case Builtin::KindFloat:
                case Builtin::KindDouble:
                case Builtin::KindString:
                {
                    return;
                }
                case Builtin::KindByte:
                case Builtin::KindShort:
                case Builtin::KindInt:
                case Builtin::KindLong:
                case Builtin::KindBool:
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindLocalObject:
                {
                    break;
                }
                }
            }

            break;
        }
        case Builtin::KindString:
        {
            if(newb)
            {
                switch(newb->kind())
                {
                case Builtin::KindByte:
                case Builtin::KindBool:
                case Builtin::KindShort:
                case Builtin::KindInt:
                case Builtin::KindLong:
                case Builtin::KindFloat:
                case Builtin::KindDouble:
                case Builtin::KindString:
                case Builtin::KindObjectProxy:
                {
                    return;
                }
                case Builtin::KindObject:
                case Builtin::KindLocalObject:
                {
                    break;
                }
                }

                break;
            }

            if(EnumPtr::dynamicCast(newType))
            {
                return;
            }

            if(ProxyPtr::dynamicCast(newType))
            {
                return;
            }

            break;
        }
        case Builtin::KindObject:
        {
            //
            // Allow change from Object to class. Validation has to
            // be done during transformation, when the actual type of
            // an instance can be compared for compatibility with the
            // new type.
            //
            ClassDeclPtr cl = ClassDeclPtr::dynamicCast(newType);
            if(cl || (newb && newb->kind() == Builtin::KindObject))
            {
                return;
            }

            break;
        }
        case Builtin::KindObjectProxy:
        {
            ProxyPtr p = ProxyPtr::dynamicCast(newType);
            if(p || (newb && newb->kind() == Builtin::KindObjectProxy) || (newb && newb->kind() == Builtin::KindString))
            {
                return;
            }

            break;
        }
        case Builtin::KindLocalObject:
        {
            assert(false);
            break;
        }
        }

        typeChange(desc, oldType, newType);
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(oldType);
    if(cl)
    {
        if(!cl->definition())
        {
            _errors.push_back("class " + cl->scoped() + " declared but not defined");
            return;
        }

        //
        // Allow target type of Object.
        //
        BuiltinPtr newb = BuiltinPtr::dynamicCast(newType);
        if(newb && newb->kind() == Builtin::KindObject)
        {
            return;
        }

        //
        // Allow target type of struct.
        //
        if(StructPtr::dynamicCast(newType))
        {
            return;
        }

        ClassDeclPtr newcl = ClassDeclPtr::dynamicCast(newType);
        if(newcl)
        {
            if(!newcl->definition())
            {
                _errors.push_back("class " + newcl->scoped() + " declared but not defined");
                return;
            }

            if(checkClasses(cl, newcl))
            {
                return;
            }
        }

        typeChange(desc, oldType, newType);
        return;
    }

    StructPtr s = StructPtr::dynamicCast(oldType);
    if(s)
    {
        StructPtr news = StructPtr::dynamicCast(newType);
        if(news && s->scoped() == news->scoped())
        {
            return;
        }

        //
        // Allow target type of class.
        //
        if(ClassDeclPtr::dynamicCast(newType))
        {
            return;
        }

        typeChange(desc, oldType, newType);
        return;
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(oldType);
    if(proxy)
    {
        //
        // Allow target type of Object* and string.
        //
        BuiltinPtr newb = BuiltinPtr::dynamicCast(newType);
        if(newb && (newb->kind() == Builtin::KindObjectProxy || newb->kind() == Builtin::KindString))
        {
            return;
        }

        ProxyPtr newProxy = ProxyPtr::dynamicCast(newType);
        if(newProxy && checkClasses(proxy->_class(), newProxy->_class()))
        {
            return;
        }

        typeChange(desc, oldType, newType);
        return;
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(oldType);
    if(dict)
    {
        DictionaryPtr newDict = DictionaryPtr::dynamicCast(newType);
        if(newDict && dict->scoped() == newDict->scoped())
        {
            return;
        }

        typeChange(desc, oldType, newType);
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(oldType);
    if(seq)
    {
        SequencePtr newSeq = SequencePtr::dynamicCast(newType);
        if(newSeq && seq->scoped() == newSeq->scoped())
        {
            return;
        }

        typeChange(desc, oldType, newType);
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(oldType);
    if(en)
    {
        EnumPtr newen = EnumPtr::dynamicCast(newType);
        BuiltinPtr newb = BuiltinPtr::dynamicCast(newType);
        if((newen && en->scoped() == newen->scoped()) || (newb && newb->kind() == Builtin::KindString))
        {
            return;
        }

        typeChange(desc, oldType, newType);
        return;
    }

    assert(false);
}

void
FreezeScript::AnalyzeTransformVisitor::typeChange(const string& desc, const TypePtr& t1, const TypePtr& t2)
{
    BuiltinPtr b1 = BuiltinPtr::dynamicCast(t1);
    BuiltinPtr b2 = BuiltinPtr::dynamicCast(t2);
    ContainedPtr c1 = ContainedPtr::dynamicCast(t1);
    ContainedPtr c2 = ContainedPtr::dynamicCast(t2);
    ProxyPtr p1 = ProxyPtr::dynamicCast(t1);
    ProxyPtr p2 = ProxyPtr::dynamicCast(t2);

    if(_ignoreTypeChanges)
    {
        _out.newline();
        _out << "<!-- NOTICE: " << desc << " has changed from ";
        if(b1)
        {
            _out << b1->kindAsString();
        }
        else if(p1)
        {
            _out << p1->_class()->scoped() << '*';
        }
        else
        {
            assert(c1);
            _out << c1->kindOf() << ' ' << c1->scoped();
        }
        _out << " to ";
        if(b2)
        {
            _out << b2->kindAsString();
        }
        else if(p2)
        {
            _out << p2->_class()->scoped() << '*';
        }
        else
        {
            assert(c2);
            _out << c2->kindOf() << ' ' << c2->scoped();
        }
        _out << " -->";
    }
    else
    {
        ostringstream ostr;
        ostr << "unsupported type change in " << desc << " from ";
        if(b1)
        {
            ostr << b1->kindAsString();
        }
        else if(p1)
        {
            ostr << p1->_class()->scoped() << '*';
        }
        else
        {
            assert(c1);
            ostr << c1->kindOf() << ' ' << c1->scoped();
        }
        ostr << " to ";
        if(b2)
        {
            ostr << b2->kindAsString();
        }
        else if(p2)
        {
            ostr << p2->_class()->scoped() << '*';
        }
        else
        {
            assert(c2);
            ostr << c2->kindOf() << ' ' << c2->scoped();
        }
        _errors.push_back(ostr.str());
    }
}

bool
FreezeScript::AnalyzeTransformVisitor::checkClasses(const ClassDeclPtr& from, const ClassDeclPtr& to)
{
    string fromScoped = from->scoped();
    string toScoped = to->scoped();

    if(fromScoped == toScoped)
    {
        return true;
    }

    //
    // The types don't match, so check them for compatibility. Specifically,
    // look up the old type id in the new Slice and see if it has the target
    // type as a base class.
    //
    TypeList l = to->unit()->lookupTypeNoBuiltin(from->scoped(), false);
    if(!l.empty())
    {
        ClassDeclPtr decl = ClassDeclPtr::dynamicCast(l.front());
        if(decl)
        {
            ClassDefPtr def = decl->definition();
            if(def)
            {
                ClassList bases = def->allBases();
                for(ClassList::iterator p = bases.begin(); p != bases.end(); ++p)
                {
                    if((*p)->scoped() == toScoped)
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

////////////////////////////////////
// InitVisitor
////////////////////////////////////

FreezeScript::AnalyzeInitVisitor::AnalyzeInitVisitor(XMLOutput& out, const UnitPtr& oldUnit) :
    _out(out), _oldUnit(oldUnit)
{
}

bool
FreezeScript::AnalyzeInitVisitor::visitClassDefStart(const ClassDefPtr& v)
{
    if(v->isInterface() || v->isLocal())
    {
        return false;
    }

    string scoped = v->scoped();
    TypeList l = _oldUnit->lookupTypeNoBuiltin(scoped, false);
    if(!l.empty())
    {
        ClassDeclPtr decl = ClassDeclPtr::dynamicCast(l.front());
        if(!decl || decl->isInterface())
        {
            typeChange(l.front(), scoped, "class");
        }
        else
        {
            return false;
        }
    }

    _out.newline();
    _out.newline();
    _out << "<!-- class " << scoped << " -->";
    _out << se("init") << attr("type", scoped);
    _out << ee;

    return false;
}

bool
FreezeScript::AnalyzeInitVisitor::visitStructStart(const StructPtr& v)
{
    if(v->isLocal())
    {
        return false;
    }

    string scoped = v->scoped();
    TypeList l = _oldUnit->lookupTypeNoBuiltin(scoped, false);
    if(!l.empty())
    {
        StructPtr s = StructPtr::dynamicCast(l.front());
        if(!s)
        {
            typeChange(l.front(), scoped, "struct");
        }
        else
        {
            return false;
        }
    }

    _out.newline();
    _out.newline();
    _out << "<!-- struct " << scoped << " -->";
    _out << se("init") << attr("type", scoped);
    _out << ee;

    return false;
}

void
FreezeScript::AnalyzeInitVisitor::visitSequence(const SequencePtr& v)
{
    if(v->isLocal())
    {
        return;
    }

    string scoped = v->scoped();
    TypeList l = _oldUnit->lookupTypeNoBuiltin(scoped, false);
    if(!l.empty())
    {
        SequencePtr s = SequencePtr::dynamicCast(l.front());
        if(!s)
        {
            typeChange(l.front(), scoped, "sequence");
        }
        else
        {
            return;
        }
    }

    _out.newline();
    _out.newline();
    _out << "<!-- sequence " << scoped << " -->";
    _out << se("init") << attr("type", scoped);
    _out << ee;
}

void
FreezeScript::AnalyzeInitVisitor::visitDictionary(const DictionaryPtr& v)
{
    if(v->isLocal())
    {
        return;
    }

    string scoped = v->scoped();
    TypeList l = _oldUnit->lookupTypeNoBuiltin(scoped, false);
    if(!l.empty())
    {
        DictionaryPtr d = DictionaryPtr::dynamicCast(l.front());
        if(!d)
        {
            typeChange(l.front(), scoped, "dictionary");
        }
        else
        {
            return;
        }
    }

    _out.newline();
    _out.newline();
    _out << "<!-- dictionary " << scoped << " -->";
    _out << se("init") << attr("type", scoped);
    _out << ee;
}

void
FreezeScript::AnalyzeInitVisitor::visitEnum(const EnumPtr& v)
{
    if(v->isLocal())
    {
        return;
    }

    string scoped = v->scoped();
    TypeList l = _oldUnit->lookupTypeNoBuiltin(scoped, false);
    if(!l.empty())
    {
        EnumPtr e = EnumPtr::dynamicCast(l.front());
        if(!e)
        {
            typeChange(l.front(), scoped, "enum");
        }
        else
        {
            return;
        }
    }

    _out.newline();
    _out.newline();
    _out << "<!-- enum " << scoped << " -->";
    _out << se("init") << attr("type", scoped);
    _out << ee;
}


void
FreezeScript::AnalyzeInitVisitor::typeChange(const TypePtr& t, const string& scoped, const string& kind)
{
    BuiltinPtr b = BuiltinPtr::dynamicCast(t);
    ContainedPtr c = ContainedPtr::dynamicCast(t);
    ProxyPtr p = ProxyPtr::dynamicCast(t);

    _out.newline();
    _out.newline();
    _out << "<!-- NOTICE: " << scoped << " has changed from ";
    if(b)
    {
        _out << b->kindAsString();
    }
    else if(p)
    {
        _out << "proxy";
    }
    else
    {
        assert(c);
        _out << c->kindOf();
    }
    _out << " to " << kind << " -->";
}

FreezeScript::TransformAnalyzer::TransformAnalyzer(const UnitPtr& oldUnit, const UnitPtr& newUnit,
                                                   bool ignoreTypeChanges, ostream& os, vector<string>& missingTypes,
                                                   vector<string>& errors) :
    _old(oldUnit), _new(newUnit), _out(os),
    _visitor(new AnalyzeTransformVisitor(_out, newUnit, ignoreTypeChanges, missingTypes, errors))
{
    _out << se("transformdb");
}

FreezeScript::TransformAnalyzer::~TransformAnalyzer()
{
    delete _visitor;
}

void
FreezeScript::TransformAnalyzer::addDatabase(const string& name, const TypePtr& oldKey, const TypePtr& newKey,
                                             const TypePtr& oldValue, const TypePtr& newValue)
{
    _visitor->addDatabase(name, oldKey, newKey, oldValue, newValue);
}

void
FreezeScript::TransformAnalyzer::finish()
{
    _old->visit(_visitor, false);

    AnalyzeInitVisitor initVisitor(_out, _old);
    _new->visit(&initVisitor, false);

    _out << ee;
    _out << '\n';
}
