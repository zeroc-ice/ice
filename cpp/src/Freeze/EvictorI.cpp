// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Freeze/EvictorI.h>
#include <Freeze/Initialize.h>
#include <Freeze/Util.h>
#include <Freeze/EvictorIteratorI.h>
#include <Freeze/PingObject.h>

#include <IceUtil/IceUtil.h>

#include <IceUtil/StringConverter.h>

#include <typeinfo>

using namespace std;
using namespace Freeze;
using namespace Ice;
using namespace IceUtil;

//
// Static members
//

string Freeze::EvictorIBase::defaultDb = "$default";
string Freeze::EvictorIBase::indexPrefix = "$index:";


//
// DeactivateController
//

Freeze::DeactivateController::Guard::Guard(const DeactivateController& controller) :
    _controller(const_cast<DeactivateController&>(controller))
{
    Lock sync(controller);
    if(controller._deactivated || _controller._deactivating)
    {
        throw EvictorDeactivatedException(__FILE__, __LINE__);
    }
    _controller._guardCount++;
}

Freeze::DeactivateController::Guard::~Guard()
{
    Lock sync(_controller);
    _controller._guardCount--;
    if(_controller._deactivating && _controller._guardCount == 0)
    {
        //
        // Notify all the threads -- although we only want to
        // reach the thread doing the deactivation.
        //
        _controller.notifyAll();
    }
}

Freeze::DeactivateController::DeactivateController(EvictorIBase* evictor) :
    _evictor(evictor),
    _deactivating(false),
    _deactivated(false),
    _guardCount(0)
{
}

bool
Freeze::DeactivateController::deactivated() const
{
    Lock sync(*this);
    return _deactivated || _deactivating;
}

bool
Freeze::DeactivateController::deactivate()
{
    Lock sync(*this);

    if(_deactivated)
    {
        return false;
    }

    if(_deactivating)
    {
        //
        // Wait for deactivated
        //
        while(!_deactivated)
        {
            wait();
        }
        return false;
    }
    else
    {
        _deactivating = true;
        while(_guardCount > 0)
        {
            if(_evictor->trace() >= 1)
            {
                Trace out(_evictor->communicator()->getLogger(), "Freeze.Evictor");
                out << "Waiting for " << _guardCount << " threads to complete before starting deactivation.";
            }

            wait();
        }

        if(_evictor->trace() >= 1)
        {
            Trace out(_evictor->communicator()->getLogger(), "Freeze.Evictor");
            out << "Starting deactivation.";
        }
        return true;
    }
}

void
Freeze::DeactivateController::deactivationComplete()
{
    if(_evictor->trace() >= 1)
    {
        Trace out(_evictor->communicator()->getLogger(), "Freeze.Evictor");
        out << "Deactivation complete.";
    }

    Lock sync(*this);
    _deactivated = true;
    _deactivating = false;
    notifyAll();
}


//
// EvictorIBase
//

Freeze::EvictorIBase::EvictorIBase(const ObjectAdapterPtr& adapter,
                                   const string& envName,
                                   DbEnv* dbEnv,
                                   const string& filename,
                                   const FacetTypeMap& facetTypes,
                                   const ServantInitializerPtr& initializer,
                                   bool createDb) :
    _evictorSize(10),
    _facetTypes(facetTypes),
    _deactivateController(this),
    _adapter(adapter),
    _communicator(adapter->getCommunicator()),
    _initializer(initializer),
    _dbEnv(SharedDbEnv::get(_communicator, envName, dbEnv)),
    _filename(filename),
    _createDb(createDb),
    _trace(0),
    _txTrace(0),
    _pingObject(new PingObject)
{
    _encoding = _dbEnv->getEncoding();

    _trace = _communicator->getProperties()->getPropertyAsInt("Freeze.Trace.Evictor");
    _txTrace = _communicator->getProperties()->getPropertyAsInt("Freeze.Trace.Transaction");
    _deadlockWarning = (_communicator->getProperties()->getPropertyAsInt("Freeze.Warn.Deadlocks") > 0);
}


void
Freeze::EvictorIBase::setSize(Int evictorSize)
{
    DeactivateController::Guard deactivateGuard(_deactivateController);

    Lock sync(*this);

    //
    // Ignore requests to set the evictor size to values smaller than zero.
    //
    if(evictorSize < 0)
    {
        return;
    }

    //
    // Update the evictor size.
    //
    _evictorSize = static_cast<size_t>(evictorSize);

    //
    // Evict as many elements as necessary.
    //
    evict();
}

