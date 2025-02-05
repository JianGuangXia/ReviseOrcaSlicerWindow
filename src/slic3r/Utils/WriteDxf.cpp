#include "WriteDxf.h"
//#include "dxf/dl_dxf.h"
#include "windows.h"
#include "dx_iface.h"
WriteDxf::WriteDxf(const std::string& file)
: m_OriginTextFilePath(file), m_tool(file)
{

}

WriteDxf::WriteDxf() {

}

  void WriteDxf::CollectLineData(std::vector<DL_LineData> &out_data, enLocation location) 
  {
      out_data.clear();
      std::map<std::string, Model_Container> *         data          = m_tool.GetMapData();
      std::map<std::string, Model_Container>::iterator LabelIterInfo = data->begin();
      for (; LabelIterInfo != data->end(); ++LabelIterInfo) {
          if (location == BOTTOM && LabelIterInfo->first.find("bottom") == std::string::npos) {
              continue;
          }

          if (location == TOP && LabelIterInfo->first.find("top") == std::string::npos) {
              continue;
          }

          Model_Container::iterator ModeIteInfo = LabelIterInfo->second.begin();
          for (; ModeIteInfo != LabelIterInfo->second.end(); ++ModeIteInfo) {
              LineGroup_Container::iterator LineGroupIteInfo = ModeIteInfo->begin();
              for (; LineGroupIteInfo != ModeIteInfo->end(); ++LineGroupIteInfo) {
                  Line_Container::iterator LineIteInfo = LineGroupIteInfo->begin();
                  for (; LineIteInfo != LineGroupIteInfo->end(); ++LineIteInfo) {
                      out_data.push_back(*LineIteInfo);
                  }
              }
          }
      }
  }

 int WriteDxf::WriteToDxf(const std::string &inputString, const std::string &outFilePath, enLocation location, DXF_VERSION version /*= AC1015*/) 
 {
     int iRetCode = 0;
     iRetCode     = m_tool.ParseString(inputString);
     if (0 != iRetCode) {
         return iRetCode;
     }

     std::vector<DL_LineData> out_data;
     CollectLineData(out_data, location);
     if (out_data.empty()) {
         return 0;
     }

     std::string       file_path = ComposeFilePath(outFilePath,location);

     dx_iface *iface = new dx_iface();
     if (!iface) 
     {
         return 0;
     }
     
     dx_data data;
     for (size_t i = 0; i < out_data.size(); i++) {
         DRW_Line *line = new DRW_Line();
         if (!line) 
         {
             continue;
         }

         const DL_LineData &line_data = out_data[i];
         line->basePoint.x            = line_data.x1;
         line->basePoint.y            = line_data.y1;
         line->basePoint.z            = 0;

         line->secPoint.x  = line_data.x2;
         line->secPoint.y  = line_data.y2;
         line->secPoint.z  = 0;
         data.mBlock->ent.insert(data.mBlock->ent.end(), line);  
     }
     
     DRW::Version out_version = DRW::AC1015;
     if (version == AC1018) {
         out_version = DRW::AC1018;
     } else if (version == AC1021) {
         out_version = DRW::AC1021;
     } else if (version == AC1024) {
         out_version = DRW::AC1024;
     } else if (version == AC1027) {
         out_version = DRW::AC1027;
     }

     iface->fileExport(file_path, out_version, false, &data);
     delete iface;

     /*
     DL_Dxf            dxf;
     DL_Dxf *          m_dxf         = &dxf;
     FILE *            pfd           = fopen(file_path.c_str(), "w+");
     DL_Codes::version exportVersion = DL_Codes::AC1015;
     DL_WriterA *      dw            = m_dxf->out(file_path.c_str(), exportVersion);
     if (dw == NULL) {
         printf("Cannot open file 'myfile.m_dxf' \ for writing.");
         return 0;
     }

     // 1  标题段
     m_dxf->writeHeader(*dw);
     dw->sectionEnd();

     // 2  表段
     dw->sectionTables();
     m_dxf->writeVPort(*dw);

     // 2.1  线型表（Linetype）
     dw->tableLinetypes(3);
     m_dxf->writeLinetype(*dw, DL_LinetypeData("BYBLOCK", "BYBLOCK", 0, 0, 0.0));
     m_dxf->writeLinetype(*dw, DL_LinetypeData("BYLAYER", "BYLAYER", 0, 0, 0.0));
     m_dxf->writeLinetype(*dw, DL_LinetypeData("CONTINUOUS", "Continuous", 0, 0, 0.0));
     dw->tableEnd();

     // 2.2  图层表（Layer）
     int numberOfLayers = 3; //层数3
     dw->tableLayers(numberOfLayers);
     m_dxf->writeLayer(*dw, DL_LayerData("0", 0), DL_Attributes(std::string(""), DL_Codes::black, 100, "CONTINUOUS", 1.0));
     m_dxf->writeLayer(*dw, DL_LayerData("mainlayer", 0), DL_Attributes(std::string(""), DL_Codes::black, 100, "CONTINUOUS", 1.0));
     m_dxf->writeLayer(*dw, DL_LayerData("anotherlayer", 0), DL_Attributes(std::string(""), DL_Codes::blue, 100, "CONTINUOUS", 1.0));
     dw->tableEnd();

     // 2.3  字样表（Style）
     dw->tableStyle(1);
     m_dxf->writeStyle(*dw, DL_StyleData("standard", 0, 2.5, 1.0, 0.0, 0, 2.5, "txt", ""));
     dw->tableEnd();

     // 2.4  视图表（View）
     m_dxf->writeView(*dw);
     m_dxf->writeUcs(*dw);
     dw->tableAppid(1);
     m_dxf->writeAppid(*dw, "ACAD");
     dw->tableEnd();

     // 2.5  DL_VERSION_R13中需要伪造dimstyle和blockrecord两部分，以使AutoCAD能够读取该文件。
     m_dxf->writeDimStyle(*dw, 1, 1, 1, 1, 1);
     m_dxf->writeBlockRecord(*dw);
     dw->tableEnd();
     dw->sectionEnd();
 
     // 3  块段
      DL_Attributes typetemp1("mainlayer", 256, -1, "BYLAYER", 1.0);    //属性
      DL_Attributes typetemp2("anotherlayer", 256, -1, "BYLAYER", 1.0); //属性
      dw->sectionBlocks();
      m_dxf->writeBlock(*dw, DL_BlockData("*Model_Space", 0, 0.0, 0.0, 0.0));
      m_dxf->writeEndBlock(*dw, "*Model_Space");
      m_dxf->writeBlock(*dw, DL_BlockData("*Paper_Space", 0, 0.0, 0.0, 0.0));
      m_dxf->writeEndBlock(*dw, "*Paper_Space");
      m_dxf->writeBlock(*dw, DL_BlockData("*Paper_Space0", 0, 0.0, 0.0, 0.0));
      m_dxf->writeEndBlock(*dw, "*Paper_Space0");

      //m_dxf->writeBlock(*dw, DL_BlockData("myblock1", 0, 0.0, 0.0, 0.0));
      	// ...
      	// write block entities e.g. with m_dxf->writeLine(), ..
      	// ...
      //m_dxf->writeEndBlock(*dw, "myblock1");
      dw->sectionEnd();

  
     // 4  实体段

     dw->sectionEntities();
     for (size_t i = 0; i < out_data.size(); i++)
     {
         m_dxf->writeLine(*dw, out_data[i], typetemp1);
     }
     dw->sectionEnd();
  
     // 5  文件结束段
     m_dxf->writeObjects(*dw);
     m_dxf->writeObjectsEnd(*dw);

     dw->dxfEOF();
     dw->close();
     delete dw;
     // delete m_dxf;
     dw    = nullptr;
     m_dxf = nullptr;
     */
 }

