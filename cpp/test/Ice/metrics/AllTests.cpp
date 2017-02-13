// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <InstrumentationI.h>
#include <Test.h>

using namespace std;
using namespace Test;

namespace
{

class Callback : public IceUtil::Shared, private IceUtil::Monitor<IceUtil::Mutex>
{
public:

    Callback() : _wait(true)
    {
    }

    void response()
    {
        Lock sync(*this);
        _wait = false;
        notify();
    }

    void exception(const Ice::Exception&)
    {
        response();
    }

    void
    waitForResponse()
    {
        Lock sync(*this);
        while(_wait)
        {
            wait();
        }
        _wait = true;
    }

private:

    bool _wait;
};
typedef IceUtil::Handle<Callback> CallbackPtr;

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

IceMX::ConnectionMetricsPtr
getServerConnectionMetrics(const IceMX::MetricsAdminPrx& metrics, Ice::Long expected)
{
    IceMX::ConnectionMetricsPtr s;
    int nRetry = 30;
    Ice::Long timestamp;
    s = IceMX::ConnectionMetricsPtr::dynamicCast(metrics->getMetricsView("View", timestamp)["Connection"][0]);
    while(s->sentBytes != expected && nRetry-- > 0)
    {
        // On some platforms, it's necessary to wait a little before obtaining the server metrics
        // to get an accurate sentBytes metric. The sentBytes metric is updated before the response
        // to the operation is sent and getMetricsView can be dispatched before the metric is really
        // updated.
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(100));
        s = IceMX::ConnectionMetricsPtr::dynamicCast(metrics->getMetricsView("View", timestamp)["Connection"][0]);
    }
    return s;
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
        {
            Lock sync(*this);
            while(!_updated)
            {
                wait();
            }
        }

        // Ensure that the previous updates were committed, the setProperties call returns before
        // notifying the callbacks so to ensure all the update callbacks have be notified we call
        // a second time, this will block until all the notifications from the first update have
        // completed.
        _serverProps->setProperties(Ice::PropertyDict());

        Lock sync(*this);
        _updated = false;
    }

    void
    updated(const Ice::PropertyDict&)
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
        Ice::Long timestamp;
        IceMX::MetricsView view = metrics->getMetricsView(viewName, timestamp);
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
    Ice::Long timestamp;
    IceMX::MetricsView view = metrics->getMetricsView("View", timestamp);
    if(view.find(map) == view.end() || view[map].empty())
    {
        if(!value.empty())
        {
            cerr << "no map `" << map << "' for group by = `" << attr << "'" << endl;
            test(false);
        }
    }
    else if(view[map].size() != 1 || view[map][0]->id != value)
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
    if(sprops->ice_getConnection())
    {
        cprops->setProperties(getClientProps(cprops, props, map));
        sprops->setProperties(getServerProps(sprops, props, map));
    }
    else
    {
        Ice::PropertyDict clientProps = getClientProps(cprops, props, map);
        Ice::PropertyDict serverProps = getClientProps(cprops, props, map);
        serverProps.insert(clientProps.begin(), clientProps.end());
        cprops->setProperties(serverProps);
    }

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
checkFailure(const IceMX::MetricsAdminPrx& m, const string& map, const string& id, const string& failure, int count = 0)
{
    IceMX::MetricsFailures f = m->getMetricsFailures("View", map, id);
    if(f.failures.find(failure) == f.failures.end())
    {
        cerr << "couldn't find failure `" << failure << "' for `" << id << "'" << endl;
        test(false);
    }
    if(count > 0 && f.failures[failure] != count)
    {
        cerr << "count for failure `" << failure << "' of `" << id << "' is different from expected: ";
        cerr << count << " != " << f.failures[failure] << endl;
        test(false);
    }
}

map<string, IceMX::MetricsPtr>
toMap(const IceMX::MetricsMap& mmap)
{
    map<string, IceMX::MetricsPtr> m;
    for(IceMX::MetricsMap::const_iterator p = mmap.begin(); p != mmap.end(); ++p)
    {
        m.insert(make_pair((*p)->id, *p));
    }
    return m;
}

}

