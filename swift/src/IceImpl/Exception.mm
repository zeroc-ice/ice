//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "Exception.h"
#import "Convert.h"

@implementation ICEDispatchException

- (instancetype)initWithCppExceptionPtr:(std::exception_ptr)cppExceptionPtr
{
    assert(cppExceptionPtr);
    self = [super init];
    if (!self)
    {
        return nil;
    }

    _cppExceptionPtr = cppExceptionPtr;

    return self;
}

+ (instancetype)objectNotExistException:(NSString*)name
                               category:(NSString*)category
                                  facet:(NSString*)facet
                              operation:(NSString*)operation
                                   file:(NSString*)file
                                   line:(int32_t)line
{
    auto epr = std::make_exception_ptr(Ice::ObjectNotExistException{
        fromNSString(file).c_str(),
        line,
        Ice::Identity{fromNSString(name), fromNSString(category)},
        fromNSString(facet),
        fromNSString(operation)});

    return [[ICEDispatchException alloc] initWithCppExceptionPtr:epr];
}

+ (instancetype)facetNotExistException:(NSString*)name
                              category:(NSString*)category
                                 facet:(NSString*)facet
                             operation:(NSString*)operation
                                  file:(NSString*)file
                                  line:(int32_t)line
{
    auto epr = std::make_exception_ptr(Ice::FacetNotExistException{
        fromNSString(file).c_str(),
        line,
        Ice::Identity{fromNSString(name), fromNSString(category)},
        fromNSString(facet),
        fromNSString(operation)});

    return [[ICEDispatchException alloc] initWithCppExceptionPtr:epr];
}

+ (instancetype)operationNotExistException:(NSString*)name
                                  category:(NSString*)category
                                     facet:(NSString*)facet
                                 operation:(NSString*)operation
                                      file:(NSString*)file
                                      line:(int32_t)line
{
    auto epr = std::make_exception_ptr(Ice::OperationNotExistException{
        fromNSString(file).c_str(),
        line,
        Ice::Identity{fromNSString(name), fromNSString(category)},
        fromNSString(facet),
        fromNSString(operation)});

    return [[ICEDispatchException alloc] initWithCppExceptionPtr:epr];
}

+ (instancetype)unknownLocalException:(NSString*)unknown file:(NSString*)file line:(int32_t)line
{
    auto epr =
        std::make_exception_ptr(Ice::UnknownLocalException{fromNSString(file).c_str(), line, fromNSString(unknown)});

    return [[ICEDispatchException alloc] initWithCppExceptionPtr:epr];
}

+ (instancetype)unknownUserException:(NSString*)unknown file:(NSString*)file line:(int32_t)line
{
    auto epr =
        std::make_exception_ptr(Ice::UnknownUserException{fromNSString(file).c_str(), line, fromNSString(unknown)});

    return [[ICEDispatchException alloc] initWithCppExceptionPtr:epr];
}

+ (instancetype)unknownException:(NSString*)unknown file:(NSString*)file line:(int32_t)line
{
    auto epr = std::make_exception_ptr(Ice::UnknownException{fromNSString(file).c_str(), line, fromNSString(unknown)});

    return [[ICEDispatchException alloc] initWithCppExceptionPtr:epr];
}

@end
