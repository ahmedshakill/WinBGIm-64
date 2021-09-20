#include <cstdlib>
#include <string>
#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
    string command = "g++ ";
    string blank = " ";
    int i;

    for (i = 1; i < argc; i++)
	command += blank + argv[i];

#ifdef WINDOWS
    command += " -mwindows";
#endif
    command += " -lbgi -lgdi32 -lcomdlg32 -luuid -loleaut32 -lole32";
    return system(command.c_str( ));
}
