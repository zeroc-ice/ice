// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/PropertiesI.h>
#include <Ice/Initialize.h>
#include <Ice/LocalException.h>
#include <fstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

string
Ice::PropertiesI::getProperty(const string& key)
{
    IceUtil::Mutex::Lock sync(*this);

    map<string, string>::const_iterator p = _properties.find(key);
    if(p != _properties.end())
    {
	return p->second;
    }
    else
    {
	return string();
    }
}

string
Ice::PropertiesI::getPropertyWithDefault(const string& key, const string& value)
{
    IceUtil::Mutex::Lock sync(*this);

    map<string, string>::const_iterator p = _properties.find(key);
    if(p != _properties.end())
    {
	return p->second;
    }
    else
    {
	return value;
    }
}

Int
Ice::PropertiesI::getPropertyAsInt(const string& key)
{
    return getPropertyAsIntWithDefault(key, 0);
}

Int
Ice::PropertiesI::getPropertyAsIntWithDefault(const string& key, Int value)
{
    IceUtil::Mutex::Lock sync(*this);
    
    map<string, string>::const_iterator p = _properties.find(key);
    if(p != _properties.end())
    {
	istringstream v(p->second);
	if(!(v >> value) || !v.eof())
	{
	    return 0;
	}
    }

    return value;
}

PropertyDict
Ice::PropertiesI::getPropertiesForPrefix(const string& prefix)
{
    IceUtil::Mutex::Lock sync(*this);

    PropertyDict result;
    map<string, string>::const_iterator p;
    for(p = _properties.begin(); p != _properties.end(); ++p)
    {
        if(prefix.empty() || p->first.compare(0, prefix.size(), prefix) == 0)
        {
            result.insert(*p);
        }
    }

    return result;
}

//
// Valid properties for each application.
// A '*' character is a wildcard. If used, it must appear at the end of the string.
// Examples: "Ice.Foo.*" allows all properties with that prefix, such as "Ice.Foo.Bar".
//           "Ice.Foo*" allows properties such as "Ice.Foo.Bar" and "Ice.FooBar".
//
static const string iceProps[] =
{
    "ChangeUser",
    "Config",
    "ConnectionIdleTime",
    "Default.Host",
    "Default.Locator",
    "Default.Package",
    "Default.Protocol", // Not used by C++ but left here to suppress warnings.
    "Default.Router",
    "GC.Interval",
    "Logger.Timestamp",
    "MessageSizeMax",
    "MonitorConnections",
    "Nohup",
    "NullHandleAbort",
    "Override.Compress",
    "Override.ConnectTimeout",
    "Override.Timeout",
    "Package.*", // Not used by C++ but left here to suppress warnings.
    "Plugin.*",
    "PrintAdapterReady",
    "PrintProcessId",
    "ProgramName",
    "RetryIntervals",
    "ServerId",
    "ServerIdleTime",
    "StdErr",
    "StdOut",
    "ThreadPool.Client.Size",
    "ThreadPool.Client.SizeMax",
    "ThreadPool.Client.SizeWarn",
    "ThreadPool.Client.StackSize",
    "ThreadPool.Server.Size",
    "ThreadPool.Server.SizeMax",
    "ThreadPool.Server.SizeWarn",
    "ThreadPool.Server.StackSize",
    "ThreadPerConnection",
    "ThreadPerConnection.StackSize",
    "Trace.GC",
    "Trace.Location",
    "Trace.Network",
    "Trace.Protocol",
    "Trace.Retry",
    "Trace.Slicing",
    "UDP.RcvSize",
    "UDP.SndSize",
    "UseEventLog",
    "UseSyslog",
    "Warn.AMICallback",
    "Warn.Connections",
    "Warn.Datagrams",
    "Warn.Dispatch",
    "Warn.Leaks"
};

static const string iceBoxProps[] =
{
    "LoadOrder",
    "PrintServicesReady",
    "Service.*",
    "ServiceManager.AdapterId",
    "ServiceManager.Endpoints",
    "ServiceManager.PublishedEndpoints",
    "ServiceManager.Identity",
    "ServiceManager.RegisterProcess",
    "ServiceManager.ThreadPool.Size",
    "ServiceManager.ThreadPool.SizeMax",
    "ServiceManager.ThreadPool.SizeWarn",
    "ServiceManager.ThreadPool.StackSize",
    "UseSharedCommunicator.*"
};

