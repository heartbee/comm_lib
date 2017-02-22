#ifndef ELF_BASE_H_
#define ELF_BASE_H_

#include "../common/Endian.h"
#include "../common/AlignOf.h"
#include "../common/ArrayRef.h"
#include "../common/ErrorOr.h"
#include "elf.h"

/*
enum endianness {big, little, native};

template <endianness E, bool Is64> struct ELFType {
private:
  template <typename Ty>
  using packed = llvm::support::detail::packed_endian_specific_integral<Ty, E, 2>;

public:
  static const endianness TargetEndianness = E;
  static const bool Is64Bits = Is64;

  typedef typename std::conditional<Is64, uint64_t, uint32_t>::type uint;
  typedef Elf_Ehdr_Impl<ELFType<E, Is64>> Ehdr;
  typedef Elf_Shdr_Impl<ELFType<E, Is64>> Shdr;
  typedef Elf_Sym_Impl<ELFType<E, Is64>> Sym;
  typedef Elf_Dyn_Impl<ELFType<E, Is64>> Dyn;
  typedef Elf_Phdr_Impl<ELFType<E, Is64>> Phdr;
  typedef Elf_Rel_Impl<ELFType<E, Is64>, false> Rel;
  typedef Elf_Rel_Impl<ELFType<E, Is64>, true> Rela;
  typedef Elf_Verdef_Impl<ELFType<E, Is64>> Verdef;
  typedef Elf_Verdaux_Impl<ELFType<E, Is64>> Verdaux;
  typedef Elf_Verneed_Impl<ELFType<E, Is64>> Verneed;
  typedef Elf_Vernaux_Impl<ELFType<E, Is64>> Vernaux;
  typedef Elf_Versym_Impl<ELFType<E, Is64>> Versym;
  typedef Elf_Hash_Impl<ELFType<E, Is64>> Hash;
  typedef Elf_GnuHash_Impl<ELFType<E, Is64>> GnuHash;
  typedef Elf_Chdr_Impl<ELFType<E, Is64>> Chdr;
  typedef ArrayRef<Dyn> DynRange;
  typedef ArrayRef<Shdr> ShdrRange;
  typedef ArrayRef<Sym> SymRange;
  typedef ArrayRef<Rel> RelRange;
  typedef ArrayRef<Rela> RelaRange;
  typedef ArrayRef<Phdr> PhdrRange;

  typedef packed<uint16_t> Half;
  typedef packed<uint32_t> Word;
  typedef packed<int32_t> Sword;
  typedef packed<uint64_t> Xword;
  typedef packed<int64_t> Sxword;
  typedef packed<uint> Addr;
  typedef packed<uint> Off;
};

typedef ELFType<support::little, false> ELF32LE;
typedef ELFType<support::big, false> ELF32BE;
typedef ELFType<support::little, true> ELF64LE;
typedef ELFType<support::big, true> ELF64BE;

class ELF32File {

public:
	const uint8_t *base() const {
		return reinterpret_cast<const uint8_t *>(Buf.data());
	}

	size_t getBufSize() const {
		return Buf.size();
	}

private:
public:
  template<typename T>
  const T        *getEntry(uint32_t Section, uint32_t Entry) const;
  template <typename T>
  const T *getEntry(const Elf_Shdr *Section, uint32_t Entry) const;

  llvm::ErrorOr<llvm::StringRef> getStringTable(const Elf_Shdr *Section) const;
  llvm::ErrorOr<llvm::StringRef> getStringTableForSymtab(const Elf_Shdr &Section) const;

  llvm::ErrorOr<llvm::ArrayRef<Elf_Word>> getSHNDXTable(const Elf_Shdr &Section) const;

  void VerifyStrTab(const Elf_Shdr *sh) const;

  llvm::StringRef getRelocationTypeName(uint32_t Type) const;
  void getRelocationTypeName(uint32_t Type, llvm::SmallVectorImpl<char> &Result) const;

  /// \brief Get the symbol for a given relocation.
  const Elf_Sym *getRelocationSymbol(const Elf_Rel *Rel,
                                     const Elf_Shdr *SymTab) const;

  ELF32File(llvm::StringRef Object, std::error_code &EC);

  bool isMipsELF64() const {
    return Header->e_machine == ELF::EM_MIPS &&
      Header->getFileClass() == ELF::ELFCLASS64;
  }

  bool isMips64EL() const {
    return Header->e_machine == llvm::ELF::EM_MIPS &&
      Header->getFileClass() == ELF::ELFCLASS64 &&
      Header->getDataEncoding() == ELF::ELFDATA2LSB;
  }

  const Elf_Shdr *section_begin() const;
  const Elf_Shdr *section_end() const;
  Elf_Shdr_Range sections() const {
    return makeArrayRef(section_begin(), section_end());
  }

  const Elf_Sym *symbol_begin(const Elf_Shdr *Sec) const {
    if (!Sec)
      return nullptr;
    if (Sec->sh_entsize != sizeof(Elf_Sym))
      // report_fatal_error("Invalid symbol size");
    return reinterpret_cast<const Elf_Sym *>(base() + Sec->sh_offset);
  }
  const Elf_Sym *symbol_end(const Elf_Shdr *Sec) const {
    if (!Sec)
      return nullptr;
    uint64_t Size = Sec->sh_size;
    if (Size % sizeof(Elf_Sym)) {
    	//report_fatal_error("Invalid symbol table size");
    }
    return symbol_begin(Sec) + Size / sizeof(Elf_Sym);
  }
  Elf_Sym_Range symbols(const Elf_Shdr *Sec) const {
    return makeArrayRef(symbol_begin(Sec), symbol_end(Sec));
  }

  const Elf_Rela *rela_begin(const Elf_Shdr *sec) const {
    if (sec->sh_entsize != sizeof(Elf_Rela))
      report_fatal_error("Invalid relocation entry size");
    return reinterpret_cast<const Elf_Rela *>(base() + sec->sh_offset);
  }

  const Elf_Rela *rela_end(const Elf_Shdr *sec) const {
    uint64_t Size = sec->sh_size;
    if (Size % sizeof(Elf_Rela))
      report_fatal_error("Invalid relocation table size");
    return rela_begin(sec) + Size / sizeof(Elf_Rela);
  }

  Elf_Rela_Range relas(const Elf_Shdr *Sec) const {
    return makeArrayRef(rela_begin(Sec), rela_end(Sec));
  }

  const Elf_Rel *rel_begin(const Elf_Shdr *sec) const {
    if (sec->sh_entsize != sizeof(Elf_Rel))
      report_fatal_error("Invalid relocation entry size");
    return reinterpret_cast<const Elf_Rel *>(base() + sec->sh_offset);
  }

  const Elf_Rel *rel_end(const Elf_Shdr *sec) const {
    uint64_t Size = sec->sh_size;
    if (Size % sizeof(Elf_Rel))
      report_fatal_error("Invalid relocation table size");
    return rel_begin(sec) + Size / sizeof(Elf_Rel);
  }

  Elf_Rel_Range rels(const Elf_Shdr *Sec) const {
    return makeArrayRef(rel_begin(Sec), rel_end(Sec));
  }

  /// \brief Iterate over program header table.
  const Elf_Phdr *program_header_begin() const {
    if (Header->e_phnum && Header->e_phentsize != sizeof(Elf_Phdr))
      // report_fatal_error("Invalid program header size");
    return reinterpret_cast<const Elf_Phdr *>(base() + Header->e_phoff);
  }

  const Elf_Phdr *program_header_end() const {
    return program_header_begin() + Header->e_phnum;
  }

  const Elf_Phdr_Range program_headers() const {
    return makeArrayRef(program_header_begin(), program_header_end());
  }

  uint64_t getNumSections() const;
  uint32_t getStringTableIndex() const;
  uint32_t getExtendedSymbolTableIndex(const Elf_Sym *Sym,
                                       const Elf_Shdr *SymTab,
                                       llvm::ArrayRef<Elf_Word> ShndxTable) const;
  uint32_t getExtendedSymbolTableIndex(const Elf_Sym *Sym,
                                       const Elf_Sym *FirstSym,
                                       llvm::ArrayRef<Elf_Word> ShndxTable) const;
  const Elf_Ehdr *getHeader() const { return Header; }
  llvm::ErrorOr<const Elf_Shdr *> getSection(const Elf_Sym *Sym,
                                       const Elf_Shdr *SymTab,
                                       llvm::ArrayRef<Elf_Word> ShndxTable) const;
  llvm::ErrorOr<const Elf_Shdr *> getSection(uint32_t Index) const;

  const Elf_Sym *getSymbol(const Elf_Shdr *Sec, uint32_t Index) const {
    return &*(symbol_begin(Sec) + Index);
  }

  llvm::ErrorOr<llvm::StringRef> getSectionName(const Elf_Shdr *Section) const;
  template <typename T>
  llvm::ErrorOr<llvm::ArrayRef<T>> getSectionContentsAsArray(const Elf_Shdr *Sec) const;
  llvm::ErrorOr<llvm::ArrayRef<uint8_t> > getSectionContents(const Elf_Shdr *Sec) const;

  llvm::StringRef Buf;

  const Elf_Ehdr *Header;
  const Elf_Shdr *SectionHeaderTable = nullptr;
  llvm::StringRef DotShstrtab;                    // Section header string table.

};
*/
#endif
