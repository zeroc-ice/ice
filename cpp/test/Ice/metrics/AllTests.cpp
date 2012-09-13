// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;

namespace
{

// void
// printMetricsView(const IceMX::MetricsView& view)
// {
//     for(IceMX::MetricsView::const_iterator q = view.begin(); q != view.end(); ++q)
//     {
//         cout << endl << q->first << " Map:" << endl;
//         for(IceMX::MetricsMap::const_iterator p = q->second.begin(); p != q->second.end(); ++p)
//         {
//             cout << (*p)->id << " current = " << (*p)->current << " total = " << (*p)->total << " failures = "
//                  << (*p)->failures << endl;
//         }
//     }
// }

Ice::PropertyDict
getClientProps(const Ice::PropertiesAdminPrx& p, const Ice::PropertyDict& orig, const string& m = string()) 
{
    Ice::PropertyDict props = p->getPropertiesForPrefix("IceMX.Metrics");
    for(Ice::PropertyDict::iterator p = props.begin(); p != props.end(); ++p)
    {
        p->second = "";
    }
    for(Ice::PropertyDict::const_iterator p = orig.begin(); p != orig.end(); ++p)
    {
        props[p->first] = p->second;
    }
    string map;
    if(!m.empty())
    {
        map += "Map." + m + '.';
    }
    props["IceMX.Metrics.View." + map + "Reject.parent"] = "Ice\\.Admin";
    props["IceMX.Metrics.View." + map + "Accept.endpointPort"] = "12010";
    props["IceMX.Metrics.View." + map + "Reject.identity"] = ".*/admin|controller";
    return props;
}

Ice::PropertyDict
getServerProps(const Ice::PropertiesAdminPrx& p, const Ice::PropertyDict& orig, const string& m = string())
{
    Ice::PropertyDict props = p->getPropertiesForPrefix("IceMX.Metrics");
    for(Ice::PropertyDict::iterator p = props.begin(); p != props.end(); ++p)
    {
        p->second = "";
    }
    for(Ice::PropertyDict::const_iterator p = orig.begin(); p != orig.end(); ++p)
    {
        props[p->first] = p->second;
    }
    string map;
    if(!m.empty())
    {
        map += "Map." + m + '.';
    }
    props["IceMX.Metrics.View." + map + "Reject.parent"] = "Ice\\.Admin|Controller";
    props["IceMX.Metrics.View." + map + "Accept.endpointPort"] = "12010";
    return props;
}

class UpdateCallbackI : public Ice::PropertiesAdminUpdateCallback, private IceUtil::Monitor<IceUtil::Mutex>
{
public:

    UpdateCallbackI(const Ice::PropertiesAdminPrx& serverProps) : _updated(false), _serverProps(serverProps)
    {
    }

    void
    waitForUpdate()
    {
        Lock sync(*this);
        while(!_updated)
        {
            wait();
        }
        // Ensure that the previous updates were committed, the setProperties call returns before 
        // notifying the callbacks so to ensure all the update callbacks have be notified we call
        // a second time, this will block until all the notifications from the first update have
        // completed.
        _serverProps->setProperties(Ice::PropertyDict()); 
        _updated = false;
    }

    void
    updated(const Ice::PropertyDict& dict)
    {
        Lock sync(*this);
        _updated = true;
        notify();
    }

private:

    bool _updated;
    Ice::PropertiesAdminPrx _serverProps;
};

void
waitForCurrent(const IceMX::MetricsAdminPrx& metrics, const string& viewName, const string& map, int value)
{
    while(true)
    {
        IceMX::MetricsView view = metrics->getMetricsView(viewName);
        test(view.find(map) != view.end());
        bool ok = true;
        for(IceMX::MetricsMap::const_iterator m = view[map].begin(); m != view[map].end(); ++m)
        {
            if((*m)->current != value)
            {
                ok = false;
                break;
            }
        }
        if(ok)
        {
            break;
        }
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(50));
    }
}

