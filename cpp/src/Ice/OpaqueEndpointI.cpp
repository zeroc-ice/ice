// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/OpaqueEndpointI.h>
#include <Ice/BasicStream.h>
#include <Ice/Exception.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/Base64.h>
#include <Ice/HashUtil.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::OpaqueEndpointI::OpaqueEndpointI(const string& str) : 
    EndpointI(""), _rawEncoding(Encoding_1_0)
{
    const string delim = " \t\n\r";

    string::size_type beg;
    string::size_type end = 0;

    int topt = 0;
    int vopt = 0;

    while(true)
    {
        beg = str.find_first_not_of(delim, end);
        if(beg == string::npos)
        {
            break;
        }
        
        end = str.find_first_of(delim, beg);
        if(end == string::npos)
        {
            end = str.length();
        }

        string option = str.substr(beg, end - beg);
        if(option.length() != 2 || option[0] != '-')
        {
            EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "expected an endpoint option but found `" + option + "' in endpoint `opaque " + str + "'";
            throw ex;
        }

        string argument;
        string::size_type argumentBeg = str.find_first_not_of(delim, end);
        if(argumentBeg != string::npos && str[argumentBeg] != '-')
        {
            beg = argumentBeg;
            end = str.find_first_of(delim, beg);
            if(end == string::npos)
            {
                end = str.length();
            }
            argument = str.substr(beg, end - beg);
        }

        switch(option[1])
        {
            case 't':
            {
                if(argument.empty())
                {
                    EndpointParseException ex(__FILE__, __LINE__);
                    ex.str = "no argument provided for -t option in endpoint `opaque " + str + "'";
                    throw ex;
                }
                istringstream p(argument);
                Ice::Int t;
                if(!(p >> t) || !p.eof())
                {
                    EndpointParseException ex(__FILE__, __LINE__);
                    ex.str = "invalid type value `" + argument + "' in endpoint `opaque " + str + "'";
                    throw ex;
                }
                else if(t < 0 || t > 65535)
                {
                    EndpointParseException ex(__FILE__, __LINE__);
                    ex.str = "type value `" + argument + "' out of range in endpoint `opaque " + str + "'";
                    throw ex;
                }
                _type = static_cast<Ice::Short>(t);
                ++topt;
                if(topt > 1)
                {
                    EndpointParseException ex(__FILE__, __LINE__);
                    ex.str = "multiple -t options in endpoint `opaque " + str + "'";
                    throw ex;
                }
                break;
            }

            case 'v':
            {
                if(argument.empty())
                {
                    EndpointParseException ex(__FILE__, __LINE__);
                    ex.str = "no argument provided for -v option in endpoint `opaque " + str + "'";
                    throw ex;
                }
                for(string::size_type i = 0; i < argument.size(); ++i)
                {
                    if(!Base64::isBase64(argument[i]))
                    {
                        EndpointParseException ex(__FILE__, __LINE__);
                        ostringstream ostr;
                        ostr << "invalid base64 character `" << argument[i] << "' (ordinal " << (int)argument[i]
                             << ") in endpoint `opaque " << str << "'";
                        ex.str = ostr.str();
                        throw ex;
                    }
                }
                const_cast<vector<Byte>&>(_rawBytes) = Base64::decode(argument);
                ++vopt;
                if(vopt > 1)
                {
                    EndpointParseException ex(__FILE__, __LINE__);
                    ex.str = "multiple -v options in endpoint `opaque " + str + "'";
                    throw ex;
                }
                break;
            }

            case 'e':
            {
                if(argument.empty())
                {
                    Ice::EndpointParseException ex(__FILE__, __LINE__);
                    ex.str = "no argument provided for -e option in endpoint `opaque " + str + "'";
                    throw ex;
                }
                
                try 
                {
                    _rawEncoding = Ice::stringToEncodingVersion(argument);
                }
                catch(const Ice::VersionParseException& e)
                {
                    Ice::EndpointParseException ex(__FILE__, __LINE__);
                    ex.str = "invalid encoding version `" + argument + "' in endpoint `opaque " + str + "':\n" + e.str;
                    throw ex;
                }
                break;
            }

            default:
            {
                EndpointParseException ex(__FILE__, __LINE__);
                ex.str = "invalid option `" + option + "' in endpoint `opaque " + str + "'";
                throw ex;
            }
        }
    }

    if(topt != 1)
    {
        EndpointParseException ex(__FILE__, __LINE__);
        ex.str = "no -t option in endpoint `opaque " + str + "'";
        throw ex;
    }
    if(vopt != 1)
    {
        EndpointParseException ex(__FILE__, __LINE__);
        ex.str = "no -v option in endpoint `opaque " + str + "'";
        throw ex;
    }
}

IceInternal::OpaqueEndpointI::OpaqueEndpointI(Short type, BasicStream* s) :
    EndpointI(""),
    _type(type)
{
    _rawEncoding = s->startReadEncaps();
    Int sz = s->getReadEncapsSize();
    s->readBlob(const_cast<vector<Byte>&>(_rawBytes), sz);
    s->endReadEncaps();
}

void
IceInternal::OpaqueEndpointI::streamWrite(BasicStream* s) const
{
    s->write(_type);
    s->startWriteEncaps(_rawEncoding, DefaultFormat);
    s->writeBlob(_rawBytes);
    s->endWriteEncaps();
}

