#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>

#include <math.h>
#include <stdlib.h>

#include "model.h"
#include "parse.h"

#define WIDTH 640
#define HEIGHT 480

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

static struct Model model;

static uint8_t xzState = 2;
static uint8_t yzState = 0;

static uint8_t vertAxis = 0x2; //010
static uint8_t horizAxis = 0x4; //100

static int light[3] = {0,0,-1};

static bool first = true;
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define MAX(a, b) ((a) > (b) ? (a) : (b))


double luminosity(struct Vertex* vertexArr[3])
{
    // 1) point2 -> point0 cross point1 -> point0
    double vector20[3] = {vertexArr[2]->x - vertexArr[0]->x, vertexArr[2]->y - vertexArr[0]->y, vertexArr[2]->z - vertexArr[0]->z};
    double vector10[3] = {vertexArr[1]->x - vertexArr[0]->x, vertexArr[1]->y - vertexArr[0]->y, vertexArr[1]->z - vertexArr[0]->z};
    double crossProduct[3] = {
        vector20[1] * vector10[2] - vector20[2] * vector10[1],
        vector20[2] * vector10[0] - vector20[0] * vector10[2],
        vector20[0] * vector10[1] - vector20[1] * vector10[0]
    };

    // 2) normalize the cross product
    double length = sqrt(crossProduct[0] * crossProduct[0] + crossProduct[1] * crossProduct[1] + crossProduct[2] * crossProduct[2]);
        crossProduct[0] /= length;
        crossProduct[1] /= length;
        crossProduct[2] /= length;
    // 3) scalar product with light vector

    return crossProduct[0] * light[0] + crossProduct[1] * light[1] + crossProduct[2] * light[2];
}

// We don't care about the area, just if this is negative or positive
int edgeFunc(int x0, int y0, int x1, int y1, int x2, int y2)
{
    return ( (x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0) );
}

//https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm

void drawLinePoints(int x0, int y0, int x1, int y1)
{
    int diffX = abs(x1 - x0);
    int stepX = x0 < x1 ? 1 : -1;

    int diffY = -1 * abs(y1 - y0);
    int stepY = y0 < y1 ? 1 : -1;

    int error = diffX + diffY;

    int x = x0;
    int y = y0;

    while(true)
    {
        SDL_RenderPoint(renderer, x, y);

        int errorTwice = 2*error;

        if( errorTwice >= diffY )
        {
            if( x == x1)
                break;
            error = error + diffY;
            x = x + stepX;
        }
        if( errorTwice <= diffX)
        {
            if(y == y1)
                break;
            error = error + diffX;
            y = y + stepY;
        }
    }
}

void fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, bool wireframe, double luminosity)
{
    if(wireframe)
    {
        drawLinePoints(x0, y0, x1, y1);
        drawLinePoints(x1, y1, x2, y2);
        drawLinePoints(x0, y0, x2, y2);
    }
    else
    {
        //Backface culling
        if(edgeFunc(x0,y0,x1,y1,x2,y2) < 0)
        {
            return;
        }

        int boundBoxMinX = MIN(MIN(x0,x1),x2);
        int boundBoxMaxX = MAX(MAX(x0, x1), x2);
        int boundBoxMinY = MIN(MIN(y0,y1),y2);
        int boundBoxMaxY = MAX(MAX(y0, y1), y2);

        uint8_t lightLevel = (uint8_t)(luminosity * 255);
        SDL_SetRenderDrawColor(renderer, lightLevel, lightLevel, lightLevel, 100);

        for(int x = boundBoxMinX; x < boundBoxMaxX; ++x)
        {
            for(int y = boundBoxMinY; y < boundBoxMaxY; ++y)
            {
                int check01 = edgeFunc(x0, y0, x1, y1, x, y);
                int check12 = edgeFunc(x1, y1, x2, y2, x, y);
                int check20 = edgeFunc(x2, y2, x0, y0, x, y);

                if(check01 < 0 || check12 < 0 || check20 < 0)
                {
                    continue;
                }

                SDL_RenderPoint(renderer, x, y);
            }
        }
    }
}

/* This function runs once at startup. */
// Create model here, parse file here too
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    //Create wireframe renderer object
    initModel(&model);
    parseObj("./src/diablo3_pose.obj", &model);
    //debug_printModel(&model);
    /* Create the window */
    if (!SDL_CreateWindowAndRenderer("Wireframe Renderer", WIDTH, HEIGHT, SDL_WINDOW_OPENGL , &window, &renderer)) {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    return SDL_APP_CONTINUE;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    if(event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.key == SDLK_RIGHT) {
            xzState = (xzState + 1) % 4;
            uint8_t nor = ~(vertAxis | horizAxis);
            horizAxis = nor;
            first = true;
        }
        else if (event->key.key == SDLK_LEFT) {
            xzState = (xzState + 3) % 4;
            uint8_t nor = ~(vertAxis | horizAxis);
            horizAxis = nor;
            first = true;
        }
        else if (event->key.key == SDLK_DOWN) {
            yzState = (yzState + 1) % 4;
            uint8_t nor = ~(vertAxis | horizAxis);
            vertAxis = nor;
            first = true;
        }
        else if (event->key.key == SDLK_UP) {
            yzState = (yzState + 3) % 4;
            uint8_t nor = ~(vertAxis | horizAxis);
            vertAxis = nor;
            first = true;
        }
        else
        {
            return SDL_APP_CONTINUE;
        }
    }
    return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    if(first)
    {
        // Clear First
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);    
        SDL_RenderClear(renderer);

        //Draw
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);    

        for(int i = 0; i < model.ctFace; ++i)
        {
            struct Face* face = &model.faceArr[i];

            SDL_FPoint points[face->ctVertex];

            struct Vertex* vertexArr[3];

            for(int v = 0; v < face->ctVertex; ++v)
            {
                int idx = face->indicies[v];

                struct Vertex* vertex;

                if(idx > 0)
                {
                    vertex = &model.vertexArr[idx-1];
                }
                else
                {                   
                    vertex = &model.vertexArr[model.ctVertex + idx + 1];
                }
                

                double pointX = vertex->x * ((horizAxis & 0x4) >> 2) + vertex->y * ((horizAxis &0x2) >> 1) + vertex->z * (horizAxis & 0x1);
                double pointY = vertex->x * ((vertAxis & 0x4) >> 2) + vertex->y * ((vertAxis &0x2) >> 1) + vertex->z * (vertAxis & 0x1);
                
                pointX = pointX * ((xzState & 0x2) ? -1 : 1);
                pointY = pointY * (((yzState & 0x1) ^ ( ( yzState & 0x2 ) >> 1)) ? -1 : 1);

                points[v].x = (WIDTH/2) * (1.0 + pointX);
                points[v].y = (HEIGHT/2) * (1.0 - pointY);

                vertexArr[v] = vertex;
            }
            
            SDL_FPoint* point0 = &points[0];
            SDL_FPoint* point1 = &points[1];
            SDL_FPoint* point2 = &points[2];

            double lum = luminosity(vertexArr);
            
            if(lum < 0)
            {
                continue; //Backface culling
            }

            fillTriangle(point0->x, point0->y, point1->x, point1->y, point2->x, point2->y, false, lum);
        }

        SDL_RenderPresent(renderer);
        first = false;
    }
    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
}
