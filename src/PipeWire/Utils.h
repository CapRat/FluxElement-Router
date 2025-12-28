//
// Created by benji on 17.12.25.
//
#include <iostream>
#include <map>
#include <string>
#include <pipewire/pipewire.h>
#ifndef RASPIHOST_UTILS_H
#define RASPIHOST_UTILS_H

namespace PipeWire::Utils {
    inline std::map<std::string, std::string> spaDictToMap(const spa_dict *dict) {
        std::map<std::string, std::string> dictMap;
        const spa_dict_item *item;
        spa_dict_for_each(item, dict) {
            dictMap[item->key] = item->value;
        }
        return dictMap;
    }


}

#endif //RASPIHOST_UTILS_H
