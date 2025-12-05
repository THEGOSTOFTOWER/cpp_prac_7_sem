cmake_minimum_required(VERSION 3.14)  # Требуется для использования FetchContent
project(FunctionLibrary)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

# Загружаем GoogleTest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/release-1.12.1.zip
)

# Для Windows: предотвращаем переопределение настроек компилятора/компоновщика родительского проекта
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Устанавливаем пути для include файлов
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TFunction
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FuncFactory
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Operators
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GradientDescent
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Functions
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Functions/IdentFunction
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Functions/ConstFunction
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Functions/PowerFunction
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Functions/ExpFunction
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Functions/PolynomialFunction
)


# Добавляем библиотеку с исходными файлами
add_library(FunctionLibrary STATIC
    src/TFunction/TFunction.cpp
    src/Functions/IdentFunction/IdentFunction.cpp
    src/Functions/ConstFunction/ConstFunction.cpp
    src/Functions/PowerFunction/PowerFunction.cpp
    src/Functions/ExpFunction/ExpFunction.cpp
    src/Functions/PolynomialFunction/PolynomialFunction.cpp
    src/FuncFactory/FuncFactory.cpp
    src/Operators/Operators.cpp
    src/GradientDescent/GradientDescent.cpp
)

target_include_directories(FunctionLibrary PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Создаем исполняемый файл для тестов
add_executable(FunctionTest FunctionTest.cpp)
target_link_libraries(FunctionTest FunctionLibrary GTest::gtest_main)

# Добавляем исполняемый файл main
add_executable(main main.cpp)
target_link_libraries(main FunctionLibrary)

include(GoogleTest)
gtest_discover_tests(FunctionTest)