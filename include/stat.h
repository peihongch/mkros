#ifndef __STAT_H_
#define __STAT_H_

#include "types.h"

#define T_DIR 1	   // Directory
#define T_FILE 2   // File
#define T_DEVICE 3 // Device

struct stat {
	int		 dev;	// File system's disk device
	uint	 ino;	// Inode number
	short	 type;	// Type of file
	short	 nlink; // Number of links to file
	uint64_t size;	// Size of file in bytes
};

#endif // __STAT_H_