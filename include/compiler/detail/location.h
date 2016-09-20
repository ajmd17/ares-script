#ifndef LOCATION_H
#define LOCATION_H

#include <string>

namespace avm {
struct SourceLocation {
    int line;
    int column;
    std::string file;

    SourceLocation()
        : line(-1),
        column(-1),
        file("")
    {
    }

    SourceLocation(const SourceLocation &other)
        : line(other.line),
        column(other.column),
        file(other.file)
    {
    }

    SourceLocation(int line, int column, const std::string &file)
        : line(line),
        column(column),
        file(file)
    {
    }
};
} // namespace avm

#endif