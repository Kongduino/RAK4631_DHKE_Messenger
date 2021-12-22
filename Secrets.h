using namespace std;
#undef max
#undef min
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept> // std::out_of_range

char pubKeys[314] = "{\"5c8fed22\":\"3a1618ad379e493372379f0ad296782f\","
                    "\"68cb0e7f\":\"2b618a0964e08db79b7b9c4feb24e02b\","
                    "\"3747a5cb\":\"9a62b580aef445d754e32cc9b1457126\","
                    "\"a5779afd\":\"1e9bf9974c886ff065f5b99bee8550f2\"}";
char rawUUID[4] = {
  0x16, 0xda, 0xec, 0xb8
};
char myUUID[9] = "a5779afd";

char rawPvtKey[16] = {
  0x1e, 0x9b, 0xf9, 0x97, 0x4c, 0x88, 0x6f, 0xf0, 0x65, 0xf5, 0xb9, 0x9b, 0xee, 0x85, 0x50, 0xf2
};

StaticJsonDocument<800> doc;

unordered_map<char *, char *>myBuddies;
