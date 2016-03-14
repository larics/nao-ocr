# TESSERACT OCR
unset(TESSERACT_FOUND)

find_path(TESSERACT_INCLUDE_DIR tesseract/baseapi.h
  HINTS
  /usr/include
  /usr/local/include)

find_library(TESSERACT_LIBRARY NAMES tesseract
  HINTS
  /usr/lib
  /usr/local/lib)

find_library(LEPTONICA_LIBRARY NAMES lept
  HINTS
  /usr/lib
  /usr/local/lib)

set(TESSERACT_LIBS ${TESSERACT_LIBRARY} ${LEPTONICA_LIBRARY})
if(TESSERACT_LIBS AND TESSERACT_INCLUDE_DIR)
    set(TESSERACT_FOUND 1)
endif()
