#include "elf32_parse.h"
#include "../utils/PrintLog.h"
#include "../system/system.h"
#include <assert.h>
#include <vector>



Elf32Parse::Elf32Parse(std::string &file_name) : file_name_(file_name) {
	// read file, and initlization ehdr_ptr_
	ehdr_ptr_ = NULL;
	elf_start_ = NULL;

	init_success_ = false;
	file_size_ = 0;

	phdr_table_ = NULL;
	phdr_num_ = 0;

	shdr_table_ = NULL;
	shdr_num_ = 0;

	sec_name_table_start_ = 0;
	sec_name_table_size_ = 0;

	dyn_sec_table_start_ = 0;
	dyn_sec_table_size_ = 0;

	init();
}

Elf32Parse::~Elf32Parse() {

}

void Elf32Parse::main() {

	/*
	print_section_headers();
	// 解析动态符号表
	print_dynsym_sec_info();
	print_rela_sec_info();
	print_rel_sec_info();

	print_sym_sec_info(); // SHT_SYMTAB
	print_hash_sec_info(); // SHT_HASH
	print_dynamic_sec_info(); //SHT_DYNAMIC

	print_program_headers();
	*/

	uint32_t got_table_base = 0;
	uint16_t got_table_size = 0;
	const std::string linker_path = "/system/bin/linker";
	if (parse_got_table(got_table_base, got_table_size, true)) {
		printf("parse got table success \n");

		std::string linker_name = "";
		size_t pos = linker_path.rfind("/");
		if (pos == std::string::npos || std::string::npos != linker_path.find("/system/")) {
			linker_name = linker_path;
		}
		else {
			linker_name = linker_path.substr(pos+1);
		}
		printf("linker name : %s \n", linker_name.c_str());
		size_t link_base = SystemInterface::get_module_base(-1, linker_name);
		printf("linker base : %08x, got offset : %08x, got size : %08x \n", link_base, got_table_base, got_table_size);
		uint32_t* ptr = (uint32_t*)(got_table_base + link_base);
		while ((uint32_t)ptr <= (uint32_t)link_base + got_table_base + got_table_size - 4) {
			printf("got_table base : %08x \n", *ptr);
			ptr ++;
		}
	}
	else {
		printf("parse got table failed \n");
	}




}

void Elf32Parse::init() {
	FILE* file= fopen(file_name_.c_str(), "rb");
	if(NULL == file){
		return;
	}
	fseek(file, 0, SEEK_END);
	file_size_ = ftell(file);
	fseek(file, 0, SEEK_SET);
	elf_start_ = (uint8_t *)malloc(file_size_);
	if(NULL == elf_start_) {
		printf("alloc memory failed \n");
		return;
	}

	memset(elf_start_, 0, file_size_);
    size_t read_len = fread(elf_start_, 1, file_size_, file);
    if(read_len != file_size_) {
        printf("[ERROR] read file %s failed!\n", file_name_.c_str());
        return;
    }
    fclose(file);

    ehdr_ptr_ = (Elf32_Ehdr*)elf_start_;
    if (sizeof(Elf32_Phdr) != ehdr_ptr_->e_phentsize) {
    	printf("[ERROR] e_phentsize not match Elf32_Phdr \n");
    	return;
    }

    phdr_table_ = (Elf32_Phdr*)(ehdr_ptr_->e_phoff + elf_start_);
    phdr_num_ = ehdr_ptr_->e_phnum;

    if (sizeof(Elf32_Shdr) != ehdr_ptr_->e_shentsize) {
    	printf("[ERROR] e_shentsize not match Elf32_Shdr");
    	return;
    }

    shdr_table_ = (Elf32_Shdr*)(ehdr_ptr_->e_shoff + elf_start_);
    shdr_num_ = ehdr_ptr_->e_shnum;

    if (shdr_num_ > 0) {
    	if (!parse_sec_strtab()) {
    		printf("parse section strtable failed \n");
    		return;
    	}
    }
    init_success_ = true;
}

bool Elf32Parse::parse_load_seg_header(std::vector<Elf32_Phdr *> &segs){
	return parse_seg_headers(segs, PT_LOAD);
}

