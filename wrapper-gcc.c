#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>

#define errExit(fmt, ...) do {  \
  fprintf(stderr, fmt, ##__VA_ARGS__); \
  exit(EXIT_FAILURE); \
} while (0)

#define MAX_LINE_LENGTH 256
#define wlog(fmt, ...) fprintf(stderr, "[DEBUG]: " fmt, ##__VA_ARGS__)

#define WRAPPER_OPTI   "WRAPPER_OPTI"
#define WRAPPER_DEBUG  "WRAPPER_DEBUG"
#define WRAPPER_APPEND "WRAPPER_APPEND"

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;

static u8** cc_params;              /* Parameters passed to the real CC  */
static u32  cc_par_cnt = 1;         /* Param count, including argv0      */

static const char* valid_optis[] = { "O0",
                                     "O1",
                                     "O2",
                                     "O3",
                                     "Os"};

int valid_opti(const char* opti) {
  if (opti[0] == '-')
    opti++;
  
  for (int i=0; i < sizeof(valid_optis)/sizeof(const char*); i++) {
    if (!strcmp(opti, valid_optis[i]))
      return 1;
      // snprintf(opti, 4, "-%s", valid_optis[i]);
  }
  return 0;
}

void update_env() {
  u8* old_path_env;
  old_path_env = getenv("OLDPATH");
  if (!old_path_env) {
    errExit("WTF? no OLDPATH");
  }
  setenv("PATH", old_path_env, 1);
}

/* Copy argv to cc_params, making the necessary edits. */
static void edit_param(u32 argc, char** argv) {
  u8* name;
  u8* OPTI_ENV;
  
  int debug = 0;
  u8* OPTI_FLAG = NULL;
  u8* APPEND_FLAG = NULL;
  cc_params = calloc(argc + 128, sizeof(u8*));
  name = strrchr(argv[0], '/');
  if (!name) name = argv[0]; else name++;

  wlog("Hijack compiler ** %s ** \n", name);
  cc_params[0] = name;

  /* parse settings from environment */
  if (getenv(WRAPPER_DEBUG))
    debug = 1;

  OPTI_ENV = getenv(WRAPPER_OPTI);
  if (OPTI_ENV) {
    assert(valid_opti(OPTI_ENV));
    OPTI_FLAG = calloc(8, sizeof(u8));
    bzero(OPTI_FLAG, 8);
    snprintf(OPTI_FLAG, 7, "-%s", OPTI_ENV);
  }

  while (--argc) {
    u8* cur = *(++argv);

    /* we don't like this option. */
    /* man gcc: -s  Remove all symbol table and relocation information from the executable. */
    if (!strcmp(cur, "-s")) continue;
    
    if (debug) {
      if (!strcmp(cur, "-g") || !strcmp(cur, "-ggdb")) {
        debug = 0;
      }
    }

    if (OPTI_FLAG) {
      if (valid_opti(cur)) {
        if (strcmp(cur, OPTI_FLAG))
          cur = OPTI_FLAG;
        OPTI_FLAG = NULL;
      }
    }
    
    // if (!strcmp(cur, "-DNDEBUG")) continue;
    cc_params[cc_par_cnt++] = cur;
  }

  if (debug)
    cc_params[cc_par_cnt++] = "-g";

  if (OPTI_FLAG)
    cc_params[cc_par_cnt++] = OPTI_FLAG;

  APPEND_FLAG = getenv(WRAPPER_APPEND);
  if (APPEND_FLAG) {
    char *token;
    const char *delim = " ";
    token = strtok(APPEND_FLAG, delim);
    while (token != NULL) {
      cc_params[cc_par_cnt++] = token;
      token = strtok(NULL, delim);
    }
    
  }

  cc_params[cc_par_cnt++] = 0;

  #ifdef WRAPPER_LOG
    wlog("cmd argv\n");
    for (u8** head=cc_params; *head ; head++) {
      wlog("%s\n", *head);
    }
  #endif

}
void help() {
    printf("Usage: bcsdcc [options] -- make or other command\n");
    printf("Options:\n");
    printf("  -g, --debug            Enable debug mode\n");
    printf("  -o, --optimization     Set optimization level (e.g., O0, O1, O2, O3, Os)\n");
    printf("  -a, --append           Other command options\n");
    printf("  -h, --help             Display this help message\n");
    printf("Example:\n");
    printf("  eg. bcsdcc -g -o O0 --append '-D MACRO' -- make\n");
}

