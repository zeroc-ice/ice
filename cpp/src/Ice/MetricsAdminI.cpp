// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/MetricsAdminI.h>

#include <Ice/InstrumentationI.h>
#include <Ice/Properties.h>
#include <Ice/Logger.h>
#include <Ice/Communicator.h>
#include <Ice/Instance.h>
#include <Ice/LoggerUtil.h>

#include <IceUtil/StringUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace IceMX;

namespace 
{

const string viewSuffixes[] =
{
    "Disabled",
    "GroupBy",
    "Accept.*",
    "Reject.*",
    "RetainDetached",
    "Map.*",
};

const string mapSuffixes[] =
{
    "GroupBy",
    "Accept.*",
    "Reject.*",
    "RetainDetached",
};

void
validateProperties(const string& prefix, const PropertiesPtr& properties, const string* suffixes, int cnt)
{
    vector<string> unknownProps;
    PropertyDict props = properties->getPropertiesForPrefix(prefix);
    for(PropertyDict::const_iterator p = props.begin(); p != props.end(); ++p)
    {
        bool valid = false;
        for(unsigned int i = 0; i < cnt; ++i)
        {
            string prop = prefix + suffixes[i];
            if(IceUtilInternal::match(p->first, prop))
            {
                valid = true;
                break;
            }
        }
        if(!valid)
        {
            unknownProps.push_back(p->first);
        }
    }

    if(!unknownProps.empty())
    {
        Warning out(getProcessLogger());
        out << "found unknown Ice metrics properties for '" << prefix.substr(0, prefix.size() - 1) << "':";
        for(vector<string>::const_iterator p = unknownProps.begin(); p != unknownProps.end(); ++p)
        {
            out << "\n    " << *p;
            properties->setProperty(*p, ""); // Clear the known property to prevent further warnings.
        }
    }
}

vector<MetricsMapI::RegExpPtr>
parseRule(const PropertiesPtr& properties, const string& name)
{
    vector<MetricsMapI::RegExpPtr> regexps;
    PropertyDict rules = properties->getPropertiesForPrefix(name + '.');
    for(PropertyDict::const_iterator p = rules.begin(); p != rules.end(); ++p)
    {
        try
        {
            regexps.push_back(new MetricsMapI::RegExp(p->first.substr(name.length() + 1), p->second));
        }
        catch(const std::exception&)
        {
            throw "invalid regular expression `" + p->second + "' for `" + p->first + "'";
        }
    }
    return regexps;
}

}

