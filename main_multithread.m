clear
clc
close all

pkg load parallel

objectIndexes=[400]; % wektor obiektow
objectSNR=[6]; % wektor SNR im odpowiadajacych [dB]
datalength=2048; % dlugosc danych
CFAR_GUARD_CELLS=4;
CFAR_TRAINING_CELLS=16;
CFAR_THRES=[0:0.2:30]; % wektor progow detekcji

Pd=zeros(1,length(CFAR_THRES));
Pf=zeros(1,length(CFAR_THRES));

loopsNo=1e2;

threadLambda=@(x)threadFunction(datalength,objectIndexes,objectSNR,CFAR_THRES,CFAR_TRAINING_CELLS,CFAR_GUARD_CELLS); % lambda dla wielowatkowosci
jobsParallel=100*nproc; % jaka liczba symulacji zostanie wykonana w przeciagu jednej petli (nproc - liczba rdzeni cpu)

for loopIdx=1:loopsNo
    [threadPd, threadPf]=pararrayfun(nproc, threadLambda,[1:jobsParallel]',"UniformOutput", false); % przetwarzanie rownolegle
    Pd=((loopIdx-1)*Pd+mean(cell2mat(threadPd)))/loopIdx; % powiekszenie Pd
    Pf=((loopIdx-1)*Pf+mean(cell2mat(threadPf)))/loopIdx; % powiekszenie Pf
    % dziwny zapis powyzej powoduje ze wraz z kolejnymi iteracjami wynik jest coraz dokladniejszy

    if mod(loopIdx,10)==0
        semilogy(CFAR_THRES,Pd)
        hold on
        semilogy(CFAR_THRES,Pf,'r')
        xlabel("T");
        ylabel("Pd/Pfa")
        title([num2str(loopIdx*jobsParallel) ' symulacji'])
        hold off
        drawnow
    end
end
