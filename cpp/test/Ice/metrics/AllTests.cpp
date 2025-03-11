// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "InstrumentationI.h"
#include "Test.h"
#include "TestHelper.h"

#include <thread>

using namespace std;
using namespace Test;

namespace
{
    string getPort(const Ice::PropertiesAdminPrx& p, int testPort = 0)
    {
        ostringstream os;
        os << TestHelper::getTestPort(p->ice_getCommunicator()->getProperties(), testPort);
        return os.str();
    }

    Ice::PropertyDict
    getClientProps(const Ice::PropertiesAdminPrx& pa, const Ice::PropertyDict& orig, const string& m = string())
    {
        Ice::PropertyDict props = pa->getPropertiesForPrefix("IceMX.Metrics");
        for (auto& prop : props)
        {
            prop.second = "";
        }
        for (const auto& p : orig)
        {
            props[p.first] = p.second;
        }
        string map;
        if (!m.empty())
        {
            map += "Map." + m + '.';
        }
        props["IceMX.Metrics.View." + map + "Reject.parent"] = "Ice\\.Admin";
        props["IceMX.Metrics.View." + map + "Accept.endpointPort"] = getPort(pa);
        props["IceMX.Metrics.View." + map + "Reject.identity"] = ".*/admin|controller";
        return props;
    }

    Ice::PropertyDict
    getServerProps(const Ice::PropertiesAdminPrx& pa, const Ice::PropertyDict& orig, const string& m = string())
    {
        Ice::PropertyDict props = pa->getPropertiesForPrefix("IceMX.Metrics");
        for (auto& prop : props)
        {
            prop.second = "";
        }
        for (const auto& p : orig)
        {
            props[p.first] = p.second;
        }
        string map;
        if (!m.empty())
        {
            map += "Map." + m + '.';
        }
        props["IceMX.Metrics.View." + map + "Reject.parent"] = "Ice\\.Admin|Controller";

        // Regular expression to match server test endpoint 0 and test endpoint 1
        ostringstream os;
        os << getPort(pa, 0) << "|" << getPort(pa, 1);

        props["IceMX.Metrics.View." + map + "Accept.endpointPort"] = os.str();
        return props;
    }

    IceMX::ConnectionMetricsPtr getServerConnectionMetrics(const IceMX::MetricsAdminPrx& metrics, int64_t expected)
    {
        IceMX::ConnectionMetricsPtr s;
        int nRetry = 30;
        int64_t timestamp;
        s = dynamic_pointer_cast<IceMX::ConnectionMetrics>(metrics->getMetricsView("View", timestamp)["Connection"][0]);
        while (s->sentBytes != expected && nRetry-- > 0)
        {
            // On some platforms, it's necessary to wait a little before obtaining the server metrics
            // to get an accurate sentBytes metric. The sentBytes metric is updated before the response
            // to the operation is sent and getMetricsView can be dispatched before the metric is really
            // updated.
            this_thread::sleep_for(chrono::milliseconds(100));
            s = dynamic_pointer_cast<IceMX::ConnectionMetrics>(
                metrics->getMetricsView("View", timestamp)["Connection"][0]);
        }
        return s;
    }

    class UpdateCallbackI
    {
    public:
        UpdateCallbackI(Ice::PropertiesAdminPrx serverProps) : _serverProps(std::move(serverProps)) {}

        void waitForUpdate()
        {
            {
                unique_lock lock(_mutex);
                _condition.wait(lock, [this] { return _updated; });
            }

            // Ensure that the previous updates were committed, the setProperties call returns before
            // notifying the callbacks so to ensure all the update callbacks have be notified we call
            // a second time, this will block until all the notifications from the first update have
            // completed.
            _serverProps->setProperties(Ice::PropertyDict());

            lock_guard lock(_mutex);
            _updated = false;
        }

        void updated(const Ice::PropertyDict&)
        {
            lock_guard lock(_mutex);
            _updated = true;
            _condition.notify_one();
        }

    private:
        bool _updated{false};
        Ice::PropertiesAdminPrx _serverProps;
        std::mutex _mutex;
        std::condition_variable _condition;
    };
    using UpdateCallbackIPtr = std::shared_ptr<UpdateCallbackI>;

    void waitForCurrent(const IceMX::MetricsAdminPrx& metrics, const string& viewName, const string& map, int value)
    {
        while (true)
        {
            int64_t timestamp;
            IceMX::MetricsView view = metrics->getMetricsView(viewName, timestamp);
            test(view.find(map) != view.end());
            bool ok = true;
            for (const auto& m : view[map])
            {
                if (m->current != value)
                {
                    ok = false;
                    break;
                }
            }
            if (ok)
            {
                break;
            }
            this_thread::sleep_for(chrono::milliseconds(50));
        }
    }

    void waitForCurrent(const ObserverIPtr& observer, int value)
    {
        for (int i = 0; i < 10; ++i)
        {
            if (observer->getCurrent() != value)
            {
                this_thread::sleep_for(chrono::milliseconds(50));
            }
            else
            {
                break;
            }
        }
    };