MetricsMapI::RegExp::RegExp(const string& attribute, const string& regexp) : _attribute(attribute)
{
#ifndef ICE_CPP11_REGEXP
    if(regcomp(&_preg, regexp.c_str(), REG_EXTENDED | REG_NOSUB) != 0)
    {
        throw SyscallException(__FILE__, __LINE__); 
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
MetricsMapI::RegExp::match(const string& value, bool reject)
{
#ifndef ICE_CPP11_REGEXP
    return regexec(&_preg, value.c_str(), 0, 0, 0) == 0;
#else
    return regex_match(value, _regex);
#endif
}

MetricsMapI::MetricsMapI(const std::string& mapPrefix, const PropertiesPtr& properties) :
    _properties(properties->getPropertiesForPrefix(mapPrefix)),
    _retain(properties->getPropertyAsIntWithDefault(mapPrefix + "RetainDetached", 10)),
    _accept(parseRule(properties, mapPrefix + "Accept")),
    _reject(parseRule(properties, mapPrefix + "Reject"))
{
    string groupBy = properties->getPropertyWithDefault(mapPrefix + "GroupBy", "id");
    vector<string>& groupByAttributes = const_cast<vector<string>&>(_groupByAttributes);
    vector<string>& groupBySeparators = const_cast<vector<string>&>(_groupBySeparators);
    if(!groupBy.empty())
    {
        string v;
        bool attribute = IceUtilInternal::isAlpha(groupBy[0]) || IceUtilInternal::isDigit(groupBy[0]);
        if(!attribute)
        {
            groupByAttributes.push_back("");
        }
        
        for(string::const_iterator p = groupBy.begin(); p != groupBy.end(); ++p)
        {
            bool isAlphaNum = IceUtilInternal::isAlpha(*p) || IceUtilInternal::isDigit(*p) || *p == '.';
            if(attribute && !isAlphaNum)
            {
                groupByAttributes.push_back(v);
                v = *p;
                attribute = false;
            }
            else if(!attribute && isAlphaNum)
            {
                groupBySeparators.push_back(v);
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
            groupByAttributes.push_back(v);
        }
        else
        {
            groupBySeparators.push_back(v);
        }
    }
}

MetricsMapI::MetricsMapI(const MetricsMapI& map) :
    _properties(map._properties),
    _groupByAttributes(map._groupByAttributes),
    _groupBySeparators(map._groupBySeparators),
    _retain(map._retain),
    _accept(map._accept),
    _reject(map._reject)
{
}

void
MetricsMapI::validateProperties(const string& prefix, const Ice::PropertiesPtr& props, const vector<string>& subMaps)
{
    if(subMaps.empty())
    {
        ::validateProperties(prefix, props, mapSuffixes, sizeof(mapSuffixes) / sizeof(*mapSuffixes));
        return;
    }

    vector<string> suffixes(mapSuffixes, mapSuffixes + sizeof(mapSuffixes) / sizeof(*mapSuffixes));
    for(vector<string>::const_iterator p = subMaps.begin(); p != subMaps.end(); ++p)
    {
        const string suffix = "Map." + *p + ".";
        ::validateProperties(prefix + suffix, props, mapSuffixes, sizeof(mapSuffixes) / sizeof(*mapSuffixes));
        suffixes.push_back(suffix + '*');
    }
    ::validateProperties(prefix, props, &suffixes[0], suffixes.size());
}

const Ice::PropertyDict&
MetricsMapI::getProperties() const
{
    return _properties;
}

MetricsMapFactory::MetricsMapFactory(Updater* updater) : _updater(updater)
{
}

void
MetricsMapFactory::update()
{
    assert(_updater);
    _updater->update();
}
    
MetricsViewI::MetricsViewI(const string& name) : _name(name)
{
}

bool
MetricsViewI::addOrUpdateMap(const PropertiesPtr& properties, const string& mapName, 
                             const MetricsMapFactoryPtr& factory, const Ice::LoggerPtr& logger)
{
    //
    // Add maps to views configured with the given map.
    //
    const string viewPrefix = "IceMX.Metrics." + _name + ".";
    const string mapsPrefix = viewPrefix + "Map.";
    PropertyDict mapsProps = properties->getPropertiesForPrefix(mapsPrefix);

    string mapPrefix;
    PropertyDict mapProps;
    if(!mapsProps.empty())
    {
        mapPrefix = mapsPrefix + mapName + ".";
        mapProps = properties->getPropertiesForPrefix(mapPrefix);
        if(mapProps.empty())
        {
            // This map isn't configured for this view.
            _maps.erase(mapName);
            return true;
        }
    }
    else
    {
        mapPrefix = viewPrefix;
        mapProps = properties->getPropertiesForPrefix(mapPrefix);
    }

    map<string, MetricsMapIPtr>::iterator q = _maps.find(mapName);
    if(q != _maps.end() && q->second->getProperties() == mapProps)
    {
        return false; // The map configuration didn't change, no need to re-create.
    }

    try
    {
        _maps[mapName] = factory->create(mapPrefix, properties);
    }
    catch(const std::exception& ex)
    {
        Ice::Warning warn(logger);
        warn << "unexpected exception while creating metrics map:\n" << ex;
        _maps.erase(mapName);
    }
    catch(const string& msg)
    {
        Ice::Warning warn(logger);
        warn << msg;
        _maps.erase(mapName);
    }
    return true;
}

bool
MetricsViewI::removeMap(const string& mapName)
{
    map<string, MetricsMapIPtr>::iterator q = _maps.find(mapName);
    if(q != _maps.end())
    {
        _maps.erase(q);
        return true;
    }
    return false;
}

MetricsView
MetricsViewI::getMetrics()
{
    MetricsView metrics;
    for(map<string, MetricsMapIPtr>::const_iterator p = _maps.begin(); p != _maps.end(); ++p)
    {
        MetricsMap map = p->second->getMetrics();
        if(!map.empty())
        {
            metrics.insert(make_pair(p->first, map));
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

MetricsMapIPtr
MetricsViewI::getMap(const string& mapName) const
{
    map<string, MetricsMapIPtr>::const_iterator p = _maps.find(mapName);
    if(p != _maps.end())
    {
        return p->second;
    }
    return 0;
}

MetricsAdminI::MetricsAdminI(const PropertiesPtr& properties, const LoggerPtr& logger) : 
    _logger(logger), _properties(properties)
{
    updateViews();
}

void
MetricsAdminI::updateViews()
{
    set<MetricsMapFactoryPtr> updatedMaps;
    {
        Lock sync(*this);
        const string viewsPrefix = "IceMX.Metrics.";
        PropertyDict viewsProps = _properties->getPropertiesForPrefix(viewsPrefix);
        map<string, MetricsViewIPtr> views;
        for(PropertyDict::const_iterator p = viewsProps.begin(); p != viewsProps.end(); ++p)
        {
            string viewName = p->first.substr(viewsPrefix.size());
            string::size_type dotPos = viewName.find('.');
            if(dotPos != string::npos)
            {
                viewName = viewName.substr(0, dotPos);
            }

            if(views.find(viewName) != views.end())
            {
                continue; // View already configured.
            }

            validateProperties(viewsPrefix + viewName + ".", _properties, viewSuffixes, sizeof(viewSuffixes) /
                               sizeof(*viewSuffixes));

            if(_properties->getPropertyAsIntWithDefault(viewsPrefix + viewName + ".Disabled", 0) > 0)
            {
                continue; // The view is disabled
            }

            //
            // Create the view or update it.
            //
            map<string, MetricsViewIPtr>::const_iterator q = _views.find(viewName);
            if(q == _views.end())
            {
                q = views.insert(make_pair(viewName, new MetricsViewI(viewName))).first;
            }
            else
            {
                q = views.insert(make_pair(viewName, q->second)).first;
            }

            for(map<string, MetricsMapFactoryPtr>::const_iterator p = _factories.begin(); p != _factories.end(); ++p)
            {
                if(q->second->addOrUpdateMap(_properties, p->first, p->second, _logger))
                {
                    updatedMaps.insert(p->second);
                }
            }
        }
        _views.swap(views);
        
        //
        // Go through removed views to collect maps to update.
        //
        for(map<string, MetricsViewIPtr>::const_iterator p = views.begin(); p != views.end(); ++p)
        {
            if(_views.find(p->first) == _views.end())
            {
                vector<string> maps = p->second->getMaps();
                for(vector<string>::const_iterator q = maps.begin(); q != maps.end(); ++q)
                {
                    updatedMaps.insert(_factories[*q]);
                }
            }
        }
    }
    
    //
    // Call the updaters to update the maps.
    //
    for(set<MetricsMapFactoryPtr>::const_iterator p = updatedMaps.begin(); p != updatedMaps.end(); ++p)
    {
        (*p)->update();
    }
}

void
MetricsAdminI::unregisterMap(const std::string& mapName)
{
    bool updated;
    MetricsMapFactoryPtr factory;
    {
        Lock sync(*this);
        map<string, MetricsMapFactoryPtr>::iterator p = _factories.find(mapName);
        if(p == _factories.end())
        {
            return;
        }
        factory = p->second;
        _factories.erase(p);
        updated = removeMap(mapName);
    }
    if(updated)
    {
        factory->update();
    }
}

StringSeq
MetricsAdminI::getMetricsViewNames(const Current&)
{
    StringSeq names;

    Lock sync(*this);
    for(map<string, MetricsViewIPtr>::const_iterator p = _views.begin(); p != _views.end(); ++p)
    {
        names.push_back(p->first);
    }
    return names;
}

MetricsView
MetricsAdminI::getMetricsView(const string& view, const Current&)
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
MetricsAdminI::getMapMetricsFailures(const string& view, const string& map, const Current&)
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
MetricsAdminI::getMetricsFailures(const string& view, const string& map, const string& id, const Current&)
{
    Lock sync(*this);
    std::map<string, MetricsViewIPtr>::const_iterator p = _views.find(view);
    if(p == _views.end())
    {
        throw UnknownMetricsView();
    }
    return p->second->getFailures(map, id);
}

vector<MetricsMapIPtr> 
MetricsAdminI::getMaps(const string& mapName) const
{
    Lock sync(*this);
    vector<MetricsMapIPtr> maps;
    for(std::map<string, MetricsViewIPtr>::const_iterator p = _views.begin(); p != _views.end(); ++p)
    {
        MetricsMapIPtr map = p->second->getMap(mapName);
        if(map)
        {
            maps.push_back(map);
        }
    }
    return maps;
}

const LoggerPtr&
MetricsAdminI::getLogger() const
{
    return _logger;
}

void
MetricsAdminI::setProperties(const Ice::PropertiesPtr& properties)
{
    _properties = properties;
}

void 
MetricsAdminI::updated(const PropertyDict& props)
{
    for(PropertyDict::const_iterator p = props.begin(); p != props.end(); ++p)
    {
        if(p->first.find("IceMX.") == 0)
        {
            // Udpate the metrics views using the new configuration.
            try
            {
                updateViews();
            }
            catch(const std::exception& ex)
            {
                Ice::Warning warn(_logger);
                warn << "unexpected exception while updating metrics view configuration:\n" << ex.what();
            }
            return;
        }
    }
}

bool
MetricsAdminI::addOrUpdateMap(const std::string& mapName, const MetricsMapFactoryPtr& factory)
{
    bool updated = false;
    for(std::map<string, MetricsViewIPtr>::const_iterator p = _views.begin(); p != _views.end(); ++p)
    {
        updated |= p->second->addOrUpdateMap(_properties, mapName, factory, _logger);
    }
    return updated;
}

bool
MetricsAdminI::removeMap(const std::string& mapName)
{
    bool updated = false;
    for(std::map<string, MetricsViewIPtr>::const_iterator p = _views.begin(); p != _views.end(); ++p)
    {
        updated |= p->second->removeMap(mapName);
    }
    return updated;
}
