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

MetricsMapI::MetricsMapI(const std::string& mapPrefix, const Ice::PropertiesPtr& properties) :
    _retain(properties->getPropertyAsIntWithDefault(mapPrefix + ".RetainDetached", 10)),
    _accept(parseRule(properties, mapPrefix + ".Accept")),
    _reject(parseRule(properties, mapPrefix + ".Reject"))
{
    string groupBy = properties->getPropertyWithDefault(mapPrefix + ".GroupBy", "id");
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

MetricsMapI::MetricsMapI(const MetricsMapI& map) :
    _groupByAttributes(map._groupByAttributes),
    _groupBySeparators(map._groupBySeparators),
    _retain(map._retain),
    _accept(map._accept),
    _reject(map._reject)
{
}

MetricsMap
MetricsMapI::getMetrics() const
{
    MetricsMap objects;

    Lock sync(*this);
    for(map<string, EntryPtr>::const_iterator p = _objects.begin(); p != _objects.end(); ++p)
    {
        objects.push_back(p->second->clone());
    }
    return objects;
}

MetricsFailuresSeq
MetricsMapI::getFailures()
{
    MetricsFailuresSeq failures;

    Lock sync(*this);
    for(map<string, EntryPtr>::const_iterator p = _objects.begin(); p != _objects.end(); ++p)
    {
        MetricsFailures f = p->second->getFailures();
        if(!f.failures.empty())
        {
            failures.push_back(f);
        }
    }
    return failures;
}

MetricsMapI::EntryPtr
MetricsMapI::getMatching(const MetricsHelper& helper)
{
    for(map<string, string>::const_iterator p = _accept.begin(); p != _accept.end(); ++p)
    {
        if(!match(helper(p->first), p->second))
        {
            return 0;
        }
    }
    
    for(map<string, string>::const_iterator p = _reject.begin(); p != _reject.end(); ++p)
    {
        if(match(helper(p->first), p->second))
        {
            return 0;
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

    Lock sync(*this);
    map<string, EntryPtr>::const_iterator p = _objects.find(key);
    if(p == _objects.end())
    {
        p = _objects.insert(make_pair(key, newEntry(helper.newMetrics(key)))).first;
    }
    return p->second;
}

void
MetricsMapI::detached(Entry* entry)
{
    Lock sync(*this);
    if(_retain == 0)
    {
        return;
    }

    assert(_detachedQueue.size() <= _retain);

    deque<Entry*>::iterator p = _detachedQueue.begin();
    while(p != _detachedQueue.end())
    {
        if(*p == entry)
        {
            _detachedQueue.erase(p);
            break;
        }
        else if(!(*p)->isDetached())
        {
            p = _detachedQueue.erase(p);
        }
        else
        {
            ++p;
        }
    }

    if(_detachedQueue.size() == _retain)
    {
        // Remove oldest entry if there's still no room
        _objects.erase(_detachedQueue.front()->id());
        _detachedQueue.pop_front();
    }

    _detachedQueue.push_back(entry);
}
    
MetricsViewI::MetricsViewI(bool enabled) : _enabled(enabled)
{
}

void
MetricsViewI::add(const string& name, const MetricsMapIPtr& map)
{
    _maps.insert(make_pair(name, map));
}

void
MetricsViewI::remove(const string& cl)
{
    _maps.erase(cl);
}

MetricsView
MetricsViewI::getMetrics()
{
    MetricsView metrics;
    if(_enabled)
    {
        for(map<string, MetricsMapIPtr>::const_iterator p = _maps.begin(); p != _maps.end(); ++p)
        {
            metrics.insert(make_pair(p->first, p->second->getMetrics()));
        }
    }
    return metrics;
}

MetricsFailuresSeq
MetricsViewI::getFailures(const string& mapName)
{
    map<string, MetricsMapIPtr>::const_iterator p = _maps.find(mapName);
    if(p != _maps.end())
    {
        return p->second->getFailures();
    }
    return MetricsFailuresSeq();
}

MetricsMapI::EntryPtr
MetricsViewI::getMatching(const MetricsHelper& helper) const
{
    if(_enabled)
    {
        map<string, MetricsMapIPtr>::const_iterator p = _maps.find(helper.getMapName());
        if(p != _maps.end())
        {
            return p->second->getMatching(helper);
        }
    }
    return 0;
}

vector<string>
MetricsViewI::getMaps() const
{
    vector<string> maps;
    for(map<string, MetricsMapIPtr>::const_iterator p = _maps.begin(); p != _maps.end(); ++p)
    {
        maps.push_back(p->first);
    }
    return maps;
}

MetricsAdminI::MetricsAdminI(const Ice::PropertiesPtr& properties) : _properties(properties)
{
}

void
MetricsAdminI::addUpdater(const string& mapName, const UpdaterPtr& updater)
{
    _updaters.insert(make_pair(mapName, updater));
}

void
MetricsAdminI::addFactory(const string& mapName, const MetricsMapFactoryPtr& factory)
{
    _factories[mapName] = factory;

    //
    // Add maps to views configured with the given map.
    //
    const string viewsPrefix = "IceMX.MetricsView.";
    PropertyDict views = _properties->getPropertiesForPrefix(viewsPrefix);
    for(PropertyDict::const_iterator p = views.begin(); p != views.end(); ++p)
    {
        string viewName = p->first.substr(viewsPrefix.size());
        string::size_type dotPos = viewName.find('.');
        if(dotPos != string::npos)
        {
            viewName = viewName.substr(0, dotPos);
        }
        
        map<string, MetricsViewIPtr>::const_iterator q = _views.find(viewName);
        if(q == _views.end())
        {
            bool enabled = _properties->getPropertyAsIntWithDefault(viewsPrefix + viewName, 1) > 0;
            q = _views.insert(make_pair(viewName, new MetricsViewI(enabled))).first;
        }
        MetricsViewIPtr view = q->second;
        
        const string mapsPrefix = viewsPrefix + viewName + ".Map.";
        string mapPrefix = mapsPrefix + mapName;
        if(_properties->getPropertyAsInt(mapPrefix) == 0 || _properties->getPropertiesForPrefix(mapPrefix).empty())
        {
            if(_properties->getPropertiesForPrefix(mapsPrefix).empty())
            {
                mapPrefix = viewsPrefix + viewName;
            }
            else
            {
                continue; // This map isn't configured for this view.
            }
        }
        view->add(mapName, factory->create(mapPrefix, _properties));
    }
}

vector<MetricsMapI::EntryPtr>
MetricsAdminI::getMatching(const MetricsHelper& helper) const
{
    Lock sync(*this);
    vector<MetricsMapI::EntryPtr> objects;
    for(map<string, MetricsViewIPtr>::const_iterator p = _views.begin(); p != _views.end(); ++p)
    {
        MetricsMapI::EntryPtr e = p->second->getMatching(helper);
        if(e)
        {
            objects.push_back(e);
        }
    }
    sort(objects.begin(), objects.end());
    return objects;
}

Ice::StringSeq
MetricsAdminI::getMetricsViewNames(const ::Ice::Current&)
{
    Ice::StringSeq names;

    Lock sync(*this);
    for(map<string, MetricsViewIPtr>::const_iterator p = _views.begin(); p != _views.end(); ++p)
    {
        names.push_back(p->first);
    }
    return names;
}

MetricsView
MetricsAdminI::getMetricsView(const string& view, const ::Ice::Current&)
{
    Lock sync(*this);
    std::map<string, MetricsViewIPtr>::const_iterator p = _views.find(view);
    if(p == _views.end())
    {
        throw UnknownMetricsView();
    }
    return p->second->getMetrics();
}

MetricsFailuresSeq
MetricsAdminI::getMetricsFailures(const string& view, const string& map, const ::Ice::Current&)
{
    Lock sync(*this);
    std::map<string, MetricsViewIPtr>::const_iterator p = _views.find(view);
    if(p == _views.end())
    {
        throw UnknownMetricsView();
    }
    return p->second->getFailures(map);
}

void
MetricsAdminI::addMapToView(const string& view, 
                            const string& mapName, 
                            const string& groupBy,
                            int retain,
                            const NameValueDict& accept, 
                            const NameValueDict& reject,
                            const ::Ice::Current&)
{
    UpdaterPtr updater;
    {
        // TODO: XXX
        // Lock sync(*this);
        // map<string, MetricsViewIPtr>::const_iterator p = _views.find(view);
        // if(p == _views.end())
        // {
        //     p = _views.insert(make_pair(view, new MetricsViewI(true))).first;
        // }
        // p->second->add(mapName, _factories[mapName]->create());

        // map<string, UpdaterPtr>::const_iterator q = _updaters.find(mapName);
        // if(q != _updaters.end())
        // {
        //     updater = q->second;
        // }
    }
    if(updater)
    {
        updater->update();
    }
}

void
MetricsAdminI::removeMapFromView(const string& view, const string& mapName, const ::Ice::Current&)
{
    UpdaterPtr updater;
    {
        Lock sync(*this);
        map<string, MetricsViewIPtr>::const_iterator p = _views.find(view);
        if(p == _views.end())
        {
            throw UnknownMetricsView();
        }
        p->second->remove(mapName);

        map<string, UpdaterPtr>::const_iterator q = _updaters.find(mapName);
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
MetricsAdminI::setViewEnabled(const string& view, bool enabled)
{
    vector<string> maps;
    {
        Lock sync(*this);
        map<string, MetricsViewIPtr>::const_iterator p = _views.find(view);
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


