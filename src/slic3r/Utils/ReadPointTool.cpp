#include "ReadPointTool.h"
#include  "cJSON.h"
#include <iostream>
#include <sstream>

typedef struct pointCoordinate
{
  double x;
  double y;
} pointCoordinate;


ReadPointTool::ReadPointTool(const std::string& file_path)
{
  m_file_path = file_path;
}

 ReadPointTool::ReadPointTool() 
 {}
ReadPointTool::~ReadPointTool()
{
   
}

  int ReadPointTool::ParseString(const std::string &input_json) 
  {
      ResetOutLineFlag();
      // read buffer
      cJSON *root = cJSON_Parse(input_json.c_str());
      if (!root) {
          return -1;
      }

      cJSON *top_inner_arr    = cJSON_GetObjectItem(root, "top_inner");
      cJSON *top_outer_arr    = cJSON_GetObjectItem(root, "top_outer");
      cJSON *bottom_inner_arr = cJSON_GetObjectItem(root, "bottom_inner");
      cJSON *bottom_outer_arr = cJSON_GetObjectItem(root, "bottom_outer");

      ParseArray(top_inner_arr, "top_inner");
      ParseArray(top_outer_arr, "top_outer");
      ParseArray(bottom_inner_arr, "bottom_inner");
      ParseArray(bottom_outer_arr, "bottom_outer");
      return 0;
  }

int ReadPointTool::ParseFile()
{
   FILE* file_hanle = fopen(m_file_path.c_str(),"r");
   if (!file_hanle)
   {
      return -1;
   }

   struct stat st_stat = {0};
   stat(m_file_path.c_str(),&st_stat);
   if (st_stat.st_size <= 0)
   {
     return -1;
   }

   char* buffer = new char[st_stat.st_size + 1];
   memset(buffer,0, st_stat.st_size + 1);
   int size = fread(buffer,1,st_stat.st_size,file_hanle);
   if (size  <= 0)
   {
     return -1;
   }
  
   fclose(file_hanle);

// read buffer
   cJSON* root = cJSON_Parse(buffer);
   delete [] buffer;
   if (!root)
   {
     return -1;
   }

   cJSON* top_inner_arr = cJSON_GetObjectItem(root, "top_inner");
   cJSON* top_outer_arr = cJSON_GetObjectItem(root, "top_outer");
   cJSON* bottom_inner_arr = cJSON_GetObjectItem(root, "bottom_inner");
   cJSON* bottom_outer_arr = cJSON_GetObjectItem(root, "bottom_outer") ;
  
   ParseArray(top_inner_arr,"top_inner");
   ParseArray(top_outer_arr, "top_outer");
   ParseArray(bottom_inner_arr, "bottom_inner");
   ParseArray(bottom_outer_arr, "bottom_outer");
   return 0;
}

std::map<std::string, Model_Container>* ReadPointTool::GetMapData()
{
  return &m_mapData;
}

double ToDouble(const char *InputString_) 
{ 
    //atof 会根据本地化语言设置,导致字符串转double型数值产生bug
    std::istringstream istr(InputString_);
    istr.imbue(std::locale("C"));
    double value = 0.0;
    istr >> value;
    return value;
}

pointCoordinate GetPointInfor(cJSON* poInt)
{
  pointCoordinate stpointCoordinate = { 0 };
  cJSON* first_item = cJSON_GetArrayItem(poInt, 0);
  cJSON* second_item = cJSON_GetArrayItem(poInt, 1);
  if (first_item && first_item->type == cJSON_String)
  {
      stpointCoordinate.x = ToDouble(first_item->valuestring);
  }

  if (second_item && second_item->type == cJSON_String)
  {
      stpointCoordinate.y = ToDouble(second_item->valuestring);
  }
  return stpointCoordinate;
}


void ReadPointTool::ParseArray( cJSON* array_node, const char* name)
{
    Model_Container & Models = m_mapData[name];
    int iCount = cJSON_GetArraySize(array_node);
    for (int i = 0; i < iCount; ++i)  
    {
      cJSON* ModelNode = cJSON_GetArrayItem(array_node, i);
      int iLineCount = cJSON_GetArraySize(ModelNode);
      if (ModelNode->type != cJSON_Array || iLineCount <= 0)
      {
          continue;
      }
    
      LineGroup_Container LineGroup;
      for(int index = 0; index < iLineCount; ++index)
      {
        cJSON* LineNode = cJSON_GetArrayItem(ModelNode, index);
        int iPointCount = cJSON_GetArraySize(LineNode);
        if (LineNode->type != cJSON_Array || iPointCount <= 2)
        {
          continue;
        }
     
        Line_Container Lines;
        std::vector<pointCoordinate> vecPoints;
        for (int j = 0; j < iPointCount; ++j)
        {
          cJSON* PointNode = cJSON_GetArrayItem(LineNode, j);
          int iPointElmentCnt = cJSON_GetArraySize(PointNode);
          if (PointNode->type != cJSON_Array || iPointElmentCnt != 2)
          {
            continue;
          }

          pointCoordinate point = GetPointInfor(PointNode);
          vecPoints.push_back(point);
        } 

        if (vecPoints.size() <= 2)
        {
          continue;
        } 

        for (int index = 0; index + 1 < vecPoints.size(); ++index)
        {
          DL_LineData stDL_LineData;
          stDL_LineData.x1 = vecPoints[index].x;
          stDL_LineData.y1 = vecPoints[index].y;

          stDL_LineData.x2 = vecPoints[index + 1].x;
          stDL_LineData.y2 = vecPoints[index + 1].y;

          Lines.push_back(stDL_LineData);
          SetOutLineFlag(name);
        }

        if (!Lines.empty())  LineGroup.push_back(Lines);
      }

     if (!LineGroup.empty())  Models.push_back(LineGroup);
    }
}



  void ReadPointTool::ResetOutLineFlag() 
  {
      m_top_inner = false;
      m_top_outer = false;
      m_bottom_inner = false;
      m_bottom_outer = false;
  }


  bool ReadPointTool::HasTopInnerOutLine() 
  { 
      return m_top_inner;
  }

  bool ReadPointTool::HasTopOuterOutLine() 
  { 
      return m_top_outer;
  }
  bool ReadPointTool::HasBottomInnerOutLine() 
  { 
      return m_bottom_inner;
  }
  bool ReadPointTool::HasBottomOuterOutLine() 
  { 
      return m_bottom_outer;
  }

  void ReadPointTool::SetOutLineFlag(const std::string &name) 
  {
      if (0 == strcmp(name.c_str(), "top_inner")) 
      {
          m_top_inner = true;
      } 
      else if (0 == strcmp(name.c_str(), "top_outer")) 
      {
          m_top_outer = true;
      } 
      else if (0 == strcmp(name.c_str(), "bottom_inner"))
      {
          m_bottom_inner = true;
      } 
      else if (0 == strcmp(name.c_str(), "bottom_outer")) 
      {
          m_bottom_outer = true;
      }
  }


