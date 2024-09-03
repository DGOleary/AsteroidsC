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
    
    // set the positions in the struct
    ship.y = (int)y_pos;
    ship.x = (int)x_pos;
    
    int speed = 0;
    // allows the adjustment of how long it takes for the ship to speed down
    int accelCount = 50;

    //linked list that has blaster shots
    LinkedList *shots = createLinkedList();


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
            // case SDL_KEYDOWN:
            //     switch (event.key.keysym.scancode)
            //     {
            //     case SDL_SCANCODE_W:
            //     case SDL_SCANCODE_UP:
            //         up = 1;
            //         down = left = right = 0;
            //         break;
            //     case SDL_SCANCODE_S:
            //     case SDL_SCANCODE_DOWN:
            //         down = 1;
            //         up = left = right = 0;
            //         break;
            //     case SDL_SCANCODE_A:
            //     case SDL_SCANCODE_LEFT:
            //         left = 1;
            //         down = up = right = 0;
            //         ship_val.dir -= 5;
            //         break;
            //     case SDL_SCANCODE_D:
            //     case SDL_SCANCODE_RIGHT:
            //         right = 1;
            //         down = left = up = 0;
            //         ship_val.dir += 5;
            //         break;
            //     }
            //     break;
            // case SDL_KEYUP:
            //     switch (event.key.keysym.scancode)
            //     {
            //     case SDL_SCANCODE_W:
            //     case SDL_SCANCODE_UP:
            //         up = 0;
            //         break;
            //     case SDL_SCANCODE_S:
            //     case SDL_SCANCODE_DOWN:
            //         down = 0;
            //         break;
            //     case SDL_SCANCODE_A:
            //     case SDL_SCANCODE_LEFT:
            //         left = 0;
            //         break;
            //     case SDL_SCANCODE_D:
            //     case SDL_SCANCODE_RIGHT:
            //         right = 0;
            //         break;
            //     }
                break;
            }
        }

        //ship movement code
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

        //ship has acceleration that goes to 0 over time when the thrusters aren't engaged
        if(keystates[SDL_SCANCODE_UP]){
            up = 1;
            down = left = right = 0;
            accelCount = 50;
            speed += 1;
            if(speed > 10){
                speed = 10;
            }
        }else{
            if(speed < 0){
                speed = 0;
            //this counter allows the post thruster acceleration period to be adjusted for how long it lasts
            }else if(speed > 0 && accelCount % 5 == 0){
                speed--;
            }
            accelCount--;

            if(accelCount < 0){
                accelCount = 0;
            }
        }

        //this code controls where the ship ends up getting put after calculating the speed/acceleration
        double rad = (double)(ship_val.dir+90) * (M_PI/180.0);
        //seperates the vector to find the x and y position, due to rounding/float issues the number is truncated
        ship.x -= speed*(cos(rad)-.0001);
        ship.y -= speed*(sin(rad)-.0001);

        //boundary checks
        if(ship.x > 825){
            ship.x = 0;
        }

        if(ship.x < -25){
            ship.x = 825;
        }
        
        if(ship.y > 800){
            ship.y = 0;
        }

        if(ship.y < -25){
            ship.y = 800;
        }

        //code for shooting lasers
        if(keystates[SDL_SCANCODE_SPACE]){
            //creates shot on top of ship
            SDL_Rect *shot = (SDL_Rect*)malloc(sizeof(SDL_Rect));
            shot->x = ship.x;
            shot->y = ship.y;
            shot->w = 25;
            shot->h = 25;
            Sprite_Values *temp = createSpriteValues(shot, 1, 1, 25, 25, 0, SDL_FLIP_NONE);
            temp->frame_offsets[0] = (int[]){0, 25};

            shots = LinkedListAdd(shots, temp);
            // LinkedList *tempList = LinkedListAdd(shots, shot);
            // shots = tempList;
        }

        // clear the window
        SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
        SDL_RenderClear(rend);

        if(shots->value != NULL){
            animateStill(&obj, (Sprite_Values*)shots->value);
        }
        
        // draw the ship to the window
        animateStill(&obj, &ship_val);

        //SDL_RenderCopyEx(rend, tex, &area, &ship, 0, NULL, SDL_FLIP_NONE);
        SDL_RenderPresent(rend);

        // wait 1/60th of a second to target 60 fps
        SDL_Delay(1000 / 60);

    }

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
