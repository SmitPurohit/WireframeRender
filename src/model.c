#include "model.h"

#include <stdio.h>

void initModel(struct Model* model)
{
    model->ctFace = 0;
    model->ctVertex = 0;
}

void debug_printVertex(const struct Vertex* vertex)
{
    printf("x: %f y: %f z: %f\n", vertex->x, vertex->y, vertex->z);
}

void debug_printFace(const struct Face* face, const struct Model* model)
{
    for(int v = 0; v < face->ctVertex; ++v)
    {
        printf("\n\t");
        printf("idx: %i", face->indicies[v]);
        int idx = face->indicies[v];

        const struct Vertex* vertex;

        if(idx > 0)
        {
            vertex = &model->vertexArr[idx];
        }
        else
        {                   
            vertex = &model->vertexArr[model->ctVertex + idx];
        }
        debug_printVertex(vertex);
    }
}

void debug_printModel(const struct Model* model)
{
    for(int v = 0; v < model->ctVertex; ++v)
    {
        printf("Vertex %i: ", v);

        debug_printVertex(&(model->vertexArr[v]));
    }

    for(int f = 0; f < model->ctFace; ++f)
    {
        printf("Face %i: ", f);

        debug_printFace(&(model->faceArr[f]), model);
    }
}

void debug_printModelStats(const struct Model* model)
{
    printf("Vertex Count: %i Face Count: %i", model->ctVertex, model->ctFace);

}