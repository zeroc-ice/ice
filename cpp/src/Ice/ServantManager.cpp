// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/ServantManager.h>
#include <Ice/ServantLocator.h>
#include <Ice/LocalException.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Instance.h>
#include <IceUtil/StringUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

ICE_API IceUtil::Shared* IceInternal::upCast(ServantManager* p) { return p; }

void
IceInternal::ServantManager::addServant(const ObjectPtr& object, const Identity& ident, const string& facet)
{
    IceUtil::Mutex::Lock sync(*this);

    assert(_instance); // Must not be called after destruction.

    ServantMapMap::iterator p = _servantMapMapHint;

    if(p == _servantMapMap.end() || p->first != ident)
    {
        p = _servantMapMap.find(ident);
    }

    if(p == _servantMapMap.end())
    {
        p = _servantMapMap.insert(_servantMapMapHint, pair<const Identity, FacetMap>(ident, FacetMap()));
    }
    else
    {
        if(p->second.find(facet) != p->second.end())
        {
            AlreadyRegisteredException ex(__FILE__, __LINE__);
            ex.kindOfObject = "servant";
            ex.id = _instance->identityToString(ident);
            if(!facet.empty())
            {
                string fs = nativeToUTF8(facet, _instance->getStringConverter());
                ex.id += " -f " + IceUtilInternal::escapeString(fs, "");
            }
            throw ex;
        }
    }

    _servantMapMapHint = p;

    p->second.insert(pair<const string, ObjectPtr>(facet, object));
}

void
IceInternal::ServantManager::addDefaultServant(const ObjectPtr& object, const string& category)
{
    IceUtil::Mutex::Lock sync(*this);

    assert(_instance); // Must not be called after destruction.

    DefaultServantMap::iterator p = _defaultServantMap.find(category);
    if(p != _defaultServantMap.end())
    {
        AlreadyRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "default servant";
        ex.id = category;
        throw ex;
    }

    _defaultServantMap.insert(pair<const string, ObjectPtr>(category, object));
}

ObjectPtr
IceInternal::ServantManager::removeServant(const Identity& ident, const string& facet)
{
    //
    // We return the removed servant to avoid releasing the last reference count
    // with *this locked. We don't want to run user code, such as the servant
    // destructor, with an internal Ice mutex locked.
    //
    ObjectPtr servant = 0;

    IceUtil::Mutex::Lock sync(*this);

    assert(_instance); // Must not be called after destruction.

    ServantMapMap::iterator p = _servantMapMapHint;
    FacetMap::iterator q;

    if(p == _servantMapMap.end() || p->first != ident)
    {
        p = _servantMapMap.find(ident);
    }

    if(p == _servantMapMap.end() || (q = p->second.find(facet)) == p->second.end())
    {
        NotRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "servant";
        ex.id = _instance->identityToString(ident);
        if(!facet.empty())
        {
            string fs = nativeToUTF8(facet, _instance->getStringConverter());
            ex.id += " -f " + IceUtilInternal::escapeString(fs, "");
        }
        throw ex;
    }

    servant = q->second;
    p->second.erase(q);

    if(p->second.empty())
    {
        if(p == _servantMapMapHint)
        {
            _servantMapMap.erase(p++);
            _servantMapMapHint = p;
        }
        else
        {
            _servantMapMap.erase(p);
        }
    }
    return servant;
}

ObjectPtr
IceInternal::ServantManager::removeDefaultServant(const string& category)
{
    //
    // We return the removed servant to avoid releasing the last reference count
    // with *this locked. We don't want to run user code, such as the servant
    // destructor, with an internal Ice mutex locked.
    //
    ObjectPtr servant = 0;

    IceUtil::Mutex::Lock sync(*this);

    assert(_instance); // Must not be called after destruction.

    DefaultServantMap::iterator p = _defaultServantMap.find(category);
    if(p == _defaultServantMap.end())
    {
        NotRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "default servant";
        ex.id = category;
        throw ex;
    }

    servant = p->second;
    _defaultServantMap.erase(p);

    return servant;
}

