#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
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

double toRadians(int deg){
    return (double)(deg) * (M_PI/180.0);
}

//function that creates a laser bolt
void createShot(SDL_Rect *shot, SDL_Rect *ship, Sprite_Values *ship_val){
    double rad = toRadians(ship_val->dir+90);
    //printf("%d\n", (int)(25*(cos(rad)-.0001)));
    shot->x = ship->x - (int)(25*(cos(rad)-.0001));
    //printf("%d\n", shot->x);
    shot->y = ship->y - (int)(25*(sin(rad)-.0001));
}

void shotCheck(Queue *shots, Queue *shotCounter, SDL_Objs *obj){
    Sprite_Values *temp = ((Sprite_Values*)shots->value); 
    double rad = toRadians(temp->dir+90);
    temp->loc->x -= (int)(20*(cos(rad)-.0001));
    temp->loc->y -= (int)(20*(sin(rad)-.0001));
    //test the boundaries for the bullets
    if(temp->loc->x > WINDOW_WIDTH){
    temp->loc->x = -25;
    }

    if(temp->loc->x < -25){
        temp->loc->x = WINDOW_WIDTH;
    }
    
    if(temp->loc->y > WINDOW_HEIGHT){
        temp->loc->y = -25;
    }

    if(temp->loc->y < -25){
        temp->loc->y = WINDOW_HEIGHT;
    }
    animateStill(obj, temp);
    int tempInt = *(int*)(shotCounter->value);
    *(int*)(shotCounter->value) = tempInt - 1;
}

void spawnAsteroid(int* cnt, LinkedList **list, LinkedList **objList, int *id){
    if(rand() <= 326 && *cnt < 10){ 
        SDL_Rect *rect = (SDL_Rect*)malloc(sizeof(SDL_Rect));
        rect->h = 50;
        rect->w = 50;
        rect->x = rand() % WINDOW_WIDTH;
        rect->y = rand() % WINDOW_HEIGHT;
        Sprite_Values *sprt = createSpriteValues(rect, 1, 1, 25, 25, (rand() % 361), SDL_FLIP_NONE);
        //("%d\n", sprt->dir);
        int astSprite = (rand() % 9) + 1;

        //create the array in dynamic memory
        sprt->frame_offsets[0]=malloc(2 * sizeof(int));
        sprt->frame_offsets[0][0] = 25 * astSprite;
        sprt->frame_offsets[0][1] = 0;
        *list = LinkedListAdd(*list, sprt);
        *objList = LinkedListAdd(*objList, createObject("asteroid", *id, sprt));
        *id = *id+1;
        *cnt = *cnt+1;
    }
}

void asteroidMove(Sprite_Values *asteroid){
    double rad = toRadians(asteroid->dir);
    asteroid->loc->x -= (int)(2*(cos(rad)-.0001));
    asteroid->loc->y -= (int)(2*(sin(rad)-.0001));
    if(asteroid->loc->x > WINDOW_WIDTH){
    asteroid->loc->x = -50;
    }

    if(asteroid->loc->x < -50){
        asteroid->loc->x = WINDOW_WIDTH;
    }
    
    if(asteroid->loc->y > WINDOW_HEIGHT){
        asteroid->loc->y = -50;
    }

    if(asteroid->loc->y < -50){
        asteroid->loc->y = WINDOW_HEIGHT;
    }
}

