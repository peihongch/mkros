#ifndef __ELF_H_
#define __ELF_H_

#include "types.h"

// Format of an ELF executable file

#define ELF_MAGIC 0x464C457FU // "\x7FELF" in little endian

// File header
struct elfhdr {
	uint	 magic; // must equal ELF_MAGIC
	uchar	 elf[12];
	ushort	 type;
	ushort	 machine;
	uint	 version;
	uint64_t entry;
	uint64_t phoff;
	uint64_t shoff;
	uint	 flags;
	ushort	 ehsize;
	ushort	 phentsize;
	ushort	 phnum;
	ushort	 shentsize;
	ushort	 shnum;
	ushort	 shstrndx;
};

// Program section header
struct proghdr {
	uint32_t type;
	uint32_t flags;
	uint64_t off;
	uint64_t vaddr;
	uint64_t paddr;
	uint64_t filesz;
	uint64_t memsz;
	uint64_t align;
};

// Values for Proghdr type
#define ELF_PROG_LOAD 1

// Flag bits for Proghdr flags
#define ELF_PROG_FLAG_EXEC 1
#define ELF_PROG_FLAG_WRITE 2
#define ELF_PROG_FLAG_READ 4

#endif // __ELF_H_