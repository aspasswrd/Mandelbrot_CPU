[![Beta](https://img.shields.io/badge/Status-Beta-orange)](https://github.com/aspasswrd/Mandelbrot_CPU/releases/tag/beta1.0)  

# Mandelbrot_CPU
Rendering the Mandelbrot Set on CPU with Dynamic Zooming

Three rendering modes are implemented:
1. **Default Version** (long double precision)  
2. **AVX-Optimized Version** (vectorized for speed)  
3. **High-Precision Version** (using Boost.Multiprecision for deep zoom)  

![preview](preview.jpg)

---

## 📋 Requirements
- **OS**: Linux (Ubuntu 22.04+ recommended), Windows (WSL2/MSYS2), macOS
- **Compiler**: GCC 12+ / Clang 15+ (with C++26 support)
- **CPU**: AVX2 support required for the AVX-optimized version

---

## 📦 Dependencies Installation

### For All Versions:
```bash
# Ubuntu/Debian
sudo apt-get install build-essential libsdl2-dev libomp-dev

# macOS (Homebrew)
brew install sdl2 libomp

# Windows (vcpkg)
vcpkg install sdl2 openmp
```

### Additional Dependencies for High-Precision Version:
```bash
# Ubuntu/Debian
sudo apt-get install libboost-dev

# macOS (Homebrew)
brew install boost

# Windows (vcpkg)
vcpkg install boost
```

---

## 🛠 Building and Running

### Via Terminal:
1. Clone the repository:
```bash
git clone https://github.com/yourusername/Mandelbrot_CPU.git
cd Mandelbrot_CPU
```

2. Build the desired version:
```bash
# Default version
cmake -DVERSION=DEFAULT -B build && cmake --build build -j4

# AVX-optimized version (requires AVX2)
cmake -DVERSION=AVX -B build && cmake --build build -j4

# High-precision version
cmake -DVERSION=HIGH_PRECISION -B build && cmake --build build -j4
```

3. Run the program:
```bash
./build/mandelbrot
```

### Controls:
- **WASD** – Pan
- **Q/E** – Zoom out/in
- **R** – Reset to default position (high-precision version only)
- **ESC** – Quit

---

## 🔍 Checking AVX Support
Before using the AVX-optimized version, ensure your CPU supports AVX2:
```bash
grep avx2 /proc/cpuinfo  # Linux
sysctl machdep.cpu.features | grep AVX2  # macOS
```

---

## ⚠️ Notes
- The high-precision version is slow – use `WIDTH=200`, `HEIGHT=200` in the code for deep zoom.
- On Windows, environment variables for Boost and SDL2 must be configured properly.

---

License: [MIT](LICENSE)
