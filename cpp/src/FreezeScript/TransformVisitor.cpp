// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <FreezeScript/TransformVisitor.h>
#include <FreezeScript/Util.h>
#include <IceUtil/InputUtil.h>

using namespace std;

FreezeScript::TransformVisitor::TransformVisitor(const DataPtr& src, const TransformInfoPtr& info,
                                                 const string& context) :
    _src(src), _info(info), _context(context)
{
    assert(_info);
}

void
FreezeScript::TransformVisitor::visitBoolean(const BooleanDataPtr& dest)
{
    Slice::TypePtr type = dest->getType();
    if(_info->doDefaultTransform(type))
    {
        BooleanDataPtr b = BooleanDataPtr::dynamicCast(_src);
        if(b)
        {
            dest->setValue(b->getValue());
        }
        else
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
                    conversionError(type, _src->getType(), v);
                }
            }
            else
            {
                typeMismatchError(type, _src->getType());
            }
        }
    }
    _info->executeCustomTransform(dest, _src);
}

void
FreezeScript::TransformVisitor::visitInteger(const IntegerDataPtr& dest)
{
    Slice::TypePtr type = dest->getType();
    if(_info->doDefaultTransform(type))
    {
        IntegerDataPtr i = IntegerDataPtr::dynamicCast(_src);
        if(i)
        {
            dest->setValue(i->getValue(), false);
        }
        else
        {
            StringDataPtr s = StringDataPtr::dynamicCast(_src);
            if(s)
            {
                string str = s->getValue();
                Ice::Long value;
                if(IceUtilInternal::stringToInt64(str, value))
                {
                    dest->setValue(value, false);
                }
                else
                {
                    conversionError(type, _src->getType(), str);
                }
            }
            else
            {
                typeMismatchError(type, _src->getType());
            }
        }
    }
    _info->executeCustomTransform(dest, _src);
}

void
FreezeScript::TransformVisitor::visitDouble(const DoubleDataPtr& dest)
{
    Slice::TypePtr type = dest->getType();
    if(_info->doDefaultTransform(type))
    {
        DoubleDataPtr d = DoubleDataPtr::dynamicCast(_src);
        if(d)
        {
            dest->setValue(d->doubleValue());
        }
        else
        {
            StringDataPtr s = StringDataPtr::dynamicCast(_src);
            if(s)
            {
                string str = s->stringValue();
                const char* start = str.c_str();
                char* end;
                double v = strtod(start, &end);
                if(errno == ERANGE)
                {
                    rangeError(str, type);
                }
                else
                {
                    while(*end)
                    {
                        if(!isspace(static_cast<unsigned char>(*end)))
                        {
                            conversionError(type, _src->getType(), str);
                            return;
                        }
                        end++;
                    }
                    if(!*end)
                    {
                        dest->setValue(v);
                    }
                }
            }
            else
            {
                typeMismatchError(type, _src->getType());
            }
        }
    }
    _info->executeCustomTransform(dest, _src);
}

void
FreezeScript::TransformVisitor::visitString(const StringDataPtr& dest)
{
    Slice::TypePtr type = dest->getType();
    if(_info->doDefaultTransform(type))
    {
        StringDataPtr s = StringDataPtr::dynamicCast(_src);
        BooleanDataPtr b = BooleanDataPtr::dynamicCast(_src);
        IntegerDataPtr i = IntegerDataPtr::dynamicCast(_src);
        DoubleDataPtr d = DoubleDataPtr::dynamicCast(_src);
        EnumDataPtr e = EnumDataPtr::dynamicCast(_src);
        ProxyDataPtr p = ProxyDataPtr::dynamicCast(_src);
        if(s || b || i || d || e || p)
        {
            dest->setValue(_src->toString());
        }
        else
        {
            typeMismatchError(type, _src->getType());
        }
    }
    _info->executeCustomTransform(dest, _src);
}

void
FreezeScript::TransformVisitor::visitProxy(const ProxyDataPtr& dest)
{
    Slice::TypePtr type = dest->getType();
    if(_info->doDefaultTransform(type))
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
                typeMismatchError(type, _src->getType());
            }
        }
    }
    _info->executeCustomTransform(dest, _src);
}

