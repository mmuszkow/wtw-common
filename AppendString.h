#pragma once

#include <string.h>

namespace strUtils
{
	class AppendString {
		char*	data;
		size_t	len;
		size_t	reserved;

		static unsigned int upper_power_of_two(unsigned int v) {
			v--;
			v |= v >> 1;
			v |= v >> 2;
			v |= v >> 4;
			v |= v >> 8;
			v |= v >> 16;
			v++;
			return v;
		}

		void assign(const AppendString& str) {
			len = str.len;
			reserved = str.reserved;
			data = new char[reserved];
			memcpy(data, str.data, len);
			data[len] = 0;
		}
	public:
		AppendString() {
			len = 0;
			reserved = 128;
			data = new char[reserved];
			data[0] = 0;
		}

		AppendString(const AppendString& str) {
			assign(str);
		}

		~AppendString() {
			delete [] data;
		}

		void reserve(size_t length) {
			char* newData = new char[length];
			strcpy_s(newData, length, data);
			delete [] data;
			data = newData;
			reserved = length;
		}

		inline void clear() {
			data[0] = 0;
			len = 0;
		}

		inline void append(char c) {
			if(len > reserved)
				reserve(reserved<<1);
			data[len] = c;
			data[++len] = 0;
		}

		void append(const char* buff, int buffLen) {
			if(len + buffLen > reserved)
				reserve(upper_power_of_two(len + buffLen + 1));
			memcpy(data + len, buff, buffLen);
			len += buffLen;
			data[len] = 0;
		}

		inline bool endsWith(const char* str) const {
			size_t pLen = strlen(str);
			if(pLen > len) return false;
			return (strcmp(&data[len-pLen], str) == 0);
		}

		inline size_t size() const {
			return len;
		}

		inline char* c_str() const {
			return data;
		}

		inline AppendString& operator=(const AppendString& str) {
			if(&str == this) return *this;
			delete [] data;
			assign(str);
			return *this;
		}

		inline bool operator==(const char* str) const {
			return (strcmp(data, str) == 0);
		}

		inline bool operator!=(const char* str) const {
			return (strcmp(data, str) != 0);
		}

		inline char operator[](int index) const {
			return data[index];
		}
	};
};
