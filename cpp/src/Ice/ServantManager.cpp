// Copyright (c) ZeroC, Inc.

#include "ServantManager.h"
#include "Ice/InputStream.h"
#include "Ice/LocalExceptions.h"
#include "Ice/LoggerUtil.h"
#include "Ice/ServantLocator.h"
#include "Ice/StringUtil.h"
#include "Instance.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

Ice::ServantLocator::~ServantLocator() = default; // avoid weak vtable

void
IceInternal::ServantManager::addServant(ObjectPtr object, Identity ident, string facet)
{
    lock_guard lock(_mutex);

    assert(_instance); // Must not be called after destruction.

    auto p = _servantMapMapHint;

    if (p == _servantMapMap.end() || p->first != ident)
    {
        p = _servantMapMap.find(ident);
    }

    if (p == _servantMapMap.end())
    {
        p = _servantMapMap.insert(_servantMapMapHint, pair<const Identity, FacetMap>(std::move(ident), FacetMap()));
    }
    else
    {
        if (p->second.find(facet) != p->second.end())
        {
            ToStringMode toStringMode = _instance->toStringMode();
            ostringstream os;
            os << Ice::identityToString(ident, toStringMode);
            if (!facet.empty())
            {
                os << " -f " << escapeString(facet, "", toStringMode);
            }
            throw AlreadyRegisteredException(__FILE__, __LINE__, "servant", os.str());
        }
    }

    _servantMapMapHint = p;

    p->second.insert(pair<const string, ObjectPtr>(std::move(facet), std::move(object)));
}

void
IceInternal::ServantManager::addDefaultServant(ObjectPtr object, string category)
{
    lock_guard lock(_mutex);

    assert(_instance); // Must not be called after destruction.

    auto p = _defaultServantMap.find(category);
    if (p != _defaultServantMap.end())
    {
        throw AlreadyRegisteredException(__FILE__, __LINE__, "default servant", category);
    }

    _defaultServantMap.insert(pair<const string, ObjectPtr>(std::move(category), std::move(object)));
}

