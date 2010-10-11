/*--------------------------------------------------------------+
 |																|
 |	ivTTS.h - AiSound 4 Kernel API	            				|
 |																|
 |		Copyright (c) 1999-2008, ANHUI USTC iFLYTEK CO.,LTD.	|
 |		All rights reserved.									|
 |																|
 +--------------------------------------------------------------*/

#ifndef IFLYTEK_VOICE__TTS__H
#define IFLYTEK_VOICE__TTS__H


#include "ivDefine.h"


#ifdef __cplusplus
extern "C" {
#endif


/*
 *	COMMON DEFINES
 */

typedef ivUInt16 ivTTSErrID;
typedef ivPointer ivHTTS;


/*
 *	FUNCTION ENTRIES
 */

/* get SDK version */
ivTTSErrID ivCall ivTTS_GetVersion(
	ivPUInt8		piMajor,			/* [out] major version number */
	ivPUInt8		piMinor,			/* [out] minor version number */
	ivPUInt16		piRevision);		/* [out] revision number */

/* create an instance */
ivTTSErrID ivCall ivTTS_Create(
	ivHTTS ivPtr	phTTS,				/* [out] handle to an instance */
	ivPointer		pHeap,				/* [in] heap for instance */
	ivSize			nHeapSize,			/* [in] size of the heap */
	ivPointer		pCBParam,			/* [in] user callback parameter */
	ivPResPackDesc	pResPackDesc,		/* [in] resource pack description array */
	ivSize			nResPackCount);		/* [in] resource pack count */

/* destroy an instance */
ivTTSErrID ivCall ivTTS_Destroy(
	ivHTTS			hTTS );				/* [in] handle to an instance */

/* get a parameter associated with an instance */
ivTTSErrID ivCall ivTTS_GetParam(
	ivHTTS			hTTS,				/* [in] handle to an instance */
	ivUInt32		nParamID,			/* [in] parameter ID */
	ivPUInt32		pnParamValue );		/* [out] buffer to receive the parameter value */

/* set a parameter associated with an instance */
ivTTSErrID ivCall ivTTS_SetParam(
	ivHTTS			hTTS,				/* [in] handle to an instance */
	ivUInt32		nParamID,			/* [in] parameter ID */
	ivUInt32		nParamValue );		/* [in] parameter value */

/* run an instance and hold current thread's control */
ivTTSErrID ivCall ivTTS_Run(
	ivHTTS			hTTS );				/* [in] handle to an instance */

/* exit running of an instance and leave current thread's control */
ivTTSErrID ivCall ivTTS_Exit(
	ivHTTS			hTTS );				/* [in] handle to an instance */

/* synthesize a buffer of text on an instance */
ivTTSErrID ivCall ivTTS_SynthText(
	ivHTTS			hTTS,				/* [in] handle to an instance */
	ivCPointer		pcData,				/* [in] pointer of text buffer data to be synthesized */
	ivSize			nSize );			/* [in] size of text buffer data to be synthesized */

/* begin to synthesize from callback on an instance */
ivTTSErrID ivCall ivTTS_SynthStart(
	ivHTTS			hTTS );				/* [in] handle to an instance */


/*
 *	ERROR CODES
 */

#define ivTTS_ERR_OK					0x0000					/* success */
#define ivTTS_ERR_FAILED				0xFFFF					/* failed */

#define ivTTS_ERR_END_OF_INPUT			0x0001					/* end of input stream */
#define ivTTS_ERR_EXIT					0x0002					/* exit TTS */

#define ivTTS_ERR_BASE					0x8000					/* error number base */

#define ivTTS_ERR_UNIMPEMENTED			ivTTS_ERR_BASE + 0		/* unimplemented function */
#define ivTTS_ERR_UNSUPPORTED			ivTTS_ERR_BASE + 1		/* unsupported on this platform */
#define ivTTS_ERR_INVALID_HANDLE		ivTTS_ERR_BASE + 2		/* invalid handle */
#define ivTTS_ERR_INVALID_PARAMETER		ivTTS_ERR_BASE + 3		/* invalid parameter(s) */
#define ivTTS_ERR_INSUFFICIENT_HEAP		ivTTS_ERR_BASE + 4		/* insufficient heap size  */
#define ivTTS_ERR_STATE_REFUSE			ivTTS_ERR_BASE + 5		/* refuse to do in current state  */
#define ivTTS_ERR_INVALID_PARAM_ID		ivTTS_ERR_BASE + 6		/* invalid parameter ID */
#define ivTTS_ERR_INVALID_PARAM_VALUE	ivTTS_ERR_BASE + 7		/* invalid parameter value */
#define ivTTS_ERR_RESOURCE				ivTTS_ERR_BASE + 8		/* Resource is error */


/*
 *	INSTANCE PARAMETERS
 */

/* constants for values of field nParamID */
#define ivTTS_PARAM_PARAMCH_CALLBACK	0x00000000	/* parameter change callback entry */
#define ivTTS_PARAM_LANGUAGE			0x00000100	/* language, e.g. Chinese */
#define ivTTS_PARAM_INPUT_CODEPAGE		0x00000101	/* input code page, e.g. GBK */
#define ivTTS_PARAM_TEXT_MARK			0x00000102	/* text mark, e.g. CSSML */
#define ivTTS_PARAM_USE_PROMPTS			0x00000104	/* whether use prompts */
#define ivTTS_PARAM_RECOGNIZE_PHONEME	0x00000105	/* how to recognize phoneme input */
#define ivTTS_PARAM_INPUT_MODE			0x00000200	/* input mode, e.g. from fixed buffer, from callback */
#define ivTTS_PARAM_INPUT_TEXT_BUFFER	0x00000201	/* input text buffer */
#define ivTTS_PARAM_INPUT_TEXT_SIZE		0x00000202	/* input text size */
#define ivTTS_PARAM_INPUT_CALLBACK		0x00000203	/* input callback entry */
#define ivTTS_PARAM_PROGRESS_BEGIN		0x00000204	/* current processing position */
#define ivTTS_PARAM_PROGRESS_LENGTH		0x00000205	/* current processing length */
#define ivTTS_PARAM_PROGRESS_CALLBACK	0x00000206	/* progress callback entry */
#define ivTTS_PARAM_READ_AS_NAME		0x00000301	/* whether read as name */
#define ivTTS_PARAM_READ_DIGIT			0x00000302	/* how to read digit, e.g. read as number, read as value  */
#define ivTTS_PARAM_CHINESE_NUMBER_1	0x00000303	/* how to read number "1" in Chinese */
#define ivTTS_PARAM_MANUAL_PROSODY		0x00000304	/* whether use manual prosody */
#define ivTTS_PARAM_ENGLISH_NUMBER_0	0x00000305	/* how to read number "0" in Englsih */
#define ivTTS_PARAM_OUTPUT_CALLBACK		0x00000401	/* output callback entry */
#define ivTTS_PARAM_ROLE				0x00000500	/* speaker role */
#define ivTTS_PARAM_SPEAK_STYLE			0x00000501	/* speak style */
#define ivTTS_PARAM_VOICE_SPEED			0x00000502	/* voice speed */
#define ivTTS_PARAM_VOICE_PITCH			0x00000503	/* voice tone */
#define ivTTS_PARAM_VOLUME				0x00000504	/* volume value */
#define ivTTS_PARAM_VEMODE				0x00000600	/* voice effect - predefined mode */
#define ivTTS_PARAM_NAVIGATION_MODE		0x00000701	/* Navigation Version*/

/* constants for values of parameter ivTTS_PARAM_LANGUAGE */
#define ivTTS_LANGUAGE_CHINESE			1			/* Chinese (with English) */
#define ivTTS_LANGUAGE_ENGLISH			2			/* English */

/* constants for values of parameter ivTTS_PARAM_INPUT_CODEPAGE */
#define ivTTS_CODEPAGE_ASCII			437			/* ASCII */
#define ivTTS_CODEPAGE_GBK				936			/* GBK (default) */
#define ivTTS_CODEPAGE_BIG5				950			/* Big5 */
#define ivTTS_CODEPAGE_UTF16LE			1200		/* UTF-16 little-endian */
#define ivTTS_CODEPAGE_UTF16BE			1201		/* UTF-16 big-endian */
#define ivTTS_CODEPAGE_UTF8				65001		/* UTF-8 */
#define ivTTS_CODEPAGE_GB2312			ivTTS_CODEPAGE_GBK
#define ivTTS_CODEPAGE_GB18030			ivTTS_CODEPAGE_GBK
#if IV_BIG_ENDIAN
#define ivTTS_CODEPAGE_UTF16			ivTTS_CODEPAGE_UTF16BE
#else
#define ivTTS_CODEPAGE_UTF16			ivTTS_CODEPAGE_UTF16LE
#endif
#define ivTTS_CODEPAGE_UNICODE			ivTTS_CODEPAGE_UTF16
#define ivTTS_CODEPAGE_PHONETIC_PLAIN	23456		/* Kingsoft Phonetic Plain */

/* constants for values of parameter ivTTS_PARAM_TEXT_MARK */
#define ivTTS_TEXTMARK_NONE				0			/* none */
#define ivTTS_TEXTMARK_SIMPLE_TAGS		1			/* simple tags (default) */

/* constants for values of parameter ivTTS_PARAM_INPUT_MODE */
#define ivTTS_INPUT_FIXED_BUFFER		0			/* from fixed buffer */
#define ivTTS_INPUT_CALLBACK			1			/* from callback */

/* constants for values of parameter ivTTS_PARAM_READ_DIGIT */
#define ivTTS_READDIGIT_AUTO			0			/* decide automatically (default) */
#define ivTTS_READDIGIT_AS_NUMBER		1			/* say digit as number */
#define ivTTS_READDIGIT_AS_VALUE		2			/* say digit as value */

/* constants for values of parameter ivTTS_PARAM_CHINESE_NUMBER_1 */
#define ivTTS_CHNUM1_READ_YAO			0			/* read number "1" [yao1] in chinese (default) */
#define ivTTS_CHNUM1_READ_YI			1			/* read number "1" [yi1] in chinese */

/* constants for values of parameter ivTTS_PARAM_ENGLISH_NUMBER_0 */
#define ivTTS_ENNUM0_READ_ZERO			0			/* read number "0" [zero] in english (default) */
#define ivTTS_ENNUM0_READ_O				1			/* read number "0" [o] in englsih */

/* constants for values of parameter ivTTS_PARAM_SPEAKER */
#define ivTTS_ROLE_TIANCHANG			1			/* Tianchang (female, Chinese) */
#define ivTTS_ROLE_WENJING				2			/* Wenjing (female, Chinese) */
#define ivTTS_ROLE_XIAOYAN				3			/* Xiaoyan (female, Chinese) */
#define ivTTS_ROLE_YANPING				3			/* Xiaoyan (female, Chinese) */
#define ivTTS_ROLE_XIAOFENG				4			/* Xiaofeng (male, Chinese) */
#define ivTTS_ROLE_YUFENG				4			/* Xiaofeng (male, Chinese) */
#define ivTTS_ROLE_SHERRI				5			/* Sherri (female, US English) */
#define ivTTS_ROLE_XIAOJIN				6			/* Xiaojin (female, Chinese) */
#define ivTTS_ROLE_NANNAN				7			/* Nannan (child, Chinese) */
#define ivTTS_ROLE_JINGER				8			/* Jinger (female, Chinese) */
#define ivTTS_ROLE_JIAJIA				9			/* Jiajia (girl, Chinese) */
#define ivTTS_ROLE_YUER					10			/* Yuer (female, Chinese) */
#define ivTTS_ROLE_XIAOQIAN				11			/* Xiaoqian (female, Chinese Northeast) */
#define ivTTS_ROLE_LAOMA				12			/* Laoma (male, Chinese) */
#define ivTTS_ROLE_BUSH					13			/* Bush (male, US English) */
#define ivTTS_ROLE_XIAORONG				14			/* Xiaorong (female, Chinese Szechwan) */
#define ivTTS_ROLE_XIAOMEI				15			/* Xiaomei (female, Cantonese) */
#define ivTTS_ROLE_ANNI					16			/* Anni (female, Chinese) */
#define ivTTS_ROLE_JOHN					17			/* John (male, US English) */
#define ivTTS_ROLE_ANITA				18			/* Anita (female, British English) */
#define ivTTS_ROLE_TERRY				19			/* Terry (female, US English) */
#define ivTTS_ROLE_CATHERINE			20			/* Catherine (female, US English) */
#define ivTTS_ROLE_TERRYW				21			/* Terry (female, US English Word) */
#define ivTTS_ROLE_XIAOLIN				22			/* Xiaolin (female, Chinese) */
#define ivTTS_ROLE_JIUXU				51			/* Jiu Xu (male, Chinese) */
#define ivTTS_ROLE_DUOXU				52			/* Duo Xu (male, Chinese) */
#define ivTTS_ROLE_XIAOPING				53			/* Xiaoping (female, Chinese) */
#define ivTTS_ROLE_DONALDDUCK			54			/* Donald Duck (male, Chinese) */
#define ivTTS_ROLE_BABYXU				55			/* Baby Xu (child, Chinese) */
#define ivTTS_ROLE_USER					99			/* user defined */

/* constants for values of parameter ivTTS_PARAM_SPEAK_STYLE */
#define ivTTS_STYLE_PLAIN				0			/* plain speak style */
#define ivTTS_STYLE_NORMAL				1			/* normal speak style (default) */

/* constants for values of parameter ivTTS_PARAM_VOICE_SPEED */
/* the range of voice speed value is from -32768 to +32767 */
#define ivTTS_SPEED_MIN					-32768		/* slowest voice speed */
#define ivTTS_SPEED_NORMAL				0			/* normal voice speed (default) */
#define ivTTS_SPEED_MAX					+32767		/* fastest voice speed */

/* constants for values of parameter ivTTS_PARAM_VOICE_PITCH */
/* the range of voice tone value is from -32768 to +32767 */
#define ivTTS_PITCH_MIN					-32768		/* lowest voice tone */
#define ivTTS_PITCH_NORMAL				0			/* normal voice tone (default) */
#define ivTTS_PITCH_MAX					+32767		/* highest voice tone */

/* constants for values of parameter ivTTS_PARAM_VOLUME */
/* the range of volume value is from -32768 to +32767 */
#define ivTTS_VOLUME_MIN				-32768		/* minimized volume */
#define ivTTS_VOLUME_NORMAL				0			/* normal volume */
#define ivTTS_VOLUME_MAX				+32767		/* maximized volume (default) */

/* constants for values of parameter ivTTS_PARAM_VEMODE */
#define ivTTS_VEMODE_NONE				0			/* none */
#define ivTTS_VEMODE_WANDER				1			/* wander */
#define ivTTS_VEMODE_ECHO				2			/* echo */
#define ivTTS_VEMODE_ROBERT				3			/* robert */
#define ivTTS_VEMODE_CHROUS				4			/* chorus */
#define ivTTS_VEMODE_UNDERWATER			5			/* underwater */
#define ivTTS_VEMODE_REVERB				6			/* reverb */
#define ivTTS_VEMODE_ECCENTRIC			7			/* eccentric */

/* constants for values of parameter ivTTS_PARAM_NAVIGATION_MODE */
#define ivTTS_USE_NAVIGATION			1			/* synthesize in the Mode of Navigation */
#define ivTTS_UNUSE_NAVIGATION			0			/* synthesize out of the Mode of Navigation */

/* parameter change callback type */
typedef ivTTSErrID (ivProc ivTTSCB_ParamChange)(
	ivPointer		pParameter,		/* [in] user callback parameter */
	ivUInt32		nParamID,		/* [in] parameter ID */
	ivUInt32		nParamValue );	/* [in] parameter value */

/* progress callback type */
typedef ivTTSErrID (ivProc ivTTSCB_Progress)(
	ivPointer		pParameter,		/* [in] user callback parameter */
	ivUInt32		iProcBegin,		/* [in] current processing position */
	ivUInt32		nProcLen );		/* [in] current processing length */

/* input callback type */
typedef ivTTSErrID (ivProc ivTTSCB_Input)(
	ivPointer		pParameter,		/* [in] user callback parameter */
	ivPointer		pText,			/* [out] input text buffer */
	ivSize ivPtr	pnSize );		/* [in/out] input text size */

/* output callback type */
typedef ivTTSErrID (ivProc ivTTSCB_Output)(
	ivPointer		pParameter,		/* [in] user callback parameter */
	ivUInt16		nCode,			/* [in] output data code */
	ivCPointer		pcData,			/* [in] output data buffer */
	ivSize			nSize );		/* [in] output data size */

/* constants for values of parameter wCode */
#define ivTTS_CODE_COMMAND				0x0000		/* command */
#define ivTTS_CODE_PCM8K16B				0x0208		/* PCM 8K 16bit */
#define ivTTS_CODE_PCM11K16B			0x020B		/* PCM 11K 16bit */
#define ivTTS_CODE_PCM16K16B			0x0210		/* PCM 16K 16bit */
#define ivTTS_CODE_PCM22K16B			0x0216		/* PCM 22K 16bit */

/*
 *	PARAMETER STRUCTURES
 */

/* parameters for voice effect amplify */
typedef struct tagIsTTSVEAmplifyParam TIsTTSVEAmplifyParam;

struct tagIsTTSVEAmplifyParam
{
	ivUInt16	m_nPeriod;			/* 1-2000ms */
	ivUInt8		m_fAmpMin;			/* 0-100% */
	ivUInt8		m_fAmpMax;			/* 0-100% */
};


/* parameters for voice effect echo */
typedef struct tagIsTTSVEEchoParam TIsTTSVEEchoParam;

struct tagIsTTSVEEchoParam
{
	ivUInt8		m_fInitDecay;		/* 0-100% */
	ivUInt8		m_fDecay;			/* 0-100% */
	ivUInt16	m_nDelay;			/* 1-2000ms */
};


/* parameters for voice effectVE reverb */
typedef struct tagIsTTSVEReverbParam TIsTTSVEReverbParam;

struct tagIsTTSVEReverbParam
{
	ivUInt8		m_fInitDecay;		/* 0-100% */
	ivUInt8		m_nFilters;			/* 1-16 */
	ivUInt8		m_fDecay[16];		/* 0-100% */
	ivUInt8		m_nDelay[16];		/* 1-100ms */
};


/* parameters for voice effect chrous */
typedef struct tagIsTTSVEChrousParam TIsTTSVEChrousParam;

struct tagIsTTSVEChrousParam
{
	ivUInt8		m_fInitDecay;		/* 0-100% */
	ivUInt8		m_fInitRatio;		/* 0-100% */
	ivUInt8		m_nDelayMin;		/* 1-40ms */
	ivUInt8		m_nDelayMax;		/* 1-40ms */
	ivUInt8		m_nFilters;			/* 1-16 */
	ivUInt8		m_nFilterFreq;		/* 1-50Hz */
	ivUInt8		m_nFilterFreqDelta;	/* 1-50Hz */
};


/* parameters for voice effect pitch */
typedef struct tagIsTTSVEPitchParam TIsTTSVEPitchParam;

struct tagIsTTSVEPitchParam
{
	ivUInt8		m_fDeltaPitch;		/* 0-90% */
	ivUInt16	m_nPeriod;			/* 1-20000ms */
};


#ifdef __cplusplus
}
#endif


#endif /* !IFLYTEK_VOICE__TTS__H */