string
IceInternal::OpaqueEndpointI::toString() const
{
    ostringstream s;
    string val = Base64::encode(_rawBytes);
    s << "opaque -t " << _type << " -e " << _rawEncoding << " -v " << val;
    return s.str();
}

namespace
{

class InfoI : public Ice::OpaqueEndpointInfo
{
public:
    
    InfoI(Ice::Short type, const Ice::EncodingVersion& rawEncoding, const Ice::ByteSeq& rawByes);

    virtual Ice::Short
    type() const
    {
        return _type;
    }
        
    virtual bool
    datagram() const
    {
        return false;
    }
    
    virtual bool
    secure() const
    {
        return false;
    }

private:
    
    Ice::Short _type;
};


//
// COMPILERFIX: inlining this constructor causes crashes with gcc 4.0.1.
//
InfoI::InfoI(Ice::Short type, const Ice::EncodingVersion& rawEncoding, const Ice::ByteSeq& rawBytes) : 
    Ice::OpaqueEndpointInfo(-1, false, rawEncoding, rawBytes), 
    _type(type)
{
}

}

Ice::EndpointInfoPtr
IceInternal::OpaqueEndpointI::getInfo() const
{
    return new InfoI(_type, _rawEncoding, _rawBytes);
}

Short
IceInternal::OpaqueEndpointI::type() const
{
    return _type;
}

std::string
IceInternal::OpaqueEndpointI::protocol() const
{
    return "opaque";
}

Int
IceInternal::OpaqueEndpointI::timeout() const
{
    return -1;
}

EndpointIPtr
IceInternal::OpaqueEndpointI::timeout(Int) const
{
    return const_cast<OpaqueEndpointI*>(this);
}

EndpointIPtr
IceInternal::OpaqueEndpointI::connectionId(const string&) const
{
    return const_cast<OpaqueEndpointI*>(this);
}

bool
IceInternal::OpaqueEndpointI::compress() const
{
    return false;
}

EndpointIPtr
IceInternal::OpaqueEndpointI::compress(bool) const
{
    return const_cast<OpaqueEndpointI*>(this);
}

bool
IceInternal::OpaqueEndpointI::datagram() const
{
    return false;
}

bool
IceInternal::OpaqueEndpointI::secure() const
{
    return false;
}

TransceiverPtr
IceInternal::OpaqueEndpointI::transceiver(EndpointIPtr& endp) const
{
    endp = const_cast<OpaqueEndpointI*>(this);
    return 0;
}

vector<ConnectorPtr>
IceInternal::OpaqueEndpointI::connectors(Ice::EndpointSelectionType) const
{
    vector<ConnectorPtr> ret;
    return ret;
}

void
IceInternal::OpaqueEndpointI::connectors_async(Ice::EndpointSelectionType, const EndpointI_connectorsPtr& cb) const
{
    cb->connectors(vector<ConnectorPtr>());
}

AcceptorPtr
IceInternal::OpaqueEndpointI::acceptor(EndpointIPtr& endp, const string&) const
{
    endp = const_cast<OpaqueEndpointI*>(this);
    return 0;
}

vector<EndpointIPtr>
IceInternal::OpaqueEndpointI::expand() const
{
    vector<EndpointIPtr> endps;
    endps.push_back(const_cast<OpaqueEndpointI*>(this));
    return endps;
}

bool
IceInternal::OpaqueEndpointI::equivalent(const EndpointIPtr&) const
{
    return false;
}

bool
IceInternal::OpaqueEndpointI::operator==(const LocalObject& r) const
{
    const OpaqueEndpointI* p = dynamic_cast<const OpaqueEndpointI*>(&r);
    if(!p)
    {
        return false;
    }

    if(this == p)
    {
        return true;
    }

    if(_type != p->_type)
    {
        return false;
    }

    if(_rawEncoding != p->_rawEncoding)
    {
        return false;
    }

    if(_rawBytes != p->_rawBytes)
    {
        return false;
    }

    return true;
}

bool
IceInternal::OpaqueEndpointI::operator<(const LocalObject& r) const
{
    const OpaqueEndpointI* p = dynamic_cast<const OpaqueEndpointI*>(&r);
    if(!p)
    {
        const EndpointI* e = dynamic_cast<const EndpointI*>(&r);
        if(!e)
        {
            return false;
        }
        return type() < e->type();
    }

    if(this == p)
    {
        return false;
    }

    if(_type < p->_type)
    {
        return true;
    }
    else if(p->_type < _type)
    {
        return false;
    }

    if(_rawEncoding < p->_rawEncoding)
    {
        return true;
    }
    else if(p->_rawEncoding < _rawEncoding)
    {
        return false;
    }

    if(_rawBytes < p->_rawBytes)
    {
        return true;
    }
    else if(p->_rawBytes < _rawBytes)
    {
        return false;
    }

    return false;
}

Ice::Int
IceInternal::OpaqueEndpointI::hashInit() const
{
    Ice::Int h = 5381;
    hashAdd(h, _type);
    hashAdd(h, _rawEncoding.major);
    hashAdd(h, _rawEncoding.minor);
    hashAdd(h, _rawBytes);
    return h;
}
