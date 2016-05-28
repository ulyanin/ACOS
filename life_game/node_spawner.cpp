#include <cstdlib>
#include <cstdio>
#include <string>
#include <unistd.h>

int main(int argc, char ** argv)
{
    if (argc < 3) {
        fprintf(stderr, "bad params; using: ./node_spawner amount ./life_game_node server port\n");
        return 0;
    }
    int n = std::stoi(argv[1]);
    char ** arg = new char*[argc - 1];
    for (int i = 0; i < argc - 2; ++i) {
        arg[i] = argv[i + 2];
    }
    argv[argc - 2] = (char *)0;
    for (int i = 0; i < n; ++i) {
        int p = fork();
        if (p == 0) {
            printf("%s %s %s %s\n", arg[0], arg[1], arg[2], arg[3]);
            return execvp(arg[0], arg);
        }
    }
    return 0;
}