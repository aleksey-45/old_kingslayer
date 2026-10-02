#pragma once
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <string>
#include <cstring>
#include "json.hpp"

using json = nlohmann::json;

static json loadLanguageFile(const std::string& langCode) {
    std::ifstream file("lang/" + langCode + ".json");
    json langJson;
    file >> langJson;
    return langJson;
}

// 替换字符串中的所有 %d 占位符
static std::string formatString(const std::string& format, const std::vector<int>& values) {
    std::string result = format;
    size_t pos = 0;

    for (int value : values) {
        pos = result.find("%d", pos); // 查找下一个 %d 的位置
        if (pos == std::string::npos) {
            break; // 如果没有找到更多占位符，退出循环
        }

        // 将 %d 替换为对应的值
        std::ostringstream oss;
        oss << value; // 将整数转换为字符串
        result.replace(pos, 2, oss.str());

        // 跳过刚刚替换的部分，继续查找下一个占位符
        pos += oss.str().length();
    }

    return result;
}

static std::string formatString(const std::string& format, const std::vector<int>& intValues, const std::vector<float>& floatValues) {
    std::string result = format;
    size_t pos = 0;
    size_t intIndex = 0;    // 当前处理的整数索引
    size_t floatIndex = 0;  // 当前处理的浮点数索引

    while (true) {
        pos = result.find("%d", pos); // 查找下一个 %d 的位置
        if (pos != std::string::npos) {
            if (intIndex >= intValues.size()) {
                //throw std::out_of_range("Not enough integer values for %d placeholders");
            }
            std::ostringstream oss;
            oss << intValues[intIndex++];
            result.replace(pos, 2, oss.str());
            pos += oss.str().length();
            continue;
        }

        break; // 如果没有找到更多占位符，退出循环
    }
    pos = 0;
    while (true) {
        pos = result.find("%f", pos); // 查找下一个 %f 的位置
        if (pos != std::string::npos) {
            if (floatIndex >= floatValues.size()) {
                //throw std::out_of_range("Not enough float values for %f placeholders");
            }
            std::ostringstream oss;
            oss << floatValues[floatIndex++];
            result.replace(pos, 2, oss.str());
            pos += oss.str().length();
            continue;
        }
        break;
    }

    // 检查是否还有未替换的占位符
    if (intIndex < intValues.size() || floatIndex < floatValues.size()) {
        //throw std::out_of_range("More values provided than placeholders");
    }

    return result;
}

static std::string formatString(const std::string& format, const std::vector<int>& intValues, const std::vector<float>& floatValues,const std::vector<std::string>& stringValues) {
    std::string result = format;
    size_t pos = 0;
    size_t intIndex = 0;    // 当前处理的整数索引
    size_t floatIndex = 0;  // 当前处理的浮点数索引
    size_t strIndex = 0;  // 当前处理的string索引

    while (true) {
        pos = result.find("%d", pos); // 查找下一个 %d 的位置
        if (pos != std::string::npos) {
            if (intIndex >= intValues.size()) {
                //throw std::out_of_range("Not enough integer values for %d placeholders");
            }
            std::ostringstream oss;
            oss << intValues[intIndex++];
            result.replace(pos, 2, oss.str());
            pos += oss.str().length();
            continue;
        }

        break; // 如果没有找到更多占位符，退出循环
    }
    pos = 0;
    while (true) {
        pos = result.find("%f", pos); // 查找下一个 %f 的位置
        if (pos != std::string::npos) {
            if (floatIndex >= floatValues.size()) {
                //throw std::out_of_range("Not enough float values for %f placeholders");
            }
            std::ostringstream oss;
            oss << floatValues[floatIndex++];
            result.replace(pos, 2, oss.str());
            pos += oss.str().length();
            continue;
        }
        break;
    }
    pos = 0;
    while (true) {
        pos = result.find("%s", pos); // 查找下一个 %s 的位置
        if (pos != std::string::npos) {
            if (strIndex >= stringValues.size()) {
                //throw std::out_of_range("Not enough float values for %s placeholders");
            }
            std::ostringstream oss;
            oss << stringValues[strIndex++];
            result.replace(pos, 2, oss.str());
            pos += oss.str().length();
            continue;
        }
        break;
    }
    return result;
}

