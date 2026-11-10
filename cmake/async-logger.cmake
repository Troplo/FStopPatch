if(TARGET AsyncLogger)
    return()
endif()

include(FetchContent)
add_compile_definitions(CXX_FORMAT_SUPPORT)
message(STATUS "Setting up AsyncLogger")
FetchContent_Declare(
    AsyncLogger
    GIT_REPOSITORY https://github.com/Troplo/AsyncLogger.git
    GIT_TAG a5f84f747373babd2e3909e79aadb3a986ec6354
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(AsyncLogger)

set_property(TARGET AsyncLogger PROPERTY CXX_STANDARD 23)
