// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
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

string
getPort(const Ice::PropertiesAdminPrxPtr& p)
{
    ostringstream os;
    os << TestHelper::getTestPort(p->ice_getCommunicator()->getProperties(), 0);
    return os.str();
}

Ice::PropertyDict
getClientProps(const Ice::PropertiesAdminPrxPtr& pa, const Ice::PropertyDict& orig, const string& m = string())
{
    Ice::PropertyDict props = pa->getPropertiesForPrefix("IceMX.Metrics");
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
    props["IceMX.Metrics.View." + map + "Accept.endpointPort"] = getPort(pa);
    props["IceMX.Metrics.View." + map + "Reject.identity"] = ".*/admin|controller";
    return props;
}

Ice::PropertyDict
getServerProps(const Ice::PropertiesAdminPrxPtr& pa, const Ice::PropertyDict& orig, const string& m = string())
{
    Ice::PropertyDict props = pa->getPropertiesForPrefix("IceMX.Metrics");
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
    props["IceMX.Metrics.View." + map + "Accept.endpointPort"] = getPort(pa);
    return props;
}

IceMX::ConnectionMetricsPtr
getServerConnectionMetrics(const IceMX::MetricsAdminPrxPtr& metrics, Ice::Long expected)
{
    IceMX::ConnectionMetricsPtr s;
    int nRetry = 30;
    Ice::Long timestamp;
    s = ICE_DYNAMIC_CAST(IceMX::ConnectionMetrics, metrics->getMetricsView("View", timestamp)["Connection"][0]);
    while(s->sentBytes != expected && nRetry-- > 0)
    {
        // On some platforms, it's necessary to wait a little before obtaining the server metrics
        // to get an accurate sentBytes metric. The sentBytes metric is updated before the response
        // to the operation is sent and getMetricsView can be dispatched before the metric is really
        // updated.
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(100));
        s = ICE_DYNAMIC_CAST(IceMX::ConnectionMetrics, metrics->getMetricsView("View", timestamp)["Connection"][0]);
    }
    return s;
}

class UpdateCallbackI :
#ifndef ICE_CPP11_MAPPING
        public Ice::PropertiesAdminUpdateCallback,
#endif
private IceUtil::Monitor<IceUtil::Mutex>
{
public:

    UpdateCallbackI(const Ice::PropertiesAdminPrxPtr& serverProps) : _updated(false), _serverProps(serverProps)
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
    Ice::PropertiesAdminPrxPtr _serverProps;
};
ICE_DEFINE_PTR(UpdateCallbackIPtr, UpdateCallbackI);

void
waitForCurrent(const IceMX::MetricsAdminPrxPtr& metrics, const string& viewName, const string& map, int value)
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

void
waitForCurrent(const ObserverIPtr& observer, int value)
{
    for(int i = 0; i < 10; ++i)
    {
        if(observer->getCurrent() != value)
        {
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(50));
        }
        else
        {
            break;
        }
    }
};

template<typename T> void
testAttribute(const IceMX::MetricsAdminPrxPtr& metrics,
              const Ice::PropertiesAdminPrxPtr& props,
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
    Connect(const Ice::ObjectPrxPtr& proxyP) : proxy(proxyP)
    {
    }

    void
    operator()() const
    {
        if(proxy->ice_getCachedConnection())
        {
            proxy->ice_getCachedConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
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
            proxy->ice_getCachedConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
        }
    }

    Ice::ObjectPrxPtr proxy;
};

struct InvokeOp
{
    InvokeOp(const Test::MetricsPrxPtr& proxyP) : proxy(proxyP)
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

    Test::MetricsPrxPtr proxy;
};

void
testAttribute(const IceMX::MetricsAdminPrxPtr& metrics,
              const Ice::PropertiesAdminPrxPtr& props,
              UpdateCallbackI* update,
              const string& map,
              const string& attr,
              const string& value)
{
    testAttribute(metrics, props, update, map, attr, value, Void());
}

