LIST (APPEND exec
    main.cpp)
FOREACH (source_file ${exec})
    GET_FILENAME_COMPONENT (source_file_we ${source_file} NAME_WE)
    ADD_EXECUTABLE (
        ${source_file_we}
        ${source_file}
    )
    TARGET_LINK_LIBRARIES (${source_file_we} PRIVATE msgpack-cpp)

ENDFOREACH()