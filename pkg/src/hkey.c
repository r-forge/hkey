#include "R.h"
#define USE_RINTERNALS
#include "Rinternals.h"
#include "Rdefines.h"
#include "R_ext/Rdynload.h"

#include <windows.h>
#include <winreg.h>

// hkey: read and write the Windows registry

// create an R external pointer object (EXTPTR SEXP) from a pointer
#define PTR_TO_EXTPTR(_P_) R_MakeExternalPtr((void *)(_P_), NULL, NULL)

// for building tables of R-accessible functions, this macro is useful:
// X is the name of the function, N is the number of parameters it takes

#define MKREF(FUN, N) {#FUN, (DL_FUNC) &FUN, N}

// a vector of strings we preserve for frequent use

SEXP PRIVATE_STRINGS = NULL;

// for convenience, we add our own strings to the end of the registry type list

#define MAX_REG_TYPE 11

#define HKEY_HKEY (MAX_REG_TYPE + 1)
#define NUM_PRIVATE_STRINGS (HKEY_HKEY + 1)

// the known Windows keys
 
typedef struct
{
  HKEY hkey;
  char *name;
} t_predefined_key;

#define NUM_KNOWN_KEYS 6
t_predefined_key known_keys[6] = {
  { HKEY_CLASSES_ROOT,		 "HKEY_CLASSES_ROOT"},
  { HKEY_CURRENT_CONFIG,	 "HKEY_CURRENT_CONFIG"},
  { HKEY_CURRENT_USER,		 "HKEY_CURRENT_USER"},
  { HKEY_LOCAL_MACHINE,		 "HKEY_LOCAL_MACHINE"},
  { HKEY_PERFORMANCE_DATA,	 "HKEY_PERFORMANCE_DATA"},
  { HKEY_USERS,                  "HKEY_USERS"}
};

char *
strip_known_key (char *keyname, HKEY *hkey) 
{
  // if keyname begins with a known key, return 
  // a pointer to the first character after the known
  // key value and any following "/", and return
  // the handle for that key in &hkey, if not NULL
  
  // returns NULL if keyname doesn't start with a known key

  int i, n;
  for (i = 0; i < NUM_KNOWN_KEYS; ++i) {
    n = strlen(known_keys[i].name);
    if (!strncmp(keyname, known_keys[i].name, n)) 
      break;
  }
  if (i == NUM_KNOWN_KEYS)
    return NULL;

  if (hkey)
    *hkey = known_keys[i].hkey;

  keyname += n;
  return keyname + (*keyname == '/' || *keyname == '\\');
}

char *
cdr_name (char *keyname) {
  // return a pointer to the remainder of a keyname
  // once a single level (and trailing '/' or '\') 
  // has been skipped.  This is destructive, since it
  // overwrites the trailing '/' or '\' with a '\0';
  // returns NULL if there is no remaining keyname component

  while (*keyname && *keyname != '/' && *keyname != '\\')
    ++keyname;
  if (! *keyname)
    return NULL;

  *keyname = '\0';
  return keyname + 1;
}

SEXP
get_node (SEXP key)
{

  // return an object corresponding to the node named by character vector "key"
  // If no such node is found, return R NULL
  // Otherwise, return an environment where each value name is
  // bound to the node's corresponding value, and each subkey
  // name has an active binding which invokes get_node if that
  // member is accessed.

  SEXP rv;
  HKEY hkey = 0, phkey = 0;

  char *keyname;
  char *carname, *cdrname;
  int ok = TRUE;

  keyname = Calloc(LENGTH(STRING_ELT(key, 0)), char);
  strcpy(keyname, CHAR(STRING_ELT(key, 0)));

  if ((cdrname = strip_known_key(keyname, &hkey))) {

    // descend the key tree until we get to the final level
    // return R NULL if a sublevel doesn't exist
    do {
      carname = cdrname;
      cdrname = cdr_name(carname);
      phkey = hkey;
      if (ERROR_SUCCESS != RegOpenKey (phkey, carname, &hkey)) {
	ok = FALSE;
	break;
      }
      RegCloseKey(phkey);
    } while (cdrname);
  } else {
    ok = FALSE;
  }

  if (phkey)
    RegCloseKey(phkey);
  Free(keyname);

  // return value is either a wrapped HKEY or NULL
  if (ok) {
    PROTECT(rv = PTR_TO_EXTPTR(hkey));
    SET_CLASS(rv, STRING_ELT(PRIVATE_STRINGS, HKEY_HKEY));
    UNPROTECT(1);
  } else {
    rv = R_NilValue;
  }
  return rv;
}

