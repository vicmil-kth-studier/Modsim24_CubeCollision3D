#pragma once
#include <iostream>
#include <math.h>
#include <string>
#include <sstream>
#include <vector>
#include <cassert>
#include <fstream>
#include <filesystem>


inline std::vector<std::string> split_string(std::string str, char separator) {
    if(str.length() == 0) {
        return std::vector<std::string>();
    }
    
    std::vector<std::string> strings;

    int startIndex = 0, endIndex = 0;
    for (int i = 0; i <= str.size(); i++) {
        // If we reached the end of the word or the end of the input.
        if (str[i] == separator || i == str.size()) {
            endIndex = i;
            std::string temp;
            temp.append(str, startIndex, endIndex - startIndex);
            strings.push_back(temp);
            startIndex = endIndex + 1;
        }
    }

    return strings;
}

inline bool string_contains(const std::string& str, const std::string& substr) {
    return str.find(substr) != std::string::npos;
}

#define GetFileName split_string(__FILE__, '/').back()


#ifdef DEBUG_KEYWORDS
    const std::string __debug_keywords_raw__ = DEBUG_KEYWORDS;
#else
    const std::string __debug_keywords_raw__ = "";
#endif

const std::vector<std::string> __debug_keywords__ = split_string(__debug_keywords_raw__, ',');

inline bool contains_debug_keyword(const std::string str, const std::vector<std::string>& in_debug_keywords) {
    //std::cout << "str: " << str << std::endl;
    for(int i = 0; i < in_debug_keywords.size(); i++) {
        //std::cout << "in_debug_keywords[i]: " << in_debug_keywords[i] << std::endl;
        if(string_contains(str, in_debug_keywords[i])) {
            //std::cout << "true" << std::endl;
            return true;
        }
    }
    //std::cout << "false" << std::endl;
    return false;
}

#define GetLineKeywords " " + GetFileName + " " +  __func__ + "() ln: " + std::to_string(__LINE__) + " " // Used to see if line should be printed
#define IfRelevantDebug(x) \
    if (contains_debug_keyword(GetLineKeywords, __debug_keywords__)) { \
        x; \
    }

        
#ifdef USE_DEBUG
#define DisableLogging const std::vector<std::string> __debug_keywords__ = {};
#else
#define DisableLogging ;
#endif 

#ifdef USE_DEBUG
#define Debug(x) IfRelevantDebug(std::cout << GetFileName << ": ln" << __LINE__ << ": " << x << std::endl)
#else
#define Debug(x) 
#endif 

#ifdef USE_DEBUG
#define Assert(x) if((x) == false) {std::cout << GetFileName << ": ln" << __LINE__ << ": " << "Assert failed! \n" << #x << std::endl; throw;}
//#define Assert(x) assert(x)
#else
#define Assert(x)
#endif 

#define ThrowError(x) std::cout << GetFileName << ": ln" << __LINE__ << ": " << x << std::endl; throw
#define ThrowNotImplemented() std::cout << GetFileName << ": ln" << __LINE__ << ": " << "Not implemented yet!" << std::endl; throw

#define START_TRACE_FUNCTION() Debug(std::string("start ") + __func__)
#define END_TRACE_FUNCTION() Debug(std::string("exit ") + __func__)

#define DebugExpr(x) Debug(#x << ": '" << x << "'")