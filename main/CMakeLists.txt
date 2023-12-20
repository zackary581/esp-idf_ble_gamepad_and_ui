idf_component_register(
    SRCS
        "main.cpp"

    INCLUDE_DIRS
        "."
        "../include"

    REQUIRES
        arduino
        esp-nimble-cpp  
)
