#pragma once
#include <string>
#include "ReadPointTool.h"


enum enLocation
{
  BOTTOM = 0,
  TOP = 1,
};

enum DXF_VERSION 
{
	AC1015 = 0,
    AC1018 = 1,
    AC1021 = 2,
    AC1024 = 3,
    AC1027 = 4,
};


class WriteDxf
{
public:
  WriteDxf(const std::string& file);
  WriteDxf();

public:
  int WriteToDxf(const std::string& outFilePath, enLocation location);
    int WriteToDxf(const std::string &inputString, const std::string &outFilePath, enLocation location, DXF_VERSION version = AC1015);

 private:
  void CollectLineData(std::vector<DL_LineData> &out_data, enLocation location);
  std::string ComposeFilePath(const std::string &out_directory, enLocation location,bool origin = false);
  private:
  std::string  m_OriginTextFilePath;
  ReadPointTool m_tool;
};