void
updateProps(const Ice::PropertiesAdminPrxPtr& cprops,
            const Ice::PropertiesAdminPrxPtr& sprops,
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
clearView(const Ice::PropertiesAdminPrxPtr& cprops, const Ice::PropertiesAdminPrxPtr& sprops, UpdateCallbackI* callback)
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
checkFailure(const IceMX::MetricsAdminPrxPtr& m, const string& map, const string& id, const string& failure,
             int count = 0)
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

MetricsPrxPtr
allTests(Test::TestHelper* helper, const CommunicatorObserverIPtr& obsv)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    string host = helper->getTestHost();
    string port;
    {
        ostringstream os;
        os << helper->getTestPort();
        port = os.str();
    }
    string hostAndPort = host + ":" + port;
    string protocol = helper->getTestProtocol();
    string endpoint;
    {
        ostringstream os;
        os << protocol << " -h " << host << " -p " << port;
        endpoint = os.str();
    }

    MetricsPrxPtr metrics = ICE_CHECKED_CAST(MetricsPrx, communicator->stringToProxy("metrics:" + endpoint));
    bool collocated = !metrics->ice_getConnection();

    cout << "testing metrics admin facet checkedCast... " << flush;
    Ice::ObjectPrxPtr admin = communicator->getAdmin();

    Ice::PropertiesAdminPrxPtr clientProps =  ICE_CHECKED_CAST(Ice::PropertiesAdminPrx, admin, "Properties");
    IceMX::MetricsAdminPrxPtr clientMetrics = ICE_CHECKED_CAST(IceMX::MetricsAdminPrx, admin, "Metrics");
    test(clientProps && clientMetrics);

    admin = metrics->getAdmin();
    Ice::PropertiesAdminPrxPtr serverProps = ICE_CHECKED_CAST(Ice::PropertiesAdminPrx, admin, "Properties");
    IceMX::MetricsAdminPrxPtr serverMetrics = ICE_CHECKED_CAST(IceMX::MetricsAdminPrx, admin, "Metrics");
    test(serverProps && serverMetrics);

    UpdateCallbackIPtr update = ICE_MAKE_SHARED(UpdateCallbackI, serverProps);

    ICE_DYNAMIC_CAST(Ice::NativePropertiesAdmin, communicator->findAdminFacet("Properties"))->addUpdateCallback(
#ifdef ICE_CPP11_MAPPING
        [update](const Ice::PropertyDict& changes) { update->updated(changes); }
#else
        update
#endif
        );

    cout << "ok" << endl;

    Ice::PropertyDict props;

    cout << "testing group by none..." << flush;

    props["IceMX.Metrics.View.GroupBy"] = "none";
    updateProps(clientProps, serverProps, update.get(), props);

#ifndef ICE_OS_UWP
    int threadCount = 4;
#else
    int threadCount = 3; // No endpoint host resolver thread with UWP.
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
    updateProps(clientProps, serverProps, update.get(), props);

    metrics->ice_ping();
    metrics->ice_ping();
    metrics->ice_connectionId("Con1")->ice_ping();
    metrics->ice_connectionId("Con1")->ice_ping();
    metrics->ice_connectionId("Con1")->ice_ping();

    waitForCurrent(clientMetrics, "View", "Invocation", 0);
    waitForCurrent(serverMetrics, "View", "Dispatch", 0);

    view = clientMetrics->getMetricsView("View", timestamp);
    if(!collocated)
    {
        test(view["Connection"].size() == 2);
    }

    test(static_cast<int>(view["Thread"].size()) == threadCount);
    test(view["Invocation"].size() == 1);

    IceMX::InvocationMetricsPtr invoke = ICE_DYNAMIC_CAST(IceMX::InvocationMetrics, view["Invocation"][0]);
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
    test(view["Dispatch"][0]->current == 0 && view["Dispatch"][0]->total == 5);
    test(view["Dispatch"][0]->id.find("[ice_ping]") > 0);

    if(!collocated)
    {
        metrics->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
        metrics->ice_connectionId("Con1")->ice_getConnection()->close(
            Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));

        waitForCurrent(clientMetrics, "View", "Connection", 0);
        waitForCurrent(serverMetrics, "View", "Connection", 0);
    }

    clearView(clientProps, serverProps, update.get());

    cout << "ok" << endl;

    map<string, IceMX::MetricsPtr> map;

    string type;
    string isSecure;
    if(!collocated)
    {
        Ice::EndpointInfoPtr endpointInfo = metrics->ice_getConnection()->getEndpoint()->getInfo();
        {
            ostringstream os;
            os << endpointInfo->type();
            type = os.str();
        }
        isSecure = endpointInfo->secure() ? "true": "false";
    }
    if(!collocated)
    {
        cout << "testing connection metrics... " << flush;

        props["IceMX.Metrics.View.Map.Connection.GroupBy"] = "none";
        updateProps(clientProps, serverProps, update.get(), props, "Connection");

        test(clientMetrics->getMetricsView("View", timestamp)["Connection"].empty());
        test(serverMetrics->getMetricsView("View", timestamp)["Connection"].empty());

        metrics->ice_ping();

        IceMX::ConnectionMetricsPtr cm1, sm1, cm2, sm2;
        cm1 = ICE_DYNAMIC_CAST(IceMX::ConnectionMetrics, clientMetrics->getMetricsView("View", timestamp)["Connection"][0]);
        sm1 = ICE_DYNAMIC_CAST(IceMX::ConnectionMetrics, serverMetrics->getMetricsView("View", timestamp)["Connection"][0]);
        sm1 = getServerConnectionMetrics(serverMetrics, 25);
        test(cm1->total == 1 && sm1->total == 1);

        metrics->ice_ping();

        cm2 = ICE_DYNAMIC_CAST(IceMX::ConnectionMetrics, clientMetrics->getMetricsView("View", timestamp)["Connection"][0]);
        sm2 = getServerConnectionMetrics(serverMetrics, 50);

        test(cm2->sentBytes - cm1->sentBytes == 45); // 45 for ice_ping request
        test(cm2->receivedBytes - cm1->receivedBytes == 25); // 25 bytes for ice_ping response
        test(sm2->receivedBytes - sm1->receivedBytes == 45);
        test(sm2->sentBytes - sm1->sentBytes == 25);

        cm1 = cm2;
        sm1 = sm2;

        Test::ByteSeq bs;
        metrics->opByteS(bs);

        cm2 = ICE_DYNAMIC_CAST(IceMX::ConnectionMetrics, clientMetrics->getMetricsView("View", timestamp)["Connection"][0]);
        sm2 = getServerConnectionMetrics(serverMetrics, sm1->sentBytes + cm2->receivedBytes - cm1->receivedBytes);
        Ice::Long requestSz = cm2->sentBytes - cm1->sentBytes;
        Ice::Long replySz = cm2->receivedBytes - cm1->receivedBytes;

        cm1 = cm2;
        sm1 = sm2;

        bs.resize(456);
        metrics->opByteS(bs);

        cm2 = ICE_DYNAMIC_CAST(IceMX::ConnectionMetrics, clientMetrics->getMetricsView("View", timestamp)["Connection"][0]);
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

        cm2 = ICE_DYNAMIC_CAST(IceMX::ConnectionMetrics, clientMetrics->getMetricsView("View", timestamp)["Connection"][0]);
        sm2 = getServerConnectionMetrics(serverMetrics, sm1->sentBytes + replySz);

        // 4 is for the seq variable size
        test(cm2->sentBytes - cm1->sentBytes == requestSz + static_cast<int>(bs.size()) + 4);
        test(cm2->receivedBytes - cm1->receivedBytes == replySz);
        test(sm2->receivedBytes - sm1->receivedBytes == requestSz + static_cast<int>(bs.size()) + 4);
        test(sm2->sentBytes - sm1->sentBytes == replySz);

        props["IceMX.Metrics.View.Map.Connection.GroupBy"] = "state";
        updateProps(clientProps, serverProps, update.get(), props, "Connection");

        map = toMap(serverMetrics->getMetricsView("View", timestamp)["Connection"]);
        test(map["active"]->current == 1);
        map = toMap(clientMetrics->getMetricsView("View", timestamp)["Connection"]);
        test(map["active"]->current == 1);

        Ice::ObjectPrxPtr cprx = communicator->stringToProxy("controller:" + helper->getTestEndpoint(1));
        ControllerPrxPtr controller = ICE_CHECKED_CAST(ControllerPrx, cprx);
        controller->hold();

        map = toMap(clientMetrics->getMetricsView("View", timestamp)["Connection"]);
        test(map["active"]->current == 1);
        map = toMap(serverMetrics->getMetricsView("View", timestamp)["Connection"]);
        test(map["holding"]->current == 1);

        metrics->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));

        map = toMap(clientMetrics->getMetricsView("View", timestamp)["Connection"]);
        test(map["closing"]->current == 1);
        map = toMap(serverMetrics->getMetricsView("View", timestamp)["Connection"]);
        test(map["holding"]->current == 1);

        controller->resume();

        map = toMap(serverMetrics->getMetricsView("View", timestamp)["Connection"]);
        test(map["holding"]->current == 0);

        props["IceMX.Metrics.View.Map.Connection.GroupBy"] = "none";
        updateProps(clientProps, serverProps, update.get(), props, "Connection");

        metrics->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));

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

        cm1 = ICE_DYNAMIC_CAST(IceMX::ConnectionMetrics, clientMetrics->getMetricsView("View", timestamp)["Connection"][0]);
        while(true)
        {
            sm1 = ICE_DYNAMIC_CAST(IceMX::ConnectionMetrics,
                                   serverMetrics->getMetricsView("View", timestamp)["Connection"][0]);
            if(sm1->failures >= 2)
            {
                break;
            }
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(10));
        }
        test(cm1->failures == 2 && sm1->failures >= 2);

        checkFailure(clientMetrics, "Connection", cm1->id, "::Ice::TimeoutException", 1);
        checkFailure(clientMetrics, "Connection", cm1->id, "::Ice::ConnectTimeoutException", 1);
        checkFailure(serverMetrics, "Connection", sm1->id, "::Ice::ConnectionLostException");

        MetricsPrxPtr m = metrics->ice_timeout(500)->ice_connectionId("Con1");
        m->ice_ping();

        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "parent", "Communicator");
        //testAttribute(clientMetrics, clientProps, update.get(), "Connection", "id", "");
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "endpoint", endpoint + " -t 500");
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "endpointType", type);
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "endpointIsDatagram", "false");
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "endpointIsSecure", isSecure);
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "endpointTimeout", "500");
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "endpointCompress", "false");
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "endpointHost", host);
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "endpointPort", port);

        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "incoming", "false");
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "adapterName", "");
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "connectionId", "Con1");
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "localHost", host);
        //testAttribute(clientMetrics, clientProps, update.get(), "Connection", "localPort", "");
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "remoteHost", host);
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "remotePort", port);
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "mcastHost", "");
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "mcastPort", "");

        m->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));

        waitForCurrent(clientMetrics, "View", "Connection", 0);
        waitForCurrent(serverMetrics, "View", "Connection", 0);

        cout << "ok" << endl;

        cout << "testing connection establishment metrics... " << flush;

        props["IceMX.Metrics.View.Map.ConnectionEstablishment.GroupBy"] = "id";
        updateProps(clientProps, serverProps, update.get(), props, "ConnectionEstablishment");
        test(clientMetrics->getMetricsView("View", timestamp)["ConnectionEstablishment"].empty());

        metrics->ice_ping();

        test(clientMetrics->getMetricsView("View", timestamp)["ConnectionEstablishment"].size() == 1);
        IceMX::MetricsPtr m1 = clientMetrics->getMetricsView("View", timestamp)["ConnectionEstablishment"][0];
        test(m1->current == 0 && m1->total == 1 && m1->id == hostAndPort);

        metrics->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
        controller->hold();
        try
        {
            communicator->stringToProxy("test:" + endpoint)->ice_timeout(10)->ice_ping();
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
        test(m1->id == hostAndPort && m1->total == 3 && m1->failures == 2);

        checkFailure(clientMetrics, "ConnectionEstablishment", m1->id, "::Ice::ConnectTimeoutException", 2);

        Connect c(metrics);
        testAttribute(clientMetrics, clientProps, update.get(), "ConnectionEstablishment", "parent", "Communicator", c);
        testAttribute(clientMetrics, clientProps, update.get(), "ConnectionEstablishment", "id", hostAndPort, c);
        testAttribute(clientMetrics, clientProps, update.get(), "ConnectionEstablishment", "endpoint",
                      endpoint + " -t 60000", c);

        testAttribute(clientMetrics, clientProps, update.get(), "ConnectionEstablishment", "endpointType", type, c);
        testAttribute(clientMetrics, clientProps, update.get(), "ConnectionEstablishment", "endpointIsDatagram", "false", c);
        testAttribute(clientMetrics, clientProps, update.get(), "ConnectionEstablishment", "endpointIsSecure", isSecure, c);
        testAttribute(clientMetrics, clientProps, update.get(), "ConnectionEstablishment", "endpointTimeout", "60000", c);
        testAttribute(clientMetrics, clientProps, update.get(), "ConnectionEstablishment", "endpointCompress", "false", c);
        testAttribute(clientMetrics, clientProps, update.get(), "ConnectionEstablishment", "endpointHost", host, c);
        testAttribute(clientMetrics, clientProps, update.get(), "ConnectionEstablishment", "endpointPort", port, c);

        cout << "ok" << endl;

        //
        // Ice doesn't do any endpoint lookup with UWP, the UWP
        // runtime takes care of if.
        //
#if !defined(ICE_OS_UWP) && TARGET_OS_IPHONE==0
        cout << "testing endpoint lookup metrics... " << flush;

        props["IceMX.Metrics.View.Map.EndpointLookup.GroupBy"] = "id";
        updateProps(clientProps, serverProps, update.get(), props, "EndpointLookup");
        test(clientMetrics->getMetricsView("View", timestamp)["EndpointLookup"].empty());

        Ice::ObjectPrxPtr prx = communicator->stringToProxy("metrics:" + protocol + " -h localhost -t 500 -p " + port);
        try
        {
            prx->ice_ping();
            prx->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
        }
        catch(const Ice::LocalException&)
        {
        }

        test(clientMetrics->getMetricsView("View", timestamp)["EndpointLookup"].size() == 1);
        m1 = clientMetrics->getMetricsView("View", timestamp)["EndpointLookup"][0];
        test(m1->current <= 1 && (m1->total == 1 || m1->total == 2));

        bool dnsException = false;
        try
        {
            communicator->stringToProxy("test:tcp -t 500 -h unknownfoo.zeroc.com -p " + port)->ice_ping();
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
        test(m1->id == "tcp -h unknownfoo.zeroc.com -p " + port + " -t 500" && m1->total == 2 &&
             (!dnsException || m1->failures == 2));
        if(dnsException)
        {
            checkFailure(clientMetrics, "EndpointLookup", m1->id, "::Ice::DNSException", 2);
        }

        c = Connect(prx);

        testAttribute(clientMetrics, clientProps, update.get(), "EndpointLookup", "parent", "Communicator", c);
        testAttribute(clientMetrics, clientProps, update.get(), "EndpointLookup", "id",
                      prx->ice_getConnection()->getEndpoint()->toString(), c);
        testAttribute(clientMetrics, clientProps, update.get(), "EndpointLookup", "endpoint",
                      prx->ice_getConnection()->getEndpoint()->toString(), c);

        testAttribute(clientMetrics, clientProps, update.get(), "EndpointLookup", "endpointType", type, c);
        testAttribute(clientMetrics, clientProps, update.get(), "EndpointLookup", "endpointIsDatagram", "false", c);
        testAttribute(clientMetrics, clientProps, update.get(), "EndpointLookup", "endpointIsSecure", isSecure, c);
        testAttribute(clientMetrics, clientProps, update.get(), "EndpointLookup", "endpointTimeout", "500", c);
        testAttribute(clientMetrics, clientProps, update.get(), "EndpointLookup", "endpointCompress", "false", c);
        testAttribute(clientMetrics, clientProps, update.get(), "EndpointLookup", "endpointHost", "localhost", c);
        testAttribute(clientMetrics, clientProps, update.get(), "EndpointLookup", "endpointPort", port, c);

        cout << "ok" << endl;
#endif
    }

    cout << "testing dispatch metrics... " << flush;

    props["IceMX.Metrics.View.Map.Dispatch.GroupBy"] = "operation";
    updateProps(clientProps, serverProps, update.get(), props, "Dispatch");
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

    IceMX::DispatchMetricsPtr dm1 = ICE_DYNAMIC_CAST(IceMX::DispatchMetrics, map["op"]);
    test(dm1->current <= 1 && dm1->total == 1 && dm1->failures == 0 && dm1->userException == 0);
    test(dm1->size == 21 && dm1->replySize == 7);

    dm1 = ICE_DYNAMIC_CAST(IceMX::DispatchMetrics, map["opWithUserException"]);
    test(dm1->current <= 1 && dm1->total == 1 && dm1->failures == 0 && dm1->userException == 1);
    test(dm1->size == 38 && dm1->replySize == 23);

    dm1 = ICE_DYNAMIC_CAST(IceMX::DispatchMetrics, map["opWithLocalException"]);
    test(dm1->current <= 1 && dm1->total == 1 && dm1->failures == 1 && dm1->userException == 0);
    checkFailure(serverMetrics, "Dispatch", dm1->id, "::Ice::SyscallException", 1);
    test(dm1->size == 39 && dm1->replySize > 7); // Reply contains the exception stack depending on the OS.

    dm1 = ICE_DYNAMIC_CAST(IceMX::DispatchMetrics, map["opWithRequestFailedException"]);
    test(dm1->current <= 1 && dm1->total == 1 && dm1->failures == 1 && dm1->userException == 0);
    checkFailure(serverMetrics, "Dispatch", dm1->id, "::Ice::ObjectNotExistException", 1);
    test(dm1->size == 47 && dm1->replySize == 40);

    dm1 = ICE_DYNAMIC_CAST(IceMX::DispatchMetrics, map["opWithUnknownException"]);
    test(dm1->current <= 1 && dm1->total == 1 && dm1->failures == 1 && dm1->userException == 0);
    checkFailure(serverMetrics, "Dispatch", dm1->id, "unknown", 1);
    test(dm1->size == 41 && dm1->replySize == 23);

    InvokeOp op(metrics);

    testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "parent", "TestAdapter", op);
    testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "id", "metrics [op]", op);
    if(!collocated)
    {
        testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "endpoint", endpoint + " -t 60000", op);
        //testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "connection", "", op);

        testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "endpointType", type, op);
        testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "endpointIsDatagram", "false", op);
        testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "endpointIsSecure", isSecure, op);
        testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "endpointTimeout", "60000", op);
        testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "endpointCompress", "false", op);
        testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "endpointHost", host, op);
        testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "endpointPort", port, op);

        testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "incoming", "true", op);
        testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "adapterName", "TestAdapter", op);
        testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "connectionId", "", op);
        testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "localHost", host, op);
        testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "localPort", port, op);
        testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "remoteHost", host, op);
        //testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "remotePort", port, op);
        testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "mcastHost", "", op);
        testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "mcastPort", "", op);
    }

    testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "operation", "op", op);
    testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "identity", "metrics", op);
    testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "facet", "", op);
    testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "mode", "twoway", op);

    testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "context.entry1", "test", op);
    testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "context.entry2", "", op);
    testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "context.entry3", "", op);

    cout << "ok" << endl;

    cout << "testing invocation metrics... " << flush;

    props["IceMX.Metrics.View.Map.Invocation.GroupBy"] = "operation";
    props["IceMX.Metrics.View.Map.Invocation.Map.Remote.GroupBy"] = "localPort";
    props["IceMX.Metrics.View.Map.Invocation.Map.Collocated.GroupBy"] = "parent";
    updateProps(clientProps, serverProps, update.get(), props, "Invocation");
    test(serverMetrics->getMetricsView("View", timestamp)["Invocation"].empty());

    CallbackPtr cb = new Callback();
    metrics->op();
