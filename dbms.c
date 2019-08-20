#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <malloc.h>

#include "ps2db.h"
#include "dbms.h"

struct PS2IDBPeripheral{
	void *models;
	unsigned short int NumModels;
};

static struct PS2IDBPeripheral peripherals[PS2IDB_COMPONENT_COUNT];

static int PS2IDBMS_LoadMoboDatabase(FILE *DatabaseFile, unsigned short int NumEntries){
	int result;
	struct PS2IDBMainboardEntry *MoboDatabase;

	if((MoboDatabase=malloc(NumEntries*sizeof(struct PS2IDBMainboardEntry)))!=NULL){
		if(fread(MoboDatabase, sizeof(struct PS2IDBMainboardEntry), NumEntries, DatabaseFile)==NumEntries){
			peripherals[PS2IDB_COMPONENT_MAINBOARD].models=MoboDatabase;
			peripherals[PS2IDB_COMPONENT_MAINBOARD].NumModels=NumEntries;

			result=0;
		}else{
			free(MoboDatabase);
			result=EIO;
		}
	}
	else result = ENOMEM;

	return result;
}

static int PS2IDBMS_LoadGenericPeripheralDatabase(unsigned short int id, FILE *DatabaseFile, unsigned short int NumEntries){
	int result;
	struct PS2IDBComponentEntry *ComponentDatabase;

	if((ComponentDatabase=malloc(NumEntries*sizeof(struct PS2IDBComponentEntry)))!=NULL){
		if(fread(ComponentDatabase, sizeof(struct PS2IDBComponentEntry), NumEntries, DatabaseFile)==NumEntries){
			peripherals[id].models=ComponentDatabase;
			peripherals[id].NumModels=NumEntries;
			result=0;
		}else{
			free(ComponentDatabase);
			result=EIO;
		}
	}
	else result = ENOMEM;

	return result;
}

static int PS2IDBMS_LoadDatabaseNormal(struct PS2IDBHeader *header, FILE *DatabaseFile){
	int result, i;
	struct PS2IDBComponentTable component;
	unsigned int *OffsetTable;

	if((OffsetTable=malloc(header->components*sizeof(unsigned int)))!=NULL){
		if(fread(OffsetTable, sizeof(unsigned int), header->components, DatabaseFile)==header->components){
			for(i=0; i<header->components; i++){
				fseek(DatabaseFile, OffsetTable[i], SEEK_SET);

				if(fread(&component, sizeof(struct PS2IDBComponentTable), 1, DatabaseFile)==1){
					if(component.id<PS2IDB_COMPONENT_COUNT && component.entries>0){
						switch(component.id){
							case PS2IDB_COMPONENT_MAINBOARD:
								result=PS2IDBMS_LoadMoboDatabase(DatabaseFile, component.entries);
								break;
							default:
								result=PS2IDBMS_LoadGenericPeripheralDatabase(component.id, DatabaseFile, component.entries);
								break;
						}
					}
					else result=0;	//Ignore unsupported entries.
				}
				else result = EIO;

				if(result!=0) break;
			}
		}else result = EIO;

		free(OffsetTable);
	}
	else result = ENOMEM;

	return result;
}

int PS2IDBMS_LoadDatabase(const char *path){
	int result;
	FILE *DatabaseFile;
	struct PS2IDBHeader header;

	PS2IDBMS_UnloadDatabase();

	if((DatabaseFile=fopen(path, "rb"))!=NULL){
		if(fread(&header, sizeof(header), 1, DatabaseFile)==1 && (strncmp(header.magic, "P2DB", 4)==0)){
			switch(header.version)
			{
				case PS2IDB_FORMAT_VERSION:
					result = PS2IDBMS_LoadDatabaseNormal(&header, DatabaseFile);
					break;
				default:
					printf("Unsupported database version!\n");
					result = -EINVAL;
			}
		}
		else result = EINVAL;

		fclose(DatabaseFile);
	}
	else result = ENOENT;

	if(result!=0) PS2IDBMS_UnloadDatabase();

	return result;
}

void PS2IDBMS_UnloadDatabase(void){
	int i;

	for(i=0; i<PS2IDB_COMPONENT_COUNT; i++){
		if(peripherals[i].models!=NULL){
			free(peripherals[i].models);
			peripherals[i].models=NULL;
			peripherals[i].NumModels=0;
		}
	}
}

