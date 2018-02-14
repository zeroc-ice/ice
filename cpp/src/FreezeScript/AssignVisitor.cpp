// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <FreezeScript/AssignVisitor.h>
#include <FreezeScript/Util.h>
#include <climits>

using namespace std;

FreezeScript::AssignVisitor::AssignVisitor(const DataPtr& src, const DataFactoryPtr& factory,
                                           const ErrorReporterPtr& errorReporter, bool convert,
                                           const string& context) :
    _src(src), _factory(factory), _errorReporter(errorReporter), _convert(convert),
    _context(context)
{
}

void
FreezeScript::AssignVisitor::visitBoolean(const BooleanDataPtr& dest)
{
    StringDataPtr s = StringDataPtr::dynamicCast(_src);
    if(s)
    {
        string v = s->getValue();
        if(v == "true")
        {
            dest->setValue(true);
        }
        else if(v == "false")
        {
            dest->setValue(false);
        }
        else
        {
            conversionError(dest->getType(), _src->getType(), v);
        }
    }
    else
    {
        dest->setValue(_src->booleanValue(_convert));
    }
}

void
FreezeScript::AssignVisitor::visitInteger(const IntegerDataPtr& dest)
{
    dest->setValue(_src->integerValue(_convert), true);
}

void
FreezeScript::AssignVisitor::visitDouble(const DoubleDataPtr& dest)
{
    dest->setValue(_src->doubleValue(_convert));
}

void
FreezeScript::AssignVisitor::visitString(const StringDataPtr& dest)
{
    dest->setValue(_src->stringValue(_convert));
}

void
FreezeScript::AssignVisitor::visitProxy(const ProxyDataPtr& dest)
{
    ProxyDataPtr p = ProxyDataPtr::dynamicCast(_src);
    if(p)
    {
        dest->setValue(p->getValue());
    }
    else
    {
        StringDataPtr s = StringDataPtr::dynamicCast(_src);
        if(s)
        {
            dest->setValue(s->getValue(), false);
        }
        else
        {
            typeMismatchError(dest->getType(), _src->getType());
        }
    }
}

void
FreezeScript::AssignVisitor::visitStruct(const StructDataPtr& dest)
{
    Slice::StructPtr type = Slice::StructPtr::dynamicCast(dest->getType());
    assert(type);
    StructDataPtr src = StructDataPtr::dynamicCast(_src);
    if(src && isCompatible(type, src->getType()))
    {
        //
        // Assign members with the same name.
        //
        DataMemberMap srcMap = src->getMembers();
        DataMemberMap destMap = dest->getMembers();
        string typeName = typeToString(type);
        for(DataMemberMap::iterator p = destMap.begin(); p != destMap.end(); ++p)
        {
            DataMemberMap::iterator q = srcMap.find(p->first);
            if(q != srcMap.end())
            {
                string context = typeName + " member " + p->first + " value";
                AssignVisitor v(q->second, _factory, _errorReporter, _convert, context);
                p->second->visit(v);
            }
        }
    }
    else
    {
        typeMismatchError(type, _src->getType());
    }
}

void
FreezeScript::AssignVisitor::visitSequence(const SequenceDataPtr& dest)
{
    Slice::TypePtr type = dest->getType();
    SequenceDataPtr src = SequenceDataPtr::dynamicCast(_src);
    if(src && isCompatible(type, src->getType()))
    {
        DataList& srcElements = src->getElements();
        DataList destElements;
        Slice::SequencePtr seqType = Slice::SequencePtr::dynamicCast(type);
        assert(seqType);
        Slice::TypePtr elemType = seqType->type();
        string typeName = typeToString(type);
        for(DataList::const_iterator p = srcElements.begin(); p != srcElements.end(); ++p)
        {
            DataPtr element = _factory->create(elemType, false);
            Destroyer<DataPtr> elementDestroyer(element);
            AssignVisitor v(*p, _factory, _errorReporter, _convert, typeName + " element");
            element->visit(v);
            destElements.push_back(element);
            elementDestroyer.release();
        }
        DataList& l = dest->getElements();
        l.swap(destElements);
    }
    else
    {
        typeMismatchError(type, _src->getType());
    }
}

