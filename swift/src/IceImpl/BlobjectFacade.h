//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "Config.h"

@class ICEConnection;
@class ICEObjectAdapter;
@class ICEDispatchException;

NS_ASSUME_NONNULL_BEGIN

typedef void (^ICEBlobjectResponse)(bool, const void*, long);
typedef void (^ICEBlobjectException)(ICEDispatchException*);

ICEIMPL_API @protocol ICEBlobjectFacade
- (void)facadeInvoke:(ICEObjectAdapter*)adapter
       inEncapsBytes:(void*)inEncapsBytes
       inEncapsCount:(long)inEncapsCount
                 con:(ICEConnection* _Nullable)con
                name:(NSString*)name
            category:(NSString*)category
               facet:(NSString*)facet
           operation:(NSString*)operation
                mode:(uint8_t)mode
             context:(NSDictionary<NSString*, NSString*>*)context
           requestId:(int32_t)requestId
       encodingMajor:(uint8_t)encodingMajor
       encodingMinor:(uint8_t)encodingMinor
            response:(ICEBlobjectResponse)response
           exception:(ICEBlobjectException)exception;
- (void)facadeRemoved;
@end

#ifdef __cplusplus

/// A C++ dispatcher that dispatches requests to a Swift object via an ObjC/Swift "facade".
class SwiftDispatcher final : public Ice::Object
{
public:
    SwiftDispatcher(id<ICEBlobjectFacade> facade) : _facade(facade) {}

    ~SwiftDispatcher() final { [_facade facadeRemoved]; }

    void dispatch(Ice::IncomingRequest&, std::function<void(Ice::OutgoingResponse)> sendResponse) final;

    id<ICEBlobjectFacade> getFacade() const { return _facade; }

private:
    id<ICEBlobjectFacade> _facade;
};

#endif

NS_ASSUME_NONNULL_END
