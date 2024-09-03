#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "drawfunctions.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "defaultfunctions.h"

#define WINDOW_WIDTH (825)
#define WINDOW_HEIGHT (800)
#define ACCEL (3)
#define DECEL (1)

//function used to check if boundaries are the same, assumes boundaries don't start at the same origin
int compareBoundary(void *bound1, void *bound2){
    //casts the voids to boundaries
    Boundary bounds1=*(Boundary*)bound1;
    Boundary bounds2=*(Boundary*)bound2;
    if(bounds1.x-bounds2.x==0){
        return bounds1.y-bounds2.y;
    }
    return bounds1.x-bounds2.x;
}

//function to hash boundaries
int boundaryHash(HashSet *set, void *add){
    //casts to a boundary
    Boundary bound=*(Boundary*)add;
    //array of primes for each value in the Boundary struct
    int prime[] = {17, 53, 2, 79};
    //initial prime to seed function 
    int seed = 67;
    //combines all the values into one hash mod the size 
    int hash = (seed * (prime[0] + bound.x) * (prime[1] + bound.y) * (prime[2] + bound.h) * (prime[3] + bound.w)) % set->size;
    
    if(hash < 0){
        hash = hash * -1;
    }

    return hash;
}


int main(int argc, char *argv[])
{
    // area of a single tile from the texture atlas
    SDL_Rect area;
    area.x = 0;
    area.y = 0;
    area.w = 25;
    area.h = 25;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        printf("error initializing SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Asteroids", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);

    if (!window)
    {
        printf("window creation error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    SDL_Renderer *rend = SDL_CreateRenderer(window, -1, render_flags);
    if (!rend)
    {
        printf("error creating renderer: %s", SDL_GetError());
        SDL_DestroyRenderer(rend);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // enables alpha blending
    SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);

    // explicitly enables pngs
    IMG_Init(IMG_INIT_PNG);

    // loads texture atlas
    SDL_Surface *surface = IMG_Load("atlas.png");

    //checks for errors
    if (!surface)
    {
        printf("error creating surface: %s", SDL_GetError());
        SDL_DestroyRenderer(rend);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture *tex = SDL_CreateTextureFromSurface(rend, surface);

    if (!tex)
    {
        printf("error creating texture: %s", SDL_GetError());
        SDL_DestroyRenderer(rend);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // loads window icon
    surface = IMG_Load("icon.png");
    if (!surface)
    {
        printf("error creating surface: %s", SDL_GetError());
        SDL_DestroyRenderer(rend);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }


    SDL_SetWindowIcon(window, surface);
    SDL_FreeSurface(surface);

    // creates rectangle to export ship texture to
    SDL_Rect ship;
    //SDL_QueryTexture(tex, NULL, NULL, &pac.w, &pac.h);
    ship.w=25;
    ship.h=25;

    //SDL_Rect *ghosts = (SDL_Rect*)malloc(4*sizeof(SDL_Rect));

    ship.x = (WINDOW_WIDTH - ship.w) / 2;

    float x_pos = (WINDOW_WIDTH - ship.w) / 2;
    float y_pos = (WINDOW_HEIGHT - ship.h) / 2;
    float x_vel = 0;
    float y_vel = 0;

    int up = 0;
    int down = 0;
    int left = 0;
    int right = 0;

    // holds the link to the renderer and the texture atlas
    SDL_Objs obj;
    obj.rend = rend;
    obj.tex = tex;

    Sprite_Values ship_val = *createSpriteValues(&ship, 5, 2, 25, 25, 0, SDL_FLIP_NONE);
    // manually making the array of frame values
    ship_val.frame_offsets[0] = (int[]){0, 0};
    ship_val.frame_offsets[1] = (int[]){25, 0};
    ship_val.flip = SDL_FLIP_NONE;
    ship_val.dir = 0;

    bool close_requested = false;

    //create boundaries for the maze
    Boundary bottom;
    bottom.x=0;
    bottom.y=375;
    bottom.w=25;
    bottom.h=25;

    HashSet *boundaries = createHashSet();

    HashSetAdd(boundaries, &bottom, boundaryHash);
    
    // set the positions in the struct
    ship.y = (int)y_pos;
    ship.x = (int)x_pos;

    bool lrDirection = false;
    
    int speed = 0;
    // allows the adjustment of how long it takes for the ship to speed down
    int accelCount = 50;

    while (!close_requested)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                close_requested = true;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode)
                {
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                    up = 1;
                    down = left = right = 0;
                    break;
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:
                    down = 1;
                    up = left = right = 0;
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    left = 1;
                    down = up = right = 0;
                    ship_val.dir -= 5;
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    right = 1;
                    down = left = up = 0;
                    ship_val.dir += 5;
                    break;
                }
                break;
            //     //TODO remove this code and edit so he continues in the same direction and can only move in available directions
            case SDL_KEYUP:
                switch (event.key.keysym.scancode)
                {
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                    up = 0;
                    break;
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:
                    down = 0;
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    left = 0;
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    right = 0;
                    break;
                }
                break;
            }
        }

        // printf("%d\n", left);
        // printf("%d\n", right);
        // printf("\n");

        const Uint8* keystates = SDL_GetKeyboardState(NULL);
        if(keystates[SDL_SCANCODE_LEFT]){
            left = 1;
            down = up = right = 0;
            ship_val.dir -= 5;
        }
        if(keystates[SDL_SCANCODE_RIGHT]){
            right = 1;
            down = up = left = 0;
            ship_val.dir += 5;
        }
        ship_val.dir %= 361;
        // bool currKey = false;
        // if(keystates[SDL_SCANCODE_LEFT]){
        //     currKey = false;
        // }else if(keystates[SDL_SCANCODE_RIGHT]){
        //     currKey = true;
        // }


        // if(keystates[SDL_SCANCODE_LEFT] && keystates[SDL_SCANCODE_RIGHT]){
        //         lrDirection = !lrDirection;
        // }else if(keystates[SDL_SCANCODE_LEFT] || keystates[SDL_SCANCODE_RIGHT]){
        //     if(keystates[SDL_SCANCODE_LEFT]){
        //         lrDirection = false;
        //     }else if(keystates[SDL_SCANCODE_RIGHT]){
        //         lrDirection = true;
        //     }
        // }

        // if(keystates[SDL_SCANCODE_LEFT] || keystates[SDL_SCANCODE_RIGHT]){
        //     if(!lrDirection){
        //             left = 1;
        //             down = up = right = 0;
        //             ship_val.dir -= 5;
        //         }else{
        //             right = 1;
        //             down = up = left = 0;
        //             ship_val.dir += 5;
        //         }
        // }
            

        

        // if(keystates[SDL_SCANCODE_LEFT]){
        //     left = 1;
        //     down = up = right = 0;
        //     ship_val.dir -= 5;
        // }else if(keystates[SDL_SCANCODE_RIGHT]){
        //     right = 1;
        //     down = up = left = 0;
        //     ship_val.dir += 5;
        // }


        if(keystates[SDL_SCANCODE_UP]){
            up = 1;
            down = left = right = 0;
            accelCount = 50;
            speed += 1;
            if(speed > 10){
                speed = 10;
            }
            // printf("%d\n", ship_val.dir+90);
            // printf("%lf\n", 3*cos(rad)-.0001);
            // printf("%lf\n",  3*sin(rad)-.0001);
            
            
            // printf("%d\n", ship.x);
            // printf("%d\n", ship.y);
            // printf("\n");
        }else{
            if(speed < 0){
                speed = 0;
            }else if(speed > 0 && accelCount % 5 == 0){
                speed--;
            }
            accelCount--;

            if(accelCount < 0){
                accelCount = 0;
            }
        }

        printf("%d\n", speed);

        double rad = (double)(ship_val.dir+90) * (M_PI/180.0);
        ship.x -= speed*(cos(rad)-.0001);
        ship.y -= speed*(sin(rad)-.0001);

        if(keystates[SDL_SCANCODE_DOWN]){
            down = 1;
            up = left = right = 0;
        }

        // x_vel = y_vel = 0;
        // if (up && !down)
        // {
        //     y_vel = -SPEED;
        // }
        // if (!up && down)
        // {
        //     y_vel = SPEED;
        // }
        // if (left && !right)
        // {
        //     x_vel = -SPEED;
        // }
        // if (!left && right)
        // {
        //     x_vel = SPEED;
        // }s

        // // update positions
        // x_pos += x_vel / 60;
        // y_pos += y_vel / 60;
        
        // if (x_pos <= 0)
        //     x_pos = 0;
        // if (y_pos <= 0)
        //     y_pos = 0;
        // if (x_pos >= WINDOW_WIDTH - pac.w)
        //     x_pos = WINDOW_WIDTH - pac.w;
        // if (y_pos >= WINDOW_HEIGHT - pac.h)
        //     y_pos = WINDOW_HEIGHT - pac.h;

        // Boundary *pos = (Boundary *)malloc(sizeof(Boundary));
        // pos->x=x_pos;
        // pos->y=y_pos;
        // pos->h=25;
        // pos->y=25;
        // printf("%d\n",323);
        // if(HashSetContains(boundaries, pos, boundaryHash, compareBoundary)){
        //     printf("bounds check");
        // }

        // clear the window
        SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
        SDL_RenderClear(rend);

        // draw the ship to the window
        animateStill(&obj, &ship_val);
        //SDL_RenderCopyEx(rend, tex, &area, &ship, 0, NULL, SDL_FLIP_NONE);
        SDL_RenderPresent(rend);

        // wait 1/60th of a second
        SDL_Delay(1000 / 60);

    }

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(window);
    SDL_Quit();
}





//TODO function that will make background
// void drawLevel(SDL_Renderer *rend, SDL_Texture *tex, Sprite_Values **vals){
//     *vals=(Sprite_Values*)malloc(10*sizeof(Sprite_Values));
//     for(int i=0;i<10;i++){
//         //rects for the location on screen and 
//         SDL_Rect *temploc=(SDL_Rect*)malloc(sizeof(SDL_Rect));
//         SDL_Rect *tempat=(SDL_Rect*)malloc(sizeof(SDL_Rect));
//         vals[i]=createBGTile(temp, 0, 0, 25, 25, SDL_FLIP_NONE);
//     }
//     SDL_RenderCopy(rend, tex, )
// }