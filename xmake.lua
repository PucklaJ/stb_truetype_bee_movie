set_project("stb_truetype_bee_movie")
add_requires("stb")

add_rules("mode.debug", "mode.release")
target("bee_movie")
    set_kind("binary")
    add_files("main.cpp")
    add_packages("stb")
    set_languages("cxx11")