FacetMap
IceInternal::ServantManager::removeAllFacets(const Identity& ident)
{
    IceUtil::Mutex::Lock sync(*this);

    assert(_instance); // Must not be called after destruction.

    ServantMapMap::iterator p = _servantMapMapHint;

    if(p == _servantMapMap.end() || p->first != ident)
    {
        p = _servantMapMap.find(ident);
    }

    if(p == _servantMapMap.end())
    {
        NotRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "servant";
        ex.id = _instance->identityToString(ident);
        throw ex;
    }

    FacetMap result = p->second;

    if(p == _servantMapMapHint)
    {
        _servantMapMap.erase(p++);
        _servantMapMapHint = p;
    }
    else
    {
        _servantMapMap.erase(p);
    }

    return result;
}

ObjectPtr
IceInternal::ServantManager::findServant(const Identity& ident, const string& facet) const
{
    IceUtil::Mutex::Lock sync(*this);

    //
    // This assert is not valid if the adapter dispatch incoming
    // requests from bidir connections. This method might be called if
    // requests are received over the bidir connection after the
    // adapter was deactivated.
    //
    //assert(_instance); // Must not be called after destruction.

    ServantMapMap::iterator p = _servantMapMapHint;
    FacetMap::iterator q;

    ServantMapMap& servantMapMap = const_cast<ServantMapMap&>(_servantMapMap);

    if(p == servantMapMap.end() || p->first != ident)
    {
        p = servantMapMap.find(ident);
    }

    if(p == servantMapMap.end() || (q = p->second.find(facet)) == p->second.end())
    {
        DefaultServantMap::const_iterator p = _defaultServantMap.find(ident.category);
        if(p == _defaultServantMap.end())
        {
            p = _defaultServantMap.find("");
            if(p == _defaultServantMap.end())
            {
                return 0;
            }
            else
            {
                return p->second;
            }
        }
        else
        {
            return p->second;
        }
    }
    else
    {
        _servantMapMapHint = p;
        return q->second;
    }
}

ObjectPtr
IceInternal::ServantManager::findDefaultServant(const string& category) const
{
    IceUtil::Mutex::Lock sync(*this);

    DefaultServantMap::const_iterator p = _defaultServantMap.find(category);
    if(p == _defaultServantMap.end())
    {
        return 0;
    }
    else
    {
        return p->second;
    }
}

FacetMap
IceInternal::ServantManager::findAllFacets(const Identity& ident) const
{
    IceUtil::Mutex::Lock sync(*this);

    assert(_instance); // Must not be called after destruction.

    ServantMapMap::iterator p = _servantMapMapHint;

    ServantMapMap& servantMapMap = const_cast<ServantMapMap&>(_servantMapMap);

    if(p == servantMapMap.end() || p->first != ident)
    {
        p = servantMapMap.find(ident);
    }

    if(p == servantMapMap.end())
    {
        return FacetMap();
    }
    else
    {
        _servantMapMapHint = p;
        return p->second;
    }
}

bool
IceInternal::ServantManager::hasServant(const Identity& ident) const
{
    IceUtil::Mutex::Lock sync(*this);

    //
    // This assert is not valid if the adapter dispatch incoming
    // requests from bidir connections. This method might be called if
    // requests are received over the bidir connection after the
    // adapter was deactivated.
    //
    //assert(_instance); // Must not be called after destruction.

    ServantMapMap::iterator p = _servantMapMapHint;
    ServantMapMap& servantMapMap = const_cast<ServantMapMap&>(_servantMapMap);

    if(p == servantMapMap.end() || p->first != ident)
    {
        p = servantMapMap.find(ident);
    }

    if(p == servantMapMap.end())
    {
        return false;
    }
    else
    {
        _servantMapMapHint = p;
        assert(!p->second.empty());
        return true;
    }
}

void
IceInternal::ServantManager::addServantLocator(const ServantLocatorPtr& locator, const string& category)
{
    IceUtil::Mutex::Lock sync(*this);

    assert(_instance); // Must not be called after destruction.

    if((_locatorMapHint != _locatorMap.end() && _locatorMapHint->first == category)
       || _locatorMap.find(category) != _locatorMap.end())
    {
        AlreadyRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "servant locator";
        ex.id = category;
        throw ex;
    }

    _locatorMapHint = _locatorMap.insert(_locatorMapHint, pair<const string, ServantLocatorPtr>(category, locator));
}

