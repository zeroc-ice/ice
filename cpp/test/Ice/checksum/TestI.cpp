// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
