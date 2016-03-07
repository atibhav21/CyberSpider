#ifndef DISKMULTIMAP_H_
#define DISKMULTIMAP_H_

#include <string>
#include "MultiMapTuple.h"
#include "BinaryFile.h"
#include <vector>

struct Node
{
    Node()
    {
        deleted = false;
    }
    Node(const char data[], const char context[], BinaryFile::Offset offset)
    {
        strcpy(m_data, data);
        strcpy(m_context, context);
        m_offset = offset;
        deleted = false;
    }
    char m_data[121];
    char m_context[121];
    BinaryFile::Offset m_offset;
    BinaryFile::Offset next;
    bool deleted;
};


/*class HashMap
{
public:
    HashMap(int numBuckets);
    Node* insertionNode(char* data);
    Node* get(char* data);
    int hashFunction(char* data);
    //void remove(char* data);
private:
    Node** m_array;
    int m_numBuckets;
    
};*/


class DiskMultiMap
{
public:
    
    class Iterator
    {
    public:
        Iterator();
        Iterator(const BinaryFile::Offset m_offset,DiskMultiMap*,const std::string& key);
        // You may add additional constructors
        bool isValid() const;
        Iterator& operator++();
        MultiMapTuple operator*();
        
    private:
        BinaryFile::Offset m_posFromStart;
        bool m_validity;
        void setInvalid();
        DiskMultiMap* m_map;
        std::string m_key;
        Node m_node;
        // Your private member declarations will go here
    };
    
    DiskMultiMap();
    ~DiskMultiMap();
    bool createNew(const std::string& filename, unsigned int numBuckets);
    bool openExisting(const std::string& filename);
    void close();
    bool insert(const std::string& key, const std::string& value, const std::string& context);
    Iterator search(const std::string& key);
    int erase(const std::string& key, const std::string& value, const std::string& context);
    
private:
    // Your private member declarations will go here
    BinaryFile::Offset hashFunction(const char* data);
    void addToDeleted(BinaryFile::Offset);
    BinaryFile::Offset startOfMap1;
    //store some initial data in the map
    
    BinaryFile::Offset firstDeletedNode;
    
    //HashMap* fileMap;
    //HashMap* websiteMap;
    BinaryFile bf;
    int numBuckets;
};

#endif // DISKMULTIMAP_H_
