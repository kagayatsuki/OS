#ifndef ELEVATOR_SAMPLE_LIB_EVR_H
#define ELEVATOR_SAMPLE_LIB_EVR_H

#define EVR_EXPORT extern "C"
#define EVR_CALL __stdcall

EVR_EXPORT void EVR_CALL evr_entry(void *arg_struct);
EVR_EXPORT void EVR_CALL evr_exit();

#endif //ELEVATOR_SAMPLE_LIB_EVR_H
