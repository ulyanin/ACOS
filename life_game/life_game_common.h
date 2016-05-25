//
// Created by ulyanin on 21.05.16.
//

#ifndef LIFE_GAME_LIFE_GAME_CONSTANTS_H
#define LIFE_GAME_LIFE_GAME_CONSTANTS_H

#define SECRET_PASSPHRASE_LENGTH 10
#define DEFAULT_PORT 1337
const char PHRASE_BE_NODE[SECRET_PASSPHRASE_LENGTH + 1] = "WANTBENODE";
const char PHRASE_NODE_DONE_STEP[SECRET_PASSPHRASE_LENGTH + 1] = "NCOMPLETED";

/* secret_phrase, row, column, len, step */
const int MINIMAL_DATA_SIZE = SECRET_PASSPHRASE_LENGTH + 4 * sizeof(int);
#define BUFSIZE 4096

char * serialize_int(int number, const char * data)
{
    memcpy(data, &number, sizeof(int));
    return data + sizeof(int);
}
char * deserialize_int(int * number, char * data)
{
    memcpy(number, data, sizeof(int));
    return data + sizeof(int);
}

char * serialize(int row, int column, int len, int step, const char * data)
{
    data = serialize_int(row);
    data = serialize_int(column);
    data = serialize_int(len);
    return data = serialize_int(step);
}

char * deserialize(int * row, int * column, int * len, int * step, const char * data)
{
    data = deserialize_int(row);
    data = deserialize_int(column);
    data = deserialize_int(len);
    return data = deserialize_int(step);
}

#endif //LIFE_GAME_LIFE_GAME_CONSTANTS_H
