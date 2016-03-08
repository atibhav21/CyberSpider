//
//  DiskMultiMap.cpp
//  CyberSpider
//
//  Created by Atibhav Mittal on 3/5/16.
//  Copyright © 2016 ati. All rights reserved.
//

#include <stdio.h>
#include "DiskMultiMap.h"
#include "BinaryFile.h"
#include "MultiMapTuple.h"
#include <cassert>
#include <functional>

/*HashMap::HashMap(int numBuckets)
{
    m_array = new Node*[numBuckets];
    m_numBuckets = numBuckets;
    for(int i = 0 ; i< m_numBuckets; i++)
    {
        m_array[i] = nullptr;
    }
}


int HashMap::hashFunction(char *data)
{
    std::hash<std::string> str_hash;
    int index = str_hash(data) % m_numBuckets;
    return index;
}

Node* HashMap::insertionNode(char *data)
{
    int index = hashFunction(data);
    return m_array[index];
    
}

Node* HashMap::get(char* data)
{
    return m_array[hashFunction(data)];
}


template <typename T>
T* HashMap<T>::remove(char* data)
{
    int index = hashFunction(data);
    m_array.erase(m_array.begin()+ index);
}*/

DiskMultiMap::Iterator::Iterator()
{
    m_posFromStart = -1;
    m_validity = false;
}

DiskMultiMap::Iterator::Iterator(const BinaryFile::Offset m_offset, BinaryFile* bf, const std::string& key)
{
    m_file = bf;
    m_posFromStart = m_offset;
    m_file->read(m_node, m_offset);
    m_key = key;
    if(m_node.next == -1)
    {
        m_validity = false;
    }
    else
    {
        m_validity = true;
    }
}

void DiskMultiMap::Iterator::setInvalid()
{
    m_validity = false;
}

bool DiskMultiMap::Iterator::isValid() const
{
    return m_validity;
}

DiskMultiMap::Iterator& DiskMultiMap::Iterator::operator++()
{
    if(m_node.next == -1)
    {
        setInvalid();
        return *this;
    }
    m_posFromStart = m_node.next;
    m_file->read(m_node, m_posFromStart);
    return *this;
}

MultiMapTuple DiskMultiMap::Iterator::operator*()
{
    MultiMapTuple x;
    x.key = m_key;
    x.value = std::string(m_node.m_data);
    x.context = std::string(m_node.m_context);
    return x;
}

DiskMultiMap::DiskMultiMap()
{
    
}

BinaryFile::Offset DiskMultiMap::hashFunction(const char* data)
{
    std::hash<std::string> str_hash;
    BinaryFile::Offset index = str_hash(data) % numBuckets;
    return startOfMap + index* sizeof(Node);
    
    //return the offset of the first link in the hash map.
}

bool DiskMultiMap::createNew(const std::string& filename, unsigned int nBuckets)
{
    //TODO: Check for load factor
    
    
    //fileMap = new HashMap(numBuckets);
    //websiteMap = new HashMap(numBuckets);
    if(bf.createNew(filename) == false)
    {
        return false;
    }
    //TODO: write some preliminary data about the file in the beginning of the file over here
    // write stuff like start of map1, numberofbuckets, offset of first deleted node
    firstDeletedNode = -1;
    BinaryFile::Offset offset = 0;
    bf.write(numBuckets, offset);
    offset+= sizeof(numBuckets);
    bf.write(firstDeletedNode, offset);
    //also write the offset for the head node of the deleted linked list.
    offset+= sizeof(firstDeletedNode);
    
    startOfMap = offset;
    
    m_filename = filename;
    
    numBuckets = nBuckets*4 /3;
    string emptyString = "";
    Node emptyNode;//(emptyString.c_str(), emptyString.c_str(), 0);
    strcpy(emptyNode.m_data, "");
    strcpy(emptyNode.m_context, "");
    for(int i = 0; i<numBuckets; i++)
    {
        BinaryFile::Offset m_offset = startOfMap + i*sizeof(Node);
        emptyNode.m_offset = m_offset;
        emptyNode.next = -1;
        bf.write(emptyNode,m_offset);
        //write numBuckets number of empty nodes in the binary file
        
        //write numBuckets number of empty binary file offsets to beginning of
    }
    bf.close();
    bf.openExisting(filename);
    return true;
}

bool DiskMultiMap::openExisting(const std::string& filename)
{
    if(bf.isOpen() == true)
    {
        bf.close();
    }
    if(bf.openExisting(filename) == false)
    {
        return false;
    }
    int offset = 0;
    bf.read(numBuckets, offset);
    offset+= sizeof(numBuckets);
    bf.read(firstDeletedNode, offset);
    offset+= sizeof(firstDeletedNode);
    bf.read(startOfMap, offset);
    m_filename = filename;
    return true;
}

