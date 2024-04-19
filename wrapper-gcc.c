#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define errExit(fmt, ...) do {  \
  fprintf(stderr, fmt, ##__VA_ARGS__); \
  exit(EXIT_FAILURE); \
} while (0)

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;

static u8** cc_params;              /* Parameters passed to the real CC  */
static u32  cc_par_cnt = 1;         /* Param count, including argv0      */

static const char* valid_optis[] = {
  "O0",
  "O1",
  "O2",
  "O3",
  "Os"
};

/* Copy argv to cc_params, making the necessary edits. */

static void edit_param(u32 argc, char** argv) {
  u8* name;
  u8* IF_DEBUG;
  u8* OPTI_LEVEL;
  
  cc_params = calloc(argc + 128, sizeof(u8*));
  name = strrchr(argv[0], '/');
  if (!name) name = argv[0]; else name++;
  
  if (!strcmp(name, "clang++")) {
    u8* alt_cxx = getenv("WRAPPER_CLANGXX");
    if (!alt_cxx) {
      errExit(
        "Environment variable %s is not set\n", 
        "WRAPPER_CLANGXX"
      );
    } else {
      cc_params[0] = alt_cxx;
    }
  } else if (!strcmp(name, "clang")) {
    u8* alt_cc = getenv("WRAPPER_CLANG");
    if (!alt_cc) {
        errExit(
        "Environment variable %s is not set\n", 
        "WRAPPER_CLANG"
      );
    } else {
      cc_params[0] = alt_cc;
    }
  } else if (!strcmp(name, "g++") || !strcmp(name, "c++")) {
    u8* alt_cxx = getenv("WRAPPER_CXX");
    if (!alt_cxx) {
        errExit(
        "Environment variable %s is not set\n", 
        "WRAPPER_CXX"
      );
    } else {
      cc_params[0] = alt_cxx;
    }
  } else if (!strcmp(name, "gcc") || !strcmp(name, "cc")) {
    u8* alt_cc = getenv("WRAPPER_CC");
    if (!alt_cc) {
        errExit(
        "Environment variable %s is not set\n", 
        "WRAPPER_CC"
      );
    } else {
      cc_params[0] = alt_cc;
    }
  } else {
    errExit(
      "Error call %s from \n", 
      name
    );
  }

  #ifdef WRAPPER_DEBUG
    fprintf(stderr, 
      "[DEBUG]: selected compiler ** %s **\n",
      cc_params[0]
    );
  #endif
  
  while (--argc) {
    u8* cur = *(++argv);

    if (!strcmp(cur, "-s")) continue;
    
    if (!strcmp(cur, "-g")) continue;
    if (!strcmp(cur, "-ggdb")) continue;

    if (!strcmp(cur, "-O0")) continue;
    if (!strcmp(cur, "-O1")) continue;
    if (!strcmp(cur, "-O2")) continue;
    if (!strcmp(cur, "-O3")) continue;
    if (!strcmp(cur, "-Os")) continue;
    
    if (!strcmp(cur, "-DNDEBUG")) continue;
    
    cc_params[cc_par_cnt++] = cur;
  }

  if (getenv("WRAPPER_OPTI")) {
    u8* opti = calloc(4, sizeof(char));
    u8* opti_env = getenv("WRAPPER_OPTI");

    bzero(opti, 4);
    for (int i=0; i<sizeof(valid_optis)/sizeof(const char*); i++) {
      if (!strcmp(opti_env, valid_optis[i]))
        snprintf(opti, 4, "-%s", valid_optis[i]);
    }
    
    if (!*opti) {
      fprintf(stderr,
        "Invaild Optimization Options '%s'.\n",
        opti_env
      );
      exit(-2);
    }
    
    cc_params[cc_par_cnt++] = opti;
    
    #ifdef WRAPPER_DEBUG
      fprintf(stderr, 
        "[DEBUG]: optimizations option '%s' added\n", 
        getenv("WRAPPER_OPTI")
      );
    #endif  
  }
  
  if (getenv("WRAPPER_DEBUG")) {
    if (!strcmp(getenv("WRAPPER_DEBUG"), "1")) {
      cc_params[cc_par_cnt++] = "-g";
      #ifdef WRAPPER_DEBUG
        fprintf(stderr, "[DEBUG]: Debugging option '-g' added\n");
      #endif  
    }
  }

  if (getenv("WRAPPER_OTHER")) {
    #ifdef WRAPPER_DEBUG
      fprintf(stderr, 
      "[DEBUG]: Other option '%s' added\n",
      getenv("WRAPPER_OTHER")
    );
    #endif  
    cc_params[cc_par_cnt++] = getenv("WRAPPER_OTHER");
  }  


  cc_params[cc_par_cnt++] = 0;

  #ifdef WRAPPER_DEBUG
    fprintf(stderr, "[DEBUG]: cmd argv\n");
    for (u8** head=cc_params; *head ; head++) {
      fprintf(stderr,
        "[DEBUG]: %s\n",
        *head
      );
    }
  #endif

}

/* Main entry point */

int main(int argc, char** argv) {

  if (argc < 2) {
    errExit( 
      "This is the the wrapper of 'gcc, g++, clang, clang++'.\n"
      "It is used to modify the compilation options in a uniform way.\n"
      "Changes are specified by environment variables 'WRAPPER_DEBUG, WRAPPER_OPTI, WRAPPER_OTHER'"
      "\n"
    );
  }
  edit_param(argc, argv);
  execvp(cc_params[0], (char**)cc_params);
  
  errExit("Return from execvp.");
  
  return 0;
}
