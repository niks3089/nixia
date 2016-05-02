/*
Copyright(c)2009DaveGamble

Permissionisherebygranted,freeofcharge,toanypersonobtainingacopy
ofthissoftwareandassociateddocumentationfiles(the"Software"),todeal
intheSoftwarewithoutrestriction,includingwithoutlimitationtherights
touse,copy,modify,merge,publish,distribute,sublicense,and/orsell
copiesoftheSoftware,andtopermitpersonstowhomtheSoftwareis
furnishedtodoso,subjecttothefollowingconditions:

Theabovecopyrightnoticeandthispermissionnoticeshallbeincludedin
allcopiesorsubstantialportionsoftheSoftware.

THESOFTWAREISPROVIDED"ASIS",WITHOUTWARRANTYOFANYKIND,EXPRESSOR
IMPLIED,INCLUDINGBUTNOTLIMITEDTOTHEWARRANTIESOFMERCHANTABILITY,
FITNESSFORAPARTICULARPURPOSEANDNONINFRINGEMENT.INNOEVENTSHALLTHE
AUTHORSORCOPYRIGHTHOLDERSBELIABLEFORANYCLAIM,DAMAGESOROTHER
LIABILITY,WHETHERINANACTIONOFCONTRACT,TORTOROTHERWISE,ARISINGFROM,
OUTOFORINCONNECTIONWITHTHESOFTWAREORTHEUSEOROTHERDEALINGSIN
THESOFTWARE.
*/

/*cJSON*/
/*JSONparserinC.*/

#include<string.h>
#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<float.h>
#include<limits.h>
#include<ctype.h>
#include"cJSON.h"

staticconstchar*ep;

constchar*cJSON_GetErrorPtr(void){returnep;}

staticintcJSON_strcasecmp(constchar*s1,constchar*s2)
{
if(!s1)return(s1==s2)?0:1;if(!s2)return1;
for(;tolower(*s1)==tolower(*s2);++s1,++s2)if(*s1==0)return0;
returntolower(*(constunsignedchar*)s1)-tolower(*(constunsignedchar*)s2);
}

staticvoid*(*cJSON_malloc)(size_tsz)=malloc;
staticvoid(*cJSON_free)(void*ptr)=free;

staticchar*cJSON_strdup(constchar*str)
{
size_tlen;
char*copy;

len=strlen(str)+1;
if(!(copy=(char*)cJSON_malloc(len)))return0;
memcpy(copy,str,len);
returncopy;
}

voidcJSON_InitHooks(cJSON_Hooks*hooks)
{
if(!hooks){/*Resethooks*/
cJSON_malloc=malloc;
cJSON_free=free;
return;
}

cJSON_malloc=(hooks->malloc_fn)?hooks->malloc_fn:malloc;
cJSON_free=(hooks->free_fn)?hooks->free_fn:free;
}

/*Internalconstructor.*/
staticcJSON*cJSON_New_Item(void)
{
cJSON*node=(cJSON*)cJSON_malloc(sizeof(cJSON));
if(node)memset(node,0,sizeof(cJSON));
returnnode;
}

/*DeleteacJSONstructure.*/
voidcJSON_Delete(cJSON*c)
{
cJSON*next;
while(c)
{
next=c->next;
if(!(c->type&cJSON_IsReference)&&c->child)cJSON_Delete(c->child);
if(!(c->type&cJSON_IsReference)&&c->valuestring)cJSON_free(c->valuestring);
if(c->string)cJSON_free(c->string);
cJSON_free(c);
c=next;
}
}

/*Parsetheinputtexttogenerateanumber,andpopulatetheresultintoitem.*/
staticconstchar*parse_number(cJSON*item,constchar*num)
{
doublen=0,sign=1,scale=0;intsubscale=0,signsubscale=1;

if(*num=='-')sign=-1,num++;/*Hassign?*/
if(*num=='0')num++;/*iszero*/
if(*num>='1'&&*num<='9')don=(n*10.0)+(*num++-'0');while(*num>='0'&&*num<='9');/*Number?*/
if(*num=='.'&&num[1]>='0'&&num[1]<='9'){num++;don=(n*10.0)+(*num++-'0'),scale--;while(*num>='0'&&*num<='9');}/*Fractionalpart?*/
if(*num=='e'||*num=='E')/*Exponent?*/
{num++;if(*num=='+')num++;elseif(*num=='-')signsubscale=-1,num++;/*Withsign?*/
while(*num>='0'&&*num<='9')subscale=(subscale*10)+(*num++-'0');/*Number?*/
}

n=sign*n*pow(10.0,(scale+subscale*signsubscale));/*number=+/-number.fraction*10^+/-exponent*/

item->valuedouble=n;
item->valueint=(int)n;
item->type=cJSON_Number;
returnnum;
}

