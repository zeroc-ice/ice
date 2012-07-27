// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/MetricsAdminI.h>

#include <Ice/ObserverI.h>
#include <Ice/Properties.h>
#include <Ice/Communicator.h>
#include <Ice/Instance.h>

#include <IceUtil/StringUtil.h>

using namespace std;
using namespace Ice;
using namespace IceMX;

namespace 
{

struct MOCompare
{
    bool operator()(const pair<MetricsObjectPtr, IceUtil::Mutex*>& lhs, 
                    const pair<MetricsObjectPtr, IceUtil::Mutex*>& rhs)
    { 
        return lhs.first.get() < rhs.first.get();
    }
};

NameValueDict
parseRule(const ::Ice::PropertiesPtr& properties, const string& name)
{
    NameValueDict dict;
    PropertyDict rules = properties->getPropertiesForPrefix(name + '.');
    for(PropertyDict::const_iterator p = rules.begin(); p != rules.end(); ++p)
    {
        dict.insert(make_pair(p->first.substr(name.size() + 1), p->second));
    }
    return dict;
}

bool
match(const string& value, const string& expr)
{
    return true; // TODO
}

}

MetricsMap::MetricsMap(const string& groupBy, bool reap, const NameValueDict& accept, const NameValueDict& reject) : 
    _reap(reap), _accept(accept), _reject(reject)
{
    if(!groupBy.empty())
    {
        string v;
        bool attribute = IceUtilInternal::isAlpha(groupBy[0]) || IceUtilInternal::isDigit(groupBy[0]);
        if(!attribute)
        {
            _groupByAttributes.push_back("");
        }
        
        for(string::const_iterator p = groupBy.begin(); p != groupBy.end(); ++p)
        {
            bool isAlphaNum = IceUtilInternal::isAlpha(*p) || IceUtilInternal::isDigit(*p);
            if(attribute && !isAlphaNum)
            {
                _groupByAttributes.push_back(v);
                v = *p;
                attribute = false;
            }
            else if(!attribute && isAlphaNum)
            {
                _groupBySeparators.push_back(v);
                v = *p;
                attribute = true;
            }
            else
            {
                v += *p;
            }
        }

        if(attribute)
        {
            _groupByAttributes.push_back(v);
        }
        else
        {
            _groupBySeparators.push_back(v);
        }
    }
}

void
MetricsMap::destroy()
{
    _objects.clear();
}

MetricsObjectSeq
MetricsMap::getMetricsObjects()
{
    MetricsObjectSeq objects;
    map<string, EntryPtr>::iterator p = _objects.begin(); 
    while(p != _objects.end())
    {
        IceUtil::Mutex::Lock sync(p->second->mutex);
        if(_reap && p->second->object->current == 0 && p->second->object->total == 1)
        {
            sync.release(); // We're going to destroy the mutex
            _objects.erase(p++);
        }
        else
        {
            // TODO: Fix ice_clone to use a co-variant type.
            objects.push_back(dynamic_cast<MetricsObject*>(p->second->object->ice_clone().get()));
            ++p;
        }
    }
    return objects;
}

pair<MetricsObjectPtr, IceUtil::Mutex*>
MetricsMap::getMatching(const ObjectHelper& helper)
{
    for(map<string, string>::const_iterator p = _accept.begin(); p != _accept.end(); ++p)
    {
        if(!match(helper(p->first), p->second))
        {
            return make_pair(MetricsObjectPtr(), static_cast<IceUtil::Mutex*>(0));
        }
    }
    
    for(map<string, string>::const_iterator p = _reject.begin(); p != _reject.end(); ++p)
    {
        if(match(helper(p->first), p->second))
        {
            return make_pair(MetricsObjectPtr(), static_cast<IceUtil::Mutex*>(0));
        }
    }
    
    ostringstream os;
    vector<string>::const_iterator q = _groupBySeparators.begin();
    for(vector<string>::const_iterator p = _groupByAttributes.begin(); p != _groupByAttributes.end(); ++p)
    {
        os << helper(*p);
        if(q != _groupBySeparators.end())
        {
            os << *q++;
        }
    }

    string key = os.str();
    map<string, EntryPtr>::const_iterator p = _objects.find(key);
    if(p == _objects.end())
    {
        p = _objects.insert(make_pair(os.str(), new Entry(helper.newMetricsObject()))).first;
        p->second->object->id = key;
    }
    return make_pair(p->second->object, &p->second->mutex);
}
    
