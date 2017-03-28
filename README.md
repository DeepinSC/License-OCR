# License-OCR
#### AUTHORED BY RICK
## Outline
- Introduction
- OpenCV 
- Main Task
- Image Segmentation
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
The ANPR is designed with several parts: Image Segmentation/SVM for valid plates/ANN for recognization.

### Image Segmentation
In this step, the ANPR is required to detect all possible plates in a specific natural image.

1. Gaussian Filter 5*5: Since the plates are retangle with erect lines, Gaussian filter is able to diminish the noise by camera.
2. Sobel Filter: Sobel Filter can find the erect lines by its horizontal derivative.
3. Otsu Algorithm: the HSV images are transferred into Binary images.
4. findContours: findContours in OpenCV is a function to find the region with specific shapes. In this project, it's used for finding retangles.
5. floodFill: floodFill in OpenCV is a function to cover a region with white color. In this project, it can help to regularize the retangles.
6. Light histogram equalization: LHE is able to fix all images into the same size and brightless.


### SVM
> In machine learning, support vector machines (SVMs, also support vector networks) are supervised learning models with associated learning algorithms that analyze data used for classification and regression analysis. Given a set of training examples, each marked as belonging to one or the other of two categories, an SVM training algorithm builds a model that assigns new examples to one category or the other, making it a non-probabilistic binary linear classifier.

In this step, the ANPR call the SVM algorithm to classify the possible regions from above. SVM is a powerful classification algorithm, which determines whether the regions are real number plates.

1. Training data: Since no number plates data set is available on the web, I could only use the small data sets from my professor, which contains 75 positive samples & 35 negative samples. All the data are saved as XML files.
2. Parameters: the SVM needs many parameters to initialize. In this project, some parameters are determined as follows: Kernel: Linear; Gamma: 1; Coeffient: 0; Degree: 0; Weight: default.
3. Predict: if output is 1, the region will be saved; else, it will be dropped.

### ANN 
> Artificial neural networks (ANNs) or connectionist systems are a computational model used in computer science and other research disciplines, which is based on a large collection of simple neural units (artificial neurons), loosely analogous to the observed behavior of a biological brain's axons. Each neural unit is connected with many others, and links can enhance or inhibit the activation state of adjoining neural units.

In this step, the ANPR finally is able to recognize each number and character in the plates. The ANN algorithm is used in this part, which is a popular method among image processing algorithms.

1. Training data: Like the data used for SVM, the training data of ANN is also saved in XML file.
2. Network Structure: ANN (or BP Network) is flexible in the number of neurons in each layer and the number of layers. In this project, the ANN has 3 layers (one hidden layer), and each layer contains 100/20/30(10 numbers + 20 letters). The activate function for all layers is Sigmoid. Tips: the best parameters come from several training with different parameters combination, like (5*5,10*10,15*15,20*20)input ,(10,200) hidden layers. 
3. character segmentation: the plates contains at least 6 chars. In order to split them apart, we need to cut those numbers via the pre-knowledge about plates. For example, we could know that each character is in a precise position in the plate, so we could cut by these positions.

### Output and result
The output is in the same folder of ANPR.exe. For most license(80%), the numbers are right. However, for some vague plates or non-Spanish plates, the result maybe not good.


### Optimization & Forecast
Admittedly, this project can be optimized by some ways. 

1. Image processing: my knowledge of Image processing area is not plentiful, so there must be some different method to process the images better. In fact, for these methods I used, I'm still not familar with their presences.
2. SVM: Gaussin Kernel is not a good choice since it's easy to overfit. However, the feature of a plate is not that good because at least I know several ways to select features like PCA.
3. ANN: BP or MLP is a relatively old method while CNN is much powerful. However, limited by the use of OpenCV, I cannot implenment a CNN by C++. Therefore, using Opencv in Python, I may apply CNN and its optimization methods(Dropout) for this project.

FIN
