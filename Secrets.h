using namespace std;
#undef max
#undef min
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept> // std::out_of_range

char pubKeys[314] = "{\"76b34757\":\"bf6e0bc132dc7cad85d6db2ebe5152e8\","
                    "\"52af7712\":\"2eec2e385e0cee442e6c49f6c889a7b9\","
                    "\"1a6239fa\":\"e51bc1874f4af73532a1d0c89a41d9cc\","
                    "\"af1717ee\":\"9f6c5c3cde29eedc5ce062a17a561978\"}";

// Simulated acquisition of the keys and UUID from EEPROM
// To be replaced by real code later
//char rawUUID[4] = {
//  0x52, 0xaf, 0x77, 0x12
//};
//char myUUID[9] = "52af7712";
//
//char rawPvtKey[16] = {
//  0x0b, 0x0a, 0xe6, 0x46, 0x30, 0x1b, 0x26, 0x5e, 0x0c, 0xc7, 0x36, 0x41, 0x83, 0xeb, 0xaa, 0xa3
//};
//
//char rawPubKey[16] = {
//  0x2e, 0xec, 0x2e, 0x38, 0x5e, 0x0c, 0xee, 0x44, 0x2e, 0x6c, 0x49, 0xf6, 0xc8, 0x89, 0xa7, 0xb9
//};

char rawUUID[4] = {
  0x76, 0xb3, 0x47, 0x57
};
char myUUID[9] = "76b34757";

char rawPvtKey[16] = {
  0x55, 0x12, 0x69, 0x4e, 0x0d, 0xc4, 0x82, 0x12, 0xc6, 0x29, 0x00, 0xf3, 0x13, 0x53, 0x39, 0x0f
};

char rawPubKey[16] = {
  0xbf, 0x6e, 0x0b, 0xc1, 0x32, 0xdc, 0x7c, 0xad, 0x85, 0xd6, 0xdb, 0x2e, 0xbe, 0x51, 0x52, 0xe8
};

StaticJsonDocument<800> doc;

unordered_map<char *, char *>myBuddies;
