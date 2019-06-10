#ifndef BTCLITE_FS_H
#define BTCLITE_FS_H


#if __GNUC__ >= 8
#include <filesystem>
namespace fs = std::filesystem;
#else 
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif


#endif // BTCLITE_FS_H
