//
//  IntelWeb.h
//  CyberSpider
//
//  Created by Atibhav Mittal on 3/7/16.
//  Copyright © 2016 ati. All rights reserved.
//

#ifndef INTELWEB_H_
#define INTELWEB_H_

#include "InteractionTuple.h"
#include "DiskMultiMap.h"
#include <string>
#include <vector>
#include <queue>

class IntelWeb
{
public:
    IntelWeb();
    ~IntelWeb();
    bool createNew(const std::string& filePrefix, unsigned int maxDataItems);
    bool openExisting(const std::string& filePrefix);
    void close();
    bool ingest(const std::string& telemetryFile);
    unsigned int crawl(const std::vector<std::string>& indicators,
                       unsigned int minPrevalenceToBeGood,
                       std::vector<std::string>& badEntitiesFound,
                       std::vector<InteractionTuple>& interactions
                       );
    bool purge(const std::string& entity);
    
private:
    // Your private member declarations will go here
    DiskMultiMap map_fileToSite; // maps from first argument to second argument
    DiskMultiMap map_siteToFile; // maps from second argument to first argument
    
    //bool malItemsContains(const std::vector<std::string>& badEntitiesFound, std::string& val) const;
    bool interactionVectorContains(std::vector<InteractionTuple>& interactions, InteractionTuple& I);
};

inline
bool operator==(const InteractionTuple& I1, const InteractionTuple& I2)
{
    if(I1.from == I2.from && I1.to == I2.to && I1.context == I2.context)
    {
        return true;
    }
    return false;
}

inline
bool operator<(const InteractionTuple& I1, const InteractionTuple& I2)
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

#endif // INTELWEB_H_
