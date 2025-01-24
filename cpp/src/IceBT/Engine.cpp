// Copyright (c) ZeroC, Inc.

#include "Engine.h"
#include "DBus.h"
#include "Ice/LocalExceptions.h"
#include "Ice/StringUtil.h"
#include "Ice/UUID.h"
#include "Util.h"

#include <algorithm>
#include <thread>

using namespace std;
using namespace Ice;
using namespace IceBT;

namespace IceBT
{
    class ConnectionI;
    using ConnectionIPtr = std::shared_ptr<ConnectionI>;

    //
    // ConnectionI implements IceBT::Connection and encapsulates a DBus connection along with
    // some additional state.
    //
    class ConnectionI final : public Connection
    {
    public:
        ConnectionI(DBus::ConnectionPtr conn, string devicePath, string uuid)
            : _connection(std::move(conn)),
              _devicePath(std::move(devicePath)),
              _uuid(std::move(uuid))
        {
        }

        [[nodiscard]] DBus::ConnectionPtr dbusConnection() const { return _connection; }

        //
        // Blocking close.
        //
        void close() final
        {
            try
            {
                //
                // Invoke DisconnectProfile to terminate the client-side connection.
                //
                DBus::MessagePtr msg =
                    DBus::Message::createCall("org.bluez", _devicePath, "org.bluez.Device1", "DisconnectProfile");
                msg->write(make_shared<DBus::StringValue>(_uuid));
                DBus::AsyncResultPtr r = _connection->callAsync(msg);
                r->waitUntilFinished(); // Block until the call completes.
            }
            catch (const DBus::Exception&)
            {
                // Ignore.
            }

            try
            {
                _connection->close();
            }
            catch (const DBus::Exception&)
            {
                // Ignore.
            }
        }

    private:
        DBus::ConnectionPtr _connection;
        string _devicePath;
        string _uuid;
    };

    //
    // Profile is an abstract base class representing a Bluetooth "profile". We have to register a DBus
    // profile object for a UUID in order to receive connection notifications. This is necessary for both
    // outgoing and incoming connections.
    //
    class Profile : public DBus::Service
    {
    public:
        void handleMethodCall(const DBus::ConnectionPtr& conn, const DBus::MessagePtr& m) final
        {
            string member = m->getMember();
            if (member == "Release")
            {
                //
                // Ignore - no reply necessary.
                //
            }
            else if (member == "NewConnection")
            {
                vector<DBus::ValuePtr> values = m->readAll();
                assert(values.size() == 3);

                //
                // This argument is the Unix file descriptor for the new connection.
                //
                auto fd = dynamic_pointer_cast<DBus::UnixFDValue>(values[1]);
                assert(fd);

                try
                {
                    //
                    // Send an empty reply.
                    //
                    DBus::MessagePtr ret = DBus::Message::createReturn(m);
                    conn->sendAsync(ret);
                }
                catch (const DBus::Exception&)
                {
                    // Ignore.
                }

                try
                {
                    newConnection(fd->v);
                }
                catch (...)
                {
                    // Ignore.
                }
            }
            else if (member == "RequestDisconnection")
            {
                try
                {
                    //
                    // Send an empty reply.
                    //
                    DBus::MessagePtr ret = DBus::Message::createReturn(m);
                    conn->sendAsync(ret);
                }
                catch (const DBus::Exception&)
                {
                    // Ignore.
                }

                //
                // Ignore disconnect requests.
                //
            }
        }

    protected:
        virtual void newConnection(int) = 0;
    };
    using ProfilePtr = std::shared_ptr<Profile>;

    //
    // ClientProfile represents an outgoing connection profile.
    //
    class ClientProfile final : public Profile
    {
    public:
        ClientProfile(ConnectionPtr conn, ConnectCallbackPtr cb)
            : _connection(std::move(conn)),
              _callback(std::move(cb))
        {
        }

        ~ClientProfile() = default;

    protected:
        void newConnection(int fd) final
        {
            //
            // The callback assumes ownership of the file descriptor and connection.
            //
            _callback->completed(fd, _connection);
            _connection = nullptr; // Remove circular reference.
            _callback = nullptr;
        }

    private:
        ConnectionPtr _connection;
        ConnectCallbackPtr _callback;
    };
    using ClientProfilePtr = std::shared_ptr<ClientProfile>;

