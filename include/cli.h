#ifndef CLI_H
#define CLI_H

class Cli {
public:
    struct Command {
        const char *name;
        void (*func)(int argc, char *argv[]);
        const char *descr;
    };

    void add_command(Command cmd);      // add commands before calling begin
    void begin();
    void handle();
    void list_commands();
};


#endif // CLI_H