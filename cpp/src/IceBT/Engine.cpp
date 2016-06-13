// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
#include <IceXML/Parser.h>

#include <stack>

using namespace std;
using namespace Ice;
using namespace IceBT;

IceUtil::Shared* IceBT::upCast(IceBT::Engine* p) { return p; }

typedef map<string, DBus::VariantValuePtr> VariantMap;

namespace
{

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

}

namespace IceBT
{

//
// Engine delegates to BluetoothService.
//
class BluetoothService : public DBus::Filter
{
public:

    typedef map<string, VariantMap> InterfacePropertiesMap;

    BluetoothService() :
        _destroyed(false)
    {
        DBus::initThreads();

        try
        {
            //
            // Block while we establish a DBus connection.
            //
            _dbusConnection = DBus::Connection::getSystemBus();
        }
        catch(const DBus::Exception& ex)
        {
            throw BluetoothException(__FILE__, __LINE__, ex.reason);
        }

        _dbusConnection->addFilter(this);

        try
        {
            //
            // Use a call DefaultAdapter() to verify that the Bluetooth daemon is present and
            // uses the expected version (BlueZ 4). If the system is running BlueZ 5, this call
            // will return the error org.freedesktop.DBus.Error.UnknownMethod.
            //
            call("/", "org.bluez.Manager", "DefaultAdapter");
        }
        catch(const DBus::Exception& ex)
        {
            if(ex.reason.find("UnknownMethod") != string::npos)
            {
                throw BluetoothException(__FILE__, __LINE__, "Bluetooth daemon uses an unsupported version");
            }
            else
            {
                throw BluetoothException(__FILE__, __LINE__, ex.reason);
            }
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

        if(intf == "org.bluez.Adapter" && member == "DeviceFound")
        {
            //
            // The DeviceFound signal contains two values: 
            //
            //   STRING address
            //   DICT<STRING,VARIANT> properties
            //
            vector<DBus::ValuePtr> values = msg->readAll();
            assert(values.size() == 2);
            DBus::StringValuePtr addr = DBus::StringValuePtr::dynamicCast(values[0]);
            assert(addr);
            VariantMap props;
            extractProperties(values[1], props);

#ifdef ICE_CPP11_MAPPING
            vector<function<void(const string&, const PropertyMap&)>> callbacks;
#else
            vector<DiscoveryCallbackPtr> callbacks;
#endif

            {
                IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);
#ifdef ICE_CPP11_MAPPING
                auto p = _discoveryCallbacks.find(msg->getPath());
#else
                map<string, vector<DiscoveryCallbackPtr> >::iterator p = _discoveryCallbacks.find(msg->getPath());
#endif
                if(p != _discoveryCallbacks.end())
                {
                    callbacks = p->second;
                }
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
                        discovered(addr->v, pm);
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
                        (*p)->discovered(addr->v, pm);
                    }
                    catch(...)
                    {
                    }
                }
#endif
            }

            return true;
        }
        else if(intf == "org.bluez.Adapter" && member == "PropertyChanged")
        {
            //
            // The PropertyChanged signal contains two values: 
            //
            //   STRING name
            //   VARIANT value
            //
            vector<DBus::ValuePtr> values = msg->readAll();
            assert(values.size() == 2);
            DBus::StringValuePtr name = DBus::StringValuePtr::dynamicCast(values[0]);
            assert(name);
            if(name->v == "Discovering")
            {
                DBus::VariantValuePtr v = DBus::VariantValuePtr::dynamicCast(values[1]);
                assert(v);
                DBus::BooleanValuePtr b = DBus::BooleanValuePtr::dynamicCast(v->v);
                assert(b);
                if(!b->v)
                {
                    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

#ifdef ICE_CPP11_MAPPING
                    auto p = _discoveryCallbacks.find(msg->getPath());
#else
                    map<string, vector<DiscoveryCallbackPtr> >::iterator p = _discoveryCallbacks.find(msg->getPath());
#endif
                    if(p != _discoveryCallbacks.end())
                    {
                        _discoveryCallbacks.erase(p);
                    }
                }
            }
            return false;
        }

