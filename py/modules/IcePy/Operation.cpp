// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Operation.h>
#include <Types.h>
#include <Util.h>

using namespace std;
using namespace IcePy;

IcePy::Operation::~Operation()
{
}

IcePy::OperationPtr
IcePy::getOperation(const string& classId, const string& name)
{
    OperationPtr result;

    ClassInfoPtr info = ClassInfoPtr::dynamicCast(getTypeInfo(classId));
    if(info)
    {
        result = info->findOperation(name);
        if(!result)
        {
            //
            // Look for the operation in the description of Ice.Object.
            //
            info = ClassInfoPtr::dynamicCast(getTypeInfo("::Ice::Object"));
            assert(info);
            result = info->findOperation(name);
        }
    }

    return result;
}
