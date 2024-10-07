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
    Object *obj;
    int cnt;
} Laser;

typedef struct{
    Sprite_Values *spv;
    Object *obj;
} Asteroid;

double toRadians(int deg){
    return (double)(deg) * (M_PI/180.0);
}

void checkXYInBoundary(int *x, int *y){
    if(*y < 0){
       *y = 0;
    }
    if(*y >= WINDOW_HEIGHT/25){
        *y = (WINDOW_HEIGHT/25)-1;
    }

    if(*x < 0){
        *x = 0;
    }
    if(*x >= WINDOW_WIDTH/25){
        *x = (WINDOW_WIDTH/25)-1;
    }  
}

void setObjectInBoundary(Boundary boundaries[WINDOW_WIDTH/25][WINDOW_HEIGHT/25], Object *object, int oldX, int oldY, int newX, int newY){  
    //checks boundaries, if it's out of bounds the boundary extends to the last onscreen box
    checkXYInBoundary(&oldX, &oldY);
    checkXYInBoundary(&newX, &newY);

    //checks if the object changed it's spot in the grid since the last movement
    if(newX != oldX || newY != oldY){
        //add the object to it's new location
        boundaries[newX][newY].objs = LinkedListAdd(boundaries[newX][newY].objs, object);
        // if(strcmp(object->type, "asteroid") != 0){
        //     printf("%s\n", object->type);
        // }
        
        // printf("row new %d\n", newX);
        // printf("col new %d\n", newY);
        // printf("\n");
        LinkedList *last = NULL;
        LinkedList *temp = boundaries[oldX][oldY].objs;

        if(temp == NULL || temp->value == NULL){
            temp = NULL;
        }

        while(temp != NULL){
            if(strcmp(((Object*)temp->value)->type, object->type) == 0 && ((Object*)temp->value)->id == object->id){
                //connect the list before and after the current object
                LinkedList *rest = temp->next;
                //free the old location of the crrent object
                free(temp);
                temp = NULL;
                //if there's previous nodes connect them to the nodes after the object being removed, otherwise make the following nodes the list, if both are empty create a new list on the location
                if(last != NULL && last->value != NULL){
                    last->next = rest;
                }else{
                    if(rest != NULL){
                        boundaries[oldX][oldY].objs = rest;
                    }else{
                        boundaries[oldX][oldY].objs = createLinkedList();
                    }
                    
                }
                break;

                //printf("%s\n", "same");
            }
            last = temp;
            temp = temp->next;
        }
    }
}

bool checkBounds(int x, int y, int oX, int oY, int w, int h){
    if(x >= oX && x <= oX+w){
        if(y >= oY && y <= oY + h){
            return true;
        }
    }
    return false;
}

