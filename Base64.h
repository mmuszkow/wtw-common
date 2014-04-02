#pragma once

#include <string>
#include <sstream>
#include <WinCrypt.h>
#pragma comment(lib, "Crypt32.lib")

/** Binary data to base64 */
static std::string base64encode(const char* data, DWORD len, DWORD flags = CRYPT_STRING_NOCRLF) {
	DWORD strLen;
	if(CryptBinaryToStringA((BYTE*)data, len, CRYPT_STRING_BASE64|flags, NULL, &strLen) == FALSE)
		return "";
	
	char* str = new char[strLen+1];
	if(CryptBinaryToStringA((BYTE*)data, len, CRYPT_STRING_BASE64|flags, str, &strLen) == FALSE) {
		delete [] str;
		return "";
	}
	str[strLen] = 0;

	std::string ret(str);
	delete [] str;
	return ret;
}

static inline std::string base64encode(const std::string& str, DWORD flags = CRYPT_STRING_NOCRLF) {
	return base64encode(str.c_str(), str.size(), flags);
}

/** Base64 to binary buffer, buffer will be allocated, free it with delete [] after use, can return NULL */
static BYTE* base64decode(const std::string& base64, DWORD* buffLen) {
	if(base64.size() == 0) 
		return NULL;

	if(CryptStringToBinaryA(base64.c_str(), base64.size(), CRYPT_STRING_BASE64, NULL, buffLen, NULL, NULL) == FALSE)
		return NULL;

	BYTE* buffer = new BYTE[(*buffLen)+1];
	if(CryptStringToBinaryA(base64.c_str(), base64.size(), CRYPT_STRING_BASE64, buffer, buffLen, NULL, NULL) == FALSE) {
		delete [] buffer;
		return NULL;
	}

	buffer[*buffLen] = 0;
	return buffer;
}

/** Assumes that encoded data is a string */
static std::string base64decodeToStr(const std::string& base64) {
	DWORD buffLen;
	BYTE* rawBuff = base64decode(base64, &buffLen);
	if(!rawBuff) return "";

	std::string res(reinterpret_cast<char*>(rawBuff));
	delete [] rawBuff;
	return res;
}

typedef enum {
	step_A, step_B, step_C
} base64_encodestep;

typedef struct {
	base64_encodestep step;
	char result;
} base64_encodestate;

static void base64_init_encodestate(base64_encodestate* state_in)
{
	state_in->step = step_A;
	state_in->result = 0;
}

static inline char base64_encode_value(char value_in)
{
	static const char* encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	if (value_in > 63) return '=';
	return encoding[(int)value_in];
}

static UINT_PTR base64_encode_block(const char* plaintext_in, int length_in, char* code_out, base64_encodestate* state_in)
{
	const char* plainchar = plaintext_in;
	const char* const plaintextend = plaintext_in + length_in;
	char* codechar = code_out;
	char result;
	char fragment;
	
	result = state_in->result;
	
	switch (state_in->step)
	{
		while (1)
		{
	case step_A:
			if (plainchar == plaintextend)
			{
				state_in->result = result;
				state_in->step = step_A;
				return codechar - code_out;
			}
			fragment = *plainchar++;
			result = (fragment & 0x0fc) >> 2;
			*codechar++ = base64_encode_value(result);
			result = (fragment & 0x003) << 4;
	case step_B:
			if (plainchar == plaintextend)
			{
				state_in->result = result;
				state_in->step = step_B;
				return codechar - code_out;
			}
			fragment = *plainchar++;
			result |= (fragment & 0x0f0) >> 4;
			*codechar++ = base64_encode_value(result);
			result = (fragment & 0x00f) << 2;
	case step_C:
			if (plainchar == plaintextend)
			{
				state_in->result = result;
				state_in->step = step_C;
				return codechar - code_out;
			}
			fragment = *plainchar++;
			result |= (fragment & 0x0c0) >> 6;
			*codechar++ = base64_encode_value(result);
			result  = (fragment & 0x03f) >> 0;
			*codechar++ = base64_encode_value(result);
		}
	}
	/* control should not reach here */
	return codechar - code_out;
}

static UINT_PTR base64_encode_blockend(char* code_out, base64_encodestate* state_in)
{
	char* codechar = code_out;
	
	switch (state_in->step)
	{
	case step_B:
		*codechar++ = base64_encode_value(state_in->result);
		*codechar++ = '=';
		*codechar++ = '=';
		break;
	case step_C:
		*codechar++ = base64_encode_value(state_in->result);
		*codechar++ = '=';
		break;
	case step_A:
		break;
	}
	*codechar++ = '\r';
	*codechar++ = '\n';
	
	return codechar - code_out;
}

