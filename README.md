# About imagegrid-viewer: An application for viewing large images in a grid.

The application *imagegrid-viewer* is written in C++ for viewing large
images in a grid.  It was inspired from wanting an application for
displaying freely available Government of Canada NTS (national
topographic system) maps, but one that has a similar feel as to when I
used to pull maps out of the university map cabinet and lay them
side-by-side.

The initial release does not have a lot of features and hogs resources
such as memory and CPU.  However, I have plans to add additional
features and fixes, see [features to be added](#features-to-be-added)
for a preliminary list.  I would like *imagegrid-viewer* to end up
being a fairly useable application for viewing large sets of large
images that are not just limited to maps but could include:

- Images such as satellite photos.
- Medical and scientific images.
- Scanned documents such as old books or comic books.
- Any other collection of images or data.

I don't intend *imagegrid-viewer* to replace more specialized
applications such as GIS, but to simply be a way to quickly view large
collections of image-like data.

# Building

Currently *imagegrid-viewer* has only been built on Debian Linux 10
(buster).  The dependencies are installed by:

> $ sudo apt-get install libsdl2-dev libtiff-dev libpng-dev

Then run `make` in the *imagegrid-viewer* directory to build.

# Usage

To display a list of sequentially numbered images:

> $ ./imagegrid-viewer -w [width of grid in number of images] -h [height of grid in number of images] [list of images]

To display a list of images:

> $ ./imagegrid-viewer -w [width of grid in number of images] -h [height of grid in number of images] -p [path with sequentially numbered images]

The current key bindings are:

```
arrow keys   move around
-            zoom out
=            zoom in
q            quit
```

A gamepad setup as a joystick for SDL will also move and zoom.
Specifically joystick axis 0/1 moves and joystick axis 4 zooms.

# Some examples and placs to get large images

## View a single large image

A example of a large single image is one of the Antenna Galaxy from
https://esahubble.org/images/heic0602a/ with a download link of:

https://esahubble.org/media/archives/images/original/heic0602a.tif

Put the image `heic0602a.tif` into the same directory as
*imagegrid-viewer* and run the command:

> ./imagegrid-viewer -w 1 -h 1 -l ./heic0602a.tif

## Canadian NTS map collection

An example of viewing a set of 30 NTS maps.

On Debian/Ubuntu distributions install `wget` with the command:

> $ sudo apt-get install wget

Download a set up of NTS topographic maps by running the script
`demo-download-canmatrix.sh` in the *imagegrid-viewer* directory with
the command:

> $ ./demo-download-canmatrix.sh


Then run the command:

```
$ ./imagegrid-viewer -w 5 -h 5 ./canmatrix/092j06_01.tif ./canmatrix/092j07_02.tif ./canmatrix/092j08_02.tif ./canmatrix/092i05_02.tif ./canmatrix/092i06_03.tif \
                               ./canmatrix/092j03_03.tif ./canmatrix/092j02_03.tif ./canmatrix/092j01_02.tif ./canmatrix/092i04_03.tif ./canmatrix/092i03_03.tif \
                               ./canmatrix/092g14_03.tif ./canmatrix/092g15_03.tif ./canmatrix/092g16_03.tif ./canmatrix/092h13_02.tif ./canmatrix/092h14_02.tif \
                               ./canmatrix/092g11_03.tif ./canmatrix/092g10_03.tif ./canmatrix/092g09_04.tif ./canmatrix/092h12_02.tif ./canmatrix/092h11_02.tif \
                               ./canmatrix/092g06_06.tif ./canmatrix/092g07_06.tif ./canmatrix/092g08_05.tif ./canmatrix/092h05_04.tif ./canmatrix/092h06_02.tif \
                               ./canmatrix/092g03_05.tif ./canmatrix/092g02_06.tif ./canmatrix/092g01_06.tif ./canmatrix/092h04_09.tif ./canmatrix/092h03_03.tif
```

The above file names were tested on March 25th, 2022.  They are
subject to change due to the versioning of NTS maps.

This area of lower mainland British Columbia that will be shown by
*imagegrid-viewer* is given by the dark black box: ![NTS lower
mainland example](./nts-example.png)

# Features to be added

## Near future

- Variable window sizes and reading the actual screen size rather than
  assuming 1080p.
- Multithreaded conversion of image data into textures, which will
  avoid pauses while scolling or zooming.
- Add more robust error checking.
- Directly use zip files, pdf files, or other file formats images are
  distributed in.
- Cache some data to disk and implement limits on memory usage.
- Configuration settings to help work take advantage of either
  available resources or resource limitations
- Break up large images in memory so the textures used are not much
  larger than screen size. NTS topographic maps can be up to about
  10000x8000, which uses up a lot of memory when loaded into a
  texture.  Astronomy images are often much larger than the
  16384x16384 allowed for an individual texture by SDL.
- Have input and/or specification methods for common sources of images
  such as NTS topo maps.
- Add GUI elements to show information or configure while running.
- Better quality of scaling and zooming of images.
- A near term goal is to be able to load the whole of southern
  BC/Alberta at once and fly around like Google Earth.

## Further future

- Getting it working on Raspberry Pi class hardware.
- Also build on Windows.
- Display latitude/longitude/UTM or other suitable domain specific
  information.
- Work with non-uniform or overlapping grids of images.
- Plot tracks/trails or other interesting data such as the [Space
  Shuttle SRTM](https://www2.jpl.nasa.gov/srtm/) elevation data.
- Points that refer to other files and/or metadata.
- Evaluate if there are better engines than SDL.