bool Elf32Parse::parse_dynamic_seg_header(std::vector<Elf32_Phdr *> &segs){
	return parse_seg_headers(segs, PT_DYNAMIC);
}

bool Elf32Parse::parse_other_seg_headers(std::vector<Elf32_Phdr *> &segs) {
	assert(NULL != phdr_table_);
	Elf32_Phdr* phdr_ptr = phdr_table_;
	do {
		if (is_other_segs(phdr_ptr->p_type)){
			segs.push_back(phdr_ptr);
		}
		phdr_ptr ++;
	}while(phdr_ptr != phdr_table_ + phdr_num_);
	return !segs.empty();
}


bool Elf32Parse::parse_seg_headers(std::vector<Elf32_Phdr *> &segs, size_t type){
	assert(NULL != phdr_table_);
	Elf32_Phdr* phdr_ptr = phdr_table_;
	do {
		if (phdr_ptr->p_type == type){
			segs.push_back(phdr_ptr);
		}
		phdr_ptr ++;
	}while(phdr_ptr != phdr_table_ + phdr_num_);
	return !segs.empty();
}

bool Elf32Parse::parse_sec_headers(std::vector<Elf32_Shdr *> &secs, size_t type){
	assert(NULL != shdr_table_);
	Elf32_Shdr* shdr_ptr = shdr_table_;
	do {
		if (shdr_ptr->sh_type == type) {
			secs.push_back(shdr_ptr);
		}
		shdr_ptr ++;
	}while(shdr_ptr != shdr_table_ + shdr_num_);
	return !secs.empty();
}

// print section info
void Elf32Parse::print_section_headers(){

	Elf32_Shdr* shdr_ptr = shdr_table_;

	do {
		const char * tmp = NULL;
		switch (shdr_ptr->sh_type) {
		case SHT_NULL:
			tmp = "NULL";
			break;

		case SHT_PROGBITS:
			tmp = "SHT_PROGBITS";
			break;

		case SHT_SYMTAB:
			tmp = "SHT_SYMTAB";
			break;

		case SHT_STRTAB:
			tmp = "SHT_STRTAB";
			break;

		case SHT_RELA:
			tmp = "SHT_RELA";
			break;

		case SHT_HASH:
			tmp = "SHT_HASH";
			break;

		case SHT_DYNAMIC:
			tmp = "SHT_DYNAMIC";
			break;

		case SHT_NOTE:
			tmp = "SHT_NOTE";
			break;

		case SHT_NOBITS:
			tmp = "SHT_NOBITS";
			break;

		case SHT_REL:
			tmp = "SHT_REL";
			break;

		case SHT_SHLIB:
			tmp = "SHT_SHLIB";
			break;

		case SHT_DYNSYM:
			tmp = "SHT_DYNSYM";
			break;

		case SHT_NUM:
			tmp = "SHT_NUM";
			break;

		default:
			tmp = "UNKOWN";
			break;
		}
		printf("sec name : %s sh_type : %04x[%s], sh_flags : %04x \n", sec_name_table_start_ + shdr_ptr->sh_name, shdr_ptr->sh_type, tmp, shdr_ptr->sh_flags);
		printf("sh_addr : %08x, sh_offset : %08x, sh_size : %04x,\n", shdr_ptr->sh_addr, shdr_ptr->sh_offset, shdr_ptr->sh_size);
		printf("sh_link : %04x, sh_info : %04x sh_addralign : %04x, sh_entsize : %04x \n", shdr_ptr->sh_link, shdr_ptr->sh_info, shdr_ptr->sh_addralign, shdr_ptr->sh_entsize);
		printf("-------------- \n");
		shdr_ptr ++;
	}while(shdr_ptr != shdr_table_ + shdr_num_);
}

