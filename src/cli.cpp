#include "cli.h"
#include "assert.h"
#include <Arduino.h>

static const int MAX_COMMANDS = 20;
static const int RX_BUF_SIZE = 20;
static const int MAX_ARGS = 3;

// TODO: move this to class/private
static bool initialized = false;
static Cli::Command commands[MAX_COMMANDS];
static int num_commands = 0;
static char rx_buf[RX_BUF_SIZE];
static int rx_cnt;

static const char *prompt = "> ";

static void parse_command()
{
    int argc = 0;
    char *argv[MAX_ARGS] = {0};

    const char* delimiters = " ";
    char *token = strtok(rx_buf, delimiters);
    while (token != NULL) {
        argv[argc++] = token;
        token = strtok(NULL, delimiters);
    }

    // Compare first token (command) with all command names
    for (int i=0; i < num_commands; i++) {
        Cli::Command cmd = commands[i];
        if (strcmp(cmd.name, argv[0]) == 0) {
            if (cmd.func != nullptr) cmd.func(argc, argv);
            else Serial.println("Command function not provided");
            return;
        }
    }
}


static void reset()
{
    rx_cnt = 0;
    memset(rx_buf, 0, RX_BUF_SIZE);
}


void Cli::list_commands()
{
    Serial.println("\nCommand name\t\tDescription");
    Serial.println("------------\t\t-----------");
    for (int i=0; i<num_commands; i++) {
        Serial.print(commands[i].name);
        Serial.print("\t\t\t");
        Serial.print(commands[i].descr);
        Serial.println();
    }
}


void Cli::handle()
{
    int n = Serial.available();
    while (n > 0) {
        char c = Serial.read();
        if (c == '\b') {                    // backspace
            rx_buf[--rx_cnt] = '\0';

        } else if (rx_cnt == RX_BUF_SIZE) { // buffer overflow
            reset();

        } else if (c == '\n' || c == '\r') { // line ending (LF or CR)
            parse_command();
            reset();
            if (Serial.peek() == '\n') Serial.read();  // handle CRLF line ending
            Serial.print(prompt);

        } else {
            rx_buf[rx_cnt++] = c;           // add to buffer
        }
        n--;
    }
}


void Cli::begin()
{
    initialized = true;
    reset();
    Serial.print(prompt);
}


void Cli::add_command(Cli::Command cmd)
{
    assert(num_commands < MAX_COMMANDS);
    assert(!initialized);                   // commands need to be added before calling begin() function
    commands[num_commands++] = cmd;
}