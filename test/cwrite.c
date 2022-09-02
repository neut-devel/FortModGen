#include "testmod.h"

#include <stdio.h>
#include <string.h>

void cwrite() {

  struct testtype_t *myinst = alloc_testtype();
  copy_testtype(myinst);

  printf(">>>>>>>>>>>>>>\n");
  printf("Writing from C\n");
  printf("<<<<<<<<<<<<<<\n");

  char const *tmpstr = "string from C";
  memset(myinst->fstr, '\0', 100);
  memcpy(myinst->fstr, tmpstr, strlen(tmpstr) + 1);
  myinst->ffloat = 5.5555555;
  myinst->fdouble = 6.6666666666666;

  for (int i = 0; i < 5; ++i) {
    myinst->ffloata[i] = 30 - i;
  }

  int ctr = 30;
  for (int i = 0; i < intpar; ++i) {
    myinst->ffloatapar[i] = ctr;
    ctr--;
  }

  ctr = 300;
  for (int j = 0; j < 3; ++j) {
    for (int i = 0; i < 5; ++i) {
      myinst->ffloat2a[i][j] = ctr;
      ctr--;
    }
  }

  ctr = 600;
  for (int j = 0; j < intpar; ++j) {
    for (int i = 0; i < 5; ++i) {
      myinst->ffloat2apar[i][j] = ctr;
      ctr--;
    }
  }

  update_testtype(myinst);
  free_testtype(myinst);
}

void csay() {

  printf(">>>>>>>>>>>>>\n");
  printf("Saying from C\n");
  printf("-------------\n");

  struct testtype_t *myinst = alloc_testtype();
  copy_testtype(myinst);

  printf("myinst->fstr: %s\n", myinst->fstr);
  printf("myinst->ffloat: %g\n", myinst->ffloat);
  printf("myinst->fdouble: %g\n", myinst->fdouble);

  printf("myinst->ffloata: [ ");
  for (int i = 0; i < 5; ++i) {
    printf("%.0f%s", myinst->ffloata[i], (i != 4 ? ", " : " "));
  }
  printf("]\n");

  printf("myinst->ffloatapar: [ ");
  for (int i = 0; i < intpar; ++i) {
    printf("%.0f%s", myinst->ffloatapar[i], (i != (intpar - 1) ? ", " : " "));
  }
  printf("]\n");

  printf("myinst->ffloat2a: [\n");
  for (int j = 0; j < 3; ++j) {
    printf("    [ ");
    for (int i = 0; i < 5; ++i) {
      printf("%.0f%s", myinst->ffloat2a[i][j], (i != (5 - 1) ? ", " : " "));
    }
    printf("]\n");
  }
  printf("]\n");

  printf("myinst->ffloat2apar: [\n");
  for (int j = 0; j < intpar; ++j) {
    printf("    [ ");
    for (int i = 0; i < 5; ++i) {
      printf("%.0f%s", myinst->ffloat2apar[i][j], (i != (5 - 1) ? ", " : " "));
    }
    printf("]\n");
  }
  printf("]\n");

  printf("<<<<<<<<<<<<<\n");
}
