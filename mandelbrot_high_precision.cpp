#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <tuple>
#include <boost/multiprecision/cpp_bin_float.hpp>

using namespace boost::multiprecision;
using high_precision = cpp_bin_float_100;

const int WIDTH = 200;
const int HEIGHT = 200;
const int MAX_ITER = 1200;

std::vector<std::tuple<uint8_t, uint8_t, uint8_t>> colorTable(MAX_ITER + 1);

high_precision offsetX("-0.705922586560551705765");
high_precision offsetY("-0.267652025962102419929");
high_precision zoom("0.5");
bool needsRedraw = true;

void initColorTable() {
    for (int iter = 0; iter <= MAX_ITER; ++iter) {
        double t = static_cast<double>(iter) / MAX_ITER;
        uint8_t r = static_cast<uint8_t>(9 * (1 - t) * t * t * t * 255);
        uint8_t g = static_cast<uint8_t>(15 * (1 - t) * (1 - t) * t * t * 255);
        uint8_t b = static_cast<uint8_t>(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
        colorTable[iter] = {r, g, b};
    }
}

int calculateMandelbrotHighPrecision(const high_precision& cx, const high_precision& cy) {
    high_precision zx = 0;
    high_precision zy = 0;
    int iter = 0;

    high_precision q = pow(cx - 0.25, 2) + pow(cy, 2);
    if (q * (q + (cx - 0.25)) <= 0.25 * pow(cy, 2) ||
        pow(cx + 1, 2) + pow(cy, 2) <= 0.0625) {
        return MAX_ITER;
    }

    while (iter < MAX_ITER) {
        high_precision zx2 = zx * zx;
        high_precision zy2 = zy * zy;
        if (zx2 + zy2 > 4) break;

        high_precision tmp = zx2 - zy2 + cx;
        zy = 2 * zx * zy + cy;
        zx = tmp;
        iter++;
    }
    return iter;
}

void generateMandelbrotHighPrecision(std::vector<uint8_t>& image) {
    high_precision scaleX = 3.5 / (WIDTH * zoom);
    high_precision scaleY = 2.0 / (HEIGHT * zoom);

    #pragma omp parallel for collapse(2) schedule(static)
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            high_precision cx = (x - WIDTH/2) * scaleX + offsetX;
            high_precision cy = (y - HEIGHT/2) * scaleY + offsetY;

            int iter = calculateMandelbrotHighPrecision(cx, cy);

            auto [r, g, b] = colorTable[iter];
            int idx = (y * WIDTH + x) * 3;
            image[idx] = r;
            image[idx + 1] = g;
            image[idx + 2] = b;
        }
    }
}

int main() {
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
                        offsetY -= (0.1L / zoom);
                        needsRedraw = true;
                        break;
                    case SDLK_s:
                        offsetY += (0.1L / zoom);
                        needsRedraw = true;
                        break;
                    case SDLK_a:
                        offsetX -= (0.1L / zoom);
                        needsRedraw = true;
                        break;
                    case SDLK_d:
                        offsetX += (0.1L / zoom);
                        needsRedraw = true;
                        break;
                    case SDLK_e:
                        zoom *= high_precision("1.05");
                        needsRedraw = true;
                        break;
                    case SDLK_q:
                        zoom /= high_precision("1.05");
                        needsRedraw = true;
                        break;
                    case SDLK_r:  // Сброс масштаба
                        zoom = 0.5;
                        offsetX = -0.705922586560551705765;
                        offsetY = -0.267652025962102419929;
                        needsRedraw = true;
                        break;
                }
            }
        }

        if (needsRedraw) {
            generateMandelbrotHighPrecision(image);
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
