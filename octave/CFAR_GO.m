function detected=CFAR_GO(_inputData,_Thres,_cfarTC,_cfarGC)
    % _inputData - wektor danych wejsciowych
    % _Thres - wektor posortowanych rosnaco progow detekcji
    % _cfarTC - dlugosc trainig cells
    % _cfarGC - dlugosc guard cells
    % @return - lista z wektorami komorek w ktorych wykryto obiekty o sile wiekszej od progu. Kolejne elementy listy odpowiadaja kolejnym progom
    datalength=length(_inputData);
    _inputData=_inputData.^2; % kwadrat danych - szybciej niz modul
    _extendedInputData=[zeros(1,_cfarGC+_cfarTC) _inputData zeros(1,_cfarGC+_cfarTC)]; % powiekszenie wektora danych o zera na poczatku i koncu
    CFARLR=zeros(1,datalength+_cfarGC+_cfarTC+_cfarGC+1); % wektor otoczenia lewo i prawostronnego
    CFARLR(1)=sum(_extendedInputData(1:_cfarTC)); % poczatkowa suma
    for cellIdx=2:length(CFARLR)
        CFARLR(cellIdx)=CFARLR(cellIdx-1)-_extendedInputData(cellIdx-1)+_extendedInputData(cellIdx+_cfarTC-1); % przesuwne okno
    end
    CFARLR=CFARLR/_cfarTC; % srednia
    CFARLR=max(CFARLR(1:datalength),CFARLR(_cfarGC+_cfarTC+_cfarGC+2:datalength+_cfarGC+_cfarTC+_cfarGC+1)); % wieksze z
    SIGMA=1; % ile razy wieksza ma byc wartosc probki od oszacowania szumu
    _inputData=_inputData-SIGMA*CFARLR(1:datalength); % odjecie CFAR_GO*SIGMA
    _inputData=max(_inputData,zeros(1,datalength)); % progowanie wzgledem zera
    % tu zostaloby progowanie z T, ale zeby przyspieszyc calosc liczone sa detekcje dla wszystkich podanych T
    [detectionValues detectionCells]=sort(_inputData); % sortowanie i zwrocenie wartosci z pierwotnymi indeksami
    detected=cell(1,length(_Thres)); % lista o długosci rownej liczbie elementow wektora progow
    tIdx=1; % indeks progu detekcji
    for detectionIdx=1:datalength
        while detectionValues(detectionIdx)>_Thres(tIdx) % probka jest wieksza od progu
            detected{tIdx}=detectionCells(detectionIdx:end); % przepisujemy indeksy wykryc o amplitudzie wiekszej niz T(tIdx)
            tIdx=tIdx+1;
            if tIdx>length(_Thres)
                return;
            end
        end
    end
end

