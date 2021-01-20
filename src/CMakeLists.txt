add_executable(kirigamidraw main.cpp drawingarea.cpp resources.qrc)
install(TARGETS kirigamidraw ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

add_custom_target(tessellation
        SOURCES
        rs/tessellation/Cargo.toml
        rs/tessellation/build.rs
        rs/tessellation/src/lib.rs
        rs/tessellation/src/tessellation.rs)

corrosion_import_crate(MANIFEST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/rs/tessellation/Cargo.toml)
target_link_libraries(kirigamidraw PRIVATE tessellation-ffi)

target_include_directories(kirigamidraw PRIVATE
    ${CMAKE_BINARY_DIR}/cargo/build/${Rust_CARGO_TARGET}/cxxbridge/tessellation-ffi/src/)

target_compile_features(kirigamidraw PRIVATE cxx_std_17)
target_include_directories(kirigamidraw PRIVATE)
target_link_libraries(kirigamidraw PRIVATE Qt5::Core Qt5::Gui Qt5::Qml Qt5::Quick Qt5::QuickControls2 Qt5::Svg KF5::I18n)
