#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <tuple>
#include <immintrin.h>

const int WIDTH = 1920;
const int HEIGHT = 1080;
const int MAX_ITER = 1200;

std::vector<std::tuple<uint8_t, uint8_t, uint8_t>> colorTable(MAX_ITER + 1);

double offsetX = -0.705922586560551705765;
double offsetY = -0.267652025962102419929;
double zoom = 0.5;
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

// Векторизованная версия функции
__attribute__((target("avx2")))
void calculateMandelbrotAVX(__m256d* cx, __m256d* cy, int* results) {
    __m256d zx = _mm256_setzero_pd();
    __m256d zy = _mm256_setzero_pd();
    __m256i iter = _mm256_setzero_si256();
    __m256i mask = _mm256_setzero_si256();

    for (int i = 0; i < MAX_ITER; ++i) {
        __m256d zx2 = _mm256_mul_pd(zx, zx);
        __m256d zy2 = _mm256_mul_pd(zy, zy);
        __m256d r2 = _mm256_add_pd(zx2, zy2);
        __m256d cmp = _mm256_cmp_pd(r2, _mm256_set1_pd(4.0), _CMP_LT_OQ);

        mask = _mm256_castpd_si256(cmp);
        if (_mm256_testz_si256(mask, mask)) break;

        __m256d tmp = _mm256_add_pd(_mm256_sub_pd(zx2, zy2), *cx);
        zy = _mm256_add_pd(_mm256_mul_pd(_mm256_set1_pd(2.0), _mm256_mul_pd(zx, zy)), *cy);
        zx = tmp;

        iter = _mm256_sub_epi64(iter, _mm256_castpd_si256(cmp));
    }

    _mm256_storeu_si256((__m256i*)results, iter);
}

void generateMandelbrot(std::vector<uint8_t>& image) {
    double scaleX = 3.5 / (WIDTH * zoom);
    double scaleY = 2.0 / (HEIGHT * zoom);

    #pragma omp parallel for schedule(static)
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x += 4) {  // Обрабатываем 4 точки за итерацию
            double cx[4], cy[4];
            for (int i = 0; i < 4; i++) {
                int xx = x + i;
                cx[i] = (xx - WIDTH/2) * scaleX + offsetX;
                cy[i] = (y - HEIGHT/2) * scaleY + offsetY;
            }

            __m256d vcx = _mm256_loadu_pd(cx);
            __m256d vcy = _mm256_loadu_pd(cy);
            int results[4];
            calculateMandelbrotAVX(&vcx, &vcy, results);

            for (int i = 0; i < 4; i++) {
                if (x + i >= WIDTH) break;
                auto [r, g, b] = colorTable[results[i]];
                int idx = (y * WIDTH + x + i) * 3;
                image[idx] = r;
                image[idx + 1] = g;
                image[idx + 2] = b;
            }
        }
    }
}

// Остальной код остается без изменений

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
