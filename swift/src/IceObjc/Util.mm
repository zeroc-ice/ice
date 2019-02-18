// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//#include <Ice/Ice.h>
//#include <IceSSL/IceSSL.h>

#import "Ice-Objc.h"

#include "Util.h"

NSError*
convertException(const std::exception& exc)
{
    if(dynamic_cast<const Ice::LocalException*>(&exc))
    {
        auto iceEx = dynamic_cast<const Ice::LocalException*>(&exc);

        Class<ICELocalExceptionFactory> factory = [ICEUtil localExceptionFactory];

        try
        {
            iceEx->ice_throw();
        }
        catch(const Ice::InitializationException& e)
        {
            return [factory initializationException:toNSString(e.reason)];
        }
        catch(const Ice::PluginInitializationException& e)
        {
            return [factory pluginInitializationException:toNSString(e.reason)];
        }
        catch(const Ice::CollocationOptimizationException& e)
        {
            return [factory collocationOptimizationException];
        }
        catch(const Ice::AlreadyRegisteredException& e)
        {
            return [factory alreadyRegisteredException:toNSString(e.kindOfObject) id:toNSString(e.id)];
        }
        catch(const Ice::NotRegisteredException& e)
        {
            return [factory notRegisteredException:toNSString(e.kindOfObject) id:toNSString(e.id)];
        }
        catch(const Ice::TwowayOnlyException& e)
        {
            return [factory twowayOnlyException:toNSString(e.operation)];
        }
        catch(const Ice::CloneNotImplementedException& e)
        {
            return [factory cloneNotImplementedException];
        }
        catch(const Ice::VersionMismatchException& e)
        {
            return [factory versionMismatchException];
        }
        catch(const Ice::CommunicatorDestroyedException& e)
        {
            return [factory communicatorDestroyedException];
        }
        catch(const Ice::ObjectAdapterDeactivatedException& e)
        {
            return [factory objectAdapterDeactivatedException:toNSString(e.name)];
        }
        catch(const Ice::ObjectAdapterIdInUseException& e)
        {
            return [factory objectAdapterIdInUseException:toNSString(e.id)];
        }
        catch(const Ice::NoEndpointException& e)
        {
            return [factory noEndpointException:toNSString(e.proxy)];
        }
        catch(const Ice::EndpointParseException& e)
        {
            return [factory endpointParseException:toNSString(e.str)];
        }
        catch(const Ice::EndpointSelectionTypeParseException& e)
        {
            return [factory endpointSelectionTypeParseException:toNSString(e.str)];
        }
        catch(const Ice::VersionParseException& e)
        {
            return [factory versionParseException:toNSString(e.str)];
        }
        catch(const Ice::IdentityParseException& e)
        {
            return [factory identityParseException:toNSString(e.str)];
        }
        catch(const Ice::ProxyParseException& e)
        {
            return [factory proxyParseException:toNSString(e.str)];
        }
        catch(const Ice::IllegalIdentityException& e)
        {
            return [factory illegalIdentityException:toNSString(e.id.name) category:toNSString(e.id.category)];
        }
        catch(const Ice::IllegalServantException& e)
        {
            return [factory illegalServantException:toNSString(e.reason)];
        }
        catch(const Ice::DNSException& e)
        {
            return [factory dNSException:e.error host:toNSString(e.host)];
        }
        catch(const Ice::OperationInterruptedException& e)
        {
            return [factory operationInterruptedException];
        }
        catch(const Ice::InvocationCanceledException& e)
        {
            return [factory invocationCanceledException];
        }
        catch(const Ice::FeatureNotSupportedException& e)
        {
            return [factory featureNotSupportedException:toNSString(e.unsupportedFeature)];
        }
        catch(const Ice::FixedProxyException& e)
        {
            return [factory fixedProxyException];
        }
        catch(const Ice::ResponseSentException& e)
        {
            return [factory responseSentException];
        }
        catch(const Ice::SecurityException& e)
        {
            return [factory securityException:toNSString(e.reason)];
        }
        catch(const Ice::UnknownLocalException& e)
        {
            return [factory unknownLocalException:toNSString(e.unknown)];
        }
        catch(const Ice::UnknownUserException& e)
        {
            return [factory unknownUserException:toNSString(e.unknown)];
        }
        catch(const Ice::UnknownException& e)
        {
            return [factory unknownException:toNSString(e.unknown)];
        }
        catch(const Ice::ObjectNotExistException& e)
        {
            return [factory objectNotExistException:toNSString(e.id.name) category:toNSString(e.id.category) facet:toNSString(e.facet) operation:toNSString(e.operation)];
        }
        catch(const Ice::FacetNotExistException& e)
        {
            return [factory facetNotExistException:toNSString(e.id.name) category:toNSString(e.id.category) facet:toNSString(e.facet) operation:toNSString(e.operation)];
        }
        catch(const Ice::OperationNotExistException& e)
        {
            return [factory operationNotExistException:toNSString(e.id.name) category:toNSString(e.id.category) facet:toNSString(e.facet) operation:toNSString(e.operation)];
        }
        catch(const Ice::RequestFailedException& e)
        {
            return [factory requestFailedException:toNSString(e.id.name) category:toNSString(e.id.category) facet:toNSString(e.facet) operation:toNSString(e.operation)];
        }
        catch(const Ice::ConnectionRefusedException& e)
        {
            return [factory connectionRefusedException:e.error];
        }
        catch(const Ice::FileException& e)
        {
            return [factory fileException:e.error path:toNSString(e.path)];
        }
        catch(const Ice::ConnectFailedException& e)
        {
            return [factory connectFailedException:e.error];
        }
        catch(const Ice::ConnectionLostException& e)
        {
            return [factory connectionLostException:e.error];
        }
        catch(const Ice::SocketException& e)
        {
            return [factory socketException:e.error];
        }
        catch(const Ice::SyscallException& e)
        {
            return [factory syscallException:e.error];
        }
        catch(const Ice::ConnectTimeoutException& e)
        {
            return [factory connectTimeoutException];
        }
        catch(const Ice::CloseTimeoutException& e)
        {
            return [factory closeTimeoutException];
        }
        catch(const Ice::ConnectionTimeoutException& e)
        {
            return [factory connectionTimeoutException];
        }
        catch(const Ice::InvocationTimeoutException& e)
        {
            return [factory invocationTimeoutException];
        }
        catch(const Ice::TimeoutException& e)
        {
            return [factory timeoutException];
        }
        catch(const Ice::BadMagicException& e)
        {
            NSMutableArray* badMagic = [[NSMutableArray alloc] initWithCapacity:e.badMagic.size()];
            for(Ice::Byte b: e.badMagic)
            {
                [badMagic addObject: [NSNumber numberWithUnsignedChar:b]];
            }

            return [factory badMagicException:toNSString(e.reason) badMagic:badMagic];
        }
        catch(const Ice::UnsupportedProtocolException& e)
        {
            return [factory unsupportedProtocolException:toNSString(e.reason) badMajor:e.bad.major badMinor:e.bad.minor supportedMajor:e.supported.major supportedMinor:e.supported.minor];
        }
        catch(const Ice::UnsupportedEncodingException& e)
        {
            return [factory unsupportedProtocolException:toNSString(e.reason) badMajor:e.bad.major badMinor:e.bad.minor supportedMajor:e.supported.major supportedMinor:e.supported.minor];
        }
        catch(const Ice::UnknownMessageException& e)
        {
            return [factory unknownMessageException:toNSString(e.reason)];
        }
        catch(const Ice::ConnectionNotValidatedException& e)
        {
            return [factory connectionNotValidatedException:toNSString(e.reason)];
        }
        catch(const Ice::UnknownRequestIdException& e)
        {
            return [factory unknownRequestIdException:toNSString(e.reason)];
        }
        catch(const Ice::UnknownReplyStatusException& e)
        {
            return [factory unknownReplyStatusException:toNSString(e.reason)];
        }
        catch(const Ice::CloseConnectionException& e)
        {
            return [factory closeConnectionException:toNSString(e.reason)];
        }
        catch(const Ice::ConnectionManuallyClosedException& e)
        {
            return [factory connectionManuallyClosedException:e.graceful];
        }
        catch(const Ice::IllegalMessageSizeException& e)
        {
            return [factory illegalMessageSizeException:toNSString(e.reason)];
        }
        catch(const Ice::CompressionException& e)
        {
            return [factory compressionException:toNSString(e.reason)];
        }
        catch(const Ice::DatagramLimitException& e)
        {
            return [factory datagramLimitException:toNSString(e.reason)];
        }
        catch(const Ice::ProxyUnmarshalException& e)
        {
            return [factory proxyUnmarshalException:toNSString(e.reason)];
        }
        catch(const Ice::UnmarshalOutOfBoundsException& e)
        {
            return [factory unmarshalOutOfBoundsException:toNSString(e.reason)];
        }
        catch(const Ice::NoValueFactoryException& e)
        {
            return [factory noValueFactoryException:toNSString(e.reason) type:toNSString(e.type)];
        }
        catch(const Ice::UnexpectedObjectException& e)
        {
            return [factory unexpectedObjectException:toNSString(e.reason) type:toNSString(e.type) expectedType:toNSString(e.expectedType)];
        }
        catch(const Ice::MemoryLimitException& e)
        {
            return [factory memoryLimitException:toNSString(e.reason)];
        }
        catch(const Ice::StringConversionException& e)
        {
            return [factory stringConversionException:toNSString(e.reason)];
        }
        catch(const Ice::EncapsulationException& e)
        {
            return [factory encapsulationException:toNSString(e.reason)];
        }
        catch(const Ice::MarshalException& e)
        {
            return [factory marshalException:toNSString(e.reason)];
        }
        catch(const Ice::ProtocolException& e)
        {
            return [factory protocolException:toNSString(e.reason)];
        }
        catch(const Ice::LocalException& e)
        {
            return [factory localException];
        }
    }

    return nil;
}

NSObject*
toObjC(const std::shared_ptr<Ice::Endpoint>& endpoint)
{
    //TODO double check this
    return [[ICEEndpoint alloc] initWithLocalObject:&(*endpoint)];
}

void
fromObjC(id object, std::shared_ptr<Ice::Endpoint>& endpoint)
{
    ICEEndpoint* endpt = object;
    endpoint = object == [NSNull null] ? nullptr : [endpt endpoint];
}

NSObject*
toObjC(const std::shared_ptr<IceSSL::Certificate>& cert)
{
    return toNSString(cert->encode());
}