/*Renderthenumbernicelyfromthegivenitemintoastring.*/
staticchar*print_number(cJSON*item)
{
char*str;
doubled=item->valuedouble;
if(fabs(((double)item->valueint)-d)<=DBL_EPSILON&&d<=INT_MAX&&d>=INT_MIN)
{
str=(char*)cJSON_malloc(21);/*2^64+1canberepresentedin21chars.*/
if(str)sprintf(str,"%d",item->valueint);
}
else
{
str=(char*)cJSON_malloc(64);/*Thisisanicetradeoff.*/
if(str)
{
if(fabs(floor(d)-d)<=DBL_EPSILON&&fabs(d)<1.0e60)sprintf(str,"%.0f",d);
elseif(fabs(d)<1.0e-6||fabs(d)>1.0e9)sprintf(str,"%e",d);
elsesprintf(str,"%f",d);
}
}
returnstr;
}

staticunsignedparse_hex4(constchar*str)
{
unsignedh=0;
if(*str>='0'&&*str<='9')h+=(*str)-'0';elseif(*str>='A'&&*str<='F')h+=10+(*str)-'A';elseif(*str>='a'&&*str<='f')h+=10+(*str)-'a';elsereturn0;
h=h<<4;str++;
if(*str>='0'&&*str<='9')h+=(*str)-'0';elseif(*str>='A'&&*str<='F')h+=10+(*str)-'A';elseif(*str>='a'&&*str<='f')h+=10+(*str)-'a';elsereturn0;
h=h<<4;str++;
if(*str>='0'&&*str<='9')h+=(*str)-'0';elseif(*str>='A'&&*str<='F')h+=10+(*str)-'A';elseif(*str>='a'&&*str<='f')h+=10+(*str)-'a';elsereturn0;
h=h<<4;str++;
if(*str>='0'&&*str<='9')h+=(*str)-'0';elseif(*str>='A'&&*str<='F')h+=10+(*str)-'A';elseif(*str>='a'&&*str<='f')h+=10+(*str)-'a';elsereturn0;
returnh;
}

/*Parsetheinputtextintoanunescapedcstring,andpopulateitem.*/
staticconstunsignedcharfirstByteMark[7]={0x00,0x00,0xC0,0xE0,0xF0,0xF8,0xFC};
staticconstchar*parse_string(cJSON*item,constchar*str)
{
constchar*ptr=str+1;char*ptr2;char*out;intlen=0;unsigneduc,uc2;
if(*str!='\"'){ep=str;return0;}/*notastring!*/

while(*ptr!='\"'&&*ptr&&++len)if(*ptr++=='\\')ptr++;/*Skipescapedquotes.*/

out=(char*)cJSON_malloc(len+1);/*Thisishowlongweneedforthestring,roughly.*/
if(!out)return0;

ptr=str+1;ptr2=out;
while(*ptr!='\"'&&*ptr)
{
if(*ptr!='\\')*ptr2++=*ptr++;
else
{
ptr++;
switch(*ptr)
{
case'b':*ptr2++='\b';break;
case'f':*ptr2++='\f';break;
case'n':*ptr2++='\n';break;
case'r':*ptr2++='\r';break;
case't':*ptr2++='\t';break;
case'u':/*transcodeutf16toutf8.*/
uc=parse_hex4(ptr+1);ptr+=4;/*gettheunicodechar.*/

if((uc>=0xDC00&&uc<=0xDFFF)||uc==0)break;/*checkforinvalid.*/

if(uc>=0xD800&&uc<=0xDBFF)/*UTF16surrogatepairs.*/
{
if(ptr[1]!='\\'||ptr[2]!='u')break;/*missingsecond-halfofsurrogate.*/
uc2=parse_hex4(ptr+3);ptr+=6;
if(uc2<0xDC00||uc2>0xDFFF)break;/*invalidsecond-halfofsurrogate.*/
uc=0x10000+(((uc&0x3FF)<<10)|(uc2&0x3FF));
}

len=4;if(uc<0x80)len=1;elseif(uc<0x800)len=2;elseif(uc<0x10000)len=3;ptr2+=len;

switch(len){
case4:*--ptr2=((uc|0x80)&0xBF);uc>>=6;
case3:*--ptr2=((uc|0x80)&0xBF);uc>>=6;
case2:*--ptr2=((uc|0x80)&0xBF);uc>>=6;
case1:*--ptr2=(uc|firstByteMark[len]);
}
ptr2+=len;
break;
default:*ptr2++=*ptr;break;
}
ptr++;
}
}
*ptr2=0;
if(*ptr=='\"')ptr++;
item->valuestring=out;
item->type=cJSON_String;
returnptr;
}

