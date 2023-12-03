function [Pd, Pfa]=threadFunction(datalength,objectIndexes,objectSNR,CFAR_THRES,CFAR_TRAINING_CELLS,CFAR_GUARD_CELLS)
    inputSignal=getSignal(datalength,objectIndexes,objectSNR);
    detected=CFAR_GO(inputSignal,CFAR_THRES,CFAR_TRAINING_CELLS,CFAR_GUARD_CELLS);
    Pd=zeros(1,length(CFAR_THRES));
    Pfa=zeros(1,length(CFAR_THRES));
    for tIdx=1:length(CFAR_THRES)
        detectedObjectsNo=sum(ismember(objectIndexes,detected{tIdx})); % liczba wykrytych obiektow
        Pd(tIdx)=detectedObjectsNo/length(objectIndexes); % powiekszenie Pd
        Pfa(tIdx)=((length(detected{tIdx})-detectedObjectsNo)/datalength); % powiekszenie Pf
    end
end