int PS2IDBMS_SaveDatabase(const char *path){
	struct PS2IDBHeader header;
	struct PS2IDBComponentTable ComponentHeader;
	int result, i;
	FILE *DatabaseFile;
	unsigned int offsets[PS2IDB_COMPONENT_COUNT], offset, NumEntries;

	//If the database is empty, simply delete it.
	for(i=0,NumEntries=0; i<PS2IDB_COMPONENT_COUNT; i++) NumEntries+=peripherals[i].NumModels;

	if(NumEntries>0){
		strncpy(header.magic, "P2DB", sizeof(header.magic));
		header.version=PS2IDB_FORMAT_VERSION;
		header.components=PS2IDB_COMPONENT_COUNT;

		//Calculate the file offsets.
		for(i=0,offset=sizeof(struct PS2IDBHeader)+sizeof(unsigned int)*PS2IDB_COMPONENT_COUNT; i<PS2IDB_COMPONENT_COUNT; i++){
			offsets[i]=offset;
			switch(i){
				case PS2IDB_COMPONENT_MAINBOARD:
					offset+=sizeof(struct PS2IDBComponentTable)+peripherals[i].NumModels*sizeof(struct PS2IDBMainboardEntry);
					break;
				default:
					offset+=sizeof(struct PS2IDBComponentTable)+peripherals[i].NumModels*sizeof(struct PS2IDBComponentEntry);
			}
		}

		if((DatabaseFile=fopen(path, "wb"))!=NULL){
			if(fwrite(&header, sizeof(header), 1, DatabaseFile)==1 && fwrite(offsets, sizeof(unsigned int), PS2IDB_COMPONENT_COUNT, DatabaseFile)==PS2IDB_COMPONENT_COUNT){
				result=0;

				for(i=0; i<PS2IDB_COMPONENT_COUNT; i++){
					ComponentHeader.entries=peripherals[i].NumModels;
					ComponentHeader.id=i;

					if(fwrite(&ComponentHeader, sizeof(ComponentHeader), 1, DatabaseFile)==1){
						if(peripherals[i].NumModels>0){
							switch(i){
								case PS2IDB_COMPONENT_MAINBOARD:
									result=fwrite(peripherals[i].models, sizeof(struct PS2IDBMainboardEntry), peripherals[i].NumModels, DatabaseFile);
									break;
								default:
									result=fwrite(peripherals[i].models, sizeof(struct PS2IDBComponentEntry), peripherals[i].NumModels, DatabaseFile);
							}

							result=result==peripherals[i].NumModels?0:EIO;
							if(result!=0) break;
						}
					}else{
						result=EIO;
						break;
					}
				}
			}
			else result=EIO;

			fclose(DatabaseFile);
			if(result!=0) remove(path);
		}
		else result=EIO;
	}else{
		remove(path);
		result=0;
	}

	return result;
}

int PS2IDBMS_AddModel(int id, const struct PS2IDBComponentEntry *entry){
	int result;
	unsigned int i, NumModels;
	struct PS2IDBComponentEntry *database;

	result=0;
	database=peripherals[id].models;
	NumModels=peripherals[id].NumModels;

	//Check to see if this model has already been added.
	if(database!=NULL){
		for(i=0; i<NumModels; i++){
			if(!memcmp(&database[i], entry, sizeof(struct PS2IDBComponentEntry))){
				result=EEXIST;
				break;
			}
		}
	}

	if(result==0){
		if((database=realloc(database, (NumModels+1)*sizeof(struct PS2IDBComponentEntry)))!=NULL){
			memcpy(&database[NumModels], entry, sizeof(struct PS2IDBComponentEntry));
			peripherals[id].NumModels++;
			result=0;
		}
		else{
			peripherals[id].NumModels=0;
			result=ENOMEM;
		}

		peripherals[id].models=database;
	}

	return result;
}

int PS2IDBMS_AddMainboardModel(const struct PS2IDBMainboardEntry *entry){
	int result;
	unsigned int NumModels;
	struct PS2IDBMainboardEntry *database;

	result=0;
	database=peripherals[PS2IDB_COMPONENT_MAINBOARD].models;
	NumModels=peripherals[PS2IDB_COMPONENT_MAINBOARD].NumModels;

	//Check to see if this model has already been added.
	if(database!=NULL){
		if(PS2IDBMS_LookupMainboardModel(entry) != NULL){
				result=EEXIST;
		}
	}

	if(result==0){
		if((database=realloc(database, (NumModels+1)*sizeof(struct PS2IDBMainboardEntry)))!=NULL){
			memcpy(&database[NumModels], entry, sizeof(struct PS2IDBMainboardEntry));
			peripherals[PS2IDB_COMPONENT_MAINBOARD].NumModels++;
			result=0;
		}
		else{
			peripherals[PS2IDB_COMPONENT_MAINBOARD].NumModels=0;
			result=ENOMEM;
		}

		peripherals[PS2IDB_COMPONENT_MAINBOARD].models=database;
	}

	return result;
}

