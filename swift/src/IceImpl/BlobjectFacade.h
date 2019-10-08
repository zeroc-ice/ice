//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "Config.h"

@class ICEConnection;
@class ICEObjectAdapter;
@class ICERuntimeException;

NS_ASSUME_NONNULL_BEGIN

typedef void (^ICEBlobjectResponse) (bool, NSData*);
typedef void (^ICEBlobjectException) (ICERuntimeException*);

ICEIMPL_API @protocol ICEBlobjectFacade
-(void) facadeInvoke:(ICEObjectAdapter*)adapter
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
-(void) facadeRemoved;
@end

#ifdef __cplusplus

class BlobjectFacade : public Ice::BlobjectArrayAsync
{
public:

    BlobjectFacade(id<ICEBlobjectFacade> facade): _facade(facade)
    {
    }

    ~BlobjectFacade()
    {
        [_facade facadeRemoved];
    }

    virtual void
    ice_invokeAsync(std::pair<const Byte*, const Byte*> inEncaps,
                    std::function<void(bool, const std::pair<const Byte*, const Byte*>&)> response,
                    std::function<void(std::exception_ptr)> error,
                    const Ice::Current& current);

    id<ICEBlobjectFacade> getFacade() const
    {
        return _facade;
    }

private:
    id<ICEBlobjectFacade> _facade;
};

#endif

NS_ASSUME_NONNULL_END
