//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_MY_BYTE_SEQ_H
#define TEST_MY_BYTE_SEQ_H

#include "Ice/Config.h"
#include <cstddef>

class MyByteSeq
{
public:
    typedef std::byte* iterator;
    typedef std::byte* const_iterator;

    typedef std::byte value_type;

    MyByteSeq();
    MyByteSeq(size_t);
    MyByteSeq(const MyByteSeq&);
    MyByteSeq(MyByteSeq&&) noexcept;
    ~MyByteSeq();

    size_t size() const;
    void swap(MyByteSeq&);
    const_iterator begin() const;
    const_iterator end() const;

    MyByteSeq& operator=(const MyByteSeq&);
    MyByteSeq& operator=(MyByteSeq&&);

    bool operator==(const MyByteSeq&) const;

private:
    size_t _size;
    std::byte* _data;
};

#endif
