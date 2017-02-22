#ifndef ELF_INFLECT_H_
#define ELF_INFLECT_H_

#include <string>
#include <elf.h>

class ElfInflect{
public:
	ElfInflect(std::string &file_name);
	~ElfInflect();

public:
	bool start_inflect();
	bool parse_elf();
	bool elf_to_yaml();
	bool yaml_to_elf();
	bool fix_yaml();
	bool rewrite_elf();

private:
	bool make_copy();

private:
	std::string org_elf_name_;
};

#endif