static const string icePackProps[] =
{
    "Node.AdapterId",
    "Node.CollocateRegistry",
    "Node.Data",
    "Node.Endpoints",
    "Node.PublishedEndpoints",
    "Node.Name",
    "Node.Output",
    "Node.PrintServersReady",
    "Node.PropertiesOverride",
    "Node.RedirectErrToOut",
    "Node.ThreadPool.Size",
    "Node.ThreadPool.SizeMax",
    "Node.ThreadPool.SizeWarn",
    "Node.ThreadPool.StackSize",
    "Node.Trace.Activator",
    "Node.Trace.Adapter",
    "Node.Trace.Server",
    "Node.WaitTime",
    "Registry.Admin.AdapterId",
    "Registry.Admin.Endpoints",
    "Registry.Admin.PublishedEndpoints",
    "Registry.Admin.ThreadPool.Size",
    "Registry.Admin.ThreadPool.SizeMax",
    "Registry.Admin.ThreadPool.SizeWarn",
    "Registry.Admin.ThreadPool.StackSize",
    "Registry.Client.AdapterId",
    "Registry.Client.Endpoints",
    "Registry.Client.PublishedEndpoints",
    "Registry.Client.ThreadPool.Size",
    "Registry.Client.ThreadPool.SizeMax",
    "Registry.Client.ThreadPool.SizeWarn",
    "Registry.Client.ThreadPool.StackSize",
    "Registry.Data",
    "Registry.DynamicRegistration",
    "Registry.Internal.AdapterId",
    "Registry.Internal.Endpoints",
    "Registry.Internal.PublishedEndpoints",
    "Registry.Internal.ThreadPool.Size",
    "Registry.Internal.ThreadPool.SizeMax",
    "Registry.Internal.ThreadPool.SizeWarn",
    "Registry.Internal.ThreadPool.StackSize",
    "Registry.Server.AdapterId",
    "Registry.Server.Endpoints",
    "Registry.Server.PublishedEndpoints",
    "Registry.Server.ThreadPool.Size",
    "Registry.Server.ThreadPool.SizeMax",
    "Registry.Server.ThreadPool.SizeWarn",
    "Registry.Server.ThreadPool.StackSize",
    "Registry.Trace.AdapterRegistry",
    "Registry.Trace.NodeRegistry",
    "Registry.Trace.ObjectRegistry",
    "Registry.Trace.ServerRegistry"
};

static const string icePatchProps[] =
{
    "BusyTimeout",
    "Directory",
    "Dynamic",
    "Endpoints",
    "PublishedEndpoints",
    "RemoveOrphaned",
    "Thorough",
    "Trace.Files",
    "UpdatePeriod"
};

static const string iceSSLProps[] =
{
    "Client.CertPath*",
    "Client.Config",
    "Client.Handshake.Retries",
    "Client.IgnoreValidPeriod",
    "Client.Overrides.CACertificate",
    "Client.Overrides.DSA.Certificate",
    "Client.Overrides.DSA.PrivateKey",
    "Client.Overrides.RSA.Certificate",
    "Client.Overrides.RSA.PrivateKey",
    "Client.Passphrase.Retries",
    "Server.CertPath*",
    "Server.Config",
    "Server.IgnoreValidPeriod",
    "Server.Overrides.CACertificate",
    "Server.Overrides.DSA.Certificate",
    "Server.Overrides.DSA.PrivateKey",
    "Server.Overrides.RSA.Certificate",
    "Server.Overrides.RSA.PrivateKey",
    "Server.Passphrase.Retries",
    "Trace.Security"
};

