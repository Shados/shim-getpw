#define _GNU_SOURCE

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <pwd.h>

// Helper to convert from environment variable charstrings to uid/gids
intmax_t
str_to_id(const char *id_str)
{
  char *endptr = 0;
  errno = 0;
  const intmax_t id = strtoimax(id_str, &endptr, 10);

  // TODO log when errors generated?
  if ((errno == ERANGE && (id == INTMAX_MAX || id == INTMAX_MIN)) 
      || (errno != 0 && id == 0)) {
    // Overflow or underflow
    return -1LL;
  };
  if (endptr == id_str) {
    // No digits found in the string
    return -1LL;
  }
  if (id < 0) {
    // Valid UIDs and GIDs are positive integers
    return -1LL;
  }

  return id;
}

// Modify a passwd struct using values from environment variables, if present
struct passwd *
shimpw(struct passwd *pw)
{
  if (pw) {
    char *user = getenv("SHIM_USER");
    char *gecos = getenv("SHIM_GECOS");
    char *home = getenv("SHIM_HOME");
    char *shell = getenv("SHIM_SHELL");

    char *uid_str = getenv("SHIM_UID");
    intmax_t pw_uid = -1;
    char *gid_str = getenv("SHIM_GID");
    intmax_t pw_gid = -1;

    // Simple char* assignments
    if (user) { pw->pw_name = user; }
    if (gecos) { pw->pw_gecos = gecos; }
    if (home) { pw->pw_dir = home; }
    if (shell) { pw->pw_shell = shell; }

    // Assignments needing char* -> uid_t conversion
    if (uid_str) {
      pw_uid = str_to_id(uid_str);
      // The maximum uid_t/gid_t value is not a valid ID, because it's used as a
      // return code to communicate an error/unknown UID by some functions
      if (pw_uid >= 0 && pw_uid < (~(uid_t)0)) {
        pw->pw_uid = (uid_t) pw_uid;
      }
    }
    if (gid_str) {
      pw_gid = str_to_id(gid_str);
      if (pw_gid >= 0 && pw_gid < (~(gid_t)0)) {
        pw->pw_gid = (gid_t) pw_gid;
      }
    }
  }
  return pw;
}


// Wrappers to get at the functions being shimmed
typedef struct passwd *
(*real_getpwuid_t) (uid_t);

struct passwd *
real_getpwuid(uid_t uid)
{
  return ((real_getpwuid_t)dlsym(RTLD_NEXT, "getpwuid"))(uid);
}

typedef struct passwd *
(*real_getpwnam_t) (const char *name);

struct passwd *
real_getpwnam(const char *name)
{
  return ((real_getpwnam_t)dlsym(RTLD_NEXT, "getpwnam"))(name);
}


// The shim functions
struct passwd *
getpwuid(uid_t uid)
{
  struct passwd *pw = real_getpwuid(uid);
  return shimpw(pw);
}
struct passwd *
getpwnam(const char *name)
{
  struct passwd *pw = real_getpwnam(name);
  return shimpw(pw);
}