void Elf32Parse::print_dynsym_sec_info(){
	// parse dynsym section
	// 该section保存着动态符号表，如“Symbol Table”的描述
	printf("print dynsym sec info \n");
	std::vector<Elf32_Shdr*> dynsym_secs;
	if (!parse_dynsym_sec_header(dynsym_secs)) {
		printf("parse dynsym section failed \n");
		return;
	}
	printf("parse dynsym section success \n");

	std::vector<Elf32_Shdr *>::iterator itr = dynsym_secs.begin();
	for (; itr != dynsym_secs.end(); ++ itr) {
		// print name and size
		std::vector<Elf32_Sym *> sym_entrys;
		printf("dynsym section name : %s, offset : %08x, size : %08x \n", sec_name_table_start_ + (*itr)->sh_name, (*itr)->sh_offset, (*itr)->sh_size);

		size_t num = (*itr)->sh_size / sizeof(Elf32_Sym);
		Elf32_Sym* sym_table = (Elf32_Sym* ) (elf_start_ + (*itr)->sh_offset);

		parse_sym_entrys(sym_table, num, sym_entrys);
	}

	return;
}

void Elf32Parse::print_rela_sec_info(){
	printf("print rela sec info \n");
	std::vector<Elf32_Shdr* > rela_secs;
	if (!parse_rela_sec_header(rela_secs)) {
		printf("parse rela section failed \n ");
		return;
	}
	printf("parse rela section success \n ");
	std::vector<Elf32_Shdr *>::iterator itr = rela_secs.begin();
	for (; itr != rela_secs.end(); ++ itr) {
		// print name and size
		std::vector<Elf32_Rela *> rela_entrys;
		printf("rela section name : %s, offset : %08x, size : %08x \n", sec_name_table_start_ + (*itr)->sh_name, (*itr)->sh_offset, (*itr)->sh_size);

		size_t num = (*itr)->sh_size / sizeof(Elf32_Rela);
		Elf32_Rela* rela_table = (Elf32_Rela* ) (elf_start_ + (*itr)->sh_offset);

		parse_rela_entrys(rela_table, num, rela_entrys);
	}

	return;
}

void Elf32Parse::print_rel_sec_info() {
	printf("print rel sec info \n");
	std::vector<Elf32_Shdr* > rel_secs;
	if (!parse_rel_sec_header(rel_secs)) {
		printf("parse rel section failed \n ");
		return;
	}
	printf("parse rel section success \n ");

	std::vector<Elf32_Shdr *>::iterator itr = rel_secs.begin();
	for (; itr != rel_secs.end(); ++ itr) {
		// print name and size
		std::vector<Elf32_Rel *> rel_entrys;
		printf("rel section name : %s, offset : %08x, size : %08x \n", sec_name_table_start_ + (*itr)->sh_name, (*itr)->sh_offset, (*itr)->sh_size);

		size_t num = (*itr)->sh_size / sizeof(Elf32_Rel);
		Elf32_Rel* rel_table = (Elf32_Rel* ) (elf_start_ + (*itr)->sh_offset);

		parse_rel_entrys(rel_table, num, rel_entrys);
	}

	return;
}

void Elf32Parse::print_sym_sec_info() {
	printf("print sym sec info \n");
	std::vector<Elf32_Shdr *> sym_secs;
	if (!parse_sym_sec_header(sym_secs)) {
		printf("parse sym section info failed \n ");
	}

	printf("parse sym section info success \n");
	std::vector<Elf32_Shdr *>::iterator itr = sym_secs.begin();
	for (; itr != sym_secs.end(); ++ itr) {
		// print name and size
		std::vector<Elf32_Sym *> sym_entrys;
		printf("sym section name : %s, offset : %08x, size : %08x \n", sec_name_table_start_ + (*itr)->sh_name, (*itr)->sh_offset, (*itr)->sh_size);
		size_t num = (*itr)->sh_size / sizeof(Elf32_Sym);
		Elf32_Sym* sym_table = (Elf32_Sym* ) (elf_start_ + (*itr)->sh_offset);
		parse_sym_entrys(sym_table, num, sym_entrys);
	}
	return;
}

void Elf32Parse::print_hash_sec_info() {
	printf("print hash sec info \n");
	std::vector<Elf32_Shdr *> hash_secs;
	if (!parse_hash_sec_header(hash_secs)) {
		printf("parse hash section info failed \n ");
	}

	printf("parse hash section info success \n");
	std::vector<Elf32_Shdr *>::iterator itr = hash_secs.begin();
	for (; itr != hash_secs.end(); ++ itr) {
		// print name and size
		std::vector<uint32_t *> hash_entrys;
		printf("hash section name : %s, offset : %08x, size : %08x \n", sec_name_table_start_ + (*itr)->sh_name, (*itr)->sh_offset, (*itr)->sh_size);
		size_t num = (*itr)->sh_size / sizeof(uint32_t);
		uint32_t* hash_table = (uint32_t * ) (elf_start_ + (*itr)->sh_offset);
		parse_hash_entrys(hash_table, num, hash_entrys);
	}
	return;
}

