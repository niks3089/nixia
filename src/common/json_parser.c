/*
*Copyright(C)2015NikhilAP
*
*Thisprogramisfreesoftware:youcanredistributeitand/ormodify
*itunderthetermsoftheGNUGeneralPublicLicenseaspublishedby
*theFreeSoftwareFoundation,eitherversion3oftheLicense,or
*(atyouroption)anylaterversion.
*
*Thisprogramisdistributedinthehopethatitwillbeuseful,
*butWITHOUTANYWARRANTY;withouteventheimpliedwarrantyof
*MERCHANTABILITYorFITNESSFORAPARTICULARPURPOSE.Seethe
*GNUGeneralPublicLicenseformoredetails.
*
*YoushouldhavereceivedacopyoftheGNUGeneralPublicLicense
*alongwiththisprogram.Ifnot,see<http://www.gnu.org/licenses/>.
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include"common.h"
#include"json_parser.h"

char*
json_parser_parse_file(char*file)
{
FILE*f;longlen;char*data;

f=fopen(file,"rb");
fseek(f,0,SEEK_END);
len=ftell(f);
fseek(f,0,SEEK_SET);
data=(char*)malloc(len+1);
if(!data){
returnNULL;
}
fread(data,1,len,f);
fclose(f);
returndata;
}

void
json_parser_get_element_str(cJSON*root,char*table_name,char*element,char**val,
char*val_default)
{
cJSON*child=NULL,*json=NULL;

child=cJSON_GetObjectItem(root,table_name);
json=cJSON_GetObjectItem(child,element);

if(json&&strlen(json->valuestring)){
*val=strdup(json->valuestring);
}elseif(val_default){
*val=strdup(val_default);
}
}

void
json_parser_get_element_double(cJSON*root,char*table_name,char*element,double*val,
doubleval_default)
{
cJSON*child=NULL,*json=NULL;

child=cJSON_GetObjectItem(root,table_name);
json=cJSON_GetObjectItem(child,element);

if(json){
*val=json->valuedouble;
}else{
*val=val_default;
}
}

void
json_parser_get_array_str(cJSON*root,char*table_name,
char*array_name,char**val)
{
cJSON*child=NULL;
inti=0;
chartmp[MAX_MSG_LENGTH]={0};
char*array_item;

child=cJSON_GetObjectItem(root,table_name);
cJSON*array=cJSON_GetObjectItem(child,array_name);

for(i=0;i<cJSON_GetArraySize(array);i++){
array_item=cJSON_GetArrayItem(array,i)->valuestring;
strcat(tmp,array_item);
}
*val=strdup(tmp);
}
