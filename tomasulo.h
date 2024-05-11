#include <iostream>
#include <vector>
using namespace std;

typedef pair<string, int> reg_item;

class reservation_station
{

public:
    // contructorù
    reservation_station()
    {
        OP = "";
        name = "";
        busy = false;
        Qj = "";
        Qk = "";
        Vj = -1;
        Vk = -1;
    }

    // functions to be inherited by other classes
    virtual bool ready();
    virtual void excecute();
    virtual void wb();
    virtual void flush();

    // components of reservation station
    string OP;
    string name;
    bool busy;
    string Qj, Qk;
    int Vj, Vk;

    // register status array
    vector<reg_item> registers;
};