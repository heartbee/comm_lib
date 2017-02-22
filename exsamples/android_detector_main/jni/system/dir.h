#include <vector>
#include <string>
typedef std::vector<std::string> FilePathArr;
enum EnumFileType  {
	EnumFileType_File,
	EnumFileType_Directory
};

size_t enum_files(
		const char* path,                          //需要枚举的路径
		FilePathArr& out,                          //枚举结果输出存储
		const char* filter = "*",                  //通配符过滤器，支持＊跟？匹配，比如“＊.jpg", "?.jpg"
		bool inc_sub_dirs = true,                  //是否包含子目录，枚举的时候需要把子目录也给翻一遍吗?
		bool clear_out = true,                     //执行前是否清空out中的数据
		EnumFileType type = EnumFileType_File,     //指定需要查找的文件类型，是目录呢，还是文件
		unsigned int nFilePerDir = 0               //指定每个文件夹中只搜寻指定个文件／目录即可
		);

bool match(const char *regexp, const char *text);
