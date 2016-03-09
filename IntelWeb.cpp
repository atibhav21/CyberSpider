//
//  IntelWeb.cpp
//  CyberSpider
//
//  Created by Atibhav Mittal on 3/7/16.
//  Copyright © 2016 ati. All rights reserved.
//

#include <stdio.h>
#include "IntelWeb.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
using namespace std;

IntelWeb::IntelWeb()
{
    
}

bool IntelWeb::createNew(const std::string& filePrefix, unsigned int maxItems)
{
    if(map_fileToSite.createNew(filePrefix+ "map1.dat", maxItems) == false)
    {
        return false;
    }
    if(map_siteToFile.createNew(filePrefix+ "map2.dat", maxItems) == false)
    {
        map_fileToSite.close();
        return false;
    }
    return true;
}

bool IntelWeb::openExisting(const std::string& filePrefix)
{
    if(map_fileToSite.openExisting(filePrefix+ "map1.dat") == false)
    {
        return false;
    }
    if(map_siteToFile.openExisting(filePrefix+ "map2.dat") == false)
    {
        map_fileToSite.close();
        return false;
    }
    return true;
}

void IntelWeb::close()
{
    map_fileToSite.close();
    map_siteToFile.close();
}

bool IntelWeb::ingest(const std::string& telemetryFile)
{
    ifstream inf(telemetryFile);
    if(! inf)
    {
        return false;
    }
    
    string line;
    while(getline(inf,line))
    {
        istringstream iss(line);
        string key,value,context;
        if(! (iss >> key >> value >> context))
        {
            cerr<<"Ignoring line due to bad input format"<<line;
        }
        if(map_fileToSite.insert(key, value, context) == false)
        {
            cerr<<"Error in inserting into file"<<endl;
            return false;
        }
        if(map_siteToFile.insert(value, key, context) == false)
        {
            cerr<<"Error in inserting into file"<<endl;
            return false;
        }
        
    }
    return true;
}

bool IntelWeb::malItemsContains(const std::vector<std::string>& badEntitiesFound, std::string& val) const
{
    if(find(badEntitiesFound.begin(), badEntitiesFound.end(), val)!= badEntitiesFound.end())
    {
        return true;
    }
    return false;
}

bool compare(const InteractionTuple& I1, const InteractionTuple& I2)
{
    if(I1.context< I2.context)
    {
        return true;
    }
    else if(I1.context == I2.context)
    {
        if(I1.from < I2.from)
        {
            return true;
        }
        else if(I1.from > I2.from)
        {
            return false;
        }
        else
        {
            if(I1.to < I2.to)
            {
                return true;
            }
            else if(I1.to > I2.to)
            {
                return false;
            }
            //otherwise all elements are the same and we do not need to add anything.
        }
    }
    return false;
}

void IntelWeb::addToInteractionsVector(std::vector<InteractionTuple>& interactions, InteractionTuple& I)
{
    //TODO: Possibly use Insertion sort since the elements inside are already sorted?
    //check for duplicates also. Duplicates not allowed
    //ordered by context, then from field, then to field
    bool alreadyContains = false;
    for(vector<InteractionTuple>::iterator it = interactions.begin(); it!= interactions.end(); it++)
    {
        if((*it) == I)
        {
            alreadyContains = true;
        }
    }
    if(alreadyContains == false)
    {
        interactions.push_back(I);
        sort(interactions.begin(), interactions.end(), compare);
    }
    
}

unsigned int IntelWeb::crawl(const std::vector<std::string>& indicators, unsigned int minPrevalenceToBeGood,
                   std::vector<std::string>& badEntitiesFound, std::vector<InteractionTuple>& interactions)
{
    unsigned int maliciousCount = 0;
    vector<std::string>::const_iterator it = indicators.begin();
    for(; it!= indicators.end(); it++)
    {
        malItemsQueue.push(*it);
    }
    while(!malItemsQueue.empty())
    {
        int count = 0;
        std::string val = malItemsQueue.front();
        malItemsQueue.pop();
        DiskMultiMap::Iterator itemIterator = map_fileToSite.search(val);
        while(itemIterator.isValid()) //find possible malicious items
        {
            //this loop is used to find all values associated with the given item and pushes them on the queue if they are possibly malicious items and are not in badEntitiesFound
            MultiMapTuple m = *itemIterator;
            if(malItemsContains(badEntitiesFound, m.value) == false)
            {
                malItemsQueue.push(m.value);
            }
            else
            {
                //push them onto interactions vector?
            }
            ++itemIterator;
        }
        
        if(malItemsContains(badEntitiesFound, val) == true)
        {
            continue;
            //item is already part of the badEntitiesFound so continue with the next item
        }
        itemIterator = map_siteToFile.search(val);
        while(itemIterator.isValid())
        {
            count++;
        }
        if(count < minPrevalenceToBeGood)
        {
            if(find(badEntitiesFound.begin(), badEntitiesFound.end(), val) == badEntitiesFound.end())
            {
                //not already present in the badEntitiesFound vector
                badEntitiesFound.push_back(val);
                maliciousCount++;
            }
        }
        /*itemIterator = map_siteToFile.search(val);
        while(itemIterator.isValid())
        {
            count++;
        }
        if(count < minPrevalenceToBeGood)
        {
            
        }*/
    }
    return maliciousCount;
}

bool IntelWeb::purge(const std::string& entity)
{
    bool returnValue = false;
    // use an iterator to get the entity from each map. Using the iterator, find the key if entity is a value and erase the entire item.
    DiskMultiMap::Iterator it = map_fileToSite.search(entity);
    while(it.isValid())
    {
        //erases items where it is the key.
        if(map_fileToSite.erase((*it).key, (*it).value, (*it).context) > 0)
        {
            map_siteToFile.erase((*it).value, (*it).key, (*it).context);
            returnValue = true;
        }
        ++it;
    }
    it = map_siteToFile.search(entity);
    while(it.isValid())
    {
        //erase items where it is the value
        if(map_siteToFile.erase((*it).key, (*it).value, (*it).context) > 0)
        {
            map_fileToSite.erase((*it).value, (*it).key, (*it).context);
            returnValue = true;
        }
        ++it;
    }
    return returnValue;
    
}

IntelWeb::~IntelWeb()
{
    map_fileToSite.close();
    map_siteToFile.close();
}