ObjectPtr
IceInternal::ServantManager::removeServant(const Identity& ident, const string_view facet)
{
    // We return the removed servant to avoid releasing the last reference count
    // with *this locked. We don't want to run user code, such as the servant
    // destructor, with an internal Ice mutex locked.

    lock_guard lock(_mutex);

    assert(_instance); // Must not be called after destruction.

    auto p = _servantMapMapHint;
    FacetMap::iterator q;

    if (p == _servantMapMap.end() || p->first != ident)
    {
        p = _servantMapMap.find(ident);
    }

    if (p == _servantMapMap.end() || (q = p->second.find(facet)) == p->second.end())
    {
        ToStringMode toStringMode = _instance->toStringMode();
        ostringstream os;
        os << Ice::identityToString(ident, toStringMode);
        if (!facet.empty())
        {
            os << " -f " + escapeString(facet, "", toStringMode);
        }
        throw NotRegisteredException(__FILE__, __LINE__, "servant", os.str());
    }

    ObjectPtr servant = q->second;
    p->second.erase(q);

    if (p->second.empty())
    {
        if (p == _servantMapMapHint)
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
IceInternal::ServantManager::removeDefaultServant(const string_view category)
{
    // We return the removed servant to avoid releasing the last reference count
    // with *this locked. We don't want to run user code, such as the servant
    // destructor, with an internal Ice mutex locked.

    lock_guard lock(_mutex);

    assert(_instance); // Must not be called after destruction.

    auto p = _defaultServantMap.find(category);
    if (p == _defaultServantMap.end())
    {
        throw NotRegisteredException(__FILE__, __LINE__, "default servant", string{category});
    }

    ObjectPtr servant = p->second;
    _defaultServantMap.erase(p);

    return servant;
}

FacetMap
IceInternal::ServantManager::removeAllFacets(const Identity& ident)
{
    lock_guard lock(_mutex);

    assert(_instance); // Must not be called after destruction.

    auto p = _servantMapMapHint;

    if (p == _servantMapMap.end() || p->first != ident)
    {
        p = _servantMapMap.find(ident);
    }

    if (p == _servantMapMap.end())
    {
        throw NotRegisteredException(
            __FILE__,
            __LINE__,
            "servant",
            Ice::identityToString(ident, _instance->toStringMode()));
    }

    FacetMap result = p->second;

    if (p == _servantMapMapHint)
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
IceInternal::ServantManager::findServant(const Identity& ident, const string_view facet) const
{
    lock_guard lock(_mutex);

    //
    // This assert is not valid if the adapter dispatch incoming
    // requests from bidir connections. This method might be called if
    // requests are received over the bidir connection after the
    // adapter was deactivated.
    //
    // assert(_instance); // Must not be called after destruction.

    auto p = _servantMapMapHint;
    FacetMap::iterator q;

    auto& servantMapMap = const_cast<ServantMapMap&>(_servantMapMap);

    if (p == servantMapMap.end() || p->first != ident)
    {
        p = servantMapMap.find(ident);
    }

    if (p == servantMapMap.end() || (q = p->second.find(facet)) == p->second.end())
    {
        auto d = _defaultServantMap.find(ident.category);
        if (d == _defaultServantMap.end())
        {
            d = _defaultServantMap.find("");
            if (d == _defaultServantMap.end())
            {
                return nullptr;
            }
            else
            {
                return d->second;
            }
        }
        else
        {
            return d->second;
        }
    }
    else
    {
        _servantMapMapHint = p;
        return q->second;
    }
}

ObjectPtr
IceInternal::ServantManager::findDefaultServant(const string_view category) const
{
    lock_guard lock(_mutex);

    auto p = _defaultServantMap.find(category);
    if (p == _defaultServantMap.end())
    {
        return nullptr;
    }
    else
    {
        return p->second;
    }
}

FacetMap
IceInternal::ServantManager::findAllFacets(const Identity& ident) const
{
    lock_guard lock(_mutex);

    assert(_instance); // Must not be called after destruction.

    auto p = _servantMapMapHint;

    auto& servantMapMap = const_cast<ServantMapMap&>(_servantMapMap);

    if (p == servantMapMap.end() || p->first != ident)
    {
        p = servantMapMap.find(ident);
    }

    if (p == servantMapMap.end())
    {
        return {};
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
    lock_guard lock(_mutex);

    //
    // This assert is not valid if the adapter dispatch incoming
    // requests from bidir connections. This method might be called if
    // requests are received over the bidir connection after the
    // adapter was deactivated.
    //
    // assert(_instance); // Must not be called after destruction.

    auto p = _servantMapMapHint;
    auto& servantMapMap = const_cast<ServantMapMap&>(_servantMapMap);

    if (p == servantMapMap.end() || p->first != ident)
    {
        p = servantMapMap.find(ident);
    }

    if (p == servantMapMap.end())
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
IceInternal::ServantManager::addServantLocator(ServantLocatorPtr locator, string category)
{
    lock_guard lock(_mutex);

    assert(_instance); // Must not be called after destruction.

    if ((_locatorMapHint != _locatorMap.end() && _locatorMapHint->first == category) ||
        _locatorMap.find(category) != _locatorMap.end())
    {
        throw AlreadyRegisteredException(__FILE__, __LINE__, "servant locator", category);
    }

    _locatorMapHint = _locatorMap.insert(
        _locatorMapHint,
        pair<const string, ServantLocatorPtr>(std::move(category), std::move(locator)));
}

ServantLocatorPtr
IceInternal::ServantManager::removeServantLocator(const string_view category)
{
    lock_guard lock(_mutex);

    assert(_instance); // Must not be called after destruction.

    auto p = _locatorMap.end();
    if (_locatorMapHint != p)
    {
        if (_locatorMapHint->first == category)
        {
            p = _locatorMapHint;
        }
    }

    if (p == _locatorMap.end())
    {
        p = _locatorMap.find(category);
    }

    if (p == _locatorMap.end())
    {
        throw NotRegisteredException(__FILE__, __LINE__, "servant locator", string{category});
    }

    ServantLocatorPtr locator = p->second;
    _locatorMap.erase(p);
    _locatorMapHint = _locatorMap.begin();
    return locator;
}

ServantLocatorPtr
IceInternal::ServantManager::findServantLocator(const string_view category) const
{
    lock_guard lock(_mutex);

    //
    // This assert is not valid if the adapter dispatch incoming
    // requests from bidir connections. This method might be called if
    // requests are received over the bidir connection after the
    // adapter was deactivated.
    //
    // assert(_instance); // Must not be called after destruction.

    auto& locatorMap = const_cast<map<string, ServantLocatorPtr, std::less<>>&>(_locatorMap);

    auto p = locatorMap.end();
    if (_locatorMapHint != locatorMap.end())
    {
        if (_locatorMapHint->first == category)
        {
            p = _locatorMapHint;
        }
    }

    if (p == locatorMap.end())
    {
        p = locatorMap.find(category);
    }

    if (p != locatorMap.end())
    {
        _locatorMapHint = p;
        return p->second;
    }
    else
    {
        return nullptr;
    }
}

IceInternal::ServantManager::ServantManager(InstancePtr instance, string adapterName)
    : _instance(std::move(instance)),
      _adapterName(std::move(adapterName)),
      _servantMapMapHint(_servantMapMap.end()),
      _locatorMapHint(_locatorMap.end())
{
}

IceInternal::ServantManager::~ServantManager() = default;

void
IceInternal::ServantManager::destroy()
{
    ServantMapMap servantMapMap;
    DefaultServantMap defaultServantMap;
    map<string, ServantLocatorPtr, std::less<>> locatorMap;
    Ice::LoggerPtr logger;

    {
        lock_guard lock(_mutex);
        //
        // If the ServantManager has already been destroyed, we're done.
        //
        if (!_instance)
        {
            return;
        }

        logger = _instance->initializationData().logger;

        servantMapMap.swap(_servantMapMap);
        _servantMapMapHint = _servantMapMap.end();

        defaultServantMap.swap(_defaultServantMap);

        locatorMap.swap(_locatorMap);
        _locatorMapHint = _locatorMap.end();

        _instance = nullptr;
    }

    for (const auto& p : locatorMap)
    {
        try
        {
            p.second->deactivate(p.first);
        }
        catch (const Exception& ex)
        {
            Error out(logger);
            out << "exception during locator deactivation:\n"
                << "object adapter: '" << _adapterName << "'\n"
                << "locator category: '" << p.first << "'\n"
                << ex;
        }
        catch (...)
        {
            Error out(logger);
            out << "unknown exception during locator deactivation:\n"
                << "object adapter: '" << _adapterName << "'\n"
                << "locator category: '" << p.first << "'";
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

void
ServantManager::dispatch(IncomingRequest& request, function<void(OutgoingResponse)> sendResponse)
{
    const Current& current = request.current();
    ObjectPtr servant = findServant(current.id, current.facet);

    if (servant)
    {
        // the simple, common path.
        servant->dispatch(request, std::move(sendResponse));
        return;
    }

    // Else, check servant locators
    ServantLocatorPtr locator = findServantLocator(current.id.category);
    if (!locator && !current.id.category.empty())
    {
        locator = findServantLocator("");
    }

    if (locator)
    {
        shared_ptr<void> cookie;
        try
        {
            servant = locator->locate(current, cookie);
        }
        catch (...)
        {
            // Skip the encapsulation. This allows the next batch requests in the same InputStream to proceed.
            request.inputStream().skipEncapsulation();
            throw;
        }

        if (servant)
        {
            try
            {
                servant->dispatch(
                    request,
                    [sendResponse = std::move(sendResponse), locator, servant, cookie](OutgoingResponse response)
                    {
                        try
                        {
                            locator->finished(response.current(), servant, cookie);
                        }
                        catch (...)
                        {
                            sendResponse(makeOutgoingResponse(std::current_exception(), response.current()));
                            return; // done
                        }
                        sendResponse(std::move(response));
                    });
                return; // done
            }
            catch (...)
            {
                // When we catch an exception, the dispatch logic guarantees sendResponse was not called.
                locator->finished(current, servant, cookie);
                throw;
            }
        }
    }

    assert(!servant);

    // Skip the encapsulation. This allows the next batch requests in the same InputStream to proceed.
    request.inputStream().skipEncapsulation();
    if (hasServant(current.id))
    {
        throw FacetNotExistException{__FILE__, __LINE__};
    }
    else
    {
        throw ObjectNotExistException{__FILE__, __LINE__};
    }
}