SEXP
get_subnode (SEXP hkeysxp, SEXP childname)
{
  // return an hkey object corresponding to child of node hkeysxp
  // named by the character vector childname

  // If no such node is found, return R NULL

  SEXP rv = R_NilValue;
  HKEY hkey = (HKEY) EXTPTR_PTR(hkeysxp);
  HKEY hkeychild;

  if (ERROR_SUCCESS == RegOpenKey (hkey, CHAR(STRING_ELT(childname, 0)), &hkeychild)) {
    PROTECT(rv = PTR_TO_EXTPTR(hkeychild));
    SET_CLASS(rv, STRING_ELT(PRIVATE_STRINGS, HKEY_HKEY));
    UNPROTECT(1);
  }
  return rv;
}

SEXP
create_subnode (SEXP hkeysxp, SEXP childname)
{
  // return an hkey object corresponding to a new child of node hkeysxp
  // named by the character vector childname
  // FIXME: we don't create symbolic link subnodes

  SEXP rv = R_NilValue;
  HKEY hkey = (HKEY) EXTPTR_PTR(hkeysxp);
  HKEY hkeychild;

  if (ERROR_SUCCESS == RegCreateKey (hkey, CHAR(STRING_ELT(childname, 0)), &hkeychild)) {
    PROTECT(rv = PTR_TO_EXTPTR(hkeychild));
    SET_CLASS(rv, STRING_ELT(PRIVATE_STRINGS, HKEY_HKEY));
    UNPROTECT(1);
  }
  return rv;
}


LONG
do_delete_subnode (HKEY hkey, char *childname)
{
  // delete the child of node hkeysxp having name childname
  // This first recursively deletes any children.
  // Windows is supposed to have a SHDeleteKey function for doing
  // this in shlwapi.dll, but I can't seem to link to it.
  int i;
  DWORD n, bufsize, subkeynamelen;
  char *buf;
  LONG err;
  HKEY subhkey;

#ifdef DEBUG
  printf("Beginning to delete subnode named '%s'\n", childname);
#endif

  // open the child to be deleted so we can 
  // learn if it has any children

  if (ERROR_SUCCESS != (err = RegOpenKey (hkey, childname, &subhkey)))
    error("hkey: delete_subnode got error %d calling RegOpenKey", err);


  // find out the target child's number of children of its own
  if (ERROR_SUCCESS != (err = RegQueryInfoKey(subhkey, NULL, NULL, NULL,
				    (LPDWORD) &n, (LPDWORD) &bufsize,
				    NULL, 
				    NULL, NULL,
				    NULL, NULL, NULL)))
    {
      error("hkey: delete_subnode got error #%d calling RegQueryInfoKey", err);
    }
#ifdef DEBUG
  printf("Got num_subkeys=%d max_subkey_len=%d\n", (int) n, (int) bufsize);
#endif
  if (n > 0) {
    // delete all children of the target child
    buf = Calloc(++bufsize, char);
    for (i = 0; i < n; ++i) {
      subkeynamelen = bufsize;
      if (ERROR_SUCCESS != (err = RegEnumKeyEx(subhkey, 0 /* always delete first child*/, buf, (LPDWORD) &subkeynamelen, NULL, NULL, NULL, NULL)))
	error("hkey: do_delete_subnode got error #%d calling RegEnumKeyEx", err);
#ifdef DEBUG
      if (!do_delete_subnode(subhkey, buf)) 
	error("hkey: do_delete_subnode: recursive call failed on child '%s'", buf);
#endif
    }
  }
  RegCloseKey(subhkey);

#ifdef DEBUG
  printf("Finished deleting children of subnode named '%s'\n", childname);
#endif

  if (ERROR_SUCCESS != (err = RegDeleteKey(hkey, childname)))
    error("hkey: do_delete_subnode got error #%d calling RegDeleteKey for child '%s'", err, childname);
 
#ifdef DEBUG
  printf("Deleted subnode named '%s'\n", childname);
#endif
  return TRUE;
}

