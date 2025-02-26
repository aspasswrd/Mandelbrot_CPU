#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <tuple>
#include <gmpxx.h>

const int WIDTH = 400;
const int HEIGHT = 300;
const int MAX_ITER = 1000;

std::vector<std::tuple<uint8_t, uint8_t, uint8_t>> colorTable(MAX_ITER + 1);

mpf_class offsetX("-0.705922586560551705765", 256);
mpf_class offsetY("-0.267652025962102419929", 256);
mpf_class zoom("0.5", 256);
bool needsRedraw = false;

void initColorTable() {
    for (int iter = 0; iter <= MAX_ITER; ++iter) {
        long double t = static_cast<long double>(iter) / MAX_ITER;
        uint8_t r = static_cast<uint8_t>(9 * (1 - t) * t * t * t * 255);
        uint8_t g = static_cast<uint8_t>(15 * (1 - t) * (1 - t) * t * t * 255);
        uint8_t b = static_cast<uint8_t>(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
        colorTable[iter] = {r, g, b};
    }
}

int calculateMandelbrot(const mpf_class& cx, const mpf_class& cy, int max_iter) {
    mpf_class zx = 0.0;
    mpf_class zy = 0.0;
    int iter = 0;

    // Оптимизация для главной кардиоиды и круга
    mpf_class q = (cx - 0.25) * (cx - 0.25) + cy * cy;
    if (q * (q + (cx - 0.25)) <= 0.25 * cy * cy ||
        (cx + 1.0) * (cx + 1.0) + cy * cy <= 0.0625) {
        return max_iter;
    }

    while (iter < max_iter) {
        mpf_class zx2 = zx * zx;
        mpf_class zy2 = zy * zy;
        if (zx2 + zy2 > 4.0) break;

        mpf_class tmp = zx2 - zy2 + cx;
        zy = 2.0 * zx * zy + cy;
        zx = tmp;
        iter++;
    }
    return iter;
}

void generateMandelbrot(std::vector<uint8_t>& image) {
    mpf_class scaleX = 3.5 / WIDTH / zoom;
    mpf_class scaleY = 2.0 / HEIGHT / zoom;

    #pragma omp parallel for collapse(2) schedule(dynamic)
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            mpf_class cx = (x - WIDTH / 2) * scaleX + offsetX;
            mpf_class cy = (y - HEIGHT / 2) * scaleY + offsetY;

            int iter = calculateMandelbrot(cx, cy, MAX_ITER);

            auto [r, g, b] = colorTable[iter];
            int idx = (y * WIDTH + x) * 3;
            image[idx] = r;
            image[idx + 1] = g;
            image[idx + 2] = b;
        }
    }
}

int main() {
    mpf_set_default_prec(256); // Установка точности

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Mandelbrot Set", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    initColorTable();
    std::vector<uint8_t> image(WIDTH * HEIGHT * 3);

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_w:
                        offsetY -= (0.1f / zoom);
                    needsRedraw = true;
                    break;
                    case SDLK_s:
                        offsetY += (0.1f / zoom);
                    needsRedraw = true;
                    break;
                    case SDLK_a:
                        offsetX -= (0.1f / zoom);
                    needsRedraw = true;
                    break;
                    case SDLK_d:
                        offsetX += (0.1f / zoom);
                    needsRedraw = true;
                    break;
                    case SDLK_e:
                        zoom *= 1.1f;
                    needsRedraw = true;
                    break;
                    case SDLK_q:
                        zoom /= 1.1f;
                    needsRedraw = true;
                    break;
                }
            }
        }

        if (needsRedraw) {
            generateMandelbrot(image);
            needsRedraw = false;
        }

        SDL_UpdateTexture(texture, nullptr, image.data(), WIDTH * 3);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
