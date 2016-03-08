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
#include "InteractionTuple.h"

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
    queue<std::string> malItemsQueue;
    bool malItemsContains(const std::vector<std::string>& badEntitiesFound, std::string& val) const;
    void addToInteractionsVector(std::vector<InteractionTuple>& interactions, InteractionTuple& I);
};

#endif // INTELWEB_H_