#ifdef ICE_CPP11_MAPPING
    try
    {
        metrics->opAsync().get();
    }
    catch(const Ice::Exception&)
    {
    }

    metrics->opAsync(
        [cb]()
        {
            cb->response();
        },
        [cb](exception_ptr e)
        {
            try
            {
                rethrow_exception(e);
            }
            catch(const Ice::Exception& ex)
            {
                cb->exception(ex);
            }
            catch(...)
            {
                test(false);
            }
        });
#else
    metrics->end_op(metrics->begin_op());
    metrics->begin_op(newCallback_Metrics_op(cb, &Callback::response, &Callback::exception));
#endif
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

#ifdef ICE_CPP11_MAPPING
    try
    {
        metrics->opWithUserExceptionAsync().get();
        test(false);
    }
    catch(const Test::UserEx&)
    {
    }
    catch(...)
    {
        test(false);
    }
    metrics->opWithUserExceptionAsync(
        [cb]()
        {
            cb->response();
        },
        [cb](exception_ptr e)
        {
            try
            {
                rethrow_exception(e);
            }
            catch(const Test::UserEx& ex)
            {
                cb->exception(ex);
            }
            catch(...)
            {
                test(false);
            }
        });
#else
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
#endif
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

#ifdef ICE_CPP11_MAPPING
    try
    {
        metrics->opWithRequestFailedExceptionAsync().get();
        test(false);
    }
    catch(const Ice::RequestFailedException&)
    {
    }
    metrics->opWithRequestFailedExceptionAsync(
        [cb]()
        {
            cb->response();
        },
        [cb](exception_ptr e)
        {
            try
            {
                rethrow_exception(e);
            }
            catch(const Ice::RequestFailedException& ex)
            {
                cb->exception(ex);
            }
            catch(...)
            {
                test(false);
            }
        });
