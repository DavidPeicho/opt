# WGPU Pathtracer

Rough attempt at creating a real-time Pathtracer in C++ using [wgpu-native](https://github.com/gfx-rs/wgpu-native)

<p align="center">
  <img src="./demo.gif"></img>
</p>

🚧 **STATUS**: this project was only created as a playground for WebGPU. I will not
continue to work on it.

If you wish to follow the development of this project, I am working right now
on a clean and stable version in Rust in the [Albedo organization](https://github.com/albedo-engine/albedo).

## Build

This repository was supposed to be a quick and dirty try out of wgpu. Unfortunately,
I only cared to build on my Macbook and I didn't do anything clean or portable.

In order to build this, you need will to:
* Compile the shaders in `src/shaders/` to SPIR-V
* Download [wgpu-native](https://github.com/gfx-rs/wgpu-native) (using Cargo for instance) and link it
* Download [GLFW](glfw.org) and link it

The scene loaded is also hardcoded in the `main.cpp` file.

## Features

### Available

* Basic SAH BVH construction (CPU)
* BVH flattening with hit/miss links for stackless traversal
* Coarse Monte Carlo Pathtracing algorithm with Lambert BRDF
* HDR Background

### Missing

* Importance Sampling
* Microfacet Model
* Texture Sampling
* Basically everything I really needed to work on :)

## References

### Rendering

* [Pharr M., Jakob .W, & Humphreys .G, Physically Based Rendering: From Theory To Implementation](http://www.pbr-book.org/)
* [Alan Wolfe Blog](https://blog.demofox.org/2020/05/25/casual-shadertoy-path-tracing-1-basic-camera-diffuse-emissive/)

### BVH

* http://www.sci.utah.edu/~wald/Publications/2007/ParallelBVHBuild/fastbuild.pdf
* https://www.ci.i.u-tokyo.ac.jp/~hachisuka/tdf2015.pdf

### Color spaces

* https://www.slideshare.net/ozlael/hable-john-uncharted2-hdr-lighting

### Data Design

* [Filament](https://github.com/google/filament) implementation
of a Data-oriented ECS

## Credits

* Thanks to **knightcrawler25** for the test datasets I took on his [repository](https://github.com/knightcrawler25/GLSL-PathTracer).