/*Renderthecstringprovidedtoanescapedversionthatcanbeprinted.*/
staticchar*print_string_ptr(constchar*str)
{
constchar*ptr;char*ptr2,*out;intlen=0;unsignedchartoken;

if(!str)returncJSON_strdup("");
ptr=str;while((token=*ptr)&&++len){if(strchr("\"\\\b\f\n\r\t",token))len++;elseif(token<32)len+=5;ptr++;}

out=(char*)cJSON_malloc(len+3);
if(!out)return0;

ptr2=out;ptr=str;
*ptr2++='\"';
while(*ptr)
{
if((unsignedchar)*ptr>31&&*ptr!='\"'&&*ptr!='\\')*ptr2++=*ptr++;
else
{
*ptr2++='\\';
switch(token=*ptr++)
{
case'\\':*ptr2++='\\';break;
case'\"':*ptr2++='\"';break;
case'\b':*ptr2++='b';break;
case'\f':*ptr2++='f';break;
case'\n':*ptr2++='n';break;
case'\r':*ptr2++='r';break;
case'\t':*ptr2++='t';break;
default:sprintf(ptr2,"u%04x",token);ptr2+=5;break;/*escapeandprint*/
}
}
}
*ptr2++='\"';*ptr2++=0;
returnout;
}
/*Invoteprint_string_ptr(whichisuseful)onanitem.*/
staticchar*print_string(cJSON*item){returnprint_string_ptr(item->valuestring);}

/*Predeclaretheseprototypes.*/
staticconstchar*parse_value(cJSON*item,constchar*value);
staticchar*print_value(cJSON*item,intdepth,intfmt);
staticconstchar*parse_array(cJSON*item,constchar*value);
staticchar*print_array(cJSON*item,intdepth,intfmt);
staticconstchar*parse_object(cJSON*item,constchar*value);
staticchar*print_object(cJSON*item,intdepth,intfmt);

/*Utilitytojumpwhitespaceandcr/lf*/
staticconstchar*skip(constchar*in){while(in&&*in&&(unsignedchar)*in<=32)in++;returnin;}

/*Parseanobject-createanewroot,andpopulate.*/
cJSON*cJSON_ParseWithOpts(constchar*value,constchar**return_parse_end,intrequire_null_terminated)
{
constchar*end=0;
cJSON*c=cJSON_New_Item();
ep=0;
if(!c)return0;/*memoryfail*/

end=parse_value(c,skip(value));
if(!end){cJSON_Delete(c);return0;}/*parsefailure.episset.*/

/*ifwerequirenull-terminatedJSONwithoutappendedgarbage,skipandthencheckforanullterminator*/
if(require_null_terminated){end=skip(end);if(*end){cJSON_Delete(c);ep=end;return0;}}
if(return_parse_end)*return_parse_end=end;
returnc;
}
/*DefaultoptionsforcJSON_Parse*/
cJSON*cJSON_Parse(constchar*value){returncJSON_ParseWithOpts(value,0,0);}

/*RenderacJSONitem/entity/structuretotext.*/
char*cJSON_Print(cJSON*item){returnprint_value(item,0,1);}
char*cJSON_PrintUnformatted(cJSON*item){returnprint_value(item,0,0);}

