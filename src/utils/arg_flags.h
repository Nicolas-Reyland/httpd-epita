#ifndef ARG_FLAGS_H
#define ARG_FLAGS_H

#define CMD_FLAG_DRY_RUN 0001
#define CMD_FLAG_DAEMON 00002
#define CMD_FLAG_START 000004
#define CMD_FLAG_STOP 0000010
#define CMD_FLAG_RELOAD 00020
#define CMD_FLAG_RESTART 0040
#define CMD_FLAG_INVALID 0100

int daemon_flag_from_string(char *arg);

#endif /* !ARG_FLAGS_H */
