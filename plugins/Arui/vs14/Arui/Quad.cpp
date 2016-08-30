#include "Quad.h"

namespace arui {
Mesh *Quad::CreateQuad() {
  std::vector<Vertex> vertices;
  vertices.resize(4);

  //VERTEX 0
  vertices[0].x = 0.0;
  vertices[0].y = 0.0;
  vertices[0].z = 0.0;
  vertices[0].nx = 0.0;
  vertices[0].ny = 0.0;
  vertices[0].nz = 1.0;
  vertices[0].s = 0.0;
  vertices[0].t = 0.0;

  //VERTEX 3
  vertices[3].x = 0.0;
  vertices[3].y = 1.0;
  vertices[3].z = 0.0;
  vertices[3].nx = 0.0;
  vertices[3].ny = 0.0;
  vertices[3].nz = 1.0;
  vertices[3].s = 0.0;
  vertices[3].t = 1.0;

  //VERTEX 2
  vertices[2].x = 1.0;
  vertices[2].y = 1.0;
  vertices[2].z = 0.0;
  vertices[2].nx = 0.0;
  vertices[2].ny = 0.0;
  vertices[2].nz = 1.0;
  vertices[2].s = 1.0;
  vertices[2].t = 1.0;

  //VERTEX 1
  vertices[1].x = 1.0;
  vertices[1].y = 0.0;
  vertices[1].z = 0.0;
  vertices[1].nx = 0.0;
  vertices[1].ny = 0.0;
  vertices[1].nz = 1.0;
  vertices[1].s = 1.0;
  vertices[1].t = 0.0;

  std::vector<unsigned short> indices = {
    0, 1, 2, 
    0, 2, 3
  };

  return new Mesh(vertices, indices);
}
}