void
FreezeScript::TransformVisitor::visitStruct(const StructDataPtr& dest)
{
    Slice::TypePtr type = dest->getType();
    if(_info->doDefaultTransform(type))
    {
        //
        // Support struct->struct and class->struct transforms.
        //
        StructDataPtr s = StructDataPtr::dynamicCast(_src);
        ObjectRefPtr obj = ObjectRefPtr::dynamicCast(_src);
        if((s || obj) && isCompatible(type, _src->getType()))
        {
            //
            // Transform members with the same name.
            //
            DataMemberMap srcMap = s ? s->getMembers() : obj->getValue()->getMembers();
            DataMemberMap destMap = dest->getMembers();
            string typeName = typeToString(type);
            for(DataMemberMap::iterator p = destMap.begin(); p != destMap.end(); ++p)
            {
                DataMemberMap::iterator q = srcMap.find(p->first);
                if(q != srcMap.end())
                {
                    string context = typeName + " member " + p->first + " value";
                    TransformVisitor v(q->second, _info, context);
                    p->second->visit(v);
                }
            }
        }
        else
        {
            typeMismatchError(type, _src->getType());
        }
    }
    _info->executeCustomTransform(dest, _src);
}

void
FreezeScript::TransformVisitor::visitSequence(const SequenceDataPtr& dest)
{
    Slice::TypePtr type = dest->getType();
    if(_info->doDefaultTransform(type))
    {
        SequenceDataPtr s = SequenceDataPtr::dynamicCast(_src);
        if(s && isCompatible(type, _src->getType()))
        {
            DataList& srcElements = s->getElements();
            DataList destElements;
            Slice::SequencePtr seqType = Slice::SequencePtr::dynamicCast(type);
            assert(seqType);
            Slice::TypePtr elemType = seqType->type();
            string typeName = typeToString(type);
            for(DataList::const_iterator p = srcElements.begin(); p != srcElements.end(); ++p)
            {
                DataPtr element = _info->getDataFactory()->create(elemType, false);
                Destroyer<DataPtr> elementDestroyer(element);
                try
                {
                    TransformVisitor v(*p, _info, typeName + " element");
                    element->visit(v);
                    destElements.push_back(element);
                    elementDestroyer.release();
                }
                catch(const ClassNotFoundException& ex)
                {
                    //
                    // If transformation of the sequence element fails because a class
                    // could not be found, then we invoke purgeObjects() to determine
                    // whether we should ignore the situation (and remove the element
                    // from the sequence) or raise the exception again.
                    //
                    if(!_info->purgeObjects())
                    {
                        throw;
                    }
                    warning("purging element of sequence " + typeToString(type) +
                            " due to missing class type " + ex.id);
                }
            }
            DataList& l = dest->getElements();
            l.swap(destElements);
        }
        else
        {
            typeMismatchError(type, _src->getType());
        }
    }
    _info->executeCustomTransform(dest, _src);
}

void
FreezeScript::TransformVisitor::visitEnum(const EnumDataPtr& dest)
{
    Slice::TypePtr type = dest->getType();
    if(_info->doDefaultTransform(type))
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
                return;
            }
        }

        if(!dest->setValueAsString(name))
        {
            conversionError(type, _src->getType(), name);
        }
    }
    _info->executeCustomTransform(dest, _src);
}

