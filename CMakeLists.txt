cmake_minimum_required(VERSION 3.30...4.4)
project(GHSLauncher)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME}
	"main.cpp"
        "icon/Icon.o"
)