/*Parsercore-whenencounteringtext,processappropriately.*/
staticconstchar*parse_value(cJSON*item,constchar*value)
{
if(!value)return0;/*Failonnull.*/
if(!strncmp(value,"null",4)){item->type=cJSON_NULL;returnvalue+4;}
if(!strncmp(value,"false",5)){item->type=cJSON_False;returnvalue+5;}
if(!strncmp(value,"true",4)){item->type=cJSON_True;item->valueint=1;returnvalue+4;}
if(*value=='\"'){returnparse_string(item,value);}
if(*value=='-'||(*value>='0'&&*value<='9')){returnparse_number(item,value);}
if(*value=='['){returnparse_array(item,value);}
if(*value=='{'){returnparse_object(item,value);}

ep=value;return0;/*failure.*/
}

/*Renderavaluetotext.*/
staticchar*print_value(cJSON*item,intdepth,intfmt)
{
char*out=0;
if(!item)return0;
switch((item->type)&255)
{
casecJSON_NULL:out=cJSON_strdup("null");break;
casecJSON_False:out=cJSON_strdup("false");break;
casecJSON_True:out=cJSON_strdup("true");break;
casecJSON_Number:out=print_number(item);break;
casecJSON_String:out=print_string(item);break;
casecJSON_Array:out=print_array(item,depth,fmt);break;
casecJSON_Object:out=print_object(item,depth,fmt);break;
}
returnout;
}

/*Buildanarrayfrominputtext.*/
staticconstchar*parse_array(cJSON*item,constchar*value)
{
cJSON*child;
if(*value!='['){ep=value;return0;}/*notanarray!*/

item->type=cJSON_Array;
value=skip(value+1);
if(*value==']')returnvalue+1;/*emptyarray.*/

item->child=child=cJSON_New_Item();
if(!item->child)return0;/*memoryfail*/
value=skip(parse_value(child,skip(value)));/*skipanyspacing,getthevalue.*/
if(!value)return0;

while(*value==',')
{
cJSON*new_item;
if(!(new_item=cJSON_New_Item()))return0;/*memoryfail*/
child->next=new_item;new_item->prev=child;child=new_item;
value=skip(parse_value(child,skip(value+1)));
if(!value)return0;/*memoryfail*/
}

if(*value==']')returnvalue+1;/*endofarray*/
ep=value;return0;/*malformed.*/
}

/*Renderanarraytotext*/
staticchar*print_array(cJSON*item,intdepth,intfmt)
{
char**entries;
char*out=0,*ptr,*ret;intlen=5;
cJSON*child=item->child;
intnumentries=0,i=0,fail=0;

/*Howmanyentriesinthearray?*/
while(child)numentries++,child=child->next;
/*Explicitlyhandlenumentries==0*/
if(!numentries)
{
out=(char*)cJSON_malloc(3);
if(out)strcpy(out,"[]");
returnout;
}
/*Allocateanarraytoholdthevaluesforeach*/
entries=(char**)cJSON_malloc(numentries*sizeof(char*));
if(!entries)return0;
memset(entries,0,numentries*sizeof(char*));
/*Retrievealltheresults:*/
child=item->child;
while(child&&!fail)
{
ret=print_value(child,depth+1,fmt);
entries[i++]=ret;
if(ret)len+=strlen(ret)+2+(fmt?1:0);elsefail=1;
child=child->next;
}

/*Ifwedidn'tfail,trytomalloctheoutputstring*/
if(!fail)out=(char*)cJSON_malloc(len);
/*Ifthatfails,wefail.*/
if(!out)fail=1;

/*Handlefailure.*/
if(fail)
{
for(i=0;i<numentries;i++)if(entries[i])cJSON_free(entries[i]);
cJSON_free(entries);
return0;
}

/*Composetheoutputarray.*/
*out='[';
ptr=out+1;*ptr=0;
for(i=0;i<numentries;i++)
{
strcpy(ptr,entries[i]);ptr+=strlen(entries[i]);
if(i!=numentries-1){*ptr++=',';if(fmt)*ptr++='';*ptr=0;}
cJSON_free(entries[i]);
}
cJSON_free(entries);
*ptr++=']';*ptr++=0;
returnout;
}