    //
    // ServerProfile represents an incoming connection profile.
    //
    class ServerProfile final : public Profile
    {
    public:
        ServerProfile(ProfileCallbackPtr cb) : _callback(std::move(cb)) {}

    protected:
        void newConnection(int fd) final { _callback->newConnection(fd); }

    private:
        ProfileCallbackPtr _callback;
    };
    using ServerProfilePtr = std::shared_ptr<ServerProfile>;

    //
    // Engine delegates to BluetoothService. It encapsulates a snapshot of the "objects" managed by the
    // DBus Bluetooth daemon. These objects include local Bluetooth adapters, paired devices, etc.
    //
    class BluetoothService : public DBus::Filter, public std::enable_shared_from_this<BluetoothService>
    {
    public:
        using VariantMap = map<string, DBus::VariantValuePtr>;
        using InterfacePropertiesMap = map<string, VariantMap>;

        struct RemoteDevice
        {
            RemoteDevice() = default;

            RemoteDevice(VariantMap m) : properties(std::move(m)) {}

            [[nodiscard]] string getAddress() const
            {
                string addr;
                auto i = properties.find("Address");
                if (i != properties.end())
                {
                    auto str = dynamic_pointer_cast<DBus::StringValue>(i->second->v);
                    assert(str);
                    addr = str->v;
                }
                return IceInternal::toUpper(addr);
            }

            [[nodiscard]] string getAdapter() const
            {
                string adapter;
                auto i = properties.find("Adapter");
                if (i != properties.end())
                {
                    auto path = dynamic_pointer_cast<DBus::ObjectPathValue>(i->second->v);
                    assert(path);
                    adapter = path->v;
                }
                return adapter;
            }

            VariantMap properties;
        };

        struct Adapter
        {
            Adapter() = default;

            Adapter(VariantMap p) : properties(std::move(p)) {}

            [[nodiscard]] string getAddress() const
            {
                string addr;
                auto i = properties.find("Address");
                if (i != properties.end())
                {
                    auto str = dynamic_pointer_cast<DBus::StringValue>(i->second->v);
                    assert(str);
                    addr = str->v;
                }
                return IceInternal::toUpper(addr);
            }

            VariantMap properties;
            vector<function<void(const string&, const PropertyMap&)>> callbacks;
        };

        using RemoteDeviceMap = map<string, RemoteDevice>; // Key is the object path.
        using AdapterMap = map<string, Adapter>;           // Key is the object path.

        void init()
        {
            DBus::initThreads();

            try
            {
                //
                // Block while we establish a DBus connection and retrieve a snapshot of the managed objects
                // from the Bluetooth service.
                //
                _dbusConnection = DBus::Connection::getSystemBus();
                _dbusConnection->addFilter(shared_from_this());
                getManagedObjects();
            }
            catch (const DBus::Exception& ex)
            {
                throw BluetoothException{__FILE__, __LINE__, ex.reason};
            }
        }

