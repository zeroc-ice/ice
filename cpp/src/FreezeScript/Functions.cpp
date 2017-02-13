// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <FreezeScript/Functions.h>
#include <FreezeScript/Util.h>
#include <IceUtil/UUID.h>
#include <IceUtil/StringUtil.h>

using namespace std;

bool
FreezeScript::invokeGlobalFunction(const Ice::CommunicatorPtr& communicator, const string& name, const DataList& args,
                                   DataPtr& result, const DataFactoryPtr& factory, 
                                   const ErrorReporterPtr& errorReporter)
{
    //
    // Global function.
    //
    if(name == "typeOf")
    {
        if(args.size() != 1)
        {
            errorReporter->error("typeOf() requires one argument");
        }
        result = factory->createString(typeToString(args.front()->getType()), false);
        return true;
    }
    else if(name == "generateUUID")
    {
        if(args.size() != 0)
        {
            errorReporter->error("generateUUID() accepts no arguments");
        }
        result = factory->createString(IceUtil::generateUUID(), false);
        return true;
    }
    else if(name == "stringToIdentity")
    {
        StringDataPtr str;
        if(args.size() > 0)
        {
            str = StringDataPtr::dynamicCast(args.front());
        }
        if(args.size() != 1 || !str)
        {
            errorReporter->error("stringToIdentity() requires a string argument");
        }

        //
        // Parse the identity string.
        //
        string idstr = str->stringValue();
        Ice::Identity id;
        try
        {
            id = communicator->stringToIdentity(idstr);
        }
        catch(const Ice::IdentityParseException& ex)
        {
            errorReporter->error("error in stringToIdentity():\n" + ex.str);
        }

        //
        // Create a data representation of Ice::Identity.
        //
        Slice::UnitPtr unit = str->getType()->unit();
        Slice::TypeList l = unit->lookupType("::Ice::Identity", false);
        assert(!l.empty());
        DataPtr identity = factory->create(l.front(), false);
        StringDataPtr member;
        member = StringDataPtr::dynamicCast(identity->getMember("name"));
        assert(member);
        member->setValue(id.name);
        member = StringDataPtr::dynamicCast(identity->getMember("category"));
        assert(member);
        member->setValue(id.category);
        result = identity;
        return true;
    }
    else if(name == "identityToString")
    {
        StructDataPtr identity;
        if(args.size() > 0)
        {
            identity = StructDataPtr::dynamicCast(args.front());
        }
        if(identity)
        {
            Slice::TypePtr argType = identity->getType();
            Slice::StructPtr st = Slice::StructPtr::dynamicCast(argType);
            if(!st || st->scoped() != "::Ice::Identity")
            {
                identity = 0;
            }
        }
        if(args.size() != 1 || !identity)
        {
            errorReporter->error("identityToString() requires a argument of type ::Ice::Identity");
        }

        //
        // Compose the identity.
        //
        Ice::Identity id;
        StringDataPtr member;
        member = StringDataPtr::dynamicCast(identity->getMember("name"));
        assert(member);
        id.name = member->stringValue();
        member = StringDataPtr::dynamicCast(identity->getMember("category"));
        assert(member);
        id.category = member->stringValue();

        result = factory->createString(communicator->identityToString(id), false);
        return true;
    }
    else if(name == "stringToProxy")
    {
        StringDataPtr str;
        if(args.size() > 0)
        {
            str = StringDataPtr::dynamicCast(args.front());
        }
        if(args.size() != 1 || !str)
        {
            errorReporter->error("stringToProxy() requires a string argument");
        }

        //
        // Parse the proxy;
        //
        string sprx = str->stringValue();
        Ice::ObjectPrx prx;
        try
        {
            prx = factory->getCommunicator()->stringToProxy(sprx);
        }
        catch(const Ice::ProxyParseException& ex)
        {
            errorReporter->error("error in stringToProxy():\n" + ex.str);
        }

        Slice::UnitPtr unit = str->getType()->unit();
        ProxyDataPtr p =
            ProxyDataPtr::dynamicCast(factory->create(unit->builtin(Slice::Builtin::KindObjectProxy), false));
        p->setValue(prx);
        result = p;
        return true;
    }
    else if(name == "proxyToString")
    {
        ProxyDataPtr prx;
        if(args.size() > 0)
        {
            prx = ProxyDataPtr::dynamicCast(args.front());
        }
        if(args.size() != 1 || !prx)
        {
            errorReporter->error("proxyToString() requires a proxy argument");
        }

        result = factory->createString(prx->toString(), false);
        return true;
    }
    else if(name == "lowercase")
    {
        StringDataPtr str;
        if(args.size() > 0)
        {
            str = StringDataPtr::dynamicCast(args.front());
        }
        if(args.size() != 1 || !str)
        {
            errorReporter->error("lowercase() requires a string argument");
        }
        string val = IceUtilInternal::toLower(str->stringValue());
        result = factory->createString(val, false);
        return true;
    }

    return false;
}

