
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include "../../cfg/config.h"
#include "../../common/misc/cfg.h"
#include "../../common/misc/coding.h"
#include "../../common/misc/misc.h"

unsigned char 	line[1000];
int		outputnum;
FILE		*output;
CFG_Header	*cfg_info;

void WriteOutput(char *mystring)
{
	unsigned char buffer[400];

	EncodeUnicode(buffer,mystring,strlen(mystring));
	fwrite(buffer,1,strlen(mystring)*2,output);
	buffer[0] = 0x00;
	buffer[1] = 0x0d;
	buffer[2] = 0x00;
	buffer[3] = 0x0a;
	fwrite(buffer,1,4,output);	
}

void ProcessSourceFile(char *filename)
{
	int 		start,stop,num,i;
	char 		*pdest;
	FILE		*file;
	unsigned char 	buffer[10000],buffer2[500];
	bool		found;
	
	printf("Processing source file %s\n",filename);

	file = fopen(filename, "rb");
	line[0]=0;
	do {
		num=fread(buffer, 1, 10000, file);
		for (i=0;i<num;i++) {
			switch (buffer[i]) {
				case 10:
					pdest = strstr( line, "printmsgerr" );
					if (pdest != NULL) {
						start = strlen(line) - strlen(pdest) + 1 + 11;
					} else {
						pdest = strstr( line, "printmsg" );
						if (pdest != NULL) start = strlen(line) - strlen(pdest) + 1 + 8;
					}
					if (pdest != NULL) {
						found = false;
						stop  = start + 2;
						while (stop < ((int)strlen(line))) {
							stop++;
							if (line[stop-1] != '\\' && line[stop] == '"') {
								found = true;
								break;
							}
						}
						if (found) {
							sprintf(buffer2,"F%04i=%s",outputnum,line+start);
							buffer2[stop-start+6+1] = 0;
							WriteOutput(buffer2);
							outputnum++;
						}
					}
					line[0]=0;
					break;
				case 13:
					break;
				default:
					line[strlen(line)+1]=0;
					line[strlen(line)]=buffer[i];
			}
		}
	} while (num==10000);
	fclose(file);
}

void RemoveDuplicatedStrings(char *filename)
{
	int 		num2,num,number;
	unsigned char 	buffer[500],buffer2[500],buff2[50];
	CFG_Entry	*e,*ebackup,*e2;
	bool		duplicated;

	printf("Removing duplicated strings from %s\n",filename);

	EncodeUnicode (buff2, "common", 6);

	cfg_info = CFG_ReadFile(filename,true);
	output = fopen(filename, "wb");

	outputnum=0;

	buffer[0] = 0xfe;
	buffer[1] = 0xff;
	fwrite(buffer,1,2,output);
	sprintf(buffer,"# Localization file for Gammu (www.mwiacek.com) version %s",VERSION);
	WriteOutput(buffer);
	WriteOutput("");
	WriteOutput("[common]");

	e = CFG_FindLastSectionEntry(cfg_info, buff2,true);
	ebackup = e;
	while (1) {
		if (e == NULL) break;
		num = -1;
		sprintf(buffer,"%s",DecodeUnicodeString(e->key));
		if (strlen(buffer) == 5 && strncmp("F", buffer, 1) == 0) {
			num = atoi(buffer+2);
		}
		if (num!=-1) {
			duplicated	= false;
			e2		= ebackup;
			number		= 0;
			while (1) {
				if (e2 == NULL) break;
				num2 = -1;
				sprintf(buffer,"%s",DecodeUnicodeString(e2->key));
				if (strlen(buffer) == 5 && strncmp("F", buffer, 1) == 0) {
					num2 = atoi(buffer+2);
					number++;
				}
				if (num2 != -1 && num2 < num) {
	                                if (mywstrncmp(e2->value, e->value, 0)) {
						duplicated = true;
						break;
	                                }
				}
				e2 = e2->prev;
				if (number == num) break;
			}
			if (!duplicated) {
				sprintf(buffer2,"F%04i=",outputnum);
				EncodeUnicode(buffer,buffer2,6);
				fwrite(buffer,1,12,output);
				fwrite(e->value,1,strlen(DecodeUnicodeString(e->value))*2,output);
				WriteOutput("");
				outputnum++;
			}
		}
		e = e->prev;
		printf("*");
	}
	fclose(output);
	printf("\n");
}

void ProcessLangFile(char *filename)
{
	CFG_Header	*cfg_lang;
	CFG_Entry	*e,*e_lang,*e2;
	int		num,num_lang;
	char		*retval_lang;
	unsigned char 	buffer[10000],buffer2[10000],buff2[50];

	printf("Processing file %s\n",filename);

	cfg_lang = CFG_ReadFile(filename,true);
	output = fopen(filename, "wb");

	buffer[0] = 0xfe;
	buffer[1] = 0xff;
	fwrite(buffer,1,2,output);
	sprintf(buffer,"# Localization file for Gammu (www.mwiacek.com) version %s",VERSION);
	WriteOutput(buffer);
	WriteOutput("");
	WriteOutput("[common]");

	EncodeUnicode (buff2, "common", 6);

	e2	= CFG_FindLastSectionEntry(cfg_lang, buff2, true);
	e	= CFG_FindLastSectionEntry(cfg_info, buff2, true);
	while (1) {
		if (e == NULL) break;
		num = -1;
		sprintf(buffer,"%s",DecodeUnicodeString(e->key));
		if (strlen(buffer) == 5 && strncmp("F", buffer, 1) == 0) {
			num = atoi(buffer+2);
		}
		printf("*");
		if (num!=-1) {
			sprintf(buffer2,"F%04i=",num);
			EncodeUnicode(buffer,buffer2,6);
			fwrite(buffer,1,12,output);
			fwrite(e->value,1,strlen(DecodeUnicodeString(e->value))*2,output);
			WriteOutput("");
			e_lang = e2;
			while (1) {
				if (e_lang == NULL) break;
				num_lang = -1;
				sprintf(buffer,"%s",DecodeUnicodeString(e_lang->key));
				if (strlen(buffer) == 5 && strncmp("F", buffer, 1) == 0) {
					num_lang = atoi(buffer+2);
				}
				if (num_lang!=-1) {
	                                if (mywstrncmp(e_lang->value, e->value, 0)) {
						sprintf(buffer2,"T%04i",num_lang);
						EncodeUnicode(buffer,buffer2,6);
					        retval_lang = CFG_Get(cfg_lang, buff2, buffer,true);
						if (retval_lang != NULL) {
							sprintf(buffer2,"T%04i=",num);
							EncodeUnicode(buffer,buffer2,6);
							fwrite(buffer,1,12,output);
							fwrite(retval_lang,1,strlen(DecodeUnicodeString(retval_lang))*2,output);
							WriteOutput("");
						}
						break;
					}
				}
				e_lang = e_lang->prev;
			}
		}
		e = e->prev;
	}
	fclose(output);
}