        //
        // From DBus::Filter.
        //
        bool handleMessage(const DBus::ConnectionPtr&, const DBus::MessagePtr& msg) override
        {
            if (!msg->isSignal())
            {
                return false; // Not handled.
            }

            string intf = msg->getInterface();
            string member = msg->getMember();

            if (intf == "org.freedesktop.DBus.ObjectManager" && member == "InterfacesAdded")
            {
                //
                // The InterfacesAdded signal contains two values:
                //
                //   OBJPATH obj_path
                //   DICT<STRING,DICT<STRING,VARIANT>> interfaces_and_properties
                //

                vector<DBus::ValuePtr> values = msg->readAll();
                assert(values.size() == 2);
                auto path = dynamic_pointer_cast<DBus::ObjectPathValue>(values[0]);
                assert(path);

                InterfacePropertiesMap interfaceProps;
                extractInterfaceProperties(values[1], interfaceProps);

                auto p = interfaceProps.find("org.bluez.Device1");
                if (p != interfaceProps.end())
                {
                    //
                    // A remote device was added.
                    //
                    deviceAdded(path->v, p->second);
                }

                p = interfaceProps.find("org.bluez.Adapter1");
                if (p != interfaceProps.end())
                {
                    //
                    // A local Bluetooth adapter was added.
                    //
                    adapterAdded(path->v, p->second);
                }

                return true;
            }
            else if (intf == "org.freedesktop.DBus.ObjectManager" && member == "InterfacesRemoved")
            {
                //
                // The InterfacesRemoved signal contains two values:
                //
                //   OBJPATH obj_path
                //   ARRAY<STRING> interfaces
                //

                vector<DBus::ValuePtr> values = msg->readAll();
                assert(values.size() == 2);
                auto path = dynamic_pointer_cast<DBus::ObjectPathValue>(values[0]);
                assert(path);
                auto ifaces = dynamic_pointer_cast<DBus::ArrayValue>(values[1]);
                assert(ifaces);

                for (const auto& element : ifaces->elements)
                {
                    assert(element->getType()->getKind() == DBus::Type::KindString);
                    auto ifaceName = dynamic_pointer_cast<DBus::StringValue>(element);

                    //
                    // A remote device was removed.
                    //
                    if (ifaceName->v == "org.bluez.Device1")
                    {
                        deviceRemoved(path->v);
                    }
                    else if (ifaceName->v == "org.bluez.Adapter1")
                    {
                        adapterRemoved(path->v);
                    }
                }

                return true;
            }
            else if (intf == "org.freedesktop.DBus.Properties" && member == "PropertiesChanged")
            {
                //
                // The PropertiesChanged signal contains three values:
                //
                //   STRING interface_name
                //   DICT<STRING,VARIANT> changed_properties
                //   ARRAY<STRING> invalidated_properties
                //

                vector<DBus::ValuePtr> values = msg->readAll();
                assert(values.size() == 3);
                auto iface = dynamic_pointer_cast<DBus::StringValue>(values[0]);
                assert(iface);

                if (iface->v != "org.bluez.Device1" && iface->v != "org.bluez.Adapter1")
                {
                    return false;
                }

                VariantMap changed;
                extractProperties(values[1], changed);

                auto a = dynamic_pointer_cast<DBus::ArrayValue>(values[2]);
                assert(a);
                vector<string> removedNames;
                for (const auto& element : a->elements)
                {
                    auto sv = dynamic_pointer_cast<DBus::StringValue>(element);
                    assert(sv);
                    removedNames.push_back(sv->v);
                }

                if (iface->v == "org.bluez.Device1")
                {
                    deviceChanged(msg->getPath(), changed, removedNames);
                }
                else
                {
                    adapterChanged(msg->getPath(), changed, removedNames);
                }

                return true;
            }

            return false;
        }

        string getDefaultAdapterAddress() const
        {
            lock_guard lock(_mutex);

            //
            // Return the device address of the default local adapter.
            //
            // TBD: Be smarter about this? E.g., consider the state of the Powered property?
            //
            if (!_adapters.empty())
            {
                return _adapters.begin()->second.getAddress();
            }

            throw BluetoothException{__FILE__, __LINE__, "no Bluetooth adapter found"};
        }

        bool adapterExists(const string& addr) const
        {
            lock_guard lock(_mutex);

            //
            // Check if a local adapter exists with the given device address.
            //
            for (const auto& adapter : _adapters)
            {
                if (addr == adapter.second.getAddress())
                {
                    return true;
                }
            }

            return false;
        }

        bool deviceExists(const string& addr) const
        {
            lock_guard lock(_mutex);

            //
            // Check if a remote device exists with the given device address.
            //
            for (const auto& remoteDevice : _remoteDevices)
            {
                if (remoteDevice.second.getAddress() == IceInternal::toUpper(addr))
                {
                    return true;
                }
            }

            return false;
        }

        //
        // Calling registerProfile will advertise a service (SDP) profile with the Bluetooth daemon.
        //
        string registerProfile(const string& uuid, const string& name, int channel, const ProfileCallbackPtr& cb)
        {
            //
            // As a subclass of DBus::Service, the ServerProfile object will receive DBus method
            // invocations for a given object path.
            //
            ProfilePtr profile = make_shared<ServerProfile>(cb);

            string path = generatePath();

            try
            {
                DBus::AsyncResultPtr ar = registerProfileImpl(_dbusConnection, path, uuid, name, channel, profile);
                DBus::MessagePtr reply = ar->waitUntilFinished(); // Block until finished.
                if (reply->isError())
                {
                    reply->throwException();
                }
            }
            catch (const DBus::Exception& ex)
            {
                throw BluetoothException{__FILE__, __LINE__, ex.reason};
            }

            return path;
        }