void displayAsteroids(LinkedList *list, SDL_Objs *obj){
    if((Sprite_Values*)list->value != NULL){
        while(list != NULL){
            // printf("%d\n", ((Sprite_Values*)displayAsteroids->value)->frame_offsets[0][0]);
            // printf("%d\n", ((Sprite_Values*)displayAsteroids->value)->frame_offsets[0][1]);
            // printf("\n");
            asteroidMove((Sprite_Values*)list->value);
            animateStill(obj, (Sprite_Values*)list->value);
            list = list->next;
        }
    }
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

    Sprite_Values ship_val = *createSpriteValues(&ship, 1, 1, 25, 25, 0, SDL_FLIP_NONE);
    Object shipObject = *createObject("ship", 0, &ship);
    // manually making the array of frame values
    ship_val.frame_offsets[0] = (int[]){0, 0};
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
    Queue *shots = createQueue();
    //linked list that has the timer for the shots
    Queue *shotsTimer = createQueue();
    //list that holds bounding objects for shots collisions
    Queue *shotsObjs = createQueue();
    
    int shotsCounter = 0;

    //controls the possible firing speed
    bool addShot = true;
    int addShotCounter = 0;

    //counter for amount of asteroids
    int astrdCount = 0;
    LinkedList *asteroids = createLinkedList();
    LinkedList *asteroidObjs = createLinkedList();
    int astrdId = 0;

    //creation of the boundaries, these hold the position of each object on screen so collisions can be detected
    Boundary boundaries[33][32];
    for(int i = 0; i < 33; i++){
        for(int j = 0; j < 32; j++){
            boundaries[i][j].w = 25;
            boundaries[i][j].h = 25;
            boundaries[i][j].x = j*25;
            boundaries[i][j].y = i*25;
            boundaries[i][j].objs = createLinkedList();
        }
    }

    //register the ship's inititial position
    boundaries[ship.y / 25][ship.x / 25].objs = LinkedListAdd(boundaries[ship.y / 25][ship.x / 25].objs, &shipObject);

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

        //save the old boundary box the ship inhabited
        int oldX = ship.x / 25;
        int oldY = ship.y / 25;
        printf("row %d\n", ship.y / 25);
        printf("col %d\n", ship.x / 25);
        printf("\n");

        //this code controls where the ship ends up getting put after calculating the speed/acceleration
        double rad = toRadians(ship_val.dir+90);
        //seperates the vector to find the x and y position, due to rounding/float issues the number is truncated
        ship.x -= (int)(speed*(cos(rad)-.0001));
        ship.y -= (int)(speed*(sin(rad)-.0001));

        //boundary checks
        if(ship.x > WINDOW_WIDTH){
            ship.x = 0;
        }

        if(ship.x < -25){
            ship.x = WINDOW_WIDTH;
        }
        
        if(ship.y > WINDOW_HEIGHT){
            ship.y = 0;
        }

        if(ship.y < -25){
            ship.y = WINDOW_HEIGHT;
        }

        //checks if the ship changed it's spot in the grid since the last movement
        if(ship.x / 25 != oldX || ship.y / 25 != oldY){
            //add the ship to it's new location
            boundaries[ship.y / 25][ship.x / 25].objs = LinkedListAdd(boundaries[ship.y / 25][ship.x / 25].objs, &shipObject);
            LinkedList *last = NULL;
            LinkedList *temp = boundaries[oldY][oldX].objs;

            while(temp != NULL){
                if(strcmp(((Object*)temp->value)->type, shipObject.type) == 0){
                    //if last is null then that means the ship is the head of the list of attached objects and can be removed, otherwise the list needs to be joined in the middle
                    if(last == NULL){
                        LinkedListPop(&temp);
                        break;
                    }else{
                        last->next = temp->next;
                        temp->value = NULL;
                        free(temp);
                        break;
                    }
                }

                temp = temp->next;
            }
        }
        // printf("%d\n", ship.x / 25);
        // printf("%d\n", ship.y / 25);
        // printf("\n");

        //code for shooting lasers
        addShotCounter++;

        if(addShotCounter == 8){
            addShot = true;
            addShotCounter = 0;
        }

        if(keystates[SDL_SCANCODE_SPACE] && addShot){
            addShot = false;
            addShotCounter = 0;

            //creates shot on top of ship
            SDL_Rect *shot = (SDL_Rect*)malloc(sizeof(SDL_Rect));
            createShot(shot, &ship, &ship_val);

            //test the boundaries for the bullets
            if(shot->x > WINDOW_WIDTH){
            shot->x = -25;
            }

            if(shot->x < -25){
                shot->x = WINDOW_WIDTH;
            }
            
            if(shot->y > WINDOW_HEIGHT){
                shot->y = -25;
            }

            if(shot->y < -25){
                shot->y = WINDOW_HEIGHT;
            }

            shot->w = 25;
            shot->h = 25;
            Sprite_Values *temp = createSpriteValues(shot, 1, 1, 25, 25, ship_val.dir, SDL_FLIP_NONE);
            temp->frame_offsets[0] = (int[]){0, 25};

            QueueAdd(shots, temp);
            //create an object for shot collisions 
            QueueAdd(shotsObjs, createObject("shot", shotsCounter++, temp));
            int *shotTimer = (int*)malloc(sizeof(int));
            *shotTimer = 50;
            QueueAdd(shotsTimer, shotTimer);
            // LinkedList *tempList = LinkedListAdd(shots, shot);
            // shots = tempList;
        }

        //checks if an asteroid should be spawned
        spawnAsteroid(&astrdCount, &asteroids, &asteroidObjs, &astrdId);
        // clear the window

        SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
        SDL_RenderClear(rend);

        //begins code that draws to the screen

        if(shots->value != NULL){
            Queue *temp = shots;
            Queue *tempCounter = shotsTimer;
            Queue *tempObj = shotsObjs;

            while(temp != NULL){
                shotCheck(temp, tempCounter, &obj);

                //checks if the counter has ran out for this laser bolt
                if(*(int*)(tempCounter->value) == 0){
                    Queue *displayVals = tempCounter;

                    // printf("%d\n", ((Sprite_Values*)temp->value)->loc->x);
                    // printf("%d\n", ((Sprite_Values*)temp->value)->loc->y);
                    // printf("\n");

                    //in the case the next is null, it needs to get these before the memory is freed
                    temp = temp->next;
                    //frees the value before temp becomes null if it is the end
                    free(tempCounter->value);
                    tempCounter = tempCounter->next;
                    Sprite_Values *del = (Sprite_Values*)QueuePoll(&shots);
                    for(int i=0;i<del->total_frames;i++){
                        free(del->frame_offsets[i]);
                    }
                    free(del->frame_offsets);
                    free(del->loc);
                    free(del);
                    QueuePoll(&shotsTimer);
                    tempObj = tempObj->next;
                    Object *delObj = QueuePoll(&shotsObjs);
                    free(delObj->type);
                    free(delObj);

                    //initialize them to empty lists if they're fully emptied 
                    if(shots == NULL){
                        shots = createQueue();
                        shotsTimer = createQueue();
                        shotsObjs = createQueue();
                    }
                }else{
                    temp = temp->next;
                    tempCounter = tempCounter->next;
                    tempObj = tempObj->next;
                }
                
            }

        }

        displayAsteroids(asteroids, &obj);
        
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

