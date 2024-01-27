#ifndef CFAR_H
#define CFAR_H

#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <thread>

struct config
{
    uint32_t cellsNo;
    std::vector<uint32_t> targetsCells;
    std::vector<float> targetsSNR;
    float noiseVariance;
    uint32_t guardCells;
    uint32_t trainingCells;
    std::vector<float> thresholdVector;
};

class dataGenerator
{
public:
    dataGenerator(const config &_cfg)
    {
        cfg=_cfg;
        gen=std::mt19937{rd()};
        dist=std::normal_distribution<float>{0.0f, cfg.noiseVariance};
        objectsAmplitude.resize(cfg.targetsCells.size());
        for(uint32_t i=0; i<objectsAmplitude.size();++i) objectsAmplitude[i]=powf(10.0f, cfg.targetsSNR[i]/20.0f)*cfg.noiseVariance*cfg.noiseVariance;
    }
    void getSignal(std::vector<float> &_outData)
    {
        for(uint32_t i=0; i<_outData.size();++i) _outData[i]=dist(gen);
        for(uint32_t i=0; i<objectsAmplitude.size();++i) _outData[cfg.targetsCells[i]]+=objectsAmplitude[i];
    }
private:
    std::random_device rd;
    std::mt19937 gen;
    std::normal_distribution<float> dist;
    config cfg;
    std::vector<float> objectsAmplitude;
};

class probabilityCalculator
{
public:
    probabilityCalculator(const config &_cfg)
    {
        cfg=_cfg;
        pdVec.resize(cfg.thresholdVector.size());
        pfaVec.resize(cfg.thresholdVector.size());
        targetsAmpl.resize(cfg.targetsCells.size());
    }
    const std::vector<float> &getPfa(void) const {return pfaVec;}
    const std::vector<float> &getPd(void) const {return pdVec;}
    void calcPdPfa(std::vector<float> &_inputData)
    {
        float objectsNo=(float)(cfg.targetsCells.size());
        float possibleFalseDetection=(float)(cfg.cellsNo-cfg.targetsCells.size());
        for(uint32_t i=0;i<cfg.targetsCells.size();++i) targetsAmpl[i]=_inputData[cfg.targetsCells[i]]; // przepisujemy amplitudy - do liczenia Pd
        for(uint32_t i=0;i<cfg.targetsCells.size();++i) _inputData[cfg.targetsCells[i]]=-1.0f; // żeby nie powodowały fałszywych wykryć
        std::sort(targetsAmpl.begin(), targetsAmpl.end());
        std::sort(_inputData.begin(), _inputData.end());
        float pd=0.0f;
        float pfa=0.0f;
        float loopIdx=(float)(loopsNo);
        float loopIdx1=(float)(loopsNo+1);

        uint32_t targetIter=0;
        uint32_t cellIter=0;
        for(uint32_t i=0;i<cfg.thresholdVector.size();++i)
        {
            float T=cfg.thresholdVector[i];
            pd=0.0f;
            if(targetIter<targetsAmpl.size())
            {
                while(T>targetsAmpl[targetIter] && targetIter<targetsAmpl.size()) targetIter++;
                pd=((float)(targetsAmpl.size()-targetIter))/objectsNo;
            }
            pfa=0.0f;
            if(cellIter<_inputData.size())
            {
                while(T>_inputData[cellIter] && cellIter<_inputData.size()) cellIter++;
                pfa=((float)(_inputData.size()-cellIter))/possibleFalseDetection;
            }
            pdVec[i]=((pdVec[i]*loopIdx)+pd)/(loopIdx1);
            pfaVec[i]=((pfaVec[i]*loopIdx)+pfa)/(loopIdx1);
        }
        loopsNo++;
    }
    uint32_t getLoopsNo(void) {return loopsNo;}
private:
    config cfg;
    uint32_t loopsNo=0;
    std::vector<float> pdVec;
    std::vector<float> pfaVec;
    std::vector<float> targetsAmpl;
};