unsigned int PS2IDBMS_GetNumDatabaseRecords(int id){
	return peripherals[id].NumModels;
}

const struct PS2IDBComponentEntry *PS2IDBMS_GetDatabaseRecord(int id, unsigned int index){
	switch(id){
		case PS2IDB_COMPONENT_MAINBOARD:
			return NULL;
		default:
			return(index<peripherals[id].NumModels?&((struct PS2IDBComponentEntry*)peripherals[id].models)[index]:NULL);
	}
}

const struct PS2IDBMainboardEntry *PS2IDBMS_GetMainboardDatabaseRecord(unsigned int index){
	return(index<peripherals[PS2IDB_COMPONENT_MAINBOARD].NumModels?&((struct PS2IDBMainboardEntry*)peripherals[PS2IDB_COMPONENT_MAINBOARD].models)[index]:NULL);
}

int PS2IDBMS_UpdateModel(int id, unsigned int index, const struct PS2IDBComponentEntry *entry){
	int result;

	if(index<peripherals[id].NumModels){
		memcpy(&((struct PS2IDBComponentEntry*)peripherals[id].models)[index], entry, sizeof(struct PS2IDBComponentEntry));
		result=0;
	}
	else result=ENOENT;

	return result;
}

int PS2IDBMS_UpdateMainboardModel(unsigned int index, const struct PS2IDBMainboardEntry *entry){
	int result;

	if(index<peripherals[PS2IDB_COMPONENT_MAINBOARD].NumModels){
		memcpy(&((struct PS2IDBMainboardEntry*)peripherals[PS2IDB_COMPONENT_MAINBOARD].models)[index], entry, sizeof(struct PS2IDBMainboardEntry));
		result=0;
	}
	else result=ENOENT;

	return result;
}


int PS2IDBMS_DeleteRecord(int id, unsigned int index){
	int result;
	unsigned int NumModels;
	struct PS2IDBComponentEntry *entry;

	NumModels=peripherals[id].NumModels;
	if(index<NumModels){
		if(NumModels>1){
			entry=&((struct PS2IDBComponentEntry*)peripherals[id].models)[index];

			memmove(entry, entry+1, (NumModels-(index+1))*sizeof(struct PS2IDBComponentEntry));
			if((peripherals[id].models=realloc(peripherals[id].models, (NumModels-1)*sizeof(struct PS2IDBComponentEntry)))!=NULL){
				peripherals[id].NumModels--;
				result=0;
			}else{
				peripherals[id].NumModels=0;
				peripherals[id].models=NULL;
				result=ENOMEM;
			}
		}else{
			//Otherwise, just empty the database.
			free(peripherals[id].models);
			peripherals[id].NumModels=0;
			peripherals[id].models=NULL;
			result=0;
		}
	}
	else result=ENOENT;

	return result;
}

int PS2IDBMS_DeleteMainboardRecord(unsigned int index){
	int result;
	unsigned int NumModels;
	struct PS2IDBMainboardEntry *entry;

	NumModels=peripherals[PS2IDB_COMPONENT_MAINBOARD].NumModels;
	if(index<NumModels){
		if(NumModels>1){
			entry=&((struct PS2IDBMainboardEntry*)peripherals[PS2IDB_COMPONENT_MAINBOARD].models)[index];

			memmove(entry, entry+1, (NumModels-(index+1))*sizeof(struct PS2IDBMainboardEntry));
			if((peripherals[PS2IDB_COMPONENT_MAINBOARD].models=realloc(peripherals[PS2IDB_COMPONENT_MAINBOARD].models, (NumModels-1)*sizeof(struct PS2IDBMainboardEntry)))!=NULL){
				peripherals[PS2IDB_COMPONENT_MAINBOARD].NumModels--;
				result=0;
			}else{
				peripherals[PS2IDB_COMPONENT_MAINBOARD].NumModels=0;
				peripherals[PS2IDB_COMPONENT_MAINBOARD].models=NULL;
				result=ENOMEM;
			}
		}else{
			//Otherwise, just empty the database.
			free(peripherals[PS2IDB_COMPONENT_MAINBOARD].models);
			peripherals[PS2IDB_COMPONENT_MAINBOARD].NumModels=0;
			peripherals[PS2IDB_COMPONENT_MAINBOARD].models=NULL;
			result=0;
		}
	}
	else result=ENOENT;

	return result;
}

