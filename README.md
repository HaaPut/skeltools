# Introduction
Codes for computation of medial surface based on `Average Outward Flux`. 
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
$sed -i '8s/.*/set(ITK_DIR \/path\/to\/ITK)/' CMakeLists.txt
``` 
* Change into the build directory and generate make files using cmake
```bash
$ cd build
$ cmake ..
```
* Compile using `make`
```bash
$ make -j2
```
If all goes well you should now have `skeltools` executable insed the build folder.
# Computing the medial surface.
To Compute the medial curve for `dinosaur` sample image run
```bash
$ skeltools -medialCurve -input samples/dinosaur.tif
```
If everything runs without errors you should see `medialCurve.tif` and other not so important files 
inside a newly created `samples/dinosaur` folder.
