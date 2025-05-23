# Create bindings for dataframe python module
set(HEADER_FILES
    include/ivwdataframe/ivwdataframe.h
    include/ivwdataframe/pydataframe.h
)
ivw_group("Header Files" BASE ${CMAKE_CURRENT_SOURCE_DIR}/include/ivwdataframe ${HEADER_FILES})

set(SOURCE_FILES
    src/ivwdataframe.cpp
    src/pydataframe.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

ivw_add_py_wrapper(ivwdataframe)
target_sources(ivwdataframe
    PUBLIC
    FILE_SET HEADERS
    TYPE HEADERS
    BASE_DIRS include
    FILES
        ${HEADER_FILES}
    PRIVATE
        ${SOURCE_FILES}
)

target_link_libraries(ivwdataframe PUBLIC inviwo::module::dataframe inviwo::module::python3)

ivw_install_helper(TARGET ivwdataframe
    NAMESPACE inviwo
    DESTINATION inviwo
)
