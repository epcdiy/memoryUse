
// stdafx.cpp : 只包括标准包含文件的源文件
// Teleprompter.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"
#include <io.h>
#define findfirst _findfirst64
#define findnext _findnext64
#define finddata_t __finddata64_t
#define fileHandle	__int64

void getFiles(const char* pdir, char* pfiletype, std::vector<xstring>& fileList, bool recursion /*= false*/)
{
	if (pdir == NULL || pfiletype == NULL)
		return;

	struct   finddata_t   c_file = { 0 };

	char   szTem[MAX_PATH] = { 0 };
	char   szDir[MAX_PATH] = { 0 };
	strcpy_s(szTem, pdir);
	if (szTem[strlen(szTem) - 1] != '\\' || szTem[strlen(szTem) - 1] != '/')
	{
		strcat_s(szTem, "/");
	}

	strcpy_s(szDir, szTem);
	strcat_s(szDir, pfiletype);

	fileHandle   hFile = findfirst(szDir, &c_file);
	if (hFile == -1)
	{
		return;
	}

	do
	{

		if (strcmp(c_file.name, ".") == 0 ||
			strcmp(c_file.name, "..") == 0 ||
			strcmp(c_file.name, ".svn") == 0)
		{
			continue;
		}

		if (c_file.attrib   &   _A_SUBDIR &&recursion == true)
		{
			//   subdir
			char   szSub[MAX_PATH] = { 0 };
			strcpy_s(szSub, pdir);
			if (szSub[strlen(szSub) - 1] != '\\' || szSub[strlen(szSub) - 1] != '/')
			{
				strcat_s(szSub, "/");
			}
			strcat_s(szSub, c_file.name);
			getFiles(szSub, pfiletype, fileList, recursion);
		}
		else
		{
			//printf( "find   the   file:   %s ",   c_file.name);
			fileList.push_back(szTem + std::string(c_file.name));
		}

	} while (findnext(hFile, &c_file) == 0);


	_findclose(hFile);
}

void splitString(const xstring&str, const xstring& delims, std::vector<xstring>& ret)
{
	if (str.empty() == true)
	{
		return;
	}

	ret.clear();

	unsigned int numSplits = 0;

	// Use STL methods
	size_t start, pos;
	start = 0;
	do
	{
#ifdef WIN32
		pos = str.find(delims, start);
#else
		pos = str.find_first_of(delims, start);
#endif
		if (pos == start)
		{
			// Do nothing
			start = pos + 1;
		}
		else if (pos == xstring::npos)
		{
			// Copy the rest of the string
			ret.push_back(str.substr(start));
			break;
		}
		else
		{
			// Copy up to delimiter
			ret.push_back(str.substr(start, pos - start));
			start = pos + 1;
		}
		// parse up to next real data
		start = str.find_first_not_of(delims, start);
		++numSplits;

	} while (pos != xstring::npos);


	return;
}

xstring loadFile(const xstring& path)
{
	FILE *fp = fopen(path.c_str(), "rb");
	if (fp == NULL)
	{
		return "";
	}
	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);
	rewind(fp);
	char *buffer = new char[len + 1];
	memset(buffer, 0, len + 1);
	fread(buffer, 1, len, fp);
	fclose(fp);
	xstring ret = xstring(buffer, len);
	delete[]buffer;
	return ret;
}

bool IsTextUTF8(const char* str, LONG64 length)
{
	int i;
	LONG64 nBytes = 0;//UFT8可用1-6个字节编码,ASCII用一个字节
	unsigned char chr;
	bool bAllAscii = true; //如果全部都是ASCII, 说明不是UTF-8
	for (i = 0; i < length; i++)
	{
		chr = *(str + i);
		if ((chr & 0x80) != 0) // 判断是否ASCII编码,如果不是,说明有可能是UTF-8,ASCII用7位编码,但用一个字节存,最高位标记为0,o0xxxxxxx
			bAllAscii = false;
		if (nBytes == 0) //如果不是ASCII码,应该是多字节符,计算字节数
		{
			if (chr >= 0x80)
			{
				if (chr >= 0xFC && chr <= 0xFD)
					nBytes = 6;
				else if (chr >= 0xF8)
					nBytes = 5;
				else if (chr >= 0xF0)
					nBytes = 4;
				else if (chr >= 0xE0)
					nBytes = 3;
				else if (chr >= 0xC0)
					nBytes = 2;
				else
				{
					return false;
				}
				nBytes--;
			}
		}
		else //多字节符的非首字节,应为 10xxxxxx
		{
			if ((chr & 0xC0) != 0x80)
			{
				return false;
			}
			nBytes--;
		}
	}
	if (nBytes > 0) //违返规则
	{
		return false;
	}
	if (bAllAscii) //如果全部都是ASCII, 说明不是UTF-8
	{
		return false;
	}
	return true;
}

xstring UTF_8ToGb2312(const char* utf8, bool force /*= false*/)
{
	if (utf8 == NULL)
		return ("");
	if (!force)
	{
		if (IsTextUTF8(utf8, strlen(utf8)) == false)
		{
			return utf8;
		}
	}


	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr)
		delete[] wstr;
	xstring ccc = str;
	delete[] str;

	return ccc;
}
