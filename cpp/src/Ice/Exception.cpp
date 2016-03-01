// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Exception.h>
#include <Ice/LocalException.h>
#include <Ice/Network.h>
#include <Ice/Plugin.h>
#include <Ice/SlicedData.h>
#include <Ice/BasicStream.h>
#include <Ice/Stream.h>
#include <IceUtil/StringUtil.h>
#ifdef ICE_OS_WINRT
#    include <IceUtil/StringConverter.h>
#endif
#include <iomanip>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{

inline string
socketErrorToString(int error)
{
    if(error == 0)
    {
        return "unknown error";
    }
#ifdef ICE_OS_WINRT
    if(error == E_ACCESSDENIED)
    {
        ostringstream os;
        os << "access to a resource or feature is denied, ensure that you have requested the appropriate\n";
        os << "capability and made the required declarations in the package manifest of your app.";
        return os.str();
    }
    else
    {
        //
        // Don't need to use a wide string converter as the wide string come
        // from Windows API.
        //
        return IceUtil::wstringToString(
            static_cast<Windows::Networking::Sockets::SocketErrorStatus>(error).ToString()->Data(),
            IceUtil::getProcessStringConverter());
    }
#else
    return IceUtilInternal::errorToString(error);
#endif
}

};

namespace IceInternal
{

namespace Ex
{

void
throwUOE(const string& expectedType, const ObjectPtr& v)
{
    //
    // If the object is an unknown sliced object, we didn't find an
    // object factory, in this case raise a NoObjectFactoryException
    // instead.
    //
    UnknownSlicedObject* uso = dynamic_cast<UnknownSlicedObject*>(v.get());
    if(uso)
    {
        throw NoObjectFactoryException(__FILE__, __LINE__, "", uso->getUnknownTypeId());
    }

    string type = v->ice_id();
    throw Ice::UnexpectedObjectException(__FILE__, __LINE__,
                                         "expected element of type `" + expectedType + "' but received '" +
                                         type, type, expectedType);
}

void
throwMemoryLimitException(const char* file, int line, size_t requested, size_t maximum)
{
    ostringstream s;
    s << "requested " << requested << " bytes, maximum allowed is " << maximum << " bytes (see Ice.MessageSizeMax)";
    throw Ice::MemoryLimitException(file, line, s.str());
}

void
throwMarshalException(const char* file, int line, const string& reason)
{
    throw Ice::MarshalException(file, line, reason);
}

}
}

void
Ice::UserException::__write(::IceInternal::BasicStream* os) const
{
    os->startWriteException(0);
    __writeImpl(os);
    os->endWriteException();
}

void
Ice::UserException::__read(::IceInternal::BasicStream* is)
{
    is->startReadException();
    __readImpl(is);
    is->endReadException(false);
}

void
Ice::UserException::__write(const Ice::OutputStreamPtr& os) const
{
    os->startException(0);
    __writeImpl(os);
    os->endException();
}

void
Ice::UserException::__read(const Ice::InputStreamPtr& is)
{
    is->startException();
    __readImpl(is);
    is->endException(false);
}

void
Ice::UserException::__writeImpl(const Ice::OutputStreamPtr&) const
{
    throw MarshalException(__FILE__, __LINE__, "user exception was not generated with stream support");
}

void
Ice::UserException::__readImpl(const Ice::InputStreamPtr&)
{
    throw MarshalException(__FILE__, __LINE__, "user exception was not generated with stream support");
}

bool
Ice::UserException::__usesClasses() const
{
    return false;
}

Ice::LocalException::LocalException(const char* file, int line) :
    Exception(file, line)
{
}

Ice::LocalException::~LocalException() throw()
{
}

Ice::SystemException::SystemException(const char* file, int line) :
    Exception(file, line)
{
}

Ice::SystemException::~SystemException() throw()
{
}

#if defined(__SUNPRO_CC)
ostream&
Ice::operator<<(ostream& out, const Ice::UserException& ex)
{
    ex.ice_print(out);
    return out;
}

