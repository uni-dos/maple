export CC := "clang"
export CXX := "clang++"
export CC_LD := "mold"
export CCXX_LD := "mold"

rebuild:
  ninja -C build/
  
build:
  meson build
  ninja -C build/

run:
  ./build/src/maple

build-run: build run

clean:
  rm -r build
