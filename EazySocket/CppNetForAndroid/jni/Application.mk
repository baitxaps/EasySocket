#添加STL支持
APP_STL := gnustl_static
#添加C++11支持
APP_CPPFLAGS := -std=c++11
#编译所有支持的CPU(指令集、架构)目标文件 APP_ABI := all
APP_ABI := armeabi-v7a
#APP平台
#APP_PLATFORM := android-14