#pragma once
//#include "dxf/dl_entities.h"
#include <map>
#include <vector>
#include <string>
#include "cJSON.h"

typedef struct ST_LineData
{
    int x1, y1, x2, y2;
	 ST_LineData() { 
		 x1 = 0;
         y1 = 0;
         x2 = 0;
         y2 = 0;
	 }

} DL_LineData;

typedef std::vector<DL_LineData> Line_Container;
typedef std::vector<Line_Container> LineGroup_Container;
typedef std::vector<LineGroup_Container> Model_Container;

class ReadPointTool
{
public:
  ReadPointTool(const std::string& file_path);
  ReadPointTool();
 ~ReadPointTool();

public:
  int ParseFile();
  int ParseString(const std::string& input_json);

  std::map<std::string, Model_Container>* GetMapData();

  bool HasTopInnerOutLine();
  bool HasTopOuterOutLine();
  bool HasBottomInnerOutLine();
  bool HasBottomOuterOutLine();

  private:
  void ParseArray( cJSON* array_node, const char* name);

  void ResetOutLineFlag();

  void SetOutLineFlag(const std::string& name);

  private:
  std::map<std::string, Model_Container>  m_mapData;
  std::string  m_file_path;
  bool                                    m_top_inner;
  bool                                    m_top_outer;
  bool                                    m_bottom_inner;
  bool                                    m_bottom_outer;
};