ServantLocatorPtr
IceInternal::ServantManager::removeServantLocator(const string& category)
{
    IceUtil::Mutex::Lock sync(*this);

    assert(_instance); // Must not be called after destruction.

    map<string, ServantLocatorPtr>::iterator p = _locatorMap.end();
    if(_locatorMapHint != p)
    {
        if(_locatorMapHint->first == category)
        {
            p = _locatorMapHint;
        }
    }

    if(p == _locatorMap.end())
    {
        p = _locatorMap.find(category);
    }

    if(p == _locatorMap.end())
    {
        NotRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "servant locator";
        ex.id = category;
        throw ex;
    }

    ServantLocatorPtr locator = p->second;
    _locatorMap.erase(p);
    _locatorMapHint = _locatorMap.begin();
    return locator;
}

ServantLocatorPtr
IceInternal::ServantManager::findServantLocator(const string& category) const
{
    IceUtil::Mutex::Lock sync(*this);

    //
    // This assert is not valid if the adapter dispatch incoming
    // requests from bidir connections. This method might be called if
    // requests are received over the bidir connection after the
    // adapter was deactivated.
    //
    //assert(_instance); // Must not be called after destruction.

    map<string, ServantLocatorPtr>& locatorMap =
        const_cast<map<string, ServantLocatorPtr>&>(_locatorMap);

    map<string, ServantLocatorPtr>::iterator p = locatorMap.end();
    if(_locatorMapHint != locatorMap.end())
    {
        if(_locatorMapHint->first == category)
        {
            p = _locatorMapHint;
        }
    }

    if(p == locatorMap.end())
    {
        p = locatorMap.find(category);
    }

    if(p != locatorMap.end())
    {
        _locatorMapHint = p;
        return p->second;
    }
    else
    {
        return 0;
    }
}

IceInternal::ServantManager::ServantManager(const InstancePtr& instance, const string& adapterName)
    : _instance(instance),
      _adapterName(adapterName),
      _servantMapMapHint(_servantMapMap.end()),
      _locatorMapHint(_locatorMap.end())
{
}

IceInternal::ServantManager::~ServantManager()
{
    //
    // Don't check whether destroy() has been called. It might have
    // not been called if the associated object adapter was not
    // properly deactivated.
    //
    //assert(!_instance);
}

void
IceInternal::ServantManager::destroy()
{
    ServantMapMap servantMapMap;
    DefaultServantMap defaultServantMap;
    map<string, ServantLocatorPtr> locatorMap;
    Ice::LoggerPtr logger;

    {
        IceUtil::Mutex::Lock sync(*this);
        //
        // If the ServantManager has already been destroyed, we're done.
        //
        if(!_instance)
        {
            return;
        }

        logger = _instance->initializationData().logger;

        servantMapMap.swap(_servantMapMap);
        _servantMapMapHint = _servantMapMap.end();

        defaultServantMap.swap(_defaultServantMap);

        locatorMap.swap(_locatorMap);
        _locatorMapHint = _locatorMap.end();

        _instance = 0;
    }

    for(map<string, ServantLocatorPtr>::const_iterator p = locatorMap.begin(); p != locatorMap.end(); ++p)
    {
        try
        {
            p->second->deactivate(p->first);
        }
        catch(const Exception& ex)
        {
            Error out(logger);
            out << "exception during locator deactivation:\n"
                << "object adapter: `" << _adapterName << "'\n"
                << "locator category: `" << p->first << "'\n"
                << ex;
        }
        catch(...)
        {
            Error out(logger);
            out << "unknown exception during locator deactivation:\n"
                << "object adapter: `" << _adapterName << "'\n"
                << "locator category: `" << p->first << "'";
        }
    }

    //
    // We clear the maps outside the synchronization as we don't want to
    // hold any internal Ice mutex while running user code (such as servant
    // or servant locator destructors).
    //
    servantMapMap.clear();
    locatorMap.clear();
    defaultServantMap.clear();
}