template<typename T> void
testAttribute(const IceMX::MetricsAdminPrx& metrics, 
              const Ice::PropertiesAdminPrx& props, 
              UpdateCallbackI* update,
              const string& map, 
              const string& attr,
              const string& value,
              const T& func)
{
    Ice::PropertyDict dict;
    dict["IceMX.Metrics.View.Map." + map + ".GroupBy"] = attr;
    if(props->ice_getIdentity().category == "client")
    {
        props->setProperties(getClientProps(props, dict, map));
        update->waitForUpdate();
    }
    else
    {
        props->setProperties(getServerProps(props, dict, map));
        props->setProperties(Ice::PropertyDict());
    }

    func();

    IceMX::MetricsView view = metrics->getMetricsView("View");
    test(view.find(map) != view.end());
    if(view[map][0]->id != value)
    {
        cerr << "invalid attribute value: " << attr << " = " << value << " got " << view[map][0]->id << endl;
        test(false);
    }

    dict.clear();
    if(props->ice_getIdentity().category == "client")
    {
        props->setProperties(getClientProps(props, dict, map));
        update->waitForUpdate();
    }
    else
    {
        props->setProperties(getServerProps(props, dict, map));
        props->setProperties(Ice::PropertyDict());
    }
}

struct Void
{
    void 
    operator()() const
    {
    }
};

struct Connect
{
    Connect(const Ice::ObjectPrx& proxy) : proxy(proxy)
    {
    }

    void
    operator()() const
    {
        if(proxy->ice_getCachedConnection())
        {
            proxy->ice_getCachedConnection()->close(false);
        }
        try
        {
            proxy->ice_ping();
        }
        catch(const Ice::LocalException&)
        {
        }
        if(proxy->ice_getCachedConnection())
        {
            proxy->ice_getCachedConnection()->close(false);
        }
    }

    Ice::ObjectPrx proxy;
};

struct InvokeOp
{
    InvokeOp(const Test::MetricsPrx& proxy) : proxy(proxy)
    {
    }

    void
    operator()() const
    {
        Ice::Context ctx;
        ctx["entry1"] = "test";
        ctx["entry2"] = "";
        proxy->op(ctx);
    }

    Test::MetricsPrx proxy;
};

void
testAttribute(const IceMX::MetricsAdminPrx& metrics, 
              const Ice::PropertiesAdminPrx& props, 
              UpdateCallbackI* update,
              const string& map, 
              const string& attr,
              const string& value)
{
    testAttribute(metrics, props, update, map, attr, value, Void());
}

void
updateProps(const Ice::PropertiesAdminPrx& cprops, 
            const Ice::PropertiesAdminPrx& sprops, 
            UpdateCallbackI* callback, 
            const Ice::PropertyDict& props,
            const string& map = string())
{
    cprops->setProperties(getClientProps(cprops, props, map));
    sprops->setProperties(getServerProps(sprops, props, map));
    callback->waitForUpdate();
}

void
clearView(const Ice::PropertiesAdminPrx& cprops, const Ice::PropertiesAdminPrx& sprops, UpdateCallbackI* callback)
{
    Ice::PropertyDict dict;

    dict = cprops->getPropertiesForPrefix("IceMX.Metrics");
    dict["IceMX.Metrics.View.Disabled"] = "1";
    cprops->setProperties(dict);

    dict = sprops->getPropertiesForPrefix("IceMX.Metrics");
    dict["IceMX.Metrics.View.Disabled"] = "1";
    sprops->setProperties(dict);

    callback->waitForUpdate();

    dict = cprops->getPropertiesForPrefix("IceMX.Metrics");
    dict["IceMX.Metrics.View.Disabled"] = "";
    cprops->setProperties(dict);

    dict = sprops->getPropertiesForPrefix("IceMX.Metrics");
    dict["IceMX.Metrics.View.Disabled"] = "";
    sprops->setProperties(dict);

    callback->waitForUpdate();
}

void
checkFailure(const IceMX::MetricsAdminPrx& m, const string& map, const string& id, const string& failure, int count)
{
    IceMX::MetricsFailures f = m->getMetricsFailures("View", map, id);
    if(f.failures.find(failure) == f.failures.end())
    {
        cerr << "couldn't find failure `" << failure << "' for `" << id << "'" << endl;
        test(false);
    }
    if(f.failures[failure] != count)
    {
        cerr << "count for failure `" << failure << "' of `" << id << "' is different from expected: ";
        cerr << count << " != " << f.failures[failure] << endl;
        test(false);
    }
}

}

MetricsPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    MetricsPrx metrics = MetricsPrx::checkedCast(communicator->stringToProxy("metrics:default -p 12010"));

    cout << "testing metrics admin facet checkedCast... " << flush;
    Ice::ObjectPrx admin = communicator->getAdmin()->ice_collocationOptimized(false);
    Ice::PropertiesAdminPrx clientProps = Ice::PropertiesAdminPrx::checkedCast(admin, "Properties");
    IceMX::MetricsAdminPrx clientMetrics = IceMX::MetricsAdminPrx::checkedCast(admin, "MetricsAdmin");
    test(clientProps && clientMetrics);

    admin = metrics->getAdmin();
    Ice::PropertiesAdminPrx serverProps = Ice::PropertiesAdminPrx::checkedCast(admin, "Properties");
    IceMX::MetricsAdminPrx serverMetrics = IceMX::MetricsAdminPrx::checkedCast(admin, "MetricsAdmin");
    test(serverProps && serverMetrics);

    UpdateCallbackI* update = new UpdateCallbackI(serverProps);
    Ice::NativePropertiesAdminPtr::dynamicCast(communicator->findAdminFacet("Properties"))->addUpdateCallback(update);

    cout << "ok" << endl;

    Ice::PropertyDict props;

    cout << "testing group by none..." << flush;

    props["IceMX.Metrics.View.GroupBy"] = "none";
    updateProps(clientProps, serverProps, update, props);

    IceMX::MetricsView view = clientMetrics->getMetricsView("View");
    test(view["Connection"].size() == 1 && view["Connection"][0]->current == 1 && view["Connection"][0]->total == 1);
    test(view["Thread"].size() == 1 && view["Thread"][0]->current == 4 && view["Thread"][0]->total == 4);
    cout << "ok" << endl;

    cout << "testing group by id..." << flush;

    props["IceMX.Metrics.View.GroupBy"] = "id";
    updateProps(clientProps, serverProps, update, props);

    metrics->ice_ping();
    metrics->ice_ping();
    metrics->ice_connectionId("Con1")->ice_ping();
    metrics->ice_connectionId("Con1")->ice_ping();
    metrics->ice_connectionId("Con1")->ice_ping();

    view = clientMetrics->getMetricsView("View");
    test(view["Thread"].size() == 4);
    test(view["Connection"].size() == 2);
    test(view["Invocation"].size() == 1);

    IceMX::InvocationMetricsPtr invoke = IceMX::InvocationMetricsPtr::dynamicCast(view["Invocation"][0]);
    test(invoke->id.find("[ice_ping]") > 0 && invoke->current == 0 && invoke->total == 5);
    test(invoke->remotes.size() == 2);
    test(invoke->remotes[0]->total = 2);
    test(invoke->remotes[1]->total = 3);

    view = serverMetrics->getMetricsView("View");
    test(view["Thread"].size() > 4);
    test(view["Connection"].size() == 2);
    test(view["Dispatch"].size() == 1);
    test(view["Dispatch"][0]->current == 0 && view["Dispatch"][0]->total == 5);
    test(view["Dispatch"][0]->id.find("[ice_ping]") > 0);

    metrics->ice_getConnection()->close(false);
    metrics->ice_connectionId("Con1")->ice_getConnection()->close(false);

    waitForCurrent(clientMetrics, "View", "Connection", 0);
    waitForCurrent(serverMetrics, "View", "Connection", 0);

    clearView(clientProps, serverProps, update);
    
    cout << "ok" << endl;

    cout << "testing connection metrics... " << flush;

    props["IceMX.Metrics.View.Map.Connection.GroupBy"] = "none";
    updateProps(clientProps, serverProps, update, props, "Connection");

    test(clientMetrics->getMetricsView("View")["Connection"].empty());
    test(serverMetrics->getMetricsView("View")["Connection"].empty());

    metrics->ice_ping();

    IceMX::ConnectionMetricsPtr cm1, sm1, cm2, sm2;
    cm1 = IceMX::ConnectionMetricsPtr::dynamicCast(clientMetrics->getMetricsView("View")["Connection"][0]);
    sm1 = IceMX::ConnectionMetricsPtr::dynamicCast(serverMetrics->getMetricsView("View")["Connection"][0]);
    test(cm1->total == 1 && sm1->total == 1);

    metrics->ice_ping();

    cm2 = IceMX::ConnectionMetricsPtr::dynamicCast(clientMetrics->getMetricsView("View")["Connection"][0]);
    sm2 = IceMX::ConnectionMetricsPtr::dynamicCast(serverMetrics->getMetricsView("View")["Connection"][0]);

    test(cm2->sentBytes - cm1->sentBytes == 45); // 45 for ice_ping request
    test(cm2->receivedBytes - cm1->receivedBytes == 25); // 25 bytes for ice_ping response
    test(sm2->receivedBytes - sm1->receivedBytes == 45);
    test(sm2->sentBytes - sm1->sentBytes == 25);

    cm1 = cm2;
    sm1 = sm2;

    Test::ByteSeq bs;
    metrics->opByteS(bs);

    cm2 = IceMX::ConnectionMetricsPtr::dynamicCast(clientMetrics->getMetricsView("View")["Connection"][0]);
    sm2 = IceMX::ConnectionMetricsPtr::dynamicCast(serverMetrics->getMetricsView("View")["Connection"][0]);
    int requestSz = cm2->sentBytes - cm1->sentBytes;
    int replySz = cm2->receivedBytes - cm1->receivedBytes;

    cm1 = cm2;
    sm1 = sm2;

    bs.resize(456);
    metrics->opByteS(bs);

    cm2 = IceMX::ConnectionMetricsPtr::dynamicCast(clientMetrics->getMetricsView("View")["Connection"][0]);
    sm2 = IceMX::ConnectionMetricsPtr::dynamicCast(serverMetrics->getMetricsView("View")["Connection"][0]);

    test(cm2->sentBytes - cm1->sentBytes == requestSz + bs.size() + 4); // 4 is for the seq variable size
    test(cm2->receivedBytes - cm1->receivedBytes == replySz);
    test(sm2->receivedBytes - sm1->receivedBytes == requestSz + bs.size() + 4);
    test(sm2->sentBytes - sm1->sentBytes == replySz);

    cm1 = cm2;
    sm1 = sm2;

    bs.resize(1024 * 1024 * 10); // Try with large amount of data which should be sent in several chunks
    metrics->opByteS(bs);

    cm2 = IceMX::ConnectionMetricsPtr::dynamicCast(clientMetrics->getMetricsView("View")["Connection"][0]);
    sm2 = IceMX::ConnectionMetricsPtr::dynamicCast(serverMetrics->getMetricsView("View")["Connection"][0]);

    test(cm2->sentBytes - cm1->sentBytes == requestSz + bs.size() + 4); // 4 is for the seq variable size
    test(cm2->receivedBytes - cm1->receivedBytes == replySz);
    test(sm2->receivedBytes - sm1->receivedBytes == requestSz + bs.size() + 4);
    test(sm2->sentBytes - sm1->sentBytes == replySz);

    sm1 = IceMX::ConnectionMetricsPtr::dynamicCast(serverMetrics->getMetricsView("View")["Connection"][0]);
    test(sm1->active == 1);

    ControllerPrx controller = ControllerPrx::checkedCast(communicator->stringToProxy("controller:default -p 12011"));
    controller->hold();

    cm1 = IceMX::ConnectionMetricsPtr::dynamicCast(clientMetrics->getMetricsView("View")["Connection"][0]);
    sm1 = IceMX::ConnectionMetricsPtr::dynamicCast(serverMetrics->getMetricsView("View")["Connection"][0]);
    test(cm1->active == 1 && sm1->holding == 1);

    metrics->ice_getConnection()->close(false);

    cm1 = IceMX::ConnectionMetricsPtr::dynamicCast(clientMetrics->getMetricsView("View")["Connection"][0]);
    sm1 = IceMX::ConnectionMetricsPtr::dynamicCast(serverMetrics->getMetricsView("View")["Connection"][0]);
    test(cm1->active == 0 && cm1->closing == 1 && sm1->active == 0 && sm1->holding == 1);

    controller->resume();

    sm1 = IceMX::ConnectionMetricsPtr::dynamicCast(serverMetrics->getMetricsView("View")["Connection"][0]);
    test(sm1->holding == 0);

    waitForCurrent(clientMetrics, "View", "Connection", 0);
    waitForCurrent(serverMetrics, "View", "Connection", 0);

    clearView(clientProps, serverProps, update);

    metrics->ice_timeout(500)->ice_ping();
    controller->hold();
    try
    {
        metrics->ice_timeout(500)->ice_ping();
        test(false);
    }
    catch(const Ice::TimeoutException&)
    {
    }
    controller->resume();

    cm1 = IceMX::ConnectionMetricsPtr::dynamicCast(clientMetrics->getMetricsView("View")["Connection"][0]);
    sm1 = IceMX::ConnectionMetricsPtr::dynamicCast(serverMetrics->getMetricsView("View")["Connection"][0]);
    test(cm1->failures == 2 && sm1->failures >= 1);

    checkFailure(clientMetrics, "Connection", cm1->id, "Ice::TimeoutException", 1);
    checkFailure(clientMetrics, "Connection", cm1->id, "Ice::ConnectTimeoutException", 1);
    checkFailure(serverMetrics, "Connection", sm1->id, "Ice::ConnectionLostException", 2);

    MetricsPrx m = metrics->ice_timeout(500)->ice_connectionId("Con1");
    m->ice_ping();

    testAttribute(clientMetrics, clientProps, update, "Connection", "parent", "Communicator");
    //testAttribute(clientMetrics, clientProps, update, "Connection", "id", "");
    testAttribute(clientMetrics, clientProps, update, "Connection", "endpoint",
                  "tcp -e 1.1 -h 127.0.0.1 -p 12010 -t 500");

    testAttribute(clientMetrics, clientProps, update, "Connection", "endpointType", "1");
    testAttribute(clientMetrics, clientProps, update, "Connection", "endpointIsDatagram", "false");
    testAttribute(clientMetrics, clientProps, update, "Connection", "endpointIsSecure", "false");
    testAttribute(clientMetrics, clientProps, update, "Connection", "endpointProtocolVersion", "1.0");
    testAttribute(clientMetrics, clientProps, update, "Connection", "endpointEncodingVersion", "1.1");
    testAttribute(clientMetrics, clientProps, update, "Connection", "endpointTimeout", "500");
    testAttribute(clientMetrics, clientProps, update, "Connection", "endpointCompress", "false");
    testAttribute(clientMetrics, clientProps, update, "Connection", "endpointHost", "127.0.0.1");
    testAttribute(clientMetrics, clientProps, update, "Connection", "endpointPort", "12010");

    testAttribute(clientMetrics, clientProps, update, "Connection", "incoming", "false");
    testAttribute(clientMetrics, clientProps, update, "Connection", "adapterName", "");
    testAttribute(clientMetrics, clientProps, update, "Connection", "connectionId", "Con1");
    testAttribute(clientMetrics, clientProps, update, "Connection", "localHost", "127.0.0.1");
    //testAttribute(clientMetrics, clientProps, update, "Connection", "localPort", "");
    testAttribute(clientMetrics, clientProps, update, "Connection", "remoteHost", "127.0.0.1");
    testAttribute(clientMetrics, clientProps, update, "Connection", "remotePort", "12010");
    testAttribute(clientMetrics, clientProps, update, "Connection", "mcastHost", "unknown");
    testAttribute(clientMetrics, clientProps, update, "Connection", "mcastPort", "unknown");
    
    m->ice_getConnection()->close(false);

    waitForCurrent(clientMetrics, "View", "Connection", 0);
    waitForCurrent(serverMetrics, "View", "Connection", 0);

    cout << "ok" << endl;

    cout << "testing connection establishment metrics... " << flush;

    props["IceMX.Metrics.View.Map.ConnectionEstablishment.GroupBy"] = "id";
    updateProps(clientProps, serverProps, update, props, "ConnectionEstablishment");
    test(clientMetrics->getMetricsView("View")["ConnectionEstablishment"].empty());

    metrics->ice_ping();
    
    test(clientMetrics->getMetricsView("View")["ConnectionEstablishment"].size() == 1);
    IceMX::MetricsPtr m1 = clientMetrics->getMetricsView("View")["ConnectionEstablishment"][0];
    test(m1->current == 0 && m1->total == 1 && m1->id == "127.0.0.1:12010");

    metrics->ice_getConnection()->close(false);

    try
    {
        communicator->stringToProxy("test:tcp -p 12010 -h 127.0.0.50")->ice_timeout(10)->ice_ping();
        test(false);
    }
    catch(const Ice::ConnectTimeoutException&)
    {
    }
    catch(const Ice::LocalException&)
    {
        test(false);
    }
    test(clientMetrics->getMetricsView("View")["ConnectionEstablishment"].size() == 2);
    m1 = clientMetrics->getMetricsView("View")["ConnectionEstablishment"][1];
    test(m1->id == "127.0.0.50:12010" && m1->total == 2 && m1->failures == 2);

    checkFailure(clientMetrics, "ConnectionEstablishment", m1->id, "Ice::ConnectTimeoutException", 2);

    Connect c(metrics);
    testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "parent", "Communicator", c);
    testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "id", "127.0.0.1:12010", c);
    testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpoint",  
                  "tcp -e 1.1 -h 127.0.0.1 -p 12010", c);

    testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointType", "1", c);
    testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointIsDatagram", "false", c);
    testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointIsSecure", "false", c);
    testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointProtocolVersion", "1.0", c);
    testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointEncodingVersion", "1.1", c);
    testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointTimeout", "-1", c);
    testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointCompress", "false", c);
    testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointHost", "127.0.0.1", c);
    testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointPort", "12010", c);

    cout << "ok" << endl;

    cout << "testing endpoint lookup metrics... " << flush;

    props["IceMX.Metrics.View.Map.ConnectionEstablishment.GroupBy"] = "id";
    updateProps(clientProps, serverProps, update, props, "EndpointLookup");
    test(clientMetrics->getMetricsView("View")["EndpointLookup"].empty());

    Ice::ObjectPrx prx = communicator->stringToProxy("metrics:default -p 12010 -h localhost");
    prx->ice_ping();
    
    test(clientMetrics->getMetricsView("View")["EndpointLookup"].size() == 1);
    m1 = clientMetrics->getMetricsView("View")["EndpointLookup"][0];
    test(m1->current == 0 && m1->total == 1 && m1->id == "tcp -e 1.1 -h localhost -p 12010");

    prx->ice_getConnection()->close(false);

    try
    {
        communicator->stringToProxy("test:tcp -p 12010 -h unknownfoo.zeroc.com")->ice_ping();
        test(false);
    }
    catch(const Ice::DNSException&)
    {
    }
    catch(const Ice::LocalException&)
    {
        test(false);
    }
    test(clientMetrics->getMetricsView("View")["EndpointLookup"].size() == 2);
    m1 = clientMetrics->getMetricsView("View")["EndpointLookup"][1];
    test(m1->id == "tcp -e 1.1 -h unknownfoo.zeroc.com -p 12010" && m1->total == 2 && m1->failures == 2);
    
    checkFailure(clientMetrics, "EndpointLookup", m1->id, "Ice::DNSException", 2);

    c = Connect(prx);

    testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "parent", "Communicator", c);
    testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "id", "tcp -e 1.1 -h localhost -p 12010", c);
    testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpoint", 
                  "tcp -e 1.1 -h localhost -p 12010", c);

    testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointType", "1", c);
    testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointIsDatagram", "false", c);
    testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointIsSecure", "false", c);
    testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointProtocolVersion", "1.0", c);
    testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointEncodingVersion", "1.1", c);
    testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointTimeout", "-1", c);
    testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointCompress", "false", c);
    testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointHost", "localhost", c);
    testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointPort", "12010", c);

    cout << "ok" << endl;

    cout << "testing dispatch metrics... " << flush;

    props["IceMX.Metrics.View.Map.Dispatch.GroupBy"] = "operation";
    updateProps(clientProps, serverProps, update, props, "Dispatch");
    test(serverMetrics->getMetricsView("View")["Dispatch"].empty());

    metrics->op();
    try
    {
        metrics->opWithUserException();
        test(false);
    }
    catch(const Test::UserEx&)
    {
    }
    try
    {
        metrics->opWithRequestFailedException();
        test(false);
    }
    catch(const Ice::RequestFailedException&)
    {
    }
    try
    {
        metrics->opWithLocalException();
        test(false);
    }
    catch(const Ice::LocalException&)
    {
    }
    try
    {
        metrics->opWithUnknownException();
        test(false);
    }
    catch(const Ice::UnknownException&)
    {
    }
    try
    {
        metrics->fail();
        test(false);
    }
    catch(const Ice::ConnectionLostException&)
    {
    }

    IceMX::MetricsMap mmap = serverMetrics->getMetricsView("View")["Dispatch"];
    test(mmap.size() == 6);
    map<string, IceMX::MetricsPtr> map;
    for(IceMX::MetricsMap::const_iterator p = mmap.begin(); p != mmap.end(); ++p)
    {
        map.insert(make_pair((*p)->id, *p));
    }
    m1 = map["op"];
    test(m1->current == 0 && m1->total == 1 && m1->failures == 0);

    m1 = map["opWithUserException"];
    test(m1->current == 0 && m1->total == 1 && m1->failures == 1);
    checkFailure(serverMetrics, "Dispatch", m1->id, "Test::UserEx", 1);

    m1 = map["opWithLocalException"];
    test(m1->current == 0 && m1->total == 1 && m1->failures == 1);
    checkFailure(serverMetrics, "Dispatch", m1->id, "Ice::SyscallException", 1);

    m1 = map["opWithRequestFailedException"];
    test(m1->current == 0 && m1->total == 1 && m1->failures == 1);
    checkFailure(serverMetrics, "Dispatch", m1->id, "Ice::ObjectNotExistException", 1);

    m1 = map["opWithUnknownException"];
    test(m1->current == 0 && m1->total == 1 && m1->failures == 1);
    checkFailure(serverMetrics, "Dispatch", m1->id, "unknown", 1);

    InvokeOp op(metrics);

    testAttribute(serverMetrics, serverProps, update, "Dispatch", "parent", "TestAdapter", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "id", "metrics [op]", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpoint", "tcp -e 1.1 -h 127.0.0.1 -p 12010", op);
    //testAttribute(serverMetrics, serverProps, update, "Dispatch", "connection", "", op);

    testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointType", "1", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointIsDatagram", "false", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointIsSecure", "false", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointProtocolVersion", "1.0", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointEncodingVersion", "1.1", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointTimeout", "-1", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointCompress", "false", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointHost", "127.0.0.1", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointPort", "12010", op);

    testAttribute(serverMetrics, serverProps, update, "Dispatch", "incoming", "true", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "adapterName", "TestAdapter", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "connectionId", "", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "localHost", "127.0.0.1", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "localPort", "12010", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "remoteHost", "127.0.0.1", op);
    //testAttribute(serverMetrics, serverProps, update, "Dispatch", "remotePort", "12010", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "mcastHost", "unknown", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "mcastPort", "unknown", op);

    testAttribute(serverMetrics, serverProps, update, "Dispatch", "operation", "op", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "identity", "metrics", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "facet", "", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "encoding", "1.1", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "mode", "twoway", op);

    testAttribute(serverMetrics, serverProps, update, "Dispatch", "context.entry1", "test", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "context.entry2", "", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "context.entry3", "unknown", op);

    cout << "ok" << endl;

    cout << "testing invocation metrics... " << flush;

    props["IceMX.Metrics.View.Map.Invocation.GroupBy"] = "operation";
    props["IceMX.Metrics.View.Map.Invocation.Map.Remote.GroupBy"] = "localPort";
    updateProps(clientProps, serverProps, update, props, "Invocation");
    test(serverMetrics->getMetricsView("View")["Invocation"].empty());

    metrics->op();
    try
    {
        metrics->opWithUserException();
        test(false);
    }
    catch(const Test::UserEx&)
    {
    }
    try
    {
        metrics->opWithRequestFailedException();
        test(false);
    }
    catch(const Ice::RequestFailedException&)
    {
    }
    try
    {
        metrics->opWithLocalException();
        test(false);
    }
    catch(const Ice::LocalException&)
    {
    }
    try
    {
        metrics->opWithUnknownException();
        test(false);
    }
    catch(const Ice::UnknownException&)
    {
    }
    try
    {
        metrics->fail();
        test(false);
    }
    catch(const Ice::ConnectionLostException&)
    {
    }

    mmap = clientMetrics->getMetricsView("View")["Invocation"];
    test(mmap.size() == 6);
    map.clear();
    for(IceMX::MetricsMap::const_iterator p = mmap.begin(); p != mmap.end(); ++p)
    {
        map.insert(make_pair((*p)->id, *p));
    }
    IceMX::InvocationMetricsPtr im1;
    im1 = IceMX::InvocationMetricsPtr::dynamicCast(map["op"]);
    test(im1->current == 0 && im1->total == 1 && im1->failures == 0 && im1->retry == 0 && im1->remotes.size() == 1);

    im1 = IceMX::InvocationMetricsPtr::dynamicCast(map["opWithUserException"]);
    test(im1->current == 0 && im1->total == 1 && im1->failures == 1 && im1->retry == 0 && im1->remotes.size() == 1);
    checkFailure(clientMetrics, "Invocation", im1->id, "Test::UserEx", 1);

    im1 = IceMX::InvocationMetricsPtr::dynamicCast(map["opWithLocalException"]);
    test(im1->current == 0 && im1->total == 1 && im1->failures == 1 && im1->retry == 0 && im1->remotes.size() == 1);
    checkFailure(clientMetrics, "Invocation", im1->id, "Ice::UnknownLocalException", 1);

    im1 = IceMX::InvocationMetricsPtr::dynamicCast(map["opWithRequestFailedException"]);
    test(im1->current == 0 && im1->total == 1 && im1->failures == 1 && im1->retry == 0 && im1->remotes.size() == 1);
    checkFailure(clientMetrics, "Invocation", im1->id, "Ice::ObjectNotExistException", 1);

    im1 = IceMX::InvocationMetricsPtr::dynamicCast(map["opWithUnknownException"]);
    test(im1->current == 0 && im1->total == 1 && im1->failures == 1 && im1->retry == 0 && im1->remotes.size() == 1);
    checkFailure(clientMetrics, "Invocation", im1->id, "Ice::UnknownException", 1);

    im1 = IceMX::InvocationMetricsPtr::dynamicCast(map["fail"]);
    test(im1->current == 0 && im1->total == 1 && im1->failures == 1 && im1->retry == 1 && im1->remotes.size() == 2);
    checkFailure(clientMetrics, "Invocation", im1->id, "Ice::ConnectionLostException", 1);

    testAttribute(clientMetrics, clientProps, update, "Invocation", "parent", "Communicator", op);
    testAttribute(clientMetrics, clientProps, update, "Invocation", "id",
                  "metrics -t:tcp -e 1.1 -h 127.0.0.1 -p 12010 [op]", op);

    testAttribute(clientMetrics, clientProps, update, "Invocation", "operation", "op", op);
    testAttribute(clientMetrics, clientProps, update, "Invocation", "identity", "metrics", op);
    testAttribute(clientMetrics, clientProps, update, "Invocation", "facet", "", op);
    testAttribute(clientMetrics, clientProps, update, "Invocation", "encoding", "1.1", op);
    testAttribute(clientMetrics, clientProps, update, "Invocation", "mode", "twoway", op);
    testAttribute(clientMetrics, clientProps, update, "Invocation", "proxy",
                  "metrics -t:tcp -e 1.1 -h 127.0.0.1 -p 12010", op);

    testAttribute(clientMetrics, clientProps, update, "Invocation", "context.entry1", "test", op);
    testAttribute(clientMetrics, clientProps, update, "Invocation", "context.entry2", "", op);
    testAttribute(clientMetrics, clientProps, update, "Invocation", "context.entry3", "unknown", op);

    cout << "ok" << endl;

    return metrics;
}
