#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <pwd.h>

int main (int argc, const char *argv[]) {
  const uid_t uid = getuid();
  struct passwd *pw_uid = getpwuid(uid);
  const char *name = pw_uid->pw_name;
  struct passwd *pw_nam = getpwnam(name);
  if (pw_uid->pw_gid == pw_nam->pw_gid) {
    printf("%lu\n", (unsigned long int) pw_uid->pw_gid);
    return EXIT_SUCCESS;
  } else {
    printf("%s\n", "Returns from getpwuid and getpwnam did not match!");
    printf("%s: %lu\n", "getpwuid", (unsigned long int) pw_uid->pw_gid);
    printf("%s: %lu\n", "getpwnam", (unsigned long int) pw_nam->pw_gid);
    return EXIT_FAILURE;
  }
}

