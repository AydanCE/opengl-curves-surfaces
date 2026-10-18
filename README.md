# OpenGL Curves and Surfaces

Computer graphics assignment implementing Bezier/B-spline curve evaluation and swept surfaces using OpenGL and C++.

## Features

- Cubic Bezier curve evaluation with TNB (Tangent/Normal/Binormal) Frenet frame
- B-spline curve evaluation via basis conversion to Bezier
- Surface of Revolution (rotate profile curve around Y axis)
- Generalized Cylinder (sweep profile along 3D curve using TNB frame)
- Interactive 3D viewer with mouse rotation and keyboard controls

## Controls

- `s` — toggle between shaded / wireframe+normals / hidden
- `c` — toggle curve display
- `p` — toggle control points
- `Space` — reset camera
- Mouse drag — rotate view

## How to Run

```cmd
g++ main.cpp parse.cpp curve.cpp surf.cpp camera.cpp vecmath/src/*.cpp -I "path/to/include" -L "path/to/lib" -lfreeglut -lglu32 -lopengl32 -lgdi32 -lwinmm -o a1.exe
.\a1.exe swp\yourfile.swp
```

## SWP File Format
- bez2 name steps numPoints # 2D Bezier curve
- bsp2 name steps numPoints # 2D B-spline curve
- srev name steps profileName # Surface of revolution
- gcyl name profileName sweepName # Generalized cylinder

## Custom Models
- swp/test.swp — Christmas tree shape
