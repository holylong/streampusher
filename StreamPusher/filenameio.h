/*************************************************************************
	> File Name: filenameio.h
	> Author: lcmf
	> Mail: mrhlingchen@163.com 
	> Created Time: 2018年03月12日 星期一 14时31分22秒
 ************************************************************************/
#ifndef _FILE_NAME_IO_H__
#define _FILE_NAME_IO_H__

#include<iostream>
#include <stdio.h>
#include <vector>
#include "ever.h"

#ifndef _WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#else
// #include <iconv.h>
#include <windows.h>
#endif
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

/*
	函数名称：GetFileLastModifyTime(char *strPath, int *size, long *modify_time)
	函数作用：获取文件的最后修改时间
	输入参数：char * strPath 需要的文件
	输出参数：文件大小
	输出参数：最后修改的时间
*/
void GetFileLastModifyTime(char *strPath,int *size,long *modify_time);

string ReplaceStr(string strSrc, const char * chBar);


string LongToStr(uint64_t num);


//根据时间在某个文件夹下创建文件夹,并返回文件夹名称
char* CreateDirectoryByTime(char *chPath);


//根据文件名称创建文件夹,返回值为0表示创建成功，返回1表示文件夹已存在，返回-1表示创建失败位置原因
int createDirectory(const char* strPath);

int gb2312toutf8(const char *sourcebuf, size_t sourcelen, char *destbuf, size_t destlen);

int utf8togb2312(const char *sourcebuf, size_t sourcelen, char *destbuf, size_t destlen);

//创建深度文件夹
int safeCreateDirectory(char* strPath);


#ifndef _WIN32
//获取当前系统时间具体到分秒
int GetCurrentTime();
#endif

//获取当前时间具体到小时
int GetCurrentHour(char* strCurrnet);

//遍历文件夹
void ReverseDirctory(const char* strPath, std::vector<std::string> & arrJsonFilePath);

/*
	函数名称：scanDir
	函数作用：遍历文件夹内容
	输入参数：const char* dir ,输入文件夹
	输出参数：vector<string>& 输出内容
	输入参数：int depth 遍历深度
	输入参数：bool bDirOrFile 输出是文件还是文件夹，true表示只遍历文件夹，false表示只遍历文件
	返回值：无
*/
// void scanDir(const char *dir, std::vector<std::string>& arrJsonPath, int depth, bool bDirOrFile);
void EXPORT_DLL scanDir(const std::string& path, std::vector<std::string>& arrJsonPath, int depth, bool bDirOrFile);

/*
	函数名称:IntToStr(int num)
	函数作用：整形转字符串
	输入参数：int num 需要处理的数字
	返回值：输出的字符串
*/
std::string IntToStr(int num);

/*
	行数名称：
	函数作用：获取当前执行程序路径
	输出参数：
*/
size_t GetCurrentExePath( char* processdir,char* processname, size_t len);


//剪切文件
bool CutFile(std::string strSrc, std::string strDis);


//删除所有文件
bool removeAllFile(char *strDir);


//substring
std::string substr(char* str, unsigned int start, unsigned int end);

/*
	函数名称：SplitWords(char* strSrc, std::vector<std::string> & arrWords);
	函数作用:分割字符串
*/
int SplitWords(char* strSrc, std::vector<std::string> & arrStr, char chStr = '_');

#ifdef __cplusplus
}
#endif

#endif  //FILENAMEIO__
