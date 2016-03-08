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

unsigned int IntelWeb::crawl(const std::vector<std::string>& indicators, unsigned int minPrevalenceToBeGood,
                   std::vector<std::string>& badEntitiesFound, std::vector<InteractionTuple>& interactions)
{
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
                badEntitiesFound.push_back(val);
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
    
}

IntelWeb::~IntelWeb()
{
    map_fileToSite.close();
    map_siteToFile.close();
}