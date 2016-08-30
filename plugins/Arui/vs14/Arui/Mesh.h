#ifndef ARUI_MESH_H
#define ARUI_MESH_H
#include "Vertex.h"
#include <vector>

#define BUFFER_OFFSET(i) ((void*)(i))

namespace arui {
class Mesh {
public:
  Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned short> &indices);
  Mesh(const Mesh &other);
  ~Mesh();

  void Draw();

private:
  unsigned int vboID, iboID;
  std::vector<Vertex> vertices;
  std::vector<unsigned short> indices;
};
}
#endif