        void unregisterProfile(const string& path)
        {
            try
            {
                //
                // Block while we unregister the profile.
                //
                DBus::AsyncResultPtr ar = unregisterProfileImpl(_dbusConnection, path);
                ar->waitUntilFinished();
                DBus::MessagePtr reply = ar->getReply();
                _dbusConnection->removeService(path);
                if (reply->isError())
                {
                    reply->throwException();
                }
            }
            catch (const DBus::Exception& ex)
            {
                throw BluetoothException{__FILE__, __LINE__, ex.reason};
            }
        }

        void connect(const string& addr, const string& uuid, const ConnectCallbackPtr& cb)
        {
            lock_guard lock(_mutex);

            //
            // Start a thread to establish the connection.
            //
            _connectThreads.emplace_back([self = shared_from_this(), addr, uuid, cb]
                                         { self->runConnectThread(this_thread::get_id(), addr, uuid, cb); });
        }

        void startDiscovery(const string& addr, function<void(const string&, const PropertyMap&)> cb)
        {
            string path;

            {
                lock_guard lock(_mutex);

                for (auto& adapter : _adapters)
                {
                    if (adapter.second.getAddress() == IceInternal::toUpper(addr))
                    {
                        path = adapter.first;
                        adapter.second.callbacks.push_back(std::move(cb));
                    }
                }
            }

            if (path.empty())
            {
                throw BluetoothException{__FILE__, __LINE__, "no Bluetooth adapter found matching address " + addr};
            }

            //
            // Invoke StartDiscovery() on the adapter object.
            //
            try
            {
                DBus::MessagePtr msg =
                    DBus::Message::createCall("org.bluez", path, "org.bluez.Adapter1", "StartDiscovery");
                DBus::AsyncResultPtr r = _dbusConnection->callAsync(msg);
                DBus::MessagePtr reply = r->waitUntilFinished();
                if (reply->isError())
                {
                    reply->throwException();
                }
            }
            catch (const DBus::Exception& ex)
            {
                throw BluetoothException{__FILE__, __LINE__, ex.reason};
            }
        }

        void stopDiscovery(const string& addr)
        {
            string path;

            {
                lock_guard lock(_mutex);

                for (auto& adapter : _adapters)
                {
                    if (adapter.second.getAddress() == IceInternal::toUpper(addr))
                    {
                        path = adapter.first;
                        adapter.second.callbacks.clear();
                    }
                }
            }

            if (path.empty())
            {
                throw BluetoothException{__FILE__, __LINE__, "no Bluetooth adapter found matching address " + addr};
            }

            //
            // Invoke StopDiscovery() on the adapter object.
            //
            try
            {
                DBus::MessagePtr msg =
                    DBus::Message::createCall("org.bluez", path, "org.bluez.Adapter1", "StopDiscovery");
                DBus::AsyncResultPtr r = _dbusConnection->callAsync(msg);
                DBus::MessagePtr reply = r->waitUntilFinished();
                if (reply->isError())
                {
                    reply->throwException();
                }
            }
            catch (const DBus::Exception& ex)
            {
                throw BluetoothException{__FILE__, __LINE__, ex.reason};
            }
        }

        DeviceMap getDevices() const
        {
            DeviceMap devices;

            {
                lock_guard lock(_mutex);

                for (const auto& remoteDevice : _remoteDevices)
                {
                    PropertyMap pm; // Convert to string-string map.
                    for (const auto& property : remoteDevice.second.properties)
                    {
                        pm[property.first] = property.second->toString();
                    }
                    devices[remoteDevice.second.getAddress()] = pm;
                }
            }

            return devices;
        }

        void destroy()
        {
            //
            // Wait for any active connect threads to finish.
            //
            vector<thread> connectThreads;

            {
                lock_guard lock(_mutex);
                connectThreads.swap(_connectThreads);
            }

            for (auto& t : connectThreads)
            {
                t.join();
            }

            if (_dbusConnection)
            {
                try
                {
                    _dbusConnection->close();
                }
                catch (const DBus::Exception& ex)
                {
                }
            }
        }