MetricsPrx
allTests(const Ice::CommunicatorPtr& communicator, const CommunicatorObserverIPtr& obsv)
{
    MetricsPrx metrics = MetricsPrx::checkedCast(communicator->stringToProxy("metrics:default -p 12010"));
    bool collocated = !metrics->ice_getConnection();

    cout << "testing metrics admin facet checkedCast... " << flush;
    Ice::ObjectPrx admin = communicator->getAdmin();
    Ice::PropertiesAdminPrx clientProps = Ice::PropertiesAdminPrx::checkedCast(admin, "Properties");
    IceMX::MetricsAdminPrx clientMetrics = IceMX::MetricsAdminPrx::checkedCast(admin, "Metrics");
    test(clientProps && clientMetrics);

    admin = metrics->getAdmin();
    Ice::PropertiesAdminPrx serverProps = Ice::PropertiesAdminPrx::checkedCast(admin, "Properties");
    IceMX::MetricsAdminPrx serverMetrics = IceMX::MetricsAdminPrx::checkedCast(admin, "Metrics");
    test(serverProps && serverMetrics);

    UpdateCallbackI* update = new UpdateCallbackI(serverProps);
    Ice::NativePropertiesAdminPtr::dynamicCast(communicator->findAdminFacet("Properties"))->addUpdateCallback(update);

    cout << "ok" << endl;

    Ice::PropertyDict props;

    cout << "testing group by none..." << flush;

    props["IceMX.Metrics.View.GroupBy"] = "none";
    updateProps(clientProps, serverProps, update, props);

#ifndef ICE_OS_WINRT
    int threadCount = 4;
#else
    int threadCount = 3; // No endpoint host resolver thread with WinRT.
#endif

    Ice::Long timestamp;
    IceMX::MetricsView view = clientMetrics->getMetricsView("View", timestamp);
    if(!collocated)
    {
        test(view["Connection"].size() == 1 && view["Connection"][0]->current == 1 &&
             view["Connection"][0]->total == 1);
    }
    test(view["Thread"].size() == 1 && view["Thread"][0]->current == threadCount &&
         view["Thread"][0]->total == threadCount);
    cout << "ok" << endl;

    cout << "testing group by id..." << flush;

    props["IceMX.Metrics.View.GroupBy"] = "id";
    updateProps(clientProps, serverProps, update, props);

    metrics->ice_ping();
    metrics->ice_ping();
    metrics->ice_connectionId("Con1")->ice_ping();
    metrics->ice_connectionId("Con1")->ice_ping();
    metrics->ice_connectionId("Con1")->ice_ping();

    view = clientMetrics->getMetricsView("View", timestamp);
    if(!collocated)
    {
        test(view["Connection"].size() == 2);
    }
    test(static_cast<int>(view["Thread"].size()) == threadCount);
    test(view["Invocation"].size() == 1);

    IceMX::InvocationMetricsPtr invoke = IceMX::InvocationMetricsPtr::dynamicCast(view["Invocation"][0]);
    test(invoke->id.find("[ice_ping]") > 0 && invoke->current == 0 && invoke->total == 5);

    if(!collocated)
    {
        test(invoke->remotes.size() == 2);
        test(invoke->remotes[0]->total = 2);
        test(invoke->remotes[1]->total = 3);
    }
    else
    {
        test(invoke->collocated.size() == 1);
        test(invoke->collocated[0]->total = 5);
    }

    view = serverMetrics->getMetricsView("View", timestamp);
    if(serverMetrics->ice_getConnection())
    {
        test(static_cast<int>(view["Thread"].size()) > threadCount);
        test(view["Connection"].size() == 2);
    }
    test(view["Dispatch"].size() == 1);
    test(view["Dispatch"][0]->current <= 1 && view["Dispatch"][0]->total == 5);
    test(view["Dispatch"][0]->id.find("[ice_ping]") > 0);

    if(!collocated)
    {
        metrics->ice_getConnection()->close(false);
        metrics->ice_connectionId("Con1")->ice_getConnection()->close(false);

        waitForCurrent(clientMetrics, "View", "Connection", 0);
        waitForCurrent(serverMetrics, "View", "Connection", 0);
    }

    clearView(clientProps, serverProps, update);

    cout << "ok" << endl;

    map<string, IceMX::MetricsPtr> map;

    if(!collocated)
    {
        cout << "testing connection metrics... " << flush;

        props["IceMX.Metrics.View.Map.Connection.GroupBy"] = "none";
        updateProps(clientProps, serverProps, update, props, "Connection");

        test(clientMetrics->getMetricsView("View", timestamp)["Connection"].empty());
        test(serverMetrics->getMetricsView("View", timestamp)["Connection"].empty());

        metrics->ice_ping();

        IceMX::ConnectionMetricsPtr cm1, sm1, cm2, sm2;
        cm1 = IceMX::ConnectionMetricsPtr::dynamicCast(clientMetrics->getMetricsView("View", timestamp)["Connection"][0]);
        sm1 = IceMX::ConnectionMetricsPtr::dynamicCast(serverMetrics->getMetricsView("View", timestamp)["Connection"][0]);
        sm1 = getServerConnectionMetrics(serverMetrics, 25);
        test(cm1->total == 1 && sm1->total == 1);

        metrics->ice_ping();

        cm2 = IceMX::ConnectionMetricsPtr::dynamicCast(clientMetrics->getMetricsView("View", timestamp)["Connection"][0]);
        sm2 = getServerConnectionMetrics(serverMetrics, 50);

        test(cm2->sentBytes - cm1->sentBytes == 45); // 45 for ice_ping request
        test(cm2->receivedBytes - cm1->receivedBytes == 25); // 25 bytes for ice_ping response
        test(sm2->receivedBytes - sm1->receivedBytes == 45);
        test(sm2->sentBytes - sm1->sentBytes == 25);

        cm1 = cm2;
        sm1 = sm2;

        Test::ByteSeq bs;
        metrics->opByteS(bs);

        cm2 = IceMX::ConnectionMetricsPtr::dynamicCast(clientMetrics->getMetricsView("View", timestamp)["Connection"][0]);
        sm2 = getServerConnectionMetrics(serverMetrics, sm1->sentBytes + cm2->receivedBytes - cm1->receivedBytes);
        Ice::Long requestSz = cm2->sentBytes - cm1->sentBytes;
        Ice::Long replySz = cm2->receivedBytes - cm1->receivedBytes;

        cm1 = cm2;
        sm1 = sm2;

        bs.resize(456);
        metrics->opByteS(bs);

        cm2 = IceMX::ConnectionMetricsPtr::dynamicCast(clientMetrics->getMetricsView("View", timestamp)["Connection"][0]);
        sm2 = getServerConnectionMetrics(serverMetrics, sm1->sentBytes + replySz);

        // 4 is for the seq variable size
        test(cm2->sentBytes - cm1->sentBytes == requestSz + static_cast<int>(bs.size()) + 4);
        test(cm2->receivedBytes - cm1->receivedBytes == replySz);
        test(sm2->receivedBytes - sm1->receivedBytes == requestSz + static_cast<int>(bs.size()) + 4);
        test(sm2->sentBytes - sm1->sentBytes == replySz);

        cm1 = cm2;
        sm1 = sm2;

        bs.resize(1024 * 1024 * 10); // Try with large amount of data which should be sent in several chunks
        metrics->opByteS(bs);

        cm2 = IceMX::ConnectionMetricsPtr::dynamicCast(clientMetrics->getMetricsView("View", timestamp)["Connection"][0]);
        sm2 = getServerConnectionMetrics(serverMetrics, sm1->sentBytes + replySz);

        // 4 is for the seq variable size
        test(cm2->sentBytes - cm1->sentBytes == requestSz + static_cast<int>(bs.size()) + 4);
        test(cm2->receivedBytes - cm1->receivedBytes == replySz);
        test(sm2->receivedBytes - sm1->receivedBytes == requestSz + static_cast<int>(bs.size()) + 4);
        test(sm2->sentBytes - sm1->sentBytes == replySz);

        props["IceMX.Metrics.View.Map.Connection.GroupBy"] = "state";
        updateProps(clientProps, serverProps, update, props, "Connection");

        map = toMap(serverMetrics->getMetricsView("View", timestamp)["Connection"]);

        test(map["active"]->current == 1);

        ControllerPrx controller = ControllerPrx::checkedCast(communicator->stringToProxy("controller:default -p 12011"));
        controller->hold();

        map = toMap(clientMetrics->getMetricsView("View", timestamp)["Connection"]);
        test(map["active"]->current == 1);
        map = toMap(serverMetrics->getMetricsView("View", timestamp)["Connection"]);
        test(map["holding"]->current == 1);

        metrics->ice_getConnection()->close(false);

        map = toMap(clientMetrics->getMetricsView("View", timestamp)["Connection"]);
        test(map["closing"]->current == 1);
        map = toMap(serverMetrics->getMetricsView("View", timestamp)["Connection"]);
        test(map["holding"]->current == 1);

        controller->resume();

        map = toMap(serverMetrics->getMetricsView("View", timestamp)["Connection"]);
        test(map["holding"]->current == 0);

        props["IceMX.Metrics.View.Map.Connection.GroupBy"] = "none";
        updateProps(clientProps, serverProps, update, props, "Connection");

        metrics->ice_getConnection()->close(false);

        metrics->ice_timeout(500)->ice_ping();
        controller->hold();
        try
        {
            Ice::ByteSeq seq;
            seq.resize(10000000);
            metrics->ice_timeout(500)->opByteS(seq);
            test(false);
        }
        catch(const Ice::TimeoutException&)
        {
        }
        controller->resume();

        cm1 = IceMX::ConnectionMetricsPtr::dynamicCast(clientMetrics->getMetricsView("View", timestamp)["Connection"][0]);
        while(true)
        {
            sm1 = IceMX::ConnectionMetricsPtr::dynamicCast(
                serverMetrics->getMetricsView("View", timestamp)["Connection"][0]);
            if(sm1->failures >= 2)
            {
                break;
            }
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(10));
        }
        test(cm1->failures == 2 && sm1->failures >= 2);

        checkFailure(clientMetrics, "Connection", cm1->id, "Ice::TimeoutException", 1);
        checkFailure(clientMetrics, "Connection", cm1->id, "Ice::ConnectTimeoutException", 1);
        checkFailure(serverMetrics, "Connection", sm1->id, "Ice::ConnectionLostException");

        MetricsPrx m = metrics->ice_timeout(500)->ice_connectionId("Con1");
        m->ice_ping();

        testAttribute(clientMetrics, clientProps, update, "Connection", "parent", "Communicator");
        //testAttribute(clientMetrics, clientProps, update, "Connection", "id", "");
        testAttribute(clientMetrics, clientProps, update, "Connection", "endpoint", "tcp -h 127.0.0.1 -p 12010 -t 500");

        testAttribute(clientMetrics, clientProps, update, "Connection", "endpointType", "1");
        testAttribute(clientMetrics, clientProps, update, "Connection", "endpointIsDatagram", "false");
        testAttribute(clientMetrics, clientProps, update, "Connection", "endpointIsSecure", "false");
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
        testAttribute(clientMetrics, clientProps, update, "Connection", "mcastHost", "");
        testAttribute(clientMetrics, clientProps, update, "Connection", "mcastPort", "");

        m->ice_getConnection()->close(false);

        waitForCurrent(clientMetrics, "View", "Connection", 0);
        waitForCurrent(serverMetrics, "View", "Connection", 0);

        cout << "ok" << endl;

        cout << "testing connection establishment metrics... " << flush;

        props["IceMX.Metrics.View.Map.ConnectionEstablishment.GroupBy"] = "id";
        updateProps(clientProps, serverProps, update, props, "ConnectionEstablishment");
        test(clientMetrics->getMetricsView("View", timestamp)["ConnectionEstablishment"].empty());

        metrics->ice_ping();

        test(clientMetrics->getMetricsView("View", timestamp)["ConnectionEstablishment"].size() == 1);
        IceMX::MetricsPtr m1 = clientMetrics->getMetricsView("View", timestamp)["ConnectionEstablishment"][0];
        test(m1->current == 0 && m1->total == 1 && m1->id == "127.0.0.1:12010");

        metrics->ice_getConnection()->close(false);
        controller->hold();
        try
        {
            communicator->stringToProxy("test:tcp -p 12010 -h 127.0.0.1")->ice_timeout(10)->ice_ping();
            test(false);
        }
        catch(const Ice::ConnectTimeoutException&)
        {
        }
        catch(const Ice::LocalException&)
        {
            test(false);
        }
        controller->resume();
        test(clientMetrics->getMetricsView("View", timestamp)["ConnectionEstablishment"].size() == 1);
        m1 = clientMetrics->getMetricsView("View", timestamp)["ConnectionEstablishment"][0];
        test(m1->id == "127.0.0.1:12010" && m1->total == 3 && m1->failures == 2);

        checkFailure(clientMetrics, "ConnectionEstablishment", m1->id, "Ice::ConnectTimeoutException", 2);

        Connect c(metrics);
        testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "parent", "Communicator", c);
        testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "id", "127.0.0.1:12010", c);
        testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpoint",
                      "tcp -h 127.0.0.1 -p 12010 -t 60000", c);

        testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointType", "1", c);
        testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointIsDatagram", "false", c);
        testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointIsSecure", "false", c);
        testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointTimeout", "60000", c);
        testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointCompress", "false", c);
        testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointHost", "127.0.0.1", c);
        testAttribute(clientMetrics, clientProps, update, "ConnectionEstablishment", "endpointPort", "12010", c);

        cout << "ok" << endl;

        //
        // Ice doesn't do any endpoint lookup with WinRT, the WinRT
        // runtime takes care of if.
        //
