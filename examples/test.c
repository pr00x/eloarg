#include <stdio.h>
#include <eloarg.h>

int main(int argc, char **argv) {
    EloArg *eloarg = eloArgInit(6);

    eloarg->add("h", "help", "Displays help information about the available options and usage.", ARG_INFO);
    eloarg->add(NULL, "version", "Displays the version number of the program.", ARG_INFO);
    eloarg->add(NULL, "port", "Specifies the port number to listen on.", ARG_REQUIRED);
    eloarg->add("f", "file", "Path to the input file.", ARG_OPTIONAL);
    eloarg->add("s", "say-hello", "Say hello.", ARG_NONE);
    eloarg->add("v", "verbose", "Increase verbosity level.", ARG_NONE);

    eloarg->parse(argc, argv);

    if(eloarg->has("help"))
        eloarg->help("CustomTool 1.0, a powerful utility for advanced system operations.\nBasic usages:\nconnect to a server:  tool [options] hostname port [port] ...\nmonitor incoming traffic:    tool -m -p port [options] [hostname] [port] ...\nsend data to remote server:   tool -S hostname:port -p port [options]\n\nArguments for long options apply equally to their short options.\n"
, "Specify custom timeouts using '-t' or '--timeout'. Example: '30' for 30 seconds.");
    else if(eloarg->has("version")) {
        puts("v1.0.0");

        eloarg->free();
        return 0;
    }

    printf("Port: %s\n", eloarg->get("port"));
    
    if(eloarg->has("file"))
        printf("File: %s\n", eloarg->get("file"));

    if(eloarg->has("say-hello"))
        puts("Hello :)");

    // Handle verbosity levels
    size_t verbosity = eloarg->getCount("v");

    if(verbosity == 0)
        puts("No verbosity: Minimal output");
    else if(verbosity == 1)
        puts("Verbose level 1: Basic information");
    else if(verbosity == 2)
        puts("Verbose level 2: Detailed information");
    else
        puts("Verbose level 3: Debugging information");

    eloarg->free();

    return 0;
}