// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/MetricsAdminI.h>

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

MetricsMap::MetricsMap(const string& groupBy, const NameValueDict& accept, const NameValueDict& reject) : 
    _accept(accept), _reject(reject)
{
    // TODO: groupBy
}

void
MetricsMap::destroy()
{
    _objects.clear();
}

MetricsObjectSeq
MetricsMap::getMetricsObjects() const
{
    MetricsObjectSeq objects;
    for(map<string, EntryPtr>::const_iterator p = _objects.begin(); p != _objects.end(); ++p)
    {
        // TODO: Fix ice_clone to use a co-variant type.
        objects.push_back(dynamic_cast<MetricsObject*>(p->second->object->ice_clone().get()));
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
        os << helper(*p) << *q;
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
MetricsView::add(const string& cl, const string& groupBy, const NameValueDict& accept, const NameValueDict& reject)
{
    _maps.insert(make_pair(cl, new MetricsMap(groupBy, accept, reject)));
}

void
MetricsView::remove(const string& cl)
{
    _maps.erase(cl);
}

MetricsObjectSeqDict
MetricsView::getMetricsObjects() const
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

MetricsAdminI::MetricsAdminI(const PropertiesPtr& properties)
{
    const string viewsPrefix = "Ice.MetricsView.";
    
    vector<string> defaultMaps;
    defaultMaps.push_back("Connection");
    defaultMaps.push_back("Thread");
    defaultMaps.push_back("ThreadPoolThread");
    defaultMaps.push_back("Request");
    
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
            
            if(mapName == "GroupBy" || mapName == "Accept" || mapName == "Reject")
            {
                continue; // Those aren't maps.
            }
            
            ++mapsCount;
            
            string groupBy = properties->getPropertyWithDefault(mapsPrefix + mapName + ".GroupBy", "parent");
            NameValueDict accept = parseRule(properties, mapsPrefix + mapName + ".Accept");
            NameValueDict reject = parseRule(properties, mapsPrefix + mapName + ".Reject");
            addMapToView(viewName, mapName, groupBy, accept, reject);
        }

        //
        // If no maps were defined explicitly, add default maps.
        //
        if(mapsCount == 0)
        {
            string groupBy = properties->getPropertyWithDefault(viewsPrefix + viewName + ".GroupBy", "parent");
            NameValueDict accept = parseRule(properties, viewsPrefix + viewName + ".Accept");
            NameValueDict reject = parseRule(properties, viewsPrefix + viewName + ".Reject");
            for(vector<string>::const_iterator p = defaultMaps.begin(); p != defaultMaps.end(); ++p)
            {
                addMapToView(viewName, *p, groupBy, accept, reject);
            }
        }
    }
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
                            const NameValueDict& accept, 
                            const NameValueDict& reject,
                            const ::Ice::Current&)
{
    {
        Lock sync(*this);
        map<string, MetricsViewPtr>::const_iterator p = _views.find(view);
        if(p == _views.end())
        {
            p = _views.insert(make_pair(view, new MetricsView())).first;
        }
        p->second->add(mapName, groupBy, accept, reject);
    }
    _updaters[mapName]->update();
}

void
MetricsAdminI::removeMapFromView(const string& view, const string& mapName, const ::Ice::Current&)
{
    {
        Lock sync(*this);
        map<string, MetricsViewPtr>::const_iterator p = _views.find(view);
        if(p == _views.end())
        {
            throw UnknownMetricsView();
        }
        p->second->remove(mapName);
    }
    _updaters[mapName]->update();
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


