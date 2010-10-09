// ivTTSStandardAPISample.c : Defines the entry point for the console application.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ivTTS.h"

/* constant for TTS heap size */
//#define ivTTS_HEAP_SIZE		38000 /* 混合无音效 */
#define ivTTS_HEAP_SIZE		58000 /* 混合有音效 */

/* constant for cache allocation */
#define ivTTS_CACHE_SIZE	512
#define ivTTS_CACHE_COUNT	1024
#define ivTTS_CACHE_EXT		8

/* Message */
ivTTSErrID DoMessage()
{
	/* 获取消息，用户实现 */
	if(1)
	{
		/* 继续合成 */
		return ivTTS_ERR_OK;
	}
	else
	{
		/* 退出合成 */
		return ivTTS_ERR_EXIT;
	}
}

FILE *fpOutput = 0;
/* output callback */
ivTTSErrID OnOutput(
		ivUInt16		nCode,			/* [in] output data code */
		ivCPointer		pcData,			/* [in] output data buffer */
		ivSize			nSize )			/* [in] output data size */
{
	/* play */
	/* 根据实际平台将语音数据传给播音接口，这里只是简单的将语音数据保存在文件中 */
	fwrite(pcData, 1, nSize, fpOutput);
	return ivTTS_ERR_OK;
}

/* read resource callback */
void ivCall ReadResCB(
		ivPointer		pParameter,		/* [in] user callback parameter */
		ivPointer		pBuffer,		/* [out] read resource buffer */
		ivResAddress	iPos,			/* [in] read start position */
		ivResSize		nSize )			/* [in] read size */
{
	FILE* pFile = (FILE*)pParameter;
	fseek(pFile, iPos, SEEK_SET);
	fread(pBuffer, nSize, 1, pFile);
}

/* output callback */
ivTTSErrID ivCall OutputCB(
		ivPointer		pParameter,		/* [in] user callback parameter */
		ivUInt16		nCode,			/* [in] output data code */
		ivCPointer		pcData,			/* [in] output data buffer */
		ivSize			nSize )			/* [in] output data size */
{
	/* 获取线程消息，是否退出合成 */
	ivTTSErrID tErr = DoMessage();
	if ( tErr != ivTTS_ERR_OK ) return tErr;
	/* 把语音数据送去播音 */
	return OnOutput(nCode, pcData, nSize);
}

int main(void)
{
	ivHTTS			hTTS;
	ivPByte			pHeap;
	ivTResPackDesc	tResPackDesc;
	ivTTSErrID		ivReturn;

	if (1)
	{
		/* 分配堆 */
		pHeap = (ivPByte)malloc(ivTTS_HEAP_SIZE);
		memset(pHeap, 0, ivTTS_HEAP_SIZE);
		fpOutput = fopen("OutPcm.pcm","wb+");
		if( !fpOutput )
			return 0;

		/* 初始化资源 */
		/* 可以有多个资源包，可以分包*/
		tResPackDesc.pCBParam = fopen("..\\..\\Resource\\Resource.irf", "rb");
		tResPackDesc.pfnRead = ReadResCB;
		tResPackDesc.pfnMap = NULL;
		tResPackDesc.nSize = 0;

		if (tResPackDesc.pCBParam)
		{
			tResPackDesc.pCacheBlockIndex = (ivPUInt8)malloc(ivTTS_CACHE_COUNT + ivTTS_CACHE_EXT);
			tResPackDesc.pCacheBuffer = (ivPUInt8)malloc((ivTTS_CACHE_COUNT + ivTTS_CACHE_EXT)*(ivTTS_CACHE_SIZE));
			tResPackDesc.nCacheBlockSize = ivTTS_CACHE_SIZE;
			tResPackDesc.nCacheBlockCount = ivTTS_CACHE_COUNT;
			tResPackDesc.nCacheBlockExt = ivTTS_CACHE_EXT;
		}
		else
		{
			return 0;
		}

		/* 创建 TTS 实例 */
		ivReturn = ivTTS_Create(&hTTS, (ivPointer)pHeap, ivTTS_HEAP_SIZE, ivNull, (ivPResPackDesc)&tResPackDesc, (ivSize)1);

		/* 设置音频输出回调 */
		ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_OUTPUT_CALLBACK, (ivUInt32)OutputCB);

		/* 设置输入文本代码页 */
		ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_INPUT_CODEPAGE, ivTTS_CODEPAGE_GBK);

		/* 设置语种 */
		ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_LANGUAGE, ivTTS_LANGUAGE_CHINESE);	

		/* 设置音量 */
		ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_VOLUME, ivTTS_VOLUME_NORMAL);

		/************************************************************************
			块式合成
		************************************************************************/
		/* 设置发音人为 XIAOYAN */
		ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_ROLE, ivTTS_ROLE_XIAOYAN);
		ivReturn = ivTTS_SynthText(hTTS, ivText("你好，这里是科大讯飞语音合成系统。"), -1);
		ivReturn = ivTTS_SynthText(hTTS, ivText("Hello, this is iFLYTEK TTS system."), -1);
		/* 设置发音人为 TERRY */
		ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_ROLE, ivTTS_ROLE_TERRY);
		ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_LANGUAGE, ivTTS_LANGUAGE_ENGLISH);
		ivReturn = ivTTS_SynthText(hTTS, ivText("Hello, this is iFLYTEK TTS system."), -1);

		/* 逆初始化 */
		ivReturn = ivTTS_Destroy(hTTS);

		if ( tResPackDesc.pCacheBlockIndex )
		{
			free(tResPackDesc.pCacheBlockIndex);
		}
		if ( tResPackDesc.pCacheBuffer )
		{
			free(tResPackDesc.pCacheBuffer);
		}
		if ( pHeap )
		{
			free(pHeap);
		}
	}
	fclose(tResPackDesc.pCBParam);
	fclose(fpOutput);	
	return 0;
}

