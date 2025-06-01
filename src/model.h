#pragma once

struct Vertex
{
    double x;
    double y;
    double z;
};

struct Face
{
    int indicies[4096];

    int ctVertex;
};

struct Model
{
    struct Vertex vertexArr[4096];
    struct Face   faceArr[4096];

    int ctVertex;
    int ctFace;
};

void initModel(struct Model* model);

void debug_printVertex(const struct Vertex* vertex);
void debug_printFace(const struct Face* face, const struct Model* model);
void debug_printModel(const struct Model* model);
void debug_printModelStats(const struct Model* model);