#else
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
#endif
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

#ifdef ICE_CPP11_MAPPING
    try
    {
        metrics->opWithLocalExceptionAsync().get();
        test(false);
    }
    catch(const Ice::LocalException&)
    {
    }
    metrics->opWithLocalExceptionAsync(
        [cb]()
        {
            cb->response();
        },
        [cb](exception_ptr e)
        {
            try
            {
                rethrow_exception(e);
            }
            catch(const Ice::LocalException& ex)
            {
                cb->exception(ex);
            }
            catch(...)
            {
                test(false);
            }
        });
#else
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
#endif
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

#ifdef ICE_CPP11_MAPPING
    try
    {
        metrics->opWithUnknownExceptionAsync().get();
        test(false);
    }
    catch(const Ice::UnknownException&)
    {
    }
    metrics->opWithUnknownExceptionAsync(
        [cb]()
        {
            cb->response();
        },
        [cb](exception_ptr e)
        {
            try
            {
                rethrow_exception(e);
            }
            catch(const Ice::UnknownException& ex)
            {
                cb->exception(ex);
            }
            catch(...)
            {
                test(false);
            }
        });
#else
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
#endif
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
#ifdef ICE_CPP11_MAPPING
        try
        {
            metrics->failAsync().get();
            test(false);
        }
        catch(const Ice::ConnectionLostException&)
        {
        }
        metrics->failAsync(
            [cb]()
            {
                cb->response();
            },
            [cb](exception_ptr e)
            {
                try
                {
                    rethrow_exception(e);
                }
                catch(const Ice::ConnectionLostException& ex)
                {
                    cb->exception(ex);
                }
                catch(...)
                {
                    test(false);
                }
            });
