// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

const string suffixes[] =
{
    "Disabled",
    "GroupBy",
    "Accept.*",
    "Reject.*",
    "RetainDetached",
    "Map.*",
};

void
validateProperties(const string& prefix, const PropertiesPtr& properties)
{
    vector<string> unknownProps;
    PropertyDict props = properties->getPropertiesForPrefix(prefix);
    for(PropertyDict::const_iterator p = props.begin(); p != props.end(); ++p)
    {
        bool valid = false;
        for(size_t i = 0; i < sizeof(suffixes) / sizeof(*suffixes); ++i)
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

    if(!unknownProps.empty() && properties->getPropertyAsIntWithDefault("Ice.Warn.UnknownProperties", 1) > 0)
    {
        Warning out(getProcessLogger());
        out << "found unknown IceMX properties for '" << prefix.substr(0, prefix.size() - 1) << "':";
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
#ifdef __MINGW32__
    //
    // No regexp support with MinGW, when MinGW C++11 mode is not experimental
    // we can use std::regex.
    //
#elif !defined(ICE_CPP11_REGEXP)
    if(regcomp(&_preg, regexp.c_str(), REG_EXTENDED | REG_NOSUB) != 0)
    {
        throw SyscallException(__FILE__, __LINE__); 
    }
#else
#   if _MSC_VER < 1600
    _regex = std::tr1::regex(regexp, std::tr1::regex_constants::extended | std::tr1::regex_constants::nosubs);
#   else
    _regex = regex(regexp, std::regex_constants::extended | std::regex_constants::nosubs);
#   endif
#endif
}

MetricsMapI::RegExp::~RegExp()
{
#ifdef __MINGW32__
    //
    // No regexp support with MinGW, when MinGW C++11 mode is not experimental
    // we can use std::regex.
    //
#elif !defined(ICE_CPP11_REGEXP)
    regfree(&_preg);
#endif
}

bool
MetricsMapI::RegExp::match(const string& value)
{
#ifdef __MINGW32__
    //
    // No regexp support with MinGW, when MinGW C++11 mode is not experimental
    // we can use std::regex.
    //
    return false;
#elif !defined(ICE_CPP11_REGEXP)
    return regexec(&_preg, value.c_str(), 0, 0, 0) == 0;
#else
#   if _MSC_VER < 1600
    return std::tr1::regex_match(value, _regex);
#   else
    return regex_match(value, _regex);
#   endif
#endif
}

MetricsMapI::MetricsMapI(const std::string& mapPrefix, const PropertiesPtr& properties) :
    _properties(properties->getPropertiesForPrefix(mapPrefix)),
    _retain(properties->getPropertyAsIntWithDefault(mapPrefix + "RetainDetached", 10)),
    _accept(parseRule(properties, mapPrefix + "Accept")),
    _reject(parseRule(properties, mapPrefix + "Reject"))
{
    validateProperties(mapPrefix, properties);

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

const ::Ice::PropertyDict&
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

void
MetricsViewI::destroy()
{
    for(map<string, MetricsMapIPtr>::const_iterator p = _maps.begin(); p != _maps.end(); ++p)
    {
        p->second->destroy();
    }
}

bool
MetricsViewI::addOrUpdateMap(const PropertiesPtr& properties, const string& mapName, 
                             const MetricsMapFactoryPtr& factory, const ::Ice::LoggerPtr& logger)
{
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
            map<string, MetricsMapIPtr>::iterator q = _maps.find(mapName);
            if(q != _maps.end())
            {
                q->second->destroy();
                _maps.erase(q);
                return true;
            }
            return false;
        }
    }
    else
    {
        mapPrefix = viewPrefix;
        mapProps = properties->getPropertiesForPrefix(mapPrefix);
    }

    if(properties->getPropertyAsInt(mapPrefix + "Disabled") > 0)
    {
        // This map is disabled for this view.
        map<string, MetricsMapIPtr>::iterator q = _maps.find(mapName);
        if(q != _maps.end())
        {
            q->second->destroy();
            _maps.erase(q);
            return true;
        }
        return false;
    }

    map<string, MetricsMapIPtr>::iterator q = _maps.find(mapName);
    if(q != _maps.end() && q->second->getProperties() == mapProps)
    {
        return false; // The map configuration didn't change, no need to re-create.
    }

    if(q != _maps.end())
    {
        // Destroy the previous map
        q->second->destroy();
        _maps.erase(q);
    }

    try
    {
        _maps.insert(make_pair(mapName, factory->create(mapPrefix, properties)));
    }
    catch(const std::exception& ex)
    {
        ::Ice::Warning warn(logger);
        warn << "unexpected exception while creating metrics map:\n" << ex;
    }
    catch(const string& msg)
    {
        ::Ice::Warning warn(logger);
        warn << msg;
    }
    return true;
}

bool
MetricsViewI::removeMap(const string& mapName)
{
    map<string, MetricsMapIPtr>::iterator q = _maps.find(mapName);
    if(q != _maps.end())
    {
        q->second->destroy();
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

MetricsAdminI::~MetricsAdminI()
{
}

void
MetricsAdminI::destroy()
{
    Lock sync(*this);
    for(map<string, MetricsViewIPtr>::const_iterator p = _views.begin(); p != _views.end(); ++p)
    {
        p->second->destroy();
    }
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
        _disabledViews.clear();
        for(PropertyDict::const_iterator p = viewsProps.begin(); p != viewsProps.end(); ++p)
        {
            string viewName = p->first.substr(viewsPrefix.size());
            string::size_type dotPos = viewName.find('.');
            if(dotPos != string::npos)
            {
                viewName = viewName.substr(0, dotPos);
            }

            if(views.find(viewName) != views.end() || _disabledViews.find(viewName) != _disabledViews.end())
            {
                continue; // View already configured.
            }

            validateProperties(viewsPrefix + viewName + ".", _properties);

            if(_properties->getPropertyAsIntWithDefault(viewsPrefix + viewName + ".Disabled", 0) > 0)
            {
                _disabledViews.insert(viewName);
                continue; // The view is disabled
            }

            //
            // Create the view or update it.
            //
            map<string, MetricsViewIPtr>::const_iterator q = _views.find(viewName);
            if(q == _views.end())
            {
                q = views.insert(map<string, MetricsViewIPtr>::value_type(viewName, new MetricsViewI(viewName))).first;
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
                p->second->destroy();
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

Ice::StringSeq
MetricsAdminI::getMetricsViewNames(Ice::StringSeq& disabledViews, const Current&)
{
    Ice::StringSeq enabledViews;

    Lock sync(*this);
    for(map<string, MetricsViewIPtr>::const_iterator p = _views.begin(); p != _views.end(); ++p)
    {
        enabledViews.push_back(p->first);
    }

#if defined(__SUNPRO_CC) && defined(_RWSTD_NO_MEMBER_TEMPLATES)
    for(set<string>::const_iterator p = _disabledViews.begin(); p != _disabledViews.end(); ++p)
    {
        disabledViews.push_back(*p);
    }

#else
    disabledViews.insert(disabledViews.end(), _disabledViews.begin(), _disabledViews.end());
#endif    

    return enabledViews;
}

void
MetricsAdminI::enableMetricsView(const string& viewName, const Current&)
{
    {
        Lock sync(*this);
        getMetricsView(viewName); // Throws if unkonwn metrics view.
        _properties->setProperty("IceMX.Metrics." + viewName + ".Disabled", "0");
    }
    updateViews();
}

void
MetricsAdminI::disableMetricsView(const string& viewName, const Current&)
{
    {
        Lock sync(*this);
        getMetricsView(viewName); // Throws if unkonwn metrics view.
        _properties->setProperty("IceMX.Metrics." + viewName + ".Disabled", "1");
    }
    updateViews();
}

MetricsView
MetricsAdminI::getMetricsView(const string& viewName, ::Ice::Long& timestamp, const Current&)
{
    Lock sync(*this);
    MetricsViewIPtr view = getMetricsView(viewName);
    timestamp = IceUtil::Time::now().toMilliSeconds();
    if(view)
    {
        return view->getMetrics();
    }
    return MetricsView();
}

MetricsFailuresSeq
MetricsAdminI::getMapMetricsFailures(const string& viewName, const string& map, const Current&)
{
    Lock sync(*this);
    MetricsViewIPtr view = getMetricsView(viewName);
    if(view)
    {
        return view->getFailures(map);
    }
    return MetricsFailuresSeq();
}

MetricsFailures
MetricsAdminI::getMetricsFailures(const string& viewName, const string& map, const string& id, const Current&)
{
    Lock sync(*this);
    MetricsViewIPtr view = getMetricsView(viewName);
    if(view)
    {
        return view->getFailures(map, id);
    }
    return MetricsFailures();
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

MetricsViewIPtr
MetricsAdminI::getMetricsView(const std::string& name)
{
    std::map<string, MetricsViewIPtr>::const_iterator p = _views.find(name);
    if(p == _views.end())
    {
        if(_disabledViews.find(name) == _disabledViews.end())
        {
            throw UnknownMetricsView();
        }
        return 0;
    }
    return p->second;
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
                ::Ice::Warning warn(_logger);
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