static const string iceStormProps[] =
{
    "Flush.Timeout",
    "Publish.AdapterId",
    "Publish.Endpoints",
    "Publish.PublishedEndpoints",
    "Publish.RegisterProcess",
    "Publish.ThreadPool.Size",
    "Publish.ThreadPool.SizeMax",
    "Publish.ThreadPool.SizeWarn",
    "Publish.ThreadPool.StackSize",
    "TopicManager.AdapterId",
    "TopicManager.Endpoints",
    "TopicManager.PublishedEndpoints",
    "TopicManager.Proxy",
    "TopicManager.RegisterProcess",
    "TopicManager.ThreadPool.Size",
    "TopicManager.ThreadPool.SizeMax",
    "TopicManager.ThreadPool.SizeWarn",
    "TopicManager.ThreadPool.StackSize",
    "Trace.Flush",
    "Trace.Subscriber",
    "Trace.Topic",
    "Trace.TopicManager"
};

static const string glacierProps[] =
{
    "Router.AcceptCert",
    "Router.AdapterId",
    "Router.AllowCategories",
    "Router.Client.AdapterId",
    "Router.Client.Endpoints",
    "Router.Client.PublishedEndpoints",
    "Router.Client.ForwardContext",
    "Router.Client.SleepTime",
    "Router.Client.ThreadPool.Size",
    "Router.Client.ThreadPool.SizeMax",
    "Router.Client.ThreadPool.SizeWarn",
    "Router.Client.ThreadPool.StackSize",
    "Router.Client.Throttle.Twoways",
    "Router.Endpoints",
    "Router.PublishedEndpoints",
    "Router.ThreadPool.Size",
    "Router.ThreadPool.SizeMax",
    "Router.ThreadPool.SizeWarn",
    "Router.ThreadPool.StackSize",
    "Router.Identity",
    "Router.PrintProxyOnFd",
    "Router.Server.AdapterId",
    "Router.Server.Endpoints",
    "Router.Server.PublishedEndpoints",
    "Router.Server.ForwardContext",
    "Router.Server.SleepTime",
    "Router.Server.ThreadPool.Size",
    "Router.Server.ThreadPool.SizeMax",
    "Router.Server.ThreadPool.SizeWarn",
    "Router.Server.ThreadPool.StackSize",
    "Router.Server.Throttle.Twoways",
    "Router.SessionManager",
    "Router.Trace.Client",
    "Router.Trace.RoutingTable",
    "Router.Trace.Server",
    "Router.Trace.Throttle",
    "Router.UserId",
    "Starter.AdapterId",
    "Starter.AddUserToAllowCategories",
    "Starter.Certificate.BitStrength",
    "Starter.Certificate.CommonName",
    "Starter.Certificate.Country",
    "Starter.Certificate.IssuedAdjust",
    "Starter.Certificate.Locality",
    "Starter.Certificate.Organization",
    "Starter.Certificate.OrganizationalUnit",
    "Starter.Certificate.SecondsValid",
    "Starter.Certificate.StateProvince",
    "Starter.CryptPasswords",
    "Starter.Endpoints",
    "Starter.PublishedEndpoints",
    "Starter.PermissionsVerifier",
    "Starter.PropertiesOverride",
    "Starter.RouterPath",
    "Starter.StartupTimeout",
    "Starter.ThreadPool.Size",
    "Starter.ThreadPool.SizeMax",
    "Starter.ThreadPool.SizeWarn",
    "Starter.ThreadPool.StackSize",
    "Starter.Trace"
};

static const string glacier2Props[] =
{
    "Admin.Endpoints",
    "Admin.PublishedEndpoints",
    "AdminIdentity",
    "Client.AlwaysBatch",
    "Client.Endpoints",
    "Client.PublishedEndpoints",
    "Client.ForwardContext",
    "Client.SleepTime",
    "Client.Trace.Override",
    "Client.Trace.Reject",
    "Client.Trace.Request",
    "Client.Buffered",
    "CryptPasswords",
    "PermissionsVerifier",
    "RouterIdentity",
    "Server.AlwaysBatch",
    "Server.Endpoints",
    "Server.PublishedEndpoints",
    "Server.ForwardContext",
    "Server.SleepTime",
    "Server.Trace.Override",
    "Server.Trace.Request",
    "Server.Buffered",
    "SessionManager",
    "SessionTimeout",
    "Trace.RoutingTable",
    "Trace.Session"
};

