#ifndef BYTECODES_H
#define BYTECODES_H

#include <vector>
#include <memory>
#include <string>
#include <cstring>

#include <common/instructions.h>

namespace avm {
template <class...Ts>
struct Instruction {
  std::vector<std::vector<char>> data;

  void Write(std::ostream &os) {
    int index = data.size() - 1;
    while (index >= 0) {
      os.write(&data[index][0], data[index].size());
      --index;
    }
  }

protected:
  void Accept(const char *str) {
    size_t length = std::strlen(str) + 1;
    std::vector<char> str_operand(length);
    std::memcpy(&str_operand[0], str, length);
    data.push_back(str_operand);
  }

  void Accept(int8_t i) {
    std::vector<char> operand(sizeof(i));
    std::memcpy(&operand[0], &i, sizeof(i));
    data.push_back(operand);
  }

  void Accept(int16_t i) {
    std::vector<char> operand(sizeof(i));
    std::memcpy(&operand[0], &i, sizeof(i));
    data.push_back(operand);
  }

  void Accept(int32_t i) {
    std::vector<char> operand(sizeof(i));
    std::memcpy(&operand[0], &i, sizeof(i));
    data.push_back(operand);
  }

  void Accept(int64_t i) {
    std::vector<char> operand(sizeof(i));
    std::memcpy(&operand[0], &i, sizeof(i));
    data.push_back(operand);
  }

  void Accept(double d) {
    std::vector<char> operand(sizeof(d));
    std::memcpy(&operand[0], &d, sizeof(d));
    data.push_back(operand);
  }

  void Accept(uint8_t u) {
    std::vector<char> operand(sizeof(u));
    std::memcpy(&operand[0], &u, sizeof(u));
    data.push_back(operand);
  }

  void Accept(uint16_t u) {
    std::vector<char> operand(sizeof(u));
    std::memcpy(&operand[0], &u, sizeof(u));
    data.push_back(operand);
  }

  void Accept(uint32_t u) {
    std::vector<char> operand(sizeof(u));
    std::memcpy(&operand[0], &u, sizeof(u));
    data.push_back(operand);
  }

  void Accept(uint64_t u) {
    std::vector<char> operand(sizeof(u));
    std::memcpy(&operand[0], &u, sizeof(u));
    data.push_back(operand);
  }

private:
  size_t pos = 0;
};

template <class T, class... Ts>
struct Instruction<T, Ts...> : Instruction<Ts...> {
  Instruction(T t, Ts...ts) : Instruction<Ts...>(ts...) {
    this->Accept(t);
  }
};

struct InstructionStream {
  std::vector<Instruction<>> instructions;

  InstructionStream &operator<<(Instruction<> command) {
    instructions.push_back(command);
    return *this;
  }
};
} // namespace avm

#endif