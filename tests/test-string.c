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
  if (strcmp(pw_uid->pw_dir,pw_nam->pw_dir) == 0) {
    printf("%s\n", pw_uid->pw_dir);
    return EXIT_SUCCESS;
  } else {
    printf("%s\n", "Returns from getpwuid and getpwnam did not match!");
    printf("%s: %s\n", "getpwuid", pw_uid->pw_dir);
    printf("%s: %s\n", "getpwnam", pw_nam->pw_dir);
    return EXIT_FAILURE;
  }
}
