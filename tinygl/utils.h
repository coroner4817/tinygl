#pragma once
#include <iostream>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>

std::string getExecutablePath()
{
  char result[ PATH_MAX ];
  ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
  return dirname(result);
}