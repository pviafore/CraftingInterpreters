
template <typename T>
class List {
public:
    struct Node {
        T value;
        Node* next = nullptr;
        Node* prev = nullptr;
    };
    List() {}
    List(const List&) = delete;
    List& operator=(const List&) = delete;

    ~List() {
        while (first) {
            auto node = first->next;
            delete first;
            first = node;
        }
    }

    void pushBack(T value) {
        if (!first) {
            first = last = new Node{value};
        } else {
            last = new Node{value, nullptr, last};
            last->prev->next = last;
        }
    }

    void popFront() {
        if (first) {
            auto node = first;
            first = first->next;
            if (first) {
                first->prev = nullptr;
            }
            delete node;
        }
    }

    void insertBefore(T value, Node* after) {
        if (!first) {
            first = last = new Node{value};
        } else if (after == nullptr) {
            last->next = new Node{value, nullptr, last};
        } else {
            auto node = new Node{value, after, after->prev};
            if (after->prev) {
                after->prev->next = node;
            }
            after->prev = node;
        }
    }

    Node* findNextClosest(T value) const {
        if (!first) {
            return nullptr;
        }
        auto node = first;
        while (node && node->value < value) {
            node = node->next;
        }
        return node;
    }

    const Node* front() const {
        return first;
    }

private:
    Node* first = nullptr;
    Node* last = nullptr;
};