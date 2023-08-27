#ifndef LIST_H
#define LIST_H

#include <iostream>
#include <initializer_list>

template <typename T>
struct Node
{
    T data;
    Node *next;
    Node *prev;
    Node(const T &value) : data(value), next(nullptr), prev(nullptr) {}
};

template <typename T>
class _list_iterator
{
private:
    Node<T> *current;

public:
    _list_iterator() : current(nullptr) {}
    _list_iterator(Node<T> *node) : current(node) {}

    // 自增操作符
    _list_iterator<T> &operator++()
    {
        if (current)
        {
            current = current->next;
        }

        return *this;
    }

    // 自减操作符
    _list_iterator<T> &operator--()
    {
        if (current)
        {
            current = current->prev;
        }

        return *this;
    }

    // 解引用操作符
    T &operator*()
    {
        return current->data;
    }

    // 比较操作符
    bool operator!=(const _list_iterator<T> &other)
    {
        return this->current != other.current;
    }

    bool operator==(const _list_iterator<T> &other)
    {
        return this->current == other.current;
    }

    void change(Node<T> *node)
    {
        this->current = node;
    }

    Node<T> *call_back()
    {
        return this->current;
    }
};

template <typename T> // 数据类型
class DeLinkList
{
public:
    DeLinkList() : head(nullptr), tail(nullptr), length(0) {}

    DeLinkList(std::initializer_list<T> values) : head(nullptr), tail(nullptr)
    {
        Node<T> *current = nullptr;
        length = 0;
        for (const T &value : values)
        {
            ++length;
            if (head == nullptr)
            {
                head = new Node<T>(value);
                tail = head;
            }
            else
            {
                Node<T> *ptr = new Node<T>(value);
                tail->next = ptr;
                ptr->prev = tail;
                tail = tail->next;
            }
        }
    }

    DeLinkList(const DeLinkList &other) : head(nullptr), tail(nullptr)
    {
        this->length = other.size();
        Node<T> *otherNode = other.head;

        while (otherNode)
        {
            Node<T> *newNode = new Node<T>(otherNode->data);

            if (!head)
            {
                head = newNode;
                tail = newNode;
            }
            else
            {
                tail->next = newNode;
                newNode->prev = tail;
                tail = newNode;
            }

            otherNode = otherNode->next;
        }
    }

    // 重写赋值符号
    DeLinkList &operator=(const DeLinkList &other)
    {
        if (this != &other)
        {
            // 清空原有列表
            Node<T> *current = head;
            while (current)
            {
                Node<T> *next = current->next;
                delete current;
                current = next;
            }
            head = nullptr;
            tail = nullptr;

            // 赋值
            this->length = other.size();
            Node<T> *otherNode = other.head;

            while (otherNode)
            {
                Node<T> *newNode = new Node<T>(otherNode->data);

                if (!head)
                {
                    head = newNode;
                    tail = newNode;
                }
                else
                {
                    tail->next = newNode;
                    newNode->prev = tail;
                    tail = newNode;
                }

                otherNode = otherNode->next;
            }
        }
    }

    ~DeLinkList()
    {
        while (head)
        {
            Node<T> *ptr = head;
            head = head->next;
            delete ptr;
        }

        head = nullptr;
        tail = nullptr;
        length = 0;
    }

    // 从前压入元素
    void push_front(const T &value)
    {
        ++length;

        Node<T> *ptr = new Node<T>(value);

        if (!head)
        {
            head = ptr;
            tail = ptr;
            return;
        }

        ptr->next = head;
        head->prev = ptr;
        head = ptr;
    }

    // 从前弹出（删除）元素
    void pop_front()
    {
        if (!head)
        {
            return;
        }

        if (head == tail)
            tail = nullptr;

        Node<T> *ptr = head;
        head = head->next;
        delete ptr;

        --length;
    }

    // 弹出第一个元素
    T front()
    {
        return head->data;
    }

    // 从后压入元素
    void push_back(const T &value)
    {
        ++length;

        Node<T> *ptr = new Node<T>(value);

        if (!head)
        {
            head = ptr;
            tail = ptr;
            return;
        }

        ptr->prev = tail;
        tail->next = ptr;
        tail = ptr;
    }

    // 弹出最后一个元素
    void pop_back()
    {
        if (!head)
        {
            return;
        }

        if (head == tail)
            head == nullptr;

        Node<T> *ptr = tail;
        tail = tail->prev;
        delete ptr;

        --length;
    }

    T back()
    {
        return tail->data;
    }

    // 删除迭代器指向的那个结点
    _list_iterator<T> remove_node(_list_iterator<T> node)
    {
        it.change(node.call_back());

        --length;
        Node<T> *ptr = it.call_back();
        ++it;

        if (ptr != head && ptr != tail)
        {
            ptr->prev->next = ptr->next;
            ptr->next->prev = ptr->prev;
        }
        else if (ptr == head)
        {
            if (head == tail)
            {
                head = nullptr;
                tail = nullptr;
            }
            else
            {
                head = head->next;
                head->prev = nullptr;
            }
        }
        else if (ptr == tail)
        {
            tail = tail->prev;
            tail->next = nullptr;
        }

        delete ptr;

        return it;
    }

    // 删除值为value的所有结点
    void remove_node(T value)
    {
        Node<T> *current = head;
        while (current)
        {
            if (current->data == value)
            {
                --length;
                Node<T> *ptr = current;
                current = current->next;

                if (ptr != head && ptr != tail)
                {
                    ptr->prev->next = ptr->next;
                    ptr->next->prev = ptr->prev;
                }
                else if (ptr == head)
                {
                    if (head == tail)
                    {
                        head = nullptr;
                        tail = nullptr;
                    }
                    else
                    {
                        head = head->next;
                        head->prev = nullptr;
                    }
                }
                else if (ptr == tail)
                {
                    tail = tail->prev;
                    tail->next = nullptr;
                }

                delete ptr;
            }
            else
            {
                current = current->next;
            }
        }
    }

    // 返会链表的起始迭代器
    _list_iterator<T> begin()
    {
        it.change(head);
        return it;
    }

    // 返回链表的结束迭代器
    _list_iterator<T> end()
    {
        it.change(nullptr);
        return it;
    }

    size_t size() const
    {
        return length;
    }

    // 判断链表是否为空
    bool empty() const
    {
        return (length == 0);
    }

    void clear()
    {
        while (head)
        {
            Node<T> *ptr = head;
            head = head->next;
            delete ptr;
        }

        head = nullptr;
        tail = nullptr;
        length = 0;
    }

private:
    Node<T> *head;        // 链表头结点
    Node<T> *tail;        // 链表尾结点
    _list_iterator<T> it; // 存储迭代器指针
    size_t length;        // 链表长度
};

#endif