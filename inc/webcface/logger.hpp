#pragma once
#include <string>
namespace WebCFace
{
inline namespace Logger
{
void initStdLogger();
void appendLogLine(int level, std::string text);
}  // namespace Logger
}  // namespace WebCFace
