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

#ifndef FREEZE_EVICTOR_I_H
#define FREEZE_EVICTOR_I_H

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <Freeze/Evictor.h>
#include <Freeze/SharedDbEnv.h>
#include <Freeze/EvictorStorage.h>
#include <Freeze/DB.h>
#include <list>
#include <vector>
#include <deque>


namespace Freeze
{

class EvictorI : public Evictor,  public IceUtil::Monitor<IceUtil::Mutex>, public IceUtil::Thread
{
public:

    EvictorI(const Ice::CommunicatorPtr, const std::string&, const std::string&, bool);
    EvictorI(const Ice::CommunicatorPtr, const std::string&,  DbEnv&, const std::string&, bool);

    virtual ~EvictorI();

    virtual void setSize(Ice::Int);
    virtual Ice::Int getSize();

    virtual void saveNow();
    
    virtual void createObject(const Ice::Identity&, const Ice::ObjectPtr&);
    virtual void addFacet(const Ice::Identity&, const Ice::FacetPath&, const Ice::ObjectPtr&);
    
    virtual void destroyObject(const Ice::Identity&);
    virtual Ice::ObjectPtr removeFacet(const Ice::Identity&, const Ice::FacetPath&);
    virtual void removeAllFacets(const Ice::Identity&);

    virtual void installServantInitializer(const ServantInitializerPtr&);
    virtual EvictorIteratorPtr getIterator(Ice::Int, bool);
    virtual bool hasObject(const Ice::Identity&);

    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&);
    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
    virtual void deactivate(const std::string&);

    //
    // Thread
    //
    virtual void run();


    //
    // For the iterator:
    //
    const Ice::CommunicatorPtr&
    communicator() const;

    Db*
    db() const;

    int
    currentGeneration() const;

    //
    // Should be private, but the Sun C++ compiler forces us to
    // make them public
    //
    struct EvictorElement;
    typedef IceUtil::Handle<EvictorElement> EvictorElementPtr;
    typedef std::map<Ice::Identity, EvictorElementPtr> EvictorMap;

    struct Facet : public Ice::LocalObject
    {
	Facet(EvictorElement*);

	IceUtil::Mutex mutex;
	Ice::Byte status;  
	ObjectRecord rec; // 64 bit alignment
	EvictorElement* const element;
    };
    typedef IceUtil::Handle<Facet> FacetPtr;
    typedef std::map<Ice::FacetPath, FacetPtr> FacetMap;
    
    struct EvictorElement : public IceUtil::Shared
    {
	EvictorElement();
	~EvictorElement();
	
	std::list<EvictorMap::iterator>::iterator position;
	int usageCount;
	FacetMap facets;
	const Ice::Identity* identity;
	FacetPtr mainObject;
    };

    //
    // For the iterator:
    //
    bool
    load(Dbc*, Key&, Value&, std::vector<Ice::Identity>&, std::vector<EvictorElementPtr>&);

    bool
    load(Dbc*, Key&, std::vector<Ice::Identity>&);

    void 
    insert(const std::vector<Ice::Identity>&, const std::vector<EvictorElementPtr>&, int);


    //
    // Streamed objects
    //
    struct StreamedObject
    {
	Key key;
	Value value;
	Ice::Byte status;
    };

#if defined(_MSC_VER) && (_MSC_VER <= 1200)

    enum 
    { 
      clean = 0,
      created = 1,
      modified = 2,
      destroyed = 3,
      dead = 4
    };
    
#else 
    //
    // Clean object; can become modified or destroyed
    //
    static const Ice::Byte clean = 0;

    //
    // New objects; can become clean, dead or destroyed
    //
    static const Ice::Byte created = 1;

    //
    // Modified object; can become clean or destroyed
    //
    static const Ice::Byte modified = 2;

    //
    // Being saved. Can become dead or created
    //
    static const Ice::Byte destroyed = 3;

    //
    // Exists only in the Evictor; for example the object was created
    // and later destroyed (without a save in between), or it was
    // destroyed on disk but is still in use. Can become created.
    //
    static const Ice::Byte dead = 4;

#endif

private:

    void init(const std::string& envName, const std::string& dbName, bool createDb);

    void evict();
    bool dbHasObject(const Ice::Identity&);
    bool getObject(const Ice::Identity&, ObjectRecord&);
    void addToModifiedQueue(const FacetMap::iterator&, const FacetPtr&);
    void streamFacet(const FacetPtr&, const Ice::FacetPath&, Ice::Byte, Ice::Long, StreamedObject&);

    void saveNowNoSync();

    EvictorElementPtr load(const Ice::Identity&);
    EvictorMap::iterator insertElement(const Ice::ObjectAdapterPtr&, const Ice::Identity&, const EvictorElementPtr&);
  
    void addFacetImpl(EvictorElementPtr&, const Ice::ObjectPtr&, const Ice::FacetPath&, bool);
    void removeFacetImpl(FacetMap&,  const Ice::FacetPath&);
    Ice::ObjectPtr destroyFacetImpl(FacetMap::iterator&, const FacetPtr& facet);

    void buildFacetMap(const FacetMap&);
    
    inline void writeObjectRecordToValue(Ice::Long, ObjectRecord&, Value&);

    EvictorMap _evictorMap;

    //
    // The _evictorList contains a list of all objects we keep,
    // with the most recently used first
    //
    // Note: relies on the stability of iterators in a std::map
    //
    std::list<EvictorMap::iterator> _evictorList;
    EvictorMap::size_type _evictorSize;

    //
    // The _modifiedQueue contains a queue of all modified objects
    // Each element in the queue "owns" a usage count, to ensure the
    // element containing the pointed element remains in the evictor
    // map.
    //
    // Note: relies on the stability of iterators in a std::map
    //
    std::deque<FacetMap::iterator> _modifiedQueue;

    bool _deactivated;
  
    Ice::CommunicatorPtr _communicator;

    DbEnv* _dbEnv;
    SharedDbEnvPtr _dbEnvHolder;
    std::auto_ptr<Db> _db;
    ServantInitializerPtr _initializer;
    Ice::Int _trace;

    //
    // Threads that have requested a "saveNow" and are waiting for
    // its completion
    //
    std::deque<IceUtil::ThreadControl> _saveNowThreads;

    Ice::Int _saveSizeTrigger;
    Ice::Int _maxTxSize;
    IceUtil::Time _savePeriod;
    IceUtil::Time _lastSave;

    //
    // _generation is incremented after committing changes
    // to disk, when releasing the usage count of the element
    // that contains the created/modified/destroyed facets. 
    // Like the usage count, it is protected by the Evictor mutex.
    //
    // It is used to detect updates when loading an element and its
    // facets without holding the Evictor mutex. If the generation
    // is the same before the loading and later when the Evictor
    // mutex is locked again, and the map still does not contain 
    // this element, then the loaded value is current.
    //
    int _generation;
};

inline const Ice::CommunicatorPtr&
EvictorI::communicator() const
{
    return _communicator;
}

inline Db*
EvictorI::db() const
{
    return _db.get();
}

inline int
EvictorI::currentGeneration() const
{
    Lock sync(*this);
    return _generation;
}

}

#endif
