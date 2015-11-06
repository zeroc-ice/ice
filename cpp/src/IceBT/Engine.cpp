// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceBT/Engine.h>
#include <IceBT/DBus.h>
#include <IceBT/Util.h>
#include <Ice/LocalException.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/Thread.h>
#include <IceUtil/UUID.h>

#include <stack>

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
typedef IceUtil::Handle<Profile> ProfilePtr;

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
typedef IceUtil::Handle<ClientProfile> ClientProfilePtr;

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
typedef IceUtil::Handle<ServerProfile> ServerProfilePtr;

//
// Engine delegates to ManagedObjects. It encapsulates a snapshot of the "objects" managed by the
// DBus Bluetooth daemon. These objects include local Bluetooth adapters, paired devices, etc.
//
class ManagedObjects : public IceUtil::Shared
{
public:

    ManagedObjects()
    {
        DBus::initThreads();

        try
        {
            //
            // Block while we establish a DBus connection and retrieve a snapshot of the managed objects
            // from the Bluetooth service.
            //
            _dbusConnection = DBus::Connection::getSystemBus();
            getManagedObjects();
        }
        catch(const DBus::Exception& ex)
        {
            throw BluetoothException(__FILE__, __LINE__, ex.reason);
        }
    }

    string getDefaultDeviceAddress() const
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

        //
        // Return the device address of the default local adapter.
        //

        if(_defaultAdapterAddress.empty())
        {
            throw BluetoothException(__FILE__, __LINE__, "no Bluetooth adapter found");
        }

