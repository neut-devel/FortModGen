#include "testmod.h"

extern "C" {
void cppwrite();
void cppsay();
void cwrite();
void csay();
void fortwrite();
void fortsay();
}

int main() {
  cppsay();
  cppwrite();
  cppsay();

  fortsay();
  fortwrite();
  fortsay();
  cppsay();

  csay();
  cwrite();
  fortsay();
  cppsay();

}