MetricsView::MetricsView()
{
}

void
MetricsView::add(const string& name, const string& groupBy, bool reap, const NameValueDict& accept, 
                 const NameValueDict& reject)
{
    _maps.insert(make_pair(name, new MetricsMap(groupBy, reap, accept, reject)));
}

void
MetricsView::remove(const string& cl)
{
    _maps.erase(cl);
}

MetricsObjectSeqDict
MetricsView::getMetricsObjects()
{
    MetricsObjectSeqDict metrics;
    if(_enabled)
    {
        for(map<string, MetricsMapPtr>::const_iterator p = _maps.begin(); p != _maps.end(); ++p)
        {
            metrics.insert(make_pair(p->first, p->second->getMetricsObjects()));
        }
    }
    return metrics;
}

pair<MetricsObjectPtr, IceUtil::Mutex*>
MetricsView::getMatching(const string& mapName, const ObjectHelper& helper) const
{
    if(_enabled)
    {
        map<string, MetricsMapPtr>::const_iterator p = _maps.find(mapName);
        if(p != _maps.end())
        {
            return p->second->getMatching(helper);
        }
    }
    return make_pair(MetricsObjectPtr(), static_cast<IceUtil::Mutex*>(0));
}

vector<string>
MetricsView::getMaps() const
{
    vector<string> maps;
    for(map<string, MetricsMapPtr>::const_iterator p = _maps.begin(); p != _maps.end(); ++p)
    {
        maps.push_back(p->first);
    }
    return maps;
}

MetricsAdminI::MetricsAdminI(InitializationData& initData)
{
    const string viewsPrefix = "IceMX.MetricsView.";
    
    vector<string> defaultMaps;
    defaultMaps.push_back("Connection");
    defaultMaps.push_back("Thread");
    defaultMaps.push_back("ThreadPoolThread");
    defaultMaps.push_back("Request");
    
    PropertiesPtr properties = initData.properties;
    if(initData.observerResolver)
    {
        // ERROR!
    }

    //getInstance(communicator)->setObserverResolver();
    __setNoDelete(true);
    initData.observerResolver = new ObserverResolverI(this);

    PropertyDict views = properties->getPropertiesForPrefix(viewsPrefix);
    for(PropertyDict::const_iterator p = views.begin(); p != views.end(); ++p)
    {
        string viewName = p->first.substr(viewsPrefix.size());
        string::size_type dotPos = viewName.find('.');
        if(dotPos != string::npos)
        {
            viewName = viewName.substr(0, dotPos);
        }
        
        MetricsViewPtr view = new MetricsView();
        _views.insert(make_pair(viewName, view));

        view->setEnabled(properties->getPropertyAsIntWithDefault(viewsPrefix + viewName, 1) > 0);
        
        int mapsCount = 0;
        const string mapsPrefix = viewsPrefix + viewName + '.';
        PropertyDict maps = properties->getPropertiesForPrefix(mapsPrefix);
        for(PropertyDict::const_iterator q = maps.begin(); q != maps.end(); ++q)
        {
            string mapName = q->first.substr(mapsPrefix.size());
            dotPos = mapName.find('.');
            if(dotPos != string::npos)
            {
                mapName = mapName.substr(0, dotPos);
            }
            
            if(mapName == "GroupBy" || mapName == "Accept" || mapName == "Reject" || mapName == "Reap")
            {
                continue; // Those aren't maps.
            }
            
            ++mapsCount;
            
            string groupBy = properties->getProperty(mapsPrefix + mapName + ".GroupBy");
            bool reap = properties->getPropertyAsInt(mapsPrefix + mapName + ".Reap") > 0;
            NameValueDict accept = parseRule(properties, mapsPrefix + mapName + ".Accept");
            NameValueDict reject = parseRule(properties, mapsPrefix + mapName + ".Reject");
            addMapToView(viewName, mapName, groupBy, reap, accept, reject);
        }

        //
        // If no maps were defined explicitly, add default maps.
        //
        if(mapsCount == 0)
        {
            string groupBy = properties->getProperty(viewsPrefix + viewName + ".GroupBy");
            bool reap = properties->getPropertyAsInt(viewsPrefix + viewName + ".Reap") > 0;
            NameValueDict accept = parseRule(properties, viewsPrefix + viewName + ".Accept");
            NameValueDict reject = parseRule(properties, viewsPrefix + viewName + ".Reject");
            for(vector<string>::const_iterator p = defaultMaps.begin(); p != defaultMaps.end(); ++p)
            {
                addMapToView(viewName, *p, groupBy, reap, accept, reject);
            }
        }
    }

    __setNoDelete(false);
}

