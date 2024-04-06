file(GLOB_RECURSE sources CONFIGURE_DEPENDS
    src/*.c
    src/*.h
        headers/*.h
)

add_executable(image-transformer ${sources}
        headers/image.h
        src/file_control.c
        src/util.c)
target_include_directories(image-transformer PRIVATE src headers)
