#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;


int main(int argc, char *args[]) {

  constexpr int
      TileSize = 32,
      SizeX = 20,
      SizeY = 14;
    
  // map as 2D-array
  int map[SizeY][SizeX];

  // positions in our tileset
  SDL_Point fieldStates [99] {};

  int countX = 0;
  int countY = 0;

  for (int i = 0; i < 99; i++) {
    fieldStates[i] = {.x = countX, .y = countY};
    if (countX >= 8) {
      countX = 0;
      countY++;
    } else {
      countX++;
    }
  }

  SDL_Window * window = SDL_CreateWindow("Mapeditor", 0, 0, 1080, 480, SDL_WINDOW_OPENGL);
  SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
  SDL_Init(SDL_INIT_EVERYTHING);
  IMG_Init(IMG_INIT_PNG);

  // loads the texture
  SDL_Texture * texture = IMG_LoadTexture(renderer, "bd.png");

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  // fills the map with standard tiles
  for (int y = 0; y < SizeY; y++) {
    for (int x = 0; x < SizeX; x++) {
      const bool isBorder = 
          y == 0 || y == (SizeY - 1) || x == 0 || x == (SizeX - 1);
      map[y][x] = isBorder
          ? 55  // hard wall
          : 64; // dirt
    }
  }

  SDL_Event event;
  bool running = true;

  int selection = 64;
  bool tileSelected = false;

  while (running) {
    while (SDL_PollEvent(&event)) {
      // check for window being closed
      if (event.type == SDL_QUIT) {
        running = false;
      } else if (event.type == SDL_KEYDOWN) {
        SDL_KeyboardEvent ke = event.key;
        SDL_Keysym ks = ke.keysym;
        // ESC and Q quits
        if (ks.sym == SDLK_q || ks.sym == SDLK_ESCAPE) {
          running = false;
        } 
        // L_Ctrl + S saves map into txt file
        else if (ks.sym == SDLK_s && (ks.mod & KMOD_LCTRL)) { 
          //exception handling
          try {
            //open file for writing
            fstream fw("File.txt", ios::out);
            
            if (fw.is_open()) {
              //store array content to file
              for (int y = 0; y < SizeY; y++) {
                for (int x = 0; x < SizeX; x++) {
                  fw << map[y][x];
                  if (x != SizeX -1)
                    fw << " ";
                  else
                    fw << "\n";    
                }
              }                                    
              fw.close();
            } else cout << "Problem with opening file";
          }
          catch (const char* msg) {
            cerr << msg << endl;
          }
        } 
        // L_Ctrl + O opens map from txt file
        else if (ks.sym == SDLK_o && (ks.mod & KMOD_LCTRL)) {  
          //open file for reading
          fstream fw("File.txt",ios::in);

          if (fw.is_open()) {              
            // write file content into map array
            for (int y = 0; y < SizeY; y++) {
              for (int x = 0; x < SizeX; x++) {
                fw >> map[y][x];  
              }
            }
            fw.close();
          }
        }
      } else if (event.type == SDL_MOUSEBUTTONDOWN) { 
        SDL_MouseButtonEvent mbe = event.button;
        if (mbe.button == 1) {
          // check if clicked inside map
          if ((mbe.x > 300 && mbe.x <= (300 + SizeX * TileSize)) && (mbe.y >= (0 + TileSize) && mbe.y <= (0 + TileSize + (SizeY * TileSize)))) {           
            // calculate the map position based on mouseclick position
            const SDL_Point mapPos = {
                .x = (int ((mbe.x - 300) / TileSize)),
                .y = (int ((mbe.y - TileSize) / TileSize))
            };
            // set current tile to the fieldstate on this map position, if there is a selected tile
            map[mapPos.y][mapPos.x] = selection;
          }
          // check if clicked inside tileset
          else if ((mbe.x >= 0 && mbe.x <= 288) && (mbe.y >= 0 && mbe.y <= 352)) {
            // calculate the start position in tileset based on mouseclick position
            const SDL_Point tilePos = {
                .x = int (mbe.x / TileSize),
                .y = (int (mbe.y / TileSize))
            };
            // set selection number to the matching tile in fieldstates array
            for (int i = 0; i < 99; i++) {
              if (fieldStates[i].x == tilePos.x && fieldStates[i].y == tilePos.y)
                selection = i;
            }
            tileSelected = true;            
          }
        }
      }
    }      
    // Rendering the map
    for (int y = 0; y < SizeY; ++y) {
      for (int x = 0; x < SizeX; ++x) {
        const int currentField = map[y][x];
        const SDL_Point & selectedState = fieldStates[currentField];
        const SDL_Rect srcrect {
            .x = selectedState.x * TileSize,
            .y = selectedState.y * TileSize,
            .w = TileSize,
            .h = TileSize
        };      
        const SDL_Rect dstrect {
            .x = (x * TileSize) + 300,
            .y = y * TileSize + TileSize,
            .w = TileSize,
            .h = TileSize
        };
        // copy from texture into the renderer
        SDL_RenderCopy(renderer, texture, &srcrect, &dstrect);
      }
    }
    // rendering the tileset
    SDL_Rect destrect {0,0,288,352};
    SDL_RenderCopy(renderer, texture, NULL, &destrect);
    // if a tile is selected make border around it
    if (tileSelected == true) {
      const SDL_Rect border {
          fieldStates[selection].x * TileSize, 
          fieldStates[selection].y * TileSize, 
          TileSize, 
          TileSize
      };
      SDL_SetRenderDrawColor(renderer, 255, 55, 55, 255);
      SDL_RenderDrawRect(renderer, &border);
    }

    SDL_RenderPresent(renderer); 
  }
  // shutdown
  SDL_DestroyTexture(texture);
  IMG_Quit(); 
}