void
FreezeScript::TransformVisitor::visitDictionary(const DictionaryDataPtr& dest)
{
    Slice::TypePtr type = dest->getType();
    if(_info->doDefaultTransform(type))
    {
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
                DataPtr key = _info->getDataFactory()->create(keyType, false);
                Destroyer<DataPtr> keyDestroyer(key);
                DataPtr value = _info->getDataFactory()->create(valueType, false);
                Destroyer<DataPtr> valueDestroyer(value);

                TransformVisitor keyVisitor(p->first, _info, typeName + " key");
                key->visit(keyVisitor);

                try
                {
                    TransformVisitor valueVisitor(p->second, _info, typeName + " value");
                    value->visit(valueVisitor);
                }
                catch(const ClassNotFoundException& ex)
                {
                    //
                    // If transformation of the dictionary value fails because a class
                    // could not be found, then we invoke purgeObjects() to determine
                    // whether we should ignore the situation (and remove the element
                    // from the dictionary) or raise the exception again.
                    //
                    if(!_info->purgeObjects())
                    {
                        throw;
                    }
                    warning("purging element of dictionary " + typeToString(dictType) + " due to missing class type " +
                            ex.id);
                    continue;
                }

                DataMap::const_iterator q = destMap.find(key);
                if(q != destMap.end())
                {
                    warning("duplicate dictionary key in " + typeToString(dictType));
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
    _info->executeCustomTransform(dest, _src);
}

void
FreezeScript::TransformVisitor::visitObject(const ObjectRefPtr& dest)
{
    //
    // Support struct->class and class->class transforms.
    //
    Slice::TypePtr type = dest->getType();
    ObjectRefPtr src = ObjectRefPtr::dynamicCast(_src);
    StructDataPtr s = StructDataPtr::dynamicCast(_src);
    if(!src && !s)
    {
        typeMismatchError(type, _src->getType());
    }
    else if(_info->doDefaultTransform(type))
    {
        if(src)
        {
            //
            // class->class transform
            //
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
                    //
                    // If the types are in the same Slice unit, then we can simply
                    // copy the reference. Otherwise, we check the object map to
                    // see if an equivalent object has already been created, and
                    // if not, then we have to create one.
                    //
                    if(type->unit().get() == srcValueType->unit().get())
                    {
                        dest->setValue(srcValue);
                    }
                    else
                    {
                        ObjectDataMap& objectDataMap = _info->getObjectDataMap();
                        ObjectDataMap::iterator p = objectDataMap.find(srcValue.get());
                        if(p != objectDataMap.end() && p->second)
                        {
                            dest->setValue(p->second);
                        }
                        else
                        {
                            //
                            // If the type has been renamed, we need to get its equivalent
                            // in the new Slice definitions.
                            //
                            Slice::TypePtr newType = _info->getRenamedType(srcValueType);
                            if(!newType)
                            {
                                string name = typeToString(srcValueType);
                                Slice::TypeList l = type->unit()->lookupType(name, false);
                                if(l.empty())
                                {
                                    throw ClassNotFoundException(name);
                                }
                                newType = l.front();
                            }

                            //
                            // Use createObject() so that an initializer is invoked if necessary.
                            //
                            DataPtr newObj = _info->getDataFactory()->createObject(newType, false);
                            ObjectRefPtr newRef = ObjectRefPtr::dynamicCast(newObj);
                            assert(newRef);

                            ObjectDataPtr newValue = newRef->getValue();
                            try
                            {
                                transformObject(newValue, srcValue);
                            }
                            catch(...)
                            {
                                newObj->destroy();
                                throw;
                            }

                            dest->setValue(newValue);
                            newObj->destroy();
                        }
                    }
                }
                else
                {
                    typeMismatchError(type, srcValueType);
                }
            }
        }
        else
        {
            //
            // struct->class transform
            //
            Slice::TypePtr srcType = _src->getType();
            if(isCompatible(type, srcType))
            {
                //
                // If the type has been renamed, we need to get its equivalent
                // in the new Slice definitions.
                //
                Slice::TypePtr newType = _info->getRenamedType(srcType);
                if(!newType)
                {
                    string name = typeToString(srcType);
                    Slice::TypeList l = type->unit()->lookupType(name, false);
                    if(l.empty())
                    {
                        throw ClassNotFoundException(name);
                    }
                    newType = l.front();
                }

                //
                // Use createObject() so that an initializer is invoked if necessary.
                //
                DataPtr newObj = _info->getDataFactory()->createObject(newType, false);
                ObjectRefPtr newRef = ObjectRefPtr::dynamicCast(newObj);
                assert(newRef);

                ObjectDataPtr newValue = newRef->getValue();
                try
                {
                    //
                    // Transform members with the same name.
                    //
                    DataMemberMap srcMap = s->getMembers();
                    DataMemberMap destMap = newValue->getMembers();
                    string typeName = typeToString(type);
                    for(DataMemberMap::iterator p = destMap.begin(); p != destMap.end(); ++p)
                    {
                        DataMemberMap::iterator q = srcMap.find(p->first);
                        if(q != srcMap.end())
                        {
                            string context = typeName + " member " + p->first + " value";
                            TransformVisitor v(q->second, _info, context);
                            p->second->visit(v);
                        }
                    }
                }
                catch(...)
                {
                    newObj->destroy();
                    throw;
                }

                dest->setValue(newValue);
                newObj->destroy();
            }
            else
            {
                typeMismatchError(type, srcType);
            }
        }
    }
    _info->executeCustomTransform(dest, _src);
}

void
FreezeScript::TransformVisitor::transformObject(const ObjectDataPtr& dest, const ObjectDataPtr& src)
{
    //
    // The source object must be present in the object map (we currently don't support
    // transforming two ObjectData instances from the same Slice unit - this transform
    // would be handled by-reference at the ObjectRef level). We must update the object
    // map before transforming members in order to handle cycles.
    //
    ObjectDataMap& objectDataMap = _info->getObjectDataMap();
    ObjectDataMap::iterator p = objectDataMap.find(src.get());
    assert(p != objectDataMap.end());
    assert(p->second == 0);
    objectDataMap.erase(p);
    objectDataMap.insert(ObjectDataMap::value_type(src.get(), dest));
    p = objectDataMap.find(src.get());

    try
    {
        //
        // Transform members with the same name.
        //
        DataMemberMap srcMap = src->getMembers();
        DataMemberMap destMap = dest->getMembers();
        string typeName = typeToString(dest->getType());
        for(DataMemberMap::iterator p = destMap.begin(); p != destMap.end(); ++p)
        {
            DataMemberMap::iterator q = srcMap.find(p->first);
            if(q != srcMap.end())
            {
                string context = typeName + " member " + p->first + " value";
                TransformVisitor v(q->second, _info, context);
                p->second->visit(v);
            }
        }
    }
    catch(...)
    {
        objectDataMap.erase(p);

#if (defined(_MSC_VER) && (_MSC_VER >= 1600))
        objectDataMap.insert(ObjectDataMap::value_type(src.get(), static_cast<ObjectDataPtr>(nullptr)));
#else
        objectDataMap.insert(ObjectDataMap::value_type(src.get(), 0));
#endif
        throw;
    }
}

