// Copyright (c) ZeroC, Inc.
#import "Convert.h"
#import "include/IceUtil.h"
#import "include/LocalExceptionFactory.h"

#include <cstdlib>
#include <typeinfo>

NSError*
convertException(std::exception_ptr exc)
{
    assert(exc);
    Class<ICELocalExceptionFactory> factory = [ICEUtil localExceptionFactory];

    try
    {
        rethrow_exception(exc);
    }
    catch (const Ice::AlreadyRegisteredException& e)
    {
        return [factory registeredException:toNSString(e.ice_id())
                               kindOfObject:toNSString(e.kindOfObject())
                                   objectId:toNSString(e.id())
                                    message:toNSString(e.what())
                                       file:toNSString(e.ice_file())
                                       line:e.ice_line()];
    }
    catch (const Ice::NotRegisteredException& e)
    {
        return [factory registeredException:toNSString(e.ice_id())
                               kindOfObject:toNSString(e.kindOfObject())
                                   objectId:toNSString(e.id())
                                    message:toNSString(e.what())
                                       file:toNSString(e.ice_file())
                                       line:e.ice_line()];
    }
    catch (const Ice::ConnectionAbortedException& e)
    {
        return [factory connectionClosedException:toNSString(e.ice_id())
                              closedByApplication:e.closedByApplication()
                                          message:toNSString(e.what())
                                             file:toNSString(e.ice_file())
                                             line:e.ice_line()];
    }
    catch (const Ice::ConnectionClosedException& e)
    {
        return [factory connectionClosedException:toNSString(e.ice_id())
                              closedByApplication:e.closedByApplication()
                                          message:toNSString(e.what())
                                             file:toNSString(e.ice_file())
                                             line:e.ice_line()];
    }
    catch (const Ice::RequestFailedException& e)
    {
        return [factory requestFailedException:static_cast<std::uint8_t>(e.replyStatus())
                                          name:toNSString(e.id().name)
                                      category:toNSString(e.id().category)
                                         facet:toNSString(e.facet())
                                     operation:toNSString(e.operation())
                                          file:toNSString(e.ice_file())
                                          line:e.ice_line()];
    }
    catch (const Ice::DispatchException& e)
    {
        return [factory dispatchException:static_cast<std::uint8_t>(e.replyStatus())
                                  message:toNSString(e.what())
                                     file:toNSString(e.ice_file())
                                     line:e.ice_line()];
    }
    catch (const Ice::LocalException& e)
    {
        return [factory localException:toNSString(e.ice_id())
                               message:toNSString(e.what())
                                  file:toNSString(e.ice_file())
                                  line:e.ice_line()];
    }
    catch (const std::exception& e)
    {
        std::string demangled = IceInternal::demangle(typeid(e).name());
        return [factory cxxException:toNSString(demangled) message:toNSString(e.what())];
    }
    catch (...)
    {
        return [factory cxxException:toNSString("unknown C++ exception") message:toNSString("(no message)")];
    }
}

NSObject*
toObjC(const std::shared_ptr<Ice::Endpoint>& endpoint)
{
    return [ICEEndpoint getHandle:endpoint];
}

void
fromObjC(id object, std::shared_ptr<Ice::Endpoint>& endpoint)
{
    ICEEndpoint* endpt = object;
    endpoint = object == [NSNull null] ? nullptr : [endpt endpoint];
}
