file(GLOB_RECURSE HDRS "*.h")
file(GLOB_RECURSE SRCS "*.cc")

set(PUBLIC_HDR_DIR include)

# Specify where our headers are
include_directories(${PUBLIC_HDR_DIR})

add_library(shuang STATIC ${HDRS} ${SRCS})

# Specify where the public headers of this library are
target_include_directories(shuang PUBLIC ${PUBLIC_HDR_DIR})
