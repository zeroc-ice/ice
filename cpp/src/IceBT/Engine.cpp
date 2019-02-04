//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceBT/Engine.h>
#include <IceBT/DBus.h>
#include <IceBT/Util.h>
#include <Ice/LocalException.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/Thread.h>
#include <IceUtil/UUID.h>

using namespace std;
using namespace Ice;
using namespace IceBT;

IceUtil::Shared* IceBT::upCast(IceBT::Engine* p) { return p; }

namespace IceBT
{

class ConnectionI;
typedef IceUtil::Handle<ConnectionI> ConnectionIPtr;

//
// ConnectionI implements IceBT::Connection and encapsulates a DBus connection along with
// some additional state.
//
class ConnectionI : public Connection
{
public:

    ConnectionI(const DBus::ConnectionPtr& conn, const string& devicePath, const string& uuid) :
        _connection(conn),
        _devicePath(devicePath),
        _uuid(uuid)
    {
    }

    DBus::ConnectionPtr dbusConnection() const
    {
        return _connection;
    }

    //
    // Blocking close.
    //
    virtual void close()
    {
        try
        {
            //
            // Invoke DisconnectProfile to terminate the client-side connection.
            //
            DBus::MessagePtr msg =
                DBus::Message::createCall("org.bluez", _devicePath, "org.bluez.Device1", "DisconnectProfile");
            msg->write(new DBus::StringValue(_uuid));
            DBus::AsyncResultPtr r = _connection->callAsync(msg);
            r->waitUntilFinished(); // Block until the call completes.
        }
        catch(const DBus::Exception&)
        {
            // Ignore.
        }

        try
        {
            _connection->close();
        }
        catch(const DBus::Exception&)
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

    virtual void handleMethodCall(const DBus::ConnectionPtr& conn, const DBus::MessagePtr& m)
    {
        string member = m->getMember();
        if(member == "Release")
        {
            //
            // Ignore - no reply necessary.
            //
        }
        else if(member == "NewConnection")
        {
            vector<DBus::ValuePtr> values = m->readAll();
            assert(values.size() == 3);

            //
            // This argument is the Unix file descriptor for the new connection.
            //
            DBus::UnixFDValuePtr fd = DBus::UnixFDValuePtr::dynamicCast(values[1]);
            assert(fd);

            try
            {
                //
                // Send an empty reply.
                //
                DBus::MessagePtr ret = DBus::Message::createReturn(m);
                conn->sendAsync(ret);
            }
            catch(const DBus::Exception&)
            {
                // Ignore.
            }

            try
            {
                newConnection(fd->v);
            }
            catch(...)
            {
                // Ignore.
            }
        }
        else if(member == "RequestDisconnection")
        {
            try
            {
                //
                // Send an empty reply.
                //
                DBus::MessagePtr ret = DBus::Message::createReturn(m);
                conn->sendAsync(ret);
            }
            catch(const DBus::Exception&)
            {
                // Ignore.
            }

            //
            // Ignore disconnect requests.
            //
        }
    }

protected:

    Profile() {}

    virtual void newConnection(int) = 0;
};
ICE_DEFINE_PTR(ProfilePtr, Profile);

//
// ClientProfile represents an outgoing connection profile.
//
class ClientProfile : public Profile
{
public:

    ClientProfile(const ConnectionPtr& conn, const ConnectCallbackPtr& cb) :
        _connection(conn),
        _callback(cb)
    {
    }

    ~ClientProfile()
    {
    }

protected:

    virtual void newConnection(int fd)
    {
        //
        // The callback assumes ownership of the file descriptor and connection.
        //
        _callback->completed(fd, _connection);
        _connection = 0; // Remove circular reference.
        _callback = 0;
    }

private:

    ConnectionPtr _connection;
    ConnectCallbackPtr _callback;
};
ICE_DEFINE_PTR(ClientProfilePtr, ClientProfile);

//
// ServerProfile represents an incoming connection profile.
//
class ServerProfile : public Profile
{
public:

    ServerProfile(const ProfileCallbackPtr& cb) :
        _callback(cb)
    {
    }

protected:

    virtual void newConnection(int fd)
    {
        _callback->newConnection(fd);
    }

private:

