#pragma once

#include "model.h"

#include <stdio.h>
#include <stdlib.h>

int parseObj(const char *path, struct Model* model)
{
    FILE* f = fopen(path, "r");

    if (!f)
    {
        printf("Error reading file %s", path);
        return -1;
    }

    char line[256];

    int vertexInd = 0;
    int faceInd = 0;

    while(fgets(line, sizeof line, f))
    {
        //Vertex
        if(line[0] == 'v' && line[1] == ' ')
        {
            double x, y, z;
            if (sscanf(line + 2, "%lf%lf%lf", &x, &y, &z) == 3)
            {
                struct Vertex* vertex = &(model->vertexArr[vertexInd++]);

                vertex->x = x;
                vertex->y = y;
                vertex->z = z;

                model->ctVertex++;
            }         
            
        }
        else if(line[0] == 'f' && line[1] == ' ')
        {
            //Note that f is an index into vertexArr
            // If > 0, index into vertexArr normally
            // If < 0, index into vertexArr backwards
            char *p = line + 1;

            struct Face* face = &(model->faceArr[faceInd++]);
            face->ctVertex = 0;

            while (*p)
            {
                
                if(*p == ' ' || *p == '\t')
                {
                    p++;
                    continue;
                }

                if (*p == '\0' || *p == '\n' || *p == '#')
                {
                    break;
                }
                
                //printf("%c", *p);
                /* read integer index, ignore optional texture/normal suffix */
                int idx = strtol(p, &p, 10);

                face->indicies[face->ctVertex++] = idx;
                
                while (*p && *p != ' ' && *p != '\t' && *p != '\n')
                {
                    ++p;
                }
                
            }

            model->ctFace++;
        }
    }

    fclose(f);

    return 0;
}
