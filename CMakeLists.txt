cmake_minimum_required(VERSION 3.0)

project(pikasso)

include(FeatureSummary)

set(QT5_MIN_VERSION 5.12)
set(KF5_MIN_VERSION 5.65)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMPoQmTools)

find_package(Qt5 ${QT5_MIN_VERSION} REQUIRED COMPONENTS Core Gui Qml QuickControls2 Svg)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS Kirigami2 I18n)

find_package(Corrosion)
set_package_properties(Corrosion PROPERTIES
    PURPOSE "Required for transforming the paths to vertices and indices"
    DESCRIPTION "CMake scripts to seamlessly build and link to targets using cargo"
    URL https://github.com/AndrewGaspar/corrosion
)

add_subdirectory(src)

install(PROGRAMS org.kde.pikasso.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.pikasso.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
