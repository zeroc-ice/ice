// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Transform/Analyzer.h>
#include <IceUtil/OutputUtil.h>
#include <map>

using namespace std;
using namespace Slice;
using namespace IceUtil;

namespace Transform
{

string typeToString(const TypePtr&);

//
// TransformVisitor visits the old definitions and compares them with
// the new definitions.
//
class TransformVisitor : public ParserVisitor
{
public:

    TransformVisitor(XMLOutput&, const UnitPtr&, const TypePtr&, const TypePtr&, const TypePtr&, const TypePtr&, bool,
                     vector<string>&, vector<string>&);

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
class InitVisitor : public ParserVisitor
{
public:

    InitVisitor(XMLOutput&, const UnitPtr&);

    virtual bool visitClassDefStart(const ClassDefPtr&);
    virtual bool visitStructStart(const StructPtr&);
    virtual void visitSequence(const SequencePtr&);
    virtual void visitDictionary(const DictionaryPtr&);
    virtual void visitEnum(const EnumPtr&);

private:

    XMLOutput& _out;
    UnitPtr _oldUnit;
};

}

string
Transform::typeToString(const TypePtr& type)
{
    BuiltinPtr b = BuiltinPtr::dynamicCast(type);
    if(b)
    {
        return b->kindAsString();
    }
    else
    {
        ContainedPtr cont = ContainedPtr::dynamicCast(type);
        assert(cont);
        return cont->scoped();
    }
}

////////////////////////////////////
// TransformVisitor
////////////////////////////////////

Transform::TransformVisitor::TransformVisitor(XMLOutput& out, const UnitPtr& newUnit,
                                              const TypePtr& oldKey, const TypePtr& newKey,
                                              const TypePtr& oldValue, const TypePtr& newValue,
                                              bool ignoreTypeChanges, vector<string>& missingTypes,
                                              vector<string>& errors) :
    _out(out), _newUnit(newUnit), _ignoreTypeChanges(ignoreTypeChanges), _missingTypes(missingTypes), _errors(errors)
{
    out << se("database");

    string oldKeyName = typeToString(oldKey);
    string newKeyName = typeToString(newKey);
    if(oldKeyName == newKeyName)
    {
        out << attr("key", oldKeyName);
    }
    else
    {
        out << attr("key", oldKeyName + "," + newKeyName);
    }

    string oldValueName = typeToString(oldValue);
    string newValueName = typeToString(newValue);
    if(oldValueName == newValueName)
    {
        out << attr("value", oldValueName);
    }
    else
    {
        out << attr("value", oldValueName + "," + newValueName);
    }

    out << se("record");
    compareTypes("database key", oldKey, newKey);
    compareTypes("database value", oldValue, newValue);
    out << ee;

    out << ee;
}

bool
Transform::TransformVisitor::visitClassDefStart(const ClassDefPtr& v)
{
    if(v->isInterface() || v->isLocal())
    {
        return false;
    }

    string scoped = v->scoped();
    TypeList l = _newUnit->lookupTypeNoBuiltin(scoped, false);
    if(l.empty())
    {
        _missingTypes.push_back(scoped);
        return false;
    }

    ClassDeclPtr decl = ClassDeclPtr::dynamicCast(l.front());
    if(!decl || decl->isInterface())
    {
        if(!_ignoreTypeChanges)
        {
            typeChange(scoped, v->declaration(), l.front());
        }
        return false;
    }

    ClassDefPtr newClass = decl->definition();
    if(!newClass)
    {
        _missingTypes.push_back(scoped);
        return false;
    }

    _out.nl();
    _out.nl();
    _out << "<!-- class " << scoped << " -->";
    _out << se("transform") << attr("type", scoped);

    DataMemberList oldMembers = v->allDataMembers();
    DataMemberList newMembers = newClass->allDataMembers();
    compareMembers(oldMembers, newMembers);

    _out << ee;

    return false;
}

bool
Transform::TransformVisitor::visitStructStart(const StructPtr& v)
{
    if(v->isLocal())
    {
        return false;
    }

    string scoped = v->scoped();
    TypeList l = _newUnit->lookupTypeNoBuiltin(scoped, false);
    if(l.empty())
    {
        _missingTypes.push_back(scoped);
        return false;
    }

    StructPtr newStruct = StructPtr::dynamicCast(l.front());
    if(!newStruct)
    {
        if(!_ignoreTypeChanges)
        {
            typeChange(scoped, v, l.front());
        }
        return false;
    }

    _out.nl();
    _out.nl();
    _out << "<!-- struct " << scoped << " -->";
    _out << se("transform") << attr("type", scoped);

    DataMemberList oldMembers = v->dataMembers();
    DataMemberList newMembers = newStruct->dataMembers();
    compareMembers(oldMembers, newMembers);

    _out << ee;

    return false;
}

void
Transform::TransformVisitor::visitSequence(const SequencePtr& v)
{
    if(v->isLocal())
    {
        return;
    }

    string scoped = v->scoped();
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

    _out.nl();
    _out.nl();
    _out << "<!-- sequence " << scoped << " -->";
    _out << se("transform") << attr("type", scoped);

    compareTypes(scoped + " sequence type", v->type(), newSeq->type());

    _out << ee;
}

void
Transform::TransformVisitor::visitDictionary(const DictionaryPtr& v)
{
    if(v->isLocal())
    {
        return;
    }

    string scoped = v->scoped();
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

    _out.nl();
    _out.nl();
    _out << "<!-- dictionary " << scoped << " -->";
    _out << se("transform") << attr("type", scoped);

    compareTypes(scoped + " key type", v->keyType(), newDict->keyType());
    compareTypes(scoped + " value type", v->valueType(), newDict->valueType());

    _out << ee;
}

void
Transform::TransformVisitor::visitEnum(const EnumPtr& v)
{
    if(v->isLocal())
    {
        return;
    }

    string scoped = v->scoped();
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

    _out.nl();
    _out.nl();
    _out << "<!-- enum " << scoped << " -->";
    _out << se("transform") << attr("type", scoped);

    Slice::EnumeratorList enumerators = v->getEnumerators();
    int i = 0;
    for(Slice::EnumeratorList::iterator p = enumerators.begin(); p != enumerators.end(); ++p, ++i)
    {
        map<string, int>::const_iterator q = m.find((*p)->name());
        if(q == m.end())
        {
            _out.nl();
            _out << "<!-- NOTICE: enumerator `" << (*p)->name() << "' has been removed -->";
        }
        else if(q->second != i)
        {
            _out.nl();
            _out << "<!-- NOTICE: enumerator `" << (*p)->name() << "' has changed position -->";
        }
    }

    _out << ee;
}

void
Transform::TransformVisitor::compareMembers(const DataMemberList& oldMembers, const DataMemberList& newMembers)
{
    map<string, DataMemberPtr> oldMap, newMap;
    map<string, DataMemberPtr>::iterator q;
    DataMemberList::const_iterator p;

    for(p = oldMembers.begin(); p != oldMembers.end(); ++p)
    {
        oldMap.insert(pair<string, DataMemberPtr>((*p)->name(), *p));
    }

    for(p = newMembers.begin(); p != newMembers.end(); ++p)
    {
        newMap.insert(pair<string, DataMemberPtr>((*p)->name(), *p));
    }

    for(p = oldMembers.begin(); p != oldMembers.end(); ++p)
    {
        string name = (*p)->name();
        q = newMap.find(name);
        if(q == newMap.end())
        {
            _out.nl();
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
    for(q = newMap.begin(); q != newMap.end(); ++q)
    {
        _out.nl();
        _out << "<!-- NOTICE: " << q->first << " has been added -->";
    }
}

void
Transform::TransformVisitor::compareTypes(const string& desc, const TypePtr& oldType, const TypePtr& newType)
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
        //
        // Allow target type of Object.
        //
        BuiltinPtr newb = BuiltinPtr::dynamicCast(newType);
        if(newb && newb->kind() == Builtin::KindObject)
        {
            return;
        }

        ClassDeclPtr newcl = ClassDeclPtr::dynamicCast(newType);
        if(newcl && checkClasses(cl, newcl))
        {
            return;
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
Transform::TransformVisitor::typeChange(const string& desc, const TypePtr& t1, const TypePtr& t2)
{
    BuiltinPtr b1 = BuiltinPtr::dynamicCast(t1);
    BuiltinPtr b2 = BuiltinPtr::dynamicCast(t2);
    ContainedPtr c1 = ContainedPtr::dynamicCast(t1);
    ContainedPtr c2 = ContainedPtr::dynamicCast(t2);

    if(_ignoreTypeChanges)
    {
        _out.nl();
        _out << "<!-- NOTICE: " << desc << " has changed from ";
        if(b1)
        {
            _out << b1->kindAsString();
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
        else
        {
            assert(c2);
            ostr << c2->kindOf() << ' ' << c2->scoped();
        }
        _errors.push_back(ostr.str());
    }
}

bool
Transform::TransformVisitor::checkClasses(const ClassDeclPtr& from, const ClassDeclPtr& to)
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

Transform::InitVisitor::InitVisitor(XMLOutput& out, const UnitPtr& oldUnit) :
    _out(out), _oldUnit(oldUnit)
{
}

bool
Transform::InitVisitor::visitClassDefStart(const ClassDefPtr& v)
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
            BuiltinPtr b1 = BuiltinPtr::dynamicCast(l.front());
            ContainedPtr c1 = ContainedPtr::dynamicCast(l.front());

            _out.nl();
            _out << "<!-- NOTICE: " << scoped << " has changed from ";
            if(b1)
            {
                _out << b1->kindAsString();
            }
            else
            {
                assert(c1);
                _out << c1->kindOf();
            }
            _out << " to class";
            _out << " -->";
        }
        else
        {
            return false;
        }
    }

    _out.nl();
    _out.nl();
    _out << "<!-- class " << scoped << " -->";
    _out << se("init") << attr("type", scoped);
    _out << ee;

    return false;
}

bool
Transform::InitVisitor::visitStructStart(const StructPtr& v)
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
            BuiltinPtr b1 = BuiltinPtr::dynamicCast(l.front());
            ContainedPtr c1 = ContainedPtr::dynamicCast(l.front());

            _out.nl();
            _out << "<!-- NOTICE: " << scoped << " has changed from ";
            if(b1)
            {
                _out << b1->kindAsString();
            }
            else
            {
                assert(c1);
                _out << c1->kindOf();
            }
            _out << " to struct";
            _out << " -->";
        }
        else
        {
            return false;
        }
    }

    _out.nl();
    _out.nl();
    _out << "<!-- struct " << scoped << " -->";
    _out << se("init") << attr("type", scoped);
    _out << ee;

    return false;
}

