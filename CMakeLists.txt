cmake_minimum_required(VERSION 3.29)
project(ar_materijali)

set(CMAKE_CXX_STANDARD 20)

add_executable(iskazne_formule 01_iskazne_formule/main.cpp)
add_executable(iskazna_logika 02_iskazna_logika/main.cpp)
add_executable(normalne_forme 03_normalne_forme/main.cpp)
add_executable(sat 04_sat/sat.cpp 04_sat/tseitin.cpp)
add_executable(minisat 05_minisat/brojac.cpp)
add_executable(logika_prvog_reda 06_logika_prvog_reda/main.cpp
        06_logika_prvog_reda/fol.h)