// Copyright (c) ZeroC, Inc.

#include "MetricsAdminI.h"
#include "Ice/Logger.h"
#include "Ice/LoggerUtil.h"
#include "Ice/Properties.h"
#include "Ice/StringUtil.h"
#include "MetricsObserverI.h"

#include <chrono>
#include <stdexcept>

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace IceMX;

namespace
{
    // NOLINTBEGIN(cert-err58-cpp)
    const string suffixes[] = {
        "Disabled",
        "GroupBy",
        "Accept.*",
        "Reject.*",
        "RetainDetached",
        "Map.*",
    };
    // NOLINTEND(cert-err58-cpp)

    void validateProperties(const string& prefix, const PropertiesPtr& properties)
    {
        vector<string> unknownProps;
        PropertyDict props = properties->getPropertiesForPrefix(prefix);
        for (const auto& p : props)
        {
            bool valid = false;
            for (const auto& suffix : suffixes)
            {
                string prop = prefix + suffix;
                if (IceInternal::match(p.first, prop))
                {
                    valid = true;
                    break;
                }
            }
            if (!valid)
            {
                unknownProps.push_back(p.first);
            }
        }

        if (!unknownProps.empty())
        {
            ostringstream os;
            os << "found unknown properties for " << "IceMX" << ": '" << prefix << "'";
            for (const auto& prop : unknownProps)
            {
                os << "\n    " << prop;
            }
            throw PropertyException{__FILE__, __LINE__, os.str()};
        }
    }

    vector<MetricsMapI::RegExpPtr> parseRule(const PropertiesPtr& properties, const string& name)
    {
        vector<MetricsMapI::RegExpPtr> regexps;
        PropertyDict rules = properties->getPropertiesForPrefix(name + '.');
        for (const auto& rule : rules)
        {
            try
            {
                regexps.push_back(make_shared<MetricsMapI::RegExp>(rule.first.substr(name.length() + 1), rule.second));
            }
            catch (const std::exception&)
            {
                throw invalid_argument("invalid regular expression '" + rule.second + "' for '" + rule.first + "'");
            }
        }
        return regexps;
    }
}

MetricsMapI::RegExp::RegExp(string attribute, const string& regexp) : _attribute(std::move(attribute))
{
    _regex = regex(regexp, std::regex_constants::extended | std::regex_constants::nosubs);
}

bool
MetricsMapI::RegExp::match(const string& value)
{
    return regex_match(value, _regex);
}

MetricsMapI::~MetricsMapI() = default; // Out of line to avoid weak vtable

MetricsMapI::MetricsMapI(const std::string& mapPrefix, const PropertiesPtr& properties)
    : _properties(properties->getPropertiesForPrefix(mapPrefix)),
      _retain(properties->getIcePropertyAsInt(mapPrefix + "RetainDetached")),
      _accept(parseRule(properties, mapPrefix + "Accept")),
      _reject(parseRule(properties, mapPrefix + "Reject"))
{
    validateProperties(mapPrefix, properties);

    string groupBy = properties->getPropertyWithDefault(mapPrefix + "GroupBy", "id");
    auto& groupByAttributes = const_cast<vector<string>&>(_groupByAttributes);
    auto& groupBySeparators = const_cast<vector<string>&>(_groupBySeparators);
    if (!groupBy.empty())
    {
        string v;
        bool attribute = IceInternal::isAlpha(groupBy[0]) || IceInternal::isDigit(groupBy[0]);
        if (!attribute)
        {
            groupByAttributes.emplace_back("");
        }

        for (char p : groupBy)
        {
            bool isAlphaNum = IceInternal::isAlpha(p) || IceInternal::isDigit(p) || p == '.';
            if (attribute && !isAlphaNum)
            {
                groupByAttributes.push_back(v);
                v = p;
                attribute = false;
            }
            else if (!attribute && isAlphaNum)
            {
                groupBySeparators.push_back(v);
                v = p;
                attribute = true;
            }
            else
            {
                v += p;
            }
        }

        if (attribute)
        {
            groupByAttributes.push_back(v);
        }
        else
        {
            groupBySeparators.push_back(v);
        }
    }
}

MetricsMapI::MetricsMapI(const MetricsMapI& map)
    : std::enable_shared_from_this<MetricsMapI>(),
      _properties(map._properties),
      _groupByAttributes(map._groupByAttributes),
      _groupBySeparators(map._groupBySeparators),
      _retain(map._retain),
      _accept(map._accept),
      _reject(map._reject)
{
}

const Ice::PropertyDict&
MetricsMapI::getProperties() const
{
    return _properties;
}

MetricsMapFactory::~MetricsMapFactory() = default; // Out of line to avoid weak vtable

