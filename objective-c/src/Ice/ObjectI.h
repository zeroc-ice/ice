//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice/Object.h>

#include <Ice/Object.h>
#include <memory>

//
// Forward declarations.
//
@class ICECurrent;
@class ICEInputStream;
@class ICEOutputStream;

namespace IceObjC
{

class ServantWrapper : public virtual Ice::Object
{
public:

    virtual ~ServantWrapper() { }
    virtual ICEObject* getServant() = 0;
};

};

@interface ICEObject (ICEInternal)
-(std::shared_ptr<Ice::Object>) iceObject;
@end

@interface ICEServantWrapper : ICEObject
{
    std::shared_ptr<Ice::Object> object_;
}
+(id) servantWrapperWithCxxObjectNoAutoRelease:(const std::shared_ptr<Ice::Object>&)arg;
@end
