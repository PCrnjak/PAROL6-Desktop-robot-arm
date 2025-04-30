cmake_minimum_required(VERSION 3.5)
project(parol6)

find_package(ament_cmake REQUIRED)

install(DIRECTORY urdf launch meshes
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