    ProfileCallbackPtr _callback;
};
ICE_DEFINE_PTR(ServerProfilePtr, ServerProfile);

//
// Engine delegates to BluetoothService. It encapsulates a snapshot of the "objects" managed by the
// DBus Bluetooth daemon. These objects include local Bluetooth adapters, paired devices, etc.
//
class BluetoothService : public DBus::Filter
#ifdef ICE_CPP11_MAPPING
                       , public std::enable_shared_from_this<BluetoothService>
#endif
{
public:

    typedef map<string, DBus::VariantValuePtr> VariantMap;
    typedef map<string, VariantMap> InterfacePropertiesMap;

    struct RemoteDevice
    {
        RemoteDevice()
        {
        }

        RemoteDevice(const VariantMap& m) :
            properties(m)
        {
        }

        string getAddress() const
        {
            string addr;
            VariantMap::const_iterator i = properties.find("Address");
            if(i != properties.end())
            {
                DBus::StringValuePtr str = DBus::StringValuePtr::dynamicCast(i->second->v);
                assert(str);
                addr = str->v;
            }
            return IceUtilInternal::toUpper(addr);
        }

        string getAdapter() const
        {
            string adapter;
            VariantMap::const_iterator i = properties.find("Adapter");
            if(i != properties.end())
            {
                DBus::ObjectPathValuePtr path = DBus::ObjectPathValuePtr::dynamicCast(i->second->v);
                assert(path);
                adapter = path->v;
            }
            return adapter;
        }

        VariantMap properties;
    };

    struct Adapter
    {
        Adapter()
        {
        }

        Adapter(const VariantMap& p) :
            properties(p)
        {
        }

        string getAddress() const
        {
            string addr;
            VariantMap::const_iterator i = properties.find("Address");
            if(i != properties.end())
            {
                DBus::StringValuePtr str = DBus::StringValuePtr::dynamicCast(i->second->v);
                assert(str);
                addr = str->v;
            }
            return IceUtilInternal::toUpper(addr);
        }

        VariantMap properties;
#ifdef ICE_CPP11_MAPPING
        vector<function<void(const string&, const PropertyMap&)>> callbacks;
#else
        vector<DiscoveryCallbackPtr> callbacks;
#endif
    };

    typedef map<string, RemoteDevice> RemoteDeviceMap; // Key is the object path.
    typedef map<string, Adapter> AdapterMap; // Key is the object path.

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
            _dbusConnection->addFilter(ICE_SHARED_FROM_THIS);
            getManagedObjects();
        }
        catch(const DBus::Exception& ex)
        {
            throw BluetoothException(__FILE__, __LINE__, ex.reason);
        }
    }

