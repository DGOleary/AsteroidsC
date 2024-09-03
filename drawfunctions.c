#include <stdlib.h>
#include <stdio.h>
#include "drawfunctions.h"
#include "defaultfunctions.h"

void animateLoop(SDL_Objs *obj, Sprite_Values *values)
{
    values->current_frame_in_animation++;
    // check if it needs to switch to the next frame or not
    if (values->current_frame_in_animation == values->frames_per)
    {
        //resets the counter for the new frame
        values->current_frame_in_animation = 0;

        //sets the current frame to next, if it reached the end restart at frame 0 of the animation
        values->current_frame++;
        if (values->current_frame == values->total_frames)
        {
            values->current_frame = 0;
        }
    }
    SDL_Rect temp_atlas_index;
    //go into the frame offset array and get the x and y value of the frame
    temp_atlas_index.x = values->frame_offsets[values->current_frame][0];
    temp_atlas_index.y = values->frame_offsets[values->current_frame][1];

    temp_atlas_index.w = values->w;
    temp_atlas_index.h = values->h;
    //put the frame to memory
    SDL_RenderCopyEx(obj->rend, obj->tex, &temp_atlas_index, values->loc, values->dir, NULL, values->flip);
}

void animateStill(SDL_Objs *obj, Sprite_Values *values)
{
    SDL_Rect temp_atlas_index;
    //go into the frame offset array and get the x and y value of the frame
    temp_atlas_index.x = values->frame_offsets[0][0];
    temp_atlas_index.y = values->frame_offsets[0][1];

    temp_atlas_index.w = values->w;
    temp_atlas_index.h = values->h;
    //put the frame to memory
    SDL_RenderCopyEx(obj->rend, obj->tex, &temp_atlas_index, values->loc, values->dir, NULL, values->flip);
}

Sprite_Values *createSpriteValues(SDL_Rect *loc, int frames_per, int total_frames, int w, int h, int dir, SDL_RendererFlip flip){
    //sets all the values in a struct
    Sprite_Values *obj=(Sprite_Values* )malloc(sizeof(Sprite_Values));
    obj->loc=loc;
    obj->frames_per=frames_per;
    obj->total_frames=total_frames;
    obj->w=w;
    obj->h=h;
    obj->dir=dir;
    obj->flip=flip;
    obj->current_frame=0;
    obj->current_frame_in_animation=0;
    //array is left empty, up to user to fill indices with the correct frames
    obj->frame_offsets=(int**)malloc(total_frames * sizeof(int*));
    for(int i=0;i<total_frames;i++){
        obj->frame_offsets[i]=(int*)malloc(2 * sizeof(int));
    }
    return obj;
}

BG_Image *createBGImage(SDL_Rect *loc, int w, int h, SDL_RendererFlip flip){
    BG_Image *obj=(BG_Image*)malloc(sizeof(BG_Image));
    obj->loc=loc;
    obj->w=w;
    obj->h=h;
    obj->flip=flip;
    return obj;
}

int BGHash(HashSet *set, void *add){
    //casts to a Boundary specifically
    Boundary *tile=(Boundary*) add;
    //array of primes for each value in the Boundary struct
    int primes[] = {23, 149, 211, 373};
    //initial prime to seed function 
    int seed = 11;
    //combines all the values into one hash mod the size 
    int hash = (seed * (primes[0] + tile->h) * (primes[1] + tile->w) * (primes[2] + tile->x) * (primes[3] + tile->y)) % set->size;

    return hash;
}
