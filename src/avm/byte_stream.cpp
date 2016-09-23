#include <detail/byte_stream.h>

#include <sstream>
#include <cstdlib>

namespace avm {

ByteStream::ByteStream(char *buffer, size_t max)
    : buffer(buffer),
      pos(0),
      max(max)
{
    // read magic bytes
    char magic[ARES_MAGIC_LEN] = { '\0' };
    ReadBytes(magic, ARES_MAGIC_LEN);
    // read version
    char version[ARES_VERSION_LEN] = { '\0' };
    ReadBytes(version, ARES_VERSION_LEN);
}
} // namespace avm