        return false;
    }

    string getDefaultAdapterAddress() const
    {
        string path = getDefaultAdapter();
        VariantMap props = getAdapterProperties(path);

        VariantMap::const_iterator i = props.find("Address");
        if(i != props.end())
        {
            DBus::StringValuePtr str = DBus::StringValuePtr::dynamicCast(i->second->v);
            assert(str);
            return IceUtilInternal::toUpper(str->v);
        }

        throw BluetoothException(__FILE__, __LINE__, "no default Bluetooth adapter found");
    }

    bool adapterExists(const string& addr) const
    {
        string path = findAdapter(addr);
        return !path.empty();
    }

    unsigned int addService(const string& addr, const string& name, const string& uuid, int channel)
    {
        string path = findAdapter(addr);

        if(path.empty())
        {
            throw BluetoothException(__FILE__, __LINE__, "no Bluetooth adapter found matching address " + addr);
        }

        //
        // Compose an XML record.
        //
        ostringstream ostr;
        ostr << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << endl
             << "<record>" << endl
             << "    <attribute id=\"0x0001\">" << endl // UUID
             << "        <sequence>" << endl
             << "            <uuid value=\"" << uuid << "\" />" << endl
             << "        </sequence>" << endl
             << "    </attribute>" << endl
             << "    <attribute id=\"0x0004\">" << endl
             << "        <sequence>" << endl
             << "            <sequence>" << endl
             << "                <uuid value=\"0x0100\" />" << endl // L2CAP
             << "            </sequence>" << endl
             << "            <sequence>" << endl
             << "                <uuid value=\"0x0003\" />" << endl // RFCOMM channel
             << "                <uint8 value=\"0x" << hex << channel << dec << "\" />" << endl
             << "            </sequence>" << endl
             << "        </sequence>" << endl
             << "    </attribute>" << endl
             << "    <attribute id=\"0x0005\">" << endl
             << "        <sequence>" << endl
             << "            <uuid value=\"0x1002\" />" << endl
             << "        </sequence>" << endl
             << "    </attribute>" << endl;
        if(!name.empty())
        {
             ostr << "    <attribute id=\"0x0100\">" << endl
                  << "        <text value=\"" << name << "\" />" << endl
                  << "    </attribute>" << endl;
        }
        ostr << "</record>";

        try
        {
            DBus::MessagePtr reply = call(path, "org.bluez.Service", "AddRecord", new DBus::StringValue(ostr.str()));
            DBus::ValuePtr v = reply->read();
            DBus::Uint32ValuePtr handle = DBus::Uint32ValuePtr::dynamicCast(v);
            return handle->v;
        }
        catch(const DBus::Exception& ex)
        {
            throw BluetoothException(__FILE__, __LINE__, ex.reason);
        }
    }

    void findService(const string& addr, const string& uuid, const FindServiceCallbackPtr& cb)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

        if(_destroyed)
        {
            cb->exception(CommunicatorDestroyedException(__FILE__, __LINE__));
        }
        else
        {
            IceUtil::ThreadPtr t = new FindServiceThread(this, addr, uuid, cb);
            _threads.push_back(t);
            t->start();
        }
    }

    void removeService(const string& addr, unsigned int handle)
    {
        string path = findAdapter(addr);

        if(path.empty())
        {
            throw BluetoothException(__FILE__, __LINE__, "no Bluetooth adapter found matching address " + addr);
        }

        try
        {
            call(path, "org.bluez.Service", "RemoveRecord", new DBus::Uint32Value(handle));
        }
        catch(const DBus::Exception& ex)
        {
            if(ex.reason.find("DoesNotExist") != string::npos)
            {
                throw BluetoothException(__FILE__, __LINE__, ex.reason);
            }
        }
    }

#ifdef ICE_CPP11_MAPPING
    void startDiscovery(const string& addr, function<void(const string&, const PropertyMap&)> cb)
