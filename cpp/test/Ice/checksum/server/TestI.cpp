// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/SliceChecksums.h>
#include <TestI.h>

ChecksumI::ChecksumI()
{
}

Ice::SliceChecksumDict
ChecksumI::getSliceChecksums(const Ice::Current&) const
{
    return Ice::sliceChecksums();
}

void
ChecksumI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
