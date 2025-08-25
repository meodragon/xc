//
// Created by Khang on 8/24/2025.
//

#ifndef MODEL_H
#define MODEL_H

#include <render_data.h>

class Model {
    public:
        void init();
        Mesh getVertexData();

    private:
        Mesh mVertexData;
};

#endif //MODEL_H
