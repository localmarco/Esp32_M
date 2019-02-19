#!/bin/bash
 ROOT=/home/marco/Codes/Esp32
 export IDF_PATH=${ROOT}/esp-idf
 IDF_ADD_PATHS_EXTRAS=
 IDF_ADD_PATHS_EXTRAS="${IDF_ADD_PATHS_EXTRAS}:${IDF_PATH}/components/esptool_py/esptool"
 IDF_ADD_PATHS_EXTRAS="${IDF_ADD_PATHS_EXTRAS}:${IDF_PATH}/components/espcoredump"
 IDF_ADD_PATHS_EXTRAS="${IDF_ADD_PATHS_EXTRAS}:${IDF_PATH}/components/partition_table/"
 IDF_ADD_PATHS_EXTRAS="${IDF_ADD_PATHS_EXTRAS}:${IDF_PATH}/tools/"
 export PATH=${PATH}:${ROOT}/xtensa-esp32-elf/bin:${IDF_ADD_PATHS_EXTRAS}
 make clean
# make menuconfig
 make all
 make flash
