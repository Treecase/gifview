/*
 * main.cpp -- Entry point for GIFView.
 *
 * Copyright (C) 2022 Trevor Last
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <cstdlib>

#include <SDL2/SDL.h>


int main(int argc, char const *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow(
        "GIF View",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        640, 480,
        SDL_WINDOW_RESIZABLE);
    if (window == nullptr)
    {
        SDL_LogCritical(0, "Failed to create window: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    bool running = true;
    while (running)
    {
        SDL_Surface *screen = SDL_GetWindowSurface(window);
        Uint32 color = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
        if (SDL_FillRect(screen, nullptr, color) != 0)
        {
            SDL_LogError(0, "ERROR SDL_FillRect: %s\n", SDL_GetError());
        }
        SDL_FreeSurface(screen);
        if (SDL_UpdateWindowSurface(window) != 0)
        {
            SDL_LogError(
                0, "ERROR SDL_UpdateWindowSurface: %s\n", SDL_GetError());
        }

        SDL_Event event{};
        SDL_WaitEvent(&event);
        switch (event.type)
        {
        case SDL_QUIT:
            running = false;
            break;
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
