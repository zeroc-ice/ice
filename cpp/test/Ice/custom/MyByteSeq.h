//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_MY_BYTE_SEQ_H
#define TEST_MY_BYTE_SEQ_H

#include <Ice/Config.h>

class MyByteSeq
{
public:

    typedef std::uint8_t* iterator;
    typedef std::uint8_t* const_iterator;

    typedef std::uint8_t value_type;

    MyByteSeq();
    MyByteSeq(size_t);
    MyByteSeq(const MyByteSeq&);
    ~MyByteSeq();

    size_t size() const;
    void swap(MyByteSeq&);
    const_iterator begin() const;
    const_iterator end() const;
    void operator=(const MyByteSeq&);
    bool operator==(const MyByteSeq&) const;

private:

    size_t _size;
    std::uint8_t* _data;
};

#endif
