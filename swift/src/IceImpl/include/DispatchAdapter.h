// Copyright (c) ZeroC, Inc.
#import "Config.h"

@class ICEConnection;
@class ICEObjectAdapter;

NS_ASSUME_NONNULL_BEGIN

// Provides the reply status, the exception ID, the exception message, and bytes of the reply message (including the
// header).
typedef void (^ICEOutgoingResponse)(uint8_t, NSString* _Nullable, NSString* _Nullable, const void*, long)
    NS_SWIFT_SENDABLE;

// The implementation must call the completion handler exactly once.
ICEIMPL_API @protocol ICEDispatchAdapter
- (void)dispatch:(ICEObjectAdapter*)adapter
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
    outgoingResponseHandler:(ICEOutgoingResponse)outgoingResponseHandler;

- (void)complete;
@end

#ifdef __cplusplus

/// A C++ dispatcher that dispatches requests to a Swift object via an ObjC/Swift dispatch adapter.
class CppDispatcher final : public Ice::Object
{
public:
    CppDispatcher(id<ICEDispatchAdapter> dispatchAdapter) : _dispatchAdapter(dispatchAdapter) {}

    ~CppDispatcher() final { [_dispatchAdapter complete]; }

    void dispatch(Ice::IncomingRequest&, std::function<void(Ice::OutgoingResponse)> sendResponse) final;

    id<ICEDispatchAdapter> dispatchAdapter() const noexcept { return _dispatchAdapter; }

private:
    id<ICEDispatchAdapter> _dispatchAdapter;
};

#endif

NS_ASSUME_NONNULL_END