void
Transform::InitVisitor::visitSequence(const SequencePtr& v)
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
            BuiltinPtr b1 = BuiltinPtr::dynamicCast(l.front());
            ContainedPtr c1 = ContainedPtr::dynamicCast(l.front());

            _out.nl();
            _out << "<!-- NOTICE: " << scoped << " has changed from ";
            if(b1)
            {
                _out << b1->kindAsString();
            }
            else
            {
                assert(c1);
                _out << c1->kindOf();
            }
            _out << " to sequence";
            _out << " -->";
        }
        else
        {
            return;
        }
    }

    _out.nl();
    _out.nl();
    _out << "<!-- sequence " << scoped << " -->";
    _out << se("transform") << attr("type", scoped);
    _out << ee;
}

void
Transform::InitVisitor::visitDictionary(const DictionaryPtr& v)
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
            BuiltinPtr b1 = BuiltinPtr::dynamicCast(l.front());
            ContainedPtr c1 = ContainedPtr::dynamicCast(l.front());

            _out.nl();
            _out << "<!-- NOTICE: " << scoped << " has changed from ";
            if(b1)
            {
                _out << b1->kindAsString();
            }
            else
            {
                assert(c1);
                _out << c1->kindOf();
            }
            _out << " to dictionary";
            _out << " -->";
        }
        else
        {
            return;
        }
    }

    _out.nl();
    _out.nl();
    _out << "<!-- dictionary " << scoped << " -->";
    _out << se("init") << attr("type", scoped);
    _out << ee;
}

