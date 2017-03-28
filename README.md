# License-OCR
## Outline
- Introduction
- OpenCV 
- Main Task
- Image Segmentation
- Image Detection
- SVM
- ANN
- Output & result
- Optimization & Forecast
### Introduction

Machine learning OCR for vehicle licenses via SVM & ANN is a project of ANPR.
>ANPR (Automatic Number Plate Recognition), a method to recognize plates, uses OCR (Optical Character Recognition) 
and Image Segmentation & Image Detection.

The main lib used for this project is OpenCV 2 while the language is C++ in VS2013.

### OpenCV
> OpenCV is released under a BSD license and hence it’s free for both academic and commercial use. 
It has C++, C, Python and Java interfaces and supports Windows, Linux, Mac OS, iOS and Android. 
OpenCV was designed for computational efficiency and with a strong focus on real-time applications. 
Written in optimized C/C++, the library can take advantage of multi-core processing. - from offcial web of OpenCV
Enabled with OpenCL, it can take advantage of the hardware acceleration of the underlying heterogeneous compute platform.

In the project, OpenCV provides sorts of functions working for the image processing. For example, using function 'cvtColor' 
to transfer BGR image into HSV. These functions in the lib are able to simplify our work in image processing.

OpenCV also provides several classic algorithms in machine learning like SVM and ANN (BP Neural Network). It's not necessary to train these model
by ourselves. More info in the next parts.

### Main Task
In this project, the target are first to detect the region which contains a number plate, and then to recognize the precise number.
In fact, it's much more easy to recongize number in the IR images because the regions containing plate are highly visible. 
However, this ANPR is trying to finish the task through natural images. For this reason, the accuracy of the result is not perfect.
The ANPR is designed with several parts: Image Segmentation/Image Detection/SVM for valid plates/ANN for recognization.

### Image Segmentation
In this step, the ANPR is required to detect all possible plates in a specific natural image. 
1. Gaussian Filter 5*5: Since the plates are retangle with erect lines, Gaussian filter is able to diminish the noise by camera.
2. Sobel Filter: Sobel Filter can find the erect lines by its horizontal derivative.
3. Otsu Algorithm: the HSV images are transferred into Binary images.
4. findContours: findContours in OpenCV is a function to find the region with specific shapes. In this project, it's used for finding retangles.
5. floodFill: floodFill in OpenCV is a function to cover a region with white color. In this project, it can help to regularize the retangles.

### Image Detection

### SVM

### ANN 

### Output and result

### Optimization & Forecast