/*Buildanobjectfromthetext.*/
staticconstchar*parse_object(cJSON*item,constchar*value)
{
cJSON*child;
if(*value!='{'){ep=value;return0;}/*notanobject!*/

item->type=cJSON_Object;
value=skip(value+1);
if(*value=='}')returnvalue+1;/*emptyarray.*/

item->child=child=cJSON_New_Item();
if(!item->child)return0;
value=skip(parse_string(child,skip(value)));
if(!value)return0;
child->string=child->valuestring;child->valuestring=0;
if(*value!=':'){ep=value;return0;}/*fail!*/
value=skip(parse_value(child,skip(value+1)));/*skipanyspacing,getthevalue.*/
if(!value)return0;

while(*value==',')
{
cJSON*new_item;
if(!(new_item=cJSON_New_Item()))return0;/*memoryfail*/
child->next=new_item;new_item->prev=child;child=new_item;
value=skip(parse_string(child,skip(value+1)));
if(!value)return0;
child->string=child->valuestring;child->valuestring=0;
if(*value!=':'){ep=value;return0;}/*fail!*/
value=skip(parse_value(child,skip(value+1)));/*skipanyspacing,getthevalue.*/
if(!value)return0;
}

if(*value=='}')returnvalue+1;/*endofarray*/
ep=value;return0;/*malformed.*/
}

/*Renderanobjecttotext.*/
staticchar*print_object(cJSON*item,intdepth,intfmt)
{
char**entries=0,**names=0;
char*out=0,*ptr,*ret,*str;intlen=7,i=0,j;
cJSON*child=item->child;
intnumentries=0,fail=0;
/*Countthenumberofentries.*/
while(child)numentries++,child=child->next;
/*Explicitlyhandleemptyobjectcase*/
if(!numentries)
{
out=(char*)cJSON_malloc(fmt?depth+4:3);
if(!out)return0;
ptr=out;*ptr++='{';
if(fmt){*ptr++='\n';for(i=0;i<depth-1;i++)*ptr++='\t';}
*ptr++='}';*ptr++=0;
returnout;
}
/*Allocatespaceforthenamesandtheobjects*/
entries=(char**)cJSON_malloc(numentries*sizeof(char*));
if(!entries)return0;
names=(char**)cJSON_malloc(numentries*sizeof(char*));
if(!names){cJSON_free(entries);return0;}
memset(entries,0,sizeof(char*)*numentries);
memset(names,0,sizeof(char*)*numentries);

/*Collectalltheresultsintoourarrays:*/
child=item->child;depth++;if(fmt)len+=depth;
while(child)
{
names[i]=str=print_string_ptr(child->string);
entries[i++]=ret=print_value(child,depth,fmt);
if(str&&ret)len+=strlen(ret)+strlen(str)+2+(fmt?2+depth:0);elsefail=1;
child=child->next;
}

/*Trytoallocatetheoutputstring*/
if(!fail)out=(char*)cJSON_malloc(len);
if(!out)fail=1;

/*Handlefailure*/
if(fail)
{
for(i=0;i<numentries;i++){if(names[i])cJSON_free(names[i]);if(entries[i])cJSON_free(entries[i]);}
cJSON_free(names);cJSON_free(entries);
return0;
}

/*Composetheoutput:*/
*out='{';ptr=out+1;if(fmt)*ptr++='\n';*ptr=0;
for(i=0;i<numentries;i++)
{
if(fmt)for(j=0;j<depth;j++)*ptr++='\t';
strcpy(ptr,names[i]);ptr+=strlen(names[i]);
*ptr++=':';if(fmt)*ptr++='\t';
strcpy(ptr,entries[i]);ptr+=strlen(entries[i]);
if(i!=numentries-1)*ptr++=',';
if(fmt)*ptr++='\n';*ptr=0;
cJSON_free(names[i]);cJSON_free(entries[i]);
}

cJSON_free(names);cJSON_free(entries);
if(fmt)for(i=0;i<depth-1;i++)*ptr++='\t';
*ptr++='}';*ptr++=0;
returnout;
}

