// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/Util.h>
#include <IceGrid/Admin.h>
#include <IceGrid/Internal.h>

using namespace std;
using namespace Ice;
using namespace IceGrid;

string 
IceGrid::toString(const vector<string>& v, const string& sep)
{
    ostringstream os;
    Ice::StringSeq::const_iterator p = v.begin();
    while(p != v.end())
    {
        os << *p;
        ++p;
        if(p != v.end())
        {
            os << sep;
        }
    }
    return os.str();
}

string
IceGrid::toString(const Ice::Exception& exception)
{
    std::ostringstream os;
    try
    {
        exception.ice_throw();
    }
    catch(const NodeUnreachableException& ex)
    {
        os << ex << ":";
        os << "\nnode: " << ex.name;
        os << "\nreason: " << ex.reason;
    }
    catch(const DeploymentException& ex)
    {
        os << ex << ":";
        os << "\nreason: " << ex.reason;
    }
    catch(const Ice::Exception& ex)
    {
        os << ex;
    }
    return os.str();
}

string
IceGrid::getProperty(const PropertyDescriptorSeq& properties, const string& name, const string& def)
{    
    string result;
    for(PropertyDescriptorSeq::const_iterator q = properties.begin(); q != properties.end(); ++q)
    {
        if(q->name == name)
        {
            result = q->value;
        }
    }
    if(result.empty())
    {
        return def;
    }
    return result;
}

int
IceGrid::getPropertyAsInt(const PropertyDescriptorSeq& properties, const string& name, int def)
{    
    string strVal;
    for(PropertyDescriptorSeq::const_iterator q = properties.begin(); q != properties.end(); ++q)
    {
        if(q->name == name)
        {
            strVal = q->value;
        }
    }
    
    int result = def;

    if(!strVal.empty())
    {
        istringstream v(strVal);
        if(!(v >> result) || !v.eof())
        {
            result = def;
        }
    }
    return result;
}

bool
IceGrid::hasProperty(const PropertyDescriptorSeq& properties, const string& name)
{    
    for(PropertyDescriptorSeq::const_iterator q = properties.begin(); q != properties.end(); ++q)
    {
        if(q->name == name)
        {
            return true;
        }
    }
    return false;
}

PropertyDescriptor
IceGrid::createProperty(const string& name, const string& value)
{
    PropertyDescriptor prop;
    prop.name = name;
    prop.value = value;
    return prop;
}

string
IceGrid::escapeProperty(const string& s, bool escapeEqual)
{
    size_t firstChar = s.find_first_not_of(' ');
    size_t lastChar = s.find_last_not_of(' ');
    string result;
    bool previousCharIsEscape = false;

    for(unsigned int i = 0; i < s.size(); ++i)
    {
        char c = s[i];
        switch(c)
        {
          case ' ':
          {
              //
              // We only escape the space character when it's at the beginning
              // or at the end of the string
              //
              if(i < firstChar || i > lastChar)
              {
                  if(previousCharIsEscape)
                  {
                      result.push_back('\\'); // escape the previous char, by adding another escape.
                  }

                  result.push_back('\\');
              }
              result.push_back(c);
              previousCharIsEscape = false;
              break;
          }
  
          case '\\':
          case '#':
          case '=':
          {
              if(c == '=' && !escapeEqual)
              {
                  previousCharIsEscape = false;
              }
              else
              {
                  //
                  // We only escape the \ character when it is followed by a
                  // character that we escape, e.g. \# is encoded as \\\#, not \#
                  // and \\server is encoded as \\\server.
                  //
                  if(previousCharIsEscape)
                  {
                      result.push_back('\\'); // escape the previous char, by adding another escape.
                  }
                  if(c == '\\')
                  {
                      previousCharIsEscape = true; // deferring the potential escaping to the next loop
                  }
                  else
                  {
                      result.push_back('\\');
                      previousCharIsEscape = false;
                  }
              }
              result.push_back(c);
              break;
          }

          default:
          {
              result.push_back(c);
              previousCharIsEscape = false;
              break;
          }
        }
    }
    return result;
}

ObjectInfo
IceGrid::toObjectInfo(const Ice::CommunicatorPtr& communicator, const ObjectDescriptor& obj, const string& adapterId)
{
    ObjectInfo info;
    info.type = obj.type;
    ostringstream proxyStr;
    proxyStr << "\"" << identityToString(obj.id) << "\"";
    if(!obj.proxyOptions.empty())
    {
        proxyStr << ' ' << obj.proxyOptions;
    }
    proxyStr << " @ " << adapterId;
    try
    {
        info.proxy = communicator->stringToProxy(proxyStr.str());
    }
    catch(const Ice::ProxyParseException&)
    {
        ostringstream fallbackProxyStr;
        fallbackProxyStr << "\"" << identityToString(obj.id) << "\"" << " @ " << adapterId;
        info.proxy = communicator->stringToProxy(fallbackProxyStr.str());
    }
    return info;
}

void
IceGrid::setupThreadPool(const PropertiesPtr& properties, const string& name, int size, int sizeMax, bool serialize)
{
    if(properties->getPropertyAsIntWithDefault(name + ".Size", 0) < size)
    {
        ostringstream os;
        os << size;
        properties->setProperty(name + ".Size", os.str());
    }
    else
    {
        size = properties->getPropertyAsInt(name + ".Size");
    }

    if(sizeMax > 0 && properties->getPropertyAsIntWithDefault(name + ".SizeMax", 0) < sizeMax)
    {
        if(size >= sizeMax)
        {
            sizeMax = size * 10;
        }
        
        ostringstream os;
        os << sizeMax;
        properties->setProperty(name + ".SizeMax", os.str());
    }

    if(serialize)
    {
        properties->setProperty(name + ".Serialize", "1");
    }
}

int
IceGrid::getMMVersion(const string& o)
{
    //
    // Strip the version
    //
    string::size_type beg = o.find_first_not_of(' ');
    string::size_type end = o.find_last_not_of(' ');
    string version = o.substr(beg == string::npos ? 0 : beg, end == string::npos ? o.length() - 1 : end - beg + 1);

    string::size_type minorPos = version.find('.');
    string::size_type patchPos = version.find('.', minorPos + 1);

    if(minorPos != 1 && minorPos != 2)
    {
        return -1;
    }

    if(patchPos != string::npos)
    { 
        if((minorPos == 1 && patchPos != 3 && patchPos != 4) || (minorPos == 2 && patchPos != 4 && patchPos != 5))
        {
            return -1;
        }
        else if((version.size() - patchPos - 1) > 2)
        {
            return -1;
        }
    }
    else if((version.size() - minorPos - 1) > 2)
    {
        return -1;
    }

    int v, ver;

    istringstream major(version.substr(0, minorPos));
    major >> v;
    if(major.fail() || v > 99 || v < 1)
    {
        return -1;
    }
    ver = v;
    ver *= 100;

    istringstream minor(version.substr(minorPos + 1, patchPos != string::npos ? patchPos : version.size()));
    minor >> v;
    if(minor.fail() || v > 99 || v < 0)
    {
        return -1;
    }
    ver += v;
    ver *= 100;

    //
    // No need to get the patch number, we're only interested in
    // MAJOR.MINOR
    //
    //     if(patchPos != string::npos)
    //     {
    //      istringstream patch(version.substr(patchPos + 1));
    //      patch >> v;
    //      if(patch.fail() || v > 99 || v < 0)
    //      {
    //          return -1;
    //      }
    //      ver += v;
    //     }
    
    return ver;
}