        void getManagedObjects()
        {
            try
            {
                //
                // Query the Bluetooth service for its managed objects. This is a standard DBus invocation
                // with the following signature:
                //
                // org.freedesktop.DBus.ObjectManager.GetManagedObjects (
                //     out DICT<OBJPATH,DICT<STRING,DICT<STRING,VARIANT>>> objpath_interfaces_and_properties);
                //
                DBus::MessagePtr msg = DBus::Message::createCall(
                    "org.bluez",
                    "/",
                    "org.freedesktop.DBus.ObjectManager",
                    "GetManagedObjects");
                DBus::AsyncResultPtr r = _dbusConnection->callAsync(msg);
                DBus::MessagePtr reply = r->waitUntilFinished();
                if (reply->isError())
                {
                    reply->throwException();
                }

                lock_guard lock(_mutex);

                _adapters.clear();
                _remoteDevices.clear();
                _defaultAdapterAddress.clear();

                //
                // The return value of GetManagedObjects is a dictionary structured like this:
                //
                //     Key: Object path (e.g., "/org/bluez")
                //     Value: Dictionary of interfaces
                //         Key: Interface name (e.g., "org.bluez.Adapter1")
                //         Value: Dictionary of properties
                //             Key: Property name
                //             Value: Property value (variant)
                //

                //
                // Extract the dictionary from the reply message.
                //
                DBus::ValuePtr v = reply->read();

                //
                // Iterate through the dictionary and collect the objects that we need.
                //
                assert(v->getType()->getKind() == DBus::Type::KindArray);
                auto a = dynamic_pointer_cast<DBus::ArrayValue>(v);
                for (const auto& element : a->elements)
                {
                    assert(element->getType()->getKind() == DBus::Type::KindDictEntry);
                    auto e = dynamic_pointer_cast<DBus::DictEntryValue>(element);
                    assert(e->key->getType()->getKind() == DBus::Type::KindObjectPath);
                    auto path = dynamic_pointer_cast<DBus::ObjectPathValue>(e->key);

                    assert(e->value->getType()->getKind() == DBus::Type::KindArray);
                    InterfacePropertiesMap ipmap;
                    extractInterfaceProperties(e->value, ipmap);

                    InterfacePropertiesMap::iterator q;

                    q = ipmap.find("org.bluez.Adapter1");
                    if (q != ipmap.end())
                    {
                        //
                        // org.bluez.Adapter1 is the interface for local Bluetooth adapters.
                        //
                        _adapters[path->v] = Adapter(q->second);
                    }

                    q = ipmap.find("org.bluez.Device1");
                    if (q != ipmap.end())
                    {
                        //
                        // org.bluez.Device1 is the interface for paired remote devices.
                        //
                        RemoteDevice d(q->second);
                        if (!d.getAddress().empty())
                        {
                            _remoteDevices[path->v] = d;
                        }
                    }
                }
            }
            catch (const DBus::Exception& ex)
            {
                throw BluetoothException{__FILE__, __LINE__, ex.reason};
            }
        }

        DBus::AsyncResultPtr registerProfileImpl(
            const DBus::ConnectionPtr& conn,
            const string& path,
            const string& uuid,
            const string& name,
            int channel,
            const ProfilePtr& profile)
        {
            conn->addService(path, profile);

            //
            // Invoke RegisterProfile on the profile manager object.
            //
            DBus::MessagePtr msg =
                DBus::Message::createCall("org.bluez", "/org/bluez", "org.bluez.ProfileManager1", "RegisterProfile");
            vector<DBus::ValuePtr> args;
            args.push_back(make_shared<DBus::ObjectPathValue>(path));
            args.push_back(make_shared<DBus::StringValue>(uuid));
            auto dt = make_shared<DBus::DictEntryType>(
                DBus::Type::getPrimitive(DBus::Type::KindString),
                make_shared<DBus::VariantType>());
            auto t = make_shared<DBus::ArrayType>(dt);
            auto options = make_shared<DBus::ArrayValue>(t);
            if (!name.empty())
            {
                options->elements.push_back(make_shared<DBus::DictEntryValue>(
                    dt,
                    make_shared<DBus::StringValue>("Name"),
                    make_shared<DBus::VariantValue>(make_shared<DBus::StringValue>(name))));
            }
            if (channel != -1)
            {
                options->elements.push_back(make_shared<DBus::DictEntryValue>(
                    dt,
                    make_shared<DBus::StringValue>("Channel"),
                    make_shared<DBus::VariantValue>(make_shared<DBus::Uint16Value>(channel))));
                options->elements.push_back(make_shared<DBus::DictEntryValue>(
                    dt,
                    make_shared<DBus::StringValue>("Role"),
                    make_shared<DBus::VariantValue>(make_shared<DBus::StringValue>("server"))));
            }
            else
            {
                options->elements.push_back(make_shared<DBus::DictEntryValue>(
                    dt,
                    make_shared<DBus::StringValue>("Role"),
                    make_shared<DBus::VariantValue>(make_shared<DBus::StringValue>("client"))));
            }
            args.push_back(options);
            msg->write(args);
            return conn->callAsync(msg);
        }

