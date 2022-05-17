set(COMPONENT_SRCS src/bt/PSController/PSController.cpp)

set(COMPONENT_ADD_INCLUDEDIRS src)

set(COMPONENT_PRIV_INCLUDEDIRS src/bt/private)

set(COMPONENT_REQUIRES minduino)
set(COMPONENT_PRIV_REQUIRES bt)

register_component()
