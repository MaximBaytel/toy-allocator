#include <iostream>
#include <stdexcept>
#include <cstring>

using namespace std;


class SmallAllocator {
public:

    SmallAllocator()
    {
        //list have at least two nodes: first node without any data and second node that is after the end of an array (so without data)
        root = new (Memory) node();
        currNode = root;

        root->prev = 0;
        root->next = endIndex;
        root->size = 0;

        currentAvailable = AllCapacity - sizeof(root);
    }

    void *Alloc(unsigned int Size)
    {
        if (Size ==0)
            throw logic_error("Size must be greater than zero!");

        size_t roundedSize = roundSize(Size);

        size_t capacity = roundedSize + sizeof(node);

        if (capacity > currentAvailable)
            throw logic_error("Not enough memory!");

        if (!currNode)
            throw logic_error("Internal error!");

        char* res=0;

        node* beginNode = currNode;

        //try to find place beginning from currnode
        while (availAtNode(currNode,res) < capacity && currNode != endNode())
           currNode = nextNode(currNode);

        //if don't find place then will try to find from root to currNode
        if (currNode == endNode())
        {
            currNode = root;

            while (availAtNode(currNode,res) < capacity && currNode != beginNode)
               currNode = nextNode(currNode);

            if (currNode == beginNode)
               throw logic_error("Not enough(too small fragment) memory!");
        }
        
	//if we are here place exists

        node* tempCurrNode = new (res) node();

        tempCurrNode->next = currNode->next;
        tempCurrNode->prev = adress2Index(currNode);
        tempCurrNode->size = roundedSize;

        node* nextnode = nextNode(currNode);

        currNode->next = adress2Index(tempCurrNode);


        if (nextnode != endNode())
            nextnode->prev = adress2Index(tempCurrNode);


        currNode = tempCurrNode;

        currentAvailable -= capacity;

        return res + sizeof(node);
    }
    void *ReAlloc(void *Pointer, unsigned int Size)
    {
        if (Pointer < Memory || Pointer >= endNode())
           throw logic_error("Adress out of range!");

        if (Size > currentAvailable)
            throw logic_error("Not enough memory!");

        if (Size ==0)
            throw logic_error("Size must be greater than zero!");

        currNode = reinterpret_cast<node*>(Pointer)-1;

        if (currNode == root)
            throw logic_error("Root pointer is not reallocable!");

        char* res=0;
        size_t capacity = availAtNode(currNode,res) + currNode->size;

        size_t roundedSize = roundSize(Size);

        //only updating size 
        //
        if (roundedSize < capacity)
        {
            currentAvailable -= currNode->size;
            currentAvailable += roundedSize;
            currNode->size = roundedSize;
            return Pointer;
        }

        void* dst = Alloc(Size);

        memcpy(dst,Pointer,currNode->size);

        Free(Pointer);

        currNode = reinterpret_cast<node*>(dst)-1;

        return dst;
    }

    void Free(void *Pointer)
    {
        if (Pointer < Memory || Pointer >= endNode())
           throw logic_error("Adress out of range!");

	//No anymore checks after that
	//consider this pointer as result of Allor or ReAlloc

        currNode = reinterpret_cast<node*>(Pointer)-1;

        if (currNode == root)
            throw logic_error("Root pointer is not removable!");



        node* prev = prevNode(currNode);
        node* next = nextNode(currNode);

        prev->next = adress2Index(next);


        if (next != endNode())
            next->prev = adress2Index(prev);


        currentAvailable -= currNode->size + sizeof(node);

        currNode = prev;

    }

    void print()
    {
        node* temp = root;

        while (true)
        {
            temp = nextNode(temp);

            if (temp == endNode())
                break;

            cout << "index = " << adress2Index(temp) << " size = " << temp->size << endl;
        }

        cout << endl;
    }

private:

    struct node
    {
        unsigned int prev; //prev block in array  
        unsigned int next; //next block in array 
        size_t   size; //used memory after this block 
        unsigned int padding; 

    };

     
    unsigned int roundSize(const unsigned int Size) const
    {
        //Size += sizeof(node);

        if (Size%align==0)
            return Size;

        return (Size/align + 1)*16;
    }

    size_t availAtNode(node* nd,char*& baseFreeMem)
    {
        if (!nd)
            throw logic_error("Bad node adress!");

        size_t res = nd->next - adress2Index(nd) - (nd->size + sizeof(node));

        if (res>0)
            baseFreeMem = reinterpret_cast<char*>(nd+1) + nd->size;

        return res;
    }

    node* endNode()
    {
        return reinterpret_cast<node*>(Memory + AllCapacity);
    }

    node* nextNode(const node* curr)
    {
        if (!curr)
            throw logic_error("Bad node adress!");

        return reinterpret_cast<node*> (Memory + curr->next);
    }

    node* prevNode(const node* curr)
    {
        if (!curr)
            throw logic_error("Bad node adress!");

        return reinterpret_cast<node*>(Memory + curr->prev);
    }

    unsigned int adress2Index(node* ptr) const
    {
        return reinterpret_cast<char*>( ptr) - Memory;
    }

    static const unsigned int align = 16;
    static const size_t AllCapacity = 1048576;
    static const unsigned int endIndex = AllCapacity;


    char Memory[AllCapacity];

    size_t currentAvailable;

    node* root;
    node* currNode;

};



int main()
{
    int maxMemory = 1048576 -16;

    SmallAllocator allocator;

    void* p1 = allocator.Alloc(maxMemory/2-16);
    void* p2 = allocator.Alloc(maxMemory/2-16);
    //void* p3 = allocator.Alloc(100);

    allocator.print();

    allocator.Free(p1);
    allocator.print();
    allocator.Free(p2);
    allocator.print();

    //allocator.ReAlloc(p1,300);
    //allocator.print();

//    allocator.Free(p3);
//    allocator.print();

    //cout << sizeof(node) << endl;
    return 0;
}