        DBus::AsyncResultPtr unregisterProfileImpl(const DBus::ConnectionPtr& conn, const string& path)
        {
            //
            // Invoke UnregisterProfile on the profile manager object.
            //
            DBus::MessagePtr msg =
                DBus::Message::createCall("org.bluez", "/org/bluez", "org.bluez.ProfileManager1", "UnregisterProfile");
            msg->write(make_shared<DBus::ObjectPathValue>(path));
            return conn->callAsync(msg);
        }

        static string generatePath()
        {
            //
            // Generate a unique object path. Path elements can only contain "[A-Z][a-z][0-9]_".
            //
            string path = "/com/zeroc/P" + Ice::generateUUID();
            for (char& p : path)
            {
                if (p == '-')
                {
                    p = '_';
                }
            }
            return path;
        }

        void deviceAdded(const string& path, const VariantMap& props)
        {
            RemoteDevice dev(props);
            if (dev.getAddress().empty())
            {
                return; // Ignore devices that don't have an Address property.
            }

            vector<function<void(const string&, const PropertyMap&)>> callbacks;
            {
                lock_guard lock(_mutex);

                auto p = _adapters.find(dev.getAdapter());
                if (p != _adapters.end())
                {
                    callbacks = p->second.callbacks;
                }
                _remoteDevices[path] = dev;
            }

            if (!callbacks.empty())
            {
                PropertyMap pm; // Convert to string-string map.
                for (const auto& prop : props)
                {
                    pm[prop.first] = prop.second->toString();
                }

                for (const auto& discovered : callbacks)
                {
                    try
                    {
                        discovered(dev.getAddress(), pm);
                    }
                    catch (...)
                    {
                    }
                }
            }
        }

        void deviceChanged(const string& path, const VariantMap& changed, const vector<string>& removedProps)
        {
            vector<function<void(const string&, const PropertyMap&)>> callbacks;
            string addr;
            string adapter;
            VariantMap props;

            {
                lock_guard lock(_mutex);

                auto p = _remoteDevices.find(path);
                if (p == _remoteDevices.end())
                {
                    RemoteDevice dev(changed);
                    addr = dev.getAddress();
                    if (!addr.empty())
                    {
                        _remoteDevices[path] = dev;
                        props = changed;
                        adapter = dev.getAdapter();
                    }
                }
                else
                {
                    updateProperties(p->second.properties, changed, removedProps);

                    addr = p->second.getAddress();

                    if (addr.empty())
                    {
                        //
                        // Remove the device if we don't know its address.
                        //
                        _remoteDevices.erase(p);
                    }
                    else
                    {
                        props = p->second.properties;
                        adapter = p->second.getAdapter();
                    }
                }

                auto q = _adapters.find(adapter);
                if (q != _adapters.end())
                {
                    callbacks = q->second.callbacks;
                }
            }

            if (!addr.empty() && !callbacks.empty())
            {
                PropertyMap pm; // Convert to string-string map.
                for (const auto& prop : props)
                {
                    pm[prop.first] = prop.second->toString();
                }

                for (const auto& discovered : callbacks)
                {
                    try
                    {
                        discovered(addr, pm);
                    }
                    catch (...)
                    {
                    }
                }
            }
        }

        void deviceRemoved(const string& path)
        {
            lock_guard lock(_mutex);

            auto p = _remoteDevices.find(path);
            if (p != _remoteDevices.end())
            {
                _remoteDevices.erase(p);
            }
        }

        void adapterAdded(const string& path, const VariantMap& props)
        {
            lock_guard lock(_mutex);

            _adapters[path] = Adapter(props);
        }

        void adapterChanged(const string& path, const VariantMap& changed, const vector<string>& removedProps)
        {
            lock_guard lock(_mutex);

            auto p = _adapters.find(path);
            if (p == _adapters.end())
            {
                _adapters[path] = Adapter(changed);
            }
            else
            {
                updateProperties(p->second.properties, changed, removedProps);
            }
        }

