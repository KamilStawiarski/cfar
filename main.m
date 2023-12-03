clear
clc
close all

objectIndexes=[400]; % wektor obiektow
objectSNR=[12]; % wektor SNR im odpowiadajacych [dB]
datalength=2048; % dlugosc danych
CFAR_GUARD_CELLS=4;
CFAR_TRAINING_CELLS=16;
CFAR_THRES=[0:0.2:40]; % wektor progow detekcji

Pd=zeros(1,length(CFAR_THRES));
Pf=zeros(1,length(CFAR_THRES));

loopsNo=1e3;

for loopIdx=1:loopsNo
    inputSignal=getSignal(datalength,objectIndexes,objectSNR);
    detected=CFAR_GO(inputSignal,CFAR_THRES,CFAR_TRAINING_CELLS,CFAR_GUARD_CELLS);
    for tIdx=1:length(CFAR_THRES)
        detectedObjectsNo=sum(ismember(objectIndexes,detected{tIdx})); % liczba wykrytych obiektow
        Pd(tIdx)=((loopIdx-1)*Pd(tIdx)+detectedObjectsNo/length(objectIndexes))/loopIdx; % powiekszenie Pd
        Pf(tIdx)=((loopIdx-1)*Pf(tIdx)+(length(detected{tIdx})-detectedObjectsNo)/datalength)/loopIdx; % powiekszenie Pf
        % dziwny zapis powyzej powoduje ze wraz z kolejnymi iteracjami wynik jest coraz dokladniejszy
    end

    if mod(loopIdx,100)==0
        semilogy(CFAR_THRES,Pd)
        hold on
        semilogy(CFAR_THRES,Pf,'r')
        xlabel("T");
        ylabel("Pd/Pfa")
        title([num2str(loopIdx) ' symulacji'])
        hold off
        drawnow
    end
end
