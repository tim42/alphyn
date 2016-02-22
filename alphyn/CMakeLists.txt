

set(PROJ_SOURCES
  
)

add_definitions(${PROJ_FLAGS})

# add_library(${PROJ_APP} STATIC ${PROJ_SOURCES})

# install
# install(TARGETS ${PROJ_APP} DESTINATION lib/neam)
install(DIRECTORY ./ DESTINATION include/neam/reflective
        FILES_MATCHING PATTERN "*.hpp"
        PATTERN "tools" EXCLUDE        # submodule tools has it own include rules
        PATTERN ".git" EXCLUDE
        PATTERN "cmake" EXCLUDE
        PATTERN "samples" EXCLUDE)