        void adapterRemoved(const string& path)
        {
            lock_guard lock(_mutex);

            auto p = _adapters.find(path);
            if (p != _adapters.end())
            {
                _adapters.erase(p);
            }
        }

        void extractInterfaceProperties(const DBus::ValuePtr& v, InterfacePropertiesMap& interfaceProps)
        {
            //
            // The given value is a dictionary structured like this:
            //
            //   Key: Interface name (e.g., "org.bluez.Adapter1")
            //   Value: Dictionary of properties
            //     Key: Property name
            //     Value: Property value (variant)
            //

            auto ifaces = dynamic_pointer_cast<DBus::ArrayValue>(v);
            assert(ifaces);

            for (const auto& element : ifaces->elements)
            {
                assert(element->getType()->getKind() == DBus::Type::KindDictEntry);
                auto ie = dynamic_pointer_cast<DBus::DictEntryValue>(element);
                assert(ie->key->getType()->getKind() == DBus::Type::KindString);
                auto ifaceName = dynamic_pointer_cast<DBus::StringValue>(ie->key);

                VariantMap pm;
                extractProperties(ie->value, pm);

                interfaceProps[ifaceName->v] = pm;
            }
        }

        void extractProperties(const DBus::ValuePtr& v, VariantMap& vm)
        {
            //
            // The given value is a dictionary structured like this:
            //
            //   Key: Property name
            //   Value: Property value (variant)
            //

            assert(v->getType()->getKind() == DBus::Type::KindArray);
            auto props = dynamic_pointer_cast<DBus::ArrayValue>(v);
            for (const auto& element : props->elements)
            {
                assert(element->getType()->getKind() == DBus::Type::KindDictEntry);
                auto pe = dynamic_pointer_cast<DBus::DictEntryValue>(element);
                assert(pe->key->getType()->getKind() == DBus::Type::KindString);
                auto propName = dynamic_pointer_cast<DBus::StringValue>(pe->key);
                assert(pe->value->getType()->getKind() == DBus::Type::KindVariant);
                vm[propName->v] = dynamic_pointer_cast<DBus::VariantValue>(pe->value);
            }
        }

        void updateProperties(VariantMap& props, const VariantMap& changed, const vector<string>& removedProps)
        {
            //
            // Remove properties.
            //
            for (const auto& removedProp : removedProps)
            {
                auto r = props.find(removedProp);
                if (r != props.end())
                {
                    props.erase(r);
                }
            }

            //
            // Merge changes.
            //
            for (const auto& q : changed)
            {
                props[q.first] = q.second;
            }
        }

