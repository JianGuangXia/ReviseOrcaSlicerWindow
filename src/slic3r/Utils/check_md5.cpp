// TestMd5.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include "boost\filesystem.hpp"
#include "boost\property_tree\ptree.hpp"
#include "boost\property_tree\json_parser.hpp"
#include "openssl\md5.h"
#include "windows.h"
#include "check_md5.h"

//#define CHECK

void md5_hex2str(unsigned char *md5, char *out)
{
    char *pdest_buffer = out;
    for (int index = 0; index < MD5_DIGEST_LENGTH; index++) {
        sprintf(pdest_buffer + index * 2, "%.2x", md5[index]);
    }
}

std::string ans_to_utf8(std::string sreAnsi)
{
    UINT   nLen        = MultiByteToWideChar(936, NULL, sreAnsi.c_str(), -1, NULL, NULL);
    WCHAR *wszBuffer   = new WCHAR[nLen + 1];
    nLen               = MultiByteToWideChar(936, NULL, sreAnsi.c_str(), -1, wszBuffer, nLen);
    wszBuffer[nLen]    = 0;
    nLen               = WideCharToMultiByte(CP_UTF8, NULL, wszBuffer, -1, NULL, NULL, NULL, NULL);
    CHAR *szBuffer     = new CHAR[nLen + 1];
    nLen               = WideCharToMultiByte(CP_UTF8, NULL, wszBuffer, -1, szBuffer, nLen, NULL, NULL);
    szBuffer[nLen]     = 0;
    std::string result = szBuffer;
    delete[] szBuffer;
    delete[] wszBuffer;
    return result;
}

std::string caculte_md5(const boost::filesystem::path &file_path)
{
    std::ifstream ifs;
    ifs.open(file_path.string().c_str(), std::ios::binary);
    if (!ifs) {
        return "";
    }

    unsigned char buffer[MD5_DIGEST_LENGTH] = {0};
    ifs.seekg(0, std::ios::end);
    int   iLength     = ifs.tellg();
    char *file_buffer = new char[iLength];
    ifs.seekg(0, std::ios::beg);
    ifs.read(file_buffer, iLength);
    ifs.close();
    MD5(reinterpret_cast<const unsigned char *>(file_buffer), iLength, buffer);
    delete[] file_buffer;

    char str_buffer[MD5_DIGEST_LENGTH * 2 + 1] = {0};
    md5_hex2str(buffer, str_buffer);
    std::string origin_md5 = str_buffer;
    return origin_md5;
}

void collect_file_info(const boost::filesystem::path &                       top_directory_path,
                       const boost::filesystem::path &                       path,
                       std::map<const boost::filesystem::path, std::string> &continer)
{
    for (const auto &iter : boost::filesystem::directory_iterator(path)) {
        if (boost::filesystem::is_directory(iter.path())) {
            collect_file_info(top_directory_path, iter.path(), continer);
        }

        if (iter.path().filename() == "md5.dat")
            continue;
        std::string result = caculte_md5(iter.path());
        if (!result.empty()) {
            boost::filesystem::path relative_path = boost::filesystem::relative(iter.path(), top_directory_path);
            continer[relative_path]               = result;
        }
    }
}

void parse_json(std::map<const boost::filesystem::path, std::string> &info_continer, boost::property_tree::ptree &root)
{
    boost::property_tree::ptree ptlist = root.get_child("list");
    for (auto it = ptlist.begin(); it != ptlist.end(); it++) {
        boost::property_tree::ptree list_obj = it->second;
        boost::property_tree::ptree path_obj = list_obj.get_child("path");
        boost::property_tree::ptree md5_obj  = list_obj.get_child("md5");
        info_continer[path_obj.data()]       = md5_obj.data();
    }
}

bool check_impl(std::map<const boost::filesystem::path, std::string> &info_continer, 
                const boost::filesystem::path &file_path,
                const boost::filesystem::path &top_directory)
{
    bool result = true;
    boost::filesystem::path relative_path = boost::filesystem::relative(file_path, top_directory);
    if (info_continer.find(relative_path) == info_continer.end()) {
        result = false;
        printf("excess file:%s\n", relative_path.string().c_str());
        return result;
    }

    std::string old_md5 = info_continer[relative_path];
    std::string now_md5 = caculte_md5(file_path);
    if (old_md5 != now_md5) {
        result = false;
        printf("%s has been revise\n", file_path.string().c_str());
        return result;
    }

    info_continer.erase(relative_path);
    return result;
}

bool recursive_check(std::map<const boost::filesystem::path, std::string> &info_continer, 
                     const boost::filesystem::path & file_path,
                     const boost::filesystem::path & top_directory)
{
    bool result = true;
    for (const auto &iter : boost::filesystem::directory_iterator(file_path)) {
        if (boost::filesystem::is_directory(iter.path())) {
            result = recursive_check(info_continer, iter.path(), top_directory);
            if (!result)
                break;
            else
                continue;
        }

        if (iter.path().filename() == "md5.dat")
            continue;
        result = check_impl(info_continer, iter.path(), top_directory);
        if (!result)
            break;
    }
    return result;
}

bool check_file_md5(const boost::filesystem::path& directory)
{
    boost::filesystem::path md5_path = directory / boost::filesystem::path("md5.dat");

    if (!boost::filesystem::exists(md5_path)) {
        printf("fail to find md5.dat\n");
        return false;
    }

    std::ifstream ifs;
    ifs.open(md5_path.string().c_str(), std::ios::binary);
    if (!ifs) {
        printf("fail to open md5.dat\n");
        return false;
    }

    // read md5.dat json file
    std::map<const boost::filesystem::path, std::string> info_continer;
    boost::property_tree::ptree                          root;
    boost::property_tree::json_parser::read_json(ifs, root);
    parse_json(info_continer, root);
    if (info_continer.empty()) {
        printf("fail to parse md5.dat json value\n");
        return false;
    }

    if (!recursive_check(info_continer, directory,directory)) {
        return false;
    }

    if (!info_continer.empty()) {
        printf("in the folder, lack of file\n");
        return false;
    }
    return true;
}


