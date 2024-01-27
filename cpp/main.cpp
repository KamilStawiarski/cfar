#include <iostream>
#include <chrono>
#include "CFAR.h"

int main(void)
{
    config cfg;
    cfg.targetsCells=std::vector<uint32_t>{400};
    cfg.targetsSNR=std::vector<float>{12};
    cfg.noiseVariance=1.0;
    cfg.cellsNo=2048;
    cfg.guardCells=4;
    cfg.trainingCells=16;
    for(float t=0.0; t<=30.0f; t+=0.2f)
        cfg.thresholdVector.push_back(t);


    threader thr;
    thr.runThreads(cfg);

	return 0;
}