    //
    // From DBus::Filter.
    //
    virtual bool handleMessage(const DBus::ConnectionPtr&, const DBus::MessagePtr& msg)
    {
        if(!msg->isSignal())
        {
            return false; // Not handled.
        }

        string intf = msg->getInterface();
        string member = msg->getMember();

        if(intf == "org.freedesktop.DBus.ObjectManager" && member == "InterfacesAdded")
        {
            //
            // The InterfacesAdded signal contains two values:
            //
            //   OBJPATH obj_path
            //   DICT<STRING,DICT<STRING,VARIANT>> interfaces_and_properties
            //

            vector<DBus::ValuePtr> values = msg->readAll();
            assert(values.size() == 2);
            DBus::ObjectPathValuePtr path = DBus::ObjectPathValuePtr::dynamicCast(values[0]);
            assert(path);

            InterfacePropertiesMap interfaceProps;
            extractInterfaceProperties(values[1], interfaceProps);

            InterfacePropertiesMap::iterator p = interfaceProps.find("org.bluez.Device1");
            if(p != interfaceProps.end())
            {
                //
                // A remote device was added.
                //
                deviceAdded(path->v, p->second);
            }

            p = interfaceProps.find("org.bluez.Adapter1");
            if(p != interfaceProps.end())
            {
                //
                // A local Bluetooth adapter was added.
                //
                adapterAdded(path->v, p->second);
            }

            return true;
        }
        else if(intf == "org.freedesktop.DBus.ObjectManager" && member == "InterfacesRemoved")
        {
            //
            // The InterfacesRemoved signal contains two values:
            //
            //   OBJPATH obj_path
            //   ARRAY<STRING> interfaces
            //

            vector<DBus::ValuePtr> values = msg->readAll();
            assert(values.size() == 2);
            DBus::ObjectPathValuePtr path = DBus::ObjectPathValuePtr::dynamicCast(values[0]);
            assert(path);
            DBus::ArrayValuePtr ifaces = DBus::ArrayValuePtr::dynamicCast(values[1]);
            assert(ifaces);

            for(vector<DBus::ValuePtr>::const_iterator q = ifaces->elements.begin(); q != ifaces->elements.end(); ++q)
            {
                assert((*q)->getType()->getKind() == DBus::Type::KindString);
                DBus::StringValuePtr ifaceName = DBus::StringValuePtr::dynamicCast(*q);

                //
                // A remote device was removed.
                //
                if(ifaceName->v == "org.bluez.Device1")
                {
                    deviceRemoved(path->v);
                }
                else if(ifaceName->v == "org.bluez.Adapter1")
                {
                    adapterRemoved(path->v);
                }
            }

            return true;
        }
        else if(intf == "org.freedesktop.DBus.Properties" && member == "PropertiesChanged")
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
            DBus::StringValuePtr iface = DBus::StringValuePtr::dynamicCast(values[0]);
            assert(iface);

            if(iface->v != "org.bluez.Device1" && iface->v != "org.bluez.Adapter1")
            {
                return false;
            }

            VariantMap changed;
            extractProperties(values[1], changed);

            DBus::ArrayValuePtr a = DBus::ArrayValuePtr::dynamicCast(values[2]);
            assert(a);
            vector<string> removedNames;
            for(vector<DBus::ValuePtr>::const_iterator p = a->elements.begin(); p != a->elements.end(); ++p)
            {
                DBus::StringValuePtr sv = DBus::StringValuePtr::dynamicCast(*p);
                assert(sv);
                removedNames.push_back(sv->v);
            }

            if(iface->v == "org.bluez.Device1")
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
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

        //
        // Return the device address of the default local adapter.
        //
        // TBD: Be smarter about this? E.g., consider the state of the Powered property?
        //
        if(!_adapters.empty())
        {
            return _adapters.begin()->second.getAddress();
        }

        throw BluetoothException(__FILE__, __LINE__, "no Bluetooth adapter found");
    }

    bool adapterExists(const string& addr) const
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

        //
        // Check if a local adapter exists with the given device address.
        //
        for(AdapterMap::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
        {
            if(addr == p->second.getAddress())
            {
                return true;
            }
        }

        return false;
    }

    bool deviceExists(const string& addr) const
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

        //
        // Check if a remote device exists with the given device address.
        //
        for(RemoteDeviceMap::const_iterator p = _remoteDevices.begin(); p != _remoteDevices.end(); ++p)
        {
            if(p->second.getAddress() == IceUtilInternal::toUpper(addr))
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
        ProfilePtr profile = ICE_MAKE_SHARED(ServerProfile, cb);

        string path = generatePath();

        try
        {
            DBus::AsyncResultPtr ar = registerProfileImpl(_dbusConnection, path, uuid, name, channel, profile);
            DBus::MessagePtr reply = ar->waitUntilFinished(); // Block until finished.
            if(reply->isError())
            {
                reply->throwException();
            }
        }
        catch(const DBus::Exception& ex)
        {
            throw BluetoothException(__FILE__, __LINE__, ex.reason);
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
            if(reply->isError())
            {
                reply->throwException();
            }
        }
        catch(const DBus::Exception& ex)
        {
            throw BluetoothException(__FILE__, __LINE__, ex.reason);
        }
    }

    void connect(const string& addr, const string& uuid, const ConnectCallbackPtr& cb)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

        //
        // Start a thread to establish the connection.
        //
        IceUtil::ThreadPtr t = new ConnectThread(ICE_SHARED_FROM_THIS, addr, uuid, cb);
        _connectThreads.push_back(t);
        t->start();
    }

#ifdef ICE_CPP11_MAPPING
    void startDiscovery(const string& addr, function<void(const string&, const PropertyMap&)> cb)
#else
    void startDiscovery(const string& addr, const DiscoveryCallbackPtr& cb)