int bcsdcc(int argc, char** argv) {
  int debug = 0;
  const char *append = NULL;
  const char* optimization = NULL;
  
  int opt;
  struct option long_options[] = {
    {"debug", no_argument, NULL, 'g'},
    {"optimization", required_argument, NULL, 'o'},
    {"append", required_argument, NULL, 'a'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}
  };

  int separator = 0;
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "--")) {
      separator = i;
      break;
    }
  }
  if (!separator || separator == argc-1) {
    help();
    exit(0);
  }

  while (1) {
    int option_index = 0;
    opt = getopt_long(separator, argv, "go:a:h", long_options, &option_index);

    if (opt == -1) {
      break;
    }

    switch (opt) {
      case 'g':
        debug = 1;
        printf("Debug mode enabled.\n");
        break;
      case 'o':
        optimization = optarg;
        if (!valid_opti(optimization)) {
          fprintf(stderr, "invalid optimization '%s'\n", optimization);
          help();
          exit(1);
        }
        printf("Optimization level set to: %s\n", optimization);
        break;
      case 'a':
        append = optarg;
        printf("Append `%s` to compile options.\n", append);
        break;
      case 'h':
      case '?':
        help();
        exit(0);
      default:
        help();
        exit(1);
    }
  }

  /* set WARPPER_DEBUG */
  if (debug)
    setenv(WRAPPER_DEBUG, "1", 1);

  /* set WARPPER_OPTI */
  if (optimization)
    setenv(WRAPPER_OPTI, optimization, 1);

  /* set WRAPPER_APPEND */
  if (append)
    setenv(WRAPPER_APPEND, append, 1);
  

  /* update PATH; export PATH=$(pwd)/bin${PATH:+:${PATH}} */
  const char *home = getenv("HOME");
  if (home == NULL) {
    fprintf(stderr, "Unable to get HOME environment variable.\n");
    return EXIT_FAILURE;
  }

  char file_path[MAX_LINE_LENGTH];
  snprintf(file_path, sizeof(file_path), "%s/.wrapper-gcc", home);
  FILE *file = fopen(file_path, "r");
  if (file == NULL) {
    perror("Error opening file");
    return EXIT_FAILURE;
  }
  
  char line[MAX_LINE_LENGTH];
  if (fgets(line, sizeof(line), file) == NULL) {
    perror("Error read file");
    fclose(file);
    return EXIT_FAILURE;  
  }
  fclose(file);
  line[strcspn(line, "\n")] = 0;
  
  char* path_env = getenv("PATH");
  if (path_env == NULL) {
    fprintf(stderr, "Unable to get PATH environment variable.\n");
    return EXIT_FAILURE;
  }
  setenv("OLDPATH", path_env, 1);

  int new_path_len = strlen(path_env) + strlen(line) + 5;
  char* new_path_env = malloc(new_path_len);
  snprintf(new_path_env, new_path_len, "%s/bin:%s", line, path_env);
  setenv("PATH", new_path_env, 1);

  /* execute new command */
  ++separator;
  execvp(argv[separator], &argv[separator]);

}

/* Main entry point */
int main(int argc, char** argv) {
  
  const char* self;
  self = strrchr(argv[0], '/');
  self = self ? self + 1 : argv[0];
  
  if (!strcmp(self, "bcsdcc")) {
    /* will never return */
    bcsdcc(argc, argv);
    errExit("WTF :( \n");
  }

  /* this will be called as cc/c++/gcc/g++/clang/clang++ */
  if (argc < 2) {
    errExit( 
      "This is the the wrapper of 'gcc, g++, clang, clang++'.\n"
      "It is used to modify the compilation options in a uniform way.\n"
      "Changes are specified by environment variables 'WRAPPER_DEBUG, WRAPPER_OPTI, WRAPPER_APPEND'"
      "\n"
    );
  }
  edit_param(argc, argv);
  update_env();
  execvp(cc_params[0], (char**)cc_params);
  
  errExit("Return from execvp.");
  return 0;
}
