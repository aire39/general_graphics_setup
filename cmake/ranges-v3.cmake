# Get RangeV3 library

include("${CMAKE_CURRENT_LIST_DIR}/utilities.cmake")
include(FetchContent)

function(add_range_v3)
    colored_message("1;34" "Fetch ranges-v3...")
    FetchContent_Declare(
            range-v3
            GIT_REPOSITORY https://github.com/ericniebler/range-v3.git
            GIT_TAG        0.12.0
    )
    FetchContent_MakeAvailable(range-v3)
    colored_message("1;34" "Fetch ranges-v3 complete!")
endfunction()