# Enable testing with CTest
enable_testing()

include(FetchContent)
FetchContent_Declare(
        Unity
        GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
        GIT_TAG v2.6.1
)

# Fetch and add Unity to the project
FetchContent_MakeAvailable(Unity)