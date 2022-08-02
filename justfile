export CC := "clang"
export CC_LD := "mold"

build:
  meson build
  ninja -C build/

run:
  ./build/src/maple

build-run: build run

clean:
  rm -r build