class CFAR
{
public:
    CFAR(const config &_cfg)
    {
        cfg=_cfg;
        signalVector.resize(cfg.cellsNo+cfg.guardCells*2+cfg.trainingCells*2);
        CFARLR.resize(cfg.cellsNo+cfg.guardCells+cfg.trainingCells+cfg.guardCells+1);
        for(uint32_t i=0;i<CFARLR.size();++i) CFARLR[i]=-10.0f;
        for(uint32_t i=0;i<cfg.guardCells+1;++i) CFARLR[i]=0;
        for(uint32_t i=CFARLR.size()-(cfg.guardCells+1);i<CFARLR.size();++i) CFARLR[i]=0;

    }
    void CFAR_GO(std::vector<float> &_inputData)
    {
        for(uint32_t i=0;i<cfg.guardCells+1;++i) CFARLR[i]=0.0f;
        for(uint32_t i=0;i<cfg.cellsNo;++i) _inputData[i]*=_inputData[i];
        for(uint32_t i=0;i<cfg.trainingCells;++i) CFARLR[cfg.guardCells+1+i]=CFARLR[cfg.guardCells+1+i-1]+_inputData[i];
        for(uint32_t i=cfg.trainingCells;i<cfg.cellsNo;++i) CFARLR[cfg.guardCells+1+i]=CFARLR[cfg.guardCells+1+i-1]+_inputData[i]-_inputData[i-cfg.trainingCells];
        for(uint32_t i=cfg.cellsNo;i<cfg.cellsNo+cfg.trainingCells;++i) CFARLR[cfg.guardCells+1+i]=CFARLR[cfg.guardCells+1+i-1]-_inputData[i-cfg.trainingCells];
        for(uint32_t i=0;i<CFARLR.size();++i) CFARLR[i]/=(float)cfg.trainingCells;
        for(uint32_t i=0;i<cfg.cellsNo;++i) CFARLR[i]=std::max(getCFARL()[i],getCFARR()[i]);
        for(uint32_t i=0;i<cfg.cellsNo;++i) _inputData[i]-=CFARLR[i];
        for(uint32_t i=0;i<cfg.cellsNo;++i) _inputData[i]=std::max(_inputData[i],0.0f);
    }
private:
    config cfg;
    std::vector<float> signalVector;
    float *getCFARL(void) {return &CFARLR[0];}
    float *getCFARR(void) {return &CFARLR[cfg.trainingCells+cfg.guardCells+cfg.guardCells+1];}
    std::vector<float> CFARLR;
};

class threader
{
public:
    threader()
    {
        nthreads=std::thread::hardware_concurrency();
    }
    ~threader()
    {
        for(uint32_t i=0;i<nthreads;++i)
            thrV[i].join();
    }
    void runThreads(const config _cfg)
    {
        std::vector<probabilityCalculator*> pcv;
        pcv.resize(nthreads);
        for(uint32_t i=0;i<nthreads;++i)
            thrV.push_back(std::thread(&threader::threadFunction,this,_cfg, std::ref(pcv[i])));
        auto start=std::chrono::high_resolution_clock::now();
        std::chrono::milliseconds sleepTime(1000);
        while(1)
        {
            std::this_thread::sleep_for(sleepTime);
            uint32_t loopsTotal=0;
            for(uint32_t i=0;i<_cfg.thresholdVector.size();i=i+5)
            {
                loopsTotal=0;
                float pd=0.0f;
                float pfa=0.0f;
                for(auto it=pcv.begin();it!=pcv.end();++it) loopsTotal+=(*it)->getLoopsNo();
                for(auto it=pcv.begin();it!=pcv.end();++it)
                {
                    pfa+=(*it)->getPfa()[i]*(*it)->getLoopsNo()/loopsTotal;
                    pd+=(*it)->getPd()[i]*(*it)->getLoopsNo()/loopsTotal;
                }
                std::cout<<"T="<<_cfg.thresholdVector[i]<<" pd="<<pd<<" pfa="<<pfa<<"\n";
            }
            auto diff=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-start);
            std::cout<<(double)loopsTotal/(float)diff.count()*1000.0f<<" loops/s\n";
            std::cout<<"LoopsTotal="<<loopsTotal<<"\n";
        }
    }
private:
    void threadFunction(const config _cfg, probabilityCalculator* &_pc)
    {
        std::vector<float> signal;
        signal.resize(_cfg.cellsNo);
        dataGenerator DG(_cfg);
        CFAR C(_cfg);
        probabilityCalculator PC(_cfg);
        _pc=&PC;
        while(1)
        {
            DG.getSignal(signal);
            C.CFAR_GO(signal);
            PC.calcPdPfa(signal);
        }
    }
    std::vector<std::thread> thrV;
    uint32_t nthreads;
};

#endif
