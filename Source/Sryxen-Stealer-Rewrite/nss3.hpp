#ifndef NSS3_HPP
#define NSS3_HPP

#include <string>

#define maxStringLength 1048576

struct TSECItem {
    int SECItemType;
    unsigned char* SECItemData;
    unsigned int SECItemLen;
};

namespace NSS {
    bool Initialize(const std::string& profilePath);
    std::string PK11SDR_Decrypt(const std::string& dbPath, const std::string& encryptedData);
}

#endif // NSS3_HPP
