#ifndef JOB_TYPE_H
#define JOB_TYPE_H

enum job_type
{
    JOB_IDLE = 0, // no job
    JOB_ACCEPT = 1, // accept new incoming connection
    JOB_PROCESS = 2, // process data
    JOB_CLOSE = 3, // close connection
};

#endif /* !JOB_TYPE_H */
