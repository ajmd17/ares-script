#include <detail/byte_stream.h>
#include <sstream>
#include <cstdlib>

namespace avm {
FileByteStream::FileByteStream(const std::string &filepath, std::streampos begin) {
  file = new std::ifstream(filepath, std::ios::in |
    std::ios::binary |
    std::ios::ate);

  max_pos = file->tellg();
  file->seekg(begin);

  char magic[32] = { '\0' };
  file->read(magic, strlen(ARES_MAGIC));
  if (strcmp(magic, ARES_MAGIC) != 0) {
    // not a valid bytecode file
  }
  // read version
  char version[ARES_VERSION_LEN] = { '\0' };
  file->read(version, ARES_VERSION_LEN);

  pos = file->tellg();
}

FileByteStream::~FileByteStream() {
  delete file;
}

size_t FileByteStream::Position() const {
  return pos;
}

size_t FileByteStream::Max() const {
  return max_pos;
}

void FileByteStream::ReadBytes(char *ptr, unsigned size) {
  file->read(ptr, size);
  pos += size;
}

void FileByteStream::Skip(unsigned amount) {
  file->seekg(pos += amount);
}

void FileByteStream::Seek(unsigned long address) {
  file->seekg(pos = address);
}

bool FileByteStream::Eof() const {
  return file->eof();
}

MemoryByteStream::MemoryByteStream(const std::stringstream &ss) {
  /*for (auto &&it : stream.instructions) {
    for (auto &&it2 : it.data) {
      data.insert(data.end(), it2.begin(), it2.end());
    }
  }*/

  /*for (size_t i = 0; i < ins_stream.instructions.size(); i++) {
    std::vector<std::vector<char>> &ins = ins_stream.instructions[i].data;
    for (int j = ins.size() - 1; j >= 0; j--) {
      stream.write(&ins[j][0], ins[j][0].size());
    }
  }*/

  stream << ss.rdbuf(); // copy buffer

  stream.seekg(0, std::ios::end);
  max_pos = stream.tellg();
  stream.seekg(0);
  pos = stream.tellg();
 // std::cout << "pos: " << pos << ", max_pos: " << max_pos << "\n\n";
 // std::system("pause");
}

size_t MemoryByteStream::Position() const {
  return pos;
}

size_t MemoryByteStream::Max() const {
  return max_pos;
}

void MemoryByteStream::ReadBytes(char *ptr, unsigned size) {
  /*size_t counter = 0;
  while (counter < size) {
    *(ptr[counter]) = stream[index][sub_index];
    ++sub_index;
    ++counter;
    if (sub_index == stream[index].size()) {
      sub_index = 0;
      index++;
    }
  }*/


  /*for (size_t i = 0; i < size; i++) {
    ptr[i] = data[pos];
    ++pos;
  }*/

  stream.read(ptr, size);
  pos += size;
}

void MemoryByteStream::Skip(unsigned amount) {
  /*size_t counter = 0;
  while (counter < amount) {
    ++sub_index;
    ++counter;
    if (sub_index == stream[index].size()) {
      sub_index = 0;
      index++;
    }
  }*/
  stream.seekg(pos += amount);
}

void MemoryByteStream::Seek(unsigned long address) {
  stream.seekg(pos = address);
}

bool MemoryByteStream::Eof() const {
  return stream.eof();
}
} // namespace avm