ostream&
Ice::operator<<(ostream& out, const Ice::LocalException& ex)
{
    ex.ice_print(out);
    return out;
}

ostream&
Ice::operator<<(ostream& out, const Ice::SystemException& ex)
{
    ex.ice_print(out);
    return out;
}
#endif

void
Ice::InitializationException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ninitialization exception";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::UnknownException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nunknown exception";
    if(!unknown.empty())
    {
        out << ":\n" << unknown;
    }
}

void
Ice::UnknownLocalException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nunknown local exception";
    if(!unknown.empty())
    {
        out << ":\n" << unknown;
    }
}

void
Ice::UnknownUserException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nunknown user exception";
    if(!unknown.empty())
    {
        out << ":\n" << unknown;
    }
}

void
Ice::VersionMismatchException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nIce library version mismatch";
}

void
Ice::CommunicatorDestroyedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ncommunicator object destroyed";
}

void
Ice::ObjectAdapterDeactivatedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nobject adapter `" << name << "' deactivated";
}

void
Ice::ObjectAdapterIdInUseException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nobject adapter with id `" << id << "' is already in use";
}

void
Ice::NoEndpointException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nno suitable endpoint available for proxy `" << proxy << "'";
}

void
Ice::EndpointParseException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nerror while parsing endpoint `" << str << "'";
}

void
Ice::EndpointSelectionTypeParseException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nerror while parsing endpoint selection type `" << str << "'";
}

void
Ice::VersionParseException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nerror while parsing version `" << str << "'";
}

void
Ice::IdentityParseException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nerror while parsing identity `" << str << "'";
}

void
Ice::ProxyParseException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nerror while parsing proxy `" << str << "'";
}

void
Ice::IllegalIdentityException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nillegal identity: `";
    if(id.category.empty())
    {
        out << IceUtilInternal::escapeString(id.name, "/");
    }
    else
    {
        out << IceUtilInternal::escapeString(id.category, "/") << '/' << IceUtilInternal::escapeString(id.name, "/");
    }
    out << "'";
}

void
Ice::IllegalServantException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nillegal servant: `" << reason << "'";
}


static void
printFailedRequestData(ostream& out, const RequestFailedException& ex)
{
    out << ":\nidentity: `";
    if(ex.id.category.empty())
    {
        out << IceUtilInternal::escapeString(ex.id.name, "/");
    }
    else
    {
        out << IceUtilInternal::escapeString(ex.id.category, "/") << '/' << IceUtilInternal::escapeString(ex.id.name, "/");
    }
    out << "'";
    out << "\nfacet: " << ex.facet;
    out << "\noperation: " << ex.operation;
}

void
Ice::RequestFailedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nrequest failed";
    printFailedRequestData(out, *this);
}

void
Ice::ObjectNotExistException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nobject does not exist";
    printFailedRequestData(out, *this);
}

void
Ice::FacetNotExistException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nfacet does not exist";
    printFailedRequestData(out, *this);
}

void
Ice::OperationNotExistException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\noperation does not exist";
    printFailedRequestData(out, *this);
}

void
Ice::SyscallException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    if(error != 0)
    {
        out << ":\nsyscall exception: " << IceUtilInternal::errorToString(error);
    }
}

void
Ice::SocketException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nsocket exception: " << socketErrorToString(error);
}

void
Ice::FileException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nfile exception: ";
    if(error == 0)
    {
        out << "couldn't open file";
    }
    else
    {
        out << IceUtilInternal::errorToString(error);
    }
    if(!path.empty())
    {
        out << "\npath: " << path;
    }
}

void
Ice::ConnectFailedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nconnect failed: " << socketErrorToString(error);
}

void
Ice::ConnectionRefusedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nconnection refused: " << socketErrorToString(error);
}

void
Ice::ConnectionLostException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nconnection lost: ";
    if(error == 0)
    {
        out << "recv() returned zero";
    }
    else
    {
        out << socketErrorToString(error);
    }
}

void
Ice::DNSException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nDNS error: ";
#ifdef ICE_OS_WINRT
    out << socketErrorToString(error);
