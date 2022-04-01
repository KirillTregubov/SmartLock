/**
 * @file helpers.cpp
 * @author Kirill Tregubov (KirillTregubov)
 * @author Philip Cai (Gadnalf)
 * @copyright Copyright (c) 2022 Kirill Tregubov & Philip Cai
 *
 * @brief This module contains helpers used by the program.
 * @bug No known bugs.
 */
#include "helpers.hpp"

char *strupr(char *str) {
  unsigned char *temp = (unsigned char *)str;
  while (*temp) {
    *temp = toupper((unsigned char)*temp);
    temp++;
  }

  return str;
}
