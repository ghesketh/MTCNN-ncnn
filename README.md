# MTCNN-ncnn
[MTCNN](https://arxiv.org/abs/1604.02878v1) for Tencent's [ncnn](https://github.com/Tencent/ncnn), based on [cpuimage's](https://github.com/cpuimage/MTCNN) C++ project.

## Projects
[libmtcnn](https://github.com/ghesketh/MTCNN-ncnn/tree/master/libmtcnn) is a static library.

[mtcnn](https://github.com/ghesketh/MTCNN-ncnn/tree/master/mtcnn) is a shared library.

[mtcnn-stdout](https://github.com/ghesketh/MTCNN-ncnn/tree/master/mtcnn-stdout) is a usage example that links to the static library.

[mtcnn-window](https://github.com/ghesketh/MTCNN-ncnn/tree/master/mtcnn-windows) is a usage example that links to the shared library.

## Dependencies
**libmtcnn** and **mtcnn** depend on [ncnn](https://github.com/Tencent/ncnn).

**mtcnn-stdout** depends on libmtcnn.

**mtcnn-window** depends on mtcnn, [SDL2](http://libsdl.org/download-2.0.php), and [SDL2_image](https://www.libsdl.org/projects/SDL_image/).

## Parameters
mtcnn-stdout and mtcnn-window accept command-line parameters:
* -n {minimum face size}
* -x {maximum face size}
* {image file name}

These can be a ratio between 0 and 1 (relative to the smaller side of the image) or an absolute number of pixels.  The limit for minimum is 12 pixels, which is the size of the model used in the first convolution.  The limit for maximum is the value of the smaller side of the image.

## API
For now, see [mtcnn.h](https://github.com/ghesketh/MTCNN-ncnn/blob/master/mtcnn.h), along with examples in [mtcnn-stdout.cpp](https://github.com/ghesketh/MTCNN-ncnn/blob/master/mtcnn-stdout.cpp) and [mtcnn-window.cpp](https://github.com/ghesketh/MTCNN-ncnn/blob/master/mtcnn-window.cpp).  More documentation will follow.
