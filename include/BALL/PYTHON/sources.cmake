### list all filenames of the directory here ###
SET(GROUP PYTHON)

FILE(GLOB HEADERS_LIST "include/BALL/${GROUP}/*.h" "include/BALL/${GROUP}/*.iC")

ADD_BALL_HEADERS("${GROUP}" "${HEADERS_LIST}")

INCLUDE(include/BALL/PYTHON/EXTENSIONS/sources.cmake)