static const string freezeProps[] =
{
    "DbEnv.*",
    "Evictor.*",
    "Trace.DbEnv",
    "Trace.Evictor",
    "Trace.Map",
    "Trace.Transaction",
    "Warn.CloseInFinalize",
    "Warn.Deadlocks"
};

struct PropertyValues
{
    string prefix;
    const string* props;
    size_t propsSize;

    PropertyValues(string pf, const string* p, size_t s) :
	prefix(pf), props(p), propsSize(s)
    {
    }
};

//
// Array of valid properties for each application.
//
static const PropertyValues validProps[] =
{
    PropertyValues("Freeze", freezeProps, sizeof(freezeProps) / sizeof(freezeProps[0])),
    PropertyValues("Glacier", glacierProps, sizeof(glacierProps) / sizeof(glacierProps[0])),
    PropertyValues("Glacier2", glacier2Props, sizeof(glacier2Props) / sizeof(glacier2Props[0])),
    PropertyValues("IceBox", iceBoxProps, sizeof(iceBoxProps) / sizeof(iceBoxProps[0])),
    PropertyValues("Ice", iceProps, sizeof(iceProps) / sizeof(iceProps[0])),
    PropertyValues("IcePack", icePackProps, sizeof(icePackProps) / sizeof(icePackProps[0])),
    PropertyValues("IcePatch", icePatchProps, sizeof(icePatchProps) / sizeof(icePatchProps[0])),
    PropertyValues("IceSSL", iceSSLProps, sizeof(iceSSLProps) / sizeof(iceSSLProps[0])),
    PropertyValues("IceStorm", iceStormProps, sizeof(iceStormProps) / sizeof(iceStormProps[0]))
};

static const size_t validPropsSize = sizeof(validProps) / sizeof(validProps[0]);

void
Ice::PropertiesI::setProperty(const string& key, const string& value)
{
    //
    // Check if the property is legal. (We write to cerr instead of
    // using a logger because no logger may be established at the time
    // the property is parsed.)
    //
    string::size_type dotPos = key.find('.');
    if(dotPos != string::npos)
    {
	string prefix = key.substr(0, dotPos);
        bool found = false;
	for(size_t i = 0; i != validPropsSize && !found; ++i)
	{
	    if(validProps[i].prefix == prefix)
	    {
		string suffix = key.substr(++dotPos, string::npos);
		for(size_t j = 0; j != validProps[i].propsSize && !found; ++j)
		{
		    string::size_type starPos = validProps[i].props[j].find('*');
		    if(starPos == string::npos)
		    {
			found = validProps[i].props[j].compare(suffix) == 0;
		    }
		    else
		    {
			found = validProps[i].props[j].compare(0, starPos - 1, suffix.substr(0, starPos - 1)) == 0;
		    }
		}
		if(!found)
		{
		    cerr << "warning: unknown property: " << key << endl;
		}
	    }
	}
    }

    IceUtil::Mutex::Lock sync(*this);

    //
    // Set or clear the property.
    //
    if(!key.empty())
    {
	if(!value.empty())
	{
	    _properties[key] = value;
	}
	else
	{
	    _properties.erase(key);
	}
    }
}

StringSeq
Ice::PropertiesI::getCommandLineOptions()
{
    IceUtil::Mutex::Lock sync(*this);

    StringSeq result;
    result.reserve(_properties.size());
    map<string, string>::const_iterator p;
    for(p = _properties.begin(); p != _properties.end(); ++p)
    {
	result.push_back("--" + p->first + "=" + p->second);
    }
    return result;
}

StringSeq
Ice::PropertiesI::parseCommandLineOptions(const string& prefix, const StringSeq& options)
{
    string pfx = prefix;
    if(!pfx.empty() && pfx[pfx.size() - 1] != '.')
    {
	pfx += '.';
    }
    pfx = "--" + pfx;
    
    StringSeq result;
    StringSeq::size_type i;
    for(i = 0; i < options.size(); i++)
    {
        string opt = options[i];
        if(opt.find(pfx) == 0)
        {
            if(opt.find('=') == string::npos)
            {
                opt += "=1";
            }
            
            parseLine(opt.substr(2));
        }
        else
        {
            result.push_back(opt);
        }
    }
    return result;
}