/*GetArraysize/item/objectitem.*/
intcJSON_GetArraySize(cJSON*array){cJSON*c=array->child;inti=0;while(c)i++,c=c->next;returni;}
cJSON*cJSON_GetArrayItem(cJSON*array,intitem){cJSON*c=array->child;while(c&&item>0)item--,c=c->next;returnc;}
cJSON*cJSON_GetObjectItem(cJSON*object,constchar*string){cJSON*c=object->child;while(c&&cJSON_strcasecmp(c->string,string))c=c->next;returnc;}

/*Utilityforarraylisthandling.*/
staticvoidsuffix_object(cJSON*prev,cJSON*item){prev->next=item;item->prev=prev;}
/*Utilityforhandlingreferences.*/
staticcJSON*create_reference(cJSON*item){cJSON*ref=cJSON_New_Item();if(!ref)return0;memcpy(ref,item,sizeof(cJSON));ref->string=0;ref->type|=cJSON_IsReference;ref->next=ref->prev=0;returnref;}

/*Additemtoarray/object.*/
voidcJSON_AddItemToArray(cJSON*array,cJSON*item){cJSON*c=array->child;if(!item)return;if(!c){array->child=item;}else{while(c&&c->next)c=c->next;suffix_object(c,item);}}
voidcJSON_AddItemToObject(cJSON*object,constchar*string,cJSON*item){if(!item)return;if(item->string)cJSON_free(item->string);item->string=cJSON_strdup(string);cJSON_AddItemToArray(object,item);}
voidcJSON_AddItemReferenceToArray(cJSON*array,cJSON*item){cJSON_AddItemToArray(array,create_reference(item));}
voidcJSON_AddItemReferenceToObject(cJSON*object,constchar*string,cJSON*item){cJSON_AddItemToObject(object,string,create_reference(item));}

cJSON*cJSON_DetachItemFromArray(cJSON*array,intwhich){cJSON*c=array->child;while(c&&which>0)c=c->next,which--;if(!c)return0;
if(c->prev)c->prev->next=c->next;if(c->next)c->next->prev=c->prev;if(c==array->child)array->child=c->next;c->prev=c->next=0;returnc;}
voidcJSON_DeleteItemFromArray(cJSON*array,intwhich){cJSON_Delete(cJSON_DetachItemFromArray(array,which));}
cJSON*cJSON_DetachItemFromObject(cJSON*object,constchar*string){inti=0;cJSON*c=object->child;while(c&&cJSON_strcasecmp(c->string,string))i++,c=c->next;if(c)returncJSON_DetachItemFromArray(object,i);return0;}
voidcJSON_DeleteItemFromObject(cJSON*object,constchar*string){cJSON_Delete(cJSON_DetachItemFromObject(object,string));}

/*Replacearray/objectitemswithnewones.*/
voidcJSON_ReplaceItemInArray(cJSON*array,intwhich,cJSON*newitem){cJSON*c=array->child;while(c&&which>0)c=c->next,which--;if(!c)return;
newitem->next=c->next;newitem->prev=c->prev;if(newitem->next)newitem->next->prev=newitem;
if(c==array->child)array->child=newitem;elsenewitem->prev->next=newitem;c->next=c->prev=0;cJSON_Delete(c);}
voidcJSON_ReplaceItemInObject(cJSON*object,constchar*string,cJSON*newitem){inti=0;cJSON*c=object->child;while(c&&cJSON_strcasecmp(c->string,string))i++,c=c->next;if(c){newitem->string=cJSON_strdup(string);cJSON_ReplaceItemInArray(object,i,newitem);}}

/*Createbasictypes:*/
cJSON*cJSON_CreateNull(void){cJSON*item=cJSON_New_Item();if(item)item->type=cJSON_NULL;returnitem;}
cJSON*cJSON_CreateTrue(void){cJSON*item=cJSON_New_Item();if(item)item->type=cJSON_True;returnitem;}
cJSON*cJSON_CreateFalse(void){cJSON*item=cJSON_New_Item();if(item)item->type=cJSON_False;returnitem;}
cJSON*cJSON_CreateBool(intb){cJSON*item=cJSON_New_Item();if(item)item->type=b?cJSON_True:cJSON_False;returnitem;}
cJSON*cJSON_CreateNumber(doublenum){cJSON*item=cJSON_New_Item();if(item){item->type=cJSON_Number;item->valuedouble=num;item->valueint=(int)num;}returnitem;}
cJSON*cJSON_CreateString(constchar*string){cJSON*item=cJSON_New_Item();if(item){item->type=cJSON_String;item->valuestring=cJSON_strdup(string);}returnitem;}
cJSON*cJSON_CreateArray(void){cJSON*item=cJSON_New_Item();if(item)item->type=cJSON_Array;returnitem;}
cJSON*cJSON_CreateObject(void){cJSON*item=cJSON_New_Item();if(item)item->type=cJSON_Object;returnitem;}