void
Transform::InitVisitor::visitEnum(const EnumPtr& v)
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
            BuiltinPtr b1 = BuiltinPtr::dynamicCast(l.front());
            ContainedPtr c1 = ContainedPtr::dynamicCast(l.front());

            _out.nl();
            _out << "<!-- NOTICE: " << scoped << " has changed from ";
            if(b1)
            {
                _out << b1->kindAsString();
            }
            else
            {
                assert(c1);
                _out << c1->kindOf();
            }
            _out << " to enum";
            _out << " -->";
        }
        else
        {
            return;
        }
    }

    _out.nl();
    _out << "<!-- enum " << scoped << " -->";
    _out << se("init") << attr("type", scoped);
    _out << ee;
}

Transform::Analyzer::Analyzer(const UnitPtr& oldUnit, const UnitPtr& newUnit, bool ignoreTypeChanges) :
    _old(oldUnit), _new(newUnit), _ignoreTypeChanges(ignoreTypeChanges)
{
}

void
Transform::Analyzer::analyze(const TypePtr& oldKey, const TypePtr& newKey, const TypePtr& oldValue,
                             const TypePtr& newValue, ostream& os, vector<string>& missingTypes,
                             vector<string>& errors)
{
    XMLOutput out(os);

    out << se("transformer");

    TransformVisitor transformVisitor(out, _new, oldKey, newKey, oldValue, newValue, _ignoreTypeChanges, missingTypes,
                                      errors);
    _old->visit(&transformVisitor);

    InitVisitor initVisitor(out, _old);
    _new->visit(&initVisitor);

    out << ee;
    out << '\n';
}
