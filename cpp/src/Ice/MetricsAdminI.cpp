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
using namespace IceMetrics;


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
    for(map<string, MetricsObjectPtr>::const_iterator p = _objects.begin(); p != _objects.end(); ++p)
    {
        // TODO: Fix ice_clone!
        objects.push_back(dynamic_cast<MetricsObject*>(p->second->ice_clone().get()));
    }
    return objects;
}

MetricsMap::Entry
MetricsMap::getMatching(const MetricsHelper& helper)
{
    for(map<string, string>::const_iterator p = _accept.begin(); p != _accept.end(); ++p)
    {
        if(!match(helper(p->first), p->second))
        {
            return MetricsMap::Entry();
        }
    }
    
    for(map<string, string>::const_iterator p = _reject.begin(); p != _reject.end(); ++p)
    {
        if(match(helper(p->first), p->second))
        {
            return MetricsMap::Entry();
        }
    }
    
    ostringstream os;
    vector<string>::const_iterator q = _groupBySeparators.begin();
    for(vector<string>::const_iterator p = _groupByAttributes.begin(); p != _groupByAttributes.end(); ++p)
    {
        os << helper(*p) << *q;
    }

    string key = os.str();
    map<string, Entry>::const_iterator p = _objects.find(key);
    if(p == _objects.end())
    {
        Entry e;
        e.object = helper.newMetricsObject();
        e.object->id = key;
        e.map = this;
        p = _objects.insert(make_pair(os.str(), e)).first;
    }
    return p->second;
}

bool
MetricsMap::match(const string& value, const string& expr) const
{
    return true; // TODO
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
    for(map<string, MetricsMapPtr>::const_iterator p = _maps.begin(); p != _maps.end(); ++p)
    {
        metrics.insert(make_pair(p->first, p->second->getMetricsObjects()));
    }
    return metrics;
}

MetricsMap::Entry
MetricsView::getMatching(const string& cl, const MetricsHelper& helper) const
{
    map<string, MetricsMapPtr>::const_iterator p = _maps.find(cl);
    if(p != _maps.end())
    {
        return p->second->getMatching(helper);
    }
    return MetricsMap::Entry()
}

void
MetricsAdminI::addUpdater(const string& cl, const ObjectObserverUpdaterPtr& updater)
{
    _updaters.insert(make_pair(cl, updater));
}

vector<MetricsMap::Entry>
MetricsAdminI::getMatching(const string& cl, const MetricsHelper& helper) const
{
    Lock sync(*this);
    vector<MetricsMap::Entry> objects;
    for(map<string, MetricsViewPtr>::const_iterator p = _views.begin(); p != _views.end(); ++p)
    {
        MetricsMap::Entry e = p->second->getMatching(cl, helper);
        if(e.object)
        {
            objects.push_back(e);
        }
    }
    return objects;
}

MetricsObjectSeqDict
MetricsAdminI::getMetrics(const string& view, const Ice::Current&)
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
MetricsAdminI::getAllMetrics(const Ice::Current&)
{
    Lock sync(*this);
    MetricsObjectSeqDictDict metrics;
    for(map<string, MetricsViewPtr>::const_iterator p = _views.begin(); p != _views.end(); ++p)
    {
        metrics.insert(make_pair(p->first, p->second->getMetricsObjects()));
    }
    return metrics;
}

void
MetricsAdminI::addClassToView(const string& view, 
                              const string& cl, 
                              const string& groupBy, 
                              const NameValueDict& accept, 
                              const NameValueDict& reject,
                              const Ice::Current&)
{
    {
        Lock sync(*this);
        map<string, MetricsViewPtr>::const_iterator p = _views.find(view);
        if(p == _views.end())
        {
            p = _views.insert(make_pair(view, new MetricsView())).first;
        }
        p->second->add(cl, groupBy, accept, reject);
    }
    _updaters[cl]->update();
}

void
MetricsAdminI::removeClassFromView(const string& view, const string& cl, const Ice::Current&)
{
    {
        Lock sync(*this);
        _views.erase(cl);
    }
    _updaters[cl]->update();
}
