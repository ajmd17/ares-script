#include "Mesh.h"
#include "Vertex.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace arui {
Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned short> &indices)
  : vertices(vertices), indices(indices) {
  glGenBuffers(1, &vboID);
  glBindBuffer(GL_ARRAY_BUFFER, vboID);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0].x, GL_STATIC_DRAW);

  glGenBuffers(1, &iboID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * indices.size(), &indices[0], GL_STATIC_DRAW);
}

Mesh::Mesh(const Mesh &other) {
  vertices = other.vertices;
  indices = other.indices;

  glGenBuffers(1, &vboID);
  glBindBuffer(GL_ARRAY_BUFFER, vboID);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0].x, GL_STATIC_DRAW);

  glGenBuffers(1, &iboID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * indices.size(), &indices[0], GL_STATIC_DRAW);
}

Mesh::~Mesh() {
  glDeleteBuffers(1, &vboID);
  glDeleteBuffers(1, &iboID);
}

void Mesh::Draw() {
  glBindBuffer(GL_ARRAY_BUFFER, vboID);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(0));   //The starting point of the VBO, for the vertices
  glEnableClientState(GL_NORMAL_ARRAY);
  glNormalPointer(GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(12));   //The starting point of normals, 12 bytes away
  glClientActiveTexture(GL_TEXTURE0);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(24));   //The starting point of texcoords, 24 bytes away
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboID);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
}
}