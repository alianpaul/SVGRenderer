# SVGRenderer

A simple svg file toy renderer for practicing the rasterization algorithms I learnt.<br> 
The rasterization algorithm are all CPU-based. I draw the pixels in a memory buffer on CPU side, then use OpenGL API to copy the pixels to GPU side.<br>
Images below are the final results generated by this toy renderer.


<div align=center>

<img src="https://github.com/alianpaul/SVGRenderer/blob/master/Results/lion.png" width="50%" height="50%"><img src="https://github.com/alianpaul/SVGRenderer/blob/master/Results/sun.png" width="50%" height="50%">

</div>

# Features

## Rasterization
* Line<br>
I use [Bresenham algorithm](https://www.cs.helsinki.fi/group/goa/mallinnus/lines/bresenh.html) to draw the line.<br>
I also tried [Xiaolin Wu's line algorithm](https://unionassets.com/blog/algorithm-brezenhema-and-wu-s-line-299).It's an anti-aliasing algoritm<br>

* Triangle<br>
I use early out strategy. I find the bounding box of the triangle first, then test if each block in the bounding box intersects with the edge of the triangle.

* Polygon Rectangle<br>

## Anti-aliasing
* MLAA<br>
I implement MLAA (Morphological anti-aliasing) rather than supersampling because it's more effcient! 
This is just a naive CPU version implementation of the original 
[MLAA algorithm presented by Alexander Reshetov](http://www.cs.cmu.edu/afs/cs/academic/class/15869-f11/www/readings/reshetov09_mlaa.pdf). Vector intrinsics (SIMD registers of CPU) can be used to boost up the edge detection process. <br>
Image on the right is the anti-alias result.
<div align=center>

<img src="https://github.com/alianpaul/SVGRenderer/blob/master/Results/unantialias.png" width="50%" height="50%"><img src="https://github.com/alianpaul/SVGRenderer/blob/master/Results/antialias.png" width="50%" height="50%">

</div>

* Mipmap<br>
I implement mipmap to decrease the alias introduced by down-sampling a high resolution picture.<br>
Image on the right is rendered by using mipmap
<div align=center>

<img src="https://github.com/alianpaul/SVGRenderer/blob/master/Results/unmipmap.png" width="50%" height="50%"><img src="https://github.com/alianpaul/SVGRenderer/blob/master/Results/mipmap.png" width="50%" height="50%">

</div>