bool
FreezeScript::TransformVisitor::checkRename(const Slice::TypePtr& dest, const Slice::TypePtr& src)
{
    if(dest->unit().get() != src->unit().get())
    {
        Slice::TypePtr t = _info->getRenamedType(src);
        return t.get() == dest.get();
    }

    return false;
}

bool
FreezeScript::TransformVisitor::isCompatible(const Slice::TypePtr& dest, const Slice::TypePtr& src)
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

        Slice::StructPtr s2 = Slice::StructPtr::dynamicCast(src);
        if(s2)
        {
            if(checkRename(dest, src))
            {
                return true;
            }

            if(s2 && cl1->scoped() == s2->scoped())
            {
                return true;
            }
        }

        return false;
    }

    Slice::StructPtr s1 = Slice::StructPtr::dynamicCast(dest);
    if(s1)
    {
        if(checkRename(dest, src))
        {
            return true;
        }

        Slice::StructPtr s2 = Slice::StructPtr::dynamicCast(src);
        Slice::ClassDeclPtr cl2 = Slice::ClassDeclPtr::dynamicCast(src);
        if((s2 && s1->scoped() == s2->scoped()) || (cl2 && s1->scoped() == cl2->scoped()))
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

        if(checkRename(dest, src))
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
FreezeScript::TransformVisitor::checkClasses(const Slice::ClassDeclPtr& dest, const Slice::ClassDeclPtr& src)
{
    //
    // Here are the rules for verifying class compatibility:
    //
    // 1. If the type ids are the same, assume they are compatible.
    // 2. If the source type has been renamed, then check its equivalent new definition for compatibility.
    // 3. Otherwise, the types are only compatible if they are defined in the same Slice unit, and if the
    //    destination type is a base type of the source type.
    //
    string s1 = dest->scoped();
    string s2 = src->scoped();
    if(s1 == s2)
    {
        return true;
    }
    else
    {
        Slice::TypePtr t = _info->getRenamedType(src);
        Slice::ClassDeclPtr s = Slice::ClassDeclPtr::dynamicCast(t);
        if(s)
        {
            return checkClasses(dest, s);
        }

        if(dest->unit().get() != src->unit().get())
        {
            Slice::TypeList l = dest->unit()->lookupTypeNoBuiltin(s2, false);
            if(l.empty())
            {
                _info->getErrorReporter()->error("class " + s2 + " not found in new Slice definitions");
            }
            s = Slice::ClassDeclPtr::dynamicCast(l.front());
        }
        else
        {
            s = src;
        }

        if(s)
        {
            Slice::ClassDefPtr def = s->definition();
            if(!def)
            {
                _info->getErrorReporter()->error("class " + s2 + " declared but not defined");
            }
            return def->isA(s1);
        }
    }

    return false;
}

void
FreezeScript::TransformVisitor::typeMismatchError(const Slice::TypePtr& dest, const Slice::TypePtr& src)
{
    ostringstream ostr;
    ostr << "unable to transform";
    if(!_context.empty())
    {
        ostr << ' ' << _context;
    }
    ostr << " from " << typeToString(src) << " to " << typeToString(dest);
    warning(ostr.str());
}

void
FreezeScript::TransformVisitor::conversionError(const Slice::TypePtr& dest, const Slice::TypePtr& src,
                                                const string& value)
{
    ostringstream ostr;
    ostr << "unable to convert";
    if(!_context.empty())
    {
        ostr << ' ' << _context;
    }
    ostr << " `" << value << "' from " << typeToString(src) << " to " << typeToString(dest);
    warning(ostr.str());
}

void
FreezeScript::TransformVisitor::rangeError(const string& value, const Slice::TypePtr& type)
{
    ostringstream ostr;
    if(!_context.empty())
    {
        ostr << _context << ' ';
    }
    ostr << "`" << value << "' is out of range for type " << typeToString(type);
    warning(ostr.str());
}

void
FreezeScript::TransformVisitor::warning(const string& msg)
{
    _info->getErrorReporter()->warning(msg);
}
