export CC := "clang"
export CXX := "clang++"
#export CC_LD := "mold"
#export CCXX_LD := "mold"


default: build-run

rebuild:
  samu -C build/

build:
  meson setup build
  samu -C build/

run:
  ./build/src/maple

build-run: build run

clean:
  rm -r build
