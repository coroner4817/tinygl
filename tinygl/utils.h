#pragma once
#include <iostream>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
std::string getExecutablePath()
{
  char buf [PATH_MAX];
  uint32_t bufsize = PATH_MAX;
  _NSGetExecutablePath(buf, &bufsize);
  buf[bufsize] = '\0';
  std::string ret = std::string(buf);
  auto i = ret.rfind('/');
  return ret.substr(0, i);
}
#else
std::string getExecutablePath()
{
  char result[ PATH_MAX ];
  ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
  return dirname(result);
}
#endif