#else
    out << errorToStringDNS(error);
#endif
    out << "\nhost: " << host;
}

void
Ice::OperationInterruptedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\noperation interrupted";
}


void
Ice::TimeoutException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ntimeout while sending or receiving data";
}

void
Ice::ConnectTimeoutException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ntimeout while establishing a connection";
}

void
Ice::CloseTimeoutException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ntimeout while closing a connection";
}

void
Ice::ConnectionTimeoutException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nconnection has timed out";
}

void
Ice::InvocationTimeoutException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ninvocation has timed out";
}

void
Ice::InvocationCanceledException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\ninvocation canceled";
}

void
Ice::ProtocolException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol exception";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::BadMagicException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nunknown magic number: ";

    ios_base::fmtflags originalFlags = out.flags();     // Save stream state
    ostream::char_type originalFill = out.fill();

    out.flags(ios_base::hex);                           // Change to hex
    out.fill('0');                                      // Fill with leading zeros

    out << "0x" << setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(badMagic[0])) << ", ";
    out << "0x" << setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(badMagic[1])) << ", ";
    out << "0x" << setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(badMagic[2])) << ", ";
    out << "0x" << setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(badMagic[3]));

    out.fill(originalFill);                             // Restore stream state
    out.flags(originalFlags);

    if(!reason.empty())
    {
        out << "\n" << reason;
    }
}

void
Ice::UnsupportedProtocolException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: unsupported protocol version: " << bad;
    out << "\n(can only support protocols compatible with version " << supported << ")";
}

void
Ice::UnsupportedEncodingException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nencoding error: unsupported encoding version: " << bad;
    out << "\n(can only support encodings compatible with version " << supported << ")";
    if(!reason.empty())
    {
        out << "\n" << reason;
    }
}

void
Ice::UnknownMessageException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: unknown message type";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::ConnectionNotValidatedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: received message over unvalidated connection";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::UnknownRequestIdException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: unknown request id";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::UnknownReplyStatusException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: unknown reply status";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::CloseConnectionException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: connection closed";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::ForcedCloseConnectionException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: connection forcefully closed";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::IllegalMessageSizeException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: illegal message size";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::CompressionException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: failed to compress or uncompress data";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::DatagramLimitException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: maximum datagram payload size exceeded";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::MarshalException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: error during marshaling or unmarshaling";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::ProxyUnmarshalException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: inconsistent proxy data during unmarshaling";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::UnmarshalOutOfBoundsException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: out of bounds during unmarshaling";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::NoObjectFactoryException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: no suitable object factory found for `" << type << "'";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::UnexpectedObjectException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nunexpected class instance of type `" << type <<
        "'; expected instance of type `" << expectedType << "'";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::MemoryLimitException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: memory limit exceeded";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::StringConversionException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: string conversion failed";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::EncapsulationException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nprotocol error: illegal encapsulation";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::PluginInitializationException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nplug-in initialization failed";
    if(!reason.empty())
    {
        out << ": " << reason;
    }
}

void
Ice::CollocationOptimizationException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nrequested feature not available with collocation optimization";
}

void
Ice::AlreadyRegisteredException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n" << kindOfObject << " with id `" << id << "' is already registered";
}

void
Ice::NotRegisteredException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n no " << kindOfObject << " with id `" << id << "' is registered";
}

void
Ice::TwowayOnlyException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n operation `" << operation << "' can only be invoked as a twoway request";
}

void
Ice::CloneNotImplementedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n ice_clone() must be implemented in classes derived from abstract base classes";
}

void
Ice::FeatureNotSupportedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nfeature `" << unsupportedFeature << "' is not supported.";
}

void
Ice::SecurityException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nsecurity exception";
    if(!reason.empty())
    {
        out << ":\n" << reason;
    }
}

void
Ice::FixedProxyException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nfixed proxy exception";
}

void
Ice::ResponseSentException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nresponse sent exception";
}

#ifdef ICE_USE_CFSTREAM
void
Ice::CFNetworkException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nnetwork exception: domain: " << domain << " error: " << error;
}
#endif
