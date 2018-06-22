/*
	ROMFS format:
		ROMDIR section
		EXTINFO section
		File data section
	All files will have an entry in all three sections. The ROMDIR section is terminated by an entry that consists of zeros.

	Required file entries (In this order):
		RESET	(0-byte)
		ROMDIR	(The size of the whole ROMDIR section)
		EXTINFO	(The size of the whole EXTINFO section)

	The EXTINFO section (Extended Information section) contains more information on the file (E.g. date and version numbers) and comments on the file.

	The EXTINFO section is also a file! (In fact, all sections are files)

	All sections and files are (must be?) aligned to 16-byte boundaries, and all records within each section must be aligned to 4-byte boundaries.
*/

struct RomImgData{
	const void *ImageStart;
	const void *RomdirStart;
	const void *RomdirEnd;
};

struct RomDirEntry{
	char name[10];
	u16 ExtInfoEntrySize;
	u32 size;
};