int main(int argc, char *argv[])
{
	unsigned char buffer[400];

#ifdef WIN32
	output = fopen("../../../../docs/docs/locale/gammu_us.txt", "wb");
#else
	output = fopen("../../docs/docs/locale/gammu_us.txt", "wb");
#endif
	buffer[0] = 0xfe;
	buffer[1] = 0xff;
	fwrite(buffer,1,2,output);
	sprintf(buffer,"# Localization file for Gammu (www.mwiacek.com) version %s",VERSION);
	WriteOutput(buffer);
	WriteOutput("");
	WriteOutput("[common]");
	WriteOutput("F0001=\"No error.\"");
	WriteOutput("F0002=\"Error opening device. Unknown or busy device.\"");
	WriteOutput("F0003=\"Error setting device DTR or RTS\"");
	WriteOutput("F0004=\"Error setting device speed. Maybe speed not supported.\"");
	WriteOutput("F0005=\"Error writing device.\"");
	WriteOutput("F0006=\"No response in specified timeout. Probably phone not connected.\"");
	WriteOutput("F0007=\"Unknown response from phone. See /readme.txt, how to report it.\"");
	WriteOutput("F0008=\"Unknown connection type string. Check config file.\"");
	WriteOutput("F0009=\"Unknown model type string. Check config file.\"");
	WriteOutput("F0010=\"Some required functions not compiled for your OS. Please contact.\"");
	WriteOutput("F0011=\"Function not supported by phone.\"");
	WriteOutput("F0012=\"Entry is empty.\"");
	WriteOutput("F0013=\"Unknown error.\"");
	WriteOutput("F0014=\"Unknown error.\"");
	WriteOutput("F0015=\"Security error. Maybe no PIN ?\"");
	WriteOutput("F0016=\"Invalid location. Maybe too high ?\"");
	WriteOutput("F0017=\"Function not implemented. Help required.\"");
	WriteOutput("F0018=\"Can't open specified file. Read only ?\"");
	WriteOutput("F0019=\"Memory full.\"");
	WriteOutput("F0020=\"More memory required...\"");
	WriteOutput("F0021=\"Permission to file/device required...\"");
	WriteOutput("F0022=\"Family\"");
	WriteOutput("F0023=\"VIP\"");
	WriteOutput("F0024=\"Friends\"");
	WriteOutput("F0025=\"Colleagues\"");
	WriteOutput("F0026=\"Other\"");
	WriteOutput("F0027=\"Inbox\"");
	WriteOutput("F0028=\"Outbox\"");
	WriteOutput("F0029=\"Empty SMSC number. Set in phone or use -smscnumber\"");
	WriteOutput("F0030=\"You're inside phone menu (during editing ?). Leave it and try again.\"");
	WriteOutput("F0031=\"Personal\"");
	WriteOutput("F0032=\"Car\"");
	WriteOutput("F0033=\"Headset\"");
	WriteOutput("F0034=\"General\"");
	WriteOutput("F0035=\"Silent\"");
	WriteOutput("F0036=\"Meeting\"");
	WriteOutput("F0037=\"Outdoor\"");
	WriteOutput("F0038=\"Pager\"");
	WriteOutput("F0039=\"Discreet\"");
	WriteOutput("F0040=\"Loud\"");
	WriteOutput("F0041=\"My style\"");
	WriteOutput("F0042=\"Function is during writing. If want help, please contact with authors.\"");
	WriteOutput("F0043=\"Error during reading device\"");                               
	WriteOutput("F0044=\"Can't set parity on device\"");
	WriteOutput("F0045=\"Phone is disabled and connected to charger\"");
	WriteOutput("F0046=\"File format not supported by Gammu\"");
	outputnum=47;
#ifdef WIN32
	ProcessSourceFile("../../../../gammu/gammu.c");
#else
	ProcessSourceFile("../../gammu/gammu.c");
#endif
	fclose(output);

#ifdef WIN32
	RemoveDuplicatedStrings("../../../../docs/docs/locale/gammu_us.txt");
	cfg_info = CFG_ReadFile("../../../../docs/docs/locale/gammu_us.txt",true);

	ProcessLangFile("../../../../docs/docs/locale/gammu_pl.txt");
#else
	RemoveDuplicatedStrings("../../docs/docs/locale/gammu_us.txt");
	cfg_info = CFG_ReadFile("../../docs/docs/locale/gammu_us.txt",true);

	ProcessLangFile("../../docs/docs/locale/gammu_pl.txt");
#endif
	printf("\n");
	
	return (0);
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
