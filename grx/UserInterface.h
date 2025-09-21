//
// Created by Khang on 9/20/2025.
//

#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include "render_data.h"
#include <model_and_instance_data.h>

class UserInterface {
public:
    bool init(RenderData& renderData);
    void hideMouse(bool hide);

    void createFrame(RenderData& renderData, ModelAndInstanceData &modInstData);
    void render(RenderData& renderData);

    void cleanup(RenderData& renderData);

private:
    float mFramesPerSecond = 0.0f;
    /* averaging speed */
    float mAveragingAlpha = 0.96f;

    std::vector<float> mFPSValues{};
    int mNumFPSValues = 90;

    std::vector<float> mFrameTimeValues{};
    int mNumFrameTimeValues = 90;

    std::vector<float> mModelUploadValues{};
    int mNumModelUploadValues = 90;

    std::vector<float> mMatrixGenerationValues{};
    int mNumMatrixGenerationValues = 90;

    std::vector<float> mMatrixUploadValues{};
    int mNumMatrixUploadValues = 90;

    std::vector<float> mUiGenValues{};
    int mNumUiGenValues = 90;

    std::vector<float> mUiDrawValues{};
    int mNumUiDrawValues = 90;

    float mNewFps = 0.0f;
    double mUpdateTime = 0.0;

    int mFpsOffset = 0;
    int mFrameTimeOffset = 0;
    int mModelUploadOffset = 0;
    int mMatrixGenOffset = 0;
    int mMatrixUploadOffset = 0;
    int mUiGenOffset = 0;
    int mUiDrawOffset = 0;

    int mManyInstanceCreateNum = 1;
};

#endif //USERINTERFACE_H
