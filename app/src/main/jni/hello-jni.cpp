/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <string.h>
#include <jni.h>
#include <list>
#include <android/log.h>
#include "aubio.h"
#include "source_wavread_pushmode.h"
#include <math.h>

extern "C" {
typedef struct _PitchData{
	long long int starttime;
	long long int pitchvalue;
}PitchData;

typedef std::list<PitchData*> PITCHLIST;

typedef struct _PitchAnalyzer{
	aubio_pitch_t* pPitch;
	aubio_source_wavread_pushmode_t* pWave;
	PITCHLIST* pList;
	fvec_t * pitch;
	long long int nBlocks;
	long long int samplerate;
}PitchAnalyzer;

#define ANALYZE_METHOD_DEFAULT	"default"
#define ANALYZE_MODE_DEFAULT	"default"
#define ANALYZE_MODE_MCOMB		"mcomb"
#define ANALYZE_MODE_YIN		"yin"
#define ANALYZE_MODE_SCHMITT	"schmitt"
#define ANALYZE_MODE_FCOMB		"fcomb"
#define ANALYZE_MODE_SPECACF	"specacf"

#define HOP_SIZE				256
#define BUFFER_SIZE				1024

#define _CALLBACK_FUNC_	0
#define _DEBUG_LOG_	0

#if _CALLBACK_FUNC_
static JNIEnv *m_env=NULL;
static jmethodID m_cb_method =0;
static jclass m_cls;
#endif


static int process_block(uint_t Object, fvec_t * ibuf, fvec_t * obuf)
{
#if _DEBUG_LOG_
  __android_log_print(ANDROID_LOG_INFO,"PitchAnalyzer","process_block---start:Object=%lu", Object);
#endif
  PitchAnalyzer* pPitchAnalyzer	= (PitchAnalyzer*)Object;;
  smpl_t freq;
  smpl_t pitch_found;
  aubio_pitch_do (pPitchAnalyzer->pPitch, ibuf, pPitchAnalyzer->pitch);

  pitch_found = fvec_get_sample(pPitchAnalyzer->pitch, 0);
  pPitchAnalyzer->nBlocks++;

  PitchData* pData = new PitchData();
  pData->starttime = (long long int)(1000*pPitchAnalyzer->nBlocks*HOP_SIZE)/pPitchAnalyzer->samplerate;
  pData->pitchvalue = (long long int)pitch_found;

  pPitchAnalyzer->pList->push_back(pData);

#if _DEBUG_LOG_
  __android_log_print(ANDROID_LOG_INFO,"PitchAnalyzer","starttime=%llu, pitchvalue=%llu, nBlocks=%llu\n",pData->starttime, pData->pitchvalue, pPitchAnalyzer->nBlocks);
#endif

#if _CALLBACK_FUNC_
  123
//  __android_log_print(ANDROID_LOG_INFO,"PitchAnalyzer","m_env=%ld, m_cls=%ld, m_cb_method=%d\n",m_env, m_cls, m_cb_method);
  m_env->CallStaticVoidMethod(m_cls,m_cb_method,pData->starttime,pData->pitchvalue);
#endif

  return 0;
}


/*
 * Class:     com_example_hellojni_PitchAnalyzer
 * Method:    NewPitchAnalyzer
 * Signature: ()I
 */
jint Java_com_example_hellojni_PitchAnalyzer_NewPitchAnalyzer
  (JNIEnv *env, jclass obj)
{
	PitchAnalyzer* analyzer = new PitchAnalyzer;
	__android_log_print(ANDROID_LOG_INFO,"PitchAnalyzer","NewPitchAnalyzer---analyzer=%lu", (analyzer));
	memset(analyzer,0x0, sizeof(PitchAnalyzer));
	analyzer->pWave = new_aubio_source_wavread_pushmode((uint_t)(analyzer), (wavread_pushmode_func_t)process_block, HOP_SIZE);
	analyzer->pitch = new_fvec(1);
	analyzer->pList = new PITCHLIST;
	analyzer->pList->clear();


#if _CALLBACK_FUNC_
	{
		m_env = env;
		m_cls = env->FindClass("com/example/hellojni/PitchAnalyzer");
//		m_cls = env->GetObjectClass(obj);
	    m_cb_method = env->GetStaticMethodID(m_cls,"callback_func","(II)V");
	    __android_log_print(ANDROID_LOG_INFO,"PitchAnalyzer","cb_method=%d", m_cb_method);
	}

#endif
	return (int)(analyzer);
}


/*
 * Class:     com_example_hellojni_PitchAnalyzer
 * Method:    PitchAnalyzerInit
 * Signature: (I[B)Z
 */
jboolean Java_com_example_hellojni_PitchAnalyzer_PitchAnalyzerInit
  (JNIEnv *env , jclass obj, jint index, jbyteArray array)
{
	PitchAnalyzer* pPitchAnalyzer	= (PitchAnalyzer*)index;

	jbyte* pBuffer = (jbyte*)env->GetByteArrayElements(array, 0);
	int size = env->GetArrayLength(array);
	__android_log_print(ANDROID_LOG_INFO,"PitchAnalyzer","PitchAnalyzerInit---analyzer=%lu, size=%d",index, size);
	if(size != 44){
		return false;
	}


	for(int i=0; i< size; i++){
		__android_log_print(ANDROID_LOG_INFO,"PitchAnalyzer","PitchAnalyzerInit----pBuffer[%d]=%x",i, pBuffer[i]);
	}

	aubio_source_wavread_pushmode_headdata(pPitchAnalyzer->pWave,(char*)pBuffer, size);
	pPitchAnalyzer->samplerate=aubio_source_wavread_pushmode_get_samplerate(pPitchAnalyzer->pWave);
	int channels = aubio_source_wavread_pushmode_get_channels(pPitchAnalyzer->pWave);
	pPitchAnalyzer->pPitch = new_aubio_pitch (ANALYZE_METHOD_DEFAULT, BUFFER_SIZE, HOP_SIZE, pPitchAnalyzer->samplerate);
	aubio_pitch_set_unit (pPitchAnalyzer->pPitch, ANALYZE_MODE_DEFAULT);
	__android_log_print(ANDROID_LOG_INFO,"PitchAnalyzer","PitchAnalyzerInit----samplerate=%d, channels=%d",pPitchAnalyzer->samplerate, channels);
	env->ReleaseByteArrayElements(array, pBuffer, 0);
	return true;
}


/*
 * Class:     com_example_hellojni_PitchAnalyzer
 * Method:    PitchAnalyzerProcess
 * Signature: (I[B)V
 */
void Java_com_example_hellojni_PitchAnalyzer_PitchAnalyzerProcess
  (JNIEnv *env, jclass obj, jint index, jbyteArray array, jint length)
{
	PitchAnalyzer* pPitchAnalyzer	= (PitchAnalyzer*)index;
	jbyte* pBuffer = (jbyte*)env->GetByteArrayElements(array, 0);
	int size = env->GetArrayLength(array);

#if _DEBUG_LOG_
	__android_log_print(ANDROID_LOG_INFO,"PitchAnalyzer","PitchAnalyzerProcess---analyzer=%lu, size=%d, length=%d",
		index, size, length);
#endif

	aubio_source_wavread_pushmode_do(pPitchAnalyzer->pWave,(char*)pBuffer,length);

	env->ReleaseByteArrayElements(array, pBuffer, 0);
}


/*
 * Class:     com_example_hellojni_PitchAnalyzer
 * Method:    PitchAnalyzerGetPitch
 * Signature: (IJJ)I
 */

static float baseParam = log(2.0);

float Java_com_example_hellojni_PitchAnalyzer_PitchAnalyzerGetPitch
  (JNIEnv *env, jclass obj, jint index, jlong starttime, jlong duration)
{
#if _DEBUG_LOG_
	//__android_log_print(ANDROID_LOG_INFO,"PitchAnalyzer","PitchAnalyzerGetPitch---analyzer=%lu,starttime=%llu, duration=%llu",index, starttime, duration);
#endif

	int value=0;
	double avageValue=0;
	PitchData* pData=NULL;
	int count=0;
	PitchAnalyzer* pPitchAnalyzer	= (PitchAnalyzer*)index;
	float  pitch=-1;
	float  param1 = 0;
	int validnum=0;

	std::list<PitchData*>::iterator it;

	for (it=pPitchAnalyzer->pList->begin(); it != pPitchAnalyzer->pList->end(); ++it){
		pData = (PitchData*)(*it);

		if(pData->starttime>=starttime && pData->starttime < starttime + duration){

			validnum++;
			if(pData->pitchvalue >= 1000 || pData->pitchvalue <= 0){
				continue;
			}

			value += pData->pitchvalue;
			count++;
		}
	}

	avageValue=(count == 0) ? -1 : (value/count);

	if(!validnum){
#if _DEBUG_LOG_
		__android_log_print(ANDROID_LOG_INFO,"PitchAnalyzer","PitchAnalyzerGetPitch---analyzer=%lu,starttime=%llu, count=%d, duration=%llu\n",index, starttime, count, duration);
#endif
		return -2;
	}

	if(avageValue != -1){
		param1 = log(avageValue/440);
		pitch = 69.0 + 12.0 * (param1/baseParam);//from wiki http://en.wikipedia.org/wiki/Pitch_(music)
	}

#if _DEBUG_LOG_
	__android_log_print(ANDROID_LOG_INFO,"PitchAnalyzer","PitchAnalyzerGetPitch---analyzer=%lu,value=%d, count=%d, avageValue=%f\n",index, value, count, pitch);
#endif
	return pitch;
}


jint Java_com_example_hellojni_PitchAnalyzer_PitchAnalyzerGetFrequency
  (JNIEnv *env, jclass obj, jint index, jlong starttime, jlong duration)
{

//	__android_log_print(ANDROID_LOG_INFO,"PitchAnalyzer","PitchAnalyzerGetPitch---analyzer=%lu,starttime=%llu, duration=%llu",index, starttime, duration);

	int value=0;
	int frequency=0;
	PitchData* pData=NULL;
	int count=0;
	PitchAnalyzer* pPitchAnalyzer	= (PitchAnalyzer*)index;
	int pitch=-1;
	double param1 = 0;

	std::list<PitchData*>::iterator it;
	for (it=pPitchAnalyzer->pList->begin(); it != pPitchAnalyzer->pList->end(); ++it){
		pData = (PitchData*)(*it);

		if(pData->pitchvalue >= 1000 || pData->pitchvalue <= 0){
			continue;
		}

		if(pData->starttime>=starttime && pData->starttime < starttime + duration){
			value += pData->pitchvalue;
			count++;
		}
	}

	frequency=(count == 0) ? -1 : (value/count);
#if _DEBUG_LOG_
	__android_log_print(ANDROID_LOG_INFO,"GetFrequency","PitchAnalyzerFrequency---analyzer=%lu,value=%d, count=%d, frequency=%d\n",index, value, count, frequency);
#endif
	return frequency;
}


/*
 * Class:     com_example_hellojni_PitchAnalyzer
 * Method:    DelPitchAnalyzer
 * Signature: (I)V
 */
void Java_com_example_hellojni_PitchAnalyzer_PitchAnalyzerReset
		(JNIEnv *env, jclass obj, jint index)
{
	PitchAnalyzer* pPitchAnalyzer	= (PitchAnalyzer*)index;
	PitchData* pData=NULL;

	__android_log_print(ANDROID_LOG_INFO,"PitchAnalyzer","Reset---analyzer=%lu",index);
//	del_aubio_source_wavread_pushmode(pPitchAnalyzer->pWave);
//	del_aubio_pitch (pPitchAnalyzer->pPitch);
//	del_fvec (pPitchAnalyzer->pitch);

	pPitchAnalyzer->nBlocks = 0;
	int i=0;
	while (!pPitchAnalyzer->pList->empty())
	{
		pData=pPitchAnalyzer->pList->front();
		i++;
#if _DEBUG_LOG_
		//__android_log_print(ANDROID_LOG_INFO,"PitchAnalyzer","starttime=%ld, pitchvalue=%ld, i=%d\n",pData->starttime, pData->pitchvalue, i);
#endif
		delete pData;
		pPitchAnalyzer->pList->pop_front();
	}
	pPitchAnalyzer->pList->clear();
//	delete pPitchAnalyzer->pList;
//	delete pPitchAnalyzer;

}


/*
 * Class:     com_example_hellojni_PitchAnalyzer
 * Method:    DelPitchAnalyzer
 * Signature: (I)V
 */
void Java_com_example_hellojni_PitchAnalyzer_DelPitchAnalyzer
  (JNIEnv *env, jclass obj, jint index)
{
	PitchAnalyzer* pPitchAnalyzer	= (PitchAnalyzer*)index;
	PitchData* pData=NULL;

	__android_log_print(ANDROID_LOG_INFO,"PitchAnalyzer","DelPitchAnalyzer---analyzer=%lu",index);
	del_aubio_source_wavread_pushmode(pPitchAnalyzer->pWave);
	del_aubio_pitch (pPitchAnalyzer->pPitch);
	del_fvec (pPitchAnalyzer->pitch);
	int i=0;
	while (!pPitchAnalyzer->pList->empty())
	{
		pData=pPitchAnalyzer->pList->front();
		i++;
#if _DEBUG_LOG_
		//__android_log_print(ANDROID_LOG_INFO,"PitchAnalyzer","starttime=%ld, pitchvalue=%ld, i=%d\n",pData->starttime, pData->pitchvalue, i);
#endif
		delete pData;
		pPitchAnalyzer->pList->pop_front();
	}
	pPitchAnalyzer->pList->clear();
	delete pPitchAnalyzer->pList;
	delete pPitchAnalyzer;

}
}