#else
    void startDiscovery(const string& addr, const DiscoveryCallbackPtr& cb)
#endif
    {
        string path;
        if(addr.empty())
        {
            path = getDefaultAdapter();
        }
        else
        {
            path = findAdapter(addr);
        }

        if(path.empty())
        {
            throw BluetoothException(__FILE__, __LINE__, "no Bluetooth adapter found matching address " + addr);
        }

        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

#ifdef ICE_CPP11_MAPPING
            auto p = _discoveryCallbacks.find(path);
            if(p == _discoveryCallbacks.end())
            {
                _discoveryCallbacks[path] = vector<function<void(const string&, const PropertyMap&)>>();
            }
            _discoveryCallbacks[path].push_back(move(cb));
#else
            map<string, vector<DiscoveryCallbackPtr> >::iterator p = _discoveryCallbacks.find(path);
            if(p == _discoveryCallbacks.end())
            {
                _discoveryCallbacks[path] = vector<DiscoveryCallbackPtr>();
            }
            _discoveryCallbacks[path].push_back(cb);
#endif
        }

        try
        {
            call(path, "org.bluez.Adapter", "StartDiscovery");
        }
        catch(const DBus::Exception& ex)
        {
            throw BluetoothException(__FILE__, __LINE__, ex.reason);
        }
    }

    void stopDiscovery(const string& addr)
    {
        string path;
        if(addr.empty())
        {
            path = getDefaultAdapter();
        }
        else
        {
            path = findAdapter(addr);
        }

        if(path.empty())
        {
            throw BluetoothException(__FILE__, __LINE__, "no Bluetooth adapter found matching address " + addr);
        }

        try
        {
            call(path, "org.bluez.Adapter", "StopDiscovery");
        }
        catch(const DBus::Exception& ex)
        {
            throw BluetoothException(__FILE__, __LINE__, ex.reason);
        }
    }

    void destroy()
    {
        vector<IceUtil::ThreadPtr> threads;

        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

            _destroyed = true;
            threads.swap(_threads);
        }

        for(vector<IceUtil::ThreadPtr>::iterator p = threads.begin(); p != threads.end(); ++p)
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

    string getDefaultAdapter() const
    {
        try
        {
            //
            // The call to DefaultAdapter returns OBJ_PATH.
            //
            DBus::MessagePtr reply = call("/", "org.bluez.Manager", "DefaultAdapter");
            DBus::ValuePtr v = reply->read();
            DBus::ObjectPathValuePtr path = DBus::ObjectPathValuePtr::dynamicCast(v);
            assert(path);
            return path->v;
        }
        catch(const DBus::Exception& ex)
        {
            if(ex.reason.find("NoSuchAdapter") == string::npos)
            {
                throw BluetoothException(__FILE__, __LINE__, ex.reason);
            }
        }

        return string();
    }

    string findAdapter(const string& addr) const
    {
        try
        {
            //
            // The call to FindAdapter returns OBJ_PATH.
            //
            DBus::MessagePtr reply = call("/", "org.bluez.Manager", "FindAdapter", new DBus::StringValue(addr));
            DBus::ValuePtr v = reply->read();
            DBus::ObjectPathValuePtr path = DBus::ObjectPathValuePtr::dynamicCast(v);
            assert(path);
            return path->v;
        }
        catch(const DBus::Exception& ex)
        {
            if(ex.reason.find("NoSuchAdapter") == string::npos)
            {
                throw BluetoothException(__FILE__, __LINE__, ex.reason);
            }
        }

        return string();
    }

    VariantMap getAdapterProperties(const string& path) const
    {
        //
        // The call to GetProperties returns
        //
        //   DICT<STRING,VARIANT> properties
        //
        DBus::MessagePtr reply = call(path, "org.bluez.Adapter", "GetProperties");
        DBus::ValuePtr v = reply->read();
        VariantMap props;
        extractProperties(v, props);
        return props;
    }

    IceXML::NodePtr findChild(const IceXML::NodePtr& parent, const string& name) const
    {
        IceXML::NodeList l = parent->getChildren();
        for(IceXML::NodeList::iterator p = l.begin(); p != l.end(); ++p)
        {
            if((*p)->getName() == name)
            {
                return *p;
            }
        }

        return 0;
    }

    IceXML::NodePtr findAttribute(const IceXML::NodePtr& record, int id) const
    {
        IceXML::NodeList l = record->getChildren();
        for(IceXML::NodeList::iterator p = l.begin(); p != l.end(); ++p)
        {
            if((*p)->getName() == "attribute")
            {
                string sid = (*p)->getAttribute("id");
                int i;
                if(sscanf(sid.c_str(), "%x", &i) == 1 && i == id)
                {
                    return *p;
                }
            }
        }

        return 0;
    }

    IceXML::NodePtr findSequence(const IceXML::NodePtr& parent, const string& uuid) const
    {
        IceXML::NodeList l = parent->getChildren();
        for(IceXML::NodeList::iterator p = l.begin(); p != l.end(); ++p)
        {
            if((*p)->getName() == "sequence")
            {
                IceXML::NodePtr u = findChild(*p, "uuid");
                if(u)
                {
                    string val = u->getAttribute("value");
                    if(IceUtilInternal::toUpper(val) == IceUtilInternal::toUpper(uuid))
                    {
                        return *p;
                    }
                }

                //
                // Recursively search for nested <sequence> elements.
                //
                IceXML::NodePtr n = findSequence(*p, uuid);
                if(n)
                {
                    return n;
                }
            }
        }

        return 0;
    }

    IceXML::NodePtr findSequence(const IceXML::NodePtr& parent, int uuid) const
    {
        IceXML::NodeList l = parent->getChildren();
        for(IceXML::NodeList::iterator p = l.begin(); p != l.end(); ++p)
        {
            if((*p)->getName() == "sequence")
            {
                IceXML::NodePtr u = findChild(*p, "uuid");
                if(u)
                {
                    string val = u->getAttribute("value");
                    int i;
                    if(sscanf(val.c_str(), "%x", &i) == 1 && i == uuid)
                    {
                        return *p;
                    }
                }

                //
                // Recursively search for nested <sequence> elements.
                //
                IceXML::NodePtr n = findSequence(*p, uuid);
                if(n)
                {
                    return n;
                }
            }
        }

        return 0;
    }

    DBus::MessagePtr call(const string& path, const string& intf, const string& member,
                          const DBus::ValuePtr& arg = 0) const
    {
        DBus::MessagePtr msg = DBus::Message::createCall("org.bluez", path, intf, member);
        if(arg)
        {
            msg->write(arg);
        }
        DBus::AsyncResultPtr r = _dbusConnection->callAsync(msg);
        DBus::MessagePtr reply = r->waitUntilFinished();
        if(reply->isError())
        {
            reply->throwException();
        }
        return reply;
    }

    void runFindService(const IceUtil::ThreadPtr& thread, const string& addr, const string& uuid,
                        const FindServiceCallbackPtr& cb)
    {
        vector<int> channels;
        bool failed = false;

        try
        {
            const string a = IceUtilInternal::toUpper(addr);
            const string adapter = getDefaultAdapter();

            string devicePath;
            bool first = true;

            while(devicePath.empty())
            {
                try
                {
                    DBus::MessagePtr reply = call(adapter, "org.bluez.Adapter", "FindDevice", new DBus::StringValue(a));
                    DBus::ValuePtr v = reply->read();
                    DBus::ObjectPathValuePtr path = DBus::ObjectPathValuePtr::dynamicCast(v);
                    assert(path);
                    devicePath = path->v;
                }
                catch(const DBus::Exception& ex)
                {
                    if(ex.reason.find("DoesNotExist") != string::npos)
                    {
                        if(first)
                        {
                            first = false;
                            try
                            {
                                DBus::MessagePtr reply = call(adapter, "org.bluez.Adapter", "CreateDevice",
                                                              new DBus::StringValue(a));
                                DBus::ValuePtr v = reply->read();
                                DBus::ObjectPathValuePtr path = DBus::ObjectPathValuePtr::dynamicCast(v);
                                assert(path);
                                devicePath = path->v;
                            }
                            catch(const DBus::Exception&)
                            {
                                // Try FindDevice one more time.
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                    else
                    {
                        throw BluetoothException(__FILE__, __LINE__, ex.reason);
                    }
                }
            }

            if(devicePath.empty())
            {
                //
                // Couldn't get the object path for the device.
                //
                throw BluetoothException(__FILE__, __LINE__, "unable to create device for address " + addr);
            }

            DBus::ValuePtr services;

            for(int iter = 0; iter < 50; ++iter)
            {
                {
                    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

                    if(_destroyed)
                    {
                        throw CommunicatorDestroyedException(__FILE__, __LINE__);
                    }
                }

                try
                {
                    //
                    // Try to call DiscoverServices on the device to get the XML service records.
                    // The return value is a DICT<UINT32, STRING>.
                    //
                    DBus::MessagePtr reply = call(devicePath, "org.bluez.Device", "DiscoverServices",
                                                  new DBus::StringValue(""));
                    services = reply->read();
                    break;
                }
                catch(const DBus::Exception& ex)
                {
                    if(ex.reason.find("InProgress") == string::npos)
                    {
                        throw BluetoothException(__FILE__, __LINE__, ex.reason);
                    }

                    //
                    // Another call to DiscoverServices is already in progress for the target device.
                    // Sleep a little and try again.
                    //
                    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(100));
                }
            }

            if(!services)
            {
                throw BluetoothException(__FILE__, __LINE__, "query for services failed for address " + addr);
            }

            //
            // The returned dictionary contains an XML record for each service advertised by the
            // remote device. We have to parse the XML and look for certain elements. Specifically,
            // we're looking for a UUID that matches the one supplied by the caller. If we find
            // that, we look in the same record for an RFCOMM channel.
            //
            IceXML::DocumentPtr doc;
            assert(services->getType()->getKind() == DBus::Type::KindArray);
            DBus::ArrayValuePtr d = DBus::ArrayValuePtr::dynamicCast(services);
            for(vector<DBus::ValuePtr>::const_iterator p = d->elements.begin(); p != d->elements.end(); ++p)
            {
                assert((*p)->getType()->getKind() == DBus::Type::KindDictEntry);
                DBus::DictEntryValuePtr pe = DBus::DictEntryValuePtr::dynamicCast(*p);
                assert(pe->value->getType()->getKind() == DBus::Type::KindString);
                DBus::StringValuePtr rec = DBus::StringValuePtr::dynamicCast(pe->value);

                try
                {
                    //
                    // This is what we're processing:
                    //
                    // <record>
                    //     <attribute id="0x0001">
                    //         <sequence>
                    //             <uuid value="f6d289b4-1596-4294-ac34-f08e8adbfe5b" />
                    //         </sequence>
                    //     </attribute>
                    //     <attribute id="0x0004">
                    //         <sequence>
                    //             <sequence>
                    //                 <uuid value="0x0100" />
                    //             </sequence>
                    //             <sequence>
                    //                 <uuid value="0x0003" />
                    //                 <uint8 value="0x01" />
                    //             </sequence>
                    //         </sequence>
                    //     </attribute>
                    //     ...
                    // </record>
                    //

                    istringstream istr(rec->v);
                    doc = IceXML::Parser::parse(istr);
                    IceXML::NodePtr record = findChild(doc, "record");
                    if(!record)
                    {
                        throw BluetoothException(__FILE__, __LINE__, "malformed XML - can't find <record>");
                    }

                    //
                    // Attribute 0x0001 holds the UUID. See if it matches the target UUID.
                    //
                    IceXML::NodePtr attrib = findAttribute(record, 1);
                    IceXML::NodePtr seq;
                    if(attrib)
                    {
                        seq = findSequence(attrib, uuid);
                    }
                    if(seq)
                    {
                        //
                        // We found a matching service UUID. Now look for the RFCOMM channel in the
                        // protocol descriptor list.
                        //
                        attrib = findAttribute(record, 4);
                        if(attrib)
                        {
                            seq = findSequence(attrib, 3);
                            if(seq)
                            {
                                IceXML::NodePtr ch = findChild(seq, "uint8");
                                if(ch)
                                {
                                    string val = ch->getAttribute("value");
                                    int channel;
                                    if(sscanf(val.c_str(), "%x", &channel) == 1)
                                    {
                                        channels.push_back(channel);
                                    }
                                }
                            }
                        }
                    }
                }
                catch(const IceXML::ParserException& ex)
                {
                    if(doc)
                    {
                        doc->destroy();
                    }
                    ostringstream ostr;
                    ostr << ex.reason() << endl << rec->v;
                    throw BluetoothException(__FILE__, __LINE__, ostr.str());
                }
                catch(...)
                {
                    if(doc)
                    {
                        doc->destroy();
                    }
                    throw;
                }
            }

            if(doc)
            {
                doc->destroy();
            }

            {
                IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

                if(_destroyed)
                {
                    throw CommunicatorDestroyedException(__FILE__, __LINE__);
                }
            }
        }
        catch(const DBus::Exception& ex)
        {
            cb->exception(BluetoothException(__FILE__, __LINE__, ex.reason));
            failed = true;
        }
        catch(const LocalException& ex)
        {
            cb->exception(ex);
            failed = true;
        }
        catch(const std::exception& ex)
        {
            cb->exception(UnknownException(__FILE__, __LINE__, ex.what()));
            failed = true;
        }
        catch(...)
        {
            cb->exception(UnknownException(__FILE__, __LINE__, "unknown C++ exception"));
            failed = true;
        }

        if(!failed)
        {
            if(channels.empty())
            {
                //
                // No service found for the UUID at the remote address. We treat this as if the
                // server is not running.
                //
                cb->exception(ConnectFailedException(__FILE__, __LINE__, ECONNREFUSED));
            }
            else
            {
                cb->completed(channels);
            }
        }

        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

            vector<IceUtil::ThreadPtr>::iterator p = find(_threads.begin(), _threads.end(), thread);
            if(p != _threads.end())
            {
                _threads.erase(p);
            }
        }
    }

    class FindServiceThread : public IceUtil::Thread
    {
    public:

        FindServiceThread(const BluetoothServicePtr& service, const string& addr, const string& uuid,
                          const FindServiceCallbackPtr& cb) :
            _service(service),
            _addr(addr),
            _uuid(uuid),
            _cb(cb)
        {
        }

        virtual void run()
        {
            _service->runFindService(this, _addr, _uuid, _cb);
        }

        BluetoothServicePtr _service;
        string _addr;
        string _uuid;
        FindServiceCallbackPtr _cb;
    };

    IceUtil::Monitor<IceUtil::Mutex> _lock;
    bool _destroyed;
    DBus::ConnectionPtr _dbusConnection;
    vector<IceUtil::ThreadPtr> _threads;
#ifdef ICE_CPP11_MAPPING
    map<string, vector<function<void(const string&, const PropertyMap&)>>> _discoveryCallbacks;
#else
    map<string, vector<DiscoveryCallbackPtr> > _discoveryCallbacks;
#endif
};

}

IceUtil::Shared* IceBT::upCast(IceBT::BluetoothService* p) { return p; }

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
    _service = new BluetoothService;
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

unsigned int
IceBT::Engine::addService(const string& address, const string& name, const string& uuid, int channel)
{
    return _service->addService(address, name, uuid, channel);
}

void
IceBT::Engine::findService(const string& address, const string& uuid, const FindServiceCallbackPtr& cb)
{
    _service->findService(address, uuid, cb);
}

void
IceBT::Engine::removeService(const string& address, unsigned int handle)
{
    _service->removeService(address, handle);
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

void
IceBT::Engine::destroy()
{
    _service->destroy();
}