void DiskMultiMap::close()
{
    if(bf.isOpen())
    {
        bf.close();
    }
}

bool DiskMultiMap::insert(const std::string &key, const std::string &value, const std::string& context)
{
    if(strlen(key.c_str())>= 121 || strlen(value.c_str())>=121 || strlen(context.c_str())>=121)
    {
        cerr<<"Some Input Value is greater than 121 bytes"<<endl;
        return false;
    }
    //get the offset of the
    BinaryFile::Offset index = hashFunction(key.c_str());
    Node temp;
    //Node nextNode;
    if(bf.read(temp, index) == false)
        cerr<<"Error in Reading node";
    //bf.read(nextNode, temp.next);
    
    
    while(temp.next!= -1) //O(N/B) efficiency
    {
        bf.read(temp, temp.next);
        
        //bf.read(nextNode, nextNode.next);
    }
    //temp now points to the last node in this list.
    
    
    //set firstDeleted Node to the next node which has been deleted
    if(firstDeletedNode!= -1)
    {
        
        Node deletedNode;
        bf.read(deletedNode, firstDeletedNode);
        firstDeletedNode = deletedNode.next;
        //if(deletedNode.next!= -1)
        //{
            temp.next = firstDeletedNode;
            Node newNode;//(value.c_str(),context.c_str(), deletedNode.m_offset);
            strcpy(newNode.m_data, value.c_str());
            strcpy(newNode.m_context, context.c_str());
            newNode.m_offset = deletedNode.m_offset;
            newNode.next = -1;
            newNode.deleted = false;
            if(bf.write(newNode, newNode.m_offset) == false)
            {
                cerr<<"Error in reusing Node"<<endl;
                return false;
            }
            else
            {
                cerr<<"Node Reused"<<endl;
            }
        //}
    }
    else
    {
        //no deleted nodes available so just write at the end of the file
        Node newNode;//(value.c_str(), context.c_str(), bf.fileLength());
        strcpy(newNode.m_data, value.c_str());
        strcpy(newNode.m_context, context.c_str());
        newNode.m_offset = bf.fileLength();
        newNode.next = -1;
        temp.next = newNode.m_offset;
        bf.write(temp, temp.m_offset);
        if(bf.write(newNode, newNode.m_offset)== false)
        {
            cerr<<"Error in allocating new Node due to no Empty nodes available"<<endl;
            return false;
        }
    }
    bf.close();
    bf.openExisting(m_filename);
    return true;
    
}

void DiskMultiMap::addToDeleted(BinaryFile::Offset offset)
{
    Node temp;
    bf.read(temp, offset);
    temp.deleted = true;
    temp.next = firstDeletedNode;
    firstDeletedNode = temp.m_offset;
    bf.write(temp, offset);
    bf.write(firstDeletedNode, sizeof(int)); //write the first deleted node at the beginning of the file
    bf.close();
    bf.openExisting(m_filename);
}

DiskMultiMap::Iterator DiskMultiMap::search(const std::string& key)
{
    //TODO: Check if it works
    BinaryFile::Offset m_offset = hashFunction(key.c_str());
    Iterator it(m_offset, &bf, key);
    if((*it).value == "" || (*it).context == "")
    {
        ++it;
    }
    return it;
}

//returns the number of items deleted
int DiskMultiMap::erase(const std::string& key, const std::string& value, const std::string& context)
{
    //add deleted nodes to the front of the deleted nodes linked list
    //TODO: Make all the head nodes, dummy nodes with just a binary file offset to the first node in their linked list
    //TODO: code does not check for the temp node. Fix that. Solution: Make temp initially the dummy node
    int index = hashFunction(key.c_str());
    int count = 0;
    Node temp;
    Node nextNode;
    bf.read(temp, index);
    bf.read(nextNode, temp.next);
    while(temp.next!= -1)
    {
        if(strcmp(value.c_str(), nextNode.m_data) == 0 && strcmp(context.c_str(), nextNode.m_context)== 0)
        {
            //TODO: Relinking of nodes. Possibly need two Nodes. Write the node in the binary file once you change the next
            temp.next = nextNode.next;
            bf.write(temp, temp.m_offset);
            addToDeleted(nextNode.m_offset);
            count++;
        }
        else
        {
            //only increment temp if item has not been deleted.
            temp = nextNode;
        }
        bf.read(nextNode, nextNode.next);
    }
    bf.close();
    bf.openExisting(m_filename);
    return count;
}

DiskMultiMap::~DiskMultiMap()
{
    close();
}