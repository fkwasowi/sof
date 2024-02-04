set(SOF_SRC_DIR ../../../src)
set(LMDK_DIR_INCLUDE ../../../lmdk/include)
target_sources(debug_module PRIVATE ${SOF_SRC_DIR}/audio/debug_module/debug_module.c)

set_target_properties(debug_module PROPERTIES
  HPSRAM_ADDR "0xa06b1000"
)

target_compile_definitions(debug_module PRIVATE __ZEPHYR__=1
												 CONFIG_ZEPHYR_NATIVE_DRIVERS=1
												 CONFIG_XTENSA=1
												 CONFIG_IPC_MAJOR_4=1
												 CONFIG_LIBRARY=1
												 __ZEPHYR_RTOS_IDC_H__=1
												 MODULE_PRIVAT=1
												 XCHAL_HAVE_HIFI3=1)

target_include_directories(debug_module PRIVATE "${LMDK_DIR_INCLUDE}"
												 "${LMDK_DIR_INCLUDE}/sof"
												 "${LMDK_DIR_INCLUDE}/sof/audio")