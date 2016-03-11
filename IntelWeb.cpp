//
//  IntelWeb.cpp
//  CyberSpider
//
//  Created by Atibhav Mittal on 3/7/16.
//  Copyright © 2016 ati. All rights reserved.
//

#include <stdio.h>
#include "IntelWeb.h"
#include "DiskMultiMap.h"
#include "MultiMapTuple.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <set>
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
    //cout<<"Ingest Method Called"<<endl;
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
        if(! (iss >> context >> key >> value))
        {
            cerr<<"Ignoring line due to bad input format"<<line;
        }
        //possible bug with insert method
        if(map_fileToSite.insert(key, value, context) == false)
        {
            cout<<"Error in inserting into file"<<endl;
            return false;
        }
        if(map_siteToFile.insert(value, key, context) == false)
        {
            cout<<"Error in inserting into file"<<endl;
            return false;
        }
    }
    return true;
}

unsigned int IntelWeb::crawl(const std::vector<std::string>& indicators, unsigned int minPrevalenceToBeGood,
                   std::vector<std::string>& badEntitiesFound, std::vector<InteractionTuple>& interactions)
{
    
    int maliciousCount = 0;
    vector<string> maliciousKeyValues;
    set<string> badEntitiesFoundSorted;
    set<InteractionTuple> badInteractionsSorted;
    for(vector<string>::const_iterator it = indicators.begin(); it!= indicators.end(); it++)
    {
        maliciousKeyValues.push_back(*it);
    }
    for(int i = 0; i<maliciousKeyValues.size(); i++) //O(Tlog T)
    {
        string val;
        val = maliciousKeyValues[i];
        DiskMultiMap::Iterator keyIterator = map_fileToSite.search(val);
        while(keyIterator.isValid()) //O(N/B)
        {
            MultiMapTuple M = *keyIterator;
            InteractionTuple I;
            I.context = M.context;
            I.from = M.key;
            I.to = M.value;
            badInteractionsSorted.insert(I);
            if(find(maliciousKeyValues.begin(), maliciousKeyValues.end(), M.value) ==
                                                                        maliciousKeyValues.end())
            {
                /*possibleMaliciousKeyValues.push_back(M.value);*/
                int count = 0;
                DiskMultiMap::Iterator it1 = map_fileToSite.search(M.value);
                while(it1.isValid() && count<minPrevalenceToBeGood)
                {
                    //number of times item is key
                    ++it1;
                    count++;
                }
                it1 = map_siteToFile.search(M.value);
                while(it1.isValid() && count<minPrevalenceToBeGood)
                {
                    //number of times item is value
                    ++it1;
                    count++;
                }
                if(count<minPrevalenceToBeGood)
                {
                    //badEntitiesFound.push_back(M.value);
                    badEntitiesFoundSorted.insert(M.value);
                    maliciousKeyValues.push_back(M.value);
                    maliciousCount++;
                }
                
            }
            ++keyIterator;
        }
        DiskMultiMap::Iterator it = map_siteToFile.search(val);
        while(it.isValid())
        {
            // Check in the opposite map
            MultiMapTuple M= *it;
            InteractionTuple I;
            I.context = M.context;
            I.from = M.value;
            I.to = M.key;
            badInteractionsSorted.insert(I); //since its a set, no duplicates will be inserted
            if(find(maliciousKeyValues.begin(), maliciousKeyValues.end(), M.value) == maliciousKeyValues.end())
            {
                int count = 0;
                DiskMultiMap::Iterator it2 = map_siteToFile.search(M.value);
                while(it2.isValid() && count<minPrevalenceToBeGood)
                {
                    count++;
                    ++it2;
                }
                it2 = map_fileToSite.search(M.value);
                while(it2.isValid() && count<minPrevalenceToBeGood)
                {
                    count++;
                    ++it2;
                }
                if(count<minPrevalenceToBeGood)
                {
                    //badEntitiesFound.push_back(M.value);
                    badEntitiesFoundSorted.insert(M.value);
                    maliciousKeyValues.push_back(M.value);
                    maliciousCount++;
                }
            }
            ++it;
        }
    }
    for(set<string>::iterator it = badEntitiesFoundSorted.begin(); it!=badEntitiesFoundSorted.end(); it++)
    {
        badEntitiesFound.push_back(*it);
    }
    for(set<InteractionTuple>::iterator it = badInteractionsSorted.begin(); it!= badInteractionsSorted.end(); it++)
    {
        interactions.push_back(*it);
    }
    //cout<<"End of method"<<endl;
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
        MultiMapTuple M = *it;
        ++it;
        if(map_fileToSite.erase(M.key, M.value, M.context) > 0)
        {
            map_siteToFile.erase(M.value, M.key, M.context);
            returnValue = true;
        }
    }
    it = map_siteToFile.search(entity);
    while(it.isValid())
    {
        //erase items where it is the value
        MultiMapTuple M = *it;
        ++it;
        if(map_siteToFile.erase(M.key, M.value, M.context) > 0)
        {
            map_fileToSite.erase(M.value, M.key, M.context);
            returnValue = true;
        }
    }
    return returnValue;
    
}

IntelWeb::~IntelWeb()
{
    map_fileToSite.close();
    map_siteToFile.close();
}