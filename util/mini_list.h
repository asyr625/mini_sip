#ifndef MINI_LIST_H
#define MINI_LIST_H
#include <cstdio>

#define MINILIST_FORWARD_ITERATOR_OPTIMIZE 1

template<class T>
class Mini_List_Node
{
public:
    Mini_List_Node( T v, Mini_List_Node *nxt = NULL) : value(v), next(nxt){}
    Mini_List_Node * get_next(){ return next; }
    void set_next( Mini_List_Node *n ){ next = n; }
    T get_value() { return value; }
private:
    T   value;
    Mini_List_Node* next;
};

template<class T>
class Mini_List
{
public:
    Mini_List();

    Mini_List(const Mini_List &l2);
    ~Mini_List();

    void empty();

    void push_front(T item);

    void push_back(T item);

    T pop_back();

    void insert(int i, T item);

    void remove(T val);

    void remove(int index);

#ifdef MINILIST_VALIDATE
    void validate() const
#endif

    Mini_List &operator=(const Mini_List &l2);
    T operator[](int i) const;
    int size() const;

private:
    Mini_List_Node<T> *head;
    Mini_List_Node<T> *end;
    int nelem;

#ifdef MINILIST_FORWARD_ITERATOR_OPTIMIZE
    mutable int last_index;
    mutable Mini_List_Node<T> *last_node;
#endif
};

#endif // MINI_LIST_H
