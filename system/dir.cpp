#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string>
#include <stack>
#include <regex.h>

#include "dir.h"

using namespace std;

//枚举文件／目录，返回找到的文件个数
//鉴于最近才开始在linux下工作，就一个寻找文件函数费了我2个小时时间，但愿后来者能节省这2个小时，多干些别的事情.
//Hope.
size_t enum_files(
		const char* path,                          //需要枚举的路径
		FilePathArr& out,                          //枚举结果输出存储
		const char* filter,                  //通配符过滤器，支持＊跟？匹配，比如“＊.jpg", "?.jpg"
		bool inc_sub_dirs,                  //是否包含子目录，枚举的时候需要把子目录也给翻一遍吗?
		bool clear_out,                     //执行前是否清空out中的数据
		EnumFileType type,     //指定需要查找的文件类型，是目录呢，还是文件
		unsigned int nFilePerDir               //指定每个文件夹中只搜寻指定个文件／目录即可
		)
{
	char real_path[260];
	size_t length = strlen(path);
	if(clear_out)
		out.clear();
	strcpy(real_path, path);
	if (real_path[length - 1] != '/')
		strcat(real_path, "/");
	stack<string> ps;
	size_t nOldCount = out.size();
	ps.push(real_path);
	while(!ps.empty())  {
		unsigned int nAlreadyCount = 0;
		string search_path = ps.top();
		ps.pop();
		DIR* dir = opendir((search_path).c_str());
		if(dir != 0)  {
			struct stat file_stat;
			struct dirent *s_dir = readdir(dir);
			do{
				if ((strcmp(s_dir->d_name,".")==0)||(strcmp(s_dir->d_name,"..")==0))
					continue;
				stat((search_path + s_dir->d_name).c_str(), &file_stat);
				bool curIsDir = S_ISDIR(file_stat.st_mode);
				if ((type == EnumFileType_File && !curIsDir) ||  (type == EnumFileType_Directory && curIsDir)){
					// printf("search file path : %s \n", s_dir->d_name);
					if (match(filter, (const char *)s_dir->d_name)) {
						out.push_back(search_path + s_dir->d_name);
					}
					if (inc_sub_dirs)
						ps.push(search_path + s_dir->d_name + "/");
					if(nFilePerDir > 0 && ++nAlreadyCount == nFilePerDir)
						break;
				}
				else{
					if (inc_sub_dirs && curIsDir)
						ps.push(search_path + s_dir->d_name + "/");
				}
			}while((s_dir = readdir(dir)) != 0);
			closedir(dir);
		}
	}
	return out.size() - nOldCount;
}


// match: 在text中查找regexp
bool match(const char *regexp, const char *text) {
	bool ret = false;
	regex_t reg;
	regmatch_t pm[1];
	int  iret = 0;
	/*编译正则表达式*/
	iret = regcomp(&reg, regexp, REG_EXTENDED|REG_NEWLINE);
	if (iret != 0){
		return false;
	}
	do{
		iret = regexec(&reg, text, 1, pm, 0);
		if (iret == REG_NOMATCH){
			break;
		}
		ret = true;
		break;
	}while(true);

	regfree(&reg);
	return ret;
}
