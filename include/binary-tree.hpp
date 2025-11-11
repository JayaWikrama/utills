#ifndef __BINARY_TREE_TEMPLATE_HPP__
#define __BINARY_TREE_TEMPLATE_HPP__

#include <functional>

template <typename T>
class Node
{
public:
    T data;
    Node *left;
    Node *right;

    Node(const T &val) : data(val), left(nullptr), right(nullptr) {}
};

template <typename T>
class BinaryTree
{
private:
    size_t sz;
    Node<T> *root;

    void insert(Node<T> *&node, const T &val)
    {
        if (node == nullptr)
        {
            node = new Node<T>(val);
            if (node)
            {
                sz++;
            }
        }
        else if (val < node->data)
        {
            insert(node->left, val);
        }
        else if (val > node->data)
        {
            insert(node->right, val);
        }
        else
        {
            /* duplicate data always on the right side */
            insert(node->right, val);
        }
    }

    bool contains(Node<T> *node, const T &val) const
    {
        if (node == nullptr)
            return false;
        if (val == node->data)
            return true;
        else if (val < node->data)
            return contains(node->left, val);
        else
            return contains(node->right, val);
    }

    Node<T> *find(Node<T> *node, const T &val) const
    {
        if (node == nullptr)
            return nullptr;
        if (val == node->data)
            return node;
        if (val < node->data)
            return find(node->left, val);
        return find(node->right, val);
    }

    Node<T> *remove(Node<T> *node, const T &val)
    {
        if (node == nullptr)
            return nullptr;

        if (val < node->data)
        {
            node->left = remove(node->left, val);
        }
        else if (val > node->data)
        {
            node->right = remove(node->right, val);
        }
        else
        {
            if (node->left == nullptr && node->right == nullptr)
            {
                delete node;
                if (sz > 0)
                    --sz;
                return nullptr;
            }
            else if (node->left == nullptr)
            {
                Node<T> *temp = node->right;
                delete node;
                if (sz > 0)
                    --sz;
                return temp;
            }
            else if (node->right == nullptr)
            {
                Node<T> *temp = node->left;
                delete node;
                if (sz > 0)
                    --sz;
                return temp;
            }
            else
            {
                Node<T> *minNode = node->right;
                while (minNode->left != nullptr)
                {
                    minNode = minNode->left;
                }
                node->data = minNode->data;
                node->right = remove(node->right, minNode->data);
            }
        }
        return node;
    }

    bool inOrderTraversal(Node<T> *node, const std::function<bool(T &)> &callback) const
    {
        if (node != nullptr)
        {
            if (!inOrderTraversal(node->left, callback))
                return false;
            if (!callback(node->data))
                return false;
            if (!inOrderTraversal(node->right, callback))
                return false;
        }
        return true;
    }

    bool inOrderTraversal(Node<T> *node, const std::function<bool(T &)> &callback)
    {
        if (node != nullptr)
        {
            if (!inOrderTraversal(node->left, callback))
                return false;
            if (!callback(node->data))
                return false;
            if (!inOrderTraversal(node->right, callback))
                return false;
        }
        return true;
    }

    void deleteTree(Node<T> *node)
    {
        if (node != nullptr)
        {
            deleteTree(node->left);
            deleteTree(node->right);
            delete node;
        }
    }

public:
    BinaryTree() : sz(0), root(nullptr) {}

    ~BinaryTree()
    {
        deleteTree(root);
        sz = 0;
    }

    void insert(const T &val)
    {
        insert(root, val);
    }

    bool contains(const T &val) const
    {
        return contains(root, val);
    }

    const Node<T> *getRoot() const
    {
        return root;
    }

    Node<T> *find(const T &val) const
    {
        return find(root, val);
    }

    void remove(const T &val)
    {
        root = remove(root, val);
    }

    void clear()
    {
        deleteTree(root);
        root = nullptr;
        sz = 0;
    }

    void inOrder(const std::function<bool(T &)> &callback) const
    {
        inOrderTraversal(root, callback);
    }

    void inOrder(const std::function<bool(T &)> &callback)
    {
        inOrderTraversal(root, callback);
    }

    size_t size() const
    {
        return this->sz;
    }
};

#endif