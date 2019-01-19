//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class ChecksumI : public Test::Checksum
{
public:

    ChecksumI();

    virtual Ice::SliceChecksumDict getSliceChecksums(const Ice::Current&) const;

    virtual void shutdown(const Ice::Current&);
};

#endif