int WriteDxf::WriteToDxf(const std::string& outFilePath, enLocation location)
{
  int iRetCode = 0;
  iRetCode = m_tool .ParseFile();
  if (0 != iRetCode)
  {
    return iRetCode;
  }

  // begin write
  /*
  DL_Dxf dxf;
  DL_Codes::version exportVersion = DL_Codes::AC1015;
  DL_WriterA* dw = dxf.out(outFilePath.c_str(), exportVersion);
  if (dw == NULL)
  {
    printf("Cannot open file 'myfile.dxf' \
        for writing.");
    return -1;
  }

  // write something
  dxf.writeHeader(*dw);
  dw->sectionEnd();

  // write line
  DL_Attributes typetemp1("main layer", 256, -1, "BYLAYER", 1.0);
  dw->sectionEntities();
  std::map<std::string, Model_Container>* data =  m_tool.GetMapData();
  std::map<std::string, Model_Container>::iterator LabelIterInfo = data->begin();
  for (;LabelIterInfo != data->end(); ++LabelIterInfo)
  {
    if (location == BOTTOM && LabelIterInfo->first.find("bottom") == std::string::npos)
    {
      continue;
    }

    if (location == TOP && LabelIterInfo->first.find("top") == std::string::npos)
    {
      continue;
    }
    
    Model_Container::iterator ModeIteInfo = LabelIterInfo->second.begin();
    for (;ModeIteInfo != LabelIterInfo->second.end(); ++ ModeIteInfo)
    {
      LineGroup_Container::iterator LineGroupIteInfo = ModeIteInfo->begin();
      for (; LineGroupIteInfo != ModeIteInfo->end(); ++ LineGroupIteInfo)
      {
        Line_Container::iterator LineIteInfo = LineGroupIteInfo->begin();
        for (; LineIteInfo != LineGroupIteInfo->end(); ++LineIteInfo)
        {
          DL_LineData& line_info = *LineIteInfo;
          dxf.writeLine(*dw,line_info, typetemp1);
        }

      }
    }
  }
  dw->sectionEnd();
  dxf.writeObjects(*dw);
  dxf.writeObjectsEnd(*dw);
 
  // close file
  dw->dxfEOF();
  dw->close();
  delete dw;
  */
}

