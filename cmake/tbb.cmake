# Get TBB library

include("${CMAKE_CURRENT_LIST_DIR}/utilities.cmake")
include(FetchContent)

function(add_tbb)
    find_package(TBB QUIET)
    if(NOT TBB_FOUND)
        colored_message("1;34" "Fetch TBB...")
        FetchContent_Declare(
                tbb
                GIT_REPOSITORY https://github.com/oneapi-src/oneTBB.git
                GIT_TAG        v2022.2.0  # or latest release
        )
        FetchContent_MakeAvailable(tbb)
        colored_message("1;34" "Fetch TBB complete!")
    else ()
        colored_message("1;34" "TBB already exists!")
    endif ()
endfunction()