SEXP
delete_subnode (SEXP hkeysxp, SEXP childname)
{
  // delete the child of node hkeysxp having name childname
  // This first recursively deletes any children.
  // Windows is supposed to have a SHDeleteKey function for doing
  // this in shlwapi.dll, but I can't seem to link to it.

  // Return NULL on success

  HKEY hkey = (HKEY) EXTPTR_PTR(hkeysxp);
  LONG err;

  err = do_delete_subnode (hkey, CHAR(STRING_ELT(childname, 0)));
#ifdef DEBUG
  if (!err)
    error("hkey: delete_subnode: recursive call to do_delete_subnode failed on child '%s'", CHAR(STRING_ELT(childname, 0)));
#endif
  return R_NilValue;
}

SEXP
delete_value (SEXP hkeysxp, SEXP valname)
{
  // delete the specified value for the given node

  LONG err;
  HKEY hkey = (HKEY) EXTPTR_PTR(hkeysxp);

#ifdef DEBUG
  printf("Deleting value named '%s'\n", CHAR(STRING_ELT(valname, 0)));
#endif

  if (ERROR_SUCCESS != (err = RegDeleteValue (hkey, CHAR(STRING_ELT(valname, 0)))))
    error("hkey: delete_value got error %d calling RegDeleteValue", err);

  return R_NilValue;
}


SEXP
close_node (SEXP hkeysxp)
{
  HKEY hkey = (HKEY) EXTPTR_PTR(hkeysxp);
  RegCloseKey(hkey);
  return R_NilValue;
}

SEXP
get_subnode_names (SEXP hkeysxp)
{
  DWORD n, bufsize, keynamelen;
  int i;
  char *buf;
  LONG err;
  SEXP rv;

  HKEY hkey = (HKEY) EXTPTR_PTR(hkeysxp);

  // determine maximum subkey name length etc.

  if (ERROR_SUCCESS != (err = RegQueryInfoKey(hkey, NULL, NULL, NULL,
				    (LPDWORD) &n, (LPDWORD) &bufsize,
				    NULL, 
				    NULL, NULL,
				    NULL, NULL, NULL)))
    {
      error("hkey: got error #%d calling RegQueryInfoKey", err);
    }
  
  PROTECT(rv = allocVector(STRSXP, n));
  buf = Calloc(++bufsize, char);

  // bind subkey names

  for (i = 0; i < n; ++i) {
    keynamelen = bufsize;
    if (ERROR_SUCCESS != (err =RegEnumKeyEx(hkey, i, buf, (LPDWORD) &keynamelen, NULL, NULL, NULL, NULL))) {
      error("hkey: got error #%d calling RegEnumKeyEx", err);
    }
    SET_STRING_ELT(rv, i, mkChar(buf));
  }
  UNPROTECT(1);
  return rv;
}