std::string WriteDxf::ComposeFilePath(const std::string &out_directory, enLocation location) 
{ 
    std::string strFilePath = out_directory + "\\";

    SYSTEMTIME stSYSTEMTIME = {0};
    GetSystemTime(&stSYSTEMTIME);
    char szbuffer[64] = {0};
    snprintf(szbuffer, sizeof(szbuffer) - 1, "%4ld%02d%02d%02d%02d%02d", stSYSTEMTIME.wYear, stSYSTEMTIME.wMonth, stSYSTEMTIME.wDay,
             stSYSTEMTIME.wHour + 8, stSYSTEMTIME.wMinute, stSYSTEMTIME.wSecond);
    std::string file_name = szbuffer;
    if (location == BOTTOM)
    {
        if (m_tool.HasBottomInnerOutLine() && m_tool.HasBottomOuterOutLine())
        {
            file_name.append("_bottom-outline_inner&outer.dxf ");
        } 
        else if (m_tool.HasBottomInnerOutLine() && !m_tool.HasBottomOuterOutLine())
        {
            file_name.append("_bottom-outline_inner.dxf ");
        } 
        else if (!m_tool.HasBottomInnerOutLine() && m_tool.HasBottomOuterOutLine())
        {
            file_name.append("_bottom-outline_outer.dxf ");
        }
    } else if (location == TOP) 
    {
        if (m_tool.HasTopInnerOutLine() && m_tool.HasTopOuterOutLine()) 
        {
            file_name.append("_top-outline_inner&outer.dxf ");
        } 
        else if (m_tool.HasTopInnerOutLine() && !m_tool.HasTopOuterOutLine()) 
        {
            file_name.append("_top-outline_inner.dxf ");
        } 
        else if (!m_tool.HasTopInnerOutLine() && m_tool.HasTopOuterOutLine()) 
        {
            file_name.append("_top-outline_outer.dxf ");
        }
    }

    return strFilePath + file_name;
}