Int
Freeze::EvictorIBase::getSize()
{
    Lock sync(*this);
    return static_cast<Int>(_evictorSize);
}


Ice::ObjectPrx
Freeze::EvictorIBase::add(const ObjectPtr& servant, const Identity& ident)
{
    return addFacet(servant, ident, "");
}

Ice::ObjectPtr
Freeze::EvictorIBase::remove(const Identity& ident)
{
    return removeFacet(ident, "");
}

bool
Freeze::EvictorIBase::hasObject(const Identity& ident)
{
    return hasFacet(ident, "");
}

ObjectPtr
Freeze::EvictorIBase::locate(const Current& current, LocalObjectPtr& cookie)
{
    //
    // Special ice_ping() handling
    //
    if(current.operation == "ice_ping")
    {
        if(hasFacet(current.id, current.facet))
        {
            if(_trace >= 3)
            {
                Trace out(_communicator->getLogger(), "Freeze.Evictor");
                out << "ice_ping found \"" << _communicator->identityToString(current.id)
                    << "\" with facet \"" << current.facet + "\"";
            }

            cookie = 0;
            return _pingObject;
        }
        else if(hasAnotherFacet(current.id, current.facet))
        {
            if(_trace >= 3)
            {
                Trace out(_communicator->getLogger(), "Freeze.Evictor");
                out << "ice_ping raises FacetNotExistException for \"" << _communicator->identityToString(current.id)
                    << "\" with facet \"" << current.facet + "\"";
            }
            throw FacetNotExistException(__FILE__, __LINE__);
        }
        else
        {
            if(_trace >= 3)
            {
                Trace out(_communicator->getLogger(), "Freeze.Evictor");
                out << "ice_ping will raise ObjectNotExistException for \""
                    << _communicator->identityToString(current.id)  << "\" with facet \"" << current.facet + "\"";
            }
            return 0;
        }
    }

    ObjectPtr result = locateImpl(current, cookie);

    if(result == 0)
    {
        if(hasAnotherFacet(current.id, current.facet))
        {
            throw FacetNotExistException(__FILE__, __LINE__);
        }
    }
    return result;
}


void
Freeze::EvictorIBase::initialize(const Identity& ident, const string& facet, const ObjectPtr& servant)
{
    if(_initializer != 0)
    {
        _initializer->initialize(_adapter, ident, facet, servant);
    }
}

void
Freeze::EvictorIBase::updateStats(Statistics& stats, IceUtil::Int64 time)
{
    IceUtil::Int64 diff = time - (stats.creationTime + stats.lastSaveTime);
    if(stats.lastSaveTime == 0)
    {
        stats.lastSaveTime = diff;
        stats.avgSaveTime = diff;
    }
    else
    {
        stats.lastSaveTime = time - stats.creationTime;
        stats.avgSaveTime = static_cast<IceUtil::Int64>(stats.avgSaveTime * 0.95 + diff * 0.05);
    }
}


const string&
Freeze::EvictorIBase::filename() const
{
    return _filename;
}

vector<string>
Freeze::EvictorIBase::allDbs() const
{
    vector<string> result;

    try
    {
        Db db(_dbEnv->getEnv(), 0);

        //
        // Berkeley DB expects file paths to be UTF8 encoded.
        //
        db.open(0, nativeToUTF8(_filename, IceUtil::getProcessStringConverter()).c_str(), 0, DB_UNKNOWN,
                DB_RDONLY, 0);

        Dbc* dbc = 0;
        db.cursor(0, &dbc, 0);

        Dbt dbKey;
        dbKey.set_flags(DB_DBT_MALLOC);

        Dbt dbValue;
        dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);

        bool more = true;
        while(more)
        {
            more = (dbc->get(&dbKey, &dbValue, DB_NEXT) == 0);
            if(more)
            {
                string dbName(static_cast<char*>(dbKey.get_data()), dbKey.get_size());

                if(dbName.find(indexPrefix) != 0)
                {
                    result.push_back(dbName);
                }
                free(dbKey.get_data());
            }
        }

        dbc->close();
        db.close(0);
    }
    catch(const DbException& dx)
    {
        if(dx.get_errno() != ENOENT)
        {
            DatabaseException ex(__FILE__, __LINE__);
            ex.message = dx.what();
            throw ex;
        }
    }

    return result;
}

