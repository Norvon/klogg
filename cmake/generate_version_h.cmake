# Get the current working branch
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
message("Git branch: ${GIT_BRANCH}")

# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
message("Git commit: ${GIT_COMMIT_HASH}")

# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND git describe --always --dirty
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_DESCRIBE
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

message("Git describe: ${GIT_DESCRIBE}")

if(NOT DEFINED KLOGG_LOCAL_BUILD_VERSION)
  set(KLOGG_LOCAL_BUILD_VERSION OFF)
endif()

string(TIMESTAMP BUILD_DATE "%Y-%m-%d" UTC)
string(TIMESTAMP BUILD_TIMESTAMP "%Y%m%d.%H%M%S")

set(KLOGG_DISPLAY_VERSION "${BUILD_VERSION}")
if(KLOGG_LOCAL_BUILD_VERSION)
  string(TIMESTAMP BUILD_DATE "%Y-%m-%d %H:%M:%S")
  if(NOT "${GIT_DESCRIBE}" STREQUAL "")
    set(KLOGG_DISPLAY_VERSION "local-${GIT_DESCRIBE}-${BUILD_TIMESTAMP}")
  else()
    set(KLOGG_DISPLAY_VERSION "local-${BUILD_TIMESTAMP}")
  endif()
endif()

file(WRITE generated/version.h "#ifndef GENERATED_KLOGG_VERSION_H\n")
file(APPEND generated/version.h "#define GENERATED_KLOGG_VERSION_H\n\n")

file(APPEND generated/version.h "#define KLOGG_DATE \"${BUILD_DATE}\"\n\n")
file(APPEND generated/version.h "#define KLOGG_GIT_VERSION \"${GIT_DESCRIBE}\"\n\n")
file(APPEND generated/version.h "#define KLOGG_COMMIT \"${GIT_COMMIT_HASH}\"\n\n")
file(APPEND generated/version.h "#define KLOGG_VERSION \"${BUILD_VERSION}\"\n\n")
file(APPEND generated/version.h "#define KLOGG_DISPLAY_VERSION \"${KLOGG_DISPLAY_VERSION}\"\n\n")

file(APPEND generated/version.h "#endif // GENERATED_KLOGG_VERSION_H\n")