void Elf32Parse::print_dynamic_sec_info(){
	printf("print dynamic sec info \n");
	std::vector<Elf32_Shdr *> dynamic_secs;
	if (!parse_dynamic_sec_header(dynamic_secs)) {
		printf("parse dynamic section info failed \n ");
	}

	printf("parse dynamic section info success \n");
	std::vector<Elf32_Shdr *>::iterator itr = dynamic_secs.begin();
	for (; itr != dynamic_secs.end(); ++ itr) {
		// print name and size
		std::vector<Elf32_Dyn *> dyn_entrys;
		printf("sym section name : %s, offset : %08x, size : %08x \n", sec_name_table_start_ + (*itr)->sh_name, (*itr)->sh_offset, (*itr)->sh_size);
		size_t num = (*itr)->sh_size / sizeof(Elf32_Dyn);
		Elf32_Dyn* dyn_table = (Elf32_Dyn* ) (elf_start_ + (*itr)->sh_offset);

		parse_dynamic_entrys(dyn_table, num, dyn_entrys);
	}
	return;
}

bool Elf32Parse::parse_sym_entrys(Elf32_Sym * sym_table, size_t num,  std::vector<Elf32_Sym *> &sym_entrys) {
	// size_t num = sym_sec->sh_size / sizeof(Elf32_Sym);
	// Elf32_Sym* sym_table = (Elf32_Sym* ) (elf_start_ + sym_sec->sh_offset);
	Elf32_Sym* sym_ptr = sym_table;
	printf("sym table num : %08x \n", num);
	while (sym_ptr != sym_table + num) {
		// print sym base info
		if (0 == sym_ptr->st_name) {
			printf("index 0, continue \n");
			sym_ptr++;
			continue;
		}

		printf("name : %s, value : %08x, size : %08x, info : %02x, other : %02x, shndx : %04x \n",\
				dyn_sec_table_start_ + sym_ptr->st_name, sym_ptr->st_value, sym_ptr->st_size, \
				sym_ptr->st_info, sym_ptr->st_other, sym_ptr->st_shndx);
		sym_entrys.push_back(sym_ptr);
		sym_ptr++;
	}
	return !sym_entrys.empty();
}

bool Elf32Parse::parse_rela_entrys(Elf32_Rela * rela_table, size_t num, std::vector<Elf32_Rela *> &rela_entrys) {
	Elf32_Rela* rela_ptr = rela_table;
	printf("rela table num : %08x \n", num);
	while (rela_ptr != rela_table + num) {
        printf("r_offset : %08x, r_info : %08x, r_addend : %08x \n", rela_ptr->r_offset, rela_ptr->r_info, rela_ptr->r_addend);
        rela_entrys.push_back(rela_ptr);
        rela_ptr++;
	}
	return !rela_entrys.empty();
}

bool Elf32Parse::parse_rel_entrys(Elf32_Rel* rel_table, size_t num, std::vector<Elf32_Rel *> &rel_entrys){

	Elf32_Rel* rel_ptr = rel_table;
	printf("rel table num : %08x \n", num);
	while (rel_ptr != rel_table + num) {
        printf("r_offset : %08x, r_info : %08x  \n", rel_ptr->r_offset, rel_ptr->r_info);
        rel_entrys.push_back(rel_ptr);
        rel_ptr++;
	}

	return !rel_entrys.empty();
}

bool Elf32Parse::parse_hash_entrys(uint32_t * hash_table, size_t num, std::vector<uint32_t *> &hash_entrys){

	uint32_t* hash_ptr = hash_table;
	printf("rel table num : %08x \n", num);
	while (hash_ptr != hash_table + num) {
        printf("hash table : hash : %08x  \n", *hash_ptr);
        hash_entrys.push_back(hash_ptr);
        hash_ptr++;
	}
	return !hash_entrys.empty();
}

