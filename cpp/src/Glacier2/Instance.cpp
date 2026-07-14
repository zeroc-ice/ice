// Copyright (c) ZeroC, Inc.

#include "Instance.h"
#include "../Ice/InstrumentationI.h"
#include "Ice/StringUtil.h"
#include "InstrumentationI.h"
#include "SessionRouterI.h"

using namespace std;
using namespace Ice;
using namespace Glacier2;

//
// Parse a space delimited string into a sequence of strings.
//

static vector<string>
stringToStringSeq(const string& str)
{
    vector<string> seq;
    IceInternal::splitString(str, " \t", seq);

    //
    // TODO: do something about unmatched quotes
    //
    return seq;
}

static vector<Identity>
stringToIdentitySeq(const string& str)
{
    vector<Identity> seq;
    string const ws = " \t";

    //
    // Eat white space.
    //
    string::size_type current = str.find_first_not_of(ws, 0);
    string::size_type end = 0;
    while (current != string::npos)
    {
        switch (str[current])
        {
            case '"':
            case '\'':
            {
                char quote = str[current];
                end = current + 1;
                while (true)
                {
                    end = str.find(quote, end);

                    if (end == string::npos)
                    {
                        //
                        // TODO: should this be an unmatched quote error?
                        //
                        seq.push_back(stringToIdentity(str.substr(current)));
                        break;
                    }

                    bool markString = true;
                    for (string::size_type r = end - 1; r > current && str[r] == '\\'; --r)
                    {
                        markString = !markString;
                    }
                    //
                    // We don't want the quote so we skip that.
                    //
                    if (markString)
                    {
                        ++current;
                        seq.push_back(stringToIdentity(str.substr(current, end - current)));
                        break;
                    }
                    else
                    {
                        ++end;
                    }
                }
                if (end != string::npos)
                {
                    ++end;
                }
                break;
            }

            default:
            {
                end = str.find_first_of(ws, current);
                string::size_type len = (end == string::npos) ? string::npos : end - current;
                seq.push_back(stringToIdentity(str.substr(current, len)));
                break;
            }
        }
        current = str.find_first_not_of(ws, end);
    }
    return seq;
}

static vector<Identity>
parseFilterIdentities(const PropertiesPtr& properties)
{
    try
    {
        return stringToIdentitySeq(properties->getIceProperty("Glacier2.Filter.Identity.Accept"));
    }
    catch (const std::exception& ex)
    {
        throw InitializationException(
            __FILE__,
            __LINE__,
            "invalid 'Glacier2.Filter.Identity.Accept' property:\n" + string{ex.what()});
    }
}

static int
parseFilterAddUserMode(const PropertiesPtr& properties)
{
    try
    {
        return properties->getIcePropertyAsInt("Glacier2.Filter.Category.AcceptUser");
    }
    catch (const std::exception& ex)
    {
        throw InitializationException(__FILE__, __LINE__, string{ex.what()});
    }
}

Glacier2::Instance::Instance(
    shared_ptr<Ice::Communicator> communicator,
    Ice::ObjectAdapterPtr clientAdapter,
    Ice::ObjectAdapterPtr serverAdapter)
    : _communicator(std::move(communicator)),
      _properties(_communicator->getProperties()),
      _logger(_communicator->getLogger()),
      _clientAdapter(std::move(clientAdapter)),
      _serverAdapter(std::move(serverAdapter)),
      _proxyVerifier(make_shared<ProxyVerifier>(_communicator)),
      _routingTableMaxSize(_properties->getIcePropertyAsInt("Glacier2.RoutingTable.MaxSize")),
      _filterCategories(stringToStringSeq(_properties->getIceProperty("Glacier2.Filter.Category.Accept"))),
      _filterAdapterIds(stringToStringSeq(_properties->getIceProperty("Glacier2.Filter.AdapterId.Accept"))),
      _filterIdentities(parseFilterIdentities(_properties)),
      _filterAddUserMode(parseFilterAddUserMode(_properties))
{
    if (_routingTableMaxSize < 1)
    {
        throw Ice::InitializationException(
            __FILE__,
            __LINE__,
            "invalid value for Glacier2.RoutingTable.MaxSize: " + to_string(_routingTableMaxSize));
    }

    //
    // If an Ice metrics observer is setup on the communicator, also enable metrics for Glacier2.
    //
    auto o = dynamic_pointer_cast<IceInternal::CommunicatorObserverI>(_communicator->getObserver());
    if (o)
    {
        const_cast<shared_ptr<Instrumentation::RouterObserver>&>(_observer) =
            make_shared<RouterObserverI>(o->getFacet(), _properties->getIceProperty("Glacier2.InstanceName"));
    }
}

void
Glacier2::Instance::destroy()
{
    _sessionRouter = nullptr;
}

void
Glacier2::Instance::setSessionRouter(shared_ptr<SessionRouterI> sessionRouter)
{
    assert(_sessionRouter == nullptr);
    _sessionRouter = std::move(sessionRouter);
}
