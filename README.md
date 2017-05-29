NAO OCR
---
Summary
---------------
NAOOCR is a software module that enables the Aldebaran NAO ATOM humanoid robot to read text from an image. The module is entirely written in c++ and can be directly run on the robot. 

## Building and setup as a remote module
Remote module is run on the remote computer that is connected to the robot via Ethernet or WiFi (we suggest Ethernet as image from the camera is sent to the module). 
Building a remote module is the easiest way to set up and try the nao-ocr package. 
### Dependencies and requirements
Nao-ocr is built and tested on Ubuntu 14.04 and 16.04. For 16.04 you need to downgrade gcc to version 4.8. Other dependencies are:

 - Naoqi C++ SDK 2.1.4
 - OpenCV 2.4.9
 - Tesseract 3.02
 - Leptonica
 - gcc 4.8

### Build nao-ocr as remote module
Download and install naoqi C++ SDK (v2.1.4) using this guide:
http://doc.aldebaran.com/2-1/dev/cpp/install_guide.html
Next, you need to fix OpenCV installation in the SDK by downloading and building OpenCV 2.4.9.  Then, copy all OpenCV libraries in the corresponding folder inside the C++ SDK. You may use the following guide to see which dependencies you should install before building OpenCV:
http://www.pyimagesearch.com/2016/10/24/ubuntu-16-04-how-to-install-opencv/
Next, install tesseract and leptonica libraries:
`sudo apt install libtesseract-dev libleptonica-dev`
You may also want to install tesseract-ocr and any other tesseract language packages you think you will need (tesseract-ocr will install support for english language).
Finally, in your qibuild worktree, clone this repo, configure and build. **Make sure to set the flag TXTREC_IS_REMOTE to ON in CMakeLists.txt**.
### Running the nao-ocr module as remote
To run the module, go to build/sdk/bin and run the OcrModule by passing the robot IP and PORT:
`./OcrModule --pip robot.ip --pport robot.port`
The module should be registered in the naoqi (you can check via the robot web-page).

## Building and setup as a local module
This requires a bit more work, as you need to build Tesseract and Leptonica for NAO's operating system. To do so, you need to set up a OpenNAO virtual machine using the OpenNAO system image (make sure it corresponds to the version of naoqi you have on the robot). You can use this guide for that:
http://doc.aldebaran.com/2-1/dev/tools/vm-setup.html

**These instructions were working for the older version of OpenNAO (v1.14.5)**. TODO: check which versions does emerge install currently and update instructions.

### Dependencies

 - OpenNAO virtual machine with the following
	 -  *autotools* (m4, automake, autoconfig, libtool)
 - Leptonica sources
 - Tesseract sources
 - Naoqi C++ cross-compilation SDK (v2.1.4)

### Build local module
Steps that are to be performed on the virtual machine are denoted VM, and steps on the host machine (i.e. your computer) are denoted HM. 

#### Building nao-ocr dependencies on the VM

Pull tesseract and leptonica sources from github (use https as you probably don't have ssh keys in the VM):
$ (VM): cd worktree
$ (VM)/worktree: git clone https://github.com/tesseract-ocr/tesseract.git
$ (VM)/worktree: git clone https://github.com/DanBloomberg/leptonica.git

You can use specific releases of the libraries by checking out a tag:
$ (VM)/worktree/tesseract: git tag -l
$ (VM)/worktree/tesseract: git checkout tags/tag_name

We used leptonica v1.73 and tesseract v3.02.02. 

Prepare local directories for installation of leptonica and tesseract files as we need those to build our local module:

$ (VM)/worktree: mkdir leptonica_install tessract_install

Now, build leptonica first. Make sure to set the install directory you just created by using the --prefix flag:
$ (VM)/worktree: cd leptonica
$ (VM)/worktree/leptonica: ./configure --prefix=/home/nao/worktree/leptonica_install
$ (VM)/worktree/leptonica: make
$ (VM)/worktree/leptonica: make install

Next, build tesseract. You need to tell the system where leptonica is and also specify to install tesseract in the directory you created:
$ (VM)/worktree: cd tesseract
$ (VM)/worktree/tesseract: ./autogen.sh
$ (VM)/worktree/tesseract: LIBLEPT_HEADERSDIR=/home/nao/worktree/leptonica_install/include/ ./configure --prefix=/home/nao/worktree/tesseract_install --with-extra-libraries=/home/nao/worktree/leptonica_install/lib/
$ (VM)/worktree/tesseract: make
$ (VM)/worktree/tesseract: make install

Workflow should be as follows:

 1. Install autotools in the VM
 2. Build and install Leptonica
 3. Link tesseract to leptonica, build and install
 4. Copy tesseract and leptonica packages (along with language packages) from the VM to both the robot and your machine 
 5. Setup local installation of tesseract on the robot. Make sure you can run the tesseract on the robot by sshing to the robot and running (put some image of text in image.png and transfer it to the robot):
 `tesseract image.png out -l eng`
 6. Build this project on your machine using cross-compilation toolchain. Make sure to add tesseract and leptonica you built on the VM to the toolchain. **Make sure to set the flag TXTREC_IS_REMOTE to OFF in CMakeLists.txt**
 7. Transfer the OcrModule.so to the robot and add it to autoload.ini, then restart the robot.
 8. On your machine, run naoocr, provide the correct ip of the robot and see if everything is running.

## Usage
The module is called by the naoocr program. For the naoocr program to run there must be a nao.config file in the same directory as the program. This file defines the default parameters for the program.

* NAO_IP - the NAO IP address
* NAO_PORT - the port (usually 9559)
* NAO_MOVEMENT - example positioning program for better reading. For now, set this to FALSE.
* INPUT_MODE - the input mode. Can be FILE or CAMERA. If FILE is specified, make sure to put the correct path in PICTURE_LOCATION
* OUTPUT_MODE - the output mode. Can be SILENT or SPEAK. If SPEAK is set, NAO will automatically say the text using english (i.e. we don't set the TTS language of the NAO currently).
* OCR_MODE - the OCR mode. Can be SEGMENTED or WHOLE. If SEGMENTED, the robot will read the text line by line (meaning that he will start speaking as soon as ocr processes the first line, and will continue speaking and recognizing text in parallel). Usually you get better recognition using WHOLE, but if there is a lot of text you may get a large delay before the robot actually starts to utter the text.
* LANGUAGE - OCR language. Can be eng or hrv (It should work for other languages too, but you need to have the tesseract language package installed. Not tested though.)
* RESULT_LOCATION - the location of the file where the result will be saved
* PICTURE_LOCATION - the location of the input picture. Is used only if the input mode is FILE
* DEBUG - boolean to enable debug outputs through steps
* DEBUG_LOCATION - the location of the debug outputs

These arguments can also be directly given to the program. For more information run `naoocr --help`.