bool Elf32Parse::parse_dynamic_entrys(Elf32_Dyn* dyn_table, size_t num, std::vector<Elf32_Dyn *> &dynamic_entrys){

	Elf32_Dyn* dyn_ptr = dyn_table;
	printf("dyn table num : %08x \n", num);
	// parse strtable offset
	size_t str_table_offset = 0;
	size_t str_table_size = 0;
	while (dyn_ptr != dyn_table + num) {
		switch (dyn_ptr->d_tag){
		case DT_STRTAB:
			printf("parse dynamic string table, string offset : % 08x \n", dyn_ptr->d_un.d_val);
			str_table_offset =  dyn_ptr->d_un.d_val;
			break;

		case DT_STRSZ:
			printf("parse dynamic string table size : %08x \n", dyn_ptr->d_un.d_val);
			str_table_size = dyn_ptr->d_un.d_val;
			break;

		default:
			break;
		}
		dyn_ptr ++;
	}

	dyn_ptr = dyn_table;

	while (dyn_ptr != dyn_table + num) {
		switch (dyn_ptr->d_tag) {
		case DT_NULL:
			break;

		case DT_NEEDED:
			// parse nt needed table
			printf("parse dynamic needed lib : %s \n", elf_start_ + str_table_offset + dyn_ptr->d_un.d_val);
			break;

		case DT_PLTRELSZ:
			// parse size of relocation entries in PLT
			break;

		case DT_PLTGOT:
			// address PLT/GOT
			break;

		case DT_HASH:
			//  address of symbol hash table
			break;

		case DT_STRTAB:
			//  address of string table
			break;

		case DT_SYMTAB:
			// address of symbol table
			break;

		case DT_RELA:
			// address of relocation table
			break;

		case DT_RELASZ:
			//  size of relocation table
			break;

		case DT_RELAENT:
			// size of relocation entry
			break;

		case DT_STRSZ:
			// size of string table
			break;

		case DT_SYMENT:
			// size of symbol table entry
			break;

		case DT_INIT:
			// address of initialization func.
			break;

		case DT_FINI:
			// address of termination function
			break;

		case DT_SONAME:
			// string table offset of shared obj
			break;

		case DT_RPATH:
			// string table offset of library search path
			break;

		case DT_SYMBOLIC:
			// start sym search in shared obj.
			break;

		case DT_REL:
			// address of rel. tbl. w addends
			break;

		case DT_RELSZ:
			// size of DT_REL relocation table
			break;

		case DT_RELENT:
			// size of DT_REL relocation entry
			break;

		case DT_PLTREL:
			// PLT referenced relocation entry
			break;

		case DT_DEBUG:
			// debugger
			break;

		case DT_TEXTREL:
			// Allow rel. mod. to unwritable seg
			break;

		case DT_JMPREL:
			// add. of PLT's relocation entries
			break;

		case DT_BIND_NOW:
			// Bind now regardless of env setting
			break;

		case DT_NUM:
			// num used
			break;

		default:
			printf("unknown type \n");
			break;
		}
		dyn_ptr++;
	}
	return !dynamic_entrys.empty();
}

bool Elf32Parse::parse_dynsym_sec_header(std::vector<Elf32_Shdr *> &secs) {
	if (!parse_sec_headers(secs, SHT_DYNSYM)) {
		printf("parse dynsym section header failed \n");
		return false;
	}

	if (!parse_dyn_sec_strtab()) {
		printf("parse dyn section strtable failed \n");
		return false;
	}
	return !secs.empty();
}

bool Elf32Parse::parse_rela_sec_header(std::vector<Elf32_Shdr *> &secs) {
	if (!parse_sec_headers(secs, SHT_RELA)) {
		printf("parse SHT_RELA section header failed \n");
		return false;
	}
	return !secs.empty();
}

bool Elf32Parse::parse_rel_sec_header(std::vector<Elf32_Shdr *> &secs){
	if (!parse_sec_headers(secs, SHT_REL)) {
		printf("parse SHT_RELA section header failed \n");
		return false;
	}
	return !secs.empty();
}

