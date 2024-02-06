set(LMDK_BUILD 0)
set(SOF_SRC_DIR ../../../src)
set(LMDK_DIR_INCLUDE ../../../lmdk/include)

target_sources(debug_module PRIVATE ${SOF_SRC_DIR}/audio/debug_module/debug_module.c
	${SOF_SRC_DIR}/../tools/rimage/src/file_simple.c
	${SOF_SRC_DIR}/../tools/rimage/src/cse.c
	${SOF_SRC_DIR}/../tools/rimage/src/css.c
	${SOF_SRC_DIR}/../tools/rimage/src/plat_auth.c
	${SOF_SRC_DIR}/../tools/rimage/src/hash.c
	${SOF_SRC_DIR}/../tools/rimage/src/pkcs1_5.c
	${SOF_SRC_DIR}/../tools/rimage/src/manifest.c
	${SOF_SRC_DIR}/../tools/rimage/src/ext_manifest.c
	${SOF_SRC_DIR}/../tools/rimage/src/rimage.c
	${SOF_SRC_DIR}/../tools/rimage/src/toml_utils.c
	${SOF_SRC_DIR}/../tools/rimage/src/adsp_config.c
	${SOF_SRC_DIR}/../tools/rimage/src/misc_utils.c
	${SOF_SRC_DIR}/../tools/rimage/src/file_utils.c
	${SOF_SRC_DIR}/../tools/rimage/src/elf_file.c
	${SOF_SRC_DIR}/../tools/rimage/src/module.c
	${SOF_SRC_DIR}/../tools/rimage/tomlc99/toml.c
	${SOF_SRC_DIR}/../../../../../../msys64/usr/lib/openssl/engines-3
	${SOF_SRC_DIR}/../../../../../../converged/msys64/usr/lib)

set_target_properties(debug_module PROPERTIES
  HPSRAM_ADDR "0xa06b1000"
)

target_compile_definitions(debug_module PRIVATE CONFIG_XTENSA=1
												 CONFIG_IPC_MAJOR_4=1
												 CONFIG_LIBRARY=1
												 MODULE_PRIVAT=1
												 XCHAL_HAVE_HIFI3=1
												 SOF_MODULE_API_PRIVATE=1)


# Windows builds use MSYS2 https://www.msys2.org/ to get linux tools and headers.
# MSYS_INSTALL_DIR variable points to MSYS2 installation directory.
# You may pass it as environmental or cmake configure variable.
if(${CMAKE_HOST_WIN32})
	cmake_minimum_required(VERSION 3.20)
	if(DEFINED ENV{MSYS_INSTALL_DIR} AND NOT MSYS_INSTALL_DIR)
		set(MSYS_INSTALL_DIR $ENV{MSYS_INSTALL_DIR})
	endif()

	if(MSYS_INSTALL_DIR)
		cmake_path(IS_ABSOLUTE MSYS_INSTALL_DIR IS_MSYS_INSTALL_DIR_ABSOLUTE)
		if(NOT IS_MSYS_INSTALL_DIR_ABSOLUTE)
			message(FATAL_ERROR "Please provide absolute path to MSYS2 installation
				setting MSYS_INSTALL_DIR env variable")
		endif()
		# Include standard posix headers. Requires pacman openssl-devel package.
		cmake_path(APPEND MSYS_INSTALL_DIR "usr" "include" OUTPUT_VARIABLE MSYS_SYSTEM_INCLUDE_PATH)
		target_include_directories(debug_module PRIVATE "${MSYS_SYSTEM_INCLUDE_PATH}")
	endif()
endif()

target_include_directories(debug_module PRIVATE "${SOF_SRC_DIR}/include"
                                                 "${SOF_SRC_DIR}/include/ipc"
                                                 "${SOF_SRC_DIR}/include/ipc4"
                                                 "${SOF_SRC_DIR}/include/sof/audio/module_adapter/iadk"
                                                 "${SOF_SRC_DIR}/../zephyr/include"
                                                 "${SOF_SRC_DIR}/../../../xtensa/XtDevTools/install/tools/RI-2022.10-win32/XtensaTools/Tools/gcc-6.3/opt/include/openssl"
                                                 "${SOF_SRC_DIR}/../../../xtensa/XtDevTools/install/tools/RI-2022.10-win32/XtensaTools/Tools/gcc-6.3/opt/include"
                                                 "${SOF_SRC_DIR}/../tools/rimage/src"
                                                 "${SOF_SRC_DIR}/../tools/rimage/src/include/rimage"
                                                 "${SOF_SRC_DIR}/../tools/rimage/tomlc99"
                                                 "${SOF_SRC_DIR}/../tools/rimage/src/include"
                                                 #"${SOF_SRC_DIR}/../../../xtensa/XtDevTools/install/tools/RI-2022.10-win32/XtensaTools/xtensa-elf/src/xclib/include/c99"
                                                 #"${SOF_SRC_DIR}/../../../xtensa/XtDevTools/install/tools/RI-2022.10-win32/XtensaTools/Tools/opt/include/openssl"
                                                 #"${SOF_SRC_DIR}/../../../xtensa/XtDevTools/install/tools/RI-2022.10-win32/XtensaTools/Tools/opt/include"
                                                 "${SOF_SRC_DIR}/../../../../../../msys64/usr/lib/openssl/engines-3"
                                                 "${SOF_SRC_DIR}/../../../../../../converged/msys64/usr/lib/openssl/engines-3"
                                                 "${SOF_SRC_DIR}/../../../../../../converged/msys64/usr/lib"
                                                 "${MSYS_SYSTEM_INCLUDE_PATH}"
                                                 )