/*CreateArrays:*/
cJSON*cJSON_CreateIntArray(constint*numbers,intcount){inti;cJSON*n=0,*p=0,*a=cJSON_CreateArray();for(i=0;a&&i<count;i++){n=cJSON_CreateNumber(numbers[i]);if(!i)a->child=n;elsesuffix_object(p,n);p=n;}returna;}
cJSON*cJSON_CreateFloatArray(constfloat*numbers,intcount){inti;cJSON*n=0,*p=0,*a=cJSON_CreateArray();for(i=0;a&&i<count;i++){n=cJSON_CreateNumber(numbers[i]);if(!i)a->child=n;elsesuffix_object(p,n);p=n;}returna;}
cJSON*cJSON_CreateDoubleArray(constdouble*numbers,intcount){inti;cJSON*n=0,*p=0,*a=cJSON_CreateArray();for(i=0;a&&i<count;i++){n=cJSON_CreateNumber(numbers[i]);if(!i)a->child=n;elsesuffix_object(p,n);p=n;}returna;}
cJSON*cJSON_CreateStringArray(constchar**strings,intcount){inti;cJSON*n=0,*p=0,*a=cJSON_CreateArray();for(i=0;a&&i<count;i++){n=cJSON_CreateString(strings[i]);if(!i)a->child=n;elsesuffix_object(p,n);p=n;}returna;}

/*Duplication*/
cJSON*cJSON_Duplicate(cJSON*item,intrecurse)
{
cJSON*newitem,*cptr,*nptr=0,*newchild;
/*Bailonbadptr*/
if(!item)return0;
/*Createnewitem*/
newitem=cJSON_New_Item();
if(!newitem)return0;
/*Copyoverallvars*/
newitem->type=item->type&(~cJSON_IsReference),newitem->valueint=item->valueint,newitem->valuedouble=item->valuedouble;
if(item->valuestring){newitem->valuestring=cJSON_strdup(item->valuestring);if(!newitem->valuestring){cJSON_Delete(newitem);return0;}}
if(item->string){newitem->string=cJSON_strdup(item->string);if(!newitem->string){cJSON_Delete(newitem);return0;}}
/*Ifnon-recursive,thenwe'redone!*/
if(!recurse)returnnewitem;
/*Walkthe->nextchainforthechild.*/
cptr=item->child;
while(cptr)
{
newchild=cJSON_Duplicate(cptr,1);/*Duplicate(withrecurse)eachiteminthe->nextchain*/
if(!newchild){cJSON_Delete(newitem);return0;}
if(nptr){nptr->next=newchild,newchild->prev=nptr;nptr=newchild;}/*Ifnewitem->childalreadyset,thencrosswire->prevand->nextandmoveon*/
else{newitem->child=newchild;nptr=newchild;}/*Setnewitem->childandmovetoit*/
cptr=cptr->next;
}
returnnewitem;
}

voidcJSON_Minify(char*json)
{
char*into=json;
while(*json)
{
if(*json=='')json++;
elseif(*json=='\t')json++;//Whitespacecharacters.
elseif(*json=='\r')json++;
elseif(*json=='\n')json++;
elseif(*json=='/'&&json[1]=='/')while(*json&&*json!='\n')json++;//double-slashcomments,toendofline.
elseif(*json=='/'&&json[1]=='*'){while(*json&&!(*json=='*'&&json[1]=='/'))json++;json+=2;}//multilinecomments.
elseif(*json=='\"'){*into++=*json++;while(*json&&*json!='\"'){if(*json=='\\')*into++=*json++;*into++=*json++;}*into++=*json++;}//stringliterals,whichare\"sensitive.
else*into++=*json++;//Allothercharacters.
}
*into=0;//andnull-terminate.
}
