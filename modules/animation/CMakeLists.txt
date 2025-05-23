# Inviwo Animation Module
ivw_module(Animation)

# Add header files
set(HEADER_FILES
    include/modules/animation/algorithm/animationrange.h
    include/modules/animation/animationcontroller.h
    include/modules/animation/animationcontrollerobserver.h
    include/modules/animation/animationmanager.h
    include/modules/animation/animationmodule.h
    include/modules/animation/animationmoduledefine.h
    include/modules/animation/animationsupplier.h
    include/modules/animation/datastructures/animation.h
    include/modules/animation/datastructures/animationobserver.h
    include/modules/animation/datastructures/animationstate.h
    include/modules/animation/datastructures/animationtime.h
    include/modules/animation/datastructures/basekeyframe.h
    include/modules/animation/datastructures/basekeyframesequence.h
    include/modules/animation/datastructures/basetrack.h
    include/modules/animation/datastructures/buttonkeyframe.h
    include/modules/animation/datastructures/buttonkeyframesequence.h
    include/modules/animation/datastructures/buttontrack.h
    include/modules/animation/datastructures/callbackkeyframe.h
    include/modules/animation/datastructures/callbackkeyframesequence.h
    include/modules/animation/datastructures/callbacktrack.h
    include/modules/animation/datastructures/camerakeyframe.h
    include/modules/animation/datastructures/cameratrack.h
    include/modules/animation/datastructures/controlkeyframe.h
    include/modules/animation/datastructures/controlkeyframesequence.h
    include/modules/animation/datastructures/controltrack.h
    include/modules/animation/datastructures/invalidationtrack.h
    include/modules/animation/datastructures/keyframe.h
    include/modules/animation/datastructures/keyframeobserver.h
    include/modules/animation/datastructures/keyframesequence.h
    include/modules/animation/datastructures/keyframesequenceobserver.h
    include/modules/animation/datastructures/propertytrack.h
    include/modules/animation/datastructures/track.h
    include/modules/animation/datastructures/trackobserver.h
    include/modules/animation/datastructures/valuekeyframe.h
    include/modules/animation/datastructures/valuekeyframesequence.h
    include/modules/animation/demo/democontroller.h
    include/modules/animation/demo/democontrollerobserver.h
    include/modules/animation/factories/imagerecorderfactory.h
    include/modules/animation/factories/interpolationfactory.h
    include/modules/animation/factories/interpolationfactoryobject.h
    include/modules/animation/factories/recorderfactories.h
    include/modules/animation/factories/recorderfactory.h
    include/modules/animation/factories/trackfactory.h
    include/modules/animation/factories/trackfactoryobject.h
    include/modules/animation/interpolation/cameralinearinterpolation.h
    include/modules/animation/interpolation/camerasphericalinterpolation.h
    include/modules/animation/interpolation/constantinterpolation.h
    include/modules/animation/interpolation/interpolation.h
    include/modules/animation/interpolation/linearinterpolation.h
    include/modules/animation/mainanimation.h
    include/modules/animation/workspaceanimations.h
)
ivw_group("Header Files" ${HEADER_FILES})

# Add source files
set(SOURCE_FILES
    src/algorithm/animationrange.cpp
    src/animationcontroller.cpp
    src/animationcontrollerobserver.cpp
    src/animationmanager.cpp
    src/animationmodule.cpp
    src/animationsupplier.cpp
    src/datastructures/animation.cpp
    src/datastructures/animationobserver.cpp
    src/datastructures/animationstate.cpp
    src/datastructures/animationtime.cpp
    src/datastructures/basekeyframe.cpp
    src/datastructures/basekeyframesequence.cpp
    src/datastructures/basetrack.cpp
    src/datastructures/buttonkeyframe.cpp
    src/datastructures/buttonkeyframesequence.cpp
    src/datastructures/buttontrack.cpp
    src/datastructures/callbackkeyframe.cpp
    src/datastructures/callbackkeyframesequence.cpp
    src/datastructures/callbacktrack.cpp
    src/datastructures/camerakeyframe.cpp
    src/datastructures/cameratrack.cpp
    src/datastructures/controlkeyframe.cpp
    src/datastructures/controlkeyframesequence.cpp
    src/datastructures/controltrack.cpp
    src/datastructures/invalidationtrack.cpp
    src/datastructures/keyframe.cpp
    src/datastructures/keyframeobserver.cpp
    src/datastructures/keyframesequence.cpp
    src/datastructures/keyframesequenceobserver.cpp
    src/datastructures/propertytrack.cpp
    src/datastructures/track.cpp
    src/datastructures/trackobserver.cpp
    src/datastructures/valuekeyframe.cpp
    src/datastructures/valuekeyframesequence.cpp
    src/demo/democontroller.cpp
    src/demo/democontrollerobserver.cpp
    src/factories/imagerecorderfactory.cpp
    src/factories/interpolationfactory.cpp
    src/factories/interpolationfactoryobject.cpp
    src/factories/recorderfactories.cpp
    src/factories/recorderfactory.cpp
    src/factories/trackfactory.cpp
    src/factories/trackfactoryobject.cpp
    src/interpolation/cameralinearinterpolation.cpp
    src/interpolation/camerasphericalinterpolation.cpp
    src/interpolation/interpolation.cpp
    src/mainanimation.cpp
    src/workspaceanimations.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Add Unittests
set(TEST_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/animation-unittest-main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/track-test.cpp
)
ivw_add_unittest(${TEST_FILES})

# Create module
ivw_create_module(NO_PCH ${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

# Add shader directory to pack
# ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
