#ifndef ASTREAM_H
#define ASTREAM_H

#include <fstream>
#include <string>
#include <sstream>
#include <common/bytecodes.h>

namespace avm {
class ByteStream {
public:
    virtual ~ByteStream() = default;

    template <typename T>
    void Read(T *ptr, unsigned size = sizeof(T))
    {
        ReadBytes(reinterpret_cast<char*>(ptr), size);
    }

    virtual size_t Position() const = 0;
    virtual size_t Max() const = 0;
    virtual void Skip(unsigned amount) = 0;
    virtual void Seek(unsigned long address) = 0;
    virtual bool Eof() const = 0;

protected:
    virtual void ReadBytes(char *ptr, unsigned size) = 0;
};

class FileByteStream : public ByteStream {
public:
    FileByteStream(const std::string &filepath, std::streampos begin = 0);
    ~FileByteStream();

    size_t Position() const;
    size_t Max() const;

    void ReadBytes(char *ptr, unsigned size);
    void Skip(unsigned amount);
    void Seek(unsigned long address);
    bool Eof() const;

private:
    std::istream *file;
    std::streampos pos;
    std::streampos max_pos;
};

class MemoryByteStream : public ByteStream {
public:
    MemoryByteStream(const std::stringstream &ss);

    size_t Position() const;
    size_t Max() const;

    void ReadBytes(char *ptr, unsigned size);
    void Skip(unsigned amount);
    void Seek(unsigned long address);
    bool Eof() const;

private:
    size_t pos;
    size_t max_pos;
    std::stringstream stream;
};
} // namespace avm

#endif