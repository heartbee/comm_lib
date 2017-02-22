#include "elf_inflect.h"
#include <unistd.h>

ElfInflect::ElfInflect(std::string &file_name) {
	org_elf_name_ = file_name;
}

ElfInflect::~ElfInflect() {

}

bool ElfInflect::start_inflect(){
	if (!parse_elf()) {
		return false;
	}
	if (!elf_to_yaml()) {
		return false;
	}
	if (!fix_yaml()) {
		return false;
	}
	if (!yaml_to_elf()) {
		return false;
	}
	if (!rewrite_elf()) {
		return false;
	}
	return false;
}

bool ElfInflect::parse_elf() {
	return false;
}

bool ElfInflect::elf_to_yaml(){
	return false;
}

bool ElfInflect::yaml_to_elf(){
	return false;
}

bool ElfInflect::fix_yaml() {
	return false;
}

bool ElfInflect::make_copy(){
	return false;
}

bool ElfInflect::rewrite_elf() {
	make_copy();
	return false;
}