#else
        try
        {
            metrics->end_fail(metrics->begin_fail());
            test(false);
        }
        catch(const Ice::ConnectionLostException&)
        {
        }
        metrics->begin_fail(newCallback_Metrics_fail(cb, &Callback::response, &Callback::exception));
#endif
        cb->waitForResponse();
    }
    map = toMap(clientMetrics->getMetricsView("View", timestamp)["Invocation"]);
    test(collocated ? (map.size() == 5) : (map.size() == 6));

    IceMX::InvocationMetricsPtr im1;
    IceMX::ChildInvocationMetricsPtr rim1;
    im1 = ICE_DYNAMIC_CAST(IceMX::InvocationMetrics, map["op"]);
    test(im1->current <= 1 && im1->total == 3 && im1->failures == 0 && im1->retry == 0);
    test(collocated ? (im1->collocated.size() == 1) : (im1->remotes.size() == 1));
    rim1 = ICE_DYNAMIC_CAST(IceMX::ChildInvocationMetrics, collocated ? im1->collocated[0] : im1->remotes[0]);
    test(rim1->current == 0 && rim1->total == 3 && rim1->failures == 0);
    test(rim1->size == 63 && rim1->replySize == 21);

    im1 = ICE_DYNAMIC_CAST(IceMX::InvocationMetrics, map["opWithUserException"]);
    test(im1->current <= 1 && im1->total == 3 && im1->failures == 0 && im1->retry == 0);
    test(collocated ? (im1->collocated.size() == 1) : (im1->remotes.size() == 1));
    rim1 = ICE_DYNAMIC_CAST(IceMX::ChildInvocationMetrics, collocated ? im1->collocated[0] : im1->remotes[0]);
    test(rim1->current == 0 && rim1->total == 3 && rim1->failures == 0);
    test(rim1->size == 114 && rim1->replySize == 69);
    test(im1->userException == 3);

    im1 = ICE_DYNAMIC_CAST(IceMX::InvocationMetrics, map["opWithLocalException"]);
    test(im1->current <= 1 && im1->total == 3 && im1->failures == 3 && im1->retry == 0);
    test(collocated ? (im1->collocated.size() == 1) : (im1->remotes.size() == 1));
    rim1 = ICE_DYNAMIC_CAST(IceMX::ChildInvocationMetrics, collocated ? im1->collocated[0] : im1->remotes[0]);
    test(rim1->current == 0 && rim1->total == 3 && rim1->failures == 0);
    test(rim1->size == 117 && rim1->replySize > 7);
    checkFailure(clientMetrics, "Invocation", im1->id, "::Ice::UnknownLocalException", 3);

    im1 = ICE_DYNAMIC_CAST(IceMX::InvocationMetrics, map["opWithRequestFailedException"]);
    test(im1->current <= 1 && im1->total == 3 && im1->failures == 3 && im1->retry == 0);
    test(collocated ? (im1->collocated.size() == 1) : (im1->remotes.size() == 1));
    rim1 = ICE_DYNAMIC_CAST(IceMX::ChildInvocationMetrics, collocated ? im1->collocated[0] : im1->remotes[0]);
    test(rim1->current == 0 && rim1->total == 3 && rim1->failures == 0);
    test(rim1->size == 141 && rim1->replySize == 120);
    checkFailure(clientMetrics, "Invocation", im1->id, "::Ice::ObjectNotExistException", 3);

    im1 = ICE_DYNAMIC_CAST(IceMX::InvocationMetrics, map["opWithUnknownException"]);
    test(im1->current <= 1 && im1->total == 3 && im1->failures == 3 && im1->retry == 0);
    test(collocated ? (im1->collocated.size() == 1) : (im1->remotes.size() == 1));
    rim1 = ICE_DYNAMIC_CAST(IceMX::ChildInvocationMetrics, collocated ? im1->collocated[0] : im1->remotes[0]);
    test(rim1->current == 0 && rim1->total == 3 && rim1->failures == 0);
    test(rim1->size == 123 && rim1->replySize == 69);
    checkFailure(clientMetrics, "Invocation", im1->id, "::Ice::UnknownException", 3);

    if(!collocated)
    {
        im1 = ICE_DYNAMIC_CAST(IceMX::InvocationMetrics, map["fail"]);
        test(im1->current <= 1 && im1->total == 3 && im1->failures == 3 && im1->retry == 3 && im1->remotes.size() == 6);
        test(im1->remotes[0]->current == 0 && im1->remotes[0]->total == 1 && im1->remotes[0]->failures == 1);
        test(im1->remotes[1]->current == 0 && im1->remotes[1]->total == 1 && im1->remotes[1]->failures == 1);
        test(im1->remotes[2]->current == 0 && im1->remotes[2]->total == 1 && im1->remotes[2]->failures == 1);
        test(im1->remotes[3]->current == 0 && im1->remotes[3]->total == 1 && im1->remotes[3]->failures == 1);
        test(im1->remotes[4]->current == 0 && im1->remotes[4]->total == 1 && im1->remotes[4]->failures == 1);
        test(im1->remotes[5]->current == 0 && im1->remotes[5]->total == 1 && im1->remotes[5]->failures == 1);
        checkFailure(clientMetrics, "Invocation", im1->id, "::Ice::ConnectionLostException", 3);
    }

    testAttribute(clientMetrics, clientProps, update.get(), "Invocation", "parent", "Communicator", op);
    testAttribute(clientMetrics, clientProps, update.get(), "Invocation", "id", "metrics -t -e 1.1 [op]", op);

    testAttribute(clientMetrics, clientProps, update.get(), "Invocation", "operation", "op", op);
    testAttribute(clientMetrics, clientProps, update.get(), "Invocation", "identity", "metrics", op);
    testAttribute(clientMetrics, clientProps, update.get(), "Invocation", "facet", "", op);
    testAttribute(clientMetrics, clientProps, update.get(), "Invocation", "encoding", "1.1", op);
    testAttribute(clientMetrics, clientProps, update.get(), "Invocation", "mode", "twoway", op);
    testAttribute(clientMetrics, clientProps, update.get(), "Invocation", "proxy",
                  "metrics -t -e 1.1:" + endpoint + " -t 60000", op);
    testAttribute(clientMetrics, clientProps, update.get(), "Invocation", "context.entry1", "test", op);
    testAttribute(clientMetrics, clientProps, update.get(), "Invocation", "context.entry2", "", op);
    testAttribute(clientMetrics, clientProps, update.get(), "Invocation", "context.entry3", "", op);

    //
    // Tests with oneway
    //
    props["IceMX.Metrics.View.Map.Invocation.GroupBy"] = "operation";
    props["IceMX.Metrics.View.Map.Invocation.Map.Remote.GroupBy"] = "localPort";
    updateProps(clientProps, serverProps, update.get(), props, "Invocation");

    MetricsPrxPtr metricsOneway = metrics->ice_oneway();
    metricsOneway->op();
