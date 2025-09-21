#ifndef __QUEUE_TEMPLATE_HPP__
#define __QUEUE_TEMPLATE_HPP__

#include <functional>

template <typename T>
class Queue
{
private:
    struct Node
    {
        T data;
        Node *next;
        Node *prev;
        Node(const T &value) : data(value), next(nullptr), prev(nullptr) {}
    };

    Node *front;
    Node *rear;
    size_t count;

    Node *getNode(size_t index) const
    {
        if (index >= count)
        {
            throw std::out_of_range("Index out of range!");
        }
        Node *current = front;
        for (size_t i = 0; i < index; ++i)
        {
            current = current->next;
        }
        return current;
    }

public:
    Queue() : front(nullptr), rear(nullptr), count(0) {}

    ~Queue()
    {
        clear();
    }

    void clear()
    {
        while (!isEmpty())
        {
            dequeue();
        }
    }

    void enqueue(const T &value)
    {
        Node *newNode = new Node(value);
        if (isEmpty())
        {
            front = rear = newNode;
        }
        else
        {
            rear->next = newNode;
            newNode->prev = rear;
            rear = newNode;
        }
        count++;
    }

    T dequeue()
    {
        if (isEmpty())
        {
            throw std::underflow_error("Queue is empty!");
        }
        Node *temp = front;
        T removedData = temp->data;
        front = front->next;
        if (front != nullptr)
        {
            front->prev = nullptr;
        }
        else
        {
            rear = nullptr;
        }
        delete temp;
        count--;
        return removedData;
    }

    T &getFront()
    {
        if (isEmpty())
        {
            throw std::underflow_error("Queue is empty!");
        }
        return front->data;
    }

    T &at(size_t index)
    {
        return getNode(index)->data;
    }

    T &operator[](size_t index)
    {
        return at(index);
    }

    const T &at(size_t index) const
    {
        return getNode(index)->data;
    }

    const T &operator[](size_t index) const
    {
        return at(index);
    }

    void iteration(const std::function<bool(T &)> &callback) const
    {
        Node *current = front;
        for (size_t i = 0; i < count; ++i)
        {
            if (callback(current->data) == false)
                return;
            current = current->next;
        }
    }

    void reverseIteration(const std::function<bool(T &)> &callback) const
    {
        Node *current = rear;
        while (current != nullptr)
        {
            if (callback(current->data) == false)
                return;
            current = current->prev;
        }
    }

    bool isEmpty() const
    {
        return count == 0;
    }

    size_t size() const
    {
        return count;
    }
};

#endif