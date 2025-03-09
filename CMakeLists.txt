cmake_minimum_required(VERSION 3.10)
project(opdracht2 C)

set(CMAKE_C_STANDARD 99)

# Zoeken naar GTK3 via pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

# Zoeken naar Paho (MQTT)
# We gaan ervan uit dat paho-mqtt3c in /usr/local/lib aanwezig is na installatie
# (anders: link_directories(/usr/local/lib) + -lpaho-mqtt3c)
# We linken het straks rechtstreeks.

# Zoeken naar PJ_RPI_USER
find_library(PJ_RPI_USER_LIB PJ_RPI_USER HINTS /usr/local/lib /usr/lib)
if(NOT PJ_RPI_USER_LIB)
  message(FATAL_ERROR "Cannot find PJ_RPI_USER library. Did you install it?")
endif()

# Maak de executable van main.c
add_executable(opdracht2 src/main.c)

# Link de libraries: PJ_RPI_USER, paho-mqtt3c, en GTK
target_link_libraries(opdracht2
    ${PJ_RPI_USER_LIB}
    paho-mqtt3c
    ${GTK3_LIBRARIES}
)
