# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS "main.c")
set(COMPONENT_ADD_INCLUDEDIRS "")

set(EXTRA_COMPONENT_DIRS game)
set(EXTRA_COMPONENT_DIRS net)
set(EXTRA_COMPONENT_DIRS oled)
register_component()