#ifdef ICE_CPP11_MAPPING
    metricsOneway->opAsync().get();
    metricsOneway->opAsync(
        [cb]()
        {
            cb->response();
        },
        [cb](exception_ptr e)
        {
            try
            {
                rethrow_exception(e);
            }
            catch(const Ice::Exception& ex)
            {
                cb->exception(ex);
            }
            catch(...)
            {
                test(false);
            }
        });
#else
    metricsOneway->end_op(metricsOneway->begin_op());
    metricsOneway->begin_op(newCallback_Metrics_op(cb, &Callback::response, &Callback::exception));
#endif
    map = toMap(clientMetrics->getMetricsView("View", timestamp)["Invocation"]);
    test(map.size() == 1);

    im1 = ICE_DYNAMIC_CAST(IceMX::InvocationMetrics, map["op"]);
    test(im1->current <= 1 && im1->total == 3 && im1->failures == 0 && im1->retry == 0);
    test(collocated ? (im1->collocated.size() == 1) : (im1->remotes.size() == 1));
    rim1 = ICE_DYNAMIC_CAST(IceMX::ChildInvocationMetrics, collocated ? im1->collocated[0] : im1->remotes[0]);
    test(rim1->current <= 1 && rim1->total == 3 && rim1->failures == 0);
    test(rim1->size == 63 && rim1->replySize == 0);

    testAttribute(clientMetrics, clientProps, update.get(), "Invocation", "mode", "oneway", InvokeOp(metricsOneway));

    //
    // Tests with batch oneway
    //
    props["IceMX.Metrics.View.Map.Invocation.GroupBy"] = "operation";
    props["IceMX.Metrics.View.Map.Invocation.Map.Remote.GroupBy"] = "localPort";
    updateProps(clientProps, serverProps, update.get(), props, "Invocation");

    MetricsPrxPtr metricsBatchOneway = metrics->ice_batchOneway();
    metricsBatchOneway->op();