        void runConnectThread(
            const thread::id& threadId,
            const string& addr,
            const string& uuid,
            const ConnectCallbackPtr& cb)
        {
            //
            // Establishing a connection is a complicated process.
            //
            // 1) Determine whether our local Bluetooth service knows about the target
            //    remote device denoted by the 'addr' argument. The known remote devices
            //    are included in the managed objects returned by the GetManagedObjects
            //    invocation on the Bluetooth service and updated dynamically during
            //    discovery.
            //
            // 2) After we find the remote device, we have to register a client profile
            //    for the given UUID.
            //
            // 3) After registering the profile, we have to invoke ConnectDevice on the
            //    local device object corresponding to the target address. The Bluetooth
            //    service will attempt to establish a connection to the remote device.
            //    If the connection succeeds, our profile object will receive a
            //    NewConnection invocation that supplies the file descriptor.
            //

            ConnectionIPtr conn;
            bool ok = true;

            try
            {
                string devicePath;

                //
                // Search our list of known devices for one that matches the given address.
                //
                {
                    lock_guard lock(_mutex);

                    for (const auto& remoteDevice : _remoteDevices)
                    {
                        if (remoteDevice.second.getAddress() == IceInternal::toUpper(addr))
                        {
                            devicePath = remoteDevice.first;
                            break;
                        }
                    }
                }

                //
                // If we don't find a match, we're done.
                //
                if (devicePath.empty())
                {
                    throw BluetoothException{__FILE__, __LINE__, "unknown address '" + addr + "'"};
                }

                //
                // We have a matching device, now register a client profile.
                //
                DBus::ConnectionPtr dbusConn = DBus::Connection::getSystemBus();
                conn = make_shared<ConnectionI>(dbusConn, devicePath, uuid);

                ProfilePtr profile = make_shared<ClientProfile>(conn, cb);
                string path = generatePath();

                //
                // Register a client profile. Client profiles are not advertised in SDP.
                //
                DBus::AsyncResultPtr r = registerProfileImpl(dbusConn, path, uuid, string(), -1, profile);
                DBus::MessagePtr reply = r->waitUntilFinished();
                if (reply->isError())
                {
                    reply->throwException();
                }

                //
                // Invoke ConnectProfile to initiate the client-side connection:
                //
                // void ConnectProfile(string uuid)
                //
                // We only care about errors from this invocation. If the connection succeeds, our
                // client profile will receive a separate NewConnection invocation.
                //
                DBus::MessagePtr msg =
                    DBus::Message::createCall("org.bluez", devicePath, "org.bluez.Device1", "ConnectProfile");
                msg->write(make_shared<DBus::StringValue>(uuid));
                r = dbusConn->callAsync(msg);
                reply = r->waitUntilFinished();
                if (reply->isError())
                {
                    try
                    {
                        reply->throwException();
                    }
                    catch (const DBus::Exception& ex)
                    {
                        ostringstream ostr;
                        ostr << "unable to establish Bluetooth connection to " << uuid << " at " << addr;
                        if (!ex.reason.empty())
                        {
                            ostr << ':' << endl << ex.reason;
                        }
                        throw BluetoothException{__FILE__, __LINE__, ostr.str()};
                    }
                }
            }
            catch (const DBus::Exception& ex)
            {
                ok = false;
                cb->failed(make_exception_ptr(BluetoothException{__FILE__, __LINE__, ex.reason}));
            }
            catch (const Ice::LocalException&)
            {
                ok = false;
                cb->failed(current_exception());
            }

            //
            // Clean up.
            //

            if (!ok && conn)
            {
                conn->close();
            }

            //
            // Remove the thread from the list.
            //
            {
                lock_guard lock(_mutex);

                auto p = find_if(
                    _connectThreads.begin(),
                    _connectThreads.end(),
                    [threadId](const thread& t) { return t.get_id() == threadId; });

                if (p != _connectThreads.end()) // May be missing if destroy() was called.
                {
                    _connectThreads.erase(p);
                }
            }
        }

        mutable std::mutex _mutex;
        DBus::ConnectionPtr _dbusConnection;

        AdapterMap _adapters;
        RemoteDeviceMap _remoteDevices;
        string _defaultAdapterAddress;
        vector<thread> _connectThreads;

        bool _discovering;
        vector<function<void(const string&, const PropertyMap&)>> _discoveryCallbacks;
    };
}

IceBT::Engine::Engine(Ice::CommunicatorPtr communicator) : _communicator(std::move(communicator)) {}

Ice::CommunicatorPtr
IceBT::Engine::communicator() const
{
    return _communicator;
}

void
IceBT::Engine::initialize()
{
    _service = make_shared<BluetoothService>();
    _service->init();
    _initialized = true;
}

bool
IceBT::Engine::initialized() const
{
    return _initialized;
}

string
IceBT::Engine::getDefaultAdapterAddress() const
{
    return _service->getDefaultAdapterAddress();
}

bool
IceBT::Engine::adapterExists(const string& addr) const
{
    return _service->adapterExists(addr);
}

bool
IceBT::Engine::deviceExists(const string& addr) const
{
    return _service->deviceExists(addr);
}

string
IceBT::Engine::registerProfile(const string& uuid, const string& name, int channel, const ProfileCallbackPtr& cb)
{
    return _service->registerProfile(uuid, name, channel, cb);
}

void
IceBT::Engine::unregisterProfile(const string& path)
{
    return _service->unregisterProfile(path);
}

void
IceBT::Engine::connect(const string& addr, const string& uuid, const ConnectCallbackPtr& cb)
{
    _service->connect(addr, uuid, cb);
}

void
IceBT::Engine::startDiscovery(const string& address, function<void(const string&, const PropertyMap&)> cb)
{
    _service->startDiscovery(address, std::move(cb));
}

void
IceBT::Engine::stopDiscovery(const string& address)
{
    _service->stopDiscovery(address);
}

IceBT::DeviceMap
IceBT::Engine::getDevices() const
{
    return _service->getDevices();
}

void
IceBT::Engine::destroy()
{
    _service->destroy();
}