SEXP
set_key_value (SEXP hkeysxp, SEXP valnamesxp, SEXP typesxp, SEXP valsxp)
{
  HKEY hkey = (HKEY) EXTPTR_PTR(hkeysxp);
  int type = INTEGER(typesxp)[0];
  int len = 0;
  LONG err;
  VOID *dat = NULL;

  switch(type) {
  case REG_NONE:
    dat = NULL;
    len = 0;
    break;
  case REG_SZ:
  case REG_EXPAND_SZ:
  case REG_LINK:
    dat = CHAR(STRING_ELT(valsxp, 0));
    len = 1 + LENGTH(STRING_ELT(valsxp, 0));
    break;
  case REG_BINARY:
    dat = RAW(valsxp);
    len = LENGTH(valsxp);
    break;
  case REG_DWORD:
  case REG_DWORD_BIG_ENDIAN:
    dat = INTEGER(valsxp);
    len = sizeof(INTEGER(valsxp)[0]);
    break;
  case REG_QWORD:
    dat = INTEGER(valsxp);
    len = 2 * sizeof(INTEGER(valsxp)[0]);
    break;
  case REG_MULTI_SZ:
    // we must create a mashed version of this character vector,
    // consisting of all elements concatenated, delimited by '\0'
    // and ending in '\0\0'
    {
      int i;
      char *p;
      for (i = 0; i < LENGTH(valsxp); ++i)
	len += 1 + strlen(CHAR(STRING_ELT(valsxp, i)));
      ++len; // for the final extra '\0'
      dat = p = (char *) Calloc(len, char);
      for (i = 0; i < LENGTH(valsxp); ++i) {
	strcpy(p, CHAR(STRING_ELT(valsxp, i)));
	p += 1 + strlen(p);
      }
      *p = '\0'; // the final extra '\0'
#ifdef DEBUG
      printf("Setting a REG_MULTI_SZ value with claimed length %d and actual length %d\n", len, p - (char *) dat);
#endif
    }
    break;
  default:
    error("hkey: I don't know how to set registry values of type %d", type);
  }
  err = RegSetValueEx(hkey, CHAR(STRING_ELT(valnamesxp, 0)), 0, type, dat, len);
  if (type == REG_MULTI_SZ)
    Free(dat);
  if (err != ERROR_SUCCESS)
    error("hkey: I was not able to set a value for data '%s'", CHAR(STRING_ELT(valnamesxp, 0)));
  return R_NilValue;
}
  
SEXP
get_node_values (SEXP hkeysxp, SEXP rho, SEXP defnamesxp)
{
  // bind the value names of an HKEY to their values
  // in the given environment.  Return the environment.
  // The name for the "(Default)" value of a node is
  // taken from the first element of character vector defnamesxp.

  DWORD n, bufsize, valbufsize, valnamelen, valtype, vallen;
  int i;
  char *buf, *valbuf;
  LONG err;
  SEXP item = NULL;

  HKEY hkey = (HKEY) EXTPTR_PTR(hkeysxp);

  // determine maximum value name length etc.

  if (ERROR_SUCCESS != (err = RegQueryInfoKey(hkey, NULL, NULL, NULL,
				    NULL, NULL,
				    NULL, 
				    (LPDWORD)&n, (LPDWORD)&bufsize, (LPDWORD)&valbufsize,
					      NULL, NULL)))
    {
      error("hkey: hkey got error #%d calling RegQueryInfo", err);
    }

#ifdef DEBUG
  printf("There are %d values with a maximum name size=%d and maxsize=%d\n", (int) n, (int) bufsize, (int) valbufsize);
#endif

  buf = Calloc(++bufsize, char);
  valbuf = Calloc(++valbufsize, char);
  for (i = 0; i < n; ++i) {
    valnamelen = bufsize;
    vallen = valbufsize;
    if (ERROR_SUCCESS != (err = RegEnumValue(hkey, i, buf, (LPDWORD)&valnamelen, NULL, (LPDWORD)&valtype, valbuf, (LPDWORD)&vallen))) {
      error("hkey: hkey got error #%d calling RegEnumValue", err);
    }
#ifdef DEBUG
    printf("Got name %s with type=%d and data size=%d\n", buf, (int) valtype, (int) vallen);
#endif
    switch(valtype) {
    case REG_BINARY:
    case REG_NONE:
      PROTECT(item = allocVector(RAWSXP, vallen));
      memcpy(RAW(item), valbuf, vallen);
      break;
    case REG_DWORD:
    case REG_DWORD_BIG_ENDIAN:
      PROTECT(item = allocVector(INTSXP, 1));
      INTEGER(item)[0] = *(int *) valbuf;
      break;
    case REG_EXPAND_SZ:
    case REG_LINK:
    case REG_SZ:
      PROTECT(item = allocVector(STRSXP, 1));
      SET_STRING_ELT(item, 0, mkChar(valbuf));
      break;
    case REG_QWORD:
      PROTECT(item = allocVector(INTSXP, 2));
      memcpy(INTEGER(item), valbuf, 2 * sizeof(int));
      break;
      
    case REG_MULTI_SZ:
      {
	// wasteful code, but who cares
	int m = 0;
	char *p = valbuf;
	while (strlen(p)){
	  p += 1 + strlen(p);
	  ++m;
	}
	PROTECT(item = allocVector(STRSXP, m));
	p = valbuf;
	m = 0;
	while (strlen(p)) {
	  SET_STRING_ELT(item, m, mkChar(p));
	  p += 1 + strlen(p);
	}
      }
      break;
    default:
      error("hkey: unknown / unsupported type (%d) in registry value", valtype);
    }
    SET_CLASS(item, VECTOR_ELT(PRIVATE_STRINGS, (int) valtype));
#ifdef DEBUG
    printf("About to install %s= something of class %d, length=%d\n", buf, TYPEOF(item), LENGTH(item));
#endif
    defineVar(install(valnamelen ? buf : CHAR(STRING_ELT(defnamesxp, 0))), item, rho);
    UNPROTECT(1); // unprotect item
  }
  return rho;
}
  
  
/*================================================================

  hkey.dll method registration, initialization, and destruction

  ================================================================*/

