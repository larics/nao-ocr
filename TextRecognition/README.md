NAO OCR
======
***
Summary
---------------
NAOOCR is a software module that enables the Aldebaran NAO ATOM humanoid robot to read text from an image. The module is entirely written in c++ and can be directly run on the robot. In the making of NAOOCR two additional open source libraries are used: 

* [OpenCV](opencv.willowgarage.com/): for help in image processing 
* [UTF8-CPP](http://utfcpp.sourceforge.net/): for help in text processing

NOTE: For now only Arial and Times New Roman fonts are supported.

Requirements
----------------------
* Naoqi 1.4 or higher
* OpenCV 2.3 or higher
 
Building and setup
-----------------------------
For building the module please consider the [qibuild 1.14](http://www.aldebaran-robotics.com/documentation/qibuild/getting_started.html) documentation. Also checkout the tutorial on [creating new modules](http://www.aldebaran-robotics.com/documentation/dev/cpp/tutos/create_module.html).
  
Usage
----------
The module is called by the naoocr program. For the naoocr program to run there must be a nao.config file in the same directory as the program. This file defines the default parameters for the program. An example of this config file is given below:

		/*-----------DEFAULT PARAMETERS----------*/
		NAO_IP=127.0.0.1
		NAO_PORT=9559
		INPUT_MODE=FILE
		OUTPUT_MODE=SILENT
		OPERATION_MODE=ONE_TEXT
		RESULT_LOCATION=/home/nyquist/result.txt
		PICTURE_LOCATION=/home/nyquist/picture.png
		DATABASE_LOCATION=/home/nyquist/development/database/

* NAO_IP - the NAO IP address
* NAO_PORT - the port
* INPUT_MODE - the input mode. Can be FILE or CAMERA
* OUTPUT_MODE - the output mode. Can be SILENT or SPEAK
* OPERATION_MODE - the operating mode. Can be ONE_TEXT/MULTI_TEXT/MONITOR_TEXT
* RESULT_LOCATION - the location of the file where the result will be saved
* PICTURE_LOCATION - the location of the input picture. Is used only if the input mode is FILE
* DATABASE_LOCATION - the location of the database

This arguments can also be directly given to the program. For more information run `naoocr --help`.

Database
---------------
The database needs to be in a special format. For now only Arial and Times New Roman fonts are available. There are two databases, one for the Croatian alphabet + special signs, and one for the English alphabet. For deleting an letter from your database, you need to delete the letters information from the database.txt file. For deleting a whole font, you can delete the font name from the fonts.txt file. The database can be found in the download section.

Documentation
------------------------
A detailed documentation is accessible through the Doxyfile.

For all other information and suggestions please contact me at: <nyquistDev@outlook.com>.