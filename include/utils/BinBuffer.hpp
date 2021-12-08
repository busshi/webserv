#pragma once
#include <fstream>
#include <stdint.h>
#include <vector>

class BinBuffer
{
    std::vector<uint8_t> _data;
    std::vector<uint8_t>::size_type _index;

  public:
    BinBuffer(void);
    BinBuffer(const BinBuffer& other);
    ~BinBuffer(void);

    BinBuffer& operator=(const BinBuffer& rhs);
    BinBuffer operator+(const BinBuffer& rhs);

    BinBuffer& append(const char* data, int n);
    BinBuffer& append(std::ifstream& ifs, int bufsize = 1024);
    BinBuffer& append(const std::string& s);
    void clear(void);
    const uint8_t* data(void) const;
    size_t size(void) const;

    std::pair<const uint8_t*, size_t> getbuf(void) const;
    BinBuffer& consume(size_t bytesToConsume);

    bool isConsumed(void) const;
};
