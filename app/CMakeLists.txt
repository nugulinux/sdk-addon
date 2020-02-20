ADD_EXECUTABLE(cli
    main.c
    mnu_settings.c
    sdk_control.cc
    stackmenu/src/stackmenu.c
    $<TARGET_OBJECTS:objaddon>
)

TARGET_INCLUDE_DIRECTORIES(cli PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    stackmenu/src)
TARGET_COMPILE_DEFINITIONS(cli PRIVATE
	# Stackmenu with GLib mainloop
	-DBACKEND_GLIB
)

TARGET_LINK_LIBRARIES(cli PRIVATE ${pkgs_LDFLAGS})
