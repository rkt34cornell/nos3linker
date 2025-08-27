cmake_minimum_required(VERSION 3.5)
project(CFS_MACSM_MGR C)

set(APP_SRC_FILES
    fsw/src/macsm_mgr_app.c
)

add_cfe_app(macsm_mgr ${APP_SRC_FILES})

target_include_directories(macsm_mgr PUBLIC
    fsw/mission_inc
    fsw/platform_inc
    fsw/src
)