bool Elf32Parse::parse_sym_sec_header(std::vector<Elf32_Shdr *> &secs){
	if (!parse_sec_headers(secs, SHT_SYMTAB)) {
		printf("parse SHT_RELA section header failed \n");
		return false;
	}

	return !secs.empty();
}
bool Elf32Parse::parse_hash_sec_header(std::vector<Elf32_Shdr *> &secs) {
	if (!parse_sec_headers(secs, SHT_HASH)) {
		printf("parse SHT_RELA section header failed \n");
		return false;
	}
	return !secs.empty();
}
bool Elf32Parse::parse_dynamic_sec_header(std::vector<Elf32_Shdr *> &secs) {
	if (!parse_sec_headers(secs, SHT_DYNAMIC)) {
		printf("parse SHT_RELA section header failed \n");
		return false;
	}
	return !secs.empty();
}

bool Elf32Parse::parse_other_sec_headers(std::vector<Elf32_Shdr *> &secs){
	assert(NULL != shdr_table_);
	Elf32_Shdr* shdr_ptr = shdr_table_;
	do {

		if (is_other_secs(shdr_ptr->sh_type)) {
			secs.push_back(shdr_ptr);
		}
		shdr_ptr ++;
	}while(shdr_ptr != shdr_table_ + shdr_num_);
	return !secs.empty();
}

void Elf32Parse::print_program_headers(){
	// Elf32_Phdr* ;
	// size_t phdr_num_;
	/* Segment types - p_type */

	std::vector<Elf32_Dyn *> dynamic_entrys;
	Elf32_Dyn* dyn_table = NULL;
	size_t num = 0;

	Elf32_Phdr* phdr_ptr = phdr_table_;
	while (phdr_ptr != phdr_table_ + phdr_num_) {
		const char * tmp = NULL;
		switch (phdr_ptr->p_type) {
		case PT_LOAD:
			tmp = "PT_LOAD";
			printf("program header ; type : %s, offset : %08x, vaddr : %08x, paddr : %08x, \n \
						filesize : %08x, memsize : %08x, flags : %08x, align : %08x \n", \
						tmp, phdr_ptr->p_offset, phdr_ptr->p_vaddr, phdr_ptr->p_paddr, phdr_ptr->p_filesz, phdr_ptr->p_memsz, phdr_ptr->p_flags, phdr_ptr->p_align);

			break;

		case PT_DYNAMIC:
			/*
			tmp = "PT_DYNAMIC";
			printf("parse dynamic segment info : \n");

			dyn_table = (Elf32_Dyn *)(phdr_ptr->p_offset + elf_start_);
			num = phdr_ptr->p_filesz / sizeof(Elf32_Dyn);
			parse_dynamic_entrys(dyn_table,  num, dynamic_entrys);
			*/
			break;

		case PT_INTERP:
			tmp = "PT_INTERP";
			break;

		case PT_NOTE:
			tmp = "PT_NOTE";
			break;

		case PT_SHLIB:
			tmp = "SHLIB";
			break;

		case PT_PHDR:
			tmp = "PT_PHDR";
			break;

		case PT_NUM:
			tmp = "PT_NUM";
			break;

		default:
			tmp = "UNKOWN";
			break;
		}
		// printf("program header ; type : %s, offset : %08x, vaddr : %08x, paddr : %08x, \n \
		//		filesize : %08x, memsize : %08x, flags : %08x, align : %08x \n", \
		//		tmp, phdr_ptr->p_offset, phdr_ptr->p_vaddr, phdr_ptr->p_paddr, phdr_ptr->p_filesz, phdr_ptr->p_memsz, phdr_ptr->p_flags, phdr_ptr->p_align);

		printf("------------------ \n");
		phdr_ptr ++;
	}


}

void Elf32Parse::print_load_segs(){

}

void Elf32Parse::print_dynamic_segs(){

}

