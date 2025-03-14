set(${PROJECT_NAME}_VERSION_MAJOR 2)
set(${PROJECT_NAME}_VERSION_MINOR 0)
set(${PROJECT_NAME}_VERSION_PATCH 1)
#set(${PROJECT_NAME}_VERSION_PRERELEASE "")

set(${PROJECT_NAME}_VERSION
    ${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH})

if (DEFINED ${PROJECT_NAME}_VERSION_PRERELEASE)
    set(${PROJECT_NAME}_VERSION
        ${${PROJECT_NAME}_VERSION}-${${PROJECT_NAME}_VERSION_PRERELEASE})
endif()

message(STATUS "VERSION:${${PROJECT_NAME}_VERSION}")
