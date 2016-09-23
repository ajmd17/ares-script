#ifndef ASTREAM_H
#define ASTREAM_H

#include <common/bytecodes.h>

#include <fstream>
#include <string>
#include <sstream>

namespace avm {
class ByteStream {
public:
    ByteStream(char *buffer, size_t max);

    inline void ReadBytes(char *ptr, size_t size)
    {
        for (size_t i = 0; i < size; i++) {
            ptr[i] = buffer[pos++];
        }
    }

    template <typename T>
    inline void Read(T *ptr, size_t size = sizeof(T))
    {
        ReadBytes(reinterpret_cast<char*>(ptr), size);
    }

    inline size_t Position() const { return pos; }
    inline size_t Max() const { return max; }
    inline void Seek(size_t address) { pos = address; }
    inline void Skip(size_t amount) { pos += amount; }
    inline bool Eof() const { return pos >= max; }

private:
    char *buffer;
    size_t pos;
    size_t max;
};
} // namespace avm

#endif