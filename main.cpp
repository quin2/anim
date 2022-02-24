#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <math.h>
#include <thread>
#include <chrono>

struct MyPoint{
  int x;
  int y;
};

void DrawCircle(SDL_Renderer * renderer, int32_t centreX, int32_t centreY, int32_t radius)
{
   const int32_t diameter = (radius * 2);

   int32_t x = (radius - 1);
   int32_t y = 0;
   int32_t tx = 1;
   int32_t ty = 1;
   int32_t error = (tx - diameter);

   while (x >= y)
   {
      SDL_RenderDrawLine(renderer, centreX + x, centreY - y, centreX + x, centreY + y);
      SDL_RenderDrawLine(renderer, centreX - x, centreY - y, centreX - x, centreY + y);
      SDL_RenderDrawLine(renderer, centreX + y, centreY - x, centreX + y, centreY + x);
      SDL_RenderDrawLine(renderer, centreX - y, centreY - x, centreX - y, centreY + x);

      if (error <= 0)
      {
         ++y;
         error += ty;
         ty += 2;
      }

      if (error > 0)
      {
         --x;
         tx += 2;
         error += (tx - diameter);
      }
   }
}

double dist(MyPoint p1, MyPoint p2){
  return sqrt(((p1.x - p2.x) * (p1.x - p2.x)) + ((p1.y - p2.y) * (p1.y - p2.y)));
}

SDL_Surface * saveScreenClip(SDL_Renderer * renderer, int32_t width, int32_t height){
  const Uint32 format = SDL_PIXELFORMAT_ARGB8888;

  SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, format);
  SDL_RenderReadPixels(renderer, NULL, format, surface->pixels, surface->pitch);

  SDL_SaveBMP(surface, "shot.bmp");

  return surface;
}

/*
we don't support rendering surfaces directly....
SDL_CreateTextureFromSurface

*/

void drawPointList(SDL_Renderer * renderer, std::vector<MyPoint> pointList){
  for(int i = 1; i < pointList.size(); i++){
    MyPoint p1 = pointList[i - 1];
    MyPoint p2 = pointList[i];

    double mag = dist(p2, p1);
    double v1 = (p1.x - p2.x) / mag;
    double v2 = (p1.y - p2.y) / mag;

    for(double t = 0; t < mag; t += 2.5){
      double su = p2.x + (t * v1);
      double sv = p2.y + (t * v2);

      DrawCircle(renderer, su, sv, 5);
    }      
  }

  return;
}

int main()
{
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "Failed to initialize the SDL2 library\n";
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow("animmmm",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          680, 480,
                                          0);

    if(!window)
    {
        std::cout << "Failed to create window\n";
        return -1;
    }

    SDL_Renderer *s = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED) ;
    
    //SDL_SetRenderDrawColor(s, 255, 0, 0, 255);
    //SDL_RenderClear(s);

    SDL_Event e;
    bool quit = false;
    int xMouse, yMouse;

    std::vector<MyPoint> pointList;
    std::vector<MyPoint> oldPointList;

    SDL_Surface *history [50]; //let's assume there are 10 for now.
    int frameLimit = 50;
    int frameCounter = 0;
    int frameClock = 0;

    bool clicked = false;

    bool editMode = true;

    while (!quit){
      if(editMode){ // in edit mode, allow canvas edits, etc
        SDL_SetRenderDrawColor(s, 255, 255, 255, 255);
        SDL_RenderClear(s); //without clearing, we can get stuff to persist

          SDL_SetRenderDrawColor(s, 192, 192, 192, 255);
          drawPointList(s, oldPointList);
  
        SDL_SetRenderDrawColor(s, 255, 0, 0, 255);
        drawPointList(s, pointList);

        SDL_RenderPresent(s);

          while (SDL_PollEvent(&e)){
              if (e.type == SDL_QUIT){
                  quit = true;
              }
              if (e.type == SDL_KEYDOWN){
                  if(e.key.keysym.sym == SDLK_SPACE){
                    //clear and play anim
                    editMode = false;
                  }
              }
              if (e.type == SDL_MOUSEBUTTONDOWN){
                clicked = true;
              }
              if(e.type == SDL_MOUSEBUTTONUP){
                // save surface
                if(frameCounter < frameLimit){
                  history[frameCounter] = saveScreenClip(s, 680, 480);
                  frameCounter++;
                }
                
                // clear out data arrays
                clicked = false;
                oldPointList.clear(); //without this depth is 2
                oldPointList = pointList;
                pointList.clear();
                
              }
              if(e.type == SDL_MOUSEMOTION){
                if(clicked){
                  SDL_GetMouseState(&xMouse,&yMouse);
                  pointList.push_back(MyPoint{xMouse, yMouse});
                }
              }
          }
      } 
      else { //show animation
        while (SDL_PollEvent(&e)){ //quit out if we want!
          if (e.type == SDL_QUIT){
            quit = true;
          }
          if (e.type == SDL_KEYDOWN){
            if(e.key.keysym.sym == SDLK_SPACE){
              frameCounter = 0;
              frameClock = 0;
              pointList.clear();
              oldPointList.clear();

              //switch out
              editMode = true;
            }
          }
        }

        if(frameClock < frameCounter){
          SDL_SetRenderDrawColor(s, 255, 255, 255, 255);
          SDL_RenderClear(s);

          SDL_Texture * hi = SDL_CreateTextureFromSurface(s, history[frameClock]);
          SDL_RenderCopy(s, hi, NULL, NULL);

          SDL_RenderPresent(s);

          frameClock++;
          std::this_thread::sleep_for(std::chrono::milliseconds(82)); //snooz

          SDL_DestroyTexture(hi);
        } 
        
        //don't show onion skin
        //at end, switch back to polling to allow user to close window

      }
      
    }

    //clean out those surfaces
    for(int i = 0; i < frameCounter; i++){
      SDL_FreeSurface(history[i]);
    }

    SDL_DestroyWindow(window);
    // We have to destroy the renderer, same as with the window.
    SDL_DestroyRenderer(s);
    SDL_Quit();
}

