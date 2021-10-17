set(EXAMPLES
        triangle
)

function(buildExample EXAMPLE)
    message(STATUS "building example: ${EXAMPLE}")
    set(EXAMPLE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE})
    # main
    set(MAIN ${EXAMPLE_DIR}/${EXAMPLE}.cc)
    add_executable(${EXAMPLE} ${MAIN})
    target_link_libraries(${EXAMPLE} shuang)
endfunction(buildExample)

function(buildExamples)
    foreach(EXAMPLE ${EXAMPLES})
        buildExample(${EXAMPLE})
    endforeach(EXAMPLE)
endfunction(buildExamples)

buildExamples()
