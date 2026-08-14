// Copyright (c) ZeroC, Inc.

#ifndef TEST_KEY_H
#define TEST_KEY_H

#include "DataStorm/DataStorm.h"

#include <stdexcept>
#include <string>

// A key type whose Decoder rejects one of the values. The writer encodes "poison" without complaint, so the value
// reaches the peer and only fails when the peer decodes it. The same type doubles as a sample filter criteria, so a
// filter can carry a criteria the writer cannot decode.
struct TestKey
{
    std::string name;

    bool operator<(const TestKey& other) const { return name < other.name; }

    bool operator==(const TestKey& other) const { return name == other.name; }
};

namespace DataStorm
{
    template<> struct Encoder<TestKey>
    {
        static Ice::ByteSeq encode(const Ice::CommunicatorPtr&, const TestKey& key)
        {
            Ice::ByteSeq bytes;
            bytes.reserve(key.name.size());
            for (char c : key.name)
            {
                bytes.push_back(static_cast<std::byte>(c));
            }
            return bytes;
        }
    };

    template<> struct Decoder<TestKey>
    {
        static TestKey decode(const Ice::CommunicatorPtr&, const Ice::ByteSeq& data)
        {
            std::string name;
            name.reserve(data.size());
            for (std::byte b : data)
            {
                name += static_cast<char>(b);
            }

            if (name == "poison")
            {
                throw std::runtime_error("undecodable key");
            }
            return TestKey{name};
        }
    };
}

#endif
