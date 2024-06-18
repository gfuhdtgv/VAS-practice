cmake_minimum_required(VERSION 3.10)

# Название проекта
project(MatrixProject)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Добавление исполняемого файла
add_executable(MatrixProject Matrix.cpp)

