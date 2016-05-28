//
// Created by ulyanin on 21.05.16.
//

#ifndef LIFE_GAME_COMMON_H
#define LIFE_GAME_COMMON_H

#define LIFE_GAME_LOG

const int SECRET_PASSPHRASE_LENGTH  = 10;
const int DEFAULT_PORT = 1337;
const char PHRASE_BE_NODE[SECRET_PASSPHRASE_LENGTH + 1] = "WANTBENODE";
const char PHRASE_NODE_DONE_STEP[SECRET_PASSPHRASE_LENGTH + 1] = "NCOMPLETED";
const char PHRASE_SERVER_DONE_STEP[SECRET_PASSPHRASE_LENGTH + 1] = "SCOMPLETED";
const char PHRASE_SERVER_ACCEPT_NODE[SECRET_PASSPHRASE_LENGTH + 1] = "ACCEPTNODE";
const int MAX_FIELD_SIZE = 5000;
const int MAX_DATA_SIZE = 1024;
const int DEFAULT_FIELD_SIZE = 666;
const int DEFAULT_STEPS_AMOUNT = 30;
const int BUFFER_SIZE = 2048;
const int TIMEOUT_WAIT_NODES_MILLISECONDS = 10000;

/* secret_phrase, row, column, len, step */
//
//char * serialize_int(int number, char * data)
//{
//    memcpy(data, &number, sizeof(int));
//    return data + sizeof(int);
//}
//


//char * serialize(int row, int column, int len, int step, char * data)
//{
//    data = serialize_int(row);
//    data = serialize_int(column);
//    data = serialize_int(len);
//    return data = serialize_int(step);
//}
//
//char * deserialize(int * row, int * column, int * len, int * step, const char * data)
//{
//    data = deserialize_int(row);
//    data = deserialize_int(column);
//    data = deserialize_int(len);
//    return data = deserialize_int(step);
//}

#endif //LIFE_GAME_LIFE_GAME_CONSTANTS_H