Object* detectCollision(int x, int y, char *type, Boundary boundaries[WINDOW_WIDTH/25][WINDOW_HEIGHT/25], bool remove){
    int xPos = x+13;
    int yPos = y+13;
    x /= 25;
    y /= 25;
    checkXYInBoundary(&x, &y);
    for(int i = 0; i < 3; i++){
        int tempY = (y - 1 + i);
        tempY = (tempY > 0) ? tempY : 0;
        tempY %= ((WINDOW_HEIGHT/25));
        for(int j = 0; j < 3; j++){
            int tempX = (x - 1 + j);
            tempX = (tempX > 0) ? tempX : 0;
            tempX %= ((WINDOW_WIDTH/25));
            LinkedList *list = boundaries[tempX][tempY].objs;
            LinkedList *last = NULL;
            while(list != NULL && ((Object*)list->value) != NULL){
                Object *ob = ((Object*)list->value);
                if(strcmp(ob->type, type) == 0){
                        SDL_Rect *loc = ((Sprite_Values*)ob->obj)->loc;
                        if(checkBounds(xPos, yPos, loc->x, loc->y, loc->w, loc->h)){
                        if(remove){
                            LinkedList *rest = list->next;
                            if(last != NULL && last->value != NULL){
                                last->next = rest;
                            }else{
                                if(rest != NULL){
                                    boundaries[tempX][tempY].objs = rest;
                                }else{
                                    boundaries[tempX][tempY].objs = createLinkedList();
                             }
                            
                            }
                        }
                        return ob;
                    }
                }
                last = list;
                list = list->next;
            }
        }
    }
    return NULL;
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

void shotCheck(Laser *laser, SDL_Objs *obj, Boundary boundaries[WINDOW_WIDTH/25][WINDOW_HEIGHT/25]){
    Sprite_Values *temp = laser->spv; 
    int oldX = temp->loc->x;
    int oldY = temp->loc->y;
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
    
    setObjectInBoundary(boundaries, laser->obj, oldX, oldY, temp->loc->x / 25, temp->loc->y / 25);
    animateStill(obj, temp);
    laser->cnt = laser->cnt - 1;
}

//TODO adjust spawn rates based on amount of asteroids
void spawnAsteroid(int* cnt, int *id, LinkedList **list, Boundary boundaries[WINDOW_WIDTH/25][WINDOW_HEIGHT/25], int shipX, int shipY, int *spawnX, int *spawnY){
    if((rand() <= 326 && *cnt < 10) || ((spawnX != NULL && spawnY != NULL))){ 
        SDL_Rect *rect = (SDL_Rect*)malloc(sizeof(SDL_Rect));
        rect->h = 50;
        rect->w = 50;
        rect->x = rand() % WINDOW_WIDTH;//(WINDOW_WIDTH - 25) / 2;
        rect->y = rand() % WINDOW_HEIGHT;//(WINDOW_HEIGHT - 25) / 2;
        if(spawnX != NULL && spawnY != NULL){
            rect->x = *spawnX;
            rect->y = *spawnY;
        }
        //make sure the asteroid doesn't spawn on or very near to the ship
        if(abs(rect->x - shipX) <= 75 && abs(rect->y - shipY) <= 75){
            spawnAsteroid(cnt, id, list, boundaries, shipX, shipY, spawnX, spawnY);
            return;
        }
        Sprite_Values *sprt = createSpriteValues(rect, 1, 1, 25, 25, (rand() % 361), SDL_FLIP_NONE);
        int astSprite = (rand() % 9) + 1;

        //create the array in dynamic memory
        sprt->frame_offsets[0][0] = 25 * astSprite;
        sprt->frame_offsets[0][1] = 0;
        Asteroid *as = (Asteroid*)malloc(sizeof(Asteroid));
        Object *ob = createObject("asteroid", *id, sprt);
        as->obj = ob;
        as->spv = sprt;
        int x = rect->x / 25;
        int y = rect->y / 25;

        boundaries[x][y].objs = LinkedListAdd(boundaries[x][y].objs, ob);

        *list = LinkedListAdd(*list, as);
        *id = *id+1;
        *cnt = *cnt+1;
    }
}

void asteroidMove(Asteroid *asteroidObj, Boundary boundaries[WINDOW_WIDTH/25][WINDOW_HEIGHT/25]){
    Sprite_Values *asteroid = asteroidObj->spv;
    double rad = toRadians(asteroid->dir);
    int oldX = asteroid->loc->x / 25;
    int oldY = asteroid->loc->y / 25;
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

    setObjectInBoundary(boundaries, asteroidObj->obj, oldX, oldY, asteroid->loc->x / 25, asteroid->loc->y / 25);
}

void displayAsteroids(LinkedList *list, SDL_Objs *obj, Boundary boundaries[WINDOW_WIDTH/25][WINDOW_HEIGHT/25]){
    if(list->value != NULL){
        while(list != NULL){
            // printf("%d\n", ((Sprite_Values*)displayAsteroids->value)->frame_offsets[0][0]);
            // printf("%d\n", ((Sprite_Values*)displayAsteroids->value)->frame_offsets[0][1]);
            // printf("\n");
            //Sprite_Values *temp = ((Asteroid*)list->value)->spv;
            asteroidMove((Asteroid*)list->value, boundaries);
            animateStill(obj, ((Asteroid*)list->value)->spv);
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
    int shotId = 0;

    //linked list holding asteroids
    LinkedList *asteroids = createLinkedList();
    int asteroidCount = 0;
    int asteroidID = 0;

    //creation of the boundaries, these hold the position of each object on screen so collisions can be detected
    int w = WINDOW_WIDTH/25;
    int h = WINDOW_HEIGHT/25;

    Boundary boundaries[w][h];
    for(int i = 0; i < w; i++){
        for(int j = 0; j < h; j++){
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

        //checks if the ship changed it's spot in the grid since the last movement
        setObjectInBoundary(boundaries, &shipObject, oldX, oldY, shipX, shipY);
        if(detectCollision(ship.x, ship.y, "asteroid", boundaries, false) != NULL){
            printf("%s\n", "asteroid hit");
        }

        if(keystates[SDL_SCANCODE_SPACE] && addShot){
            //for(int i = 0; i < 5; i++){
            addShot = false;
            addShotCounter = 0;

            //printf("shot\n");
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
            Object *ob = createObject("laser", shotId, temp);
            shotId++;
            hold->spv = temp;
            hold->obj = ob;
            hold->cnt = 50;//- (i*5);

            QueueAdd(shots, hold);
            int x = shot->x;
            int y = shot->y;
            checkXYInBoundary(&x, &y);
            boundaries[x][y].objs = LinkedListAdd(boundaries[x][y].objs, ob);
            //checks if the asteroid is hit in the initial spawn
            //TODO add asteroid removal code
            // if(detectCollision(shot->x, shot->y, "asteroid", boundaries, false) != NULL){
            //     printf("%s\n", "laser shot");
            // }
        }

        spawnAsteroid(&asteroidCount, &asteroidID, &asteroids, boundaries, ship.x, ship.y, NULL, NULL);

        SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
        SDL_RenderClear(rend);

        if(shots->value != NULL){
            Queue *temp = shots;
            Queue *prev = shots;
            while(temp != NULL){
                Laser *shot_laser = (Laser*)temp->value;
                //TODO remove lasers that hit from the game
                Object *hit = detectCollision(shot_laser->spv->loc->x, shot_laser->spv->loc->y, "asteroid", boundaries, true);
                if(hit != NULL){
                    printf("%s\n", "laser");
                    LinkedList *tempList = asteroids;
                    LinkedList *last = NULL;
                    int spawnX = 0;
                    int spawnY = 0;
                    while(tempList != NULL){
                        Asteroid *as = (Asteroid*)tempList->value;
                        if(as->obj == hit){
                            spawnX = as->spv->loc->x;
                            spawnY = as->spv->loc->y;
                            LinkedList *rest = tempList->next;
                            if(last != NULL && last->value != NULL){
                                last->next = tempList->next;
                            }else{
                                if(rest != NULL){
                                    asteroids = rest;
                                }else{
                                    asteroids = createLinkedList();
                             }
                            }
                            free(tempList);
                            free(hit->type);
                            free(hit);
                            freeSpriteValues(as->spv);
                            free(as);
                            asteroidCount--;
                            //remove laser when it hits an asteroid before enabling
                            //spawnAsteroid(&asteroidCount, &asteroidID, &asteroids, boundaries, ship.x, ship.y, &spawnX, &spawnY);
                            //spawnAsteroid(&asteroidCount, &asteroidID, &asteroids, boundaries, ship.x, ship.y, &spawnX, &spawnY);
                            break;
                        }
                        last = tempList;
                        tempList = tempList->next;
                    }
                }
                shotCheck(shot_laser, &obj, boundaries);
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
                    prev = temp;
                    temp = temp->next;
                }
            }
        }

        displayAsteroids(asteroids, &obj, boundaries);

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