bool Elf32Parse::parse_sec_strtab(){
	bool ret = false;
	std::vector<Elf32_Shdr*> secs;
	if (!parse_sec_headers(secs, SHT_STRTAB)) {
		return false;
	}

	std::vector<Elf32_Shdr *>::iterator itr = secs.begin();
	for (; itr != secs.end(); ++ itr) {
		// get section offset
		if ((*itr)->sh_name  >= (*itr)->sh_size) {
			continue;
		}
		const char * tmp = (const char *)((*itr)->sh_name + elf_start_ + (*itr)->sh_offset);
		if (0 == strncmp(tmp, ".shstrtab", strlen(".shstrtab"))) {
			sec_name_table_start_ = (size_t)elf_start_ + (*itr)->sh_offset;
			sec_name_table_size_ = (*itr)->sh_size;
			printf("sec_name_table base : %08x, size : %08x, name : %s \n", sec_name_table_start_, sec_name_table_size_, sec_name_table_start_+(*itr)->sh_name);
			ret = true;
			break;
		}
		/*

		if ((*itr)->sh_name == 0x01) {
			sec_name_table_start_ = (size_t)elf_start_ + (*itr)->sh_offset;
			sec_name_table_size_ = (*itr)->sh_size;
			printf("sec_name_table base : %08x, size : %08x, name : %s \n", sec_name_table_start_, sec_name_table_size_, sec_name_table_start_+(*itr)->sh_name);
			ret = true;
			break;
		}
		*/
	}
	if (!ret) {
		printf("parse sec name table failed \n");
	}
	return ret;
}

bool Elf32Parse::parse_dyn_sec_strtab() {
	if (!init_success_)
		return false;

	bool ret = false;
	std::vector<Elf32_Shdr*> secs;
	if (!parse_sec_headers(secs, SHT_STRTAB)) {
		return false;
	}
	std::vector<Elf32_Shdr *>::iterator itr = secs.begin();
	for (; itr != secs.end(); ++ itr) {
		// get section offset
		if ((*itr)->sh_name != 0x01) {
			const char* sec_name = (const char *) (sec_name_table_start_ + (*itr)->sh_name);
			if (0 == strcmp(sec_name, ".dynstr")) {
				dyn_sec_table_start_ = (size_t)elf_start_ + (*itr)->sh_offset;
				dyn_sec_table_size_= (*itr)->sh_size;
				printf("parse .dynstr table success, offset : %08x, size : %08x \n", (*itr)->sh_offset, (*itr)->sh_size);
				ret = true;
				break;
			}
		}
	}

	if (!ret) {
		printf("parse dyn sec strtabl failed \n");
	}
	return ret;
}

bool Elf32Parse::is_other_segs(size_t type){
	static size_t exclude_pt_types[] = {PT_DYNAMIC, PT_LOAD};

	for (size_t i = 0; i < sizeof(exclude_pt_types) / sizeof(size_t); ++i) {
		if (type == exclude_pt_types[i]) {
			return false;
		}
	}
	return true;
}

bool Elf32Parse::is_other_secs(size_t type){
	static size_t exclude_sh_types[] = {SHT_RELA, SHT_DYNSYM, SHT_REL, SHT_SYMTAB, SHT_HASH, SHT_DYNAMIC};
	for (size_t i = 0; i < sizeof(exclude_sh_types) / sizeof(size_t); ++i) {
		if (type == exclude_sh_types[i]) {
			return false;
		}
	}
	return true;
}

bool Elf32Parse::parse_got_table(uint32_t &got_table_base, uint16_t &got_table_size, bool voffset) {
	// get sections names
	bool ret = false;
	Elf32_Shdr* shdr_ptr = NULL;
	for (size_t i = 1; i < shdr_num_; ++i) {
		shdr_ptr = shdr_table_ + i;
		const char* sec_name = (const char *) (sec_name_table_start_ + shdr_ptr->sh_name);
		LOGD("Parse got table, table name %s", sec_name);
		if (0 == strncmp(sec_name, ".got", strlen(".got"))) {
			if (voffset) {
				got_table_base = shdr_ptr->sh_addr;
				got_table_size = shdr_ptr->sh_size;
			}
			else {
				got_table_base = shdr_ptr->sh_offset;
				got_table_size = shdr_ptr->sh_size;
			}
			ret = true;
			LOGD("Parse got table success, table base : %08x, table size : %08x", got_table_base, got_table_size);
			break;
		}
	}

	return ret;
}
