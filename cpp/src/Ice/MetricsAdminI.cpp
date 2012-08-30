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

vector<MetricsMapI::RegExpPtr>
parseRule(const ::Ice::PropertiesPtr& properties, const string& name)
{
    vector<MetricsMapI::RegExpPtr> regexps;
    PropertyDict rules = properties->getPropertiesForPrefix(name + '.');
    for(PropertyDict::const_iterator p = rules.begin(); p != rules.end(); ++p)
    {
        regexps.push_back(new MetricsMapI::RegExp(p->first.substr(name.length() + 1), p->second));
    }
    return regexps;
}

}

MetricsMapI::RegExp::RegExp(const string& attribute, const string& regexp) : _attribute(attribute)
{
#ifndef ICE_CPP11_REGEXP
    if(regcomp(&_preg, regexp.c_str(), REG_EXTENDED | REG_NOSUB) != 0)
    {
        throw Ice::SyscallException(__FILE__, __LINE__); 
    }
#else
    _regex = regex(regexp, std::regex_constants::extended | std::regex_constants::nosubs);
#endif
}

MetricsMapI::RegExp::~RegExp()
{
#ifndef ICE_CPP11_REGEXP
    regfree(&_preg);
#endif
}

bool
MetricsMapI::RegExp::match(const MetricsHelper& helper)
{
    string value = helper(_attribute);
#ifndef ICE_CPP11_REGEXP
    if(regexec(&_preg, value.c_str(), 0, 0, 0) == 0)
    {
        return true;
    }
    return false;
#else
    return regex_match(value, _regex);
#endif
}

MetricsMapI::MetricsMapI(const std::string& mapPrefix, const Ice::PropertiesPtr& properties) :
    _properties(properties->getPropertiesForPrefix(mapPrefix)),
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

MetricsFailures
MetricsMapI::getFailures(const string& id)
{
    Lock sync(*this);
    map<string, EntryPtr>::const_iterator p = _objects.begin();
    if(p != _objects.end())
    {
        return p->second->getFailures();
    }
    return MetricsFailures();
}

