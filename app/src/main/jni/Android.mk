# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
#LOCAL_C_INCLUDES  += system/core/include/cutils
LOCAL_LDLIBS := -llog
LOCAL_SHARED_LIBRARIES := libcutils libstlport
LOCAL_MODULE    := hello-jni
LOCAL_SRC_FILES := hello-jni.cpp \
			cvec.c \
			fmat.c \
			fvec.c \
			lvec.c \
			mathutils.c \
			vecutils.c \
			source_wavread_pushmode.c \
			io/audio_unit.c \
			io/sink.c \
			io/sink_apple_audio.c \
			io/sink_sndfile.c \
			io/sink_wavwrite.c \
			io/source.c \
			io/source_apple_audio.c \
			io/source_avcodec.c \
			io/source_sndfile.c \
			io/source_wavread.c \
			io/utils_apple_audio.c \
			synth/sampler.c	\
			synth/wavetable.c \
			tempo/beattracking.c \
			tempo/tempo.c \
			temporal/a_weighting.c \
			temporal/biquad.c \
			temporal/c_weighting.c \
			temporal/filter.c \
			temporal/resampler.c \
			utils/hist.c \
			utils/parameter.c \
			utils/scale.c \
			spectral/fft.c \
			spectral/filterbank.c \
			spectral/filterbank_mel.c \
			spectral/mfcc.c \
			spectral/ooura_fft8g.c \
			spectral/phasevoc.c \
			spectral/specdesc.c \
			spectral/statistics.c \
			spectral/tss.c \
			onset/onset.c \
			onset/peakpicker.c \
			pitch/pitchfcomb.c \
			pitch/pitchmcomb.c \
			pitch/pitchschmitt.c \
			pitch/pitchspecacf.c \
			pitch/pitchyin.c \
			pitch/pitchyinfft.c \
			pitch/pitch.c

include $(BUILD_SHARED_LIBRARY)
