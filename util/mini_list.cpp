#include "mini_list.h"
#include "my_assert.h"

template<class T>
Mini_List<T>::Mini_List()
    :head(NULL),end(NULL),nelem(0)
    #ifdef MINILIST_FORWARD_ITERATOR_OPTIMIZE
    ,last_index(-2),last_node(NULL)
    #endif
{
}

template<class T>
Mini_List<T>::Mini_List(const Mini_List &l2)
#ifdef MINILIST_FORWARD_ITERATOR_OPTIMIZE
    :last_index(-2),last_node(NULL)
    #endif
{
    head = end = NULL;
    nelem = l2.nelem;
    Mini_List_Node<T> *l2cur = l2.head;
    Mini_List_Node<T> *last = NULL;

    for (int i=0; i< nelem; i++)
    {
        Mini_List_Node<T> *tmp = new Mini_List_Node<T>( l2cur->get_value(), NULL ) ;
        if (i == 0)
            head = tmp;
        if (i == nelem-1)
            end = tmp;
        if (last != NULL) // last==NULL on first itteration
            last->set_next(tmp);
        last = tmp;
        l2cur = l2cur->get_next();
    }
}

template<class T>
Mini_List<T>::~Mini_List()
{
    empty();
}

template<class T>
void Mini_List<T>::empty()
{
    Mini_List_Node<T> *cur = head;
    while (cur)
    {
        Mini_List_Node<T> *tmp = cur;
        cur = cur->get_next();
        delete tmp;
    }
    nelem = 0;
    head = NULL;
    end = NULL;
}

template<class T>
void Mini_List<T>::push_front(T item)
{
#ifdef MINILIST_FORWARD_ITERATOR_OPTIMIZE
    if (last_index >= 0)
        last_index ++;
#endif
    nelem ++;
    Mini_List_Node<T> *n = new Mini_List_Node<T>(item, NULL);
    n->set_next(head);
    head = n;
    if (end == NULL)
        end = n;
}

template<class T>
void Mini_List<T>::push_back(T item)
{
    nelem++;
    Mini_List_Node<T> *n = new Mini_List_Node<T>(item, NULL);
    if (head == NULL)
    {
        head = end = n;
    }
    else
    {
        end->set_next(n);
        end = n;
    }
}

template<class T>
T Mini_List<T>::pop_back()
{
    my_assert(size()>0);
#ifdef MINILIST_FORWARD_ITERATOR_OPTIMIZE
    if (last_index >= size()-1)
        last_index = -2;
#endif
    nelem--;
    T ret = end->get_value();
    if (head == end)
    {
        delete head;
        head = end = NULL;
        return ret;
    }
    Mini_List_Node<T> *cur = head;
    for (int i=0; i<nelem-1; i++)
        cur = cur->get_next();
    cur->set_next(NULL);
    delete end;
    end = cur;
    return ret;
}

template<class T>
void Mini_List<T>::insert(int i, T item)
{
#ifdef MINILIST_FORWARD_ITERATOR_OPTIMIZE
    last_index = -2;
#endif
    my_assert(i<=size());

    if (size()==0 && i==0) // only item
    {
        nelem++;
        Mini_List_Node<T> *n = new Mini_List_Node<T>(item, NULL);
        head = end = n;
#ifdef MINILIST_VALIDATE
        validate();
#endif
        return;
    }

    if (i==0)  // first, not only
    {
        nelem ++;
        Mini_List_Node<T> *n = new Mini_List_Node<T>(item, head);
        //				MiniListNode<T> *old_first = head;
        head = n;
#ifdef MINILIST_VALIDATE
        validate();
#endif
        return;
    }

    if (i==size())  // last, not only
    {
        nelem++;
        Mini_List_Node<T> *n = new Mini_List_Node<T>(item, NULL);
        end->set_next(n);
        end = n;
        return;
    }
    else
    {
        Mini_List_Node<T> *cur =head;
        for (int j=0; j<i-1; j++)
            cur = cur->get_next();

        nelem++;
        Mini_List_Node<T> *n = new Mini_List_Node<T>(item, cur->get_next() );
        cur->set_next(n);
    }
#ifdef MINILIST_VALIDATE
    validate();
#endif
}

template<class T>
void Mini_List<T>::remove(T val)
{
    for (int i=0; i<size(); i++)
    {
        if (val == (*this)[i])
        {
            remove(i);
            i=0;
        }
    }
}

template<class T>
void Mini_List<T>::remove(int index)
{
    my_assert(size()>0 && index >= 0 && index < size());

#ifdef MINILIST_FORWARD_ITERATOR_OPTIMIZE
    last_index = -2;
#endif

    //only item
    if (index==0 && size()==1)
    {
        nelem--;
        end = NULL;
        delete head;
        head = NULL;
        return;
    }

    //if first item
    if (index==0)
    {
        nelem--;
        Mini_List_Node<T> *tmp = head;
        head = head->get_next();
        delete tmp;
        return;
    }
    //middle or last
    Mini_List_Node<T> *cur = head;
    Mini_List_Node<T> *prev = head;
    for (int i=0; i<index; i++)
    {
        prev = cur;
        cur = cur->get_next();
    }
    prev->set_next( cur->get_next() );
    if (cur == end)
        end = prev;
    nelem--;
    delete cur;
}

#ifdef MINILIST_VALIDATE
void validate() const
{
    Mini_List_Node<T> *cur = head;
    int i;

    for (i=0; i<nelem; i++)
    {
        if (i == nelem-1)
        {
            my_assert(cur->get_next() == NULL);
        }
        cur= cur->get_next();
    }
    for (i=0; i<size(); i++)
    {
        T t = (*this)[i];
    }
    for (i=nelem-1; i>=0; i--)
    {
        T t = (*this)[i];
    }

    cur = head;
    int n = 0;
    if ( nelem > 0 )
    {
        while (cur!=end)
        {
            n++;
            cur= cur->get_next();
        };
        my_assert(n==nelem-1);
    }
    else
    {
        my_assert(head==NULL && end==NULL);
    }
    cur = head;
    n=0;
    if (nelem>0)
    {
        while (cur!=NULL)
        {
            n++;
            cur= cur->get_next();
        };
        my_assert(n == nelem);
    }else{
        my_assert(head==NULL && end==NULL);
    }

}
#endif

template<class T>
Mini_List<T> &Mini_List<T>::operator=(const Mini_List<T> &l2)
{
    nelem = l2.nelem;
    Mini_List_Node<T> *l2cur = l2.head;
    Mini_List_Node<T> *last = NULL;

    for (int i=0; i< nelem; i++){
        Mini_List_Node<T> *tmp = new Mini_List_Node<T>( l2cur->get_value(), l2cur->get_next() ) ;
        if (i == 0)
            head = tmp;
        if (i == nelem-1)
            end = tmp;
        if (last != NULL)
            last->set_next(tmp);
        last = tmp;
        l2cur = l2cur->get_value();
    }
    return *this;
}

template<class T>
T Mini_List<T>::operator[](int i) const
{
    Mini_List<T>* cur = head;
    my_assert( i >= 0 && i < size() );

    int j = 0;
#ifdef MINILIST_FORWARD_ITERATOR_OPTIMIZE
    if( last_index >= 0 && i >= last_index )
    {
        j = last_index;
        cur = last_node;
    }
#endif
    for ( ; j<i; j++)
        cur = cur->get_next();
#ifdef MINILIST_FORWARD_ITERATOR_OPTIMIZE
    last_index = i;
    last_node = cur;
#endif
    return cur->get_value();
}

template<class T>
int Mini_List<T>::size() const
{
    return nelem;
}
