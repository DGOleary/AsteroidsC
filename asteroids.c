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

typedef struct{
    Sprite_Values *spv;
    int cnt;
} Laser;

double toRadians(int deg){
    return (double)(deg) * (M_PI/180.0);
}

void setObjectInBoundary(Boundary **boundaries, Object *object, int oldX, int oldY, int newX, int newY){    
    //checks if the ship changed it's spot in the grid since the last movement
    if(newX != oldX || newY != oldY){
        //add the ship to it's new location
        boundaries[newX][newY].objs = LinkedListAdd(boundaries[newX][newY].objs, object);
        printf("row new %d\n", newX);
        printf("col new %d\n", newY);
        printf("\n");
        LinkedList *last = NULL;
        LinkedList *temp = boundaries[oldX][oldY].objs;

        if(temp == NULL || temp->value == NULL){
            temp = NULL;
        }

        while(temp != NULL){
            if(strcmp(((Object*)temp->value)->type, object->type) == 0){
                //if last is null then that means the ship is the head of the list of attached objects and can be removed, otherwise the list needs to be joined in the middle
                if(last == NULL){
                    LinkedListPop(&temp);
                    if(temp == NULL){
                        boundaries[oldX][oldY].objs = createLinkedList();
                    }
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
}
//function that creates a laser bolt
void createShot(SDL_Rect *shot, SDL_Rect *ship, Sprite_Values *ship_val){
    double rad = toRadians(ship_val->dir+90);
    //printf("%d\n", (int)(25*(cos(rad)-.0001)));
    shot->x = ship->x - (int)(25*(cos(rad)-.0001));
    //printf("%d\n", shot->x);
    shot->y = ship->y - (int)(25*(sin(rad)-.0001));
    shot->w = 25;
    shot->h = 25;
}

void shotCheck(Laser *laser, SDL_Objs *obj){
    Sprite_Values *temp = laser->spv; 
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
    laser->cnt = laser->cnt - 1;
}

//void spawnAsteroid(int* cnt, LinkedList **list, LinkedList **objList, int *id){
void spawnAsteroid(int* cnt, LinkedList **list){
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
        //sprt->frame_offsets[0]=malloc(2 * sizeof(int));
        sprt->frame_offsets[0][0] = 25 * astSprite;
        sprt->frame_offsets[0][1] = 0;
        *list = LinkedListAdd(*list, sprt);
        //*objList = LinkedListAdd(*objList, createObject("asteroid", *id, sprt));
        //*id = *id+1;
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

    SDL_Rect area;
    area.w=25;
    area.h=25;
    area.x=0;
    area.y=0;

    // creates rectangle to export ship texture to
    SDL_Rect ship;
    //SDL_QueryTexture(tex, NULL, NULL, &pac.w, &pac.h);
    ship.w=25;
    ship.h=25;

    ship.x = (WINDOW_WIDTH - ship.w) / 2;
    ship.y = (WINDOW_HEIGHT - ship.h) / 2;

    Sprite_Values ship_val = *createSpriteValues(&ship, 1, 1, 25, 25, 0, SDL_FLIP_NONE);
    Object shipObject = *createObject("ship", 0, &ship);
    // manually making the array of frame values
    ship_val.frame_offsets[0][0] = 0;// = (int[]){0, 0};
    ship_val.frame_offsets[0][1] = 0;
    ship_val.flip = SDL_FLIP_NONE;
    ship_val.dir = 0;

    int speed = 0;
    // allows the adjustment of how long it takes for the ship to speed down
    int accelCount = 50;

    int shotsCounter = 0;

    //controls the possible firing speed
    bool addShot = true;
    int addShotCounter = 0;

    int up = 0;
    int down = 0;
    int left = 0;
    int right = 0;

    // holds the link to the renderer and the texture atlas
    SDL_Objs obj;
    obj.rend = rend;
    obj.tex = tex;

    bool close_requested = false;
    
    //Queue that has blaster shots
    Queue *shots = createQueue();

    //linked list holding asteroids
    LinkedList *asteroids = createLinkedList();
    int asteroidCount = 0;

    //creation of the boundaries, these hold the position of each object on screen so collisions can be detected
    Boundary boundaries[33][32];
    for(int i = 0; i < 33; i++){
        for(int j = 0; j < 32; j++){
            boundaries[i][j].w = 25;
            boundaries[i][j].h = 25;
            boundaries[i][j].x = i*25;
            boundaries[i][j].y = j*25;
            boundaries[i][j].objs = createLinkedList();
        }
    }

    //register the ship's inititial position
    boundaries[ship.x / 25][ship.y / 25].objs = LinkedListAdd(boundaries[ship.x / 25][ship.y / 25].objs, &shipObject);

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
        //checks boundaries, if it's out of bounds the boundary extends to the last onscreen box
        if(oldY < 0){
            oldY = 0;
        }
        if(oldY >= WINDOW_HEIGHT/25){
            oldY = (WINDOW_HEIGHT/25)-1;
        }

        if(oldX < 0){
            oldX = 0;
        }
        if(oldX >= WINDOW_WIDTH/25){
            oldX = (WINDOW_WIDTH/25)-1;
        }
        //this code controls where the ship ends up getting put after calculating the speed/acceleration
        double rad = toRadians(ship_val.dir+90);
        //seperates the vector to find the x and y position, due to rounding/float issues the number is truncated
        ship.x -= (int)(speed*(cos(rad)-.0001));
        ship.y -= (int)(speed*(sin(rad)-.0001));

        //boundary checks
        if(ship.x > WINDOW_WIDTH){
            ship.x = -25;
        }

        if(ship.x < -25){
            ship.x = WINDOW_WIDTH;
        }
        
        if(ship.y > WINDOW_HEIGHT){
            ship.y = -25;
        }

        if(ship.y < -25){
            ship.y = WINDOW_HEIGHT;
        }

        //code for shooting lasers
        addShotCounter++;

        if(addShotCounter == 11){
            addShot = true;
            addShotCounter = 0;
        }

        int shipY = ship.y / 25;
        int shipX = ship.x / 25;
        //checks boundaries, if it's out of bounds the boundary extends to the last onscreen box
        if(shipY < 0){
            shipY = 0;
        }
        if(shipY >= WINDOW_HEIGHT/25){
            shipY = (WINDOW_HEIGHT/25)-1;
        }

        if(shipX < 0){
            shipX = 0;
        }
        if(shipX >= WINDOW_WIDTH/25){
            shipX = (WINDOW_WIDTH/25)-1;
        }
        
        if(shipX < 0 || shipX >= WINDOW_HEIGHT/25 || shipY < 0 || shipY >= WINDOW_WIDTH/25){
            printf("row err %d\n", shipX);
            printf("col err %d\n", shipY);
            printf("\n");
        }

        //TODO encapsulate
        //checks if the ship changed it's spot in the grid since the last movement
        if(shipX != oldX || shipY != oldY){
            //add the ship to it's new location
            boundaries[shipX][shipY].objs = LinkedListAdd(boundaries[shipX][shipY].objs, &shipObject);
            printf("row new %d\n", shipX);
            printf("col new %d\n", shipY);
            printf("\n");
            LinkedList *last = NULL;
            LinkedList *temp = boundaries[oldX][oldY].objs;

            if(temp == NULL || temp->value == NULL){
                temp = NULL;
            }

            while(temp != NULL){
                if(strcmp(((Object*)temp->value)->type, shipObject.type) == 0){
                    //if last is null then that means the ship is the head of the list of attached objects and can be removed, otherwise the list needs to be joined in the middle
                    if(last == NULL){
                        LinkedListPop(&temp);
                        if(temp == NULL){
                            boundaries[oldX][oldY].objs = createLinkedList();
                        }
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

        if(keystates[SDL_SCANCODE_SPACE] && addShot){
            //for(int i = 0; i < 5; i++){
            addShot = false;
            addShotCounter = 0;

            printf("shot\n");
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

            Sprite_Values *temp = createSpriteValues(shot, 1, 1, 25, 25, ship_val.dir, SDL_FLIP_NONE);
            temp->frame_offsets[0][0] = 0;//(int[]){0, 25};
            temp->frame_offsets[0][1] = 25;

            Laser *hold = (Laser*)malloc(sizeof(Laser));
            hold->spv = temp;
            hold->cnt = 50;//- (i*5);

            QueueAdd(shots, hold);
        }
            //create an object for shot collisions 
            // Object *shotTemp = createObject("shot", shotsCounter++, temp);
            // QueueAdd(shotsObjs, shotTemp);
            // free(shotTemp->type);
            // int *shotTimer = (int*)malloc(sizeof(int));
            // *shotTimer = 50;
            // QueueAdd(shotsTimer, shotTimer);
            // LinkedList *tempList = LinkedListAdd(shots, shot);
            // shots = tempList;
        //}
        spawnAsteroid(&asteroidCount, &asteroids);

        SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
        SDL_RenderClear(rend);

        if(shots->value != NULL){
            Queue *temp = shots;

            while(temp != NULL){
                Laser *shot_laser = (Laser*)temp->value;
                shotCheck(shot_laser, &obj);
                //checks if the counter has ran out for this laser bolt
                if(shot_laser->cnt == 0){              
                    //frees the value before temp becomes null if it is the end
                    Laser *del = (Laser*)QueuePoll(&shots);
                    freeSpriteValues(del->spv);
                    free(del);

                    //initialize them to empty lists if they're fully emptied 
                    if(shots == NULL){
                        shots = createQueue();
                    }
                    temp = NULL;
                }else{
                    temp = temp->next;
                }
            }
        }

        displayAsteroids(asteroids, &obj);

        // draw the ship to the window
        animateStill(&obj, &ship_val);

        SDL_RenderPresent(rend);

        // wait 1/60th of a second to target 60 fps
        SDL_Delay(1000 / 60);

    }

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

