#include "vstream/vstream.h"

#include <string.h>

int main(int argc, char const *argv[])
{
  (void)argc;
  (void)argv;

  return strcmp(exported_function(), "vstream") == 0 ? 0 : 1;
}
