#include "utils/BinBuffer.hpp"

BinBuffer::BinBuffer(void)
  : _index(0)
{}

BinBuffer::BinBuffer(const BinBuffer& other)
{
    *this = other;
}

BinBuffer&
BinBuffer::operator=(const BinBuffer& rhs)
{
    if (this != &rhs) {
        _data = rhs._data;
    }

    return *this;
}

BinBuffer
BinBuffer::operator+(const BinBuffer& rhs)
{
    BinBuffer b(*this);

    b._data.insert(b._data.end(), rhs._data.begin(), rhs._data.end());

    return b;
}

BinBuffer::~BinBuffer(void) {}

void
BinBuffer::clear(void)
{
    _data.clear();
}

size_t
BinBuffer::size(void) const
{
    return _data.size();
}

const uint8_t*
BinBuffer::data(void) const
{
    return _data.empty() ? 0 : &_data[0];
}

BinBuffer&
BinBuffer::append(const char* data, int n)
{
    _data.insert(_data.end(), data, data + n);

    return *this;
}

BinBuffer&
BinBuffer::append(const std::string& s)
{
    _data.insert(_data.end(), s.begin(), s.end());

    return *this;
}

BinBuffer&
BinBuffer::append(std::ifstream& ifs, int bufsize)
{
    char* buf = new char[bufsize]; // VLAs do not exist in C++

    while (ifs) {
        ifs.read(buf, bufsize);
        append(buf, ifs.gcount());
    }

    delete[] buf;

    return *this;
}

std::pair<const uint8_t*, size_t>
BinBuffer::getbuf(void) const
{
    return std::make_pair(&_data[_index], size() - _index);
}

BinBuffer&
BinBuffer::consume(size_t bytesToConsume)
{
    _index += bytesToConsume;

    return *this;
}

bool
BinBuffer::isConsumed(void) const
{
    return _index >= size();
}
