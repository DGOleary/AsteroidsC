#ifndef DRAWFUNCTIONS_H
#define DRAWFUNCTIONS_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "defaultfunctions.h"

#define TEX_ATLAS_SIZE (500)

//struct to hole SDL objects used to draw to the screen
typedef struct {
    SDL_Renderer *rend;
    SDL_Texture *tex;
} SDL_Objs;

//this holds the values for a texture object, the sdl rect holds where the first frame sits on the texture atlas
typedef struct {
    SDL_Rect *loc;
    //the location on screen of the object
    int **frame_offsets;
    //frame_offsets is a 2d array of the x and y offset of the frames of its animation in the texture atlas
    int current_frame;
    //current frame is the current frame in the frame_offsets array, current_frame_in_animation is what frame it currently is out of how many cycles a frame stays on screen for in an animation
    int current_frame_in_animation;
    int frames_per;
    //total amount of frames the sprite has, is up to the user to correctly define this so that it matches the array, frames are indexed at 0 so the frame count goes from 0 to frames_per-1
    int total_frames;
    int w;
    int h;
    //direction the sprite should be rotated in degrees
    int dir;
    //SDL value to determine if the sprite should be flipped on an axis or not
    SDL_RendererFlip flip;

} Sprite_Values;

//background sprite object
typedef struct {
    //the location on screen of the object
    SDL_Rect *loc;
    //width and height of the tile
    int w;
    int h;
    //SDL value to determine if the sprite should be flipped on an axis or not
    SDL_RendererFlip flip;
} BG_Image;

//boundary object
typedef struct {
    //the upper right point of the boundary retangle
    int x;
    int y;
    //the width (positive to right) and height (positive down) of the boundary rectangle
    int w;
    int h;
} Boundary;


//function to do a constant animation on a sprite
void animateLoop(SDL_Objs *obj,  Sprite_Values* values);

//function to animate a single frame from a sprite
void animateStill(SDL_Objs *obj, Sprite_Values *values);

//function to create a Sprite_Values struct
Sprite_Values *createSpriteValues(SDL_Rect *loc, int frames_per, int total_frames, int w, int h, int dir, SDL_RendererFlip flip);

//function to create a background tile
BG_Image *createBGImage(SDL_Rect *loc, int w, int h, SDL_RendererFlip flip);

//function designed specifically to hash Boundary structs, used in conjunction with HashSet to put objects into a HashSet
int BGHash(HashSet *set, void *add);


#endif // DRAWFUNCTIONS_H