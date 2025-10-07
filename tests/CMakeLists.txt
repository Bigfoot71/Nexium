set(RESOURCES_PATH "${NX_ROOT_PATH}/tests/resources/")
set(INCLUDE_PATH "${NX_ROOT_PATH}/include")

function(add_hyperion_test test_name source_file)
    add_executable(${test_name} ${source_file})
    target_link_libraries(${test_name} PRIVATE nexium)
    target_compile_definitions(${test_name} PRIVATE RESOURCES_PATH="${RESOURCES_PATH}")
    target_include_directories(${test_name} PRIVATE "include")
    if(NX_HAVE_LIB_M)
        target_link_libraries(${test_name} PUBLIC m)
    endif()
endfunction()

add_hyperion_test("nx-instanced-material-shader" "${NX_ROOT_PATH}/tests/instanced_material_shader.c")
add_hyperion_test("nx-skybox-procedural" "${NX_ROOT_PATH}/tests/skybox_procedural.c")
add_hyperion_test("nx-material-shader" "${NX_ROOT_PATH}/tests/material_shader.c")
add_hyperion_test("nx-render-texture" "${NX_ROOT_PATH}/tests/render_texture.c")
add_hyperion_test("nx-shading-mode" "${NX_ROOT_PATH}/tests/shading_mode.c")
add_hyperion_test("nx-animation" "${NX_ROOT_PATH}/tests/animation.c")
add_hyperion_test("nx-instanced" "${NX_ROOT_PATH}/tests/instanced.c")
add_hyperion_test("nx-billboard" "${NX_ROOT_PATH}/tests/billboard.c")
add_hyperion_test("nx-bunnymark" "${NX_ROOT_PATH}/tests/bunnymark.c")
add_hyperion_test("nx-streams" "${NX_ROOT_PATH}/tests/streams.c")
add_hyperion_test("nx-overlay" "${NX_ROOT_PATH}/tests/overlay.c")
add_hyperion_test("nx-shadows" "${NX_ROOT_PATH}/tests/shadows.c")
add_hyperion_test("nx-skybox" "${NX_ROOT_PATH}/tests/skybox.c")
add_hyperion_test("nx-lights" "${NX_ROOT_PATH}/tests/lights.c")
add_hyperion_test("nx-bloom" "${NX_ROOT_PATH}/tests/bloom.c")
add_hyperion_test("nx-ssao" "${NX_ROOT_PATH}/tests/ssao.c")
add_hyperion_test("nx-fog" "${NX_ROOT_PATH}/tests/fog.c")
add_hyperion_test("nx-pbr" "${NX_ROOT_PATH}/tests/pbr.c")

if(WIN32)
    set(vendored_dirs
        "${CMAKE_BINARY_DIR}/external/SDL"
        "${CMAKE_BINARY_DIR}/external/physfs"
        "${CMAKE_BINARY_DIR}/external/freetype"
        "${CMAKE_BINARY_DIR}/external/assimp"
    )
    foreach(dir IN LISTS vendored_dirs)
        if(EXISTS "${dir}")
            file(GLOB_RECURSE dll_files "${dir}/*.dll")
            foreach(dll_file ${dll_files})
                add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            "${dll_file}"
                            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
                    COMMENT "Copying ${dll_file}"
                )
            endforeach()
        endif()
    endforeach()
endif()
