#include <string.h>

#include <error.h>

// we should maybe make this just a pointer later,
// since error messages propably will only be string literals
static char error_msg_buf[__ECS_ERROR_MSG_BUF_SIZE];
void ecs_set_error(const char* message) {
    strcpy(error_msg_buf, message);
}
inline void _ecs_set_error(const char* file_and_line, const char* func, const char* message) {
    ecs_set_error(file_and_line);
    strcat(error_msg_buf, func);
    strcat(error_msg_buf, message);
}

const char* ecs_get_error() {
    return error_msg_buf;
}