    template<typename T>
    void testAttribute(
        const IceMX::MetricsAdminPrx& metrics,
        const Ice::PropertiesAdminPrx& props,
        UpdateCallbackI* update,
        const string& map,
        const string& attr,
        const string& value,
        const T& func)
    {
        Ice::PropertyDict dict;
        dict["IceMX.Metrics.View.Map." + map + ".GroupBy"] = attr;
        if (props->ice_getIdentity().category == "client")
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
        int64_t timestamp;
        IceMX::MetricsView view = metrics->getMetricsView("View", timestamp);
        if (view.find(map) == view.end() || view[map].empty())
        {
            if (!value.empty())
            {
                cerr << "no map `" << map << "' for group by = `" << attr << "'" << endl;
                test(false);
            }
        }
        else if (view[map].size() != 1 || view[map][0]->id != value)
        {
            cerr << "size of view[map] is: " << view[map].size() << endl;
            cerr << "expected value for attribute " << attr << " = " << value << "; got " << view[map][0]->id << endl;
            test(false);
        }

        dict.clear();
        if (props->ice_getIdentity().category == "client")
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
        void operator()() const {}
    };

    struct Connect
    {
        Connect(Ice::ObjectPrx proxyP) : proxy(std::move(proxyP)) {}

        void operator()() const
        {
            if (proxy->ice_getCachedConnection())
            {
                proxy->ice_getCachedConnection()->close().get();
            }
            try
            {
                proxy->ice_ping();
            }
            catch (const Ice::LocalException&)
            {
            }
            if (proxy->ice_getCachedConnection())
            {
                proxy->ice_getCachedConnection()->close().get();
            }
        }

        Ice::ObjectPrx proxy;
    };

    struct InvokeOp
    {
        InvokeOp(Test::MetricsPrx proxyP) : proxy(std::move(proxyP)) {}

        void operator()() const
        {
            Ice::Context ctx;
            ctx["entry1"] = "test";
            ctx["entry2"] = "";
            proxy->op(ctx);
        }

        Test::MetricsPrx proxy;
    };

    void testAttribute(
        const IceMX::MetricsAdminPrx& metrics,
        const Ice::PropertiesAdminPrx& props,
        UpdateCallbackI* update,
        const string& map,
        const string& attr,
        const string& value)
    {
        testAttribute(metrics, props, update, map, attr, value, Void());
    }

    void updateProps(
        const Ice::PropertiesAdminPrx& cprops,
        const Ice::PropertiesAdminPrx& sprops,
        UpdateCallbackI* callback,
        const Ice::PropertyDict& props,
        const string& map = string())
    {
        if (sprops->ice_getConnection())
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

    void checkFailure(
        const IceMX::MetricsAdminPrx& m,
        const string& map,
        const string& id,
        const string& failure,
        int count = 0)
    {
        IceMX::MetricsFailures f = m->getMetricsFailures("View", map, id);
        if (f.failures.find(failure) == f.failures.end())
        {
            cerr << "couldn't find failure `" << failure << "' for `" << id << "'" << endl;
            test(false);
        }
        if (count > 0 && f.failures[failure] != count)
        {
            cerr << "count for failure `" << failure << "' of `" << id << "' is different from expected: ";
            cerr << count << " != " << f.failures[failure] << endl;
            test(false);
        }
    }

    map<string, IceMX::MetricsPtr> toMap(const IceMX::MetricsMap& mmap)
    {
        map<string, IceMX::MetricsPtr> m;
        for (const auto& p : mmap)
        {
            m.insert(make_pair(p->id, p));
        }
        return m;
    }
}

MetricsPrx
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
    string forwardingEndpoint;
    {
        ostringstream os;
        os << protocol << " -h " << host << " -p " << helper->getTestPort(1);
        forwardingEndpoint = os.str();
    }

    MetricsPrx metrics(communicator, "metrics:" + endpoint);
    bool collocated = !metrics->ice_getConnection();

    cout << "testing metrics admin facet checkedCast... " << flush;
    optional<Ice::ObjectPrx> admin = communicator->getAdmin();
    test(admin);

    auto clientProps = Ice::uncheckedCast<Ice::PropertiesAdminPrx>(admin->ice_facet("Properties"));
    auto clientMetrics = Ice::uncheckedCast<IceMX::MetricsAdminPrx>(admin->ice_facet("Metrics"));

    admin = metrics->getAdmin();

    auto serverProps = Ice::uncheckedCast<Ice::PropertiesAdminPrx>(admin->ice_facet("Properties"));
    auto serverMetrics = Ice::uncheckedCast<IceMX::MetricsAdminPrx>(admin->ice_facet("Metrics"));

    UpdateCallbackIPtr update = make_shared<UpdateCallbackI>(serverProps);

    dynamic_pointer_cast<Ice::NativePropertiesAdmin>(communicator->findAdminFacet("Properties"))
        ->addUpdateCallback([update](const Ice::PropertyDict& changes) { update->updated(changes); });

    cout << "ok" << endl;

    Ice::PropertyDict props;

    cout << "testing group by none..." << flush;

    props["IceMX.Metrics.View.GroupBy"] = "none";
    updateProps(clientProps, serverProps, update.get(), props);