const char *PS2IDBMS_LookupComponentModel(unsigned short int component, unsigned int id){
	unsigned int i;
	const char *result;
	struct PS2IDBComponentEntry *entry;

	for(i=0,result=NULL,entry=peripherals[component].models; i<peripherals[component].NumModels; i++,entry++){
		if(entry->revision==id){
			result=entry->name;
			break;
		}
	}

	return result;
}

const struct PS2IDBMainboardEntry *PS2IDBMS_LookupMainboardModel(const struct PS2IDBMainboardEntry *model){
	unsigned int i;
	const struct PS2IDBMainboardEntry *entry, *result;

	for(i = 0,result = NULL,entry = peripherals[PS2IDB_COMPONENT_MAINBOARD].models; i < peripherals[PS2IDB_COMPONENT_MAINBOARD].NumModels; i++,entry++){
		/*	A console's mainboard can be determined based on the following:
				BOOT ROM CRC
				DVD ROM CRC
				ROMVER string
				Model name
				EE revision
				FPU revision
				IOP revision
				GS revision
				MECHACON revision and region
				SPU2 revision
				SSBUSC revision
				Model ID
				Console model ID
				EMCS ID
				M Renewal Date
				ADD0x010	*/

		if( model->BOOT_ROM.IsExists == entry->BOOT_ROM.IsExists &&
			model->BOOT_ROM.crc16 == entry->BOOT_ROM.crc16 &&
			model->DVD_ROM.IsExists == entry->DVD_ROM.IsExists &&
			model->DVD_ROM.crc16 == entry->DVD_ROM.crc16 &&
			strcmp(model->romver, entry->romver)==0	&&
			strcmp(model->ModelName, entry->ModelName)==0 &&
			(model->ee.implementation == entry->ee.implementation) &&
			(model->ee.revision == entry->ee.revision) &&
			(model->ee.FPUImplementation == entry->ee.FPUImplementation) &&
			(model->ee.FPURevision == entry->ee.FPURevision) &&
			(model->iop.revision == entry->iop.revision) &&
			(model->gs.revision == entry->gs.revision) &&
			(model->gs.id == entry->gs.id) &&
			(memcmp(model->MECHACONVersion, entry->MECHACONVersion, sizeof(entry->MECHACONVersion)) == 0) &&
			(model->spu2.revision == entry->spu2.revision) &&
			(model->ssbus.revision == entry->ssbus.revision) &&
			(memcmp(model->ModelID, entry->ModelID, sizeof(model->ModelID)) == 0) &&
			(memcmp(model->ConModelID, entry->ConModelID, sizeof(model->ConModelID))== 0) &&
			(model->EMCSID == entry->EMCSID) &&
			(model->ADD010 == entry->ADD010) &&
			(memcmp(model->MRenewalDate, entry->MRenewalDate, sizeof(model->MRenewalDate)) == 0))
		{
			result = entry;
			break;
		}
	}

	return result;
}

const struct PS2IDBMainboardEntry *PS2IDBMS_LookupMatchingROM(const struct PS2IDBMainboardEntry *model){
	unsigned int i;
	const struct PS2IDBMainboardEntry *entry, *result;

	for(i = 0,result = NULL,entry = peripherals[PS2IDB_COMPONENT_MAINBOARD].models; i < peripherals[PS2IDB_COMPONENT_MAINBOARD].NumModels; i++,entry++){
		/*	Attempt to locate a matching entry, based on:
				ROMVER string
				ROMGEN

			Ignore entries that have the PS2IDB_STAT_ERR_MODDED bit set.
		*/

		if( model->BOOT_ROM.IsExists == entry->BOOT_ROM.IsExists &&
			model->DVD_ROM.IsExists == entry->DVD_ROM.IsExists &&
			strcmp(model->romver, entry->romver)==0	&&
			(memcmp(model->MRenewalDate, entry->MRenewalDate, sizeof(model->MRenewalDate)) == 0) &&
			!(entry->status & PS2IDB_STAT_ERR_MODDED) &&
			model->ROMGEN_MonthDate == entry->ROMGEN_MonthDate &&
			model->ROMGEN_Year == entry->ROMGEN_Year
		)
		{
			result = entry;
			break;
		}
	}

	return result;
}