#if !defined(ICE_OS_WINRT) && TARGET_OS_IPHONE==0
        cout << "testing endpoint lookup metrics... " << flush;

        props["IceMX.Metrics.View.Map.ConnectionEstablishment.GroupBy"] = "id";
        updateProps(clientProps, serverProps, update, props, "EndpointLookup");
        test(clientMetrics->getMetricsView("View", timestamp)["EndpointLookup"].empty());

        Ice::ObjectPrx prx = communicator->stringToProxy("metrics:default -p 12010 -h localhost -t infinite");
        prx->ice_ping();

        test(clientMetrics->getMetricsView("View", timestamp)["EndpointLookup"].size() == 1);
        m1 = clientMetrics->getMetricsView("View", timestamp)["EndpointLookup"][0];

        test(m1->current <= 1 && m1->total == 1 && m1->id == "tcp -h localhost -p 12010 -t infinite");

        prx->ice_getConnection()->close(false);

        bool dnsException = false;
        try
        {
            communicator->stringToProxy("test:tcp -t 500 -p 12010 -h unknownfoo.zeroc.com")->ice_ping();
            test(false);
        }
        catch(const Ice::DNSException&)
        {
            dnsException = true;
        }
        catch(const Ice::LocalException&)
        {
            // Some DNS servers don't fail on unknown DNS names.
        }
        test(clientMetrics->getMetricsView("View", timestamp)["EndpointLookup"].size() == 2);
        m1 = clientMetrics->getMetricsView("View", timestamp)["EndpointLookup"][1];
        test(m1->id == "tcp -h unknownfoo.zeroc.com -p 12010 -t 500" && m1->total == 2 &&
             (!dnsException || m1->failures == 2));
        if(dnsException)
        {
            checkFailure(clientMetrics, "EndpointLookup", m1->id, "Ice::DNSException", 2);
        }

        c = Connect(prx);

        testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "parent", "Communicator", c);
        testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "id",
                      "tcp -h localhost -p 12010 -t infinite", c);
        testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpoint",
                      "tcp -h localhost -p 12010 -t infinite", c);

        testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointType", "1", c);
        testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointIsDatagram", "false", c);
        testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointIsSecure", "false", c);
        testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointTimeout", "-1", c);
        testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointCompress", "false", c);
        testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointHost", "localhost", c);
        testAttribute(clientMetrics, clientProps, update, "EndpointLookup", "endpointPort", "12010", c);

        cout << "ok" << endl;