#endif
    {
        string path;

        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

            for(AdapterMap::iterator p = _adapters.begin(); p != _adapters.end(); ++p)
            {
                if(p->second.getAddress() == IceUtilInternal::toUpper(addr))
                {
                    path = p->first;
#ifdef ICE_CPP11_MAPPING
                    p->second.callbacks.push_back(move(cb));
#else
                    p->second.callbacks.push_back(cb);
#endif
                }
            }
        }

        if(path.empty())
        {
            throw BluetoothException(__FILE__, __LINE__, "no Bluetooth adapter found matching address " + addr);
        }

        //
        // Invoke StartDiscovery() on the adapter object.
        //
        try
        {
            DBus::MessagePtr msg = DBus::Message::createCall("org.bluez", path, "org.bluez.Adapter1", "StartDiscovery");
            DBus::AsyncResultPtr r = _dbusConnection->callAsync(msg);
            DBus::MessagePtr reply = r->waitUntilFinished();
            if(reply->isError())
            {
                reply->throwException();
            }
        }
        catch(const DBus::Exception& ex)
        {
            throw BluetoothException(__FILE__, __LINE__, ex.reason);
        }
    }

    void stopDiscovery(const string& addr)
    {
        string path;

        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

            for(AdapterMap::iterator p = _adapters.begin(); p != _adapters.end(); ++p)
            {
                if(p->second.getAddress() == IceUtilInternal::toUpper(addr))
                {
                    path = p->first;
                    p->second.callbacks.clear();
                }
            }
        }

        if(path.empty())
        {
            throw BluetoothException(__FILE__, __LINE__, "no Bluetooth adapter found matching address " + addr);
        }

        //
        // Invoke StopDiscovery() on the adapter object.
        //
        try
        {
            DBus::MessagePtr msg = DBus::Message::createCall("org.bluez", path, "org.bluez.Adapter1", "StopDiscovery");
            DBus::AsyncResultPtr r = _dbusConnection->callAsync(msg);
            DBus::MessagePtr reply = r->waitUntilFinished();
            if(reply->isError())
            {
                reply->throwException();
            }
        }
        catch(const DBus::Exception& ex)
        {
            throw BluetoothException(__FILE__, __LINE__, ex.reason);
        }
    }

    DeviceMap getDevices() const
    {
        DeviceMap devices;

        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

            for(RemoteDeviceMap::const_iterator p = _remoteDevices.begin(); p != _remoteDevices.end(); ++p)
            {
                PropertyMap pm; // Convert to string-string map.
                for(VariantMap::const_iterator q = p->second.properties.begin(); q != p->second.properties.end(); ++q)
                {
                    pm[q->first] = q->second->toString();
                }
                devices[p->second.getAddress()] = pm;
            }
        }

        return devices;
    }

    void destroy()
    {
        //
        // Wait for any active connect threads to finish.
        //
        vector<IceUtil::ThreadPtr> v;

        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);
            v.swap(_connectThreads);
        }

        for(vector<IceUtil::ThreadPtr>::iterator p = v.begin(); p != v.end(); ++p)
        {
            (*p)->getThreadControl().join();
        }

        if(_dbusConnection)
        {
            try
            {
                _dbusConnection->close();
            }
            catch(const DBus::Exception& ex)
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
            DBus::MessagePtr msg =
                DBus::Message::createCall("org.bluez", "/", "org.freedesktop.DBus.ObjectManager", "GetManagedObjects");
            DBus::AsyncResultPtr r = _dbusConnection->callAsync(msg);
            DBus::MessagePtr reply = r->waitUntilFinished();
            if(reply->isError())
            {
                reply->throwException();
            }

            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

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
            DBus::ArrayValuePtr a = DBus::ArrayValuePtr::dynamicCast(v);
            for(vector<DBus::ValuePtr>::const_iterator p = a->elements.begin(); p != a->elements.end(); ++p)
            {
                assert((*p)->getType()->getKind() == DBus::Type::KindDictEntry);
                DBus::DictEntryValuePtr e = DBus::DictEntryValuePtr::dynamicCast(*p);
                assert(e->key->getType()->getKind() == DBus::Type::KindObjectPath);
                DBus::ObjectPathValuePtr path = DBus::ObjectPathValuePtr::dynamicCast(e->key);

                assert(e->value->getType()->getKind() == DBus::Type::KindArray);
                InterfacePropertiesMap ipmap;
                extractInterfaceProperties(e->value, ipmap);

                InterfacePropertiesMap::iterator q;

                q = ipmap.find("org.bluez.Adapter1");
                if(q != ipmap.end())
                {
                    //
                    // org.bluez.Adapter1 is the interface for local Bluetooth adapters.
                    //
                    _adapters[path->v] = Adapter(q->second);
                }

                q = ipmap.find("org.bluez.Device1");
                if(q != ipmap.end())
                {
                    //
                    // org.bluez.Device1 is the interface for paired remote devices.
                    //
                    RemoteDevice d(q->second);
                    if(!d.getAddress().empty())
                    {
                        _remoteDevices[path->v] = d;
                    }
                }
            }
        }
        catch(const DBus::Exception& ex)
        {
            throw BluetoothException(__FILE__, __LINE__, ex.reason);
        }
    }

    DBus::AsyncResultPtr registerProfileImpl(const DBus::ConnectionPtr& conn, const string& path, const string& uuid,
                                             const string& name, int channel, const ProfilePtr& profile)
    {
        conn->addService(path, profile);

        //
        // Invoke RegisterProfile on the profile manager object.
        //
        DBus::MessagePtr msg =
            DBus::Message::createCall("org.bluez", "/org/bluez", "org.bluez.ProfileManager1", "RegisterProfile");
        vector<DBus::ValuePtr> args;
        args.push_back(new DBus::ObjectPathValue(path));
        args.push_back(new DBus::StringValue(uuid));
        DBus::DictEntryTypePtr dt =
            new DBus::DictEntryType(DBus::Type::getPrimitive(DBus::Type::KindString), new DBus::VariantType);
        DBus::TypePtr t = new DBus::ArrayType(dt);
        DBus::ArrayValuePtr options = new DBus::ArrayValue(t);
        if(!name.empty())
        {
            options->elements.push_back(
                new DBus::DictEntryValue(dt, new DBus::StringValue("Name"),
                                         new DBus::VariantValue(new DBus::StringValue(name))));
        }
        if(channel != -1)
        {
            options->elements.push_back(
                new DBus::DictEntryValue(dt, new DBus::StringValue("Channel"),
                                         new DBus::VariantValue(new DBus::Uint16Value(channel))));
            options->elements.push_back(
                new DBus::DictEntryValue(dt, new DBus::StringValue("Role"),
                                         new DBus::VariantValue(new DBus::StringValue("server"))));
        }
        else
        {
            options->elements.push_back(
                new DBus::DictEntryValue(dt, new DBus::StringValue("Role"),
                                         new DBus::VariantValue(new DBus::StringValue("client"))));
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
        msg->write(new DBus::ObjectPathValue(path));
        return conn->callAsync(msg);
    }

    static string generatePath()
    {
        //
        // Generate a unique object path. Path elements can only contain "[A-Z][a-z][0-9]_".
        //
        string path = "/com/zeroc/P" + IceUtil::generateUUID();
        for(string::iterator p = path.begin(); p != path.end(); ++p)
        {
            if(*p == '-')
            {
                *p = '_';
            }
        }
        return path;
    }

    void deviceAdded(const string& path, const VariantMap& props)
    {
        RemoteDevice dev(props);
        if(dev.getAddress().empty())
        {
            return; // Ignore devices that don't have an Address property.
        }

#ifdef ICE_CPP11_MAPPING
        vector<function<void(const string&, const PropertyMap&)>> callbacks;
#else
        vector<DiscoveryCallbackPtr> callbacks;
#endif

        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

            AdapterMap::iterator p = _adapters.find(dev.getAdapter());
            if(p != _adapters.end())
            {
                callbacks = p->second.callbacks;
            }
            _remoteDevices[path] = dev;
        }

        if(!callbacks.empty())
        {
            PropertyMap pm; // Convert to string-string map.
            for(VariantMap::const_iterator p = props.begin(); p != props.end(); ++p)
            {
                pm[p->first] = p->second->toString();
            }
#ifdef ICE_CPP11_MAPPING
            for(const auto& discovered : callbacks)
            {
                try
                {
                    discovered(dev.getAddress(), pm);
                }
                catch(...)
                {
                }
            }
#else
            for(vector<DiscoveryCallbackPtr>::iterator p = callbacks.begin(); p != callbacks.end(); ++p)
            {
                try
                {
                    (*p)->discovered(dev.getAddress(), pm);
                }
                catch(...)
                {
                }
            }
#endif
        }
    }

    void deviceChanged(const string& path, const VariantMap& changed, const vector<string>& removedProps)
    {
#ifdef ICE_CPP11_MAPPING
        vector<function<void(const string&, const PropertyMap&)>> callbacks;
#else
        vector<DiscoveryCallbackPtr> callbacks;
#endif
        string addr;
        string adapter;
        VariantMap props;

        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

            RemoteDeviceMap::iterator p = _remoteDevices.find(path);
            if(p == _remoteDevices.end())
            {
                RemoteDevice dev(changed);
                addr = dev.getAddress();
                if(!addr.empty())
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

                if(addr.empty())
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

            AdapterMap::iterator q = _adapters.find(adapter);
            if(q != _adapters.end())
            {
                callbacks = q->second.callbacks;
            }
        }

        if(!addr.empty() && !callbacks.empty())
        {
            PropertyMap pm; // Convert to string-string map.
            for(VariantMap::iterator p = props.begin(); p != props.end(); ++p)
            {
                pm[p->first] = p->second->toString();
            }
#ifdef ICE_CPP11_MAPPING
            for(const auto& discovered : callbacks)
            {
                try
                {
                    discovered(addr, pm);
                }
                catch(...)
                {
                }
            }
#else
            for(vector<DiscoveryCallbackPtr>::iterator p = callbacks.begin(); p != callbacks.end(); ++p)
            {
                try
                {
                    (*p)->discovered(addr, pm);
                }
                catch(...)
                {
                }
            }
#endif
        }
    }

    void deviceRemoved(const string& path)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

        RemoteDeviceMap::iterator p = _remoteDevices.find(path);
        if(p != _remoteDevices.end())
        {
            _remoteDevices.erase(p);
        }
    }

    void adapterAdded(const string& path, const VariantMap& props)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

        _adapters[path] = Adapter(props);
    }

    void adapterChanged(const string& path, const VariantMap& changed, const vector<string>& removedProps)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

        AdapterMap::iterator p = _adapters.find(path);
        if(p == _adapters.end())
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
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

        AdapterMap::iterator p = _adapters.find(path);
        if(p != _adapters.end())
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

        DBus::ArrayValuePtr ifaces = DBus::ArrayValuePtr::dynamicCast(v);
        assert(ifaces);

        for(vector<DBus::ValuePtr>::const_iterator q = ifaces->elements.begin(); q != ifaces->elements.end(); ++q)
        {
            assert((*q)->getType()->getKind() == DBus::Type::KindDictEntry);
            DBus::DictEntryValuePtr ie = DBus::DictEntryValuePtr::dynamicCast(*q);
            assert(ie->key->getType()->getKind() == DBus::Type::KindString);
            DBus::StringValuePtr ifaceName = DBus::StringValuePtr::dynamicCast(ie->key);

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
        DBus::ArrayValuePtr props = DBus::ArrayValuePtr::dynamicCast(v);
        for(vector<DBus::ValuePtr>::const_iterator s = props->elements.begin(); s != props->elements.end(); ++s)
        {
            assert((*s)->getType()->getKind() == DBus::Type::KindDictEntry);
            DBus::DictEntryValuePtr pe = DBus::DictEntryValuePtr::dynamicCast(*s);
            assert(pe->key->getType()->getKind() == DBus::Type::KindString);
            DBus::StringValuePtr propName = DBus::StringValuePtr::dynamicCast(pe->key);
            assert(pe->value->getType()->getKind() == DBus::Type::KindVariant);
            vm[propName->v] = DBus::VariantValuePtr::dynamicCast(pe->value);
        }
    }

    void updateProperties(VariantMap& props, const VariantMap& changed, const vector<string>& removedProps)
    {
        //
        // Remove properties.
        //
        for(vector<string>::const_iterator q = removedProps.begin(); q != removedProps.end(); ++q)
        {
            VariantMap::iterator r = props.find(*q);
            if(r != props.end())
            {
                props.erase(r);
            }
        }

        //
        // Merge changes.
        //
        for(VariantMap::const_iterator q = changed.begin(); q != changed.end(); ++q)
        {
            props[q->first] = q->second;
        }
    }

    void runConnectThread(const IceUtil::ThreadPtr& thread, const string& addr, const string& uuid,
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
                IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

                for(RemoteDeviceMap::iterator p = _remoteDevices.begin(); p != _remoteDevices.end(); ++p)
                {
                    if(p->second.getAddress() == IceUtilInternal::toUpper(addr))
                    {
                        devicePath = p->first;
                        break;
                    }
                }
            }

            //
            // If we don't find a match, we're done.
            //
            if(devicePath.empty())
            {
                throw BluetoothException(__FILE__, __LINE__, "unknown address `" + addr + "'");
            }

            //
            // We have a matching device, now register a client profile.
            //
            DBus::ConnectionPtr dbusConn = DBus::Connection::getSystemBus();
            conn = new ConnectionI(dbusConn, devicePath, uuid);

            ProfilePtr profile = ICE_MAKE_SHARED(ClientProfile, conn, cb);
            string path = generatePath();

            //
            // Register a client profile. Client profiles are not advertised in SDP.
            //
            DBus::AsyncResultPtr r = registerProfileImpl(dbusConn, path, uuid, string(), -1, profile);
            DBus::MessagePtr reply = r->waitUntilFinished();
            if(reply->isError())
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
            msg->write(new DBus::StringValue(uuid));
            r = dbusConn->callAsync(msg);
            reply = r->waitUntilFinished();
            if(reply->isError())
            {
                try
                {
                    reply->throwException();
                }
                catch(const DBus::Exception& ex)
                {
                    ostringstream ostr;
                    ostr << "unable to establish connection to " << uuid << " at " << addr;
                    if(!ex.reason.empty())
                    {
                        ostr << ':' << endl << ex.reason;
                    }
                    throw BluetoothException(__FILE__, __LINE__, ostr.str());
                }
            }
        }
        catch(const DBus::Exception& ex)
        {
            ok = false;
            cb->failed(BluetoothException(__FILE__, __LINE__, ex.reason));
        }
        catch(const Ice::LocalException& ex)
        {
            ok = false;
            cb->failed(ex);
        }

        //
        // Clean up.
        //

        if(!ok && conn)
        {
            conn->close();
        }

        //
        // Remove the thread from the list.
        //
        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

            vector<IceUtil::ThreadPtr>::iterator p = find(_connectThreads.begin(), _connectThreads.end(), thread);
            if(p != _connectThreads.end()) // May be missing if destroy() was called.
            {
                _connectThreads.erase(p);
            }
        }
    }

    class ConnectThread : public IceUtil::Thread
    {
    public:

        ConnectThread(const BluetoothServicePtr& mo, const string& addr, const string& uuid,
                      const ConnectCallbackPtr& cb) :
            _mo(mo),
            _addr(addr),
            _uuid(uuid),
            _cb(cb)
        {
        }

        virtual void run()
        {
            _mo->runConnectThread(this, _addr, _uuid, _cb);
        }

    private:

        BluetoothServicePtr _mo;
        string _addr;
        string _uuid;
        ConnectCallbackPtr _cb;
    };

    IceUtil::Monitor<IceUtil::Mutex> _lock;
    DBus::ConnectionPtr _dbusConnection;

    AdapterMap _adapters;
    RemoteDeviceMap _remoteDevices;
    string _defaultAdapterAddress;
    vector<IceUtil::ThreadPtr> _connectThreads;

    bool _discovering;
#ifdef ICE_CPP11_MAPPING
    vector<function<void(const string&, const PropertyMap&)>> _discoveryCallbacks;
#else
    vector<DiscoveryCallbackPtr> _discoveryCallbacks;
#endif
};

}

#ifndef ICE_CPP11_MAPPING
IceUtil::Shared* IceBT::upCast(IceBT::BluetoothService* p) { return p; }
#endif

IceBT::Engine::Engine(const Ice::CommunicatorPtr& communicator) :
    _communicator(communicator),
    _initialized(false)
{
}

Ice::CommunicatorPtr
IceBT::Engine::communicator() const
{
    return _communicator;
}

void
IceBT::Engine::initialize()
{
    _service = ICE_MAKE_SHARED(BluetoothService);
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
#ifdef ICE_CPP11_MAPPING
IceBT::Engine::startDiscovery(const string& address, function<void(const string&, const PropertyMap&)> cb)
#else
IceBT::Engine::startDiscovery(const string& address, const DiscoveryCallbackPtr& cb)
#endif
{
    _service->startDiscovery(address, cb);
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
