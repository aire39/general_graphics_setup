## build examples

option(BUILD_ALL_EXAMPLES "Build All Examples" OFF)

## filtered images examples

option(BUILD_EXAMPLE_FIMAGE_GRAYSCALE "Build Grayscale Image Filtering Example" OFF)
option(BUILD_EXAMPLE_FIMAGE_BLUR "Build Blur Image Filtering Example" OFF)
option(BUILD_EXAMPLE_FIMAGE_VARYBITS "Build Varying Bits Image Filtering Example" OFF)
option(BUILD_EXAMPLE_FIMAGE_MANFILT "Build Manual Filtering Example" OFF)

if (BUILD_EXAMPLE_FIMAGE_GRAYSCALE OR BUILD_ALL_EXAMPLES)
    message("Build Grayscale Example...")
    add_subdirectory(examples/filtered-images/grayscale)
endif ()

if (BUILD_EXAMPLE_FIMAGE_BLUR OR BUILD_ALL_EXAMPLES)
    message("Build Blur Example...")
    add_subdirectory(examples/filtered-images/blur)
endif ()

if (BUILD_EXAMPLE_FIMAGE_VARYBITS OR BUILD_ALL_EXAMPLES)
    message("Build Varying Bits Example...")
    add_subdirectory(examples/filtered-images/varyingbits)
endif ()

if (BUILD_EXAMPLE_FIMAGE_MANFILT OR BUILD_ALL_EXAMPLES)
    message("Build Manual Filtering Example...")
    add_subdirectory(examples/filtered-images/manual-filter)
endif ()

## camera examples

option(BUILD_EXAMPLE_STREAM_CAMERA "Build Stream Camera Example" OFF)
option(BUILD_EXAMPLE_FILTER_COPY_STREAM_CAMERA "Build Filter Copy Stream Camera Example" OFF)
option(BUILD_EXAMPLE_ONOFF_TEST_STREAM_CAMERA "Build On/Off Test Stream Camera Example" OFF)

if (BUILD_EXAMPLE_STREAM_CAMERA OR BUILD_ALL_EXAMPLES)
    message("Build Stream Camera Example...")
    add_subdirectory(examples/camera-streaming/stream-video)
endif ()

if (BUILD_EXAMPLE_FILTER_COPY_STREAM_CAMERA OR BUILD_ALL_EXAMPLES)
    message("Build Filter Copy Stream Camera Example...")
    add_subdirectory(examples/camera-streaming/filter-copy-stream-video)
endif ()

if (BUILD_EXAMPLE_ONOFF_TEST_STREAM_CAMERA OR BUILD_ALL_EXAMPLES)
    message("Build On/Off Test Stream Camera Example...")
    add_subdirectory(examples/camera-streaming/on-off-test-stream-video)
endif ()