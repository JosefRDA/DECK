#ifndef _CSTMLINKEDLIST_HPP_
#define _CSTMLINKEDLIST_HPP_



template <class T>
class CstmListNode {
  public:
    T element;
    CstmListNode* next;
    CstmListNode* prev;

    CstmListNode(T element, CstmListNode* prev, CstmListNode* next) : element(element)
    {
      this->next = next;
      this->prev = prev;
    };
};

template <class T>
class CstmLinkedList  {
  private:
    int length;
    CstmListNode<T>* head;
    CstmListNode<T>* tail;
    CstmListNode<T>* curr;
  public:
    CstmLinkedList();
    CstmLinkedList(const CstmLinkedList<T>&);
    ~CstmLinkedList();
    T& getCurrent();
    T& First() const;
    T& Last() const;
    int getLength();
    void Append(T);
    void DeleteLast();
    void DeleteFirst();
    void DeleteCurrent();
    bool next();
    bool moveToStart();
    bool prev();
    void Delete(T&);
    bool Search(T);
    void Clear();
    void PutFirstToLast();
    void Update(T elem);
    CstmLinkedList& operator = (const CstmLinkedList<T>&);
};

template <class T>
CstmLinkedList<T>::CstmLinkedList() {
    length = 0;
    head = nullptr;
    tail = nullptr;
    curr = nullptr;
}

template <class T>
CstmLinkedList<T>::CstmLinkedList(const CstmLinkedList<T> & list) {
    length = 0;
    head = nullptr;
    tail = nullptr;
    curr = nullptr;

    CstmListNode<T> * temp = list.head;

    while(temp != nullptr)
    {
        Append(temp->element);
        temp = temp->next;
    }
}

template <class T>
CstmLinkedList<T> & CstmLinkedList<T>::operator=(const CstmLinkedList<T> & list)
{
    Clear();

    CstmListNode<T> * temp = list.head;

    while(temp != nullptr)
    {
        Append(temp->element);
        temp = temp->next;
    }

    return *this;
}

template <class T>
CstmLinkedList<T>::~CstmLinkedList() {
    Clear();
}

template<class T>
T& CstmLinkedList<T>::getCurrent()
{
  return curr->element;
}

template<class T>
T& CstmLinkedList<T>::First() const
{
  return head->element;
}

template<class T>
T& CstmLinkedList<T>::Last() const
{
  return tail->element;
}

template<class T>
int CstmLinkedList<T>::getLength()
{
  return length;
}

template <class T>
void CstmLinkedList<T>::Append(T element)
{
    CstmListNode<T> * node = new CstmListNode<T>(element, tail, nullptr);

    if(length == 0)
        curr = tail = head = node;
    else {
        tail->next = node;
        tail = node;
    }

    length++;

}

template <class T>
void CstmLinkedList<T>::DeleteLast()
{
    if(length == 0)
      return;
    curr = tail;
    DeleteCurrent();
}

template <class T>
void CstmLinkedList<T>::DeleteFirst()
{
    if(length == 0)
      return;
    curr = head;
    DeleteCurrent();
}

template <class T>
bool CstmLinkedList<T>::next()
{
    if(length == 0)
        return false;

    if(curr->next == nullptr)
        return false;

    curr = curr->next;
    return true;
}

template <class T>
bool CstmLinkedList<T>::moveToStart()
{
    curr = head;
    return length != 0;
}

template<class T>
bool CstmLinkedList<T>::prev()
{
    if(length == 0)
        return false;

    if(curr->prev != nullptr)
        return false;

    curr = curr->prev;
    return true;
}

template <class T>
void CstmLinkedList<T>::Delete(T & elem)
{
    if(Search(elem))
        DeleteCurrent();
}

template <class T>
void CstmLinkedList<T>::DeleteCurrent()
{
    if(length == 0)
        return;
    length--;
    CstmListNode<T> * temp = curr;

    if(temp->prev != nullptr)
        temp->prev->next = temp->next;
    if(temp->next != nullptr)
        temp->next->prev = temp->prev;

    if(length == 0)
        head = curr = tail = nullptr;
    else if(curr == head)
        curr = head = head->next;
    else if(curr == tail)
        curr = tail = tail->prev;
    else
        curr = curr->prev;

     delete temp;
}

template <class T>
bool CstmLinkedList<T>::Search(T elem)
{
    if(length == 0)
        return false;
    if(moveToStart())
        do {
            if(curr->element == elem)
                return true;
        } while (next());
    return false;
}

template <class T>
void CstmLinkedList<T>::PutFirstToLast()
{
  if(length < 2)
    return;
  CstmListNode<T>* temp = head->next;
  head->next->prev = nullptr;
  head->next = nullptr;
  head->prev = tail;
  tail->next = head;
  tail = head;
  head = temp;
}

template <class T>
void CstmLinkedList<T>::Update(T elem)
{
    if(Search(elem))
        curr->element = elem;
}

template <class T>
void CstmLinkedList<T>::Clear()
{
    if(length == 0)
        return;
    CstmListNode<T> * temp = head;

    while(temp != nullptr)
    {
        head = head->next;
        delete temp;
        temp = head;
    }

    head = curr = tail = nullptr;

    length = 0;

}

#endif // end _CSTMLINKEDLIST_HPP_
