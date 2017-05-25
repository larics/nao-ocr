cmake_minimum_required(VERSION 2.8)
project(TessTextRec)

SET( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR} )

find_package(qibuild)


option(TXTREC_IS_REMOTE
  "ON - OFF"
  ON)

set(CMAKE_CXX_FLAGS "-std=gnu++0x")

aux_source_directory(. SRC_LIST)
list(REMOVE_ITEM SRC_LIST "./naoocr.cpp")

if(TXTREC_IS_REMOTE)
   add_definitions( " -DTXTREC_IS_REMOTE ")
   qi_create_bin(OcrModule ${SRC_LIST})
else()
   qi_create_lib(OcrModule SHARED ${SRC_LIST} SUBFOLDER naoqi)
endif()

qi_use_lib(OcrModule ALVISION ALCOMMON BOOST OPENCV2_CORE OPENCV2_HIGHGUI OPENCV2_IMGPROC TESSERACT LEPTONICA)

qi_create_bin(naoocr naoocr.cpp) 
qi_use_lib(naoocr ALVISION ALCOMMON BOOST PTHREAD OPENCV2_CORE OPENCV2_HIGHGUI ZLIB)