        return _defaultAdapterAddress;
    }

    bool deviceExists(const string& addr) const
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

        //
        // Check if a local adapter exists with the given device address.
        //
        for(ObjectMap::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
        {
            PropertyMap::const_iterator q = p->second.find("Address");
            if(q != p->second.end())
            {
                DBus::StringValuePtr str = DBus::StringValuePtr::dynamicCast(q->second->v);
                assert(str);
                if(addr == str->v)
                {
                    return true;
                }
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
        ProfilePtr profile = new ServerProfile(cb);

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
        IceUtil::ThreadPtr t = new ConnectThread(this, addr, uuid, cb);
        _connectThreads.push_back(t);
        t->start();
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
                DBus::ArrayValuePtr ifaces = DBus::ArrayValuePtr::dynamicCast(e->value);
                for(vector<DBus::ValuePtr>::const_iterator q = ifaces->elements.begin(); q != ifaces->elements.end();
                    ++q)
                {
                    assert((*q)->getType()->getKind() == DBus::Type::KindDictEntry);
                    DBus::DictEntryValuePtr ie = DBus::DictEntryValuePtr::dynamicCast(*q);
                    assert(ie->key->getType()->getKind() == DBus::Type::KindString);
                    DBus::StringValuePtr ifaceName = DBus::StringValuePtr::dynamicCast(ie->key);

                    PropertyMap propertyMap;
                    assert(ie->value->getType()->getKind() == DBus::Type::KindArray);
                    DBus::ArrayValuePtr props = DBus::ArrayValuePtr::dynamicCast(ie->value);
                    for(vector<DBus::ValuePtr>::const_iterator s = props->elements.begin(); s != props->elements.end();
                        ++s)
                    {
                        assert((*s)->getType()->getKind() == DBus::Type::KindDictEntry);
                        DBus::DictEntryValuePtr pe = DBus::DictEntryValuePtr::dynamicCast(*s);
                        assert(pe->key->getType()->getKind() == DBus::Type::KindString);
                        DBus::StringValuePtr propName = DBus::StringValuePtr::dynamicCast(pe->key);
                        assert(pe->value->getType()->getKind() == DBus::Type::KindVariant);
                        propertyMap[propName->v] = DBus::VariantValuePtr::dynamicCast(pe->value);
                    }

                    if(ifaceName->v == "org.bluez.Adapter1")
                    {
                        //
                        // org.bluez.Adapter1 is the interface for local Bluetooth adapters.
                        //

                        _adapters[path->v] = propertyMap;

                        //
                        // Save the address of the first adapter we encounter as our "default" adapter.
                        //
                        if(_defaultAdapterAddress.empty())
                        {
                            PropertyMap::iterator t = propertyMap.find("Address");
                            if(t != propertyMap.end())
                            {
                                DBus::StringValuePtr str = DBus::StringValuePtr::dynamicCast(t->second->v);
                                assert(str);
                                if(!str->v.empty())
                                {
                                    _defaultAdapterAddress = str->v;
                                }
                            }
                        }
                    }
                    else if(ifaceName->v == "org.bluez.Device1")
                    {
                        //
                        // org.bluez.Device1 is the interface for paired devices.
                        //

                        PropertyMap::iterator t = propertyMap.find("Address");
                        if(t != propertyMap.end())
                        {
                            DBus::StringValuePtr str = DBus::StringValuePtr::dynamicCast(t->second->v);
                            assert(str);
                            if(!str->v.empty())
                            {
                                _remoteDevices[IceUtilInternal::toUpper(str->v)] = path->v;
                            }
                        }
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

    void runConnectThread(const IceUtil::ThreadPtr& thread, const string& addr, const string& uuid,
                          const ConnectCallbackPtr& cb)
    {
        //
        // Establishing a connection is a complicated process.
        //
        // 1) Determine whether our local Bluetooth service knows about the target
        //    remote device denoted by the 'addr' argument. The known remote devices
        //    are included in the managed objects returned by the GetManagedObjects
        //    invocation on the Bluetooth service.
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
            //
            // Block while we refresh the list of known devices.
            //
            getManagedObjects();

            string devicePath;

            //
            // Search our list of known devices for one that matches the given address.
            //
            {
                IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

                DeviceMap::iterator p = _remoteDevices.find(IceUtilInternal::toUpper(addr));
                if(p != _remoteDevices.end())
                {
                    devicePath = p->second;
                }
            }

            //
            // If we don't find a match, we're done.
            //
            if(devicePath.empty())
            {
                //
                // ConnectorI handles this situation specially.
                //
                throw Ice::NoEndpointException(__FILE__, __LINE__, addr);
            }

            //
            // We have a matching device, now register a client profile.
            //
            DBus::ConnectionPtr dbusConn = DBus::Connection::getSystemBus();
            conn = new ConnectionI(dbusConn, devicePath, uuid);

            ProfilePtr profile = new ClientProfile(conn, cb);
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
            assert(p != _connectThreads.end());
            _connectThreads.erase(p);
        }
    }

    class ConnectThread : public IceUtil::Thread
    {
    public:

        ConnectThread(const ManagedObjectsPtr& mo, const string& addr, const string& uuid,
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

        ManagedObjectsPtr _mo;
        string _addr;
        string _uuid;
        ConnectCallbackPtr _cb;
    };

    IceUtil::Monitor<IceUtil::Mutex> _lock;
    DBus::ConnectionPtr _dbusConnection;

    typedef map<string, DBus::VariantValuePtr> PropertyMap;
    typedef map<string, PropertyMap> ObjectMap;
    typedef map<string, string> DeviceMap; // Maps device address to object path.

    ObjectMap _adapters;
    DeviceMap _remoteDevices;
    string _defaultAdapterAddress;
    vector<IceUtil::ThreadPtr> _connectThreads;
};

}

IceUtil::Shared* IceBT::upCast(IceBT::ManagedObjects* p) { return p; }

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
    _managedObjects = new ManagedObjects;
    _initialized = true;
}

bool
IceBT::Engine::initialized() const
{
    return _initialized;
}

string
IceBT::Engine::getDefaultDeviceAddress() const
{
    return _managedObjects->getDefaultDeviceAddress();
}

bool
IceBT::Engine::deviceExists(const string& addr) const
{
    return _managedObjects->deviceExists(addr);
}

string
IceBT::Engine::registerProfile(const string& uuid, const string& name, int channel, const ProfileCallbackPtr& cb)
{
    return _managedObjects->registerProfile(uuid, name, channel, cb);
}

void
IceBT::Engine::unregisterProfile(const string& path)
{
    return _managedObjects->unregisterProfile(path);
}

void
IceBT::Engine::connect(const string& addr, const string& uuid, const ConnectCallbackPtr& cb)
{
    _managedObjects->connect(addr, uuid, cb);
}

void
IceBT::Engine::destroy()
{
    _managedObjects->destroy();
}