void
FreezeScript::AssignVisitor::visitEnum(const EnumDataPtr& dest)
{
    Slice::TypePtr type = dest->getType();
    IntegerDataPtr i = IntegerDataPtr::dynamicCast(_src);
    if(i)
    {
        if(_convert)
        {
            Ice::Long l = i->integerValue();
            if(l < 0 || l > INT_MAX || !dest->setValue(static_cast<Ice::Int>(l)))
            {
                rangeError(i->toString(), type);
            }
        }
        else
        {
            conversionError(type, i->getType(), i->toString());
        }
    }
    else
    {
        string name;
        EnumDataPtr e = EnumDataPtr::dynamicCast(_src);
        if(e && isCompatible(type, _src->getType()))
        {
            name = e->toString();
        }
        else
        {
            StringDataPtr s = StringDataPtr::dynamicCast(_src);
            if(s)
            {
                name = s->getValue();
            }
            else
            {
                typeMismatchError(type, _src->getType());
            }
        }

        if(!dest->setValueAsString(name))
        {
            conversionError(type, _src->getType(), name);
        }
    }
}

void
FreezeScript::AssignVisitor::visitDictionary(const DictionaryDataPtr& dest)
{
    Slice::TypePtr type = dest->getType();
    DictionaryDataPtr d = DictionaryDataPtr::dynamicCast(_src);
    if(d && isCompatible(type, _src->getType()))
    {
        DataMap& srcMap = d->getElements();
        DataMap destMap;
        Slice::DictionaryPtr dictType = Slice::DictionaryPtr::dynamicCast(type);
        assert(dictType);
        Slice::TypePtr keyType = dictType->keyType();
        Slice::TypePtr valueType = dictType->valueType();
        string typeName = typeToString(type);
        for(DataMap::const_iterator p = srcMap.begin(); p != srcMap.end(); ++p)
        {
            DataPtr key = _factory->create(keyType, false);
            Destroyer<DataPtr> keyDestroyer(key);
            DataPtr value = _factory->create(valueType, false);
            Destroyer<DataPtr> valueDestroyer(value);

            AssignVisitor keyVisitor(p->first, _factory, _errorReporter, _convert, typeName + " key");
            key->visit(keyVisitor);

            AssignVisitor valueVisitor(p->second, _factory, _errorReporter, _convert, typeName + " value");
            value->visit(valueVisitor);

            DataMap::const_iterator q = destMap.find(key);
            if(q != destMap.end())
            {
                error("duplicate dictionary key in " + typeToString(dictType));
            }
            else
            {
                destMap.insert(DataMap::value_type(key, value));
                keyDestroyer.release();
                valueDestroyer.release();
            }
        }
        DataMap& m = dest->getElements();
        m.swap(destMap);
    }
    else
    {
        typeMismatchError(type, _src->getType());
    }
}

void
FreezeScript::AssignVisitor::visitObject(const ObjectRefPtr& dest)
{
    Slice::TypePtr type = dest->getType();
    ObjectRefPtr src = ObjectRefPtr::dynamicCast(_src);
    if(!src)
    {
        typeMismatchError(type, _src->getType());
    }

    ObjectDataPtr srcValue = src->getValue();
    Slice::TypePtr srcType = src->getType();
    if(!srcValue)
    {
        //
        // Allow a nil value from type Object.
        //
        if(Slice::BuiltinPtr::dynamicCast(srcType) || isCompatible(type, srcType))
        {
            dest->setValue(0);
        }
        else
        {
            typeMismatchError(type, srcType);
        }
    }
    else
    {
        Slice::TypePtr srcValueType = srcValue->getType();
        if(isCompatible(type, srcValueType))
        {
            dest->setValue(srcValue);
        }
        else
        {
            typeMismatchError(type, srcValueType);
        }
    }
}

