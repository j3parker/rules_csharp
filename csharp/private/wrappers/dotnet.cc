#include <iostream>
#include <sstream>
#include <string>

#ifdef _WIN32
#include <errno.h>
#include <process.h>
#include <windows.h>
#else  // not _WIN32
#include <stdlib.h>
#include <unistd.h>
#endif  // _WIN32

#include "tools/cpp/runfiles/runfiles.h"

using bazel::tools::cpp::runfiles::Runfiles;

std::string evprintf(std::string name, std::string path) {
  std::stringstream ss;
  ss << name << "=" << path;
  return ss.str();
}

int main(int argc, char** argv) {
  std::string error;

  auto runfiles = Runfiles::Create(argv[0], &error);

  if (runfiles == nullptr) {
    std::cerr << "Couldn't load runfiles: " << error << std::endl;
    return 101;
  }

  auto dotnet = runfiles->Rlocation("{DotnetExe}");
  if (dotnet.empty()) {
    std::cerr << "Couldn't find the .NET runtime" << std::endl;
    return 404;
  }

  // Get the name of the directory containing dotnet.exe
  auto dotnetDir = dotnet.substr(0, dotnet.find_last_of("/\\"));

  /*
  dotnet requires these environment variables to be set.
  */
  std::vector<std::string> envvars = {
      evprintf("HOME", dotnetDir),
      evprintf("DOTNET_CLI_HOME", dotnetDir),
      evprintf("APPDATA", dotnetDir),
      evprintf("PROGRAMFILES", dotnetDir),
      evprintf("USERPROFILE", dotnetDir),
      evprintf("DOTNET_CLI_TELEMETRY_OPTOUT", "1"),  // disable telemetry
  };

  // dotnet wants this to either be dotnet or dotnet.exe but doesn't have a
  // preference otherwise.
  auto dotnet_argv = new char*[argc];
  dotnet_argv[0] = (char*)"dotnet";
  for (int i = 1; i < argc; i++) {
    dotnet_argv[i] = argv[i];
  }
  dotnet_argv[argc] = nullptr;

#ifdef _WIN32
  // _spawnve has a limit on the size of the environment variables
  // passed to the process. So here we will set the environment
  // variables for this process, and the spawned instance will inherit them
  for (int i = 1; i < envvars.size(); i++) {
    putenv(envvars[i].c_str());
  }

  // run `dotnet.exe` and wait for it to complete
  // the output from this cmd will be emitted to stdout
  auto result = _spawnv(_P_WAIT, dotnet.c_str(), dotnet_argv);
#else
  auto envc = envvars.size();
  auto envp = new char*[envc + 1];
  for (uint i = 0; i < envc; i++) {
    envp[i] = const_cast<char*>(&envvars[i][0]);
  }
  envp[envc] = nullptr;

  // run `dotnet.exe` and wait for it to complete
  // the output from this cmd will be emitted to stdout
  auto result = execve(dotnet.c_str(), const_cast<char**>(dotnet_argv), envp);
#endif  // _WIN32
  if (result != 0) {
    std::cout << "dotnet failed: " << errno << std::endl;
    return -1;
  }

  return result;
}