StringSeq
Ice::PropertiesI::parseIceCommandLineOptions(const StringSeq& options)
{
    StringSeq args;
    args = parseCommandLineOptions("Ice", options);
    args = parseCommandLineOptions("Freeze", args);
    args = parseCommandLineOptions("Glacier", args);
    args = parseCommandLineOptions("Glacier2", args);
    args = parseCommandLineOptions("IceBox", args);
    args = parseCommandLineOptions("IcePack", args);
    args = parseCommandLineOptions("IcePatch", args);
    args = parseCommandLineOptions("IceStorm", args);
    return args;

    //
    // IceSSL is *not* in the above list because IceSSL is a plug-in. The properties for plug-ins are
    // initialized by PluginManager, so there is no need to initialize them here.
    //
}

void
Ice::PropertiesI::load(const std::string& file)
{
    ifstream in(file.c_str());
    if(!in)
    {
        SyscallException ex(__FILE__, __LINE__);
        ex.error = getSystemErrno();
        throw ex;
    }

    char line[1024];
    while(in.getline(line, 1024))
    {
	parseLine(line);
    }
}

PropertiesPtr
Ice::PropertiesI::clone()
{
    IceUtil::Mutex::Lock sync(*this);
    return new PropertiesI(this);
}

Ice::PropertiesI::PropertiesI(const PropertiesI* p) :
    _properties(p->_properties)
{
}

Ice::PropertiesI::PropertiesI()
{
    loadConfig();
}

Ice::PropertiesI::PropertiesI(StringSeq& args)
{
    StringSeq::iterator q = args.begin();
    if(q != args.end())
    {
	setProperty("Ice.ProgramName", *q);
    }
    while(q != args.end())
    {
        string s = *q;
        if(s.find("--Ice.Config") == 0)
        {
            if(s.find('=') == string::npos)
            {
                s += "=1";
            }
            parseLine(s.substr(2));
            args.erase(q);
        }
        else
        {
            ++q;
        }
    }

    loadConfig();

    args = parseIceCommandLineOptions(args);
}

void
Ice::PropertiesI::parseLine(const string& line)
{
    const string delim = " \t\r\n";
    string s = line;
    
    string::size_type idx = s.find('#');
    if(idx != string::npos)
    {
	s.erase(idx);
    }
    
    idx = s.find_last_not_of(delim);
    if(idx != string::npos && idx + 1 < s.length())
    {
	s.erase(idx + 1);
    }
    
    string::size_type beg = s.find_first_not_of(delim);
    if(beg == string::npos)
    {
	return;
    }
    
    string::size_type end = s.find_first_of(delim + "=", beg);
    if(end == string::npos)
    {
	return;
    }
    
    string key = s.substr(beg, end - beg);
    
    end = s.find('=', end);
    if(end == string::npos)
    {
	return;
    }
    ++end;

    string value;
    beg = s.find_first_not_of(delim, end);
    if(beg != string::npos)
    {
	end = s.length();
	value = s.substr(beg, end - beg);
    }
    
    setProperty(key, value);
}

void
Ice::PropertiesI::loadConfig()
{
    string value = getProperty("Ice.Config");

    if(value.empty() || value == "1")
    {
        const char* s = getenv("ICE_CONFIG");
        if(s && *s != '\0')
        {
            value = s;
        }
    }

    if(!value.empty())
    {
        const string delim = " \t\r\n";
        string::size_type beg = value.find_first_not_of(delim);
        while(beg != string::npos)
        {
            string::size_type end = value.find(",", beg);
            string file;
            if(end == string::npos)
            {
                file = value.substr(beg);
                beg = end;
            }
            else
            {
                file = value.substr(beg, end - beg);
                beg = value.find_first_not_of("," + delim, end);
            }
            load(file);
        }
    }

    setProperty("Ice.Config", value);
}