void
MetricsAdminI::addUpdater(const string& mapName, const ObjectObserverUpdaterPtr& updater)
{
    _updaters.insert(make_pair(mapName, updater));
}

vector<pair<MetricsObjectPtr, IceUtil::Mutex*> >
MetricsAdminI::getMatching(const string& mapName, const ObjectHelper& helper) const
{
    Lock sync(*this);
    vector<pair<MetricsObjectPtr, IceUtil::Mutex*> > objects;
    for(map<string, MetricsViewPtr>::const_iterator p = _views.begin(); p != _views.end(); ++p)
    {
        pair<MetricsObjectPtr, IceUtil::Mutex*> e = p->second->getMatching(mapName, helper);
        if(e.first)
        {
            objects.push_back(e);
        }
    }
    sort(objects.begin(), objects.end(), MOCompare());
    return objects;
}

MetricsObjectSeqDict
MetricsAdminI::getMetricsMaps(const string& view, const ::Ice::Current&)
{
    Lock sync(*this);
    std::map<string, MetricsViewPtr>::const_iterator p = _views.find(view);
    if(p == _views.end())
    {
        throw UnknownMetricsView();
    }
    return p->second->getMetricsObjects();
}

MetricsObjectSeqDictDict 
MetricsAdminI::getAllMetricsMaps(const ::Ice::Current&)
{
    Lock sync(*this);
    MetricsObjectSeqDictDict metrics;
    for(map<string, MetricsViewPtr>::const_iterator p = _views.begin(); p != _views.end(); ++p)
    {
        if(p->second->isEnabled())
        {
            metrics.insert(make_pair(p->first, p->second->getMetricsObjects()));
        }
    }
    return metrics;
}

void
MetricsAdminI::addMapToView(const string& view, 
                            const string& mapName, 
                            const string& groupBy,
                            bool reap,
                            const NameValueDict& accept, 
                            const NameValueDict& reject,
                            const ::Ice::Current&)
{
    ObjectObserverUpdaterPtr updater;
    {
        Lock sync(*this);
        map<string, MetricsViewPtr>::const_iterator p = _views.find(view);
        if(p == _views.end())
        {
            p = _views.insert(make_pair(view, new MetricsView())).first;
        }
        p->second->add(mapName, groupBy, reap, accept, reject);

        map<string, ObjectObserverUpdaterPtr>::const_iterator q = _updaters.find(mapName);
        if(q != _updaters.end())
        {
            updater = q->second;
        }
    }
    if(updater)
    {
        updater->update();
    }
}

void
MetricsAdminI::removeMapFromView(const string& view, const string& mapName, const ::Ice::Current&)
{
    ObjectObserverUpdaterPtr updater;
    {
        Lock sync(*this);
        map<string, MetricsViewPtr>::const_iterator p = _views.find(view);
        if(p == _views.end())
        {
            throw UnknownMetricsView();
        }
        p->second->remove(mapName);

        map<string, ObjectObserverUpdaterPtr>::const_iterator q = _updaters.find(mapName);
        if(q != _updaters.end())
        {
            updater = q->second;
        }
    }
    if(updater)
    {
        updater->update();
    }
}

void
MetricsAdminI::enableView(const string& view, const ::Ice::Current&)
{
    setViewEnabled(view, true);
}

void
MetricsAdminI::disableView(const string& view, const ::Ice::Current&)
{
    setViewEnabled(view, false);
}

void
MetricsAdminI::setViewEnabled(const string& view, bool enabled)
{
    vector<string> maps;
    {
        Lock sync(*this);
        map<string, MetricsViewPtr>::const_iterator p = _views.find(view);
        if(p == _views.end())
        {
            throw UnknownMetricsView();
        }
        p->second->setEnabled(enabled);
        maps = p->second->getMaps();
    }
    for(vector<string>::const_iterator p = maps.begin(); p != maps.end(); ++p)
    {
        _updaters[*p]->update();
    }
}