#ifdef ICE_CPP11_MAPPING
    metricsBatchOneway->opAsync().get();
    metricsBatchOneway->opAsync([cb]() {}, [cb](exception_ptr) {});
#else
    metricsBatchOneway->end_op(metricsBatchOneway->begin_op());
    metricsBatchOneway->begin_op(newCallback_Metrics_op(cb, &Callback::response, &Callback::exception))->waitForCompleted();
#endif

    map = toMap(clientMetrics->getMetricsView("View", timestamp)["Invocation"]);
    test(map.size() == 1);

    im1 = ICE_DYNAMIC_CAST(IceMX::InvocationMetrics, map["op"]);
    test(im1->current == 0 && im1->total == 3 && im1->failures == 0 && im1->retry == 0);
    test(im1->remotes.size() == 0);

    testAttribute(clientMetrics, clientProps, update.get(), "Invocation", "mode", "batch-oneway",
                  InvokeOp(metricsBatchOneway));

    //
    // Tests flushBatchRequests
    //
    props["IceMX.Metrics.View.Map.Invocation.GroupBy"] = "operation";
    props["IceMX.Metrics.View.Map.Invocation.Map.Remote.GroupBy"] = "localPort";
    updateProps(clientProps, serverProps, update.get(), props, "Invocation");

    metricsBatchOneway = metrics->ice_batchOneway();
    metricsBatchOneway->op();

    metricsBatchOneway->ice_flushBatchRequests();
#ifdef ICE_CPP11_MAPPING
    metricsBatchOneway->ice_flushBatchRequestsAsync().get();
    metricsBatchOneway->ice_flushBatchRequestsAsync([cb](exception_ptr) {});