MetricsMapI::EntryPtr
MetricsMapI::getMatching(const MetricsHelper& helper)
{
    for(vector<RegExpPtr>::const_iterator p = _accept.begin(); p != _accept.end(); ++p)
    {
        if(!(*p)->match(helper))
        {
            return 0;
        }
    }
    
    for(vector<RegExpPtr>::const_iterator p = _reject.begin(); p != _reject.end(); ++p)
    {
        if((*p)->match(helper))
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
        p = _objects.insert(make_pair(key, newEntry(key))).first;
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

    assert(static_cast<int>(_detachedQueue.size()) <= _retain);

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

    if(static_cast<int>(_detachedQueue.size()) == _retain)
    {
        // Remove oldest entry if there's still no room
        _objects.erase(_detachedQueue.front()->id());
        _detachedQueue.pop_front();
    }

    _detachedQueue.push_back(entry);
}
    
MetricsViewI::MetricsViewI(const string& name) : _name(name)
{
}

void
MetricsViewI::update(const Ice::PropertiesPtr& properties, 
                     const map<string, MetricsMapFactoryPtr>& factories,
                     set<string>& updatedMaps)
{
    //
    // Add maps to views configured with the given map.
    //
    const string viewPrefix = "IceMX.Metrics." + _name;
    const string mapsPrefix = viewPrefix + ".Map.";
    bool hasMapsProperties = !properties->getPropertiesForPrefix(mapsPrefix).empty();
    for(map<string, MetricsMapFactoryPtr>::const_iterator p = factories.begin(); p != factories.end(); ++p)
    {
        const string& mapName = p->first;
        string mapPrefix;
        PropertyDict mapProps;
        if(hasMapsProperties)
        {
            mapPrefix = mapsPrefix + mapName;
            mapProps = properties->getPropertiesForPrefix(mapPrefix);
            if(mapProps.empty())
            {
                // This map isn't configured for this view.
                updatedMaps.insert(mapName);
                _maps.erase(mapName);
                continue; 
            }
        }
        else
        {
            mapPrefix = viewPrefix;
            mapProps = properties->getPropertiesForPrefix(mapPrefix);
        }

        map<string, MetricsMapIPtr>::iterator q = _maps.find(mapName);
        if(q != _maps.end() && q->second->getMapProperties() == mapProps)
        {
            continue; // The map configuration didn't change.
        }
        _maps.insert(make_pair(mapName, p->second->create(mapPrefix, properties)));
        updatedMaps.insert(mapName);
    }
}

MetricsView
MetricsViewI::getMetrics()
{
    MetricsView metrics;
    for(map<string, MetricsMapIPtr>::const_iterator p = _maps.begin(); p != _maps.end(); ++p)
    {
        metrics.insert(make_pair(p->first, p->second->getMetrics()));
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

MetricsFailures
MetricsViewI::getFailures(const string& mapName, const string& id)
{
    map<string, MetricsMapIPtr>::const_iterator p = _maps.find(mapName);
    if(p != _maps.end())
    {
        return p->second->getFailures(id);
    }
    return MetricsFailures();
}

MetricsMapI::EntryPtr
MetricsViewI::getMatching(const string& mapName, const MetricsHelper& helper) const
{
    map<string, MetricsMapIPtr>::const_iterator p = _maps.find(mapName);
    if(p != _maps.end())
    {
        return p->second->getMatching(helper);
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
MetricsAdminI::updateViews()
{
    //
    // Add maps to views configured with the given map.
    //
    const string viewsPrefix = "IceMX.Metrics.";
    PropertyDict views = _properties->getPropertiesForPrefix(viewsPrefix);
    set<string> updatedMaps;
    for(PropertyDict::const_iterator p = views.begin(); p != views.end(); ++p)
    {
        string viewName = p->first.substr(viewsPrefix.size());
        string::size_type dotPos = viewName.find('.');
        if(dotPos != string::npos)
        {
            viewName = viewName.substr(0, dotPos);
        }

        if(_properties->getPropertyAsIntWithDefault(viewsPrefix + viewName + ".Disabled", 0) > 0)
        {
            // The view is disabled
            _views.erase(viewName);
            continue;
        }
        
        map<string, MetricsViewIPtr>::const_iterator q = _views.find(viewName);
        if(q == _views.end())
        {
            q = _views.insert(make_pair(viewName, new MetricsViewI(viewName))).first;
        }
        q->second->update(_properties, _factories, updatedMaps);
    }

    for(set<string>::const_iterator p = updatedMaps.begin(); p != updatedMaps.end(); ++p)
    {
        map<string, UpdaterPtr>::const_iterator q = _updaters.find(*p);
        if(q != _updaters.end())
        {
            q->second->update();
        }
    }
}

vector<MetricsMapI::EntryPtr>
MetricsAdminI::getMatching(const string& mapName, const MetricsHelper& helper) const
{
    Lock sync(*this);
    vector<MetricsMapI::EntryPtr> objects;
    for(map<string, MetricsViewIPtr>::const_iterator p = _views.begin(); p != _views.end(); ++p)
    {
        MetricsMapI::EntryPtr e = p->second->getMatching(mapName, helper);
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
MetricsAdminI::getMapMetricsFailures(const string& view, const string& map, const ::Ice::Current&)
{
    Lock sync(*this);
    std::map<string, MetricsViewIPtr>::const_iterator p = _views.find(view);
    if(p == _views.end())
    {
        throw UnknownMetricsView();
    }
    return p->second->getFailures(map);
}

MetricsFailures
MetricsAdminI::getMetricsFailures(const string& view, const string& map, const string& id, const ::Ice::Current&)
{
    Lock sync(*this);
    std::map<string, MetricsViewIPtr>::const_iterator p = _views.find(view);
    if(p == _views.end())
    {
        throw UnknownMetricsView();
    }
    return p->second->getFailures(map, id);
}
