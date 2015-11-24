#ifndef MUTEX_H
#define MUTEX_H

class Mutex
{
    friend class Cond_Var;

public:
    Mutex();
    ~Mutex();
    Mutex(const Mutex &);

    void lock();
    void unlock();

    Mutex& operator=(const Mutex &m);
private:
    void create_mutex();
    void *handle_ptr;
};

#endif // MUTEX_H
