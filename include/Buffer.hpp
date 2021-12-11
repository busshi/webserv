#pragma once
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>

/**
 * Custom container designed to hold any byte sequence
 * Provides common std::string operations such as find, rfind and sub
 */

template<typename Allocator = std::allocator<unsigned char> >
class Buffer
{
  public:
    typedef unsigned char* iterator;
    typedef const unsigned char* const_iterator;
    typedef size_t size_type;

    static const size_type npos = -1;

  private:
    Allocator _alloc;

    size_type _size, _cap;

    unsigned char* _buf;

    bool _check_resize(size_t newSize)
    {
        if (newSize > _cap) {
            size_t newCap = newSize * 2;
            unsigned char* newData = _alloc.allocate(newCap);

            memcpy(newData, _buf, _size);
            _alloc.deallocate(_buf, _cap);
            _buf = newData;
            _cap = newCap;

            return true;
        }

        return false;
    }

  public:
    Buffer(void)
      : _alloc()
      , _size(0)
      , _cap(10)
      , _buf(_alloc.allocate(_cap))
    {}

    Buffer(const char* data, size_t n)
      : _alloc()
      , _size(n)
      , _cap(n * 2)
      , _buf(_alloc.allocate(_cap))
    {
        memcpy(_buf, data, _size);
    }

    Buffer(const std::string& s)
      : _alloc()
      , _size(s.size())
      , _cap(s.size() * 2)
      , _buf(_alloc.allocate(_cap))
    {
        memcpy(_buf, s.data(), _size);
    }

    Buffer(const_iterator first, const_iterator last)
      : _alloc()
      , _size(last - first)
      , _cap(_size * 2)
      , _buf(_alloc.allocate(_cap))
    {
        memcpy(_buf, first, _size);
    }

    Buffer(const Buffer& other)
      : _alloc()
      , _size(other.size())
      , _cap(other.capacity())
      , _buf(_alloc.allocate(_cap))
    {
        memcpy(_buf, other._buf, _size);
    }

    Buffer& operator=(const Buffer& rhs)
    {
        if (this != &rhs) {
            _alloc = rhs._alloc;
            _check_resize(rhs._size); // perform reallocation if required before
                                      // updating to new size
            _size = rhs._size;
            _cap = rhs._cap;
            memcpy(_buf, rhs._buf, _size);
        }
    }

    ~Buffer(void) { _alloc.deallocate(_buf, _cap); }

    Buffer subbuf(size_type pos, size_type n = npos)
    {
        const_iterator first = begin() + pos;
        const_iterator last =
          first + std::min(static_cast<size_type>(end() - first), n);

        return Buffer(first, last);
    }

    template<typename A>
    size_type find(const Buffer<A>& sub, size_t fromPos = 0)
    {
        for (size_t i = fromPos; i < size() && size() - i >= sub.size(); ++i) {
            if (memcmp(&_buf[i], sub.raw(), sub.size()) == 0) {
                return i;
            }
        }

        return npos;
    }

    template<typename A>
    size_type rfind(const Buffer<A>& sub, size_t uptoPos = 0)
    {
        if (sub.size() > size()) {
            return npos;
        }

        size_t i = size() - sub.size();

        do {
            if (memcmp(&_buf[i], sub.raw(), sub.size()) == 0) {
                return i;
            }
        } while (i-- != uptoPos);

        return npos;
    }

    size_type find(const std::string& s, size_t fromPos = 0)
    {
        for (size_t i = fromPos; i < size() && size() - i >= s.size(); ++i) {
            if (memcmp(&_buf[i], s.data(), s.size()) == 0) {
                return i;
            }
        }

        return npos;
    }

    size_type rfind(const std::string& sub, size_t uptoPos = 0)
    {
        if (sub.size() > size()) {
            return npos;
        }

        size_t i = size() - sub.size();

        do {
            if (memcmp(&_buf[i], sub.data(), sub.size()) == 0) {
                return i;
            }
        } while (i-- != uptoPos);

        return npos;
    }

    Buffer& operator+=(const Buffer& rhs)
    {
        _check_resize(size() + rhs.size());

        memcpy(_buf + size(), rhs._buf, rhs.size());
        _size = size() + rhs.size();

        return *this;
    }

    const void* raw(void) const { return _buf; }

    size_type size(void) const { return _size; }
    size_type capacity(void) const { return _cap; }

    iterator begin(void) { return _buf; }
    const_iterator begin(void) const { return _buf; }

    iterator end(void) { return _buf + _size + 1; }
    const_iterator end(void) const { return _buf + _size + 1; }
};

template<typename Allocator>
Buffer<>
operator+(const Buffer<Allocator>& lhs, const Buffer<Allocator>& rhs)
{
    Buffer<Allocator> tmp;

    return (tmp += lhs) += rhs;
}

template<typename Allocator>
std::ostream&
operator<<(std::ostream& lhs, const Buffer<Allocator>& rhs)
{
    lhs.write(reinterpret_cast<const char*>(rhs.raw()), rhs.size());

    return lhs;
}
