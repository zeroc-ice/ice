//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice/Object.h>

#include <Ice/Object.h>

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
typedef IceUtil::Handle<ServantWrapper> ServantWrapperPtr;

};

@interface ICEObject (ICEInternal)
-(Ice::Object*) iceObject;
@end

@interface ICEServantWrapper : ICEObject
{
    Ice::Object* object_;
}
+(id) servantWrapperWithCxxObjectNoAutoRelease:(Ice::Object*)arg;
@end
