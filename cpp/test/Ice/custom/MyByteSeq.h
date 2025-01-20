// Copyright (c) ZeroC, Inc.

#ifndef TEST_MY_BYTE_SEQ_H
#define TEST_MY_BYTE_SEQ_H

#include "Ice/Config.h"
#include <cstddef>

class MyByteSeq
{
public:
    using iterator = std::byte*;
    using const_iterator = std::byte*;

    using value_type = std::byte;

    MyByteSeq();
    MyByteSeq(size_t);
    MyByteSeq(const MyByteSeq&);
    MyByteSeq(MyByteSeq&&) noexcept;
    ~MyByteSeq();

    [[nodiscard]] size_t size() const;
    void swap(MyByteSeq&) noexcept;
    [[nodiscard]] const_iterator begin() const;
    [[nodiscard]] const_iterator end() const;

    MyByteSeq& operator=(const MyByteSeq&);
    MyByteSeq& operator=(MyByteSeq&&) noexcept;

    bool operator==(const MyByteSeq&) const;

private:
    size_t _size;
    std::byte* _data;
};

#endif