bool
FreezeScript::invokeMemberFunction(const string& name, const DataPtr& target, const DataList& args, DataPtr& result,
                                   const DataFactoryPtr& factory, const ErrorReporterPtr& errorReporter)
{
    //
    // string
    //
    StringDataPtr targetStr = StringDataPtr::dynamicCast(target);
    if(targetStr)
    {
        if(name == "find")
        {
            StringDataPtr argData;
            IntegerDataPtr startData;
            if(args.size() > 0)
            {
                argData = StringDataPtr::dynamicCast(args[0]);
            }
            if(args.size() > 1)
            {
                startData = IntegerDataPtr::dynamicCast(args[1]);
            }
            if(args.size() == 0 || args.size() > 2 || !argData || (args.size() == 2 && !startData))
            {
                errorReporter->error("invalid arguments to find(string str[, int len])");
            }
            string targ = targetStr->stringValue();
            string arg = argData->stringValue();
            string::size_type pos;
            if(startData)
            {
                string::size_type start = static_cast<string::size_type>(startData->integerValue());
                pos = targ.find(arg, start);
            }
            else
            {
                pos = targ.find(arg);
            }
            result = factory->createInteger(pos == string::npos ? -1 : static_cast<Ice::Long>(pos), false);
            return true;
        }
        else if(name == "substr")
        {
            IntegerDataPtr startData;
            IntegerDataPtr lenData;
            if(args.size() > 0)
            {
                startData = IntegerDataPtr::dynamicCast(args[0]);
            }
            if(args.size() > 1)
            {
                lenData = IntegerDataPtr::dynamicCast(args[1]);
            }
            if(args.size() == 0 || args.size() > 2 || !startData || (args.size() == 2 && !lenData))
            {
                errorReporter->error("invalid arguments to substr(int start[, int len])");
            }
            string targ = targetStr->stringValue();
            string::size_type start = static_cast<string::size_type>(startData->integerValue());
            string::size_type len = string::npos;
            if(lenData)
            {
                len = static_cast<string::size_type>(lenData->integerValue());
            }
            if(start > targ.size())
            {
                ostringstream ostr;
                ostr << "substr() starting position (" << start << ") is greater than string length ("
                     << targ.size() << ")";
                errorReporter->error(ostr.str());
            }
            result = factory->createString(targ.substr(start, len), false);
            return true;
        }
        else if(name == "replace")
        {
            IntegerDataPtr startData;
            IntegerDataPtr lenData;
            StringDataPtr strData;
            if(args.size() == 3)
            {
                startData = IntegerDataPtr::dynamicCast(args[0]);
                lenData = IntegerDataPtr::dynamicCast(args[1]);
                strData = StringDataPtr::dynamicCast(args[2]);
            }
            if(args.size() != 3 || !startData || !lenData || !strData)
            {
                errorReporter->error("invalid arguments to replace(int start, int len, string val)");
            }
            string targ = targetStr->stringValue();
            string::size_type start = static_cast<string::size_type>(startData->integerValue());
            string::size_type len = static_cast<string::size_type>(lenData->integerValue());
            string str = strData->stringValue();
            if(start > targ.size())
            {
                ostringstream ostr;
                ostr << "replace() starting position (" << start << ") is greater than string length ("
                     << targ.size() << ")";
                errorReporter->error(ostr.str());
            }
            result = factory->createString(targ.replace(start, len, str), false);
            return true;
        }

        return false;
    }

    //
    // Object
    //
    ObjectRefPtr targetObj = ObjectRefPtr::dynamicCast(target);
    if(targetObj)
    {
        if(name == "ice_isA")
        {
            ObjectDataPtr value = targetObj->getValue();
            if(!value)
            {
                errorReporter->error("ice_isA() invoked on nil object");
            }
            StringDataPtr str;
            if(args.size() > 0)
            {
                str = StringDataPtr::dynamicCast(args.front());
            }
            if(args.size() != 1 || !str)
            {
                errorReporter->error("invalid arguments to ice_isA(string id)");
            }

            string id = str->stringValue();
            if(id == "::Ice::Object")
            {
                result = factory->createBoolean(true, false);
                return true;
            }

            Slice::ClassDeclPtr decl = Slice::ClassDeclPtr::dynamicCast(value->getType());
            if(!decl)
            {
                //
                // Ice::Object
                //
                result = factory->createBoolean(false, false);
                return true;
            }

            Slice::ClassDefPtr def = decl->definition();
            assert(def);
            result = factory->createBoolean(def->isA(id), false);
            return true;
        }

        return false;
    }

    return false;
}
