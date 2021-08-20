file(GLOB SRCS Kernels/ref/*.c)
add_library(openvx_kernels ${SRCS} "Kernels/ref/ref_ConvexHull.c")
target_include_directories(openvx_kernels PUBLIC Common .)
