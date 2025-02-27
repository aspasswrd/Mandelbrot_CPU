#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <tuple>

const int WIDTH = 800;
const int HEIGHT = 600;
const int MAX_ITER = 800;

std::vector<std::tuple<uint8_t, uint8_t, uint8_t>> colorTable(MAX_ITER + 1);

long double offsetX = -0.705922586560551705765;
long double offsetY = -0.267652025962102419929;
long double zoom = 0.5;
bool needsRedraw = true;

void initColorTable() {
    for (int iter = 0; iter <= MAX_ITER; ++iter) {
        long double t = static_cast<long double>(iter) / MAX_ITER;
        uint8_t r = static_cast<uint8_t>(9 * (1 - t) * t * t * t * 255);
        uint8_t g = static_cast<uint8_t>(15 * (1 - t) * (1 - t) * t * t * 255);
        uint8_t b = static_cast<uint8_t>(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
        colorTable[iter] = {r, g, b};
    }
}

int calculateMandelbrot(const long double& cx, const long double& cy, int max_iter) {
    long double zx = 0.0;
    long double zy = 0.0;
    int iter = 0;

    long double q = (cx - 0.25) * (cx - 0.25) + cy * cy;
    if (q * (q + (cx - 0.25)) <= 0.25 * cy * cy ||
        (cx + 1.0) * (cx + 1.0) + cy * cy <= 0.0625) {
        return max_iter;
    }

    while (iter < max_iter) {
        long double zx2 = zx * zx;
        long double zy2 = zy * zy;
        if (zx2 + zy2 > 4.0) break;

        long double tmp = zx2 - zy2 + cx;
        zy = 2.0 * zx * zy + cy;
        zx = tmp;
        iter++;
    }
    return iter;
}

void generateMandelbrot(std::vector<uint8_t>& image) {
    long double scaleX = 3.5 / WIDTH / zoom;
    long double scaleY = 2.0 / HEIGHT / zoom;

    #pragma omp parallel for collapse(2) schedule(static)
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            long double cx = (x - WIDTH / 2) * scaleX + offsetX;
            long double cy = (y - HEIGHT / 2) * scaleY + offsetY;

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
                        zoom *= 1.05L;
                        needsRedraw = true;
                        break;
                    case SDLK_q:
                        zoom /= 1.05L;
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