R_CallMethodDef hkey_call_methods[]  = {
  // R hook functions
  MKREF(get_node		, 1),
  MKREF(get_subnode		, 2),
  MKREF(create_subnode		, 2),
  MKREF(close_node		, 1),
  MKREF(get_subnode_names	, 1),
  MKREF(get_node_values		, 3),
  MKREF(delete_subnode          , 2),
  MKREF(delete_value            , 2),
  MKREF(set_key_value           , 4),
  {NULL, NULL, 0}
};

void
R_init_hkey(DllInfo *info)
{
  /* Register routines, allocate resources. */
  
  R_registerRoutines(info, NULL, hkey_call_methods, NULL, NULL);
  R_PreserveObject(PRIVATE_STRINGS = allocVector(VECSXP, NUM_PRIVATE_STRINGS));

  // create string object in same order which constants are defined

  SET_VECTOR_ELT(PRIVATE_STRINGS, REG_BINARY,			mkString("REG_BINARY"));
  SET_VECTOR_ELT(PRIVATE_STRINGS, REG_DWORD,			mkString("REG_DWORD"));
  SET_VECTOR_ELT(PRIVATE_STRINGS, REG_DWORD_LITTLE_ENDIAN,	mkString("REG_DWORD_LITTLE_ENDIAN"));
  SET_VECTOR_ELT(PRIVATE_STRINGS, REG_DWORD_BIG_ENDIAN,		mkString("REG_DWORD_BIG_ENDIAN"));
  SET_VECTOR_ELT(PRIVATE_STRINGS, REG_EXPAND_SZ,		mkString("REG_EXPAND_SZ"));
  SET_VECTOR_ELT(PRIVATE_STRINGS, REG_LINK,			mkString("REG_LINK"));
  SET_VECTOR_ELT(PRIVATE_STRINGS, REG_MULTI_SZ,			mkString("REG_MULTI_SZ"));
  SET_VECTOR_ELT(PRIVATE_STRINGS, REG_NONE,			mkString("REG_NONE"));
  SET_VECTOR_ELT(PRIVATE_STRINGS, REG_QWORD,			mkString("REG_QWORD"));
  SET_VECTOR_ELT(PRIVATE_STRINGS, REG_QWORD_LITTLE_ENDIAN,	mkString("REG_QWORD_LITTLE_ENDIAN"));
  SET_VECTOR_ELT(PRIVATE_STRINGS, REG_SZ,                    	mkString("REG_SZ"));
  SET_VECTOR_ELT(PRIVATE_STRINGS, HKEY_HKEY,               	mkString("hkey"));
}

void
R_unload_hkey(DllInfo *info)
{
  /* Release resources. */
  R_ReleaseObject(PRIVATE_STRINGS);
}
