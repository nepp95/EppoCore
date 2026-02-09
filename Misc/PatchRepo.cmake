function(PatchRepo name)
    FetchContent_GetProperties(${name})
    if (NOT ${name}_POPULATED)
        FetchContent_Populate(${name})

        set(WORKING_DIR ${${name}_SOURCE_DIR})

        file(GLOB PATCH_FILES "${CMAKE_CURRENT_SOURCE_DIR}/Misc/Patches/${name}/*.patch")
        list(SORT PATCH_FILES)

        foreach (patch IN LISTS PATCH_FILES)
            execute_process(
                COMMAND git -C "${WORKING_DIR}" apply --check --ignore-whitespace --unsafe-paths "${patch}"
                RESULT_VARIABLE can_apply
                OUTPUT_QUIET
                ERROR_VARIABLE err_check
                ERROR_STRIP_TRAILING_WHITESPACE
            )

            if (can_apply EQUAL 0)
                execute_process(
                    COMMAND git -C "${WORKING_DIR}" apply --ignore-whitespace --unsafe-paths "${patch}"
                    RESULT_VARIABLE apply_res
                    OUTPUT_QUIET
                    ERROR_VARIABLE err_apply
                    ERROR_STRIP_TRAILING_WHITESPACE
                )

                if (NOT apply_res EQUAL 0)
                    message(FATAL_ERROR "Failed to apply patch: ${patch}\n${err_apply}")
                endif ()

                message(STATUS "Applied patch: ${patch}")

            else ()
                execute_process(
                    COMMAND git -C "${WORKING_DIR}" apply --reverse --check --ignore-whitespace --unsafe-paths "${patch}"
                    RESULT_VARIABLE rev_check
                    OUTPUT_QUIET
                    ERROR_VARIABLE err_rev
                    ERROR_STRIP_TRAILING_WHITESPACE
                )

                if (rev_check EQUAL 0)
                    message(STATUS "Patch already applied, skipping: ${patch}")
                else ()
                    message(FATAL_ERROR
                        "Patch ${patch} cannot be applied and does not appear to be already applied.\n"
                        "git apply --check: ${err_check}\n"
                        "git apply --reverse --check: ${err_rev}"
                    )
                endif ()
            endif ()
        endforeach ()

        add_subdirectory(${${name}_SOURCE_DIR} ${${name}_BINARY_DIR})
    endif ()
endfunction()
