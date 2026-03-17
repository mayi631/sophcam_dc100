# Get current commit
execute_process(COMMAND git show --oneline -s OUTPUT_VARIABLE CURRENT_COMMIT)

message("==================================================")
message("Project          ${PROJECT_NAME}")
message("Commit           ${CURRENT_COMMIT}")
message("[Summary]")
message("Build type       ${CMAKE_BUILD_TYPE}")
message("C compiler       ${CMAKE_C_COMPILER}")
message("CXX compiler     ${CMAKE_CXX_COMPILER}")
message("C Flags          ${CMAKE_C_FLAGS}")
message("CXX Flags        ${CMAKE_CXX_FLAGS}")
message("Install dir      ${CMAKE_INSTALL_PREFIX}")
message("==================================================")
