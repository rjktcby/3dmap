## 3dmap

An intercative 3D map

## Building (Mac)

        brew install curl libpng cmake git glew wget
        wget https://github.com/glfw/glfw/releases/download/3.1.1/glfw-3.1.1.zip
        unzip glfw-3.1.1.zip
        cd glfw-3.1.1
        mkdir build
        cd build
        cmake .. -DBUILD_SHARED_LIBS=1
        make -j 4
        sudo make install
        cd ../..
        git clone https://github.com/rjktcby/3dmap
        cd 3dmap
        make -f Makefile.mac

## Controls

* x-rotation: W-S
* y-rotation: A-D
* z-rotation: Q-Z
* camera movement: mouse
* zoom: scroll events

