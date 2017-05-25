# LEPTONICA OCR
unset(LEPTONICA_FOUND)

find_path(LEPTONICA_INCLUDE_DIR leptonica/allheaders.h
  HINTS
  /usr/include
  /usr/local/include)

find_library(LEPTONICA_LIBRARY NAMES lept
  HINTS
  /usr/lib
  /usr/local/lib)



set(LEPTONICA_LIBS ${LEPTONICA_LIBRARY})
if(LEPTONICA_LIBS AND LEPTONICA_INCLUDE_DIR)
    set(LEPTONICA_FOUND 1)
endif()
