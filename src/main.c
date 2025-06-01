#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>

#include <stdlib.h>

#include "model.h"
#include "parse.h"

#define WIDTH 640
#define HEIGHT 480

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

static struct Model model;

static uint8_t xzState = 0;
static uint8_t yzState = 0;

static uint8_t vertAxis = 0x2; //010
static uint8_t horizAxis = 0x4; //100

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

void renderFace(){}
/* This function runs once at startup. */
// Create model here, parse file here too
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    //Create wireframe renderer object
    initModel(&model);
    parseObj("./src/test_triangle.obj", &model);
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
        }
        else if (event->key.key == SDLK_LEFT) {
            xzState = (xzState + 3) % 4;
            uint8_t nor = ~(vertAxis | horizAxis);
            horizAxis = nor;
        }
        else if (event->key.key == SDLK_DOWN) {
            yzState = (yzState + 1) % 4;
            uint8_t nor = ~(vertAxis | horizAxis);
            vertAxis = nor;
        }
        else if (event->key.key == SDLK_UP) {
            yzState = (yzState + 3) % 4;
            uint8_t nor = ~(vertAxis | horizAxis);
            vertAxis = nor;
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

    // Clear First
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);    
    SDL_RenderClear(renderer);

    //Draw
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);    

    for(int i = 0; i < model.ctFace; ++i)
    {
        struct Face* face = &model.faceArr[i];

        SDL_FPoint points[face->ctVertex];

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
        }

        for(int p = 0; p < face->ctVertex; ++p)
        {
            SDL_FPoint* point0 = &points[p];
            SDL_FPoint* point1 = &points[(p+1)%(face->ctVertex)];
           
            drawLinePoints(point0->x, point0->y, point1->x, point1->y);
        }
    }
    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
}
