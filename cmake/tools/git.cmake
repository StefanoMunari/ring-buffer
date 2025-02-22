function(GetGitDescription SHA1 DATE COMMIT_SUBJECT)
    # the commit's SHA1, and whether the building workspace was dirty or not
    execute_process(COMMAND
        git describe --match=NeVeRmAtCh --always --abbrev=10 --dirty
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_SHA1
        ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

    # the date of the commit
    execute_process(COMMAND
        git log -1 --format=%ad --date=local
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_DATE
        ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

    # the subject of the commit
    execute_process(COMMAND
        git log -1 --format=%s
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_COMMIT_SUBJECT
        ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

    message(STATUS "CMAKE_CURRENT_SOURCE_DIR:${CMAKE_CURRENT_SOURCE_DIR}")
    message(STATUS "GIT_SHA1:${GIT_SHA1}")
    message(STATUS "GIT_DATE:${GIT_DATE}")
    message(STATUS "GIT_COMMIT_SUBJECT:${GIT_COMMIT_SUBJECT}")

    set(${SHA1} ${GIT_SHA1} PARENT_SCOPE)
    set(${DATE} ${GIT_DATE} PARENT_SCOPE)
    set(${COMMIT_SUBJECT} ${GIT_COMMIT_SUBJECT} PARENT_SCOPE)
endfunction()