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
            cerr<<"Error in inserting into file"<<endl;
            return false;
        }
        if(map_siteToFile.insert(value, key, context) == false)
        {
            cerr<<"Error in inserting into file"<<endl;
            return false;
        }
        //cerr<<context<<" "<<key<<" "<<value<<endl;
    }
    return true;
}

/*bool IntelWeb::malItemsContains(const std::vector<std::string>& badEntitiesFound, std::string& val) const
{
    if(find(badEntitiesFound.begin(), badEntitiesFound.end(), val)!= badEntitiesFound.end())
    {
        return true;
    }
    return false;
}*/

bool IntelWeb::interactionVectorContains(std::vector<InteractionTuple>& interactions, InteractionTuple& I)
{
    //TODO: Possibly use Insertion sort since the elements inside are already sorted?
    //check for duplicates also. Duplicates not allowed
    //ordered by context, then from field, then to field
    for(vector<InteractionTuple>::iterator it = interactions.begin(); it!= interactions.end(); it++)
    {
        if((*it) == I)
        {
            return true;
        }
    }
    return false;
    
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
    //while(possibleMaliciousKeyValues.empty() == false)
    {
        string val;
        val = maliciousKeyValues[i]; //possibleMaliciousKeyValues[i];
        DiskMultiMap::Iterator keyIterator = map_fileToSite.search(val);
        while(keyIterator.isValid()) //O(N/B)
        {
            MultiMapTuple M = *keyIterator;
            InteractionTuple I;
            I.context = M.context;
            I.from = M.key;
            I.to = M.value;
            if(interactionVectorContains(interactions, I) == false)
            {
                badInteractionsSorted.insert(I);
            }
            if(find(maliciousKeyValues.begin(), maliciousKeyValues.end(), M.value) ==
                                                                        maliciousKeyValues.end())
            {
                /*possibleMaliciousKeyValues.push_back(M.value);*/
                int count = 0;
                DiskMultiMap::Iterator it1 = map_fileToSite.search(M.value);
                while(it1.isValid())
                {
                    //number of times item is key
                    ++it1;
                    count++;
                }
                it1 = map_siteToFile.search(M.value);
                while(it1.isValid())
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
            // Check in the second back.
            MultiMapTuple M= *it;
            InteractionTuple I;
            I.context = M.context;
            I.from = M.value;
            I.to = M.key;
            if(interactionVectorContains(interactions, I) == false)
            {
                badInteractionsSorted.insert(I);
            }
            if(find(maliciousKeyValues.begin(), maliciousKeyValues.end(), M.value) == maliciousKeyValues.end())
            {
                int count = 0;
                DiskMultiMap::Iterator it2 = map_siteToFile.search(M.value);
                while(it2.isValid())
                {
                    count++;
                    ++it2;
                }
                it2 = map_fileToSite.search(M.value);
                while(it2.isValid())
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
    cout<<"End of method"<<endl;
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