MetricsMapFactory::MetricsMapFactory(Updater* updater) : _updater(updater) {}

void
MetricsMapFactory::update()
{
    assert(_updater);
    _updater->update();
}

MetricsViewI::MetricsViewI(string name) : _name(std::move(name)) {}

void
MetricsViewI::destroy()
{
    for (const auto& map : _maps)
    {
        map.second->destroy();
    }
}

bool
MetricsViewI::addOrUpdateMap(
    const PropertiesPtr& properties,
    const string& mapName,
    const MetricsMapFactoryPtr& factory,
    const Ice::LoggerPtr& logger)
{
    const string viewPrefix = "IceMX.Metrics." + _name + ".";
    const string mapsPrefix = viewPrefix + "Map.";
    PropertyDict mapsProps = properties->getPropertiesForPrefix(mapsPrefix);

    string mapPrefix;
    PropertyDict mapProps;
    if (!mapsProps.empty())
    {
        mapPrefix = mapsPrefix + mapName + ".";
        mapProps = properties->getPropertiesForPrefix(mapPrefix);
        if (mapProps.empty())
        {
            // This map isn't configured for this view.
            auto q = _maps.find(mapName);
            if (q != _maps.end())
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

    if (properties->getPropertyAsInt(mapPrefix + "Disabled") > 0)
    {
        // This map is disabled for this view.
        auto q = _maps.find(mapName);
        if (q != _maps.end())
        {
            q->second->destroy();
            _maps.erase(q);
            return true;
        }
        return false;
    }

    auto q = _maps.find(mapName);
    if (q != _maps.end() && q->second->getProperties() == mapProps)
    {
        return false; // The map configuration didn't change, no need to re-create.
    }

    if (q != _maps.end())
    {
        // Destroy the previous map
        q->second->destroy();
        _maps.erase(q);
    }

    try
    {
        _maps.insert(make_pair(mapName, factory->create(mapPrefix, properties)));
    }
    catch (const std::exception& ex)
    {
        Ice::Warning warn(logger);
        warn << "unexpected exception while creating metrics map:\n" << ex;
    }
    return true;
}

bool
MetricsViewI::removeMap(const string& mapName)
{
    auto q = _maps.find(mapName);
    if (q != _maps.end())
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
    for (const auto& map : _maps)
    {
        metrics.insert(make_pair(map.first, map.second->getMetrics()));
    }
    return metrics;
}

MetricsFailuresSeq
MetricsViewI::getFailures(const string& mapName)
{
    auto p = _maps.find(mapName);
    if (p != _maps.end())
    {
        return p->second->getFailures();
    }
    return {};
}

MetricsFailures
MetricsViewI::getFailures(const string& mapName, const string& id)
{
    auto p = _maps.find(mapName);
    if (p != _maps.end())
    {
        return p->second->getFailures(id);
    }
    return {};
}

vector<string>
MetricsViewI::getMaps() const
{
    vector<string> maps;
    maps.reserve(_maps.size());
    for (const auto& map : _maps)
    {
        maps.push_back(map.first);
    }
    return maps;
}

MetricsMapIPtr
MetricsViewI::getMap(const string& mapName) const
{
    auto p = _maps.find(mapName);
    if (p != _maps.end())
    {
        return p->second;
    }
    return nullptr;
}

MetricsAdminI::MetricsAdminI(PropertiesPtr properties, LoggerPtr logger)
    : _logger(std::move(logger)),
      _properties(std::move(properties))
{
    updateViews();
}

MetricsAdminI::~MetricsAdminI() = default;

void
MetricsAdminI::destroy()
{
    lock_guard lock(_mutex);
    for (const auto& view : _views)
    {
        view.second->destroy();
    }
}

void
MetricsAdminI::updateViews()
{
    set<MetricsMapFactoryPtr> updatedMaps;
    {
        lock_guard lock(_mutex);
        const string viewsPrefix = "IceMX.Metrics.";
        PropertyDict viewsProps = _properties->getPropertiesForPrefix(viewsPrefix);
        map<string, MetricsViewIPtr> views;
        _disabledViews.clear();
        for (const auto& viewsProp : viewsProps)
        {
            string viewName = viewsProp.first.substr(viewsPrefix.size());
            string::size_type dotPos = viewName.find('.');
            if (dotPos != string::npos)
            {
                viewName = viewName.substr(0, dotPos);
            }

            if (views.find(viewName) != views.end() || _disabledViews.find(viewName) != _disabledViews.end())
            {
                continue; // View already configured.
            }

            validateProperties(viewsPrefix + viewName + ".", _properties);

            if (_properties->getPropertyAsIntWithDefault(viewsPrefix + viewName + ".Disabled", 0) > 0)
            {
                _disabledViews.insert(viewName);
                continue; // The view is disabled
            }

            //
            // Create the view or update it.
            //
            auto q = _views.find(viewName);
            if (q == _views.end())
            {
                q = views
                        .insert(map<string, MetricsViewIPtr>::value_type(viewName, make_shared<MetricsViewI>(viewName)))
                        .first;
            }
            else
            {
                q = views.insert(make_pair(viewName, q->second)).first;
            }

            for (const auto& fact : _factories)
            {
                if (q->second->addOrUpdateMap(_properties, fact.first, fact.second, _logger))
                {
                    updatedMaps.insert(fact.second);
                }
            }
        }
        _views.swap(views);

        //
        // Go through removed views to collect maps to update.
        //
        for (const auto& view : views)
        {
            if (_views.find(view.first) == _views.end())
            {
                vector<string> maps = view.second->getMaps();
                for (const auto& map : maps)
                {
                    updatedMaps.insert(_factories[map]);
                }
                view.second->destroy();
            }
        }
    }

    //
    // Call the updaters to update the maps.
    //
    for (const auto& updatedMap : updatedMaps)
    {
        updatedMap->update();
    }
}

void
MetricsAdminI::unregisterMap(const std::string& mapName)
{
    bool updated;
    MetricsMapFactoryPtr factory;
    {
        lock_guard lock(_mutex);
        auto p = _factories.find(mapName);
        if (p == _factories.end())
        {
            return;
        }
        factory = p->second;
        _factories.erase(p);
        updated = removeMap(mapName);
    }
    if (updated)
    {
        factory->update();
    }
}

Ice::StringSeq
MetricsAdminI::getMetricsViewNames(Ice::StringSeq& disabledViews, const Current&)
{
    Ice::StringSeq enabledViews;

    lock_guard lock(_mutex);
    for (const auto& view : _views)
    {
        enabledViews.push_back(view.first);
    }
    disabledViews.insert(disabledViews.end(), _disabledViews.begin(), _disabledViews.end());
    return enabledViews;
}

void
MetricsAdminI::enableMetricsView(string viewName, const Current&)
{
    {
        lock_guard lock(_mutex);
        getMetricsView(viewName); // Throws if unknown metrics view.
        _properties->setProperty("IceMX.Metrics." + viewName + ".Disabled", "0");
    }
    updateViews();
}

void
MetricsAdminI::disableMetricsView(string viewName, const Current&)
{
    {
        lock_guard lock(_mutex);
        getMetricsView(viewName); // Throws if unknown metrics view.
        _properties->setProperty("IceMX.Metrics." + viewName + ".Disabled", "1");
    }
    updateViews();
}

MetricsView
MetricsAdminI::getMetricsView(string viewName, int64_t& timestamp, const Current&)
{
    lock_guard lock(_mutex);
    MetricsViewIPtr view = getMetricsView(viewName);
    timestamp =
        chrono::duration_cast<chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (view)
    {
        return view->getMetrics();
    }
    return {};
}

MetricsFailuresSeq
MetricsAdminI::getMapMetricsFailures(string viewName, string map, const Current&)
{
    lock_guard lock(_mutex);
    MetricsViewIPtr view = getMetricsView(viewName);
    if (view)
    {
        return view->getFailures(map);
    }
    return {};
}

MetricsFailures
MetricsAdminI::getMetricsFailures(string viewName, string map, string id, const Current&)
{
    lock_guard lock(_mutex);
    MetricsViewIPtr view = getMetricsView(viewName);
    if (view)
    {
        return view->getFailures(map, id);
    }
    return {};
}

vector<MetricsMapIPtr>
MetricsAdminI::getMaps(const string& mapName) const
{
    lock_guard lock(_mutex);
    vector<MetricsMapIPtr> maps;
    for (const auto& view : _views)
    {
        MetricsMapIPtr map = view.second->getMap(mapName);
        if (map)
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
    auto p = _views.find(name);
    if (p == _views.end())
    {
        if (_disabledViews.find(name) == _disabledViews.end())
        {
            throw UnknownMetricsView();
        }
        return nullptr;
    }
    return p->second;
}

void
MetricsAdminI::updated(const PropertyDict& props)
{
    for (const auto& prop : props)
    {
        if (prop.first.find("IceMX.") == 0)
        {
            // Update the metrics views using the new configuration.
            try
            {
                updateViews();
            }
            catch (const std::exception& ex)
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
    for (const auto& view : _views)
    {
        updated |= view.second->addOrUpdateMap(_properties, mapName, factory, _logger);
    }
    return updated;
}

bool
MetricsAdminI::removeMap(const std::string& mapName)
{
    bool updated = false;
    for (const auto& view : _views)
    {
        updated |= view.second->removeMap(mapName);
    }
    return updated;
}