bool
FreezeScript::AssignVisitor::isCompatible(const Slice::TypePtr& dest, const Slice::TypePtr& src)
{
    Slice::BuiltinPtr b1 = Slice::BuiltinPtr::dynamicCast(dest);
    if(b1)
    {
        Slice::BuiltinPtr b2 = Slice::BuiltinPtr::dynamicCast(src);
        switch(b1->kind())
        {
        case Slice::Builtin::KindByte:
        case Slice::Builtin::KindShort:
        case Slice::Builtin::KindInt:
        case Slice::Builtin::KindLong:
        {
            if(b2)
            {
                switch(b2->kind())
                {
                case Slice::Builtin::KindByte:
                case Slice::Builtin::KindShort:
                case Slice::Builtin::KindInt:
                case Slice::Builtin::KindLong:
                case Slice::Builtin::KindString:
                {
                    return true;
                }
                case Slice::Builtin::KindBool:
                case Slice::Builtin::KindFloat:
                case Slice::Builtin::KindDouble:
                case Slice::Builtin::KindObject:
                case Slice::Builtin::KindObjectProxy:
                case Slice::Builtin::KindLocalObject:
                {
                    return false;
                }
                }
            }

            return false;
        }
        case Slice::Builtin::KindBool:
        {
            if(b2 && (b2->kind() == Slice::Builtin::KindBool || b2->kind() == Slice::Builtin::KindString))
            {
                return true;
            }

            return false;
        }
        case Slice::Builtin::KindFloat:
        case Slice::Builtin::KindDouble:
        {
            if(b2)
            {
                switch(b2->kind())
                {
                case Slice::Builtin::KindFloat:
                case Slice::Builtin::KindDouble:
                case Slice::Builtin::KindString:
                {
                    return true;
                }
                case Slice::Builtin::KindByte:
                case Slice::Builtin::KindShort:
                case Slice::Builtin::KindInt:
                case Slice::Builtin::KindLong:
                case Slice::Builtin::KindBool:
                case Slice::Builtin::KindObject:
                case Slice::Builtin::KindObjectProxy:
                case Slice::Builtin::KindLocalObject:
                {
                    return false;
                }
                }
            }

            return false;
        }
        case Slice::Builtin::KindString:
        {
            if(b2)
            {
                switch(b2->kind())
                {
                case Slice::Builtin::KindByte:
                case Slice::Builtin::KindBool:
                case Slice::Builtin::KindShort:
                case Slice::Builtin::KindInt:
                case Slice::Builtin::KindLong:
                case Slice::Builtin::KindFloat:
                case Slice::Builtin::KindDouble:
                case Slice::Builtin::KindString:
                case Slice::Builtin::KindObjectProxy:
                {
                    return true;
                }
                case Slice::Builtin::KindObject:
                case Slice::Builtin::KindLocalObject:
                {
                    return false;
                }
                }

                return false;
            }
            else if(Slice::EnumPtr::dynamicCast(src))
            {
                return true;
            }
            else if(Slice::ProxyPtr::dynamicCast(src))
            {
                return true;
            }

            return false;
        }
        case Slice::Builtin::KindObject:
        {
            //
            // Allow transformation from Object to class. Validation has to
            // be done during transformation, when the actual type of
            // an instance can be compared for compatibility with the
            // new type.
            //
            Slice::ClassDeclPtr cl = Slice::ClassDeclPtr::dynamicCast(src);
            if(cl || (b2 && b2->kind() == Slice::Builtin::KindObject))
            {
                return true;
            }

            return false;
        }
        case Slice::Builtin::KindObjectProxy:
        {
            Slice::ProxyPtr p = Slice::ProxyPtr::dynamicCast(src);
            if(p || (b2 && b2->kind() == Slice::Builtin::KindObjectProxy) ||
               (b2 && b2->kind() == Slice::Builtin::KindString))
            {
                return true;
            }

            return false;
        }
        case Slice::Builtin::KindLocalObject:
        {
            assert(false);
            return false;
        }
        }

        assert(false);
    }

    Slice::ClassDeclPtr cl1 = Slice::ClassDeclPtr::dynamicCast(dest);
    if(cl1)
    {
        Slice::ClassDeclPtr cl2 = Slice::ClassDeclPtr::dynamicCast(src);
        if(cl2 && checkClasses(cl1, cl2))
        {
            return true;
        }

        return false;
    }

    Slice::StructPtr s1 = Slice::StructPtr::dynamicCast(dest);
    if(s1)
    {
        Slice::StructPtr s2 = Slice::StructPtr::dynamicCast(src);
        if(s2 && s1->scoped() == s2->scoped())
        {
            return true;
        }

        return false;
    }

    Slice::ProxyPtr p1 = Slice::ProxyPtr::dynamicCast(dest);
    if(p1)
    {
        Slice::BuiltinPtr b2 = Slice::BuiltinPtr::dynamicCast(src);
        if(b2 && (b2->kind() == Slice::Builtin::KindObjectProxy || b2->kind() == Slice::Builtin::KindString))
        {
            return true;
        }

        Slice::ProxyPtr p2 = Slice::ProxyPtr::dynamicCast(src);
        if(p2 && checkClasses(p1->_class(), p2->_class()))
        {
            return true;
        }

        return false;
    }

    Slice::DictionaryPtr d1 = Slice::DictionaryPtr::dynamicCast(dest);
    if(d1)
    {
        Slice::DictionaryPtr d2 = Slice::DictionaryPtr::dynamicCast(src);
        if(d2)
        {
            return isCompatible(d1->keyType(), d2->keyType()) &&
                   isCompatible(d1->valueType(), d2->valueType());
        }

        return false;
    }

    Slice::SequencePtr seq1 = Slice::SequencePtr::dynamicCast(dest);
    if(seq1)
    {
        Slice::SequencePtr seq2 = Slice::SequencePtr::dynamicCast(src);
        if(seq2)
        {
            return isCompatible(seq1->type(), seq2->type());
        }

        return false;
    }

    Slice::EnumPtr e1 = Slice::EnumPtr::dynamicCast(dest);
    if(e1)
    {
        Slice::BuiltinPtr b2 = Slice::BuiltinPtr::dynamicCast(src);
        if(b2 && b2->kind() == Slice::Builtin::KindString)
        {
            return true;
        }

        Slice::EnumPtr e2 = Slice::EnumPtr::dynamicCast(src);
        if(e2 && e1->scoped() == e2->scoped())
        {
            return true;
        }

        return false;
    }

    assert(false);
    return false;
}

