#include <stdint.h>
#include <vector>

class BinBuffer
{
    std::vector<uint8_t> _buf;

  public:
    BinBuffer(const char* data = "");
    ~BinBuffer(void);

    uint8_t* str(void) const;
};
