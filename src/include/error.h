#pragma once
#define __ECS_ERROR_MSG_BUF_SIZE 256

void ecs_set_error(const char* message);
void _ecs_set_error(const char* file_and_line, const char* func, const char* message);

#define __ECS_SET_ERROR_HELPER(file, line, func, error) _ecs_set_error("CECS error: "file ":" #line " ", func, "(): " error)
#define __ECS_SET_ERROR_LINE_HELPER(file, line, func, error) __ECS_SET_ERROR_HELPER(file, line, func, error)
#define ECS_SET_ERROR(ERROR) __ECS_SET_ERROR_LINE_HELPER(__FILE__, __LINE__, __func__, ERROR)
// i think this system with macros and buffer for errors
// is not flexible and any good at all, but at this i don't know
// what else to do so i'll stay with it