#else
    metricsBatchOneway->end_ice_flushBatchRequests(metricsBatchOneway->begin_ice_flushBatchRequests());
    metricsBatchOneway->begin_ice_flushBatchRequests(
                    Ice::newCallback_Object_ice_flushBatchRequests(cb, &Callback::exception))->waitForCompleted();
#endif

    map = toMap(clientMetrics->getMetricsView("View", timestamp)["Invocation"]);
    test(map.size() == 2);

    im1 = ICE_DYNAMIC_CAST(IceMX::InvocationMetrics, map["ice_flushBatchRequests"]);
    test(im1->current == 0 && im1->total == 3 && im1->failures == 0 && im1->retry == 0);
    if(!collocated)
    {
        test(im1->remotes.size() == 1); // The first operation got sent over a connection
    }

    if(!collocated)
    {
        clearView(clientProps, serverProps, update.get());

        Ice::ConnectionPtr con = metricsBatchOneway->ice_getConnection();

        metricsBatchOneway = metricsBatchOneway->ice_fixed(con);
        metricsBatchOneway->op();

        con->flushBatchRequests(ICE_SCOPED_ENUM(Ice::CompressBatch, No));
#ifdef ICE_CPP11_MAPPING
        con->flushBatchRequestsAsync(ICE_SCOPED_ENUM(Ice::CompressBatch, No)).get();
        con->flushBatchRequestsAsync(ICE_SCOPED_ENUM(Ice::CompressBatch, No), [cb](exception_ptr) {});
#else
        con->end_flushBatchRequests(con->begin_flushBatchRequests(ICE_SCOPED_ENUM(Ice::CompressBatch, No)));
        con->begin_flushBatchRequests(ICE_SCOPED_ENUM(Ice::CompressBatch, No),
            Ice::newCallback_Connection_flushBatchRequests(cb, &Callback::exception))->waitForCompleted();
#endif
        map = toMap(clientMetrics->getMetricsView("View", timestamp)["Invocation"]);
        test(map.size() == 3);

        im1 = ICE_DYNAMIC_CAST(IceMX::InvocationMetrics, map["flushBatchRequests"]);
        test(im1->current == 0 && im1->total == 3 && im1->failures == 0 && im1->retry == 0);
        test(im1->remotes.size() == 1); // The first operation got sent over a connection

        clearView(clientProps, serverProps, update.get());
        metricsBatchOneway->op();

        communicator->flushBatchRequests(ICE_SCOPED_ENUM(Ice::CompressBatch, No));
#ifdef ICE_CPP11_MAPPING
        communicator->flushBatchRequestsAsync(ICE_SCOPED_ENUM(Ice::CompressBatch, No)).get();
        communicator->flushBatchRequestsAsync(ICE_SCOPED_ENUM(Ice::CompressBatch, No),
                                              [cb](exception_ptr) {});
#else
        communicator->end_flushBatchRequests(
            communicator->begin_flushBatchRequests(ICE_SCOPED_ENUM(Ice::CompressBatch, No)));
        communicator->begin_flushBatchRequests(ICE_SCOPED_ENUM(Ice::CompressBatch, No),
            Ice::newCallback_Communicator_flushBatchRequests(cb, &Callback::exception))->waitForCompleted();
#endif
        map = toMap(clientMetrics->getMetricsView("View", timestamp)["Invocation"]);
        test(map.size() == 2);

        im1 = ICE_DYNAMIC_CAST(IceMX::InvocationMetrics, map["flushBatchRequests"]);
        test(im1->current == 0 && im1->total == 3 && im1->failures == 0 && im1->retry == 0);
        test(im1->remotes.size() == 1); // The first operation got sent over a connection
    }
    cout << "ok" << endl;

    cout << "testing metrics view enable/disable..." << flush;

    Ice::StringSeq disabledViews;
    props["IceMX.Metrics.View.GroupBy"] = "none";
    props["IceMX.Metrics.View.Disabled"] = "0";
    updateProps(clientProps, serverProps, update.get(), props, "Thread");
    test(!clientMetrics->getMetricsView("View", timestamp)["Thread"].empty());
    test(clientMetrics->getMetricsViewNames(disabledViews).size() == 1 && disabledViews.empty());

    props["IceMX.Metrics.View.Disabled"] = "1";
    updateProps(clientProps, serverProps, update.get(), props, "Thread");
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
#if !defined(ICE_OS_UWP) && TARGET_OS_IPHONE==0
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
#if !defined(ICE_OS_UWP) && TARGET_OS_IPHONE==0
        test(obsv->endpointLookupObserver->getCurrent() == 0);
#endif
        waitForCurrent(obsv->invocationObserver->remoteObserver, 0);
        test(obsv->invocationObserver->remoteObserver->getCurrent() == 0);
    }
    else
    {
        waitForCurrent(obsv->invocationObserver->collocatedObserver, 0);
        test(obsv->invocationObserver->collocatedObserver->getCurrent() == 0);
    }

    waitForCurrent(obsv->dispatchObserver, 0);
    test(obsv->dispatchObserver->getCurrent() == 0);

    waitForCurrent(obsv->invocationObserver, 0);
    test(obsv->invocationObserver->getCurrent() == 0);

    test(obsv->threadObserver->getFailedCount() == 0);
    if(!collocated)
    {
        test(obsv->connectionObserver->getFailedCount() > 0);
        test(obsv->connectionEstablishmentObserver->getFailedCount() > 0);
#if !defined(ICE_OS_UWP) && TARGET_OS_IPHONE==0
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
