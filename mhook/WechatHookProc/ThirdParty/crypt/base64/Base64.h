//
//  Base64.h
//
//  Created by Collin B. Stuart on 2013-10-29.
//
//

#ifndef __CBS__Base64__
#define __CBS__Base64__

#include <stdio.h>
#include <string>

using namespace std;

class Base64
{
public:
    static std::string base64Encode(unsigned char const* , unsigned int length);
    static std::string base64Decode(std::string const& string);
    static size_t base64DecodeToData(const std::string& sourceString, void* destinationPointer, size_t destinationSize);
};



#endif /* defined(__CBS__Base64__) */
