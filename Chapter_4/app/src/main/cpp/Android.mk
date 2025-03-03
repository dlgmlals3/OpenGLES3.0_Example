MY_CUR_LOCAL_PATH := $(call my-dir)
FRAMEWORK_DIR = ${MY_CUR_LOCAL_PATH}/../../../../../GLPIFramework
SCENE_DIR = ${MY_CUR_LOCAL_PATH}/Scene
GLM_SRC_PATH = $(FRAMEWORK_DIR)/glm
ZLIB_DIR = $(FRAMEWORK_DIR)/zlib
WAVEFRONTOBJ_LIB_PATH=$(FRAMEWORK_DIR)/WaveFrontOBJ

include $(CLEAR_VARS)
include $(FRAMEWORK_DIR)/zlib/Android.mk

LOCAL_PATH := $(MY_CUR_LOCAL_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE    := glNative

LOCAL_C_INCLUDES := $(GLM_SRC_PATH)/core \
		$(GLM_SRC_PATH)/gtc \
		$(GLM_SRC_PATH)/gtx \
		$(GLM_SRC_PATH)/virtrev \
		$(ZLIB_DIR) \
		$(FRAMEWORK_DIR) \
		$(WAVEFRONTOBJ_LIB_PATH) \
		$(SCENE_DIR)

LOCAL_SRC_FILES :=     $(FRAMEWORK_DIR)/GLutils.cpp \
	$(FRAMEWORK_DIR)/Cache.cpp \
	$(FRAMEWORK_DIR)/ShaderManager.cpp \
	$(FRAMEWORK_DIR)/ProgramManager.cpp \
	$(FRAMEWORK_DIR)/Transform.cpp \
	$(SCENE_DIR)/Model.cpp \
	$(SCENE_DIR)/Renderer.cpp \
	$(WAVEFRONTOBJ_LIB_PATH)/WaveFrontAssetOBJ.cpp \
	$(SCENE_DIR)/ObjLoader.cpp \
	NativeTemplate.cpp


LOCAL_SHARED_LIBRARIES := zlib
LOCAL_LDLIBS    := -landroid -llog -lEGL -lGLESv3

include $(BUILD_SHARED_LIBRARY)