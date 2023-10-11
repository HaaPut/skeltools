# Introduction
Tools related to skeletonization of objects. 
The codes requires the [ITK](https://itk.org/) library to work and `CMake` to build.
# Compilation
* Create a build directory for compilation
```bash
$ mkdir build
```
* Set the `ITK_DIR` variable inside the `CMakeLists.txt`. 
The exact path will depend on the location where ITK is installed on your machine.
Run following command (with appropriate `path/to/ITK`) to set the variable.
```bash
$sed -i '10s/.*/set(ITK_DIR \/path\/to\/ITK)/' CMakeLists.txt
``` 
* Change into the build directory and generate make files using cmake
```bash
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
```
* Compile using `make`
```bash
$ make -j2
```
If all goes well you should now have `skeltools` executable insed the build folder.

# Computing object centerline.
To compute (boundary distance weighted) medial curve for `dinosaur` sample image run
```bash
$ skeltools -priority distance -curve -input data/dinosaur.tif -smooth 0.2  -fillholes -outputFolder results
```
If everything runs without errors you should see `dinosaur_distance_curve.tif` inside `results` folder.

<figure style="display:block; margin: 0 auto; text-align: center; width:600px">
<img alt="dinosaur-noanchor-unweighted" src="images/dino-medialcurve-unweighted-noanchor.png"/>
<figcaption>Sample dinosaur object in translucent blue with reddish colored medial curve inside</figcaption>
</figure>

# Computing object medial surface
To compute medial surface for `dinosaur` sample image run
```bash
$ skeltools -priority distance -surface -input data/dinosaur.tif -smooth 0.1  -fillholes -output results/dino_surface.tif
```
The output of the above command should be the medial surface `dino_surface.tif` inside the `results` folder.
<figure style="display:block; margin: 0 auto; text-align: center; width:600px">
<img alt="dinosaur-aofanchor-unweighted-surface" src="images/dino-medialsurface-unweighted-aofanchor.png"/>
<figcaption>Sample dinosaur object in translucent blue with reddish colored medial surface inside</figcaption>
</figure>

# Mapping object thickness to object boundary
`MapToBoundaryImageFilter` can be used to map features (for example object width) from medial surface to the object boundary. Example 
`map-medial-thickness-to-boundary.cpp` computes object width weighted medial skeleton of `dinosaur.tif' and maps it back to 
the boundary.
<figure style="display:block; margin: 0 auto; text-align: center; width:600px">
<img alt="dinosaur-mapped-object-width" src="images/dino-boundary-mapped-thickness.png"/>
<figcaption>Visualization of object thickness to object boundary for dinosaur sample image</figcaption>
</figure>