    int threadCount = 4;

    int64_t timestamp;
    IceMX::MetricsView view = clientMetrics->getMetricsView("View", timestamp);
    if (!collocated)
    {
        test(
            view["Connection"].size() == 1 && view["Connection"][0]->current == 1 && view["Connection"][0]->total == 1);
    }

    test(
        view["Thread"].size() == 1 && view["Thread"][0]->current == threadCount &&
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
    if (!collocated)
    {
        test(view["Connection"].size() == 2);
    }

    test(static_cast<int>(view["Thread"].size()) == threadCount);
    test(view["Invocation"].size() == 1);

    IceMX::InvocationMetricsPtr invoke = dynamic_pointer_cast<IceMX::InvocationMetrics>(view["Invocation"][0]);
    test(invoke->id.find("[ice_ping]") > 0 && invoke->current == 0 && invoke->total == 5);

    if (!collocated)
    {
        test(invoke->remotes.size() == 2);
        test(invoke->remotes[0]->total == 2);
        test(invoke->remotes[1]->total == 3);
    }
    else
    {
        test(invoke->collocated.size() == 1);
        test(invoke->collocated[0]->total == 5);
    }

    view = serverMetrics->getMetricsView("View", timestamp);
    if (serverMetrics->ice_getConnection())
    {
        test(static_cast<int>(view["Thread"].size()) > threadCount);
        test(view["Connection"].size() == 2);
    }
    test(view["Dispatch"].size() == 1);
    test(view["Dispatch"][0]->current == 0 && view["Dispatch"][0]->total == 5);
    test(view["Dispatch"][0]->id.find("[ice_ping]") > 0);

    if (!collocated)
    {
        metrics->ice_getConnection()->close().get();
        metrics->ice_connectionId("Con1")->ice_getConnection()->close().get();

        waitForCurrent(clientMetrics, "View", "Connection", 0);
        waitForCurrent(serverMetrics, "View", "Connection", 0);
    }

    clearView(clientProps, serverProps, update.get());

    cout << "ok" << endl;

    map<string, IceMX::MetricsPtr> map;

    string type;
    string isSecure;
    if (!collocated)
    {
        Ice::EndpointInfoPtr endpointInfo = metrics->ice_getConnection()->getEndpoint()->getInfo();
        {
            ostringstream os;
            os << endpointInfo->type();
            type = os.str();
        }
        isSecure = endpointInfo->secure() ? "true" : "false";
    }
    if (!collocated)
    {
        cout << "testing connection metrics... " << flush;

        props["IceMX.Metrics.View.Map.Connection.GroupBy"] = "none";
        updateProps(clientProps, serverProps, update.get(), props, "Connection");

        test(clientMetrics->getMetricsView("View", timestamp)["Connection"].empty());
        test(serverMetrics->getMetricsView("View", timestamp)["Connection"].empty());

        metrics->ice_ping();

        IceMX::ConnectionMetricsPtr cm1, sm1, cm2, sm2;
        cm1 = dynamic_pointer_cast<IceMX::ConnectionMetrics>(
            clientMetrics->getMetricsView("View", timestamp)["Connection"][0]);
        sm1 = dynamic_pointer_cast<IceMX::ConnectionMetrics>(
            serverMetrics->getMetricsView("View", timestamp)["Connection"][0]);
        sm1 = getServerConnectionMetrics(serverMetrics, 25);
        test(cm1->total == 1 && sm1->total == 1);

        metrics->ice_ping();

        cm2 = dynamic_pointer_cast<IceMX::ConnectionMetrics>(
            clientMetrics->getMetricsView("View", timestamp)["Connection"][0]);
        sm2 = getServerConnectionMetrics(serverMetrics, 50);

        test(cm2->sentBytes - cm1->sentBytes == 45);         // 45 for ice_ping request
        test(cm2->receivedBytes - cm1->receivedBytes == 25); // 25 bytes for ice_ping response
        test(sm2->receivedBytes - sm1->receivedBytes == 45);
        test(sm2->sentBytes - sm1->sentBytes == 25);

        cm1 = cm2;
        sm1 = sm2;

        Test::ByteSeq bs;
        metrics->opByteS(bs);

        cm2 = dynamic_pointer_cast<IceMX::ConnectionMetrics>(
            clientMetrics->getMetricsView("View", timestamp)["Connection"][0]);
        sm2 = getServerConnectionMetrics(serverMetrics, sm1->sentBytes + cm2->receivedBytes - cm1->receivedBytes);
        int64_t requestSz = cm2->sentBytes - cm1->sentBytes;
        int64_t replySz = cm2->receivedBytes - cm1->receivedBytes;

        cm1 = cm2;
        sm1 = sm2;

        bs.resize(456);
        metrics->opByteS(bs);

        cm2 = dynamic_pointer_cast<IceMX::ConnectionMetrics>(
            clientMetrics->getMetricsView("View", timestamp)["Connection"][0]);
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

        cm2 = dynamic_pointer_cast<IceMX::ConnectionMetrics>(
            clientMetrics->getMetricsView("View", timestamp)["Connection"][0]);
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

        ControllerPrx controller(communicator, "controller:" + helper->getTestEndpoint(2));
        controller->hold();

        map = toMap(clientMetrics->getMetricsView("View", timestamp)["Connection"]);
        test(map["active"]->current == 1);
        map = toMap(serverMetrics->getMetricsView("View", timestamp)["Connection"]);
        test(map["holding"]->current == 1);

        metrics->ice_getConnection()->close(nullptr, nullptr);

        map = toMap(clientMetrics->getMetricsView("View", timestamp)["Connection"]);
        test(map["closing"]->current == 1);
        map = toMap(serverMetrics->getMetricsView("View", timestamp)["Connection"]);
        test(map["holding"]->current == 1);

        controller->resume();

        map = toMap(serverMetrics->getMetricsView("View", timestamp)["Connection"]);
        test(map["holding"]->current == 0);

        props["IceMX.Metrics.View.Map.Connection.GroupBy"] = "none";
        updateProps(clientProps, serverProps, update.get(), props, "Connection");

        metrics->ice_getConnection()->close().get();

        // TODO: this appears necessary on slow macos VMs to give time to the server to clean-up the connection.
        this_thread::sleep_for(chrono::milliseconds(100));

        MetricsPrx m = metrics->ice_connectionId("Con1");
        m->ice_ping();

        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "parent", "Communicator");
        // testAttribute(clientMetrics, clientProps, update.get(), "Connection", "id", "");
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "endpoint", endpoint + " -t infinite");
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "endpointType", type);
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "endpointIsDatagram", "false");
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "endpointIsSecure", isSecure);
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "endpointTimeout", "-1");
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "endpointCompress", "false");
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "endpointHost", host);
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "endpointPort", port);

        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "incoming", "false");
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "adapterName", "");
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "connectionId", "Con1");
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "localHost", host);
        // testAttribute(clientMetrics, clientProps, update.get(), "Connection", "localPort", "");
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "remoteHost", host);
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "remotePort", port);
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "mcastHost", "");
        testAttribute(clientMetrics, clientProps, update.get(), "Connection", "mcastPort", "");

        m->ice_getConnection()->close().get();

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

        metrics->ice_getConnection()->close().get();
        controller->hold();
        try
        {
            communicator->stringToProxy("test:" + endpoint)->ice_connectionId("Con2")->ice_ping();
            test(false);
        }
        catch (const Ice::ConnectTimeoutException&)
        {
        }
        catch (const Ice::LocalException&)
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
        testAttribute(clientMetrics, clientProps, update.get(), "ConnectionEstablishment", "endpoint", endpoint, c);

        testAttribute(clientMetrics, clientProps, update.get(), "ConnectionEstablishment", "endpointType", type, c);
        testAttribute(
            clientMetrics,
            clientProps,
            update.get(),
            "ConnectionEstablishment",
            "endpointIsDatagram",
            "false",
            c);
        testAttribute(
            clientMetrics,
            clientProps,
            update.get(),
            "ConnectionEstablishment",
            "endpointIsSecure",
            isSecure,
            c);
        testAttribute(
            clientMetrics,
            clientProps,
            update.get(),
            "ConnectionEstablishment",
            "endpointTimeout",
            "60000",
            c);
        testAttribute(
            clientMetrics,
            clientProps,
            update.get(),
            "ConnectionEstablishment",
            "endpointCompress",
            "false",
            c);
        testAttribute(clientMetrics, clientProps, update.get(), "ConnectionEstablishment", "endpointHost", host, c);
        testAttribute(clientMetrics, clientProps, update.get(), "ConnectionEstablishment", "endpointPort", port, c);

        cout << "ok" << endl;

        //
        // Ice doesn't do any endpoint lookup with iOS.
        //
