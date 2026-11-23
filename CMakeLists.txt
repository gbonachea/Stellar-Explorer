cmake_minimum_required(VERSION 3.21)
project(StellarExplorer VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Usar la instalación de Qt en /opt/qt (p.ej. 6.11) si el usuario no indicó otra.
# Se elige la versión más alta disponible.
if(NOT CMAKE_PREFIX_PATH)
    file(GLOB _stellar_qt_candidates "/opt/qt/*/gcc_64")
    if(_stellar_qt_candidates)
        list(SORT _stellar_qt_candidates)
        list(REVERSE _stellar_qt_candidates)
        list(GET _stellar_qt_candidates 0 CMAKE_PREFIX_PATH)
        message(STATUS "Usando Qt de: ${CMAKE_PREFIX_PATH}")
    endif()
endif()

find_package(Qt6 6.11 REQUIRED COMPONENTS Core Widgets)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/filetab.cpp
    src/breadcrumbs.cpp
    src/fileopworker.cpp
    src/configdialog.cpp
    src/utils.cpp
)

set(HEADERS
    src/mainwindow.h
    src/filetab.h
    src/breadcrumbs.h
    src/fileopworker.h
    src/configdialog.h
    src/utils.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} resources.qrc)

target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
)

# Instalar el binario
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
# Instalar temas CSS
install(FILES themes.css DESTINATION share/${PROJECT_NAME})
# Instalar icono del sistema para el .desktop
install(FILES icons/StellarExplorer.png DESTINATION share/icons/hicolor/256x256/apps RENAME ${PROJECT_NAME}.png)
install(FILES StellarExplorer.desktop DESTINATION share/applications)
