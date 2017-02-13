#ifndef ELF_PARSE_H_
#define ELF_PARSE_H_

#include <string>
#include <elf.h>
#include <vector>

class Elf32Parse{
public:
	Elf32Parse(std::string &file_name);
	~Elf32Parse();

public:
	void main();

	// print section info
	void print_section_headers();

	void print_dynsym_sec_info();
	void print_rela_sec_info();
	void print_rel_sec_info();

	void print_sym_sec_info(); // SHT_SYMTAB
	void print_hash_sec_info(); // SHT_HASH
	void print_dynamic_sec_info(); //SHT_DYNAMIC

	// print program header
	void print_program_headers();
	void print_load_segs();
	void print_dynamic_segs();

public:
	bool parse_got_table(uint32_t &got_table_base, uint16_t &got_table_size, bool voffset = false);

	bool parse_load_seg_header(std::vector<Elf32_Phdr *> &segs);
    bool parse_dynamic_seg_header(std::vector<Elf32_Phdr *> &segs);
	bool parse_other_seg_headers(std::vector<Elf32_Phdr *> &segs);
	bool parse_seg_headers(std::vector<Elf32_Phdr *> &segs, size_t type);

	bool parse_dynsym_sec_header(std::vector<Elf32_Shdr *> &secs);
	bool parse_rela_sec_header(std::vector<Elf32_Shdr *> &secs);
	bool parse_rel_sec_header(std::vector<Elf32_Shdr *> &secs);
	bool parse_sym_sec_header(std::vector<Elf32_Shdr *> &secs);
	bool parse_hash_sec_header(std::vector<Elf32_Shdr *> &secs);
	bool parse_dynamic_sec_header(std::vector<Elf32_Shdr *> &secs);
	bool parse_other_sec_headers(std::vector<Elf32_Shdr *> &secs);
	bool parse_sec_headers(std::vector<Elf32_Shdr *> &secs, size_t type);

	bool parse_sec_strtab();
	bool parse_dyn_sec_strtab();

	bool parse_sym_entrys(Elf32_Sym* sym_table, size_t num, std::vector<Elf32_Sym *> &sym_entrys);
	bool parse_rela_entrys(Elf32_Rela * rela_table, size_t num, std::vector<Elf32_Rela *> &rela_entrys);
	bool parse_rel_entrys(Elf32_Rel * rel_table, size_t num,  std::vector<Elf32_Rel *> &rel_entrys);
	bool parse_hash_entrys(uint32_t * hash_table, size_t num, std::vector<uint32_t *> &hash_entrys);
	bool parse_dynamic_entrys(Elf32_Dyn * dyn_table, size_t num, std::vector<Elf32_Dyn *> &dynamic_entrys);

	bool is_other_segs(size_t type);
	bool is_other_secs(size_t type);

private:
	void init();

private:
	Elf32_Ehdr* ehdr_ptr_;

	Elf32_Phdr* phdr_table_;
	size_t phdr_num_;

	Elf32_Shdr* shdr_table_;
	size_t shdr_num_;

	size_t sec_name_table_start_;
	size_t sec_name_table_size_;

	size_t dyn_sec_table_start_;
	size_t dyn_sec_table_size_;

	uint8_t* elf_start_;

	bool init_success_;

	size_t file_size_;
	std::string file_name_;
};
#endif
