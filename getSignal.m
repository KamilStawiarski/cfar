function out=getSignal(_datalength, _objectCell=[], _objectSNR=[])
    noiseVariance=1;
    out=randn(_datalength,noiseVariance)'; % sygnal z jednostkowa wariancja
    objectsAmplitude=10.^(_objectSNR/20); % przeliczenie SNR na amplitudy
    for objectIdx=1:length(_objectCell) % przejscie przez wszystkie obiekty
        out(_objectCell(objectIdx))=out(_objectCell(objectIdx))+objectsAmplitude(objectIdx); % dodanie sygnału od obiektu
    end
end
