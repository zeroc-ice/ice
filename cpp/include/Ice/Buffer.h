// Copyright (c) ZeroC, Inc.

#ifndef ICE_BUFFER_H
#define ICE_BUFFER_H

#include "Config.h"

#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

namespace IceInternal
{
    class Buffer
    {
    public:
        Buffer() : i(b.begin()) {}
        Buffer(const std::byte* beg, const std::byte* end) : b(beg, end), i(b.begin()) {}
        Buffer(const std::vector<std::byte>& v) : b(v), i(b.begin()) {}
        Buffer(Buffer& o, bool adopt) : b(o.b, adopt), i(b.begin()) {}

        Buffer(Buffer&& other) noexcept : b(std::move(other.b)), i(other.i) { other.i = other.b.begin(); }

        Buffer& operator=(Buffer&& other) noexcept
        {
            if (this != &other)
            {
                b = std::move(other.b);
                i = other.i;
                other.i = other.b.begin();
            }
            return *this;
        }

        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        void swapBuffer(Buffer& other) noexcept
        {
            b.swap(other.b);
            std::swap(i, other.i);
        }

        class ICE_API Container
        {
        public:
            //
            // Standard vector-like operations.
            //
            using value_type = std::byte;
            using iterator = std::byte*;
            using const_iterator = const std::byte*;
            using reference = std::byte&;
            using const_reference = const std::byte&;
            using pointer = std::byte*;
            using size_type = size_t;

            Container() noexcept;
            Container(const_iterator, const_iterator) noexcept;
            Container(const std::vector<value_type>&) noexcept;
            Container(Container&, bool) noexcept;

            Container(Container&&) noexcept;
            Container& operator=(Container&&) noexcept;

            Container(const Container&) = delete;
            Container& operator=(const Container&) = delete;

            ~Container();

            iterator begin() { return _buf; }

            [[nodiscard]] const_iterator begin() const { return _buf; }

            iterator end() { return _buf + _size; }

            [[nodiscard]] const_iterator end() const { return _buf + _size; }

            [[nodiscard]] size_type size() const { return _size; }

            [[nodiscard]] bool empty() const { return !_size; }

            [[nodiscard]] bool ownsMemory() const noexcept { return _owned; }

            void swap(Container&) noexcept;

            void clear();

            void resize(size_type n) // Inlined for performance reasons.
            {
                if (n == 0)
                {
                    clear();
                }
                else if (n > _capacity)
                {
                    reserve(n);
                }
                _size = n;
            }

            void reset()
            {
                if (_size > 0 && _size * 2 < _capacity)
                {
                    //
                    // If the current buffer size is smaller than the
                    // buffer capacity, we shrink the buffer memory to the
                    // current size. This is to avoid holding onto too much
                    // memory if it's not needed anymore.
                    //
                    if (++_shrinkCounter > 2)
                    {
                        reserve(_size);
                        _shrinkCounter = 0;
                    }
                }
                else
                {
                    _shrinkCounter = 0;
                }
                _size = 0;
            }

            void push_back(value_type v)
            {
                resize(_size + 1);
                _buf[_size - 1] = v;
            }

            reference operator[](size_type n)
            {
                assert(n < _size);
                return _buf[n];
            }

            const_reference operator[](size_type n) const
            {
                assert(n < _size);
                return _buf[n];
            }

        private:
            void reserve(size_type);

            pointer _buf;
            size_type _size;
            size_type _capacity;
            int _shrinkCounter;
            bool _owned;
        };

        Container b;
        Container::iterator i;
    };
}

#endif
