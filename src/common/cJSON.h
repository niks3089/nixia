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

#ifndefcJSON__h
#definecJSON__h

#ifdef__cplusplus
extern"C"
{
#endif

/*cJSONTypes:*/
#definecJSON_False0
#definecJSON_True1
#definecJSON_NULL2
#definecJSON_Number3
#definecJSON_String4
#definecJSON_Array5
#definecJSON_Object6

#definecJSON_IsReference256

/*ThecJSONstructure:*/
typedefstructcJSON{
structcJSON*next,*prev;/*next/prevallowyoutowalkarray/objectchains.Alternatively,useGetArraySize/GetArrayItem/GetObjectItem*/
structcJSON*child;/*Anarrayorobjectitemwillhaveachildpointerpointingtoachainoftheitemsinthearray/object.*/

inttype;/*Thetypeoftheitem,asabove.*/

char*valuestring;/*Theitem'sstring,iftype==cJSON_String*/
intvalueint;/*Theitem'snumber,iftype==cJSON_Number*/
doublevaluedouble;/*Theitem'snumber,iftype==cJSON_Number*/

char*string;/*Theitem'snamestring,ifthisitemisthechildof,orisinthelistofsubitemsofanobject.*/
}cJSON;

typedefstructcJSON_Hooks{
void*(*malloc_fn)(size_tsz);
void(*free_fn)(void*ptr);
}cJSON_Hooks;

/*Supplymalloc,reallocandfreefunctionstocJSON*/
externvoidcJSON_InitHooks(cJSON_Hooks*hooks);


/*SupplyablockofJSON,andthisreturnsacJSONobjectyoucaninterrogate.CallcJSON_Deletewhenfinished.*/
externcJSON*cJSON_Parse(constchar*value);
/*RenderacJSONentitytotextfortransfer/storage.Freethechar*whenfinished.*/
externchar*cJSON_Print(cJSON*item);
/*RenderacJSONentitytotextfortransfer/storagewithoutanyformatting.Freethechar*whenfinished.*/
externchar*cJSON_PrintUnformatted(cJSON*item);
/*DeleteacJSONentityandallsubentities.*/
externvoidcJSON_Delete(cJSON*c);

/*Returnsthenumberofitemsinanarray(orobject).*/
externintcJSON_GetArraySize(cJSON*array);
/*Retrieveitemnumber"item"fromarray"array".ReturnsNULLifunsuccessful.*/
externcJSON*cJSON_GetArrayItem(cJSON*array,intitem);
/*Getitem"string"fromobject.Caseinsensitive.*/
externcJSON*cJSON_GetObjectItem(cJSON*object,constchar*string);

/*Foranalysingfailedparses.Thisreturnsapointertotheparseerror.You'llprobablyneedtolookafewcharsbacktomakesenseofit.DefinedwhencJSON_Parse()returns0.0whencJSON_Parse()succeeds.*/
externconstchar*cJSON_GetErrorPtr(void);

/*ThesecallscreateacJSONitemoftheappropriatetype.*/
externcJSON*cJSON_CreateNull(void);
externcJSON*cJSON_CreateTrue(void);
externcJSON*cJSON_CreateFalse(void);
externcJSON*cJSON_CreateBool(intb);
externcJSON*cJSON_CreateNumber(doublenum);
externcJSON*cJSON_CreateString(constchar*string);
externcJSON*cJSON_CreateArray(void);
externcJSON*cJSON_CreateObject(void);

/*TheseutilitiescreateanArrayofcountitems.*/
externcJSON*cJSON_CreateIntArray(constint*numbers,intcount);
externcJSON*cJSON_CreateFloatArray(constfloat*numbers,intcount);
externcJSON*cJSON_CreateDoubleArray(constdouble*numbers,intcount);
externcJSON*cJSON_CreateStringArray(constchar**strings,intcount);

/*Appenditemtothespecifiedarray/object.*/
externvoidcJSON_AddItemToArray(cJSON*array,cJSON*item);
externvoidcJSON_AddItemToObject(cJSON*object,constchar*string,cJSON*item);
/*Appendreferencetoitemtothespecifiedarray/object.UsethiswhenyouwanttoaddanexistingcJSONtoanewcJSON,butdon'twanttocorruptyourexistingcJSON.*/
externvoidcJSON_AddItemReferenceToArray(cJSON*array,cJSON*item);
externvoidcJSON_AddItemReferenceToObject(cJSON*object,constchar*string,cJSON*item);

/*Remove/DetatchitemsfromArrays/Objects.*/
externcJSON*cJSON_DetachItemFromArray(cJSON*array,intwhich);
externvoidcJSON_DeleteItemFromArray(cJSON*array,intwhich);
externcJSON*cJSON_DetachItemFromObject(cJSON*object,constchar*string);
externvoidcJSON_DeleteItemFromObject(cJSON*object,constchar*string);

/*Updatearrayitems.*/
externvoidcJSON_ReplaceItemInArray(cJSON*array,intwhich,cJSON*newitem);
externvoidcJSON_ReplaceItemInObject(cJSON*object,constchar*string,cJSON*newitem);

/*DuplicateacJSONitem*/
externcJSON*cJSON_Duplicate(cJSON*item,intrecurse);
/*Duplicatewillcreateanew,identicalcJSONitemtotheoneyoupass,innewmemorythatwill
needtobereleased.Withrecurse!=0,itwillduplicateanychildrenconnectedtotheitem.
Theitem->nextand->prevpointersarealwayszeroonreturnfromDuplicate.*/

/*ParseWithOptsallowsyoutorequire(andcheck)thattheJSONisnullterminated,andtoretrievethepointertothefinalbyteparsed.*/
externcJSON*cJSON_ParseWithOpts(constchar*value,constchar**return_parse_end,intrequire_null_terminated);

externvoidcJSON_Minify(char*json);

/*Macrosforcreatingthingsquickly.*/
#definecJSON_AddNullToObject(object,name)cJSON_AddItemToObject(object,name,cJSON_CreateNull())
#definecJSON_AddTrueToObject(object,name)cJSON_AddItemToObject(object,name,cJSON_CreateTrue())
#definecJSON_AddFalseToObject(object,name)cJSON_AddItemToObject(object,name,cJSON_CreateFalse())
#definecJSON_AddBoolToObject(object,name,b)cJSON_AddItemToObject(object,name,cJSON_CreateBool(b))
#definecJSON_AddNumberToObject(object,name,n)cJSON_AddItemToObject(object,name,cJSON_CreateNumber(n))
#definecJSON_AddStringToObject(object,name,s)cJSON_AddItemToObject(object,name,cJSON_CreateString(s))

/*Whenassigninganintegervalue,itneedstobepropagatedtovaluedoubletoo.*/
#definecJSON_SetIntValue(object,val)((object)?(object)->valueint=(object)->valuedouble=(val):(val))

#ifdef__cplusplus
}
#endif

#endif
