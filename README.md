# wgpu-pathtracer
WebGPU Pathtracer library + binary

## HOW TO

* Instance: contains material + pointer to BVH
* BVH contains node with AABB + leaf that are 3 vertices

How to do that with only one BVH when the user asks?

## TOOD

* Adds tinygltf as a cmake dependency. Adds the loader as an optional .so


## References

### BVH

* http://www.sci.utah.edu/~wald/Publications/2007/ParallelBVHBuild/fastbuild.pdf
