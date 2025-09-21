//
// Created by Khang on 8/24/2025.
//

#ifndef MODELANDINSTANCEDATA_H
#define MODELANDINSTANCEDATA_H

#include <memory>
#include <unordered_set>
#include <functional>

class AssimpModel;
class AssimpInstance;

using modelCheckCallback = std::function<bool(std::string)>;
using modelAddCallback = std::function<bool(std::string)>;
using modelDeleteCallback = std::function<void(std::string)>;

using instanceAddCallback = std::function<std::shared_ptr<AssimpInstance>(std::shared_ptr<AssimpModel>)>;
using instanceAddManyCallback = std::function<void(std::shared_ptr<AssimpModel>, int)>;
using instanceDeleteCallback = std::function<void(std::shared_ptr<AssimpInstance>)>;
using instanceCloneCallback = std::function<void(std::shared_ptr<AssimpInstance>)>;

struct ModelAndInstanceData {
    std::vector<std::shared_ptr<AssimpModel>> miModelList{};
    int miSelectedModel = 0;

    std::vector<std::shared_ptr<AssimpInstance>> miAssimpInstances{};
    std::unordered_map<std::string, std::vector<std::shared_ptr<AssimpInstance>>> miAssimpInstancesPerModel{};
    int miSelectedInstance = 0;

    /* we can only delete models in Vulkan outside the command buffers,
     * so let's use a separate pending list */
    std::unordered_set<std::shared_ptr<AssimpModel>> miPendingDeleteAssimpModels{};

    /* callbacks */
    modelCheckCallback miModelCheckCallbackFunction;
    modelAddCallback miModelAddCallbackFunction;
    modelDeleteCallback miModelDeleteCallbackFunction;

    instanceAddCallback miInstanceAddCallbackFunction;
    instanceAddManyCallback miInstanceAddManyCallbackFunction;
    instanceDeleteCallback miInstanceDeleteCallbackFunction;
    instanceCloneCallback miInstanceCloneCallbackFunction;
};

#endif //MODELANDINSTANCEDATA_H
