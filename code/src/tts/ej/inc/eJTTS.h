
/* eJTTS.h
 *
 * Copyright (C) 1999-2008, SinoVoice Ltd.
 *
 * 该程序文件是InfoQuik TTS的头文件。
 */

#ifndef __SINO_VOICE__EJTTS__H__
#define __SINO_VOICE__EJTTS__H__

#define jtTTSAPI 

#ifdef __cplusplus
extern "C" {
#endif

// macro defines of input text codepage
#define jtTTS_CODEPAGE_ASCII		437		// ASCII
#define	jtTTS_CODEPAGE_GBK			936		// GBK (default)
#define jtTTS_CODEPAGE_BIG5			950		// Big5
#define jtTTS_CODEPAGE_UNICODE		1200	// Unicode (UTF-16) little endien
#define jtTTS_CODEPAGE_UNICODE_BE	1201	// Unicode (UTF-16) big endien	
#define jtTTS_CODEPAGE_UTF7			65000	// UTF-7
#define jtTTS_CODEPAGE_UTF8			65001	// UTF-8
#define jtTTS_CODEPAGE_UTF16		jtTTS_CODEPAGE_UNICODE
#define jtTTS_CODEPAGE_UTF16_LE		jtTTS_CODEPAGE_UNICODE
#define jtTTS_CODEPAGE_UTF16_BE		jtTTS_CODEPAGE_UNICODE_BE
#define jtTTS_CODEPAGE_GB2312		jtTTS_CODEPAGE_GBK
#define jtTTS_CODEPAGE_GB18030		jtTTS_CODEPAGE_GBK

// macro defines of read digit mode
#define jtTTS_DIGIT_NUMBER_AUTO		0		// auto read digit, and read digit based on number rule first (default)	
#define jtTTS_DIGIT_TELEGRAM_AUTO	1		// auto read digit, and read digit based on telegram rule first
#define jtTTS_DIGIT_TELEGRAM_ONLY	2		// read digit based on telegram rule
#define jtTTS_DIGIT_NUMBER_ONLY		3		// read digit based on number rule

// macro defines of output voice format
#define jtTTS_FORMAT_PCM_NORMAL		0		// normal pcm mode depend on voice data library (default)
#define jtTTS_FORMAT_PCM_8K8B		1		// 8K 8bit mono pcm mode	
#define jtTTS_FORMAT_PCM_8K16B		2		// 8k 16bit mono pcm mode
#define jtTTS_FORMAT_PCM_16K8B		3		// 16k 16bit mono pcm mode
#define jtTTS_FORMAT_PCM_16K16B		4		// 16k 16bit mono pcm mode
#define jtTTS_FORMAT_PCM_11K8B		5		// 11k 8bit mono pcm mode
#define jtTTS_FORMAT_PCM_11K16B		6		// 11k 16bit mono pcm mode
#define jtTTS_FORMAT_PCM_22K8B		7		// 22k 8bit mono pcm mode
#define jtTTS_FORMAT_PCM_22K16B		8		// 22k 16bit mono pcm mode
#define jtTTS_FORMAT_PCM_44K8B		9		// 44k 8bit mono pcm mode
#define jtTTS_FORMAT_PCM_44K16B		10		// 44k 16bit mono pcm mode
#define jtTTS_FORMAT_VOX_6K			11		// 6k vox mode
#define jtTTS_FORMAT_VOX_8K			12		// 8k vox mode
#define jtTTS_FORMAT_ALAW_8K		13		// 8k aLaw mode
#define jtTTS_FORMAT_uLAW_8K		14		// 8k uLaw mode

// marco defines of tag mode
#define jtTTS_TAG_NONE				0		// not support tag (default)
#define jtTTS_TAG_S3ML				1		// support S3ML

// marco defines of read punctation mode
#define jtTTS_PUNC_OFF				0		// not read punctation (default)
#define jtTTS_PUNC_ON				1		// read punctation, but not read RTN
#define jtTTS_PUNC_OFF_RTN			2		// not read punctation, but read TRN
#define jtTTS_PUNC_ON_RTN			3		// read punctation and RTN

// marco defines of english read mode
#define jtTTS_ENG_AUTO				0		// auto read english text (default)
#define jtTTS_ENG_LETTER			1		// read english text through letter by letter
#define jtTTS_ENG_ENGLISH			2		// read english text

// marco defines of speak style
#define jtTTS_SPEAK_STYLE_CLEAR		-1		// clear speak style
#define jtTTS_SPEAK_STYLE_NORMAL	0		// normal speak style (default)
#define jtTTS_SPEAK_STYLE_PLAIN		1		// plain speak style
#define jtTTS_SPEAK_STYLE_VIVID		2		// vivid speak style

// marco defines of voice speed
#define jtTTS_SPEED_MIN				-32768	// minimum voice speed
#define jtTTS_SPEED_NORMAL			0		// normal voice speed (default)
#define jtTTS_SPEED_MAX				32767	// maximum voice speed

// marco defines of voice tone
#define jtTTS_PITCH_MIN				-32768	// minimum voice tone
#define jtTTS_PITCH_NORMAL			0		// normal voice tone (default)
#define jtTTS_PITCH_MAX				32767	// maximum voice tone

// marco defines of volume value
#define jtTTS_VOLUME_MIN			-32768	// minimum volume value
#define jtTTS_VOLUME_NORMAL			0		// normal volume value
#define jtTTS_VOLUME_MAX			32767	// maximum voice value (default)


// marco defines of input text mode
#define jtTTS_INPUT_TEXT_DIRECT		0		// input text directly (default)
#define jtTTS_INPUT_TEXT_CALLBACK	1		// input text through callback

// marco defines of output voice data size
#define jtTTS_OUTPUT_DATA_SIZE		4096	// size of output voice data (default)

// marco defines of input text size
#define jtTTS_INPUT_TEXT_SIZE		1024	// 输入的合成文本的最大长度
	
// error return code
typedef enum 
{
	jtTTS_ERR_NONE,				//	无错误
	
	jtTTS_ERR_TIME_EXPIRED,		//	授权时间过期
	jtTTS_ERR_LICENCE,			//	授权错误

	jtTTS_ERR_INPUT_PARAM,		//	传入参数错误
	jtTTS_ERR_TOO_MORE_TEXT,	//	输入文本太长
	jtTTS_ERR_NOT_INIT,			//	引擎没有初始化，或者没有正确初始化
	jtTTS_ERR_OPEN_DATA,		//	打开资源数据错误
	jtTTS_ERR_NO_INPUT,			//	没有输入文本
	jtTTS_ERR_MORE_TEXT,		//	文本没有合成完毕
	jtTTS_ERR_INPUT_MODE,		//	输入方式错误
	jtTTS_ERR_ENGINE_BUSY		//  引擎在工作中

} jtErrCode;

// indexs of parameter type
typedef enum 
{
	jtTTS_PARAM_VOLUME,				// volume value
	jtTTS_PARAM_SPEED,				// voice speed
	jtTTS_PARAM_PITCH,				// voice tone
	jtTTS_PARAM_CODEPAGE,			// codepage of input text
	jtTTS_PARAM_DIGIT_MODE,			// read digit mode
	jtTTS_PARAM_PUNC_MODE,			// whether read punctation
	jtTTS_PARAM_TAG_MODE,			// TAG mode
	jtTTS_PARAM_WAV_FORMAT,			// output voice format
	jtTTS_PARAM_ENG_MODE,			// read mode to english text
	jtTTS_PARAM_INPUTTXT_MODE,		// input text mode

 	jtTTS_PARAM_OUTPUT_SIZE,		// size of output voice data
 
	jtTTS_PARAM_PROGRESS_CALLBACK,	// progress callback entry
	jtTTS_PARAM_INPUT_CALLBACK,		// input text callback entry
	jtTTS_PARAM_PARAM_CALLBACK,		// parameter change callback entry
	jtTTS_PARAM_OUTPUT_CALLBACK,	// output voice data callback entry
	jtTTS_PARAM_SYLLABLE_CALLBACK,	// syllable progress callback entry
	jtTTS_PARAM_SLEEP_CALLBACK,		// release cpu control callback entry
	jtTTS_PARAM_CALLBACK_USERDATA,	// user data pointer for callback

	jtTTS_PARAM_SPEAK_STYLE			// speak style
} jtTTSParam;

// get SDK version
jtErrCode jtTTSAPI jtTTS_GetVersion(
	unsigned char*	pbyMajor,			// [out] major version number
	unsigned char*	pbyMinor,			// [out] minor version number
	unsigned long*	piRevision);		// [out] revision number

// get extend buffer size
jtErrCode jtTTSAPI jtTTS_GetExtBufSize(
	const signed char* szCNLib,			// [in] chinese data library
	const signed char* szENLib,			// [in] english data library
	const signed char* szDMLib,			// [in] domain data library
	long*			piSize);			// [out] size of extend buffer size

// init tts engine
jtErrCode jtTTSAPI jtTTS_Init(
	const signed char* szCNLib,			// [in] chinese data library
	const signed char* szENLib,			// [in] english data library
	const signed char* szDMLib,			// [in] domain data library
	unsigned long*	pdwHandle,			// [out] handle to TTS engine
	void*			pExtBuf);			// [in] extend buffer size

// exit tts engine
jtErrCode jtTTSAPI jtTTS_End(
	unsigned long	dwHandle);			// [in] handle to TTS engine

// set parameter
jtErrCode jtTTSAPI jtTTS_SetParam(
	unsigned long	dwHandle,			// [in] handle to TTS engine
	jtTTSParam		nParam,				// [in] parameter index
	long			iValue);			// [in] parameter value

// get parameter
jtErrCode jtTTSAPI jtTTS_GetParam(
	unsigned long	dwHandle,			// [in] handle to TTS engine
	jtTTSParam		nParam,				// [in] parameter index
	long*			piValue);			// [out] buffer to receive the parameter value

// run an instance and hold current thread's control
jtErrCode jtTTSAPI jtTTS_SynthStart(
	unsigned long	dwHandle);			// [in] handle to TTS engine

// exit running of an instance and leave current thread's control
jtErrCode jtTTSAPI jtTTS_SynthStop(
	unsigned long	dwHandle);			// [in] handle to TTS engine

// begin to synthesize from InputTextProc callback, and output data through OutputVoiceProc callback
jtErrCode jtTTSAPI jtTTS_Synthesize(
	unsigned long	dwHandle);			// [in] handle to TTS engine

// begin to synthesize text, and output data through OutputVoiceProc callback
jtErrCode jtTTSAPI jtTTS_SynthesizeText(
	unsigned long	dwHandle,			// [in] handle to TTS
	const void*		pText,				// [in] input text buffer to be synthesized
	long			iSize);				// [in] size of input text buffer to be synthesized

// output voice callback type
typedef jtErrCode (* jtTTS_OutputVoiceProc)(
	void*			pParameter,			// [in] user callback parameter
	long			iOutputFormat,		// [in] output data format
	void*			pVoiceData,			// [in] output data buffer
	long			iVoiceSize);		// [in] output data size

// progress callback type
typedef jtErrCode (* jtTTS_ProgressProc)(
	void*			pParameter,			// [in] user callback parameter
	long			iProcBegin,			// [in] current processing position
	long			iProcLen);			// [in] current processing length

// input text callback type
typedef jtErrCode (* jtTTS_InputTextProc)(
	void*			pParameter,			// [in] user callback parameter
	void*			pText,				// [out] input text buffer
	long*			piSize);			// [in/out] input text size

// syllable progress callback type
typedef jtErrCode (* jtTTS_SyllableProc)(
	void*			pParameter,			// [in] user callback parameter
	unsigned short	nSylType,			// [in] syllable type
	void*			pSylText,			// [in] syllable text buffer
	long			iTextLen,			// [in] syllable text length
	void*			pSylPhone,			// [in] syllable phoneme buffer
	long			iPhoneLen);			// [in] syllable phoneme length

// parameter change callback type
typedef jtErrCode (* jtTTS_ParamChangeProc)(
	void*			pParameter,			// [in] user callback parameter
	jtTTSParam		nParam,				// [in] parameter index
	long			iValue);			// [in] parameter value

#ifdef __cplusplus
}; // extern "C"
#endif

#endif // __SINO_VOICE__EJTTS__H__
