#ifndef JOB_TYPE_H
#define JOB_TYPE_H

enum job_type
{
    JOB_ACCEPT, // accept new incoming connection
    JOB_PROCESS, // process data
    JOB_FINISH, // close connection
}

#endif /* !JOB_TYPE_H */
