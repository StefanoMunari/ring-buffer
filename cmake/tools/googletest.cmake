include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest
        GIT_TAG v1.16.0
)

# Fetch and add Unity to the project
FetchContent_MakeAvailable(googletest)