bool
FreezeScript::AssignVisitor::checkClasses(const Slice::ClassDeclPtr& dest, const Slice::ClassDeclPtr& src)
{
    string s1 = dest->scoped();
    string s2 = src->scoped();
    if(s1 == s2)
    {
        return true;
    }
    else
    {
        Slice::ClassDefPtr def = src->definition();
        if(!def)
        {
            error("class " + s2 + " declared but not defined");
        }
        return def->isA(s1);
    }

    return false;
}

void
FreezeScript::AssignVisitor::typeMismatchError(const Slice::TypePtr& dest, const Slice::TypePtr& src)
{
    ostringstream ostr;
    ostr << "unable to transform";
    if(!_context.empty())
    {
        ostr << ' ' << _context;
    }
    ostr << " from " << typeToString(src) << " to " << typeToString(dest);
    error(ostr.str());
}

void
FreezeScript::AssignVisitor::conversionError(const Slice::TypePtr& dest, const Slice::TypePtr& src,
                                             const string& value)
{
    ostringstream ostr;
    ostr << "unable to convert";
    if(!_context.empty())
    {
        ostr << ' ' << _context;
    }
    ostr << " `" << value << "' from " << typeToString(src) << " to " << typeToString(dest);
    error(ostr.str());
}

void
FreezeScript::AssignVisitor::rangeError(const string& value, const Slice::TypePtr& type)
{
    ostringstream ostr;
    if(!_context.empty())
    {
        ostr << _context << ' ';
    }
    ostr << "`" << value << "' is out of range for type " << typeToString(type);
    error(ostr.str());
}

void
FreezeScript::AssignVisitor::error(const string& msg)
{
    _errorReporter->error(msg);
}