#endif
    }

    cout << "testing dispatch metrics... " << flush;

    props["IceMX.Metrics.View.Map.Dispatch.GroupBy"] = "operation";
    updateProps(clientProps, serverProps, update, props, "Dispatch");
    test(serverMetrics->getMetricsView("View", timestamp)["Dispatch"].empty());

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
    if(!collocated)
    {
        try
        {
            metrics->fail();
            test(false);
        }
        catch(const Ice::ConnectionLostException&)
        {
        }
    }

    map = toMap(serverMetrics->getMetricsView("View", timestamp)["Dispatch"]);
    if(!collocated)
    {
        test(map.size() == 6);
    }
    else
    {
        test(map.size() == 5);
    }

    IceMX::DispatchMetricsPtr dm1 = IceMX::DispatchMetricsPtr::dynamicCast(map["op"]);
    test(dm1->current <= 1 && dm1->total == 1 && dm1->failures == 0 && dm1->userException == 0);
    test(dm1->size == 21 && dm1->replySize == 7);

    dm1 = IceMX::DispatchMetricsPtr::dynamicCast(map["opWithUserException"]);
    test(dm1->current <= 1 && dm1->total == 1 && dm1->failures == 0 && dm1->userException == 1);
    test(dm1->size == 38 && dm1->replySize == 23);

    dm1 = IceMX::DispatchMetricsPtr::dynamicCast(map["opWithLocalException"]);
    test(dm1->current <= 1 && dm1->total == 1 && dm1->failures == 1 && dm1->userException == 0);
    checkFailure(serverMetrics, "Dispatch", dm1->id, "Ice::SyscallException", 1);
    test(dm1->size == 39 && dm1->replySize > 7); // Reply contains the exception stack depending on the OS.

    dm1 = IceMX::DispatchMetricsPtr::dynamicCast(map["opWithRequestFailedException"]);
    test(dm1->current <= 1 && dm1->total == 1 && dm1->failures == 1 && dm1->userException == 0);
    checkFailure(serverMetrics, "Dispatch", dm1->id, "Ice::ObjectNotExistException", 1);
    test(dm1->size == 47 && dm1->replySize == 40);

    dm1 = IceMX::DispatchMetricsPtr::dynamicCast(map["opWithUnknownException"]);
    test(dm1->current <= 1 && dm1->total == 1 && dm1->failures == 1 && dm1->userException == 0);
    checkFailure(serverMetrics, "Dispatch", dm1->id, "unknown", 1);
    test(dm1->size == 41 && dm1->replySize == 23);

    InvokeOp op(metrics);

    testAttribute(serverMetrics, serverProps, update, "Dispatch", "parent", "TestAdapter", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "id", "metrics [op]", op);
    if(!collocated)
    {
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpoint", "tcp -h 127.0.0.1 -p 12010 -t 60000",
                      op);
        //testAttribute(serverMetrics, serverProps, update, "Dispatch", "connection", "", op);

        testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointType", "1", op);
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointIsDatagram", "false", op);
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointIsSecure", "false", op);
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "endpointTimeout", "60000", op);
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
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "mcastHost", "", op);
        testAttribute(serverMetrics, serverProps, update, "Dispatch", "mcastPort", "", op);
    }

    testAttribute(serverMetrics, serverProps, update, "Dispatch", "operation", "op", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "identity", "metrics", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "facet", "", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "mode", "twoway", op);

    testAttribute(serverMetrics, serverProps, update, "Dispatch", "context.entry1", "test", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "context.entry2", "", op);
    testAttribute(serverMetrics, serverProps, update, "Dispatch", "context.entry3", "", op);

    cout << "ok" << endl;

    cout << "testing invocation metrics... " << flush;

    props["IceMX.Metrics.View.Map.Invocation.GroupBy"] = "operation";
    props["IceMX.Metrics.View.Map.Invocation.Map.Remote.GroupBy"] = "localPort";
    props["IceMX.Metrics.View.Map.Invocation.Map.Collocated.GroupBy"] = "parent";
    updateProps(clientProps, serverProps, update, props, "Invocation");
    test(serverMetrics->getMetricsView("View", timestamp)["Invocation"].empty());

    CallbackPtr cb = new Callback();
    metrics->op();
    metrics->end_op(metrics->begin_op());
    metrics->begin_op(newCallback_Metrics_op(cb, &Callback::response, &Callback::exception));
    cb->waitForResponse();

    // User exception
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
        metrics->end_opWithUserException(metrics->begin_opWithUserException());
        test(false);
    }
    catch(const Test::UserEx&)
    {
    }
    metrics->begin_opWithUserException(newCallback_Metrics_opWithUserException(
                                           cb, &Callback::response, &Callback::exception));
    cb->waitForResponse();

    // Request failed exception
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
        metrics->end_opWithRequestFailedException(metrics->begin_opWithRequestFailedException());
        test(false);
    }
    catch(const Ice::RequestFailedException&)
    {
    }
    metrics->begin_opWithRequestFailedException(newCallback_Metrics_opWithRequestFailedException(
                                                    cb, &Callback::response, &Callback::exception));
    cb->waitForResponse();

    // Local exception
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
        metrics->end_opWithLocalException(metrics->begin_opWithLocalException());
        test(false);
    }
    catch(const Ice::LocalException&)
    {
    }
    metrics->begin_opWithLocalException(newCallback_Metrics_opWithLocalException(
                                            cb, &Callback::response, &Callback::exception));
    cb->waitForResponse();

    // Unknown exception
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
        metrics->end_opWithUnknownException(metrics->begin_opWithUnknownException());
        test(false);
    }
    catch(const Ice::UnknownException&)
    {
    }
    metrics->begin_opWithUnknownException(newCallback_Metrics_opWithUnknownException(
                                              cb, &Callback::response, &Callback::exception));
    cb->waitForResponse();

    // Fail
    if(!collocated)
    {
        try
        {
            metrics->fail();
            test(false);
        }
        catch(const Ice::ConnectionLostException&)
        {
        }
        try
        {
            metrics->end_fail(metrics->begin_fail());
            test(false);
        }
        catch(const Ice::ConnectionLostException&)
        {
        }
        metrics->begin_fail(newCallback_Metrics_fail(cb, &Callback::response, &Callback::exception));
        cb->waitForResponse();
    }
    map = toMap(clientMetrics->getMetricsView("View", timestamp)["Invocation"]);
    test(!collocated ? (map.size() == 6) : (map.size() == 5));

    IceMX::InvocationMetricsPtr im1;
    IceMX::ChildInvocationMetricsPtr rim1;
    im1 = IceMX::InvocationMetricsPtr::dynamicCast(map["op"]);
    test(im1->current <= 1 && im1->total == 3 && im1->failures == 0 && im1->retry == 0);
    test(!collocated ? (im1->remotes.size() == 1) : (im1->collocated.size() == 1));
    rim1 = IceMX::ChildInvocationMetricsPtr::dynamicCast(!collocated ? im1->remotes[0] : im1->collocated[0]);
    test(rim1->current == 0 && rim1->total == 3 && rim1->failures == 0);
    test(rim1->size == 63 && rim1->replySize == 21);

    im1 = IceMX::InvocationMetricsPtr::dynamicCast(map["opWithUserException"]);
    test(im1->current <= 1 && im1->total == 3 && im1->failures == 0 && im1->retry == 0);
    test(!collocated ? (im1->remotes.size() == 1) : (im1->collocated.size() == 1));
    rim1 = IceMX::ChildInvocationMetricsPtr::dynamicCast(!collocated ? im1->remotes[0] : im1->collocated[0]);
    test(rim1->current == 0 && rim1->total == 3 && rim1->failures == 0);
    test(rim1->size == 114 && rim1->replySize == 69);
    test(im1->userException == 3);

    im1 = IceMX::InvocationMetricsPtr::dynamicCast(map["opWithLocalException"]);
    test(im1->current <= 1 && im1->total == 3 && im1->failures == 3 && im1->retry == 0);
    test(!collocated ? (im1->remotes.size() == 1) : (im1->collocated.size() == 1));
    rim1 = IceMX::ChildInvocationMetricsPtr::dynamicCast(!collocated ? im1->remotes[0] : im1->collocated[0]);
    test(rim1->current == 0 && rim1->total == 3 && rim1->failures == 0);
    test(rim1->size == 117 && rim1->replySize > 7);
    checkFailure(clientMetrics, "Invocation", im1->id, "Ice::UnknownLocalException", 3);

    im1 = IceMX::InvocationMetricsPtr::dynamicCast(map["opWithRequestFailedException"]);
    test(im1->current <= 1 && im1->total == 3 && im1->failures == 3 && im1->retry == 0);
    test(!collocated ? (im1->remotes.size() == 1) : (im1->collocated.size() == 1));
    rim1 = IceMX::ChildInvocationMetricsPtr::dynamicCast(!collocated ? im1->remotes[0] : im1->collocated[0]);
    test(rim1->current == 0 && rim1->total == 3 && rim1->failures == 0);
    test(rim1->size == 141 && rim1->replySize == 120);
    checkFailure(clientMetrics, "Invocation", im1->id, "Ice::ObjectNotExistException", 3);

    im1 = IceMX::InvocationMetricsPtr::dynamicCast(map["opWithUnknownException"]);
    test(im1->current <= 1 && im1->total == 3 && im1->failures == 3 && im1->retry == 0);
    test(!collocated ? (im1->remotes.size() == 1) : (im1->collocated.size() == 1));
    rim1 = IceMX::ChildInvocationMetricsPtr::dynamicCast(!collocated ? im1->remotes[0] : im1->collocated[0]);
    test(rim1->current == 0 && rim1->total == 3 && rim1->failures == 0);
    test(rim1->size == 123 && rim1->replySize == 69);
    checkFailure(clientMetrics, "Invocation", im1->id, "Ice::UnknownException", 3);

    if(!collocated)
    {
        im1 = IceMX::InvocationMetricsPtr::dynamicCast(map["fail"]);
        test(im1->current <= 1 && im1->total == 3 && im1->failures == 3 && im1->retry == 3 && im1->remotes.size() == 6);
        test(im1->remotes[0]->current == 0 && im1->remotes[0]->total == 1 && im1->remotes[0]->failures == 1);
        test(im1->remotes[1]->current == 0 && im1->remotes[1]->total == 1 && im1->remotes[1]->failures == 1);
        test(im1->remotes[2]->current == 0 && im1->remotes[2]->total == 1 && im1->remotes[2]->failures == 1);
        test(im1->remotes[3]->current == 0 && im1->remotes[3]->total == 1 && im1->remotes[3]->failures == 1);
        test(im1->remotes[4]->current == 0 && im1->remotes[4]->total == 1 && im1->remotes[4]->failures == 1);
        test(im1->remotes[5]->current == 0 && im1->remotes[5]->total == 1 && im1->remotes[5]->failures == 1);
        checkFailure(clientMetrics, "Invocation", im1->id, "Ice::ConnectionLostException", 3);
    }

    testAttribute(clientMetrics, clientProps, update, "Invocation", "parent", "Communicator", op);
    testAttribute(clientMetrics, clientProps, update, "Invocation", "id", "metrics -t -e 1.1 [op]", op);

    testAttribute(clientMetrics, clientProps, update, "Invocation", "operation", "op", op);
    testAttribute(clientMetrics, clientProps, update, "Invocation", "identity", "metrics", op);
    testAttribute(clientMetrics, clientProps, update, "Invocation", "facet", "", op);
    testAttribute(clientMetrics, clientProps, update, "Invocation", "encoding", "1.1", op);
    testAttribute(clientMetrics, clientProps, update, "Invocation", "mode", "twoway", op);
    testAttribute(clientMetrics, clientProps, update, "Invocation", "proxy",
                  "metrics -t -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 60000", op);

    testAttribute(clientMetrics, clientProps, update, "Invocation", "context.entry1", "test", op);
    testAttribute(clientMetrics, clientProps, update, "Invocation", "context.entry2", "", op);
    testAttribute(clientMetrics, clientProps, update, "Invocation", "context.entry3", "", op);

    //
    // Tests with oneway
    //
    props["IceMX.Metrics.View.Map.Invocation.GroupBy"] = "operation";
    props["IceMX.Metrics.View.Map.Invocation.Map.Remote.GroupBy"] = "localPort";
    updateProps(clientProps, serverProps, update, props, "Invocation");

    MetricsPrx metricsOneway = metrics->ice_oneway();
    metricsOneway->op();
    metricsOneway->end_op(metricsOneway->begin_op());
    metricsOneway->begin_op(newCallback_Metrics_op(cb, &Callback::response, &Callback::exception));

    map = toMap(clientMetrics->getMetricsView("View", timestamp)["Invocation"]);
    test(map.size() == 1);

    im1 = IceMX::InvocationMetricsPtr::dynamicCast(map["op"]);
    test(im1->current <= 1 && im1->total == 3 && im1->failures == 0 && im1->retry == 0);
    test(!collocated ? (im1->remotes.size() == 1) : (im1->collocated.size() == 1));
    rim1 = IceMX::ChildInvocationMetricsPtr::dynamicCast(!collocated ? im1->remotes[0] : im1->collocated[0]);
    test(rim1->current <= 1 && rim1->total == 3 && rim1->failures == 0);
    test(rim1->size == 63 && rim1->replySize == 0);

    testAttribute(clientMetrics, clientProps, update, "Invocation", "mode", "oneway", InvokeOp(metricsOneway));

    //
    // Tests with batch oneway
    //
    props["IceMX.Metrics.View.Map.Invocation.GroupBy"] = "operation";
    props["IceMX.Metrics.View.Map.Invocation.Map.Remote.GroupBy"] = "localPort";
    updateProps(clientProps, serverProps, update, props, "Invocation");

    MetricsPrx metricsBatchOneway = metrics->ice_batchOneway();
    metricsBatchOneway->op();
    //metricsBatchOneway->end_op(metricsOneway->begin_op());
    //metricsBatchOneway->begin_op(newCallback_Metrics_op(cb, &Callback::response, &Callback::exception));

    map = toMap(clientMetrics->getMetricsView("View", timestamp)["Invocation"]);
    test(map.size() == 1);

    im1 = IceMX::InvocationMetricsPtr::dynamicCast(map["op"]);
    test(im1->current == 0 && im1->total == 1 && im1->failures == 0 && im1->retry == 0);
    test(im1->remotes.size() == 0);

    testAttribute(clientMetrics, clientProps, update, "Invocation", "mode", "batch-oneway",
                  InvokeOp(metricsBatchOneway));

    cout << "ok" << endl;

    cout << "testing metrics view enable/disable..." << flush;

    Ice::StringSeq disabledViews;
    props["IceMX.Metrics.View.GroupBy"] = "none";
    props["IceMX.Metrics.View.Disabled"] = "0";
    updateProps(clientProps, serverProps, update, props, "Thread");
    test(!clientMetrics->getMetricsView("View", timestamp)["Thread"].empty());
    test(clientMetrics->getMetricsViewNames(disabledViews).size() == 1 && disabledViews.empty());

    props["IceMX.Metrics.View.Disabled"] = "1";
    updateProps(clientProps, serverProps, update, props, "Thread");
    test(clientMetrics->getMetricsView("View", timestamp)["Thread"].empty());
    test(clientMetrics->getMetricsViewNames(disabledViews).empty() && disabledViews.size() == 1);

    clientMetrics->enableMetricsView("View");
    test(!clientMetrics->getMetricsView("View", timestamp)["Thread"].empty());
    test(clientMetrics->getMetricsViewNames(disabledViews).size() == 1 && disabledViews.empty());

    clientMetrics->disableMetricsView("View");
    test(clientMetrics->getMetricsView("View", timestamp)["Thread"].empty());
    test(clientMetrics->getMetricsViewNames(disabledViews).empty() && disabledViews.size() == 1);

    try
    {
        clientMetrics->enableMetricsView("UnknownView");
    }
    catch(const IceMX::UnknownMetricsView&)
    {
    }

    cout << "ok" << endl;

    cout << "testing instrumentation observer delegate... " << flush;

    test(obsv->threadObserver->getTotal() > 0);
    if(!collocated)
    {
        test(obsv->connectionObserver->getTotal() > 0);
        test(obsv->connectionEstablishmentObserver->getTotal() > 0);
#if !defined(ICE_OS_WINRT) && TARGET_OS_IPHONE==0
        test(obsv->endpointLookupObserver->getTotal() > 0);
#endif
        test(obsv->invocationObserver->remoteObserver->getTotal() > 0);
    }
    else
    {
        test(obsv->invocationObserver->collocatedObserver->getTotal() > 0);
    }
    test(obsv->dispatchObserver->getTotal() > 0);
    test(obsv->invocationObserver->getTotal() > 0);

    test(obsv->threadObserver->getCurrent() > 0);
    if(!collocated)
    {
        test(obsv->connectionObserver->getCurrent() > 0);
        test(obsv->connectionEstablishmentObserver->getCurrent() == 0);
#if !defined(ICE_OS_WINRT) && TARGET_OS_IPHONE==0
        test(obsv->endpointLookupObserver->getCurrent() == 0);
#endif
        test(obsv->invocationObserver->remoteObserver->getCurrent() == 0);
    }
    else
    {
        for(int i = 0; i < 10; ++i)
        {
            if(obsv->invocationObserver->collocatedObserver->getCurrent() > 0)
            {
                IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(50));
            }
            else
            {
                break;
            }
        }
        test(obsv->invocationObserver->collocatedObserver->getCurrent() == 0);
    }
    test(obsv->dispatchObserver->getCurrent() == 0);
    test(obsv->invocationObserver->getCurrent() == 0);

    test(obsv->threadObserver->getFailedCount() == 0);
    if(!collocated)
    {
        test(obsv->connectionObserver->getFailedCount() > 0);
        test(obsv->connectionEstablishmentObserver->getFailedCount() > 0);
#if !defined(ICE_OS_WINRT) && TARGET_OS_IPHONE==0
        test(obsv->endpointLookupObserver->getFailedCount() > 0);
#endif
    }
    //test(obsv->dispatchObserver->getFailedCount() > 0);
    test(obsv->invocationObserver->getFailedCount() > 0);
    if(!collocated)
    {
        test(obsv->invocationObserver->remoteObserver->getFailedCount() > 0);
        test(obsv->threadObserver->states > 0);
    }
    if(!collocated)
    {
        test(obsv->connectionObserver->received > 0 && obsv->connectionObserver->sent > 0);
    }
    //test(obsv->dispatchObserver->userExceptionCount > 0);
    test(obsv->invocationObserver->userExceptionCount > 0);
    if(!collocated)
    {
        test(obsv->invocationObserver->retriedCount > 0);
        test(obsv->invocationObserver->remoteObserver->replySize > 0);
    }
    else
    {
        test(obsv->invocationObserver->collocatedObserver->replySize > 0);
    }
    cout << "ok" << endl;

    return metrics;
}