#if TARGET_OS_IPHONE == 0
        cout << "testing endpoint lookup metrics... " << flush;

        props["IceMX.Metrics.View.Map.EndpointLookup.GroupBy"] = "id";
        updateProps(clientProps, serverProps, update.get(), props, "EndpointLookup");
        test(clientMetrics->getMetricsView("View", timestamp)["EndpointLookup"].empty());

        Ice::ObjectPrx prx(communicator, "metrics:" + protocol + " -h localhost -t 500 -p " + port);
        try
        {
            prx->ice_ping();
            prx->ice_getConnection()->close().get();
        }
        catch (const Ice::LocalException&)
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
        catch (const Ice::DNSException&)
        {
            dnsException = true;
        }
        catch (const Ice::LocalException&)
        {
            // Some DNS servers don't fail on unknown DNS names.
        }
        test(clientMetrics->getMetricsView("View", timestamp)["EndpointLookup"].size() == 2);
        m1 = clientMetrics->getMetricsView("View", timestamp)["EndpointLookup"][1];
        test(
            m1->id == "tcp -h unknownfoo.zeroc.com -p " + port + " -t 500" && m1->total == 2 &&
            (!dnsException || m1->failures == 2));
        if (dnsException)
        {
            checkFailure(clientMetrics, "EndpointLookup", m1->id, "::Ice::DNSException", 2);
        }

        c = Connect(prx);

        testAttribute(clientMetrics, clientProps, update.get(), "EndpointLookup", "parent", "Communicator", c);

        string expected = protocol + " -h localhost -p " + port + " -t 500";

        testAttribute(clientMetrics, clientProps, update.get(), "EndpointLookup", "id", expected, c);
        testAttribute(clientMetrics, clientProps, update.get(), "EndpointLookup", "endpoint", expected, c);
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
    catch (const Test::UserEx&)
    {
    }
    try
    {
        metrics->opWithRequestFailedException();
        test(false);
    }
    catch (const Ice::RequestFailedException&)
    {
    }
    try
    {
        metrics->opWithLocalException();
        test(false);
    }
    catch (const Ice::LocalException&)
    {
    }
    try
    {
        metrics->opWithUnknownException();
        test(false);
    }
    catch (const Ice::UnknownException&)
    {
    }
    if (!collocated)
    {
        try
        {
            metrics->fail();
            test(false);
        }
        catch (const Ice::ConnectionLostException&)
        {
        }
    }

    map = toMap(serverMetrics->getMetricsView("View", timestamp)["Dispatch"]);
    if (!collocated)
    {
        test(map.size() == 6);
    }
    else
    {
        test(map.size() == 5);
    }

    IceMX::DispatchMetricsPtr dm1 = dynamic_pointer_cast<IceMX::DispatchMetrics>(map["op"]);
    test(dm1->current <= 1 && dm1->total == 1 && dm1->failures == 0 && dm1->userException == 0);
    test(dm1->size == 21 && dm1->replySize == 7);

    dm1 = dynamic_pointer_cast<IceMX::DispatchMetrics>(map["opWithUserException"]);
    test(dm1->current <= 1 && dm1->total == 1 && dm1->failures == 0 && dm1->userException == 1);
    test(dm1->size == 38 && dm1->replySize == 27);

    dm1 = dynamic_pointer_cast<IceMX::DispatchMetrics>(map["opWithLocalException"]);
    test(dm1->current <= 1 && dm1->total == 1 && dm1->failures == 1 && dm1->userException == 0);
    checkFailure(serverMetrics, "Dispatch", dm1->id, "::Ice::SyscallException", 1);
    test(dm1->size == 39 && dm1->replySize > 7); // Reply contains the exception stack depending on the OS.

    dm1 = dynamic_pointer_cast<IceMX::DispatchMetrics>(map["opWithRequestFailedException"]);
    test(dm1->current <= 1 && dm1->total == 1 && dm1->failures == 1 && dm1->userException == 0);
    checkFailure(serverMetrics, "Dispatch", dm1->id, "::Ice::ObjectNotExistException", 1);
    test(dm1->size == 47 && dm1->replySize == 40);

    dm1 = dynamic_pointer_cast<IceMX::DispatchMetrics>(map["opWithUnknownException"]);
    test(dm1->current <= 1 && dm1->total == 1 && dm1->failures == 1 && dm1->userException == 0);
    checkFailure(serverMetrics, "Dispatch", dm1->id, "unknown", 1);
    test(dm1->size == 41 && dm1->replySize == 45);

    InvokeOp op(metrics);

    testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "parent", "TestAdapter", op);
    testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "id", "metrics [op]", op);
    if (!collocated)
    {
        testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "endpoint", endpoint, op);
        // testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "connection", "", op);

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
        // testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "remotePort", port, op);
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

    cout << "testing dispatch metrics with forwarding object adapter... " << flush;
    MetricsPrx indirectMetrics(communicator, "metrics:" + forwardingEndpoint);
    InvokeOp secondOp(indirectMetrics);

    testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "parent", "ForwardingAdapter", secondOp);
    testAttribute(serverMetrics, serverProps, update.get(), "Dispatch", "id", "metrics [op]", secondOp);
    cout << "ok" << endl;

    cout << "testing invocation metrics... " << flush;

    props["IceMX.Metrics.View.Map.Invocation.GroupBy"] = "operation";
    props["IceMX.Metrics.View.Map.Invocation.Map.Remote.GroupBy"] = "id";
    props["IceMX.Metrics.View.Map.Invocation.Map.Collocated.GroupBy"] = "parent";
    updateProps(clientProps, serverProps, update.get(), props, "Invocation");
    test(serverMetrics->getMetricsView("View", timestamp)["Invocation"].empty());

    metrics->op();
    metrics->opAsync().get();

    {
        std::promise<void> p;
        metrics->opAsync([&p]() { p.set_value(); }, [](exception_ptr) { test(false); });
        p.get_future().wait();
    }

    // User exception
    try
    {
        metrics->opWithUserException();
        test(false);
    }
    catch (const Test::UserEx&)
    {
    }

    try
    {
        metrics->opWithUserExceptionAsync().get();
        test(false);
    }
    catch (const Test::UserEx&)
    {
    }
    catch (...)
    {
        test(false);
    }

    {
        std::promise<void> p;
        metrics->opWithUserExceptionAsync(
            []() { test(false); },
            [&p](exception_ptr e)
            {
                try
                {
                    rethrow_exception(e);
                }
                catch (const Test::UserEx&)
                {
                    p.set_value();
                }
                catch (...)
                {
                    test(false);
                }
            });
        p.get_future().wait();
    }

    // Request failed exception
    try
    {
        metrics->opWithRequestFailedException();
        test(false);
    }
    catch (const Ice::RequestFailedException&)
    {
    }

    try
    {
        metrics->opWithRequestFailedExceptionAsync().get();
        test(false);
    }
    catch (const Ice::RequestFailedException&)
    {
    }

    {
        std::promise<void> p;
        metrics->opWithRequestFailedExceptionAsync(
            []() { test(false); },
            [&p](exception_ptr e)
            {
                try
                {
                    rethrow_exception(e);
                }
                catch (const Ice::RequestFailedException&)
                {
                    p.set_value();
                }
                catch (...)
                {
                    test(false);
                }
            });
        p.get_future().wait();
    }

    // Local exception
    try
    {
        metrics->opWithLocalException();
        test(false);
    }
    catch (const Ice::LocalException&)
    {
    }

    try
    {
        metrics->opWithLocalExceptionAsync().get();
        test(false);
    }
    catch (const Ice::LocalException&)
    {
    }

    {
        std::promise<void> p;
        metrics->opWithLocalExceptionAsync(
            []() { test(false); },
            [&p](exception_ptr e)
            {
                try
                {
                    rethrow_exception(e);
                }
                catch (const Ice::LocalException&)
                {
                    p.set_value();
                }
                catch (...)
                {
                    test(false);
                }
            });
        p.get_future().wait();
    }

    // Unknown exception
    try
    {
        metrics->opWithUnknownException();
        test(false);
    }
    catch (const Ice::UnknownException&)
    {
    }

    try
    {
        metrics->opWithUnknownExceptionAsync().get();
        test(false);
    }
    catch (const Ice::UnknownException&)
    {
    }

    {
        std::promise<void> p;
        metrics->opWithUnknownExceptionAsync(
            []() { test(false); },
            [&p](exception_ptr e)
            {
                try
                {
                    rethrow_exception(e);
                }
                catch (const Ice::UnknownException&)
                {
                    p.set_value();
                }
                catch (...)
                {
                    test(false);
                }
            });
        p.get_future().wait();
    }

    // Fail
    if (!collocated)
    {
        try
        {
            metrics->fail();
            test(false);
        }
        catch (const Ice::ConnectionLostException&)
        {
        }

        try
        {
            metrics->failAsync().get();
            test(false);
        }
        catch (const Ice::ConnectionLostException&)
        {
        }

        std::promise<void> p;
        metrics->failAsync(
            []() { test(false); },
            [&p](exception_ptr e)
            {
                try
                {
                    rethrow_exception(e);
                }
                catch (const Ice::ConnectionLostException&)
                {
                    p.set_value();
                }
                catch (...)
                {
                    test(false);
                }
            });
        p.get_future().wait();
    }

    map = toMap(clientMetrics->getMetricsView("View", timestamp)["Invocation"]);
    test(collocated ? (map.size() == 5) : (map.size() == 6));

    IceMX::InvocationMetricsPtr im1;
    IceMX::ChildInvocationMetricsPtr rim1;
    im1 = dynamic_pointer_cast<IceMX::InvocationMetrics>(map["op"]);
    test(im1->current <= 1 && im1->total == 3 && im1->failures == 0 && im1->retry == 0);
    test(collocated ? (im1->collocated.size() == 1) : (im1->remotes.size() == 1));
    rim1 = dynamic_pointer_cast<IceMX::ChildInvocationMetrics>(collocated ? im1->collocated[0] : im1->remotes[0]);
    test(rim1->current == 0 && rim1->total == 3 && rim1->failures == 0);
    test(rim1->size == 63 && rim1->replySize == 21);

    im1 = dynamic_pointer_cast<IceMX::InvocationMetrics>(map["opWithUserException"]);
    test(im1->current <= 1 && im1->total == 3 && im1->failures == 0 && im1->retry == 0);
    test(collocated ? (im1->collocated.size() == 1) : (im1->remotes.size() == 1));
    rim1 = dynamic_pointer_cast<IceMX::ChildInvocationMetrics>(collocated ? im1->collocated[0] : im1->remotes[0]);
    test(rim1->current == 0 && rim1->total == 3 && rim1->failures == 0);
    test(rim1->size == 114 && rim1->replySize == 81);
    test(im1->userException == 3);

    im1 = dynamic_pointer_cast<IceMX::InvocationMetrics>(map["opWithLocalException"]);
    test(im1->current <= 1 && im1->total == 3 && im1->failures == 3 && im1->retry == 0);
    test(collocated ? (im1->collocated.size() == 1) : (im1->remotes.size() == 1));
    rim1 = dynamic_pointer_cast<IceMX::ChildInvocationMetrics>(collocated ? im1->collocated[0] : im1->remotes[0]);
    test(rim1->current == 0 && rim1->total == 3 && rim1->failures == 0);
    test(rim1->size == 117 && rim1->replySize > 7);
    checkFailure(clientMetrics, "Invocation", im1->id, "::Ice::UnknownLocalException", 3);

    im1 = dynamic_pointer_cast<IceMX::InvocationMetrics>(map["opWithRequestFailedException"]);
    test(im1->current <= 1 && im1->total == 3 && im1->failures == 3 && im1->retry == 0);
    test(collocated ? (im1->collocated.size() == 1) : (im1->remotes.size() == 1));
    rim1 = dynamic_pointer_cast<IceMX::ChildInvocationMetrics>(collocated ? im1->collocated[0] : im1->remotes[0]);
    test(rim1->current == 0 && rim1->total == 3 && rim1->failures == 0);
    test(rim1->size == 141 && rim1->replySize == 120);
    checkFailure(clientMetrics, "Invocation", im1->id, "::Ice::ObjectNotExistException", 3);

    im1 = dynamic_pointer_cast<IceMX::InvocationMetrics>(map["opWithUnknownException"]);
    test(im1->current <= 1 && im1->total == 3 && im1->failures == 3 && im1->retry == 0);
    test(collocated ? (im1->collocated.size() == 1) : (im1->remotes.size() == 1));
    rim1 = dynamic_pointer_cast<IceMX::ChildInvocationMetrics>(collocated ? im1->collocated[0] : im1->remotes[0]);
    test(rim1->current == 0 && rim1->total == 3 && rim1->failures == 0);
    test(rim1->size == 123 && rim1->replySize == 135);

    checkFailure(clientMetrics, "Invocation", im1->id, "::Ice::UnknownException", 3);

    if (!collocated)
    {
        im1 = dynamic_pointer_cast<IceMX::InvocationMetrics>(map["fail"]);
        test(im1->current <= 1 && im1->total == 3 && im1->failures == 3 && im1->retry == 3 && im1->remotes.size() == 1);
        rim1 = dynamic_pointer_cast<IceMX::ChildInvocationMetrics>(im1->remotes[0]);
        if (rim1->current != 0 || rim1->total != 6 || rim1->failures != 6)
        {
            cerr << "rim1->current = " << rim1->current << endl;
            cerr << "rim1->total = " << rim1->total << endl;
            cerr << "rim1->failures = " << rim1->failures << endl;
            IceMX::MetricsFailures f = clientMetrics->getMetricsFailures("View", "Invocation", im1->id);
            for (auto& failure : f.failures)
            {
                cerr << failure.first << " = " << failure.second << endl;
            }
        }
        test(rim1->current == 0 && rim1->total == 6 && rim1->failures == 6);
        checkFailure(clientMetrics, "Invocation", im1->id, "::Ice::ConnectionLostException", 3);
    }

    testAttribute(clientMetrics, clientProps, update.get(), "Invocation", "parent", "Communicator", op);
    testAttribute(clientMetrics, clientProps, update.get(), "Invocation", "id", "metrics [op]", op);

    testAttribute(clientMetrics, clientProps, update.get(), "Invocation", "operation", "op", op);
    testAttribute(clientMetrics, clientProps, update.get(), "Invocation", "identity", "metrics", op);
    testAttribute(clientMetrics, clientProps, update.get(), "Invocation", "facet", "", op);
    testAttribute(clientMetrics, clientProps, update.get(), "Invocation", "encoding", "1.1", op);
    testAttribute(clientMetrics, clientProps, update.get(), "Invocation", "mode", "twoway", op);
    testAttribute(clientMetrics, clientProps, update.get(), "Invocation", "proxy", "metrics:" + endpoint, op);
    testAttribute(clientMetrics, clientProps, update.get(), "Invocation", "context.entry1", "test", op);
    testAttribute(clientMetrics, clientProps, update.get(), "Invocation", "context.entry2", "", op);
    testAttribute(clientMetrics, clientProps, update.get(), "Invocation", "context.entry3", "", op);

    //
    // Tests with oneway
    //
    props["IceMX.Metrics.View.Map.Invocation.GroupBy"] = "operation";
    props["IceMX.Metrics.View.Map.Invocation.Map.Remote.GroupBy"] = "localPort";
    updateProps(clientProps, serverProps, update.get(), props, "Invocation");
    MetricsPrx metricsOneway = metrics->ice_oneway();
    metricsOneway->op();
    metricsOneway->opAsync().get();
    promise<void> sent;
    metricsOneway->opAsync([]() {}, [](exception_ptr) { test(false); }, [&](bool) { sent.set_value(); });
    sent.get_future().get();
    map = toMap(clientMetrics->getMetricsView("View", timestamp)["Invocation"]);
    test(map.size() == 1);

    im1 = dynamic_pointer_cast<IceMX::InvocationMetrics>(map["op"]);
    test(im1->current <= 1 && im1->total == 3 && im1->failures == 0 && im1->retry == 0);
    test(collocated ? (im1->collocated.size() == 1) : (im1->remotes.size() == 1));
    rim1 = dynamic_pointer_cast<IceMX::ChildInvocationMetrics>(collocated ? im1->collocated[0] : im1->remotes[0]);
    test(rim1->current <= 1 && rim1->total == 3 && rim1->failures == 0);
    test(rim1->size == 63 && rim1->replySize == 0);

    testAttribute(clientMetrics, clientProps, update.get(), "Invocation", "mode", "oneway", InvokeOp(metricsOneway));

    //
    // Tests with batch oneway
    //
    props["IceMX.Metrics.View.Map.Invocation.GroupBy"] = "operation";
    props["IceMX.Metrics.View.Map.Invocation.Map.Remote.GroupBy"] = "localPort";
    updateProps(clientProps, serverProps, update.get(), props, "Invocation");

    MetricsPrx metricsBatchOneway = metrics->ice_batchOneway();
    metricsBatchOneway->op();
    metricsBatchOneway->opAsync().get();
    metricsBatchOneway->opAsync([]() {}, [](exception_ptr) { test(false); });

    map = toMap(clientMetrics->getMetricsView("View", timestamp)["Invocation"]);
    test(map.size() == 1);

    im1 = dynamic_pointer_cast<IceMX::InvocationMetrics>(map["op"]);
    test(im1->current == 0 && im1->total == 3 && im1->failures == 0 && im1->retry == 0);
    test(im1->remotes.size() == 0);

    testAttribute(
        clientMetrics,
        clientProps,
        update.get(),
        "Invocation",
        "mode",
        "batch-oneway",
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
    metricsBatchOneway->ice_flushBatchRequestsAsync().get();
    metricsBatchOneway->ice_flushBatchRequestsAsync([](exception_ptr) { test(false); });

    map = toMap(clientMetrics->getMetricsView("View", timestamp)["Invocation"]);
    test(map.size() == 2);

    im1 = dynamic_pointer_cast<IceMX::InvocationMetrics>(map["ice_flushBatchRequests"]);
    test(im1->current <= 1 && im1->total == 3 && im1->failures == 0 && im1->retry == 0);
    if (!collocated)
    {
        test(im1->remotes.size() == 1); // The first operation got sent over a connection
    }

    if (!collocated)
    {
        clearView(clientProps, serverProps, update.get());

        Ice::ConnectionPtr con = metricsBatchOneway->ice_getConnection();

        metricsBatchOneway = metricsBatchOneway->ice_fixed(con);
        metricsBatchOneway->op();

        con->flushBatchRequests(Ice::CompressBatch::No);
        con->flushBatchRequestsAsync(Ice::CompressBatch::No).get();
        con->flushBatchRequestsAsync(Ice::CompressBatch::No, [](exception_ptr) { test(false); });
        map = toMap(clientMetrics->getMetricsView("View", timestamp)["Invocation"]);
        test(map.size() == 3);

        im1 = dynamic_pointer_cast<IceMX::InvocationMetrics>(map["flushBatchRequests"]);
        test(im1->current == 0 && im1->total == 3 && im1->failures == 0 && im1->retry == 0);
        test(im1->remotes.size() == 1); // The first operation got sent over a connection

        clearView(clientProps, serverProps, update.get());
        metricsBatchOneway->op();

        communicator->flushBatchRequests(Ice::CompressBatch::No);
        communicator->flushBatchRequestsAsync(Ice::CompressBatch::No).get();
        communicator->flushBatchRequestsAsync(Ice::CompressBatch::No, [](exception_ptr) { test(false); });
        map = toMap(clientMetrics->getMetricsView("View", timestamp)["Invocation"]);
        test(map.size() == 2);

        im1 = dynamic_pointer_cast<IceMX::InvocationMetrics>(map["flushBatchRequests"]);
        test(im1->current <= 1 && im1->total == 3 && im1->failures == 0 && im1->retry == 0);
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
    catch (const IceMX::UnknownMetricsView&)
    {
    }

    cout << "ok" << endl;

    cout << "testing instrumentation observer delegate... " << flush;

    test(obsv->threadObserver->getTotal() > 0);
    if (!collocated)
    {
        test(obsv->connectionObserver->getTotal() > 0);
        test(obsv->connectionEstablishmentObserver->getTotal() > 0);
#if TARGET_OS_IPHONE == 0
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
    if (!collocated)
    {
        test(obsv->connectionObserver->getCurrent() > 0);
        test(obsv->connectionEstablishmentObserver->getCurrent() == 0);
#if TARGET_OS_IPHONE == 0
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
    if (!collocated)
    {
        test(obsv->connectionObserver->getFailedCount() > 0);
        test(obsv->connectionEstablishmentObserver->getFailedCount() > 0);
#if TARGET_OS_IPHONE == 0
        test(obsv->endpointLookupObserver->getFailedCount() > 0);
#endif
    }
    // test(obsv->dispatchObserver->getFailedCount() > 0);

    test(obsv->invocationObserver->getFailedCount() > 0);
    if (!collocated)
    {
        test(obsv->invocationObserver->remoteObserver->getFailedCount() > 0);
        test(obsv->threadObserver->states > 0);
    }
    if (!collocated)
    {
        test(obsv->connectionObserver->received > 0 && obsv->connectionObserver->sent > 0);
    }

    // test(obsv->dispatchObserver->userExceptionCount > 0);
    test(obsv->invocationObserver->userExceptionCount > 0);
    if (!collocated)
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
