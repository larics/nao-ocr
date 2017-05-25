NAO OCR
======
***
Summary
---------------
NAOOCR is a software module that enables the Aldebaran NAO ATOM humanoid robot to read text from an image. The module is entirely written in c++ and can be directly run on the robot. 

Requirements
----------------------
* Naoqi SDK 2.1.4
* OpenCV 2.4.9
* Tesseract 3.02
* Leptonica
* OpenNAO 2.1.4

For building Tesseract and Leptonica on OpenNAO:
* m4
* automake
* autoconfig
* libtool


Building and setup
-----------------------------
For building please read "Projekt[2015]Kokot_Mirko.pdf" page 11 - 14
  
Usage
----------
The module is called by the naoocr program. For the naoocr program to run there must be a nao.config file in the same directory as the program. This file defines the default parameters for the program.

* NAO_IP - the NAO IP address
* NAO_PORT - the port
* NAO_MOVEMENT - example positioning program for better reading
* INPUT_MODE - the input mode. Can be FILE or CAMERA
* OUTPUT_MODE - the output mode. Can be SILENT or SPEAK
* OCR_MODE - the OCR mode. Can be SEGMENTED or WHOLE
* LANGUAGE - OCR language. Can be eng or hrv
* OPERATION_MODE - the operating mode. Can be ONE_TEXT/MULTI_TEXT/MONITOR_TEXT
* RESULT_LOCATION - the location of the file where the result will be saved
* PICTURE_LOCATION - the location of the input picture. Is used only if the input mode is FILE
* DEBUG - boolean to enable debug outputs through steps
* DEBUG_LOCATION - the location of the debug outputs

This arguments can also be directly given to the program. For more information run `naoocr --help`.

